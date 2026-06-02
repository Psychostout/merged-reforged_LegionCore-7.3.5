/*
* Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
* Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Common.h"
#include "ObjectMgr.h"
#include "BattlePayMgr.h"
// #include <sstream>
// #include <iomanip>
#include "WorldSession.h"
#include "Player.h"
#include "BattlePayData.h"
#include "DatabaseEnv.h"
#include "QuestData.h"
#include "LoginQueryHolder.h"
#include "ScriptMgr.h"
#include "AccountMgr.h"
#include "PetBattle.h"
#include "BattlePetData.h"
#include "CharacterService.h"
#include "CollectionMgr.h"
#include "Chat.h"
#include "Config.h"

using namespace Battlepay;

BattlepayManager::BattlepayManager(WorldSession* session)
{
    _session = session;
    _walletName = "Donation points";
    _purchaseIDCount = 0;
    _distributionIDCount = 0;
}

BattlepayManager::~BattlepayManager() = default;

void BattlepayManager::RegisterStartPurchase(Purchase purchase)
{
    _actualTransaction = purchase;
}

uint64 BattlepayManager::GenerateNewPurchaseID()
{
    return uint64(0x1E77800000000000 | ++_purchaseIDCount);
}

uint64 BattlepayManager::GenerateNewDistributionId()
{
    return uint64(0x1E77800000000000 | ++_distributionIDCount);
}

Purchase* BattlepayManager::GetPurchase()
{
    return &_actualTransaction;
}

std::string const& BattlepayManager::GetDefaultWalletName() const
{
    return _walletName;
}

BattlePayCurrency BattlepayManager::GetShopCurrency() const
{
    return static_cast<BattlePayCurrency>(
        sConfigMgr->GetIntDefault("BattlePay.CurrencyID", 4));
}

bool BattlepayManager::IsAvailable() const
{
    bool isMod = AccountMgr::IsModeratorAccount(_session->GetSecurity());
    bool configEnabled = sWorld->getBoolConfig(CONFIG_FEATURE_SYSTEM_BPAY_STORE_ENABLED);
    if (isMod)
        return true;

    return configEnabled;
}

std::string Product::Serialize() const
{
    std::string res;
    res += "Expansion: " + std::to_string(CURRENT_EXPANSION);
    res += ", Type: " + std::to_string(WebsiteType);
    res += ", Quantity: " + std::to_string(1);
    res += ", IngameShop: " + std::to_string(1);
    res += ", CustomData: " + sScriptMgr->BattlePayGetCustomData(*this);

    uint32 idx = 0;
    for (auto const& itr : Items)
    {
        std::string iconName;
        if (auto itemTemplate = sObjectMgr->GetItemTemplate(itr.ItemID))
        {
            if (auto fileDataId = sDB2Manager.GetItemDIconFileDataId(itr.ItemID))
                iconName = std::to_string(fileDataId);

            switch (WebsiteType)
            {
            case Item:
                res += ", Item(ItemID: " + std::to_string(itr.ItemID) + ", Quality: " + std::to_string(itemTemplate->GetQuality()) + ", Icon: " + iconName + ")";
                break;
            case PackItems:
                res += ", Pack(Icon: " + iconName + ", ItemsEntry: " + std::to_string(itr.ItemID) + ", Num: " + std::to_string(idx) + ")";
                break;
            default:
                break;
            }
        }

        idx++;
    }

    return res;
}

void BattlepayManager::ProcessDelivery(Purchase* purchase)
{
    // _existProducts.insert
    auto player = _session->GetPlayer(); // atm only ingame shop -_-

    auto const* product = sBattlePayDataStore->GetProduct(purchase->ProductID);
    if (!product)
        return;

    switch (product->WebsiteType)
    {
    case Battlepay::Item:
        for (auto const& itr : product->Items)
            if (player)
                player->AddItem(itr.ItemID, itr.Quantity);
        break;
    case Battlepay::BattlePet:
        if (player)
            for (auto const& itr : product->Items)
                player->AddBattlePetByCreatureId(itr.ItemID, true, true);
        break;
    case Rename:
        if (player)
            sCharacterService->SetRename(player);
        break;
    case Faction:
        if (player)
            sCharacterService->ChangeFaction(player);
        break;
    case DeletedCharacter:
        sCharacterService->RestoreDeletedCharacter(_session);
        break;
    case Customization:
        if (player)
            sCharacterService->Customize(player);
        break;
    case Race:
        if (player)
            sCharacterService->ChangeRace(player);
        break;
    case CharacterBoost:
    {
        if (!sWorld->getBoolConfig(CONFIG_CHARACTER_BOOST_ENABLED))
            break;

        // Determine target level from ScriptName
        uint8 targetLevel = 0;
        if (product->ScriptName.find("level90") != std::string::npos)       targetLevel = 90;
        else if (product->ScriptName.find("level100") != std::string::npos) targetLevel = 100;

        if (!targetLevel)
            break;

        // Persist boost flag FIRST so BuildPendingBoostDistributions includes it
        {
            AuthFlags flag = (targetLevel <= 90) ? AT_AUTH_FLAG_90_LVL_UP : AT_AUTH_FLAG_100_LVL_UP;
            _session->AddAuthFlag(flag);
        }

        // Build distributions (Status=AVAILABLE) and send to client.
        auto distributions = BuildPendingBoostDistributions();

        // 1. DistributionListResponse
        {
            WorldPackets::BattlePay::DistributionListResponse distResp;
            distResp.DistributionObject = distributions;
            _session->SendPacket(distResp.Write());
        }

        // 2. DistributionUpdate — fires PRODUCT_DISTRIBUTIONS_UPDATED
        for (auto const& dist : distributions)
        {
            WorldPackets::BattlePay::DistributionUpdate update;
            update.DistributionObject = dist;
            _session->SendPacket(update.Write());
        }

        _session->SendCharacterEnum();

        return; // Skip script call — CharacterBoost is fully handled here
    }

    //case Category:
    //    break;
    //case Battlepay::Spell:
    //    break;
    //case Currency:
    //    break;
    //case GuildRename:
    //    break;
    //case Gold:
    //    break;
    //case Level:
    //    break;
    //case PremadeCharacter:
    //    break;
    //case RealmTransfer:
    //    break;
    //case ExpansionTransfer:
    //    break;
    //case Premium:
    //    break;
    //case PackItems:
    //    break;
    //case ItemProfession:
    //    break;
    //case Transmogrification:
    //    break;
    //case CategoryProfession:
    //    break;
    //case CategoryPremade:
    //    break;
    //case ItemMount:
    //    break;
    //case CategoryCharacterManagement:
    //    break;
    //case CategoryRealmTransfer:
    //    break;
    //case CategoryExpansionTransfer:
    //    break;
    //case CategoryGold:
    //    break;
    default:
        break;
    }

    if (!product->ScriptName.empty())
        sScriptMgr->OnBattlePayProductDelivery(_session, product);
}

bool BattlepayManager::AlreadyOwnProduct(uint32 itemId) const
{
    //if (_existProducts.find(productId) != _existProducts.end())
    //    return turue;

    auto const& player = _session->GetPlayer();
    if (player)
    {
        auto itemTemplate = sObjectMgr->GetItemTemplate(itemId);
        if (!itemTemplate)
            return true;

        for (auto itr : itemTemplate->Effects)
            if (itr->TriggerType == ITEM_SPELLTRIGGER_LEARN_SPELL_ID && player->HasSpell(itr->SpellID))
                return true;

        if (player->GetCollectionMgr()->HasToy(itemId))
            return true;
    }

    return false;
}

auto BattlepayManager::ProductFilter(Product product) -> bool
{
    auto player = _session->GetPlayer();
    if (!player)
    {
        switch (product.WebsiteType)
        {
        case Battlepay::BattlePet:
        case Rename:
        case Faction:
        case DeletedCharacter:
        case Customization:
        case Race:
        case CharacterBoost:
            //case Category:
            //    break;
            //case Battlepay::Spell:
            //    break;
            //case Currency:
            //    break;
            //case GuildRename:
            //    break;
            //case Gold:
            //    break;
            //case Level:
            //    break;
        case PremadeCharacter:
            //    break;
            //case RealmTransfer:
            //    break;
            //case ExpansionTransfer:
            //    break;
        case Premium:
            //    break;
            //case PackItems:
            //    break;
            //case ItemProfession:
            //    break;
            //case Transmogrification:
            //    break;
            //case CategoryProfession:
            //    break;
            //case CategoryPremade:
            //    break;
        case ItemMount:
            //    break;
            //case CategoryCharacterManagement:
            //    break;
            //case CategoryRealmTransfer:
            //    break;
            //case CategoryExpansionTransfer:
            //    break;
            //case CategoryGold:
            //    break;
            return true;
        default:
            return false;
        }
    }

    if (product.ClassMask && (player->getClassMask() & product.ClassMask) == 0)
        return false;

    for (auto& itr : product.Items)
    {
        if (AlreadyOwnProduct(itr.ItemID))
            return false;

        if (auto itemTemplate = sObjectMgr->GetItemTemplate(itr.ItemID))
        {
            if (itemTemplate->AllowableClass && (itemTemplate->AllowableClass & player->getClassMask()) == 0)
                return false;

            if (itemTemplate->AllowableRace && (itemTemplate->AllowableRace & player->getRaceMask()) == 0)
                return false;

            if (itemTemplate->GetMinFactionID() && uint32(player->GetReputationRank(itemTemplate->GetMinFactionID())) < itemTemplate->GetMinReputation())
                return false;

            for (auto effectData : itemTemplate->Effects)
            {
                if (effectData->SpellID != 0 && effectData->TriggerType == ITEM_SPELLTRIGGER_LEARN_SPELL_ID)
                {
                    if (auto spellInfo = sSpellMgr->GetSpellInfo(effectData->SpellID))
                    {
                        if (spellInfo->HasAttribute(SPELL_ATTR7_HORDE_ONLY) && (player->getRaceMask() & RACEMASK_HORDE) == 0)
                            return false;

                        if (spellInfo->HasAttribute(SPELL_ATTR7_ALLIANCE_ONLY) && (player->getRaceMask() & RACEMASK_ALLIANCE) == 0)
                            return false;
                    }
                }
            }
        }
    }

    return true;
};

void BattlepayManager::SendProductList()
{
    WorldPackets::BattlePay::ProductListResponse response;
    if (!IsAvailable())
    {
        TC_LOG_INFO("server.battlepay", "SendProductList: not available, sending LockUnk1");
        response.Result = ProductListResult::LockUnk1;
        _session->SendPacket(response.Write());
        return;
    }

    auto const& player = _session->GetPlayer();
    auto const& localeIndex = _session->GetSessionDbLocaleIndex();

    response.Result = ProductListResult::Available;
    response.ProductList.CurrencyID = GetShopCurrency();

    for (auto& itr : sBattlePayDataStore->GetProductGroups())
    {
        if (!player && itr.IngameOnly)
            continue;

        if (itr.OwnsTokensOnly && _session->GetTokenBalance(itr.TokenType) <= 0)
            continue;

        WorldPackets::BattlePay::BattlePayProductGroup pGroup;
        pGroup.GroupID = itr.GroupID;
        pGroup.IconFileDataID = itr.IconFileDataID;
        pGroup.Ordering = itr.Ordering;
        pGroup.Flags = itr.Flags;
        pGroup.IsAvailableDescription = "";
        pGroup.DisplayType = itr.DisplayType;

        auto name = itr.Name;
        if (auto productLocale = sBattlePayDataStore->GetProductGroupLocale(itr.GroupID))
            ObjectMgr::GetLocaleString(productLocale->Name, localeIndex, name);
        pGroup.Name = name;
        response.ProductList.ProductGroup.emplace_back(pGroup);
    }

    for (auto const& itr : sBattlePayDataStore->GetShopEntries())
    {
        Battlepay::ProductGroup* productGroup = sBattlePayDataStore->GetProductGroup(itr.GroupID);
        if (!productGroup)
            continue;

        if (!player && productGroup->IngameOnly)
            continue;

        if (productGroup->OwnsTokensOnly && _session->GetTokenBalance(productGroup->TokenType) <= 0)
            continue;

        WorldPackets::BattlePay::BattlePayShopEntry sEntry;
        sEntry.EntryID = itr.EntryID;
        sEntry.GroupID = itr.GroupID;
        sEntry.ProductID = itr.ProductID;
        sEntry.Ordering = itr.Ordering;
        sEntry.VasServiceType = itr.Flags;
        sEntry.StoreDeliveryType = itr.BannerType;

        auto data = WriteDisplayInfo(itr.DisplayInfoID, localeIndex);
        if (std::get<0>(data))
        {
            sEntry.DisplayInfo.emplace();
            sEntry.DisplayInfo = std::get<1>(data);
        }

        response.ProductList.Shop.emplace_back(sEntry);
    }

    for (auto const& itr : sBattlePayDataStore->GetProducts())
    {
        auto const& product = itr.second;
        if (!ProductFilter(product))
            continue;

        int64 tokenBalance = 0;
        Battlepay::ProductGroup* productGroup = sBattlePayDataStore->GetProductGroupForProductId(product.ProductID);
        if (!productGroup)
        {
            // CharacterBoost products must appear in ProductListResponse even without a shop entry,
            // because GetUpgradeDistributions() cross-references distribution ProductID against the
            // client's product cache (populated by ProductListResponse).
            if (product.WebsiteType != Battlepay::WebsiteType::CharacterBoost)
                continue;
            // CharacterBoost without a shop entry: always available on character select (not IngameOnly)
        }
        else
        {
            if (!player && productGroup->IngameOnly)
                continue;

            tokenBalance = _session->GetTokenBalance(productGroup->TokenType);
            if (productGroup->OwnsTokensOnly && tokenBalance <= 0)
                continue;
        }

        WorldPackets::BattlePay::ProductInfoStruct pInfo;
        pInfo.NormalPriceFixedPoint = product.NormalPriceFixedPoint * g_CurrencyPrecision;
        pInfo.CurrentPriceFixedPoint = product.CurrentPriceFixedPoint * g_CurrencyPrecision;
        pInfo.ProductID = product.ProductID;
        pInfo.ChoiceType = product.ChoiceType;
        pInfo.ProductIDs.emplace_back(product.ProductID);
        //std::vector<uint32> UnkInts;
        pInfo.UnkInt2 = 47; // 2 ?

        auto dataPI = WriteDisplayInfo(product.DisplayInfoID, localeIndex);
        if (std::get<0>(dataPI))
        {
            pInfo.DisplayInfo.emplace();
            pInfo.DisplayInfo = std::get<1>(dataPI);
        }

        bool hideProductPrice = false;
        if (pInfo.DisplayInfo.has_value() && pInfo.DisplayInfo->Flags.has_value())
            hideProductPrice = *pInfo.DisplayInfo->Flags & BattlepayDisplayInfoFlag::HidePrice;
        bool hasEnoughTokens = tokenBalance >= static_cast<int64>(product.CurrentPriceFixedPoint);

        response.ProductList.ProductInfo.emplace_back(pInfo);

        WorldPackets::BattlePay::BattlePayProduct pProduct;
        pProduct.ProductID = product.ProductID;
        pProduct.Flags = product.Flags;
        pProduct.Type = product.Type;

        if (product.WebsiteType == Battlepay::WebsiteType::CharacterBoost)
        {
            if (product.ScriptName.find("level90") != std::string::npos)
                pProduct.UnkBits = 1;
            else
                pProduct.UnkBits = 2;
            //TC_LOG_INFO("server.battlepay", "SendProductList: including CharacterBoost product %u with UnkBits=%u (BoostType)",
            //    product.ProductID, pProduct.UnkBits ? uint32(*pProduct.UnkBits) : 0);
        }

        for (auto& item : product.Items)
        {
            WorldPackets::BattlePay::ProductItem pItem;
            pItem.ID = item.ID;
            pItem.ItemID = product.Items.size() > 1 ? 0 : item.ItemID; ///< Disable tooltip for packs (client handle only one tooltip).
            pItem.Quantity = item.Quantity;
            //pItem.UnkInt1 = 0;
            //pItem.UnkInt2 = 0;
            //pItem.UnkByte = 0;

            // if the product is already owned disable the buy button
            // also disable the button if we don't show the price for a product
            // and the player does not have enough tokens to pay for the product
            pItem.HasPet = AlreadyOwnProduct(item.ItemID) || (hideProductPrice && !hasEnoughTokens);
            pItem.PetResult = item.PetResult;

            auto dataP = WriteDisplayInfo(item.DisplayInfoID, localeIndex);
            if (std::get<0>(dataP))
            {
                pItem.DisplayInfo.emplace();
                pItem.DisplayInfo = std::get<1>(dataP);
            }

            pProduct.Items.emplace_back(pItem);
        }

        auto dataP = WriteDisplayInfo(product.DisplayInfoID, localeIndex);
        if (std::get<0>(dataP))
        {
            pProduct.DisplayInfo.emplace();
            pProduct.DisplayInfo = std::get<1>(dataP);
        }

        response.ProductList.Product.emplace_back(pProduct);
    }



    _session->SendPacket(response.Write());
}

std::tuple<bool, WorldPackets::BattlePay::ProductDisplayInfo> BattlepayManager::WriteDisplayInfo(uint32 displayInfoID, LocaleConstant localeIndex, uint32 productId /*= 0*/)
{
    auto GeneratePackDescription = [localeIndex](Product const* product) -> std::string
    {
        auto getQualityColor = [](uint32 quality) -> std::string
        {
            switch (quality)
            {
            case ITEM_QUALITY_POOR:
                return "|cff9d9d9d";
            case ITEM_QUALITY_NORMAL:
                return "|cffffffff";
            case ITEM_QUALITY_UNCOMMON:
                return "|cff1eff00";
            case ITEM_QUALITY_RARE:
                return "|cff0070dd";
            case ITEM_QUALITY_EPIC:
                return "|cffa335ee";
            case ITEM_QUALITY_LEGENDARY:
                return "|cffff8000";
            case ITEM_QUALITY_ARTIFACT:
                return "|cffe5cc80";
            case ITEM_QUALITY_HEIRLOOM:
                return "|cffe5cc80";
            default:
                return "|cffe5cc80";
            }
        };

        std::string res;
        for (auto itr : product->Items)
            if (auto itemTemplate = sObjectMgr->GetItemTemplate(itr.ItemID))
                res += getQualityColor(itemTemplate->GetQuality()) + itemTemplate->GetName()->Get(localeIndex) + "\n";
        return res;
    };

    auto info = WorldPackets::BattlePay::ProductDisplayInfo();
    if (!displayInfoID)
        return std::make_tuple(false, info);

    auto displayInfo = sBattlePayDataStore->GetDisplayInfo(displayInfoID);
    if (!displayInfo)
        return std::make_tuple(false, info);

    auto displayLocale = sBattlePayDataStore->GetDisplayInfoLocale(displayInfoID);

    info.Name1 = displayInfo->Name1;
    if (displayLocale)
        ObjectMgr::GetLocaleString(displayLocale->Name1, localeIndex, info.Name1);

    info.Name2 = displayInfo->Name2;
    if (displayLocale)
        ObjectMgr::GetLocaleString(displayLocale->Name2, localeIndex, info.Name2);

    info.Name3 = displayInfo->Name3;
    if (productId)
    {
        auto product = sBattlePayDataStore->GetProduct(productId);
        if (!product->Items.empty())
            info.Name3 = GeneratePackDescription(product);
    }
    else if (displayLocale)
        ObjectMgr::GetLocaleString(displayLocale->Name3, localeIndex, info.Name3);

    info.Name4 = displayInfo->Name4;
    if (displayLocale)
        ObjectMgr::GetLocaleString(displayLocale->Name4, localeIndex, info.Name4);

    if (displayInfo->CreatureDisplayInfoID != 0)
        info.CreatureDisplayInfoID = displayInfo->CreatureDisplayInfoID;

    if (auto visualsId = displayInfo->VisualsId)
    {
        if (auto visuals = sBattlePayDataStore->GetDisplayInfoVisuals(displayInfoID))
        {
            info.VisualsId = displayInfo->VisualsId;

            for (auto const& itr : *visuals)
            {
                WorldPackets::BattlePay::ProductDisplayVisualData visual;
                visual.DisplayId = itr.DisplayId;
                visual.VisualId = itr.VisualId;
                visual.ProductName = itr.ProductName;
                info.Visuals.emplace_back(visual);
            }
        }
    }

    if (displayInfo->Flags != 0)
        info.Flags = displayInfo->Flags;

    //Optional<uint32> UnkInt1;
    //Optional<uint32> UnkInt2;
    //Optional<uint32> UnkInt3;

    return std::make_tuple(true, info);
}

void BattlepayManager::SendPointsBalance()
{
    ChatHandler chatHandler(_session);
    if (!_session->GetPlayer())
    {
        return;
    }

    chatHandler.PSendSysMessage("Account name: %s", _session->GetAccountName());

    for (auto& tokenType : sBattlePayDataStore->GetTokenTypes())
    {
        int64 balance = _session->GetTokenBalance(tokenType.first);
        if (balance || tokenType.second.listIfNone)
            chatHandler.PSendSysMessage("%s: %d", tokenType.second.name, balance);
    }
}

void BattlepayManager::SendBattlePayDistribution(uint32 productId, uint8 status, uint64 distributionId, ObjectGuid targetGuid)
{
    WorldPackets::BattlePay::DistributionUpdate distributionBattlePay;
    auto product = sBattlePayDataStore->GetProduct(productId);
    if (!product || !product->ProductID)
        return;

    auto const& localeIndex = _session->GetSessionDbLocaleIndex();
    distributionBattlePay.DistributionObject.DistributionID = distributionId;
    distributionBattlePay.DistributionObject.Status = status;
    distributionBattlePay.DistributionObject.ProductID = productId;
    distributionBattlePay.DistributionObject.PurchaseID = _actualTransaction.PurchaseID;
    distributionBattlePay.DistributionObject.Revoked = false;

    if (!targetGuid.IsEmpty())
    {
        distributionBattlePay.DistributionObject.TargetPlayer = targetGuid;
        distributionBattlePay.DistributionObject.TargetVirtualRealm = GetVirtualRealmAddress();
        distributionBattlePay.DistributionObject.TargetNativeRealm = GetVirtualRealmAddress();
    }

    WorldPackets::BattlePay::BattlePayProduct productData;

    for (auto const& item : product->Items)
    {
        WorldPackets::BattlePay::ProductItem productItem;

        auto dataP = WriteDisplayInfo(item.DisplayInfoID, localeIndex);
        if (std::get<0>(dataP))
        {
            productItem.DisplayInfo.emplace();
            productItem.DisplayInfo = std::get<1>(dataP);
        }

        productItem.PetResult = item.PetResult;
        productItem.ID = item.ID;
        productItem.ItemID = item.ItemID;
        productItem.Quantity = item.Quantity;
        productItem.UnkInt1 = item.DisplayInfoID;
        productItem.UnkInt2 = 0;
        productItem.PetResult = 0;
        productItem.HasPet = item.HasPet;
        productData.Items.emplace_back(productItem);
    }

    auto dataP = WriteDisplayInfo(product->DisplayInfoID, localeIndex);
    if (std::get<0>(dataP))
    {
        productData.DisplayInfo.emplace();
        productData.DisplayInfo = std::get<1>(dataP);
    }

    if (product->WebsiteType == Battlepay::WebsiteType::CharacterBoost)
    {
        if (product->ScriptName.find("level90") != std::string::npos)
            productData.UnkBits = 1;
        else
            productData.UnkBits = 2;
    }

    productData.ProductID = product->ProductID;
    productData.Flags = product->Flags;
    productData.UnkInt1 = 0;
    productData.DisplayId = product->DisplayInfoID;
    productData.ItemId = 0;
    productData.UnkInt4 = 0;
    productData.UnkInt5 = 0;
    productData.UnkString = "";
    productData.Type = 0;
    productData.UnkBit = false;

    distributionBattlePay.DistributionObject.Product = std::move(productData);
    auto const* pkt = distributionBattlePay.Write();
    // TC_LOG_INFO("server.battlepay", "SendBattlePayDistribution: opcode=0x%04X, size=%zu, productId=%u, status=%u, distId=%llu",
    //     pkt->GetOpcode(), pkt->size(), productId, status, distributionId);
    // {
    //     std::ostringstream hex;
    //     size_t len = std::min<size_t>(pkt->size(), 128);
    //     for (size_t i = 0; i < len; ++i)
    //         hex << std::hex << std::setfill('0') << std::setw(2) << (int)pkt->contents()[i] << ' ';
    //     TC_LOG_INFO("server.battlepay", "DistributionUpdate hex: %s", hex.str().c_str());
    // }
    _session->SendPacket(pkt);
}

void BattlepayManager::SendPendingBoostDistributions()
{
    // Called after SendProductList() so the client already knows the products
    struct BoostFlag { AuthFlags flag; uint32 productId; };
    static const BoostFlag boostFlags[] = {
        { AT_AUTH_FLAG_90_LVL_UP,  109 },
        { AT_AUTH_FLAG_100_LVL_UP, 110 },
    };

    for (auto const& bf : boostFlags)
    {
        if (!_session->HasAuthFlag(bf.flag))
            continue;

        auto const* product = sBattlePayDataStore->GetProduct(bf.productId);
        if (!product)
            continue;

        uint64 distId = GenerateNewDistributionId();
        uint64 purchaseId = GenerateNewPurchaseID();

        // Store in _actualTransaction so AssignDistributionToCharacter can find it
        _actualTransaction.ProductID = bf.productId;
        _actualTransaction.DistributionId = distId;
        _actualTransaction.PurchaseID = purchaseId;
        _actualTransaction.Status = DistributionStatus::BATTLE_PAY_DIST_STATUS_AVAILABLE;
        _actualTransaction.TargetCharacter = ObjectGuid::Empty;

        SendBattlePayDistribution(bf.productId, _actualTransaction.Status, distId, ObjectGuid::Empty);
    }
}

std::vector<WorldPackets::BattlePay::BattlePayDistributionObject> BattlepayManager::BuildPendingBoostDistributions()
{
    std::vector<WorldPackets::BattlePay::BattlePayDistributionObject> result;

    //TC_LOG_INFO("server.battlepay", "BuildPendingBoostDistributions: account %u, atAuthFlag=0x%X",
    //    _session->GetAccountId(), _session->GetAF());

    struct BoostFlag { AuthFlags flag; uint32 productId; };
    static const BoostFlag boostFlags[] = {
        { AT_AUTH_FLAG_90_LVL_UP,  109 },
        { AT_AUTH_FLAG_100_LVL_UP, 110 },
    };

    for (auto const& bf : boostFlags)
    {
        if (!_session->HasAuthFlag(bf.flag))
            continue;

        auto const* product = sBattlePayDataStore->GetProduct(bf.productId);
        if (!product || !product->ProductID)
        {
            //TC_LOG_ERROR("server.battlepay", "BuildPendingBoostDistributions: product %u NOT FOUND in data store!", bf.productId);
            continue;
        }

        uint64 distId = GenerateNewDistributionId();
        uint64 purchaseId = GenerateNewPurchaseID();

        // Store in _actualTransaction so AssignDistributionToCharacter can find it later
        _actualTransaction.ProductID = bf.productId;
        _actualTransaction.DistributionId = distId;
        _actualTransaction.PurchaseID = purchaseId;
        // AVAILABLE (1) = "pending boost, ready to assign to a character".
        // GetUpgradeDistributions() returns AVAILABLE distributions → shows the boost button.
        // PROCESS_COMPLETE (3) is for when the boost is actively being applied (send VasPurchaseStarted).
        _actualTransaction.Status = DistributionStatus::BATTLE_PAY_DIST_STATUS_AVAILABLE;
        _actualTransaction.TargetCharacter = ObjectGuid::Empty;

        WorldPackets::BattlePay::BattlePayDistributionObject distObj;
        distObj.DistributionID = distId;
        distObj.Status = DistributionStatus::BATTLE_PAY_DIST_STATUS_AVAILABLE;
        distObj.ProductID = bf.productId;
        distObj.PurchaseID = purchaseId;
        distObj.Revoked = false;

        // Build product data inline so the client can display the boost wizard correctly
        {
            LocaleConstant localeIndex = _session->GetSessionDbLocaleIndex();
            WorldPackets::BattlePay::BattlePayProduct productData;

            for (auto const& item : product->Items)
            {
                WorldPackets::BattlePay::ProductItem productItem;
                auto dataP = WriteDisplayInfo(item.DisplayInfoID, localeIndex);
                if (std::get<0>(dataP))
                {
                    productItem.DisplayInfo.emplace();
                    productItem.DisplayInfo = std::get<1>(dataP);
                }
                productItem.ID = item.ID;
                productItem.ItemID = item.ItemID;
                productItem.Quantity = item.Quantity;
                productItem.UnkInt1 = item.DisplayInfoID;
                productItem.UnkInt2 = 0;
                productItem.PetResult = 0;
                productItem.HasPet = item.HasPet;
                productData.Items.emplace_back(productItem);
            }

            auto dataP = WriteDisplayInfo(product->DisplayInfoID, localeIndex);
            if (std::get<0>(dataP))
            {
                productData.DisplayInfo.emplace();
                productData.DisplayInfo = std::get<1>(dataP);
            }

            if (product->WebsiteType == Battlepay::WebsiteType::CharacterBoost)
            {
                if (product->ScriptName.find("level90") != std::string::npos)
                    productData.UnkBits = 1;
                else
                    productData.UnkBits = 2;
            }

            productData.ProductID = product->ProductID;
            productData.Flags = product->Flags;
            productData.UnkInt1 = 0;
            productData.DisplayId = product->DisplayInfoID;
            productData.ItemId = 0;
            productData.UnkInt4 = 0;
            productData.UnkInt5 = 0;
            productData.UnkString = "";
            productData.Type = 0;
            productData.UnkBit = false;

            distObj.Product = std::move(productData);
        }

        result.emplace_back(std::move(distObj));

        //TC_LOG_INFO("server.battlepay", "BuildPendingBoostDistributions: BUILT distribution for product %u (distId=%llu, purchaseId=%llu, UnkBits=%u, status=PROCESS_COMPLETE)",
        //    bf.productId, distId, purchaseId, (product->ScriptName.find("level90") != std::string::npos) ? 1 : 2);
    }

    //TC_LOG_INFO("server.battlepay", "BuildPendingBoostDistributions: returning %zu distributions", result.size());
    return result;
}


void BattlepayManager::AssignDistributionToCharacter(ObjectGuid const& targetCharGuid, uint64 distributionId, uint32 productId, uint16 specId, uint16 choiceId)
{
    TC_LOG_INFO("server.battlepay", "AssignDistributionToCharacter: account %u, target %s, distId=%llu, productId=%u, specId=%u",
        _session->GetAccountId(), targetCharGuid.ToString().c_str(), distributionId, productId, specId);

    auto const* product = sBattlePayDataStore->GetProduct(productId);
    if (!product)
        return;

    if (product->WebsiteType != Battlepay::CharacterBoost)
    {
        WorldPackets::BattlePay::BattlePayStartDistributionAssignToTargetResponse assignResponse;
        assignResponse.DistributionID = distributionId;
        assignResponse.unkint1 = 0;
        assignResponse.unkint2 = 0;
        _session->SendPacket(assignResponse.Write());
        return;
    }

    if (!sWorld->getBoolConfig(CONFIG_CHARACTER_BOOST_ENABLED))
        return;

    // Determine target level from ScriptName
    uint8 targetLevel = 0;
    if (product->ScriptName.find("level90") != std::string::npos)       targetLevel = 90;
    else if (product->ScriptName.find("level100") != std::string::npos) targetLevel = 100;

    if (!targetLevel)
        return;

    // Verify eligibility
    auto charInfo = sWorld->GetCharacterInfo(targetCharGuid);
    if (!charInfo)
        return;

    if (charInfo->AccountId != _session->GetAccountId())
        return;

    TC_LOG_INFO("server.battlepay", "AssignDistributionToCharacter: char %s level=%u, targetLevel=%u",
        targetCharGuid.ToString().c_str(), charInfo->Level, targetLevel);

    if (charInfo->Level >= targetLevel)
    {
        TC_LOG_INFO("server.battlepay", "AssignDistributionToCharacter: EARLY RETURN — char level %u >= targetLevel %u",
            charInfo->Level, targetLevel);
        return;
    }

    // Apply the boost via DB
    sCharacterService->BoostCharacter(_session, targetCharGuid, targetLevel, 0, 0, 0.f, 0.f, 0.f, 0.f, false, specId);

    // Send UpgradeStarted
    WorldPackets::BattlePay::UpgradeStarted upgrade;
    upgrade.CharacterGUID = targetCharGuid;
    _session->SendPacket(upgrade.Write());

    // Send equipment items for gear fly-in animation
    uint8 loadoutType = (targetLevel >= 100) ? 6 : 3;
    WorldPackets::BattlePay::BattlePayCharacterUpgradeQueued upgradeQueued;
    upgradeQueued.Character = targetCharGuid;
    upgradeQueued.EquipmentItems = sDB2Manager.GetItemLoadOutItemsByClassID(charInfo->Class, loadoutType)[0];
    _session->SendPacket(upgradeQueued.Write());

    // Send assign response
    WorldPackets::BattlePay::BattlePayStartDistributionAssignToTargetResponse assignResponse;
    assignResponse.DistributionID = distributionId;
    assignResponse.unkint1 = 0;
    assignResponse.unkint2 = 0;
    _session->SendPacket(assignResponse.Write());

    // Clear auth flags
    if (_session->HasAuthFlag(AT_AUTH_FLAG_90_LVL_UP))
        _session->RemoveAuthFlag(AT_AUTH_FLAG_90_LVL_UP);
    if (_session->HasAuthFlag(AT_AUTH_FLAG_100_LVL_UP))
        _session->RemoveAuthFlag(AT_AUTH_FLAG_100_LVL_UP);

    // Set distribution finished
    auto purchase = GetPurchase();
    purchase->Status = DistributionStatus::BATTLE_PAY_DIST_STATUS_FINISHED;
    SendBattlePayDistribution(productId, purchase->Status, distributionId, targetCharGuid);

    // Delay UpgradeComplete + character list refresh to let client play the animation
    ObjectGuid charGuid = targetCharGuid;
    _session->AddDelayedEvent(3000, [this, charGuid]()
    {
        WorldPackets::BattlePay::UpgradeComplete upgradeComplete;
        upgradeComplete.CharacterGUID = charGuid;
        _session->SendPacket(upgradeComplete.Write());

        _session->SendCharacterEnum();
    });
}

void BattlepayManager::Update(uint32 diff)
{
    auto& data = _actualTransaction;
    auto product = sBattlePayDataStore->GetProduct(data.ProductID);
    if (!product)
        return;

    switch (data.Status)
    {
    case DistributionStatus::BATTLE_PAY_DIST_STATUS_ADD_TO_PROCESS:
    case DistributionStatus::BATTLE_PAY_DIST_STATUS_PROCESS_COMPLETE:
    case DistributionStatus::BATTLE_PAY_DIST_STATUS_FINISHED:
    case DistributionStatus::BATTLE_PAY_DIST_STATUS_AVAILABLE:
    case DistributionStatus::BATTLE_PAY_DIST_STATUS_NONE:
    default:
        break;
    }
}

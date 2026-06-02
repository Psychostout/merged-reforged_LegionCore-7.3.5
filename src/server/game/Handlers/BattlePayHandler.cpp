/*
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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

#include "BattlePayPackets.h"
#include "BattlePayMgr.h"
#include "BattlePayData.h"
#include "CharacterService.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "World.h"
#include "DatabaseEnv.h"

auto GetBagsFreeSlots = [](Player* player) -> uint32
{
    uint32 freeBagSlots = 0;
    for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; i++)
        if (auto bag = player->GetBagByPos(i))
            freeBagSlots += bag->GetFreeSlots();

    uint8 inventoryEnd = INVENTORY_SLOT_ITEM_START + player->GetInventorySlotCount();
    for (uint8 i = INVENTORY_SLOT_ITEM_START; i < inventoryEnd; i++)
        if (!player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            ++freeBagSlots;

    return freeBagSlots;
};

auto SendStartPurchaseResponse = [](WorldSession* session, Battlepay::Purchase const& purchase, Battlepay::Error const& result) -> void
{
    WorldPackets::BattlePay::StartPurchaseResponse response;
    response.PurchaseID = purchase.PurchaseID;
    response.ClientToken = purchase.ClientToken;
    response.PurchaseResult = result;
    session->SendPacket(response.Write());
};

auto SendPurchaseUpdate = [](WorldSession* session, Battlepay::Purchase const& purchase, uint32 result) -> void
{
    WorldPackets::BattlePay::PurchaseUpdate packet;
    WorldPackets::BattlePay::BattlePayPurchase data;
    data.PurchaseID = purchase.PurchaseID;
    data.UnkLong = 0;
    data.UnkLong2 = 0;
    data.Status = purchase.Status;
    data.ResultCode = result;
    data.ProductID = purchase.ProductID;
    data.UnkInt = purchase.ServerToken;
    data.WalletName = session->GetBattlePayMgr()->GetDefaultWalletName();
    packet.Purchase.emplace_back(data);
    session->SendPacket(packet.Write());
};

void WorldSession::HandleGetPurchaseListQuery(WorldPackets::BattlePay::GetPurchaseListQuery& /*packet*/)
{
    WorldPackets::BattlePay::PurchaseListResponse packet;
    //uint32 Result = 0;
    //std::vector<WorldPackets::BattlePay::BattlePayPurchase> Purchase;
    SendPacket(packet.Write());
}

void WorldSession::HandleUpdateVasPurchaseStates(WorldPackets::BattlePay::UpdateVasPurchaseStates& /*packet*/)
{
    //TC_LOG_INFO("server.battlepay", "HandleUpdateVasPurchaseStates: account %u, atAuthFlag=0x%X",
    //    GetAccountId(), GetAF());

    if (!GetBattlePayMgr()->IsAvailable())
        return;

    auto distributions = GetBattlePayMgr()->BuildPendingBoostDistributions();

    // 1. DistributionListResponse — creates client distribution cache (Status=AVAILABLE)
    {
        WorldPackets::BattlePay::DistributionListResponse distResp;
        distResp.DistributionObject = distributions;
        SendPacket(distResp.Write());
    }

    // 2. DistributionUpdate — fires PRODUCT_DISTRIBUTIONS_UPDATED
    for (auto const& dist : distributions)
    {
        WorldPackets::BattlePay::DistributionUpdate update;
        update.DistributionObject = dist;
        SendPacket(update.Write());
    }
}

void WorldSession::HandleBattlePayDistributionAssign(WorldPackets::BattlePay::DistributionAssignToTarget& packet)
{
    TC_LOG_INFO("server.battlepay", "HandleBattlePayDistributionAssign: account %u, target %s, distId=%llu, productId=%u",
        GetAccountId(), packet.TargetCharacter.ToString().c_str(), packet.DistributionID, packet.ProductID);

    if (!GetBattlePayMgr()->IsAvailable())
        return;

    GetBattlePayMgr()->AssignDistributionToCharacter(packet.TargetCharacter, packet.DistributionID, packet.ProductID, packet.SpecializationID, packet.ChoiceID);
}

void WorldSession::HandleGetProductList(WorldPackets::BattlePay::GetProductList& /*packet*/)
{
    TC_LOG_INFO("server.battlepay", "HandleGetProductList: account %u, player %s, IsAvailable=%u",
        GetAccountId(),
        GetPlayer() ? GetPlayer()->GetName() : "<charselect>",
        GetBattlePayMgr()->IsAvailable() ? 1 : 0);

    if (!GetBattlePayMgr()->IsAvailable())
    {
        TC_LOG_INFO("server.battlepay", "HandleGetProductList: store not available, sending LockUnk1 response");
        WorldPackets::BattlePay::ProductListResponse response;
        response.Result = Battlepay::ProductListResult::LockUnk1;
        SendPacket(response.Write());
        return;
    }

    GetBattlePayMgr()->SendProductList();
    GetBattlePayMgr()->SendPointsBalance();

    // Send distributions AFTER ProductListResponse so the client has product data cached.
    {
        auto distributions = GetBattlePayMgr()->BuildPendingBoostDistributions();

        //TC_LOG_INFO("server.battlepay", "HandleGetProductList: sending DistributionListResponse with %zu entries after ProductList",
        //    distributions.size());

        // 1. DistributionListResponse
        {
            WorldPackets::BattlePay::DistributionListResponse distResp;
            distResp.DistributionObject = distributions;
            SendPacket(distResp.Write());
        }

        // 2. DistributionUpdate — fires PRODUCT_DISTRIBUTIONS_UPDATED
        for (auto const& dist : distributions)
        {
            WorldPackets::BattlePay::DistributionUpdate update;
            update.DistributionObject = dist;
            SendPacket(update.Write());
        }
    }
}

auto MakePurchase = [](ObjectGuid targetCharacter, uint32 clientToken , uint32 productID, WorldSession* session) -> void
{
    if (!session || !session->GetBattlePayMgr()->IsAvailable())
        return;

    auto mgr = session->GetBattlePayMgr();
    auto player = session->GetPlayer();
    auto accountID = session->GetAccountId();

    TC_LOG_INFO("server.battlepay", "MakePurchase: account %u, player %s, productID %u, target %s",
        accountID,
        player ? player->GetName() : "<charselect>",
        productID,
        targetCharacter.ToString().c_str());

    Battlepay::Purchase purchase;
    purchase.ProductID = productID;
    purchase.ClientToken = clientToken;
    purchase.TargetCharacter = targetCharacter;
    purchase.Status = Battlepay::UpdateStatus::Loading;
    purchase.DistributionId = mgr->GenerateNewDistributionId();

    auto characterInfo = sWorld->GetCharacterInfo(targetCharacter);
    if (!characterInfo)
    {
        TC_LOG_INFO("server.battlepay", "MakePurchase: target character not found");
        SendStartPurchaseResponse(session, purchase, Battlepay::Error::PurchaseDenied);
        return;
    }

    if (characterInfo->AccountId != accountID)
    {
        TC_LOG_INFO("server.battlepay", "MakePurchase: character belongs to another account");
        SendStartPurchaseResponse(session, purchase, Battlepay::Error::PurchaseDenied);
        return;
    }

    if (!sBattlePayDataStore->ProductExist(productID))
    {
        TC_LOG_INFO("server.battlepay", "MakePurchase: product %u does not exist", productID);
        SendStartPurchaseResponse(session, purchase, Battlepay::Error::PurchaseDenied);
        return;
    }

    Battlepay::ProductGroup* group = sBattlePayDataStore->GetProductGroupForProductId(purchase.ProductID);
    if (!group)
    {
        TC_LOG_INFO("server.battlepay", "MakePurchase: no product group for product %u", productID);
        SendStartPurchaseResponse(session, purchase, Battlepay::Error::PurchaseDenied);
        return;
    }

    auto const* product = sBattlePayDataStore->GetProduct(purchase.ProductID);
    if (!product)
    {
        TC_LOG_INFO("server.battlepay", "MakePurchase: product %u not found in store", productID);
        SendStartPurchaseResponse(session, purchase, Battlepay::Error::PurchaseDenied);
        return;
    }

    purchase.CurrentPrice = product->CurrentPriceFixedPoint;

    mgr->RegisterStartPurchase(purchase);

    auto accountBalance = session->GetTokenBalance(group->TokenType);
    auto purchaseData = mgr->GetPurchase();

    TC_LOG_INFO("server.battlepay", "MakePurchase: price=%llu, balance=%lld, tokenType=%u",
        static_cast<unsigned long long>(purchaseData->CurrentPrice),
        static_cast<long long>(accountBalance),
        group->TokenType);

    if (!accountBalance)
    {
        TC_LOG_INFO("server.battlepay", "MakePurchase: no balance");
        SendStartPurchaseResponse(session, *purchaseData, Battlepay::Error::InsufficientBalance);
        return;
    }

    if (accountBalance < static_cast<int64>(purchaseData->CurrentPrice))
    {
        TC_LOG_INFO("server.battlepay", "MakePurchase: insufficient balance");
        SendStartPurchaseResponse(session, *purchaseData, Battlepay::Error::InsufficientBalance);
        return;
    }

    // Player-dependent checks (only when in-game)
    if (player)
    {
        if (!product->Items.empty())
        {
            if (product->Items.size() > GetBagsFreeSlots(player))
            {
                std::ostringstream data;
                data << sObjectMgr->GetTrinityString(Battlepay::String::NotEnoughFreeBagSlots, session->GetSessionDbLocaleIndex());
                player->SendCustomMessage("Store purchase failed ", data);
                SendStartPurchaseResponse(session, *purchaseData, Battlepay::Error::PurchaseDenied);
                return;
            }
        }

        if (!product->ScriptName.empty())
        {
            std::string reason;
            if (!sScriptMgr->BattlePayCanBuy(session, product, reason))
            {
                std::ostringstream data;
                data << reason;
                player->SendCustomMessage("Store purchase failed ", data);
                SendStartPurchaseResponse(session, *purchaseData, Battlepay::Error::PurchaseDenied);
                return;
            }
        }

        for (auto itr : product->Items)
        {
            if (mgr->AlreadyOwnProduct(itr.ItemID))
            {
                std::ostringstream data;
                data << sObjectMgr->GetTrinityString(Battlepay::String::YouAlreadyOwnThat, session->GetSessionDbLocaleIndex());;
                player->SendCustomMessage("Store purchase failed ", data);
                SendStartPurchaseResponse(session, *purchaseData, Battlepay::Error::PurchaseDenied);
                return;
            }
        }
    }
    else
    {
        // On char select, deny products that require items (need bags)
        if (!product->Items.empty())
        {
            TC_LOG_INFO("server.battlepay", "MakePurchase: cannot buy item products from charselect");
            SendStartPurchaseResponse(session, *purchaseData, Battlepay::Error::PurchaseDenied);
            return;
        }
    }

    purchaseData->PurchaseID = mgr->GenerateNewPurchaseID();
    purchaseData->ServerToken = urand(0, 0xFFFFFFF);

    TC_LOG_INFO("server.battlepay", "MakePurchase: OK, sending purchase flow (PurchaseID=%llu)",
        static_cast<unsigned long long>(purchaseData->PurchaseID));

    SendStartPurchaseResponse(session, *purchaseData, Battlepay::Error::Ok);
    SendPurchaseUpdate(session, *purchaseData, Battlepay::Error::Ok);

    WorldPackets::BattlePay::ConfirmPurchase confirmPurchase;
    confirmPurchase.PurchaseID = purchaseData->PurchaseID;
    confirmPurchase.ServerToken = purchaseData->ServerToken;
    session->SendPacket(confirmPurchase.Write());
};

void WorldSession::HandleBattlePayStartPurchase(WorldPackets::BattlePay::StartPurchase& packet)
{
    MakePurchase(packet.TargetCharacter, packet.ClientToken, packet.ProductID, this);
}

void WorldSession::HandleBattlePayPurchaseProduct(WorldPackets::BattlePay::PurchaseProduct& packet)
{
    MakePurchase(packet.TargetCharacter, packet.ClientToken, packet.ProductID, this);
}

void WorldSession::HandleBattlePayConfirmPurchase(WorldPackets::BattlePay::ConfirmPurchaseResponse& packet)
{
    if (!GetBattlePayMgr()->IsAvailable())
        return;

    packet.ClientCurrentPriceFixedPoint /= Battlepay::g_CurrencyPrecision;

    auto purchase = GetBattlePayMgr()->GetPurchase();
    if (!purchase)
        return;

    if (purchase->Lock)
    {
        SendPurchaseUpdate(this, *purchase, Battlepay::Error::PurchaseDenied);
        return;
    }

    if (purchase->ServerToken != packet.ServerToken || !packet.ConfirmPurchase || purchase->CurrentPrice != packet.ClientCurrentPriceFixedPoint)
    {
        TC_LOG_INFO("server.battlepay", "ConfirmPurchase: token/price mismatch (server=%u client=%u, price=%llu clientPrice=%llu, confirm=%u)",
            purchase->ServerToken, packet.ServerToken,
            static_cast<unsigned long long>(purchase->CurrentPrice),
            static_cast<unsigned long long>(packet.ClientCurrentPriceFixedPoint),
            packet.ConfirmPurchase ? 1 : 0);
        SendPurchaseUpdate(this, *purchase, Battlepay::Error::PurchaseDenied);
        return;
    }

    auto player = GetPlayer();

    Battlepay::ProductGroup* group = sBattlePayDataStore->GetProductGroupForProductId(purchase->ProductID);
    if (!group)
    {
        SendPurchaseUpdate(this, *purchase, Battlepay::Error::PurchaseDenied);
        return;
    }

    if (GetTokenBalance(group->TokenType) < static_cast<int64>(purchase->CurrentPrice))
    {
        SendPurchaseUpdate(this, *purchase, Battlepay::Error::PurchaseDenied);
        return;
    }

    auto const* product = sBattlePayDataStore->GetProduct(purchase->ProductID);
    if (!product)
    {
        SendPurchaseUpdate(this, *purchase, Battlepay::Error::PurchaseDenied);
        return;
    }

    purchase->Lock = true;
    purchase->Status = Battlepay::UpdateStatus::Finish;

    if (player)
    {
        if (!product->ScriptName.empty())
        {
            std::string reason;
            if (!sScriptMgr->BattlePayCanBuy(this, product, reason))
            {
                std::ostringstream data;
                data << reason;
                player->SendCustomMessage("Store purchase failed ", data);
                SendPurchaseUpdate(this, *purchase, Battlepay::Error::PaymentFailed);
                return;
            }
        }

        if (!product->Items.empty())
        {
            if (product->Items.size() > GetBagsFreeSlots(player))
            {
                std::ostringstream data;
                data << sObjectMgr->GetTrinityString(Battlepay::String::NotEnoughFreeBagSlots, GetSessionDbLocaleIndex());
                player->SendCustomMessage("Store purchase failed ", data);
                SendStartPurchaseResponse(this, *purchase, Battlepay::Error::PurchaseDenied);
                return;
            }
        }

        for (auto itr : product->Items)
        {
            if (GetBattlePayMgr()->AlreadyOwnProduct(itr.ItemID))
            {
                std::ostringstream data;
                data << sObjectMgr->GetTrinityString(Battlepay::String::YouAlreadyOwnThat, GetSessionDbLocaleIndex());
                player->SendCustomMessage("Store purchase failed ", data);
                SendStartPurchaseResponse(this, *purchase, Battlepay::Error::PurchaseDenied);
                return;
            }
        }
    }
    else
    {
        // On char select, deny products that require items
        if (!product->Items.empty())
        {
            TC_LOG_INFO("server.battlepay", "ConfirmPurchase: cannot buy item products from charselect");
            SendPurchaseUpdate(this, *purchase, Battlepay::Error::PurchaseDenied);
            return;
        }
    }

    TC_LOG_INFO("server.battlepay", "ConfirmPurchase: OK, processing delivery for product %u (player=%s)",
        purchase->ProductID, player ? player->GetName() : "<charselect>");

    SendPurchaseUpdate(this, *purchase, Battlepay::Error::Other);

    bool tokenDeducted = false;
    if (player)
    {
        tokenDeducted = player->ChangeTokenCount(group->TokenType, -static_cast<int64>(purchase->CurrentPrice), Battlepay::BattlepayCustomType::BattlePayShop, purchase->ProductID);
    }
    else
    {
        // Charselect: deduct tokens directly via session + DB
        int64 cost = static_cast<int64>(purchase->CurrentPrice);
        if (GetTokenBalance(group->TokenType) >= cost)
        {
            ChangeTokenBalance(group->TokenType, -cost);
            LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_OR_UPD_TOKEN);
            stmt->setUInt32(0, GetAccountId());
            stmt->setUInt8(1, group->TokenType);
            stmt->setInt64(2, -cost);
            stmt->setInt64(3, -cost);
            LoginDatabase.Execute(stmt);
            tokenDeducted = true;
        }
    }

    if (tokenDeducted)
        GetBattlePayMgr()->ProcessDelivery(purchase);
}

void WorldSession::HandleBattlePayAckFailedResponse(WorldPackets::BattlePay::BattlePayAckFailedResponse& /*packet*/)
{
}

void WorldSession::HandleBattlePayPurchaseDetailsResponse(WorldPackets::BattlePay::BattlePayPurchaseDetailsResponse& packet)
{
    TC_LOG_INFO("server.battlepay", "HandleBattlePayPurchaseDetailsResponse: account %u", GetAccountId());
    WorldPackets::BattlePay::BattlePayPurchaseUnk response;
    response.UnkInt = 0;
    response.Key = "";
    response.UnkByte = packet.UnkByte;
    SendPacket(response.Write());
}

void WorldSession::HandleBattlePayPurchaseUnkResponse(WorldPackets::BattlePay::BattlePayPurchaseUnkResponse& /*packet*/)
{
    TC_LOG_INFO("server.battlepay", "HandleBattlePayPurchaseUnkResponse: account %u, atAuthFlag=0x%X",
        GetAccountId(), GetAF());
    auto purchaseData = GetBattlePayMgr()->GetPurchase();
    SendPurchaseUpdate(this, *purchaseData, Battlepay::Error::Ok);
}

void WorldSession::SendDisplayPromo(int32 promotionID /*= 0*/)
{
    SendPacket(WorldPackets::BattlePay::DisplayPromotion(promotionID).Write());
    // The definitive DistributionListResponse is sent in HandleGetProductList
}

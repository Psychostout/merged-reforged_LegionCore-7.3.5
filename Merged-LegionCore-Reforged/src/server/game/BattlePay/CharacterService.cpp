#include "CharacterService.h"
#include "DatabaseEnv.h"
#include "ObjectMgr.h"
#include "DB2Stores.h"
#include "World.h"
#include "SharedDefines.h"
#include "Item.h"
#include "ItemTemplate.h"
#include "WorldSession.h"

CharacterService* CharacterService::instance()
{
    static CharacterService instance;
    return &instance;
}

void CharacterService::SetRename(Player* player)
{
    player->SetAtLoginFlag(AT_LOGIN_RENAME);

    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
    stmt->setUInt16(0, AT_LOGIN_RENAME);
    stmt->setUInt64(1, player->GetGUID().GetCounter());
    CharacterDatabase.Execute(stmt);
}

void CharacterService::ChangeFaction(Player* player)
{
    player->SetAtLoginFlag(AT_LOGIN_CHANGE_FACTION);

    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
    stmt->setUInt16(0, AT_LOGIN_CHANGE_FACTION);
    stmt->setUInt64(1, player->GetGUID().GetCounter());
    CharacterDatabase.Execute(stmt);
}

void CharacterService::ChangeRace(Player* player)
{
    player->SetAtLoginFlag(AT_LOGIN_CHANGE_RACE);

    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
    stmt->setUInt16(0, AT_LOGIN_CHANGE_RACE);
    stmt->setUInt64(1, player->GetGUID().GetCounter());
    CharacterDatabase.Execute(stmt);
}

void CharacterService::Customize(Player* player)
{
    player->SetAtLoginFlag(AT_LOGIN_CUSTOMIZE);

    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
    stmt->setUInt16(0, AT_LOGIN_CUSTOMIZE);
    stmt->setUInt64(1, player->GetGUID().GetCounter());
    CharacterDatabase.Execute(stmt);
}

void CharacterService::Boost(Player* player)
{
    player->GetSession()->AddAuthFlag(AT_AUTH_FLAG_90_LVL_UP);
}

void CharacterService::RestoreDeletedCharacter(WorldSession* session)
{
    session->AddAuthFlag(AT_AUTH_FLAG_RESTORE_DELETED_CHARACTER);
}

void CharacterService::BoostCharacter(WorldSession* /*session*/, ObjectGuid targetCharGuid, uint8 targetLevel,
    uint16 overrideMapId, uint16 overrideZoneId,
    float overrideX, float overrideY, float overrideZ, float overrideO,
    bool isClassTrial, uint16 specId)
{
    auto charInfo = sWorld->GetCharacterInfo(targetCharGuid);
    if (!charInfo)
        return;

    uint64 charLowGuid = targetCharGuid.GetCounter();
    uint8 charClass = charInfo->Class;
    uint8 charRace = charInfo->Race;
    bool isAlliance = ((1 << (charRace - 1)) & RACEMASK_ALLIANCE) != 0;

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    // a) Set level + XP=0
    auto stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_LEVEL);
    stmt->setUInt8(0, targetLevel);
    stmt->setUInt64(1, charLowGuid);
    trans->Append(stmt);

    // b) Teleport based on level and faction (or use override if provided)
    float x, y, z, o = 0.0f;
    uint16 mapId, zoneId;

    if (overrideMapId != 0)
    {
        mapId = overrideMapId; zoneId = overrideZoneId;
        x = overrideX; y = overrideY; z = overrideZ; o = overrideO;
    }
    else if (targetLevel == 90)
    {
        // Blasted Lands / Dark Portal
        mapId = 0; zoneId = 4709;
        x = -11840.0f; y = -3207.0f; z = -29.0f;
    }
    else if (targetLevel == 100)
    {
        if (isAlliance)
        {
            // Stormwind (placeholder — official boost location TBD)
            mapId = 0; zoneId = 1519;
            x = -8833.0f; y = 628.0f; z = 94.0f;
        }
        else
        {
            // Orgrimmar (placeholder — official boost location TBD)
            mapId = 1; zoneId = 1637;
            x = 1569.0f; y = -4397.0f; z = 16.0f;
        }
    }
    else // 110
    {
        // Dalaran Broken Isles
        mapId = 1220; zoneId = 7502;
        x = -831.0f; y = 4374.0f; z = 738.0f;
    }

    stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_CHARACTER_POSITION);
    stmt->setFloat(0, x);
    stmt->setFloat(1, y);
    stmt->setFloat(2, z);
    stmt->setFloat(3, o);
    stmt->setUInt16(4, mapId);
    stmt->setUInt16(5, zoneId);
    stmt->setUInt64(6, charLowGuid);
    trans->Append(stmt);

    // c) Learn flying spells
    uint32 flyingSpells[] = { 34091, 54197, 90267, 115913 }; // Artisan Riding, Cold Weather, Flight Master, Wisdom of Four Winds
    for (uint32 spellId : flyingSpells)
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_SPELL);
        stmt->setUInt64(0, charLowGuid);
        stmt->setUInt32(1, spellId);
        stmt->setUInt8(2, 1); // active
        stmt->setUInt8(3, 0); // disabled
        trans->Append(stmt);
    }

    if (targetLevel >= 110)
    {
        // Broken Isles Pathfinder
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHAR_SPELL);
        stmt->setUInt64(0, charLowGuid);
        stmt->setUInt32(1, 191645);
        stmt->setUInt8(2, 1);
        stmt->setUInt8(3, 0);
        trans->Append(stmt);
    }

    // d) Equip items from CharacterLoadout DB2
    uint8 loadoutType = targetLevel < 100 ? 3 : 6;
    auto loadoutItems = sDB2Manager.GetLowestIdItemLoadOutItemsBy(charClass, loadoutType);

    bool finger1Used = false, trinket1Used = false;

    // Track equipped items per slot for equipmentCache
    uint32 slotItems[INVENTORY_SLOT_BAG_END] = {};

    for (uint32 itemId : loadoutItems)
    {
        auto itemTemplate = sObjectMgr->GetItemTemplate(itemId);
        if (!itemTemplate)
            continue;

        int8 slot = -1;
        switch (itemTemplate->GetInventoryType())
        {
            case INVTYPE_HEAD:            slot = EQUIPMENT_SLOT_HEAD; break;
            case INVTYPE_NECK:            slot = EQUIPMENT_SLOT_NECK; break;
            case INVTYPE_SHOULDERS:       slot = EQUIPMENT_SLOT_SHOULDERS; break;
            case INVTYPE_BODY:            slot = EQUIPMENT_SLOT_BODY; break;
            case INVTYPE_CHEST:
            case INVTYPE_ROBE:            slot = EQUIPMENT_SLOT_CHEST; break;
            case INVTYPE_WAIST:           slot = EQUIPMENT_SLOT_WAIST; break;
            case INVTYPE_LEGS:            slot = EQUIPMENT_SLOT_LEGS; break;
            case INVTYPE_FEET:            slot = EQUIPMENT_SLOT_FEET; break;
            case INVTYPE_WRISTS:          slot = EQUIPMENT_SLOT_WRISTS; break;
            case INVTYPE_HANDS:           slot = EQUIPMENT_SLOT_HANDS; break;
            case INVTYPE_FINGER:
                slot = finger1Used ? EQUIPMENT_SLOT_FINGER2 : EQUIPMENT_SLOT_FINGER1;
                finger1Used = true;
                break;
            case INVTYPE_TRINKET:
                slot = trinket1Used ? EQUIPMENT_SLOT_TRINKET2 : EQUIPMENT_SLOT_TRINKET1;
                trinket1Used = true;
                break;
            case INVTYPE_CLOAK:           slot = EQUIPMENT_SLOT_BACK; break;
            case INVTYPE_WEAPON:
            case INVTYPE_2HWEAPON:
            case INVTYPE_WEAPONMAINHAND:  slot = EQUIPMENT_SLOT_MAINHAND; break;
            case INVTYPE_SHIELD:
            case INVTYPE_WEAPONOFFHAND:
            case INVTYPE_HOLDABLE:        slot = EQUIPMENT_SLOT_OFFHAND; break;
            case INVTYPE_RANGED:
            case INVTYPE_THROWN:
            case INVTYPE_RANGEDRIGHT:     slot = EQUIPMENT_SLOT_RANGED; break;
            case INVTYPE_TABARD:          slot = EQUIPMENT_SLOT_TABARD; break;
            default: continue;
        }

        if (slot < 0)
            continue;

        slotItems[slot] = itemId;

        // Delete old item from this slot
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHAR_INVENTORY_BY_BAG_SLOT);
        stmt->setUInt64(0, 0); // bag = 0 (equipped)
        stmt->setUInt8(1, static_cast<uint8>(slot));
        stmt->setUInt64(2, charLowGuid);
        trans->Append(stmt);

        // Create new item
        Item* pItem = Item::CreateItem(itemId, 1, nullptr);
        if (!pItem)
            continue;

        pItem->SetOwnerGUID(targetCharGuid);
        pItem->SaveToDB(trans);

        // Insert into character inventory
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_INVENTORY_ITEM);
        stmt->setUInt64(0, charLowGuid);       // character guid
        stmt->setUInt64(1, 0);                  // bag = 0 (equipped)
        stmt->setUInt8(2, static_cast<uint8>(slot));
        stmt->setUInt64(3, pItem->GetGUIDLow());
        trans->Append(stmt);

        delete pItem;
    }

    // e) Build and update equipmentCache for charselect display
    std::ostringstream eqCache;
    for (uint32 i = 0; i < INVENTORY_SLOT_BAG_END; ++i)
    {
        if (slotItems[i])
        {
            auto tmpl = sObjectMgr->GetItemTemplate(slotItems[i]);
            uint32 invType = tmpl ? tmpl->GetInventoryType() : 0;
            uint32 displayId = sDB2Manager.GetItemDisplayId(slotItems[i], 0);
            eqCache << invType << ' ' << displayId << " 0 ";
        }
        else
            eqCache << "0 0 0 ";
    }
    trans->PAppend("UPDATE characters SET equipmentCache='%s' WHERE guid=" UI64FMTD, eqCache.str().c_str(), charLowGuid);

    if (isClassTrial)
    {
        // Class trial: set flags + specialization + skip cinematic (all in same transaction)
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_ADD_AT_LOGIN_FLAG);
        stmt->setUInt16(0, AT_LOGIN_CLASS_TRIAL);
        stmt->setUInt64(1, charLowGuid);
        trans->Append(stmt);

        if (specId)
        {
            stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_SPECIALIZATION);
            stmt->setUInt16(0, specId);
            stmt->setUInt64(1, charLowGuid);
            trans->Append(stmt);
        }

        trans->PAppend("UPDATE characters SET cinematic = 1 WHERE guid = " UI64FMTD, charLowGuid);
    }
    else
    {
        // Regular boost: remove class trial flags if present (unlocks the character permanently)
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_UPD_REM_AT_LOGIN_FLAG);
        stmt->setUInt16(0, AT_LOGIN_CLASS_TRIAL | AT_LOGIN_CLASS_TRIAL_LOCKED);
        stmt->setUInt64(1, charLowGuid);
        trans->Append(stmt);
    }

    CharacterDatabase.CommitTransaction(trans);

    // e) Update in-memory cache
    sWorld->UpdateCharacterInfoLevel(targetCharGuid, targetLevel);

    TC_LOG_INFO("server.battlepay", "BoostCharacter: character '%s' (guid: %u, race: %u, class: %u) boosted to level %u",
        charInfo->Name.c_str(), uint32(charLowGuid), charRace, charClass, targetLevel);
}


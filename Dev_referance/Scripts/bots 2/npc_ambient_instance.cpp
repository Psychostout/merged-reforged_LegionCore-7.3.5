/*
 * npc_ambient_instance.cpp
 * Dungeon/raid entry, auto-fill, replica management
 *
 * CRITICAL FIX: Tier-2a (roster) and Tier-2b (auto-fill) NPCs now use
 * HireCompanion() so they get full combat AI (healing, tanking, interrupts,
 * target switching) instead of just SetReactState + MoveFollow.
 *
 * Instance stat scaling: uses ScaleNpcStatsForInstance() which applies
 * 2x HP/1.5x damage for dungeons, 3x HP/2x damage for raids.
 */

#include "npc_ambient_world.h"

// Forward-declare the AI struct so we can CAST_AI
struct npc_ambient_aiAI;

// ============================================================
//  Flush replica progress to DB on instance exit
// ============================================================
void FlushAndCleanReplicas(Player* player)
{
    std::vector<std::tuple<std::string, uint32, uint32, uint32>> toSave;
    {
        std::lock_guard<std::mutex> lk(s_companionMutex);
        for (auto it = s_companions.begin(); it != s_companions.end(); )
        {
            if (it->second.ownerGuid == player->GetGUID() && it->second.isReplica)
            {
                toSave.emplace_back(it->second.displayName, it->second.currentLevel,
                    it->second.xp, it->second.killCount);
                it = s_companions.erase(it);
            }
            else ++it;
        }
    }
    for (size_t ti = 0; ti < toSave.size(); ++ti)
        DB_UpdateCompanionProgress(player->GetGUID().GetCounter(),
            std::get<0>(toSave[ti]), std::get<1>(toSave[ti]),
            std::get<2>(toSave[ti]), std::get<3>(toSave[ti]));
}

// ============================================================
//  Restore persisted raid roster from DB into memory
// ============================================================
void RestoreRosterForPlayer(Player* player)
{
    QueryResult result = CharacterDatabase.PQuery(
        "SELECT name, role, level FROM character_raid_roster WHERE player_guid=%u",
        player->GetGUID().GetCounter());
    if (!result) return;
    std::vector<RosterEntry> entries;
    do {
        Field* f = result->Fetch();
        RosterEntry re;
        re.displayName = f[0].GetString();
        re.role        = (AmbientRole)f[1].GetUInt8();
        re.level       = f[2].GetUInt8();
        re.isManual    = true;
        entries.push_back(re);
    } while (result->NextRow());
    {
        std::lock_guard<std::mutex> lk(s_rosterMutex);
        s_raidRoster[player->GetGUID()] = std::move(entries);
    }
}

// ============================================================
//  Instance entry: Tier-1 replicas + Tier-2 roster + auto-fill
//
//  KEY FIX: All NPCs now go through HireCompanion() so they get:
//   - Full AI state machine (healing, tanking, interrupts, target switching)
//   - Proper formation follow
//   - Group membership
//   - Kill/XP tracking
//
//  Instance scaling: ScaleNpcStatsForInstance() gives companions
//  enough HP and damage to be meaningful against instance-tuned mobs.
// ============================================================
void HandleInstanceEntry(Player* player, Map* map)
{
    uint32 playerGuid = player->GetGUID().GetCounter();
    uint8  plvl       = player->getLevel();
    bool   isRaid     = map->IsRaid();
    InstanceSize inst = GetInstanceSize(map);
    uint32 npcSlots   = inst.total > 0 ? inst.total - 1 : 4;
    uint32 extraTanks = GetExtraTanks(player->GetGUID());

    // ── Tier 1: Re-summon companion replicas inside the instance ─────────────
    QueryResult compResult = CharacterDatabase.PQuery(
        "SELECT entry, level, xp, name, display_id, kill_count "
        "FROM character_companion WHERE player_guid=%u", playerGuid);

    uint32 compTanks = 0, compHealers = 0, compDps = 0;
    if (compResult)
    {
        do {
            if (compTanks + compHealers + compDps >= npcSlots) break;
            Field*      f     = compResult->Fetch();
            uint32      entry = f[0].GetUInt32();
            uint32      lvl   = f[1].GetUInt32();
            uint32      xp    = f[2].GetUInt32();
            std::string name  = f[3].GetString();
            uint32      disp  = f[4].GetUInt32();
            uint32      kc    = f[5].GetUInt32();

            // T4: Validate entry
            if (!IsValidAmbientEntry(entry)) continue;
            if (!sObjectMgr->GetCreatureTemplate(entry)) continue;

            Position pos = RandomPositionNear(player);
            TempSummon* s = player->SummonCreature(entry, pos,
                TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, INSTANCE_DESPAWN_MS);
            if (!s) continue;

            uint8 slvl = (uint8)std::min(lvl, 110u);
            s->SetLevel(slvl);
            // Instance-boosted stats instead of base stats
            ScaleNpcStatsForInstance(s, slvl, isRaid);
            s->SetName(name);
            if (disp) s->SetDisplayId(disp);

            // Mount instance companion replicas at appropriate levels
            uint32 mountId = GetMountDisplayId(entry, slvl);
            if (mountId) s->Mount(mountId);

            if (!AmbientHireCompanion(s, player, slvl, xp, true)) continue;
            if (kc > 0)
            {
                std::lock_guard<std::mutex> lk(s_companionMutex);
                auto it = s_companions.find(s->GetGUID());
                if (it != s_companions.end())
                    it->second.killCount = kc;
            }
            AmbientRole r = AmbientGetRole(s);
            if      (r == AMBIENT_TANK)   ++compTanks;
            else if (r == AMBIENT_HEALER) ++compHealers;
            else                          ++compDps;
        } while (compResult->NextRow());
    }

    // ── Tier 2a: Summon manually rostered NPCs ───────────────────────────────
    // FIX: These now use HireCompanion() for full AI instead of bare SetReactState/MoveFollow
    std::vector<RosterEntry> rosterCopy;
    {
        std::lock_guard<std::mutex> lk(s_rosterMutex);
        auto it = s_raidRoster.find(player->GetGUID());
        if (it != s_raidRoster.end()) rosterCopy = it->second;
    }
    uint32 rT = 0, rH = 0, rD = 0;
    uint32 usedSlots = compTanks + compHealers + compDps;
    for (auto const& re : rosterCopy)
    {
        if (usedSlots + rT + rH + rD >= npcSlots) break;
        uint32 entry = EntryForRole(re.role);
        if (!sObjectMgr->GetCreatureTemplate(entry)) continue;
        Position pos = RandomPositionNear(player);
        TempSummon* s = player->SummonCreature(entry, pos,
            TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, INSTANCE_DESPAWN_MS);
        if (!s) continue;
        uint8 rlvl = re.level > 0 ? re.level : plvl;
        s->SetLevel(rlvl);
        ScaleNpcStatsForInstance(s, rlvl, isRaid);
        s->SetName(re.displayName);

        // Mount roster NPCs at appropriate levels
        uint32 mountId = GetMountDisplayId(entry, rlvl);
        if (mountId) s->Mount(mountId);

        // CRITICAL FIX: Use HireCompanion for full AI (was previously bare SetReactState/MoveFollow)
        if (!AmbientHireCompanion(s, player, rlvl, 0, true))
        {
            // Fallback if AI cast fails — at least make them follow
            s->SetReactState(REACT_DEFENSIVE);
            s->GetMotionMaster()->MoveFollow(player, 3.5f + float(rT+rH+rD) * 0.4f, float(M_PI));
        }

        if      (re.role == AMBIENT_TANK)   ++rT;
        else if (re.role == AMBIENT_HEALER) ++rH;
        else                                ++rD;
    }

    // ── Tier 2b: Auto-fill remaining slots with correct role ratios ──────────
    // CRITICAL FIX: Auto-fill NPCs now use HireCompanion for full AI
    uint32 haveTanks   = compTanks   + rT;
    uint32 haveHealers = compHealers + rH;
    uint32 haveDps     = compDps     + rD;
    uint32 haveTotal   = haveTanks + haveHealers + haveDps;
    uint32 fillSlots   = npcSlots > haveTotal ? npcSlots - haveTotal : 0u;
    uint32 needTanks   = (inst.minTanks + extraTanks > haveTanks)
                         ? (inst.minTanks + extraTanks - haveTanks) : 0u;
    uint32 needHealers = (inst.minHealers > haveHealers) ? (inst.minHealers - haveHealers) : 0u;
    uint32 needDps     = (fillSlots > needTanks + needHealers)
                         ? (fillSlots - needTanks - needHealers) : 0u;

    // Raid-tuned DPS rotation: 2:1 ranged-to-melee ratio for better positioning
    static const AmbientRole RAID_DPS_ROLES[] = {
        AMBIENT_MAGE, AMBIENT_HUNTER, AMBIENT_WARRIOR,
        AMBIENT_MAGE, AMBIENT_ROGUE,  AMBIENT_HUNTER,
        AMBIENT_MAGE, AMBIENT_WARRIOR, AMBIENT_HUNTER, AMBIENT_ROGUE
    };
    static const AmbientRole DUNGEON_DPS_ROLES[] = {
        AMBIENT_WARRIOR, AMBIENT_MAGE, AMBIENT_HUNTER, AMBIENT_ROGUE, AMBIENT_DEFAULT
    };
    uint32 autoFilled = 0, dpsIdx = 0;

    auto spawnFill = [&](AmbientRole role)
    {
        uint32 entry = EntryForRole(role);
        if (!sObjectMgr->GetCreatureTemplate(entry)) return;
        Position pos = RandomPositionNear(player);
        TempSummon* s = player->SummonCreature(entry, pos,
            TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, INSTANCE_DESPAWN_MS);
        if (!s) return;
        s->SetLevel(plvl);
        ScaleNpcStatsForInstance(s, plvl, isRaid);
        s->SetName(AmbientNames::Roll(entry));

        // Mount auto-fill NPCs at appropriate levels
        uint32 mountId = GetMountDisplayId(entry, plvl);
        if (mountId) s->Mount(mountId);

        // CRITICAL FIX: Use HireCompanion for full combat AI
        if (!AmbientHireCompanion(s, player, plvl, 0, true))
        {
            s->SetReactState(REACT_DEFENSIVE);
            s->GetMotionMaster()->MoveFollow(player,
                3.5f + float(autoFilled) * 0.5f,
                float(M_PI) + float(autoFilled) * 0.42f);
        }
        ++autoFilled;
    };

    for (uint32 i = 0; i < needTanks;   ++i) spawnFill(AMBIENT_TANK);
    for (uint32 i = 0; i < needHealers; ++i) spawnFill(AMBIENT_HEALER);
    for (uint32 i = 0; i < needDps;     ++i)
    {
        if (isRaid)
            spawnFill(RAID_DPS_ROLES[dpsIdx++ % 10]);
        else
            spawnFill(DUNGEON_DPS_ROLES[dpsIdx++ % 5]);
    }

    // ── Summary message ────────────────────────────────────────────────────
    uint32 finalTanks   = haveTanks   + needTanks;
    uint32 finalHealers = haveHealers + needHealers;
    uint32 finalDps     = haveDps     + needDps;
    ChatHandler(player->GetSession()).PSendSysMessage(
        "|cff00ffff[%s]|r Entering with %uT %uH %u DPS (%u/%u) \xe2\x80\x94 %u auto-filled",
        isRaid ? "Raid" : "Dungeon",
        finalTanks, finalHealers, finalDps,
        1 + haveTotal + autoFilled, inst.total, autoFilled);
    if (finalTanks < inst.minTanks + extraTanks)
        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cffff8800[%s]|r Warning: insufficient tanks for this content!",
            isRaid ? "Raid" : "Dungeon");
    if (finalHealers < inst.minHealers)
        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cffff8800[%s]|r Warning: insufficient healers for this content!",
            isRaid ? "Raid" : "Dungeon");
}

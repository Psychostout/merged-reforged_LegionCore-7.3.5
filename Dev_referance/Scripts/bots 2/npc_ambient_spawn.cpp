/*
 * npc_ambient_spawn.cpp
 * PlayerScript: spawning, login restore, zone transitions, chat commands, OnCreatureKill
 */

#include "npc_ambient_world.h"

// Forward declarations for instance functions (npc_ambient_instance.cpp)
void FlushAndCleanReplicas(Player* player);
void RestoreRosterForPlayer(Player* player);
void HandleInstanceEntry(Player* player, Map* map);

// AI wrapper functions declared in npc_ambient_world.h — no CAST_AI needed

// ============================================================
//  Spawn helpers
// ============================================================
uint32 PickAmbientEntry(Player* player)
{
    // 100% faction-based — Alliance sees Alliance, Horde sees Horde
    if (player->GetTeam() == ALLIANCE)
        return ALLIANCE_POOL[urand(0, 4)];
    if (player->GetTeam() == HORDE)
        return HORDE_POOL[urand(0, 4)];
    // Fallback (should not happen)
    return ALLIANCE_POOL[urand(0, 4)];
}

uint32 CountNearbyAmbient(Player* player)
{
    uint32 count = 0;
    ObjectGuid playerGuid = player->GetGUID();
    for (uint32 e = AMBIENT_ENTRY_MIN; e <= AMBIENT_ENTRY_MAX; ++e)
    {
        std::list<Creature*> nl;
        GetCreatureListWithEntryInGrid(nl, player, e, SEARCH_RADIUS);
        for (Creature* c : nl)
            if (c && c->IsSummon() && c->ToTempSummon()->GetSummonerGUID() == playerGuid)
                ++count;
    }
    return count;
}

Position RandomPositionNear(Player* player)
{
    Map* m = player->GetMap();
    float px = player->GetPositionX();
    float py = player->GetPositionY();
    float pz = player->GetPositionZ();

    // Try up to 5 positions, pick the first valid one
    for (uint32 attempt = 0; attempt < 5; ++attempt)
    {
        float angle = frand(0.f, float(M_PI) * 2.f);
        float dist  = frand(12.f, SPREAD_RADIUS);
        float x = px + std::cos(angle) * dist;
        float y = py + std::sin(angle) * dist;
        float z = pz;

        if (m)
        {
            float h = m->GetHeight(player->GetPhaseShift(), x, y, z + 5.f, true, 50.f);
            // Skip if no valid ground found
            if (h <= INVALID_HEIGHT + 1.f)
                continue;
            z = h;

            // Reject positions with too steep a height difference from player (cliffs, walls)
            float dz = std::abs(z - pz);
            if (dz > 12.0f)
                continue;

            // Skip positions in deep water (liquid level well above ground)
            float liquidLevel = m->GetWaterLevel(player->GetPhaseShift(), x, y);
            if (liquidLevel > INVALID_HEIGHT && liquidLevel > z + 2.0f)
                continue;
        }

        return { x, y, z, frand(0.f, float(M_PI) * 2.f) };
    }

    // Fallback: spawn close to player if all attempts failed
    float fAngle = frand(0.f, float(M_PI) * 2.f);
    float fx = px + std::cos(fAngle) * 5.f;
    float fy = py + std::sin(fAngle) * 5.f;
    return { fx, fy, pz, frand(0.f, float(M_PI) * 2.f) };
}

// ============================================================
//  PlayerScript
// ============================================================
class AmbientWorldPlayerScript : public PlayerScript
{
public:
    AmbientWorldPlayerScript() : PlayerScript("AmbientWorldPlayerScript") { }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        if (!player) return;
        {
            std::lock_guard<std::mutex> lk(s_mutex);
            s_nextSpawnTime.erase(player->GetGUID());
        }
        if (!player->GetMap()->Instanceable())
            RestoreCompanionForPlayer(player);
        RestoreRosterForPlayer(player);
    }

    void OnMapChanged(Player* player) override
    {
        if (!player) return;
        Map* map = player->GetMap();
        if (!map) return;

        if (map->Instanceable())
        {
            HandleInstanceEntry(player, map);
        }
        else
        {
            FlushAndCleanReplicas(player);
            RestoreCompanionForPlayer(player);
        }
    }

    void OnUpdateZone(Player* player, uint32 newZone, uint32 /*newArea*/, uint32 oldZone) override
    {
        if (!player || !player->IsInWorld() || !player->IsAlive()) return;
        if (player->getLevel() == 0) return;

        // C1: Despawn non-companion NPCs from previous zone
        if (oldZone && oldZone != newZone)
        {
            for (uint32 amb = AMBIENT_ENTRY_MIN; amb <= AMBIENT_ENTRY_MAX; ++amb)
            {
                std::list<Creature*> clist;
                GetCreatureListWithEntryInGrid(clist, player, amb, 300.f);
                for (Creature* c : clist)
                {
                    if (!c || !c->IsAlive()) continue;
                    if (!c->IsSummon() || c->ToTempSummon()->GetSummonerGUID() != player->GetGUID()) continue;
                    if (c->GetZoneId() != oldZone) continue;
                    if (IsAmbientCompanionCreature(c)) continue;
                    c->DespawnOrUnsummon(5000);
                }
            }
        }

        TrySpawnAmbient(player, newZone);
    }

    // I1: Companion chat commands
    void OnChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg) override
    {
        if (type != CHAT_MSG_SAY || msg.empty() || msg[0] != '.') return;

        std::string cmd = msg;
        for (std::string::size_type i = 0; i < cmd.size(); ++i)
            cmd[i] = (char)tolower((unsigned char)cmd[i]);

        bool doStay = false, doFollow = false, doPassive = false,
             doAttack = false, doDismiss = false, doStatus = false;

        if      (cmd == ".stay")    doStay    = true;
        else if (cmd == ".follow")  doFollow  = true;
        else if (cmd == ".passive") doPassive = true;
        else if (cmd == ".attack")  doAttack  = true;
        else if (cmd == ".dismiss") doDismiss = true;
        else if (cmd == ".cs")      doStatus  = true;
        else return;

        msg = "";

        std::vector<Creature*> comp;
        for (uint32 e = AMBIENT_ENTRY_MIN; e <= AMBIENT_ENTRY_MAX; ++e)
        {
            std::list<Creature*> cl;
            GetCreatureListWithEntryInGrid(cl, player, e, 100.f);
            for (Creature* c : cl)
            {
                if (!c || !c->IsAlive()) continue;
                if (IsAmbientCompanionOf(c, player->GetGUID()))
                    comp.push_back(c);
            }
        }

        ChatHandler ch(player->GetSession());

        if (doStay)
        {
            for (Creature* c : comp)
            {
                c->GetMotionMaster()->Clear();
                c->GetMotionMaster()->MoveIdle();
            }
            ch.PSendSysMessage("|cff00ccff[Companions]|r All companions: Stay.");
        }
        else if (doFollow)
        {
            for (Creature* c : comp)
            {
                c->GetMotionMaster()->Clear();
                c->GetMotionMaster()->MoveFollow(player, 3.0f, float(M_PI));
            }
            ch.PSendSysMessage("|cff00ccff[Companions]|r All companions: Follow.");
        }
        else if (doPassive)
        {
            for (Creature* c : comp)
            {
                c->SetReactState(REACT_PASSIVE);
                if (c->IsInCombat()) c->CombatStop(true);
            }
            ch.PSendSysMessage("|cff00ccff[Companions]|r All companions: Passive (will not attack).");
        }
        else if (doAttack)
        {
            for (Creature* c : comp)
                c->SetReactState(REACT_DEFENSIVE);
            ch.PSendSysMessage("|cff00ccff[Companions]|r All companions: Aggressive.");
        }
        else if (doDismiss)
        {
            for (Creature* c : comp)
                AmbientDismissCompanion(c);
            ch.PSendSysMessage("|cff00ccff[Companions]|r All companions dismissed.");
        }
        else if (doStatus)
        {
            if (comp.empty())
            {
                ch.PSendSysMessage("|cff00ccff[Companions]|r No companions active.");
                return;
            }
            ch.PSendSysMessage("|cff00ccff[Companions] ---------- Party ----------");
            for (uint32 i = 0; i < (uint32)comp.size(); ++i)
            {
                Creature*    c  = comp[i];
                CompanionData cd;
                if (!GetCompanionData(c->GetGUID(), cd)) continue;
                ch.PSendSysMessage("  #%u %-16s (%-7s Lv%-3u) HP:%3u%%  XP:%-6u  Kills:%u",
                    i + 1, cd.displayName.c_str(),
                    RoleName(GetRoleForEntry(c->GetEntry())),
                    (uint32)c->getLevel(), (uint32)c->GetHealthPct(),
                    cd.xp, cd.killCount);
            }
        }
    }

    void OnLogout(Player* player) override
    {
        if (!player) return;
        {
            std::lock_guard<std::mutex> lk(s_mutex);
            s_nextSpawnTime.erase(player->GetGUID());
        }
        SaveCompanionOnLogout(player);
    }

    void OnCreatureKill(Player* killer, Creature* killed) override
    {
        if (!killer || !killed)
            return;

        uint32 xpGain = CompanionXpGain((uint32)killed->getLevel());

        for (uint32 entry = AMBIENT_ENTRY_MIN; entry <= AMBIENT_ENTRY_MAX; ++entry)
        {
            std::list<Creature*> cList;
            GetCreatureListWithEntryInGrid(cList, killer, entry, 50.f);
            for (Creature* companion : cList)
            {
                if (!companion || !companion->IsAlive())
                    continue;
                CompanionData cd;
                if (!GetCompanionDataIfOwned(companion->GetGUID(), killer->GetGUID(), cd))
                    continue;

                uint32 newLevel = 0;
                bool   leveledUp = AwardCompanionXP(companion->GetGUID(), xpGain, newLevel);

                std::string cname = (!cd.displayName.empty()) ? cd.displayName
                                                              : companion->GetName();
                ChatHandler(killer->GetSession()).PSendSysMessage(
                    "|cff00ccff[Party]|r %s gained %u XP  (Lv %u)",
                    cname.c_str(), xpGain, newLevel);

                if (leveledUp)
                {
                    companion->SetLevel(newLevel);
                    ChatHandler(killer->GetSession()).PSendSysMessage(
                        "|cffffd700[Companion]|r %s reached level %u!",
                        cname.c_str(), newLevel);
                }
            }
        }
    }

private:
    static std::mutex s_mutex;
    static std::map<ObjectGuid, std::chrono::steady_clock::time_point> s_nextSpawnTime;

    void RestoreCompanionForPlayer(Player* player)
    {
        QueryResult result = CharacterDatabase.PQuery(
            "SELECT entry, level, xp, name, display_id, kill_count "
            "FROM character_companion WHERE player_guid = %u",
            player->GetGUID().GetCounter());
        if (!result)
            return;

        uint32 count = 0;
        do
        {
            Field*      f         = result->Fetch();
            uint32      entry     = f[0].GetUInt32();
            uint32      lvl       = f[1].GetUInt32();
            uint32      xp        = f[2].GetUInt32();
            std::string name      = f[3].GetString();
            uint32      dispId    = f[4].GetUInt32();
            uint32      killCount = f[5].GetUInt32();

            // T4: Validate entry
            if (!IsValidAmbientEntry(entry))
            {
                TC_LOG_WARN("scripts", "AmbientNPC: invalid companion entry %u for player %s — skipping",
                    entry, player->GetName().c_str());
                continue;
            }

            if (!sObjectMgr->GetCreatureTemplate(entry))
                continue;

            // Dedup: skip if a companion with this name is already active for this player
            bool alreadyActive = false;
            for (uint32 e2 = AMBIENT_ENTRY_MIN; e2 <= AMBIENT_ENTRY_MAX && !alreadyActive; ++e2)
            {
                std::list<Creature*> existing;
                GetCreatureListWithEntryInGrid(existing, player, e2, 150.f);
                for (Creature* ec : existing)
                {
                    if (!ec->IsSummon()) continue;
                    if (ec->ToTempSummon()->GetSummonerGUID() != player->GetGUID()) continue;
                    if (!IsAmbientCompanionCreature(ec)) continue;
                    if (ec->GetName() == name) { alreadyActive = true; break; }
                }
            }
            if (alreadyActive)
            {
                TC_LOG_DEBUG("scripts", "AmbientNPC: skipping duplicate companion '%s' for %s",
                    name.c_str(), player->GetName().c_str());
                continue;
            }

            Position pos = RandomPositionNear(player);
            TempSummon* s = player->SummonCreature(entry, pos,
                TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, DESPAWN_TIME_MS);
            if (!s) continue;

            uint8 slvl = (uint8)std::min(lvl, 110u);
            s->SetLevel(slvl);
            ScaleNpcStats(s, slvl);
            s->SetName(name);
            if (dispId)
                s->SetDisplayId(dispId);

            uint32 mountId = GetMountDisplayId(entry, slvl);
            if (mountId) s->Mount(mountId);

            if (!AmbientHireCompanion(s, player, slvl, xp))
                continue;

            if (killCount > 0)
            {
                std::lock_guard<std::mutex> lk(s_companionMutex);
                auto it = s_companions.find(s->GetGUID());
                if (it != s_companions.end())
                    it->second.killCount = killCount;
            }
            ++count;

            ChatHandler(player->GetSession()).PSendSysMessage(
                "|cff00ff00[Companion]|r %s is waiting nearby.", name.c_str());
        }
        while (result->NextRow());

        TC_LOG_DEBUG("scripts", "AmbientNPC: restored %u companion(s) for %s",
            count, player->GetName().c_str());
    }

    void SaveCompanionOnLogout(Player* player)
    {
        for (uint32 entry = AMBIENT_ENTRY_MIN; entry <= AMBIENT_ENTRY_MAX; ++entry)
        {
            std::list<Creature*> cList;
            GetCreatureListWithEntryInGrid(cList, player, entry, 100.f);
            for (Creature* companion : cList)
            {
                if (!IsCompanion(companion->GetGUID()))
                    continue;
                CompanionData cd;
                if (!GetCompanionData(companion->GetGUID(), cd))
                    continue;
                if (cd.ownerGuid != player->GetGUID())
                    continue;

                std::string savedName = (!cd.displayName.empty()) ? cd.displayName
                                                                  : companion->GetName();
                DB_UpdateCompanionProgress(player->GetGUID().GetCounter(),
                    savedName, cd.currentLevel, cd.xp, cd.killCount);
            }
        }
    }

    void TrySpawnAmbient(Player* player, uint32 zoneId)
    {
        Map* map = player->GetMap();
        if (!map) return;
        if (map->Instanceable()) return;
        if (player->InBattleground()) return;
        if (SKIP_ZONES.count(zoneId)) return;

        {
            std::lock_guard<std::mutex> lk(s_mutex);
            auto now = std::chrono::steady_clock::now();
            auto it  = s_nextSpawnTime.find(player->GetGUID());
            if (it != s_nextSpawnTime.end() && now < it->second)
                return;
            s_nextSpawnTime[player->GetGUID()] = now + std::chrono::milliseconds(SPAWN_THROTTLE_MS);
        }

        uint32 existing = CountNearbyAmbient(player);
        TC_LOG_DEBUG("scripts", "AmbientNPC: [%s] zone %u map %u existing=%u",
            player->GetName().c_str(), zoneId, player->GetMapId(), existing);

        bool   isStarting   = STARTING_ZONES.count(zoneId) > 0;
        bool   isCapital    = CAPITAL_ZONES.count(zoneId) > 0;
        uint32 minThreshold = isCapital  ? MIN_CAPITAL_NPCS
                            : isStarting ? MIN_STARTING_NPCS
                            :              MIN_AMBIENT_NPCS;

        if (existing >= minThreshold)
            return;

        uint32 spawnMin = isCapital  ? CAPITAL_SPAWN_MIN
                        : isStarting ? STARTING_SPAWN_MIN
                        :              SPAWN_COUNT_MIN;

        // Scale ambient cap based on nearby players — fewer NPCs needed when more players around
        uint32 baseCap;
        {
            uint32 nearPlayers = map->GetPlayersCountExceptGMs();
            if (isCapital)
            {
                // Capitals always stay busy
                if      (nearPlayers <= 1) baseCap = CAPITAL_SPAWN_MAX;       // Solo: full 40
                else if (nearPlayers <= 5) baseCap = CAPITAL_SPAWN_MAX - 5;   // Small: 35
                else                       baseCap = CAPITAL_SPAWN_MIN;       // Populated: 25
            }
            else
            {
                if      (nearPlayers <= 1) baseCap = SPAWN_COUNT_MAX;       // Solo: full 20
                else if (nearPlayers <= 3) baseCap = SPAWN_COUNT_MAX - 4;   // Small group: 16
                else if (nearPlayers <= 6) baseCap = SPAWN_COUNT_MAX - 8;   // Medium: 12
                else                       baseCap = spawnMin;              // Populated: min 10
            }
        }
        uint32 spawnMax = isCapital  ? std::max(spawnMin, baseCap)
                        : isStarting ? STARTING_SPAWN_MAX
                        :              std::max(spawnMin, baseCap);
        uint32 needed   = urand(spawnMin, spawnMax);

        bool   groupActive      = false;
        uint32 groupMembersLeft = 0;
        float  groupX = 0.f, groupY = 0.f, groupZ = 0.f;

        uint32 spawned = 0;

        bool isNightSpawn = false;
        {
            time_t nt     = time(nullptr);
            int    secs   = int(nt % 86400);
            isNightSpawn  = (secs >= 72000 || secs < 21600);
        }

        for (uint32 i = 0; i < needed; ++i)
        {
            uint32 entry = PickAmbientEntry(player);

            if (!isNightSpawn && (entry == 9500084 || entry == 9500089))
                entry = (entry == 9500084) ? 9500080 : 9500085;

            if (!sObjectMgr->GetCreatureTemplate(entry))
            {
                TC_LOG_ERROR("scripts", "AmbientNPC: missing template for entry %u", entry);
                continue;
            }

            Position pos;
            if (!groupActive && urand(0, 3) == 0)
            {
                pos          = RandomPositionNear(player);
                groupX       = pos.GetPositionX();
                groupY       = pos.GetPositionY();
                groupZ       = pos.GetPositionZ();
                groupActive      = true;
                groupMembersLeft = urand(1, 2);
            }
            else if (groupActive && groupMembersLeft > 0)
            {
                float ox = groupX + frand(-6.f, 6.f);
                float oy = groupY + frand(-6.f, 6.f);
                float oz = groupZ;
                if (Map* m = player->GetMap())
                {
                    float h = m->GetHeight(player->GetPhaseShift(), ox, oy, oz + 5.f, true, 50.f);
                    if (h > INVALID_HEIGHT + 1.f) oz = h;
                }
                pos.Relocate(ox, oy, oz, frand(0.f, float(M_PI) * 2.f));
                --groupMembersLeft;
                if (groupMembersLeft == 0) groupActive = false;
            }
            else
            {
                groupActive = false;
                pos = RandomPositionNear(player);
            }

            TempSummon* s = player->SummonCreature(entry, pos, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, DESPAWN_TIME_MS);
            if (!s) continue;

            uint8 plvl  = player->getLevel();
            int32 offset = (int32)urand(0, 10) - 5;
            uint8 lvl   = (uint8)std::max(1, std::min(110, (int32)plvl + offset));
            s->SetLevel(lvl);
            ScaleNpcStats(s, lvl);

            s->SetName(AmbientNames::Roll(entry));

            // Zone-based race appearance
            {
                auto raceIt = ZONE_RACE_MAP.find(zoneId);
                if (raceIt != ZONE_RACE_MAP.end())
                {
                    const ZoneRacePool& pool = raceIt->second;
                    bool usePrimary = !pool.primary.empty() && (urand(0, 9) < 7);
                    bool isAllianceEntry = (entry >= 9500080 && entry <= 9500084);
                    bool isHordeEntry    = (entry >= 9500085 && entry <= 9500089);
                    if (!isAllianceEntry && !isHordeEntry)
                    {
                        if (player->GetTeam() == HORDE)    isHordeEntry    = true;
                        else                                isAllianceEntry = true;
                    }
                    auto FilterPool = [&](const std::vector<uint32>& src) -> std::vector<uint32>
                    {
                        std::vector<uint32> out;
                        for (uint32 id : src)
                        {
                            if (isAllianceEntry && HORDE_ONLY_DISPLAY.count(id))    continue;
                            if (isHordeEntry    && ALLIANCE_ONLY_DISPLAY.count(id)) continue;
                            out.push_back(id);
                        }
                        return out;
                    };
                    std::vector<uint32> filtered = FilterPool(usePrimary ? pool.primary : pool.secondary);
                    if (filtered.empty())
                        filtered = FilterPool(usePrimary ? pool.secondary : pool.primary);
                    if (!filtered.empty())
                        s->SetDisplayId(filtered[urand(0, uint32(filtered.size()) - 1u)]);
                }
            }

            // Mount riders
            uint32 mountId = GetMountDisplayId(entry, lvl);
            if (mountId) s->Mount(mountId);

            ++spawned;
        }

        TC_LOG_DEBUG("scripts", "AmbientNPC: [%s] spawned %u/%u in zone %u (map %u)",
            player->GetName().c_str(), spawned, needed, zoneId, player->GetMapId());
    }
};

std::mutex AmbientWorldPlayerScript::s_mutex;
std::map<ObjectGuid, std::chrono::steady_clock::time_point> AmbientWorldPlayerScript::s_nextSpawnTime;

// Called from npc_ambient_world.cpp registration
AmbientWorldPlayerScript* CreateAmbientPlayerScript() { return new AmbientWorldPlayerScript(); }

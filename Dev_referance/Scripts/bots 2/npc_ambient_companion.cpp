/*
 * npc_ambient_companion.cpp
 * Companion data management: globals, map operations, DB persistence, roster system
 */

#include "npc_ambient_world.h"

// ============================================================
//  Global companion map + mutex  (externed in header)
// ============================================================
std::mutex                          s_companionMutex;
std::map<ObjectGuid, CompanionData> s_companions;

// ============================================================
//  Global roster data  (externed in header)
// ============================================================
std::mutex                                     s_rosterMutex;
std::map<ObjectGuid, std::vector<RosterEntry>> s_raidRoster;
std::map<ObjectGuid, uint32>                   s_extraTankOverride;

// ============================================================
//  XP formulas
// ============================================================
uint32 CompanionXpForLevel(uint32 level)
{
    return 100u * level * (1u + level / 10u);
}

uint32 CompanionXpGain(uint32 killedLevel)
{
    return killedLevel * 5u + 10u;
}

// ============================================================
//  Companion map operations (thread-safe)
// ============================================================
void RegisterCompanion(ObjectGuid guid, ObjectGuid owner, uint32 level,
    std::string const& name)
{
    std::lock_guard<std::mutex> lk(s_companionMutex);
    CompanionData d;
    d.ownerGuid    = owner;
    d.currentLevel = level;
    d.xp           = 0;
    d.xpNeeded     = CompanionXpForLevel(level);
    d.displayName  = name;
    s_companions[guid] = d;
}

void UnregisterCompanion(ObjectGuid guid)
{
    std::lock_guard<std::mutex> lk(s_companionMutex);
    s_companions.erase(guid);
}

bool IsCompanion(ObjectGuid guid)
{
    std::lock_guard<std::mutex> lk(s_companionMutex);
    return s_companions.count(guid) > 0;
}

bool AwardCompanionXP(ObjectGuid guid, uint32 gain, uint32& outNewLevel)
{
    std::lock_guard<std::mutex> lk(s_companionMutex);
    auto it = s_companions.find(guid);
    if (it == s_companions.end())
    {
        outNewLevel = 0;
        return false;
    }
    CompanionData& d = it->second;
    d.xp += gain;
    if (d.xp >= d.xpNeeded && d.currentLevel < 110)
    {
        d.xp      -= d.xpNeeded;
        ++d.currentLevel;
        d.xpNeeded = CompanionXpForLevel(d.currentLevel);
        outNewLevel = d.currentLevel;
        return true;
    }
    outNewLevel = d.currentLevel;
    return false;
}

bool GetCompanionData(ObjectGuid guid, CompanionData& out)
{
    std::lock_guard<std::mutex> lk(s_companionMutex);
    auto it = s_companions.find(guid);
    if (it == s_companions.end())
        return false;
    out = it->second;
    return true;
}

bool GetCompanionDataIfOwned(ObjectGuid guid, ObjectGuid requiredOwner, CompanionData& out)
{
    std::lock_guard<std::mutex> lk(s_companionMutex);
    auto it = s_companions.find(guid);
    if (it == s_companions.end())
        return false;
    if (it->second.ownerGuid != requiredOwner)
        return false;
    out = it->second;
    return true;
}

bool IncrementKillCount(ObjectGuid guid, uint32& outKills)
{
    std::lock_guard<std::mutex> lk(s_companionMutex);
    auto it = s_companions.find(guid);
    if (it == s_companions.end()) { outKills = 0; return false; }
    ++it->second.killCount;
    outKills = it->second.killCount;
    return true;
}

const char* GetCompanionTitle(uint32 kills)
{
    if (kills >= 500) return "Warlord";
    if (kills >= 250) return "Champion";
    if (kills >= 100) return "Slayer";
    if (kills >=  50) return "Veteran";
    if (kills >=  10) return "Freshman";
    return nullptr;
}

// ============================================================
//  T1: Kill rate tracking (anti-farm)
// ============================================================
static std::map<ObjectGuid, std::vector<std::chrono::steady_clock::time_point>> s_killTimestamps;
static std::mutex s_killRateMutex;

bool CheckKillRateLimit(ObjectGuid companionGuid)
{
    auto now = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lk(s_killRateMutex);
    auto& stamps = s_killTimestamps[companionGuid];
    // Purge entries older than the window
    auto cutoff = now - std::chrono::milliseconds(KILL_RATE_WINDOW_MS);
    stamps.erase(std::remove_if(stamps.begin(), stamps.end(),
        [&](auto const& tp) { return tp < cutoff; }), stamps.end());
    if (stamps.size() >= KILL_RATE_MAX_PER_WINDOW)
        return false; // rate limited
    stamps.push_back(now);
    return true; // OK
}

// ============================================================
//  T3: Gossip rate limiter
// ============================================================
static std::map<ObjectGuid, std::chrono::steady_clock::time_point> s_lastGossip;
static std::mutex s_gossipRateMutex;

bool CheckGossipRateLimit(ObjectGuid playerGuid)
{
    auto now = std::chrono::steady_clock::now();
    std::lock_guard<std::mutex> lk(s_gossipRateMutex);
    auto it = s_lastGossip.find(playerGuid);
    if (it != s_lastGossip.end())
    {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count();
        if (elapsed < 500) // 500ms cooldown between gossip opens
            return false;
    }
    s_lastGossip[playerGuid] = now;
    return true;
}

// ============================================================
//  DB persistence helpers
// ============================================================
void DB_SaveCompanion(uint32 playerGuid, Creature* companion,
    uint32 level, uint32 xp, uint32 killCount)
{
    std::string esc = companion->GetName();
    CharacterDatabase.EscapeString(esc);
    CharacterDatabase.PExecute(
        "REPLACE INTO character_companion "
        "(player_guid, name, entry, level, xp, display_id, kill_count) "
        "VALUES (%u, '%s', %u, %u, %u, %u, %u)",
        playerGuid, esc.c_str(),
        companion->GetEntry(), level, xp,
        companion->GetDisplayId(), killCount);
}

void DB_UpdateCompanionProgress(uint32 playerGuid,
    std::string const& name, uint32 level, uint32 xp, uint32 killCount)
{
    std::string esc = name;
    CharacterDatabase.EscapeString(esc);
    CharacterDatabase.PExecute(
        "UPDATE character_companion SET level=%u, xp=%u, kill_count=%u "
        "WHERE player_guid=%u AND name='%s'",
        level, xp, killCount, playerGuid, esc.c_str());
}

void DB_DeleteCompanion(uint32 playerGuid, std::string const& name)
{
    std::string esc = name;
    CharacterDatabase.EscapeString(esc);
    CharacterDatabase.PExecute(
        "DELETE FROM character_companion WHERE player_guid=%u AND name='%s'",
        playerGuid, esc.c_str());
}

void DB_DeleteAllCompanions(uint32 playerGuid)
{
    CharacterDatabase.PExecute(
        "DELETE FROM character_companion WHERE player_guid=%u", playerGuid);
}

void TryDisbandGroupIfLast(Player* owner, ObjectGuid removedGuid)
{
    if (!owner)
        return;
    bool hasMore = false;
    {
        std::lock_guard<std::mutex> lk(s_companionMutex);
        for (auto const& kv : s_companions)
            if (kv.first != removedGuid && kv.second.ownerGuid == owner->GetGUID())
                { hasMore = true; break; }
    }
    if (!hasMore)
    {
        if (Group* grp = owner->GetGroup())
            if (grp->GetMembersCount() <= 1)
                grp->Disband();
    }
}

// ============================================================
//  Roster DB helpers
// ============================================================
void DB_AddToRoster(uint32 playerGuid, std::string const& name, uint8 rosterRole, uint8 rosterLevel)
{
    std::string esc = name;
    CharacterDatabase.EscapeString(esc);
    CharacterDatabase.PExecute(
        "REPLACE INTO character_raid_roster (player_guid, name, role, level) VALUES (%u, '%s', %u, %u)",
        playerGuid, esc.c_str(), (uint32)rosterRole, (uint32)rosterLevel);
}

void DB_ClearRoster(uint32 playerGuid)
{
    CharacterDatabase.PExecute(
        "DELETE FROM character_raid_roster WHERE player_guid=%u", playerGuid);
}

uint32 GetRosterCount(ObjectGuid playerGuid)
{
    std::lock_guard<std::mutex> lk(s_rosterMutex);
    auto it = s_raidRoster.find(playerGuid);
    return (it != s_raidRoster.end()) ? (uint32)it->second.size() : 0;
}

uint32 GetExtraTanks(ObjectGuid playerGuid)
{
    std::lock_guard<std::mutex> lk(s_rosterMutex);
    auto it = s_extraTankOverride.find(playerGuid);
    return (it != s_extraTankOverride.end()) ? it->second : 0;
}

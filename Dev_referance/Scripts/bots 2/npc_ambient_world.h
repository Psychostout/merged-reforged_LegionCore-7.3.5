/*
 * npc_ambient_world.h
 * Shared header for the Ambient NPC + Companion system
 *
 * File layout:
 *   npc_ambient_world.h        — this file (types, enums, declarations)
 *   npc_ambient_data.cpp       — name pools, speech pools, zone race maps, mount pools
 *   npc_ambient_companion.cpp  — CompanionData, DB helpers, roster, companion management
 *   npc_ambient_ai.cpp         — AI state machine + CreatureScript (gossip)
 *   npc_ambient_instance.cpp   — dungeon/raid entry, auto-fill, replica management
 *   npc_ambient_spawn.cpp      — PlayerScript, spawning, restore, chat commands
 *   npc_ambient_world.cpp      — registration only (AddSC_npc_ambient_world)
 */

#ifndef NPC_AMBIENT_WORLD_H
#define NPC_AMBIENT_WORLD_H

#include "ScriptMgr.h"
#include "Player.h"
#include "Map.h"
#include "ObjectMgr.h"
#include "Random.h"
#include "TemporarySummon.h"
#include "ObjectAccessor.h"
#include "ScriptedCreature.h"
#include "MotionMaster.h"
#include "Creature.h"
#include "Unit.h"
#include "Log.h"
#include "Chat.h"
#include "ScriptedGossip.h"
#include "DatabaseEnv.h"
#include "Group.h"
#include "GroupMgr.h"
#include "GameObject.h"

#include <ctime>
#include <string>
#include <chrono>
#include <mutex>
#include <map>
#include <set>
#include <cmath>
#include <vector>
#include <tuple>

// ============================================================
//  Constants
// ============================================================
static constexpr uint32 AMBIENT_ENTRY_MIN     = 9500080;
static constexpr uint32 AMBIENT_ENTRY_MAX     = 9500094;

static constexpr uint32 MIN_AMBIENT_NPCS      = 10;
static constexpr uint32 SPAWN_COUNT_MIN       = 10;
static constexpr uint32 SPAWN_COUNT_MAX       = 20;
static constexpr float  SEARCH_RADIUS         = 200.f;
static constexpr float  SPREAD_RADIUS         = 50.f;
static constexpr uint32 DESPAWN_TIME_MS       = 30 * 60 * 1000;
static constexpr uint32 SPAWN_THROTTLE_MS     = 30 * 1000;
static constexpr uint32 INSTANCE_DESPAWN_MS   = 4u * 3600u * 1000u;

static constexpr uint32 ALLIANCE_POOL[5] = { 9500080, 9500081, 9500082, 9500083, 9500084 };
static constexpr uint32 HORDE_POOL[5]    = { 9500085, 9500086, 9500087, 9500088, 9500089 };
static constexpr uint32 NEUTRAL_POOL[5]  = { 9500090, 9500091, 9500092, 9500093, 9500094 };

// Gossip action IDs
static constexpr uint32 COMPANION_GOSSIP_SENDER  = 200;
static constexpr uint32 COMPANION_ACTION_HIRE     = 1;
static constexpr uint32 COMPANION_ACTION_DISMISS  = 2;
static constexpr uint32 COMPANION_ACTION_STATUS   = 3;
static constexpr uint32 COMPANION_ACTION_CLOSE    = 4;
static constexpr uint32 COMPANION_ACTION_AUTOPARTY = 5;

static constexpr uint32 ROSTER_GOSSIP_SENDER   = 201;
static constexpr uint32 ROSTER_ACTION_SIGNUP   = 10;
static constexpr uint32 ROSTER_ACTION_VIEW     = 11;
static constexpr uint32 ROSTER_ACTION_CLEAR    = 12;
static constexpr uint32 ROSTER_ACTION_EXTRA_TANK = 13;
static constexpr uint32 ROSTER_MAX_SIZE        = 40;

static constexpr uint32 MIN_STARTING_NPCS      = 3;
static constexpr uint32 STARTING_SPAWN_MIN     = 3;
static constexpr uint32 STARTING_SPAWN_MAX     = 6;

// Capital cities — bustling with faction NPCs
static constexpr uint32 MIN_CAPITAL_NPCS       = 25;
static constexpr uint32 CAPITAL_SPAWN_MIN      = 25;
static constexpr uint32 CAPITAL_SPAWN_MAX      = 40;

// T1: Anti-farm rate limit
static constexpr uint32 KILL_RATE_WINDOW_MS    = 60000;
static constexpr uint32 KILL_RATE_MAX_PER_WINDOW = 60;

// T2: Companion level cap relative to player
static constexpr uint32 COMPANION_LEVEL_BUFFER = 5;

// ============================================================
//  Role system
// ============================================================
enum AmbientRole : uint8
{
    AMBIENT_WARRIOR  = 0,
    AMBIENT_MAGE     = 1,
    AMBIENT_HEALER   = 2,
    AMBIENT_HUNTER   = 3,
    AMBIENT_ROGUE    = 4,
    AMBIENT_DEFAULT  = 5,
    AMBIENT_TANK     = 6,
};

AmbientRole GetRoleForEntry(uint32 entry);
uint32      GetCombatEmote(AmbientRole role);
const char* RoleName(AmbientRole role);
uint32      EntryForRole(AmbientRole role);

// ============================================================
//  AI state enum
// ============================================================
enum AmbientState : uint8
{
    STATE_IDLE     = 0,
    STATE_WANDER   = 1,
    STATE_ACTIVITY = 2,
    STATE_HUNT     = 3,
    STATE_SOCIAL   = 4,
    STATE_WORK     = 5,
};

// ============================================================
//  Companion data
// ============================================================
struct CompanionData
{
    ObjectGuid  ownerGuid;
    uint32      currentLevel = 1;
    uint32      xp           = 0;
    uint32      xpNeeded     = 110;
    uint32      killCount    = 0;
    std::string displayName;
    bool        isReplica    = false;
};

// Global companion map + mutex
extern std::mutex                          s_companionMutex;
extern std::map<ObjectGuid, CompanionData> s_companions;

// Companion XP formulas
uint32 CompanionXpForLevel(uint32 level);
uint32 CompanionXpGain(uint32 killedLevel);

// Companion map operations (thread-safe)
void RegisterCompanion(ObjectGuid guid, ObjectGuid owner, uint32 level, std::string const& name);
void UnregisterCompanion(ObjectGuid guid);
bool IsCompanion(ObjectGuid guid);
bool AwardCompanionXP(ObjectGuid guid, uint32 gain, uint32& outNewLevel);
bool GetCompanionData(ObjectGuid guid, CompanionData& out);
bool GetCompanionDataIfOwned(ObjectGuid guid, ObjectGuid requiredOwner, CompanionData& out);
bool IncrementKillCount(ObjectGuid guid, uint32& outKills);
const char* GetCompanionTitle(uint32 kills);

// T1: Kill rate tracking
bool CheckKillRateLimit(ObjectGuid companionGuid);

// DB persistence
void DB_SaveCompanion(uint32 playerGuid, Creature* companion, uint32 level, uint32 xp, uint32 killCount = 0);
void DB_UpdateCompanionProgress(uint32 playerGuid, std::string const& name, uint32 level, uint32 xp, uint32 killCount = 0);
void DB_DeleteCompanion(uint32 playerGuid, std::string const& name);
void DB_DeleteAllCompanions(uint32 playerGuid);
void TryDisbandGroupIfLast(Player* owner, ObjectGuid removedGuid);

// ============================================================
//  Roster system
// ============================================================
struct RosterEntry
{
    std::string displayName;
    AmbientRole role     = AMBIENT_DEFAULT;
    uint8       level    = 1;
    bool        isManual = true;
};

extern std::mutex                                     s_rosterMutex;
extern std::map<ObjectGuid, std::vector<RosterEntry>> s_raidRoster;
extern std::map<ObjectGuid, uint32>                   s_extraTankOverride;

void   DB_AddToRoster(uint32 playerGuid, std::string const& name, uint8 rosterRole, uint8 rosterLevel);
void   DB_ClearRoster(uint32 playerGuid);
uint32 GetRosterCount(ObjectGuid playerGuid);
uint32 GetExtraTanks(ObjectGuid playerGuid);

// ============================================================
//  Instance sizing
// ============================================================
struct InstanceSize { uint32 total; uint32 minTanks; uint32 minHealers; };
InstanceSize GetInstanceSize(Map* map);

// ============================================================
//  Zone data
// ============================================================
struct ZoneRacePool
{
    std::vector<uint32> primary;
    std::vector<uint32> secondary;
};

extern const std::set<uint32> SKIP_ZONES;
extern const std::set<uint32> STARTING_ZONES;
extern const std::set<uint32> CAPITAL_ZONES;
extern const std::set<uint32> ALLIANCE_ONLY_DISPLAY;
extern const std::set<uint32> HORDE_ONLY_DISPLAY;
extern const std::map<uint32, ZoneRacePool> ZONE_RACE_MAP;

// ============================================================
//  Name / Speech / Mount data (defined in npc_ambient_data.cpp)
// ============================================================
namespace AmbientNames  { std::string Roll(uint32 npcEntry); }
namespace AmbientSpeech { const char* Roll(uint32 npcEntry); }
uint32 GetMountDisplayId(uint32 npcEntry, uint8 level);

// ============================================================
//  Spawn helpers (defined in npc_ambient_spawn.cpp)
// ============================================================
uint32   PickAmbientEntry(Player* player);
uint32   CountNearbyAmbient(Player* player);
Position RandomPositionNear(Player* player);

// ============================================================
//  NPC stat scaling helper — shared across spawn, restore, instance fill
// ============================================================
inline void ScaleNpcStats(Creature* c, uint8 level)
{
    uint32 hp = 100u * level * level + 500u * level + 1000u;
    c->SetMaxHealth(hp);
    c->SetHealth(hp);
    c->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(level) * 0.90f);
    c->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(level) * 1.35f);
    c->UpdateDamagePhysical(BASE_ATTACK);
}

// Instance-specific stat boost — companions in dungeons/raids need more HP/damage
// to meaningfully contribute against instance-tuned mobs
inline void ScaleNpcStatsForInstance(Creature* c, uint8 level, bool isRaid)
{
    // Raid = 3x multiplier, Dungeon = 2x multiplier over base
    float mult = isRaid ? 3.0f : 2.0f;
    uint32 hp = uint32((100u * level * level + 500u * level + 1000u) * mult);
    c->SetMaxHealth(hp);
    c->SetHealth(hp);
    float dmgMult = isRaid ? 2.0f : 1.5f;
    c->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(level) * 0.90f * dmgMult);
    c->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(level) * 1.35f * dmgMult);
    c->UpdateDamagePhysical(BASE_ATTACK);
}

// T4: Validate NPC entry is in our ambient range
inline bool IsValidAmbientEntry(uint32 entry)
{
    return entry >= AMBIENT_ENTRY_MIN && entry <= AMBIENT_ENTRY_MAX;
}

// ============================================================
//  Forward declarations for AI (defined in npc_ambient_ai.cpp)
// ============================================================
struct npc_ambient_aiAI;

// Free-function wrappers (no full struct def needed to call these)
bool AmbientHireCompanion(Creature* creature, Player* player,
    uint32 restoreLevel = 0, uint32 restoreXp = 0, bool isReplica = false);
bool AmbientDismissCompanion(Creature* creature);
bool IsAmbientCompanionCreature(Creature* creature);
bool IsAmbientCompanionOf(Creature* creature, ObjectGuid playerGuid);
AmbientRole AmbientGetRole(Creature* creature);

// Party composition check
struct PartyCompo { uint32 tanks = 0, healers = 0, dps = 0, total = 0; };
PartyCompo ComputePartyCompo(Player* player);

// T3: Gossip rate limiter
bool CheckGossipRateLimit(ObjectGuid playerGuid);

// ============================================================
//  Instance functions (defined in npc_ambient_instance.cpp)
// ============================================================
void FlushAndCleanReplicas(Player* player);
void RestoreRosterForPlayer(Player* player);
void HandleInstanceEntry(Player* player, Map* map);

// ============================================================
//  Registration entry point
// ============================================================
void AddSC_npc_ambient_world();

#endif // NPC_AMBIENT_WORLD_H

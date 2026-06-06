/*
 * npc_ambient_world.cpp
 * BotSystem Phase 5 -- Dynamic Ambient NPC Population + Fake-Player AI
 *
 * SPAWNING  (PlayerScript):
 *   - OnUpdateZone fires for every zone/subzone change
 *   - Counts ambient summons within 200y; spawns 3-6 if below minimum
 *   - Works on ALL non-instanced, non-capital outdoor zones (including DK start)
 *   - Spawns player-archetype NPCs (Warrior/Mage/Rogue/etc.) with class subnames
 *
 * BEHAVIOUR (CreatureScript  "npc_ambient_ai"):
 *   State-machine loop:  IDLE -> WANDER -> ACTIVITY -> HUNT -> SOCIAL
 *
 *   IDLE     - stand still 3-7s; play a random flavour emote
 *   WANDER   - walk via MovePoint to a fresh position near home (15-60y)
 *              terrain Z is looked up so they land on the ground
 *   ACTIVITY - role-specific anim: spellcast (mage/healer), work-sheathed
 *              (warrior/rogue), bow-shot (hunter), eating (default)
 *   HUNT     - scan 40y for any non-player hostile creature not in combat;
 *              rush over and actually attack it (real melee / emotes)
 *   SOCIAL   - face the closest ambient NPC within 20y and wave/cheer/bow;
 *              the target waves back half the time
 *
 * Template pools (NPC entries in the world DB):
 *   Alliance  9500080-9500084   Warrior / Paladin / Mage / Hunter / Rogue
 *   Horde     9500085-9500089   Warrior / Shaman  / Hunter/ Mage / Rogue
 *   Neutral   9500090-9500094   Death Knight / Demon Hunter / Druid / Monk / Priest
 *   Scourge pool removed -- all zones now use the player-like entries above
 */

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

// ============================================================
//  Name generator - culture-appropriate pools
// ============================================================
namespace AmbientNames
{
    static const char* ALLIANCE[] = {
        "Aldric","Arwyn","Brennan","Caelan","Davin","Elara","Fenwick","Gareth",
        "Haldor","Idris","Jaryn","Kaelan","Lira","Maren","Neth","Owyn","Petra",
        "Quill","Raena","Seldra","Toven","Uric","Vael","Wynn","Xara","Yona","Zael",
        "Borin","Bryndis","Caldric","Dara","Edric","Fenna","Gilda","Holt","Ira",
        "Jeth","Kira","Lodric","Mira","Niall","Odric","Pell","Rann","Sigrid","Thal",
        "Ulric","Vara","Weiss","Ylva","Aeron","Beric","Corra","Dwyn","Elan","Fyrd",
        "Gwynn","Hadric","Ilara","Jorel","Kaera","Lorin","Myra","Naric","Orla","Perin",
    };
    static constexpr uint32 ALLIANCE_COUNT = 63;

    static const char* HORDE_ORC[] = {
        "Goruk","Krag","Morg","Nathrak","Rakh","Skrag","Throk","Urgok","Vrak","Zug",
        "Grish","Lurg","Mash","Nok","Org","Pug","Rok","Snag","Trog","Urg",
        "Grak","Hurk","Kash","Murk","Prak","Rorg","Snork","Targ","Urk","Vorg",
        "Bruk","Drakh","Fruk","Gnak","Hurg","Kruk","Lurk","Nrug","Pruk","Sruk",
        "Drakka","Grokka","Hrakka","Krukka","Mrakka","Nrakka","Prikka","Rrakka",
    };
    static constexpr uint32 HORDE_ORC_COUNT = 48;

    static const char* HORDE_TROLL[] = {
        "Zek'han","Rokhan","Jen'ari","Khal'dun","Maz'jin",
        "Zen'kaji","Dal'jin","Fal'zul","Gal'jin","Hal'zek","Jal'zan","Kal'jin",
        "Lal'zul","Mal'dun","Nal'jin","Pal'zek","Ral'zul","Sal'jin","Tal'dun",
        "Ulu'zek","Val'jin","Wal'zul","Xal'jin","Yul'zek","Zal'jin","Bel'dun",
        "Drek'zul","Frek'jin","Grek'zul","Hrek'jin",
    };
    static constexpr uint32 HORDE_TROLL_COUNT = 30;

    static const char* HORDE_TAUREN[] = {
        "Hamuul","Trag","Mak","Brightmane","Longrunner",
        "Earthcaller","Ragehoof","Plainstrider","Moonsong","Rivermane",
        "Stonehoof","Bloodhoof","Thunderhorn","Sunwalker","Highmountain",
        "Grimtotem","Windsong","Dustwalker","Ironhorn","Swiftmane",
        "Skychaser","Mudhorn","Rockhide","Firewalker","Cloudhoof",
    };
    static constexpr uint32 HORDE_TAUREN_COUNT = 25;

    static const char* HORDE_BELF[] = {
        "Selvaine","Aelindris","Vaelris","Thelris","Kaelindra","Sylvaris",
        "Faelindra","Maelindra","Naelindra","Raelindra","Daelindra","Baelindra",
        "Sorel","Lorel","Morel","Norel","Porel","Rorel","Torel","Vorel",
        "Lyria","Myria","Pyria","Tyria","Xyria","Zyria",
    };
    static constexpr uint32 HORDE_BELF_COUNT = 26;

    static const char* NEUTRAL[] = {
        // Faction-ambiguous traveller / wanderer names (no iconic race-specific characters)
        "Aelindra","Faelyn","Sylara","Varethis","Thalion","Isaeryn","Vaelrin",
        "Celindra","Xarven","Ylarven","Zarven","Alorin","Belorin","Celorin",
        "Elorin","Felorin","Gelorin","Helorin","Ilorin","Jalorin","Kalorin",
        "Morthalun","Yunlan","Drevok",
        // generic; no Night Elf, no iconic characters
        "Xuen","Niuzao","Yulon","Wavemender",
        "Aravel","Brevel","Crevel","Drevel","Erevel","Frevel","Grevel",
        "Caldren","Selvaine","Vexthar","Morwen","Aldric","Serath","Kyven",
        "Draeven","Lorath","Miravel","Torven","Aethon","Quiran","Zaneth",
    };
    static constexpr uint32 NEUTRAL_COUNT = 47;

    static const char* SURNAMES[] = {
        "Swiftblade","Stonehammer","Ironhide","Dawnseeker","Shadowstep",
        "Frostweave","Emberforge","Stormcaller","Nightwhisper","Sunfire",
        "Moonveil","Ashvale","Grimshaw","Brightmantle","Coldwater",
        "Dustrunner","Ironclad","Lightbringer","Windwalker","Duskmantle",
        "Emberveil","Frostfall","Goldvein","Highpeak","Ironwood",
        "Jadewing","Kindlesmith","Longstride","Mistwalker","Northwind",
        "Oakenshield","Pinecrest","Quickstrike","Redthorn","Silverbow",
        "Thornwood","Veilshroud","Wardbane","Zenith","Flamestrike",
        "Crystalvein","Dawnblade","Earthshaker","Farseer","Galeforce",
    };
    static constexpr uint32 SURNAMES_COUNT = 45;

    static std::string Roll(uint32 npcEntry)
    {
        const char* first = nullptr;
        if (npcEntry >= 9500080 && npcEntry <= 9500084)
            first = ALLIANCE[urand(0, ALLIANCE_COUNT - 1)];
        else if (npcEntry >= 9500085 && npcEntry <= 9500086)
            first = HORDE_ORC[urand(0, HORDE_ORC_COUNT - 1)];
        else if (npcEntry == 9500087)
            first = HORDE_TROLL[urand(0, HORDE_TROLL_COUNT - 1)];
        else if (npcEntry == 9500088)
            first = HORDE_BELF[urand(0, HORDE_BELF_COUNT - 1)];
        else if (npcEntry == 9500089)
            first = HORDE_TAUREN[urand(0, HORDE_TAUREN_COUNT - 1)];
        else
            first = NEUTRAL[urand(0, NEUTRAL_COUNT - 1)];

        std::string name = first ? first : "Adventurer";
        if (urand(0, 99) < 35)
        {
            name += " ";
            name += SURNAMES[urand(0, SURNAMES_COUNT - 1)];
        }
        return name;
    }
}

// ============================================================
//  Ambient NPC speech lines (B1) — culture-appropriate flavour
// ============================================================
namespace AmbientSpeech
{
    // Alliance — noble, hopeful, weary of war
    static const char* ALLIANCE_LINES[] = {
        "The Light illuminate your path.",
        "Stormwind will endure. It always does.",
        "King Anduin grows into a fine leader.",
        "I've heard the Broken Isles are deadly. I believe every word.",
        "Have you seen the view from up here? Breathtaking.",
        "These roads grow more dangerous by the day.",
        "Stay sharp. There are things lurking in the shadows.",
        "For the Alliance! Every last one of us.",
        "The Burning Legion is no joke. Be ready.",
        "The Illidari are unsettling... but useful in a fight.",
        "Watch your purse in Dalaran. Even a floating city has thieves.",
        "We've bled for this world. We'll bleed for it again.",
        "Heroes rise when the world needs them most.",
        "I still can't believe the demon invasions. Dark days.",
    };
    static constexpr uint32 ALLIANCE_COUNT = 14;

    // Horde — honour, strength, quiet pride
    static const char* HORDE_LINES[] = {
        "Lok'tar Ogar! Victory or death!",
        "The Horde endures through strength and cunning.",
        "Vol'jin's sacrifice will not be forgotten.",
        "Blood and thunder. We stand as one.",
        "Honor above all else. Remember that.",
        "The demons thought us prey. They were wrong.",
        "Thrall made us more than savages. Never forget that.",
        "Suramar... that city games with your mind.",
        "For the Horde! We do not falter!",
        "The spirits guide our blades. Trust in them.",
        "These Broken Isles hold old power. Tread carefully.",
        "Only the strong survive. That is the way of things.",
        "We will not be broken. Not by the Legion. Not by anyone.",
        "Sylvanas keeps her own counsel. Wise to do so.",
    };
    static constexpr uint32 HORDE_COUNT = 14;

    // Neutral — philosophical, worldly, a little ominous
    static const char* NEUTRAL_LINES[] = {
        "The world turns, with or without us.",
        "Power attracts the corrupt. Guard yourself.",
        "Watch your back out there.",
        "The Burning Legion's time is ending. I can feel it.",
        "Neither the Light nor the Shadow is absolute.",
        "Ancient evils stir in the deep places.",
        "Order. Chaos. The balance shifts.",
        "Strange times... stranger alliances.",
        "Valor is not the absence of fear. It is action despite it.",
        "The dead walk and the skies burn. What does that tell you?",
        "Choose your battles wisely.",
        "I've seen empires fall. I've seen hope survive worse than this.",
        "There is still beauty in this world, if you know where to look.",
        "Every scar on this land tells a story worth knowing.",
    };
    static constexpr uint32 NEUTRAL_COUNT = 14;

    static const char* Roll(uint32 npcEntry)
    {
        if (npcEntry >= 9500080 && npcEntry <= 9500084)
            return ALLIANCE_LINES[urand(0, ALLIANCE_COUNT - 1)];
        if (npcEntry >= 9500085 && npcEntry <= 9500089)
            return HORDE_LINES[urand(0, HORDE_COUNT - 1)];
        return NEUTRAL_LINES[urand(0, NEUTRAL_COUNT - 1)];
    }
}

// ============================================================
//  Spawn configuration
// ============================================================
static constexpr uint32 MIN_AMBIENT_NPCS  = 4;
static constexpr uint32 SPAWN_COUNT_MIN   = 3;
static constexpr uint32 SPAWN_COUNT_MAX   = 6;
static constexpr float  SEARCH_RADIUS     = 200.f;
static constexpr float  SPREAD_RADIUS     = 60.f;
static constexpr uint32 DESPAWN_TIME_MS   = 30 * 60 * 1000;
static constexpr uint32 SPAWN_THROTTLE_MS = 30 * 1000;

static constexpr uint32 ALLIANCE_POOL[5] = { 9500080, 9500081, 9500082, 9500083, 9500084 };
static constexpr uint32 HORDE_POOL[5]    = { 9500085, 9500086, 9500087, 9500088, 9500089 };
static constexpr uint32 NEUTRAL_POOL[5]  = { 9500090, 9500091, 9500092, 9500093, 9500094 };

static const std::set<uint32> SKIP_ZONES =
{
    1519, 1637, 1657, 1638, 1539, 362,
    14,   3487, 3557,
    4395, 7502, 7563,
};

static const std::set<uint32> STARTING_ZONES =
{
    12,   // Elwynn Forest
    1,    // Dun Morogh
    141,  // Teldrassil
    3524, // Azuremyst Isle
    4987, // Gilneas
    85,   // Tirisfal Glades
    215,  // Mulgore
    3430, // Eversong Woods
    5170, // Wandering Isle
    4815, // Lost Isles
};

static constexpr uint32 MIN_STARTING_NPCS  = 10;
static constexpr uint32 STARTING_SPAWN_MIN = 10;
static constexpr uint32 STARTING_SPAWN_MAX = 18;

// ============================================================
//  Race display ID pools (zone-based NPC appearance).
//  All IDs verified against creature_model_info in this DB build.
//  Primary pool = ~70 % of spawns (dominant zone race).
//  Secondary pool = ~30 % (faction racial variety).
//
//  Verified display IDs (BoundingRadius ~0.3-0.4 = humanoid-scale):
//   Human    : 3167, 3258, 3257   (Stormwind Guards)
//   Dwarf    : 1598, 1608, 3524   (Ironforge Guards/Mountaineer)
//   Night Elf: 4408, 4841, 4842   (NE Trainer, NE Sentinel)
//   Draenei  : 11650,11652,16602  (Draenei Refugee, Vindicator)
//   Worgen   : 29317,29318,30215  (Gilnean Militia)
//   Forsaken : 2858, 2855, 1648   (Tirisfal Deathguards)
//   Tauren   : 1678, 3797         (Tauren Druid Trainers)
//   Blood Elf: 18980,18982,18981  (BE Reclaimer / Surveyor)
//   Orc      : 4573, 4551         (World Orc humanoids)
//   Goblin   : 7107, 7109, 7108   (Goblin Mercenary / Engineer)
//   Troll    : 4609, 1882, 15574  (World Troll Trainers, Darkspear)
//   Pandaren : 29421,29422        (Wandering Isle)
//   Gnome    : 2490, 2891, 3562   (Gnome Racer, World Gnome Trainers)
// ============================================================
struct ZoneRacePool
{
    std::vector<uint32> primary;    // heavily weighted
    std::vector<uint32> secondary;  // lightly weighted (racial variety)
};

// Faction-exclusive display ID sets — prevents Alliance models on Horde NPCs and vice versa
static const std::set<uint32> ALLIANCE_ONLY_DISPLAY = {
    3167, 3258, 3257,           // Human
    1598, 1608, 3524,           // Dwarf
    4408, 4841, 4842,           // Night Elf
    11650, 11652, 16602,        // Draenei
    29317, 29318, 30215,        // Worgen
    2490, 2891, 3562,           // Gnome
};
static const std::set<uint32> HORDE_ONLY_DISPLAY = {
    2858, 2855, 1648,           // Forsaken
    1678, 3797,                 // Tauren
    18980, 18982, 18981,        // Blood Elf
    4573, 4551,                 // Orc
    7107, 7109, 7108,           // Goblin
    4609, 1882, 15574,          // Troll
};

static const std::map<uint32, ZoneRacePool> ZONE_RACE_MAP =
{
    // ---- Alliance starting zones ----
    { 12,   { {3167,3258,3167,3258,3257}, {1598,1608,4408,4841} } },         // Elwynn:         Human, Dwarf/NElf
    { 1,    { {1598,1608,3524,1598,1608}, {3167,3258,2490,2891} } },         // Dun Morogh:     Dwarf, Human/Gnome
    { 141,  { {4408,4841,4842,4408,4841}, {3167,3258,11650,11652} } },       // Teldrassil:     Night Elf, Human/Draenei
    { 3524, { {11650,11652,16602,11650},  {4408,4841,3167,3258} } },         // Azuremyst:      Draenei, NElf/Human
    { 4987, { {29317,29318,30215,3167},   {1598,1608,4408,4841} } },         // Gilneas:        Worgen+Human, Dwarf/NElf
    // ---- Horde starting zones ----
    { 85,   { {2858,2855,1648,2858,2855}, {18980,18982,4573,4551,1678} } },  // Tirisfal:       Forsaken, BElf/Orc/Tauren
    { 215,  { {1678,3797,1678,3797},      {4573,4551,2858,2855,18980} } },   // Mulgore:        Tauren, Orc/Forsaken/BElf
    { 3430, { {18980,18982,18981,18980},  {4573,4551,2858,2855,1678} } },    // Eversong:       Blood Elf, Orc/Forsaken/Tauren
    { 4815, { {7107,7109,4573,4551,7108}, {1678,2858,2855,18980} } },        // LostIsles:      Goblin+Orc, Tauren/Forsaken/BElf
    // ---- Neutral ----
    { 5170, { {29421,29422,29421,29422},  {3167,3258,4573,4551} } },         // WanderingIsle:  Pandaren, Human/Orc
    // ---- B3: Eastern Kingdoms — open-world zones ----
    { 10,   { {3167,3258,3257,3167,3258}, {29317,29318,2490,2891} } },       // Duskwood:        Human, Worgen/Gnome
    { 40,   { {3167,3258,3257,3167,3258}, {1598,1608,2490,2891} } },         // Westfall:        Human, Dwarf/Gnome
    { 44,   { {3167,3258,3257,1598,1608}, {4408,4841,11650,11652} } },       // Redridge Mtns:   Human+Dwarf, NElf/Draenei
    { 45,   { {3167,3258,3257,2858,2855}, {1598,1608,4573,4551} } },         // Arathi Highlands: Human+Forsaken, Dwarf/Orc
    { 33,   { {3167,3258,3257,4609,1882}, {4573,4551,7107,7109} } },         // Stranglethorn:   Human+Troll, Orc/Goblin
    { 224,  { {3167,3258,3257,4609,1882}, {4573,4551,7107,7109} } },         // N.Stranglethorn: Human+Troll, Orc/Goblin
    { 267,  { {3167,3258,2858,2855,3257}, {4573,4551,1678,3797} } },         // Hillsbrad:       Human+Forsaken, Orc/Tauren
    { 28,   { {2858,2855,1648,2858,2855}, {3167,3258,4408,4841} } },         // W.Plaguelands:   Forsaken, Human/NElf
    { 139,  { {2858,2855,1648,3167,3258}, {4408,4841,11650,11652} } },       // E.Plaguelands:   Forsaken+Human, NElf/Draenei
    { 46,   { {3167,3258,1598,1608,3257}, {4573,4551,2858,2855} } },         // Burning Steppes: Human+Dwarf, Orc/Forsaken
    { 3,    { {1598,1608,3524,1598,1608}, {3167,3258,4573,4551} } },         // Badlands:        Dwarf, Human/Orc
    { 8,    { {3167,3258,3257,18980,18982},{4573,4551,2858,2855} } },         // Swamp of Sorrows: Human+BElf, Orc/Forsaken
    { 4,    { {3167,3258,3257,4573,4551}, {2858,2855,1678,3797} } },         // Blasted Lands:   Human+Orc, Forsaken/Tauren
    // ---- B3: Kalimdor — open-world zones ----
    { 148,  { {4408,4841,4842,4408,4841}, {3167,3258,11650,11652} } },       // Darkshore:       Night Elf, Human/Draenei
    { 331,  { {4408,4841,4842,4408,4841}, {4573,4551,4609,1882} } },         // Ashenvale:       Night Elf, Orc/Troll
    { 17,   { {4573,4551,1678,3797,4573}, {4609,1882,2858,2855} } },         // The Barrens:     Orc+Tauren, Troll/Forsaken
    { 400,  { {1678,3797,1678,3797},      {4573,4551,2490,2891} } },         // Thousand Needles: Tauren, Orc/Gnome
    { 440,  { {7107,7109,3167,3258,7108}, {4609,1882,4573,4551} } },         // Tanaris:         Goblin+Human, Troll/Orc
    { 357,  { {4408,4841,4842,1678,3797}, {3167,3258,1598,1608} } },         // Feralas:         NElf+Tauren, Human/Dwarf
    { 490,  { {3167,3258,3257,7107,7109}, {4573,4551,4609,1882} } },         // Un'Goro Crater:  Human+Goblin, Orc/Troll
    // ---- B3: Outland zones ----
    { 464,  { {4573,4551,3167,3258,4573}, {11650,11652,2858,2855} } },       // Hellfire Pen.:   Orc+Human, Draenei/Forsaken
    { 478,  { {11650,11652,16602,1678},   {3167,3258,4408,4841} } },         // Zangarmarsh:     Draenei+Tauren, Human/NElf
    { 477,  { {18980,18982,11650,18981},  {4573,4551,3167,3258} } },         // Terokkar Forest: BElf+Draenei, Orc/Human
    { 475,  { {1678,3797,4573,4551,1678}, {11650,11652,3167,3258} } },       // Nagrand:         Tauren+Orc, Draenei/Human
    { 473,  { {4573,4551,1678,3797,4573}, {11650,11652,3167,3258} } },       // Blade's Edge:    Orc+Tauren, Draenei/Human
    { 476,  { {18980,18982,11650,11652},  {4573,4551,2858,2855} } },         // Shadowmoon Val.: BElf+Draenei, Orc/Forsaken
    { 3523, { {18980,18982,11650,11652},  {2490,2891,3167,3258} } },         // Netherstorm:     BElf+Draenei, Gnome/Human
    // ---- B3: Northrend zones ----
    { 3537, { {3167,3258,3257,1598,1608}, {1678,3797,4573,4551} } },         // Borean Tundra:   Human+Dwarf, Tauren/Orc
    { 495,  { {3167,3258,3257,2858,2855}, {1598,1608,4573,4551} } },         // Howling Fjord:   Human+Forsaken, Dwarf/Orc
    { 65,   { {3167,3258,2858,2855,3257}, {1678,3797,4408,4841} } },         // Dragonblight:    Human+Forsaken, Tauren/NElf
    { 394,  { {3167,3258,2858,2855,4573}, {1598,1608,1678,3797} } },         // Icecrown:        Human+Forsaken+Orc, Dwarf/Tauren
    // ---- B3: Cataclysm zones ----
    { 616,  { {4408,4841,4842,1678,3797}, {3167,3258,4573,4551} } },         // Hyjal:           NElf+Tauren, Human/Orc
    { 545,  { {1598,1608,3524,4573,4551}, {3167,3258,18980,18982} } },       // Deepholm:        Dwarf+Orc, Human/BElf
    { 540,  { {3167,3258,3257,7107,7109}, {11650,11652,4573,4551} } },       // Uldum:           Human+Goblin, NElf/Orc
    // ---- B3: Pandaria zones ----
    { 376,  { {29421,29422,3167,3258},    {4408,4841,4573,4551} } },         // Jade Forest:     Pandaren+Human, NElf/Orc
    { 379,  { {29421,29422,4573,4551},    {1678,3797,3167,3258} } },         // Kun-Lai Summit:  Pandaren+Orc, Tauren/Human
    { 371,  { {29421,29422,3167,3258},    {1678,3797,4573,4551} } },         // Valley 4 Winds:  Pandaren+Human, Tauren/Orc
    // ---- B3: Draenor zones ----
    { 534,  { {4573,4551,3167,3258,4573}, {1598,1608,1678,3797} } },         // Tanaan Jungle:   Orc+Human, Dwarf/Tauren
    { 543,  { {4573,4551,1678,3797,4573}, {3167,3258,11650,11652} } },       // Gorgrond:        Orc+Tauren, Human/Draenei
    { 539,  { {18980,18982,11650,11652},  {4573,4551,3167,3258} } },         // Spires of Arak:  BElf+Draenei, Orc/Human
    // ---- B3: Broken Isles zones ----
    { 630,  { {4408,4841,4842,18980,18982},{3167,3258,11650,11652} } },      // Azsuna:          NElf+BElf, Human/Draenei
    { 641,  { {4408,4841,4842,1678,3797}, {3167,3258,4573,4551} } },         // Val'sharah:      NElf+Tauren, Human/Orc
    { 634,  { {1678,3797,4408,4841,1678}, {4573,4551,3167,3258} } },         // Highmountain:    Tauren+NElf, Orc/Human
    { 628,  { {3167,3258,2858,2855,3257}, {4573,4551,1598,1608} } },         // Stormheim:       Human+Forsaken, Orc/Dwarf
    { 708,  { {18980,18982,4408,4841},    {3167,3258,11650,11652} } },        // Suramar:         BElf+NElf, Human/Draenei
};

// ============================================================
//  Companion system
// ============================================================
static constexpr uint32 COMPANION_GOSSIP_SENDER  = 200;
static constexpr uint32 COMPANION_ACTION_HIRE    = 1;
static constexpr uint32 COMPANION_ACTION_DISMISS = 2;
static constexpr uint32 COMPANION_ACTION_STATUS  = 3;
static constexpr uint32 COMPANION_ACTION_CLOSE      = 4;  // "Goodbye" - just close menu
static constexpr uint32 COMPANION_ACTION_AUTOPARTY  = 5;  // "Form a balanced party" auto-hire

// Raid roster gossip
static constexpr uint32 ROSTER_GOSSIP_SENDER      = 201;
static constexpr uint32 ROSTER_ACTION_SIGNUP      = 10;  // Sign NPC up for my raid
static constexpr uint32 ROSTER_ACTION_VIEW        = 11;  // Print roster summary
static constexpr uint32 ROSTER_ACTION_CLEAR       = 12;  // Wipe roster
static constexpr uint32 ROSTER_ACTION_EXTRA_TANK  = 13;  // +1 tank for boss overrides
static constexpr uint32 ROSTER_MAX_SIZE           = 40;  // max roster entries (40-man cap)

static uint32 CompanionXpForLevel(uint32 level)
{
    return 100u * level * (1u + level / 10u);
}

static uint32 CompanionXpGain(uint32 killedLevel)
{
    return killedLevel * 5u + 10u;
}

struct CompanionData
{
    ObjectGuid  ownerGuid;
    uint32      currentLevel = 1;
    uint32      xp           = 0;
    uint32      xpNeeded     = 110;
    uint32      killCount    = 0;     // lifetime kills for this companion
    std::string displayName;          // cached at HireCompanion time
    bool        isReplica    = false; // true = instance copy; do NOT delete DB row on death
};

static std::mutex                          s_companionMutex;
static std::map<ObjectGuid, CompanionData> s_companions;

static void RegisterCompanion(ObjectGuid guid, ObjectGuid owner, uint32 level,
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

static void UnregisterCompanion(ObjectGuid guid)
{
    std::lock_guard<std::mutex> lk(s_companionMutex);
    s_companions.erase(guid);
}

static bool IsCompanion(ObjectGuid guid)
{
    std::lock_guard<std::mutex> lk(s_companionMutex);
    return s_companions.count(guid) > 0;
}

static bool AwardCompanionXP(ObjectGuid guid, uint32 gain, uint32& outNewLevel)
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

static bool GetCompanionData(ObjectGuid guid, CompanionData& out)
{
    std::lock_guard<std::mutex> lk(s_companionMutex);
    auto it = s_companions.find(guid);
    if (it == s_companions.end())
        return false;
    out = it->second;
    return true;
}

// E4: Single-lock helper — avoids TOCTOU between IsCompanion() + GetCompanionData() pair.
// Returns true only if the companion exists AND belongs to requiredOwner.
static bool GetCompanionDataIfOwned(ObjectGuid guid, ObjectGuid requiredOwner, CompanionData& out)
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

// A8/A9: Increment kill count, return new total
static bool IncrementKillCount(ObjectGuid guid, uint32& outKills)
{
    std::lock_guard<std::mutex> lk(s_companionMutex);
    auto it = s_companions.find(guid);
    if (it == s_companions.end()) { outKills = 0; return false; }
    ++it->second.killCount;
    outKills = it->second.killCount;
    return true;
}

// A9: Title thresholds
static const char* GetCompanionTitle(uint32 kills)
{
    if (kills >= 500) return "Warlord";
    if (kills >= 250) return "Champion";
    if (kills >= 100) return "Slayer";
    if (kills >=  50) return "Veteran";
    if (kills >=  10) return "Freshman";
    return nullptr;
}

// ============================================================
//  DB persistence helpers (character_companion table)
//  PK = (player_guid, name)  -- one row per companion per player.
// ============================================================
static void DB_SaveCompanion(uint32 playerGuid, Creature* companion,
    uint32 level, uint32 xp, uint32 killCount = 0)
{
    // Escape companion name to prevent SQL injection
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

static void DB_UpdateCompanionProgress(uint32 playerGuid,
    std::string const& name, uint32 level, uint32 xp, uint32 killCount = 0)
{
    std::string esc = name;
    CharacterDatabase.EscapeString(esc);
    CharacterDatabase.PExecute(
        "UPDATE character_companion SET level=%u, xp=%u, kill_count=%u "
        "WHERE player_guid=%u AND name='%s'",
        level, xp, killCount, playerGuid, esc.c_str());
}

static void DB_DeleteCompanion(uint32 playerGuid, std::string const& name)
{
    std::string esc = name;
    CharacterDatabase.EscapeString(esc);
    CharacterDatabase.PExecute(
        "DELETE FROM character_companion WHERE player_guid=%u AND name='%s'",
        playerGuid, esc.c_str());
}

static void DB_DeleteAllCompanions(uint32 playerGuid)
{
    CharacterDatabase.PExecute(
        "DELETE FROM character_companion WHERE player_guid=%u", playerGuid);
}

// Group helper: disband the player's party if this was the last companion.
// Call AFTER the companion has already been removed from s_companions.
static void TryDisbandGroupIfLast(Player* owner, ObjectGuid removedGuid)
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
//  Role system - drives which emotes/AI each archetype uses
// ============================================================
enum AmbientRole : uint8
{
    AMBIENT_WARRIOR     = 0,  // Plate melee DPS (Warrior)
    AMBIENT_MAGE        = 1,  // Caster DPS (Mage)
    AMBIENT_HEALER      = 2,  // Healer (Holy Pala / Resto Shaman / Resto Druid / Mistweaver / Holy Priest)
    AMBIENT_HUNTER      = 3,  // Ranged physical DPS (Hunter)
    AMBIENT_ROGUE       = 4,  // Agile melee DPS (Rogue / Demon Hunter)
    AMBIENT_DEFAULT     = 5,  // Generic melee fallback
    AMBIENT_TANK        = 6,  // Dedicated tank (DK / Prot Pala / Bear Druid / Brewmaster Monk)
};

// Hybrid specs (Paladin, Druid, Monk) randomly choose at spawn.
// Called once in the constructor initializer list.
static AmbientRole GetRoleForEntry(uint32 entry)
{
    switch (entry)
    {
        // --- Alliance ---
        case 9500080: return AMBIENT_WARRIOR;   // Warrior
        case 9500081: return (urand(0,1)) ? AMBIENT_TANK : AMBIENT_HEALER; // Prot or Holy Paladin
        case 9500082: return AMBIENT_MAGE;      // Mage
        case 9500083: return AMBIENT_HUNTER;    // Hunter
        case 9500084: return AMBIENT_ROGUE;     // Rogue
        // --- Horde ---
        case 9500085: return AMBIENT_WARRIOR;   // Warrior
        case 9500086: return AMBIENT_HEALER;    // Resto Shaman
        case 9500087: return AMBIENT_HUNTER;    // Hunter (Troll)
        case 9500088: return AMBIENT_MAGE;      // Mage (Blood Elf)
        case 9500089: return AMBIENT_ROGUE;     // Rogue
        // --- Neutral ---
        case 9500090: return AMBIENT_TANK;      // Death Knight (always tank)
        case 9500091: return AMBIENT_ROGUE;     // Demon Hunter (agile melee, like Rogue)
        case 9500092: return (urand(0,9) < 4) ? AMBIENT_TANK : AMBIENT_HEALER; // Bear or Resto Druid
        case 9500093: return (urand(0,1)) ? AMBIENT_TANK : AMBIENT_HEALER;     // Brewmaster or Mistweaver Monk
        case 9500094: return AMBIENT_HEALER;    // Priest (always healer)
        default:      return AMBIENT_DEFAULT;
    }
}

static uint32 GetCombatEmote(AmbientRole role)
{
    switch (role)
    {
        case AMBIENT_WARRIOR: case AMBIENT_ROGUE: return EMOTE_ONESHOT_ATTACK1H;
        case AMBIENT_TANK:                    return EMOTE_ONESHOT_ATTACK1H;
        case AMBIENT_HUNTER:                  return EMOTE_ONESHOT_ATTACK_BOW;
        case AMBIENT_MAGE:                    return EMOTE_ONESHOT_SPELL_CAST_W_SOUND;
        case AMBIENT_HEALER:                  return EMOTE_ONESHOT_SPELL_CAST;
        default:                              return EMOTE_ONESHOT_ATTACK1H;
    }
}

// Short display label for each role — shown in gossip hire text
static const char* RoleName(AmbientRole role)
{
    switch (role)
    {
        case AMBIENT_TANK:    return "Tank";
        case AMBIENT_HEALER:  return "Healer";
        default:              return "DPS";
    }
}

// ============================================================
//  Tier-2 Raid Roster  (persistent, manually curated)
//  Defined here so AmbientRole is fully in scope.
// ============================================================
struct RosterEntry
{
    std::string displayName;
    AmbientRole role     = AMBIENT_DEFAULT;
    uint8       level    = 1;
    bool        isManual = true;
};

static std::mutex                                     s_rosterMutex;
static std::map<ObjectGuid, std::vector<RosterEntry>> s_raidRoster;
static std::map<ObjectGuid, uint32>                   s_extraTankOverride;

// ---- Roster DB helpers ----
static void DB_AddToRoster(uint32 playerGuid, std::string const& name, uint8 rosterRole, uint8 rosterLevel)
{
    std::string esc = name;
    CharacterDatabase.EscapeString(esc);
    CharacterDatabase.PExecute(
        "REPLACE INTO character_raid_roster (player_guid, name, role, level) VALUES (%u, '%s', %u, %u)",
        playerGuid, esc.c_str(), (uint32)rosterRole, (uint32)rosterLevel);
}

static void DB_ClearRoster(uint32 playerGuid)
{
    CharacterDatabase.PExecute(
        "DELETE FROM character_raid_roster WHERE player_guid=%u", playerGuid);
}

static uint32 GetRosterCount(ObjectGuid playerGuid)
{
    std::lock_guard<std::mutex> lk(s_rosterMutex);
    auto it = s_raidRoster.find(playerGuid);
    return (it != s_raidRoster.end()) ? (uint32)it->second.size() : 0;
}

static uint32 GetExtraTanks(ObjectGuid playerGuid)
{
    std::lock_guard<std::mutex> lk(s_rosterMutex);
    auto it = s_extraTankOverride.find(playerGuid);
    return (it != s_extraTankOverride.end()) ? it->second : 0;
}

// ---- Instance size requirements ----
// Returns total group cap + minimum tank/healer counts for auto-fill.
struct InstanceSize { uint32 total; uint32 minTanks; uint32 minHealers; };

static InstanceSize GetInstanceSize(Map* map)
{
    // Difficulty integral values in Legion:
    //  3=10N  5=10HC  4=25N  6=25HC  9=40-man  16=Mythic-20  14=flex
    if (!map->IsRaid())
    {
        InstanceSize s; s.total = 5;  s.minTanks = 1; s.minHealers = 1; return s;
    }
    uint32 diff = (uint32)map->GetDifficultyID();
    if (diff == 3 || diff == 5) { InstanceSize s; s.total = 10; s.minTanks = 2; s.minHealers = 2; return s; }
    if (diff == 4 || diff == 6) { InstanceSize s; s.total = 25; s.minTanks = 2; s.minHealers = 6; return s; }
    if (diff == 9)              { InstanceSize s; s.total = 40; s.minTanks = 3; s.minHealers = 8; return s; }
    if (diff == 16)             { InstanceSize s; s.total = 20; s.minTanks = 2; s.minHealers = 4; return s; }
    InstanceSize s; s.total = 25; s.minTanks = 2; s.minHealers = 5; return s;
}

// Pick the best ambient NPC entry to represent a given role for auto-fill summons
static uint32 EntryForRole(AmbientRole role)
{
    switch (role)
    {
        case AMBIENT_TANK:    return 9500090;  // Death Knight
        case AMBIENT_HEALER:  return 9500086;  // Shaman
        case AMBIENT_MAGE:    return 9500082;
        case AMBIENT_HUNTER:  return 9500083;
        case AMBIENT_ROGUE:   return 9500084;
        case AMBIENT_WARRIOR: return 9500080;
        default:              return 9500085;
    }
}

// ============================================================
//  Mount display ID pools for rider NPCs (B-section, mount riders)
//  Applied to level 20+ ambient NPCs with chance scaling by level:
//   Lv 20-39: 30%   Lv 40-59: 45%   Lv 60-79: 60%   Lv 80+: 75%
// ============================================================
static const uint32 MOUNTS_ALLIANCE[] = {
    2411,   // Brown Horse
    2414,   // Grey Horse
    2417,   // Pinto Horse
    2418,   // Palomino
    4802,   // Brown Ram      (Dwarf)
    4805,   // White Ram       (Dwarf)
    10045,  // Spotted Night Saber (Night Elf)
    10046,  // Striped Night Saber (Night Elf)
    4491,   // White Mechanostrider (Gnome)
    19481,  // Brown Elekk    (Draenei)
};
static constexpr uint32 MOUNTS_ALLIANCE_COUNT = 10;

static const uint32 MOUNTS_HORDE[] = {
    2607,   // Brown Wolf
    2610,   // Timber Wolf
    2612,   // Large Gray Wolf
    9470,   // Black War Wolf
    6467,   // Gray Kodo      (Tauren)
    6471,   // Green Kodo     (Tauren)
    9345,   // Emerald Raptor (Troll)
    22723,  // Red Hawkstrider (Blood Elf)
    9471,   // Red Wolf (high level)
};
static constexpr uint32 MOUNTS_HORDE_COUNT = 9;

static const uint32 MOUNTS_NEUTRAL[] = {
    25162,  // Deathcharger   (Death Knight)
    4080,   // Felsteed       (Warlock-style)
    2411,   // Brown Horse
    2607,   // Brown Wolf
    10045,  // Night Saber
    6467,   // Gray Kodo
};
static constexpr uint32 MOUNTS_NEUTRAL_COUNT = 6;

// Epic mounts for 60+ NPCs (rarer, more impressive)
static const uint32 MOUNTS_EPIC[] = {
    14349,  // Blue Mechanostrider (epic)
    13334,  // Dreadsteed (epic warlock)
    9470,   // Black War Wolf
    2411,   // Black War Horse (max-speed horse)
    2607,   // Black Wolf
    10051,  // Epic Swift Frostsaber
};
static constexpr uint32 MOUNTS_EPIC_COUNT = 6;

static uint32 GetMountDisplayId(uint32 npcEntry, uint8 level)
{
    if (level < 20) return 0;

    uint32 roll    = urand(0, 99);
    uint32 chance  = (level >= 80) ? 75 : (level >= 60) ? 60 : (level >= 40) ? 45 : 30;
    if (roll >= chance) return 0;

    // High-level NPCs (80+) occasionally get the epic pool
    if (level >= 80 && urand(0, 2) == 0)
        return MOUNTS_EPIC[urand(0, MOUNTS_EPIC_COUNT - 1)];

    // Alliance faction
    if (npcEntry >= 9500080 && npcEntry <= 9500084)
        return MOUNTS_ALLIANCE[urand(0, MOUNTS_ALLIANCE_COUNT - 1)];

    // Horde faction
    if (npcEntry >= 9500085 && npcEntry <= 9500089)
        return MOUNTS_HORDE[urand(0, MOUNTS_HORDE_COUNT - 1)];

    // Neutral — Death Knight always gets Deathcharger if available
    if (npcEntry == 9500090) return 25162;  // Deathcharger
    return MOUNTS_NEUTRAL[urand(0, MOUNTS_NEUTRAL_COUNT - 1)];
}

// ============================================================
//  AI state machine
// ============================================================
enum AmbientState : uint8
{
    STATE_IDLE     = 0,
    STATE_WANDER   = 1,
    STATE_ACTIVITY = 2,
    STATE_HUNT     = 3,
    STATE_SOCIAL   = 4,
    STATE_WORK     = 5,  // P1: interacting with a forge, anvil, or cooking fire
};

struct npc_ambient_aiAI : public ScriptedAI
{
    npc_ambient_aiAI(Creature* c) : ScriptedAI(c),
        _state(STATE_IDLE),
        _timer(urand(1000, 3000)),
        _role(GetRoleForEntry(c->GetEntry())),
        _homeX(c->GetPositionX()),
        _homeY(c->GetPositionY()),
        _homeZ(c->GetPositionZ()),
        _combatEmoteTimer(0),
        _healTimer(0),
        _spellTimer(0),
        _tauntTimer(0),
        _aoeThreatTimer(0),
        _shieldWallTimer(0),
        _healBigCdTimer(0),
        _interruptTimer(0),
        _interruptCheckTimer(0),
        _prioritySwitchTimer(urand(0, 3000)),
        _addScanTimer(urand(0, 3000)),
        _speechTimer(urand(0, 120000)),
        _moveDone(true),
        _isReplica(false),
        _executeActive(false),
        _fightingBoss(false),
        _ageArchetype(0),
        _isCompanion(false),
        _ownerGuid(),
        _formDist(3.0f),
        _formAngle(float(M_PI)),
        _leashWhisperTimer(0),
        _workEmoteTimer(0),
        _workObjectType(0)
    {
        // Ambient NPCs must never auto-engage; HUNT state handles targeting explicitly.
        me->SetReactState(REACT_PASSIVE);

        // B14: Age archetype — Young(0), Adult(1), Elder(2) — 1/3 : 1/2 : 1/6
        if (!_isCompanion)
        {
            uint32 agRoll = urand(0, 5);
            _ageArchetype = (agRoll == 0) ? 2 : (agRoll <= 2) ? 0 : 1;
            if (_ageArchetype == 0)       // Young
                me->SetObjectScale(frand(0.90f, 0.95f));
            else if (_ageArchetype == 2)  // Elder
                me->SetObjectScale(frand(1.05f, 1.10f));
        }
    }

    void Reset() override
    {
        _state        = STATE_IDLE;
        _timer        = urand(2000, 5000);
        _moveDone     = true;
        _executeActive = false;
        _fightingBoss  = false;
        // Ambient NPCs must never auto-aggro; only HUNT state calls AttackStart.
        // Companion mode switches to REACT_DEFENSIVE in HireCompanion.
        if (!_isCompanion)
            me->SetReactState(REACT_PASSIVE);
    }

    void EnterCombat(Unit* who) override
    {
        if (!who || !_isCompanion)
            return;
        // H8: Boss awareness — tighten response timers when fighting a dungeon boss
        if (Creature* c = who->ToCreature())
            _fightingBoss = c->IsDungeonBoss();
        else
            _fightingBoss = false;
    }

    void MovementInform(uint32 type, uint32 /*id*/) override
    {
        if (type == POINT_MOTION_TYPE)
        {
            _moveDone = true;
            // Shorten timer so NPC picks next state within 1-2s after arriving
            // instead of standing frozen for the full 6-14s wander timeout
            if (_state == STATE_WANDER)
                _timer = urand(800, 2000);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        // ─── Companion mode ──────────────────────────────────────────
        if (_isCompanion)
        {
            Player* owner = ObjectAccessor::GetPlayer(*me, _ownerGuid);
            if (!owner || !owner->IsAlive())
            {
                _isCompanion = false;
                _ownerGuid   = ObjectGuid::Empty;
                UnregisterCompanion(me->GetGUID());
                Reset();
                return;
            }

            // Companion in combat — fight
            if (me->IsInCombat())
            {
                if (!me->GetVictim())
                {
                    if (Unit* t = owner->GetVictim())
                        AttackStart(t);
                    return;
                }

            // H2: Healer — 4-tier priority healing (triage/critical/normal/group-CD)
            if (_role == AMBIENT_HEALER)
            {
                // Big-CD ticks independently
                if (_healBigCdTimer > diff)
                    _healBigCdTimer -= diff;
                else
                    _healBigCdTimer = 0;

                if (_healTimer <= diff)
                {
                    // Find lowest-HP ally within 60y (owner + companions of same owner)
                    Unit*  healTarget = nullptr;
                    float  lowestPct  = 100.0f;
                    if (owner->IsAlive() && owner->GetHealthPct() < lowestPct)
                    {
                        lowestPct  = owner->GetHealthPct();
                        healTarget = owner;
                    }
                    for (uint32 ce = 9500080; ce <= 9500094; ++ce)
                    {
                        std::list<Creature*> clist;
                        GetCreatureListWithEntryInGrid(clist, me, ce, 60.f);
                        for (Creature* cm : clist)
                        {
                            if (!cm->IsAlive() || cm == me) continue;
                            CompanionData ccd;
                            if (!GetCompanionData(cm->GetGUID(), ccd) || ccd.ownerGuid != _ownerGuid) continue;
                            if (cm->GetHealthPct() < lowestPct)
                            {
                                lowestPct  = cm->GetHealthPct();
                                healTarget = cm;
                            }
                        }
                    }

                    if (healTarget && lowestPct < 20.0f)
                    {
                        // TIER 1: Triage — emergency heal
                        uint32 ha = uint32(healTarget->GetMaxHealth() * 0.35f);
                        healTarget->SetHealth(std::min(healTarget->GetHealth() + ha, healTarget->GetMaxHealth()));
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SPELL_CAST);
                        _healTimer = _fightingBoss ? urand(800,  1500) : urand(1500, 2500);
                        return;
                    }
                    else if (healTarget && lowestPct < 40.0f)
                    {
                        // TIER 2: Critical — large single-target heal
                        uint32 ha = uint32(healTarget->GetMaxHealth() * 0.20f);
                        healTarget->SetHealth(std::min(healTarget->GetHealth() + ha, healTarget->GetMaxHealth()));
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SPELL_CAST);
                        _healTimer = _fightingBoss ? urand(1500, 2500) : urand(2500, 4000);
                        return;
                    }
                    else if (healTarget && lowestPct < 65.0f)
                    {
                        // TIER 3: Normal — steady heal, fighter continues attacking
                        uint32 ha = uint32(healTarget->GetMaxHealth() * 0.12f);
                        healTarget->SetHealth(std::min(healTarget->GetHealth() + ha, healTarget->GetMaxHealth()));
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SPELL_CAST);
                        _healTimer = _fightingBoss ? urand(2500, 4000) : urand(4000, 6000);
                    }
                    else
                        _healTimer = urand(1000, 2500); // nothing urgent — recheck soon

                    // TIER 4: Group-heal big CD when party HP is below threshold
                    if (_healBigCdTimer == 0 && healTarget && lowestPct < 65.0f)
                    {
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SPELL_CAST_W_SOUND);
                        uint32 bigHeal = uint32(owner->GetMaxHealth() * 0.15f);
                        owner->SetHealth(std::min(owner->GetHealth() + bigHeal, owner->GetMaxHealth()));
                        for (uint32 ce2 = 9500080; ce2 <= 9500094; ++ce2)
                        {
                            std::list<Creature*> cl2;
                            GetCreatureListWithEntryInGrid(cl2, me, ce2, 60.f);
                            for (Creature* cm2 : cl2)
                            {
                                if (!cm2->IsAlive() || cm2 == me) continue;
                                CompanionData ccd2;
                                if (!GetCompanionData(cm2->GetGUID(), ccd2) || ccd2.ownerGuid != _ownerGuid) continue;
                                uint32 mh = uint32(cm2->GetMaxHealth() * 0.15f);
                                cm2->SetHealth(std::min(cm2->GetHealth() + mh, cm2->GetMaxHealth()));
                            }
                        }
                        _healBigCdTimer = 60000;
                    }
                }
                else
                    _healTimer -= diff;
            }

            // H1/G3: Tank — single-target threat spike + AoE threat + defensive CDs
            if (_role == AMBIENT_TANK && me->GetVictim())
            {
                // Single-target threat spike
                if (_tauntTimer <= diff)
                {
                    float bonus = _fightingBoss ? 0.20f : 0.10f;
                    me->AddThreat(me->GetVictim(), me->GetMaxHealth() * bonus);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                    _tauntTimer = urand(4000, 7000);
                }
                else
                    _tauntTimer -= diff;

                // H1a: AoE threat on all nearby hostiles every 8-10s
                if (_aoeThreatTimer <= diff)
                {
                    std::list<Creature*> nearHostile;
                    me->GetCreatureListInGrid(nearHostile, 12.f);
                    for (Creature* nc : nearHostile)
                    {
                        if (nc && nc->IsAlive() && nc->IsHostileTo(me) && nc != me->GetVictim())
                            me->AddThreat(nc, me->GetMaxHealth() * 0.05f);
                    }
                    _aoeThreatTimer = _fightingBoss ? urand(5000, 7000) : urand(7000, 10000);
                }
                else
                    _aoeThreatTimer -= diff;

                // H1b: Defensive CD at low HP
                if (_shieldWallTimer <= diff)
                {
                    float hpPct = me->GetHealthPct();
                    if (hpPct < 20.0f)
                    {
                        // Last Stand: emergency self-heal (15% max HP)
                        me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                        me->PlayDistanceSound(5481);
                        uint32 lsBonus = uint32(me->GetMaxHealth() * 0.15f);
                        me->SetHealth(std::min(me->GetHealth() + lsBonus, me->GetMaxHealth()));
                        _shieldWallTimer = 60000;
                    }
                    else if (hpPct < 40.0f)
                    {
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                        _shieldWallTimer = 30000;
                    }
                }
                else
                    _shieldWallTimer -= diff;
            }

            // A2: Mage — periodic triggered fireball on top of melee (faster during execute)
            if (_role == AMBIENT_MAGE && me->GetVictim())
            {
                if (_spellTimer <= diff)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_SPELL_CAST_W_SOUND);
                    me->CastSpell(me->GetVictim(), 133, true); // Fireball (triggered)
                    _spellTimer = _executeActive ? urand(1200, 2000) : urand(2500, 4000);
                }
                else
                    _spellTimer -= diff;
            }

            // A2: Hunter — ranged bow emote between melee swings (faster during execute)
            if (_role == AMBIENT_HUNTER && me->GetVictim())
            {
                if (_spellTimer <= diff)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_BOW);
                    _spellTimer = _executeActive ? urand(800, 1500) : urand(1800, 3000);
                }
                else
                    _spellTimer -= diff;
            }

            // H3: Interrupt — kick enemy spell casts (all roles, role-based CD)
            if (me->GetVictim())
            {
                if (_interruptTimer > diff)
                    _interruptTimer -= diff;
                else
                    _interruptTimer = 0;

                if (_interruptTimer == 0)
                {
                    if (_interruptCheckTimer <= diff)
                    {
                        _interruptCheckTimer = _fightingBoss ? 1000 : 2000;
                        if (me->GetVictim()->IsNonMeleeSpellCast(false))
                        {
                            me->GetVictim()->InterruptSpell(CURRENT_GENERIC_SPELL, false, true);
                            me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
                            if      (_role == AMBIENT_ROGUE)                              _interruptTimer = 15000;
                            else if (_role == AMBIENT_HEALER)                             _interruptTimer = 12000;
                            else if (_role == AMBIENT_TANK || _role == AMBIENT_WARRIOR)   _interruptTimer = 15000;
                            else if (_role == AMBIENT_MAGE || _role == AMBIENT_HUNTER)    _interruptTimer = 24000;
                            else                                                           _interruptTimer = 20000;
                        }
                    }
                    else
                        _interruptCheckTimer -= diff;
                }
            }

            // H4: Priority target — DPS switches to any mob attacking the owner
            if (_role != AMBIENT_TANK && _role != AMBIENT_HEALER)
            {
                if (_prioritySwitchTimer <= diff)
                {
                    _prioritySwitchTimer = _fightingBoss ? 2000 : 3000;
                    std::list<Creature*> nearby;
                    me->GetCreatureListInGrid(nearby, 40.f);
                    for (Creature* hc : nearby)
                    {
                        if (!hc || !hc->IsAlive() || !hc->IsHostileTo(me)) continue;
                        Unit* hcVictim = hc->GetVictim();
                        if (!hcVictim) continue;
                        if (hcVictim->GetGUID() == _ownerGuid && hc != me->GetVictim())
                        {
                            me->GetMotionMaster()->Clear();
                            AttackStart(hc);
                            break;
                        }
                    }
                }
                else
                    _prioritySwitchTimer -= diff;
            }

            // H5: Add scan — Tank/Warrior intercepts any mob attacking a companion or owner
            if (_role == AMBIENT_TANK || _role == AMBIENT_WARRIOR)
            {
                if (_addScanTimer <= diff)
                {
                    _addScanTimer = _fightingBoss ? 2000 : 3000;
                    std::list<Creature*> addon;
                    me->GetCreatureListInGrid(addon, 20.f);
                    for (Creature* ac : addon)
                    {
                        if (!ac || !ac->IsAlive() || !ac->IsHostileTo(me) || ac == me->GetVictim()) continue;
                        Unit* acVictim = ac->GetVictim();
                        if (!acVictim) continue;
                        bool ownerThreat = (acVictim->GetGUID() == _ownerGuid);
                        bool allyThreat  = false;
                        CompanionData acd;
                        if (GetCompanionData(acVictim->GetGUID(), acd) && acd.ownerGuid == _ownerGuid)
                            allyThreat = true;
                        if (ownerThreat || allyThreat)
                        {
                            me->AddThreat(ac, 999999.f);
                            me->GetMotionMaster()->Clear();
                            AttackStart(ac);
                            me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                            break;
                        }
                    }
                }
                else
                    _addScanTimer -= diff;
            }

            // H6: Execute phase — signal frantic burst when enemy drops below 20% HP
            if (me->GetVictim())
            {
                if (!_executeActive && me->GetVictim()->GetHealthPct() < 20.0f)
                {
                    _executeActive = true;
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                }
                else if (_executeActive && me->GetVictim()->GetHealthPct() >= 25.0f)
                    _executeActive = false;
            }

            if (_combatEmoteTimer <= diff)
            {
                me->HandleEmoteCommand(GetCombatEmote(_role));
                // Execute phase: faster emote cadence signals increased aggression
                _combatEmoteTimer = _executeActive ? urand(800, 1500) : urand(2200, 4000);
            }
            else
                _combatEmoteTimer -= diff;
            DoMeleeAttackIfReady();
            return;
        }

        // A4 / G3: Tank and Warrior rush to intercept threats targeting the owner.
        // Rogues and DH are agile DPS — they assist but don't intercept.
        if ((_role == AMBIENT_WARRIOR || _role == AMBIENT_TANK) && owner->IsInCombat())
        {
            if (Unit* t = owner->GetVictim())
            {
                me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                me->GetMotionMaster()->Clear();
                AttackStart(t);
                return;
            }
        }

        // Assist owner if they pulled something (all roles)
        if (owner->IsInCombat())
        {
            if (Unit* t = owner->GetVictim())
            {
                me->GetMotionMaster()->Clear();
                AttackStart(t);
                return;
            }
        }

        // A1: Healer — top up the most-wounded ally (owner OR fellow companions) between fights
        if (_role == AMBIENT_HEALER)
        {
            if (_healTimer <= diff)
            {
                // Find ally below 95% HP — lowest first
                Unit*  healTarget = nullptr;
                float  lowestPct  = 95.0f;
                if (owner->IsAlive() && owner->GetHealthPct() < lowestPct)
                {
                    lowestPct  = owner->GetHealthPct();
                    healTarget = owner;
                }
                for (uint32 ce = 9500080; ce <= 9500094; ++ce)
                {
                    std::list<Creature*> clist;
                    GetCreatureListWithEntryInGrid(clist, me, ce, 40.f);
                    for (Creature* cm : clist)
                    {
                        if (!cm->IsAlive() || cm == me) continue;
                        CompanionData ccd;
                        if (!GetCompanionData(cm->GetGUID(), ccd) || ccd.ownerGuid != _ownerGuid) continue;
                        if (cm->GetHealthPct() < lowestPct)
                        {
                            lowestPct  = cm->GetHealthPct();
                            healTarget = cm;
                        }
                    }
                }
                if (healTarget)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_SPELL_CAST);
                    uint32 healAmt = uint32(healTarget->GetMaxHealth() * 0.10f);
                    healTarget->SetHealth(std::min(healTarget->GetHealth() + healAmt, healTarget->GetMaxHealth()));
                    _healTimer = urand(3500, 6000);
                }
                else
                    _healTimer = urand(2000, 3500);  // party is full — recheck soon
            }
            else
                _healTimer -= diff;
        }
        if (_leashWhisperTimer > diff)
            _leashWhisperTimer -= diff;
        else
            _leashWhisperTimer = 0;

        // Leash-back: if companion has drifted too far, teleport them home
        float dist2Owner = me->GetDistance(owner);
        if (dist2Owner > 80.f)
        {
            if (_leashWhisperTimer == 0)
            {
                std::string dn = _myCompanionName.empty() ? me->GetName() : _myCompanionName;
                ChatHandler(owner->GetSession()).PSendSysMessage(
                    "|cff888888[Companion]|r %s is finding their way back...", dn.c_str());
                _leashWhisperTimer = 30000;
            }
            me->NearTeleportTo(
                owner->GetPositionX() + frand(-3.f, 3.f),
                owner->GetPositionY() + frand(-3.f, 3.f),
                owner->GetPositionZ(), owner->GetOrientation());
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveFollow(owner, _formDist, _formAngle);
            return;
        }

        // Follow owner (re-engage after combat or if fallen behind)
        if (dist2Owner > 5.0f)
        {
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveFollow(owner, _formDist, _formAngle);
        }
        return;
    }
    // ─── End companion mode ───────────────────────────────────────

        // If actually in melee combat, fight and show combat emotes
        if (me->IsInCombat())
        {
            if (!me->GetVictim())
            {
                _state = STATE_IDLE;
                _timer = urand(2000, 4000);
                return;
            }
            if (_combatEmoteTimer <= diff)
            {
                me->HandleEmoteCommand(GetCombatEmote(_role));
                _combatEmoteTimer = urand(2200, 4000);
            }
            else
                _combatEmoteTimer -= diff;

            DoMeleeAttackIfReady();
            return;
        }

        // B1: Speech cooldown decrement
        if (_speechTimer > diff) _speechTimer -= diff;
        else                     _speechTimer  = 0;

        // P1: Sustain work emotes while the NPC is busy at a world object
        if (_state == STATE_WORK)
        {
            if (_workEmoteTimer <= diff)
            {
                if (_workObjectType == 0)
                    me->HandleEmoteCommand(EMOTE_ONESHOT_WORK);         // forge/anvil hammering
                else
                    me->HandleEmoteCommand(EMOTE_ONESHOT_EAT_NO_SHEATHE); // cooking at fire
                _workEmoteTimer = urand(2500, 4500);
            }
            else
                _workEmoteTimer -= diff;
        }

        // State timer countdown
        if (_timer <= diff)
        {
            _timer = 0;
            _SelectNextState();
        }
        else
            _timer -= diff;
    }

    // ── Companion state (public so gossip handler can access) ──────────────
    bool        _isCompanion;
    ObjectGuid  _ownerGuid;
    std::string _myCompanionName;  // name cached at hire, used in kill messages
    AmbientRole GetRole() const { return _role; }

    void HireCompanion(Player* player, uint32 restoreLevel = 0, uint32 restoreXp = 0, bool isReplica = false)
    {
        if (_isCompanion)
            return;

        // A3: Hard cap — player cannot have more than 4 companions at once
        if (restoreLevel == 0) // skip cap check during login restore
        {
            std::lock_guard<std::mutex> lk(s_companionMutex);
            uint32 activeCount = 0;
            for (auto const& kv : s_companions)
                if (kv.second.ownerGuid == player->GetGUID())
                    ++activeCount;
            if (activeCount >= 4)
            {
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "|cffff4444[Companion]|r You already have 4 companions. Dismiss one first.");
                return;
            }
        }

        _isCompanion       = true;
        _isReplica         = isReplica;
        _ownerGuid         = player->GetGUID();
        _myCompanionName   = me->GetName();   // cache NOW - template name already overridden by SetName()
        uint32 lvl         = restoreLevel > 0 ? restoreLevel : (uint32)me->getLevel();

        RegisterCompanion(me->GetGUID(), player->GetGUID(), lvl, _myCompanionName);
        // Mark replica flag inside CompanionData so FlushAndCleanReplicas can save progress on map exit
        if (isReplica)
        {
            std::lock_guard<std::mutex> lk(s_companionMutex);
            auto ri = s_companions.find(me->GetGUID());
            if (ri != s_companions.end())
                ri->second.isReplica = true;
        }

        // Inject restored XP if this is a login-restore
        if (restoreXp > 0 && restoreLevel > 0)
        {
            std::lock_guard<std::mutex> lk(s_companionMutex);
            auto it = s_companions.find(me->GetGUID());
            if (it != s_companions.end())
            {
                it->second.xp       = restoreXp;
                it->second.xpNeeded = CompanionXpForLevel(lvl);
            }
        }

        me->SetReactState(REACT_DEFENSIVE);
        me->GetMotionMaster()->Clear();

        // Formation follow: spread companions so they don't all stack at the same point.
        // Slot 0 = directly behind, 1 = behind-left, 2 = behind-right, 3 = far-left.
        {
            static const float FORM_DIST[]  = { 2.5f, 3.1f, 3.1f, 3.7f };
            static const float FORM_ANGLE[] = {
                float(M_PI),
                float(M_PI) + 0.42f,
                float(M_PI) - 0.42f,
                float(M_PI) + 0.85f
            };
            uint32 slotIdx = 0;
            {
                std::lock_guard<std::mutex> lk(s_companionMutex);
                for (auto const& kv : s_companions)
                    if (kv.second.ownerGuid == player->GetGUID() && kv.first != me->GetGUID())
                        ++slotIdx;
            }
            uint32 si  = slotIdx < 4u ? slotIdx : slotIdx % 4u;
            _formDist  = FORM_DIST[si];
            _formAngle = FORM_ANGLE[si];
        }
        me->GetMotionMaster()->MoveFollow(player, _formDist, _formAngle);

        // Guardian mode: SetCreatorGUID links the NPC to the player server-side
        // WITHOUT occupying the pet frame. This means no conflict with Hunter pets,
        // Warlock demons, or any other class pet. The NPC simply follows and assists.
        me->SetCreatorGUID(player->GetGUID());

        // Create a real WoW party group so group loot/XP rules apply and
        // the blue [Party] channel activates. Only create if not already grouped.
        if (!player->GetGroup())
        {
            Group* grp = new Group();
            if (grp->Create(player))
                sGroupMgr->AddGroup(grp);
            else
                delete grp;
        }

        // Save to DB (skip on restore to avoid overwriting the restored XP)
        if (restoreLevel == 0)
            DB_SaveCompanion(player->GetGUID().GetCounter(), me, lvl, 0);
    }

    void DismissCompanion()
    {
        if (!_isCompanion)
            return;
        Player* owner     = ObjectAccessor::GetPlayer(*me, _ownerGuid);
        ObjectGuid myGuid = me->GetGUID();
        if (owner)
            DB_DeleteCompanion(owner->GetGUID().GetCounter(), _myCompanionName);
        UnregisterCompanion(myGuid);
        _isCompanion     = false;
        _ownerGuid       = ObjectGuid::Empty;
        _myCompanionName.clear();
        me->SetCreatorGUID(ObjectGuid::Empty);
        me->SetReactState(REACT_PASSIVE);
        me->GetMotionMaster()->Clear();
        // Disband the group if this was the last companion
        TryDisbandGroupIfLast(owner, myGuid);
        Reset();
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (_isCompanion)
        {
            Player*    owner  = ObjectAccessor::GetPlayer(*me, _ownerGuid);
            ObjectGuid myGuid = me->GetGUID();

            // A5: Flavour death message
            if (owner)
            {
                static const char* DEATH_LINES[] = {
                    "%s has fallen in battle!",
                    "%s fought to the last \xe2\x80\x94 may they be remembered.",
                    "You have lost %s.",
                    "%s gave everything. Honor their sacrifice.",
                    "No... %s is gone.",
                };
                std::string dname = _myCompanionName.empty() ? me->GetName() : _myCompanionName;
                ChatHandler(owner->GetSession()).PSendSysMessage(
                    (std::string("|cffff4444[Companion]|r ") + DEATH_LINES[urand(0, 4)]).c_str(),
                    dname.c_str());
            }

            // Instance replicas preserve the DB row so the open-world restore still works
            if (owner && !_isReplica)
                DB_DeleteCompanion(owner->GetGUID().GetCounter(), _myCompanionName);
            UnregisterCompanion(myGuid);
            _isCompanion     = false;
            _isReplica       = false;
            _ownerGuid       = ObjectGuid::Empty;
            _myCompanionName.clear();
            TryDisbandGroupIfLast(owner, myGuid);
        }
        else
        {
            // B8: Non-companion corpse cleanup — body despawns after 8 seconds
            me->DespawnOrUnsummon(8000);
        }
    }

    // When the companion lands a killing blow:
    //   • owner player gets XP
    //   • this companion and ALL other companions of the same owner get XP
    // Creatures cannot join WoW Groups, so we manually share XP here.
    void KilledUnit(Unit* killed) override
    {
        if (!_isCompanion || !killed)
            return;

        Player* owner = ObjectAccessor::GetPlayer(*me, _ownerGuid);
        if (!owner)
            return;

        uint32 xpGain = CompanionXpGain((uint32)killed->getLevel());

        // 1. Award XP to the owner player
        owner->GiveXP(xpGain, nullptr);

        // 2. Award XP and kill count to THIS companion
        uint32 myNewLevel = 0;
        bool   myLvlUp   = AwardCompanionXP(me->GetGUID(), xpGain, myNewLevel);
        uint32 myKills   = 0;
        IncrementKillCount(me->GetGUID(), myKills);
        std::string killerName = _myCompanionName.empty() ? me->GetName() : _myCompanionName;
        ChatHandler(owner->GetSession()).PSendSysMessage(
            "|cff00ccff[Party]|r %s scored a kill \xe2\x80\x94 all companions gained %u XP!",
            killerName.c_str(), xpGain);
        if (myLvlUp)
        {
            me->SetLevel(myNewLevel);
            me->PlayDistanceSound(5906); // A6: level-up sound
            ChatHandler(owner->GetSession()).PSendSysMessage(
                "|cffffd700[Companion]|r %s reached level %u!",
                killerName.c_str(), myNewLevel);
        }
        // A9: Title milestone announcement
        {
            static const uint32 TITLE_THRESHOLDS[] = { 10, 50, 100, 250, 500 };
            for (uint32 t : TITLE_THRESHOLDS)
                if (myKills == t)
                    ChatHandler(owner->GetSession()).PSendSysMessage(
                        "|cffffd700[Companion]|r %s has earned the title \"%s\"!",
                        killerName.c_str(), GetCompanionTitle(myKills));
        }

        // 3. Award XP to ALL other companions of the same owner
        std::vector<ObjectGuid> siblings;
        {
            std::lock_guard<std::mutex> lk(s_companionMutex);
            for (auto const& kv : s_companions)
                if (kv.first != me->GetGUID() &&
                    kv.second.ownerGuid == _ownerGuid)
                    siblings.push_back(kv.first);
        }
        for (ObjectGuid sibGuid : siblings)
        {
            Creature* sib = ObjectAccessor::GetCreature(*me, sibGuid);
            if (!sib || !sib->IsAlive())
                continue;
            uint32 sibNewLevel = 0;
            bool   sibLvlUp   = AwardCompanionXP(sibGuid, xpGain, sibNewLevel);
            CompanionData sibCd;
            std::string sibName = (GetCompanionData(sibGuid, sibCd) && !sibCd.displayName.empty())
                ? sibCd.displayName : sib->GetName();
            ChatHandler(owner->GetSession()).PSendSysMessage(
                "|cff00ccff[Party]|r %s gained %u XP  (Lv %u)",
                sibName.c_str(), xpGain, sibNewLevel);
            if (sibLvlUp)
            {
                sib->SetLevel(sibNewLevel);
                ChatHandler(owner->GetSession()).PSendSysMessage(
                    "|cffffd700[Companion]|r %s reached level %u!",
                    sibName.c_str(), sibNewLevel);
            }
        }
    }

private:
    AmbientState _state;
    uint32       _timer;
    AmbientRole  _role;
    float        _homeX, _homeY, _homeZ;
    uint32       _combatEmoteTimer;
    uint32       _healTimer;          // H2: healer heal cooldown
    uint32       _healBigCdTimer;     // H2: group-heal big CD (60s)
    uint32       _spellTimer;         // A2: mage/hunter spell cooldown
    uint32       _tauntTimer;         // H1: single-target threat spike
    uint32       _aoeThreatTimer;     // H1: AoE threat on nearby hostiles
    uint32       _shieldWallTimer;    // H1: defensive CD cooldown
    uint32       _interruptTimer;     // H3: interrupt CD (role-based)
    uint32       _interruptCheckTimer;// H3: poll enemy cast every 1-2s
    uint32       _prioritySwitchTimer;// H4: DPS priority-target scan
    uint32       _addScanTimer;       // H5: off-tank add intercept scan
    uint32       _speechTimer;        // B1: NPC speech cooldown (2 min)
    bool         _moveDone;
    bool         _isReplica;          // true = inside-instance copy; do not delete DB row on death
    bool         _executeActive;      // H6: enemy < 20% HP — execute phase
    bool         _fightingBoss;       // H8: boss awareness (tighten timers)
    uint8        _ageArchetype;       // B14: 0=Young, 1=Adult, 2=Elder
    float        _formDist;           // formation: follow distance (slot-based)
    float        _formAngle;          // formation: follow angle (slot-based)
    uint32       _leashWhisperTimer;  // leash-back: rate-limit the "finding way back" whisper
    uint32       _workEmoteTimer;     // P1: cadence for work emotes while at a GO
    uint8        _workObjectType;     // P1: 0=forge/anvil  1=cooking fire

    // Weight table: WANDER=38%, IDLE=27%, HUNT=18%, ACTIVITY=12%, SOCIAL=5%
    // B2: Night hours (20:00-06:00 server time) shift weight heavily toward rest
    void _SelectNextState()
    {
        // B2: Night mode check
        {
            time_t now = time(nullptr);
            int secsInDay = int(now % 86400);
            bool isNight  = (secsInDay >= 72000 || secsInDay < 21600);
            if (isNight)
            {
                uint32 nightRoll = urand(0, 99);
                if (nightRoll < 50)
                {
                    // Rest in place — sit or sleep
                    _state = STATE_IDLE;
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveIdle();
                    me->HandleEmoteCommand(urand(0,1) ? EMOTE_STATE_SLEEP : EMOTE_STATE_SIT);
                    // B1: whisper something while resting (less frequent at night)
                    if (_speechTimer == 0 && urand(0, 9) == 0)
                    {
                        me->Say(AmbientSpeech::Roll(me->GetEntry()), LANG_UNIVERSAL);
                        _speechTimer = 120000;
                    }
                    _timer = urand(18000, 40000);  // long night rests
                    return;
                }
                else if (nightRoll < 70)
                {
                    _DoWander();   // slow night stroll
                    return;
                }
                else if (nightRoll < 88)
                {
                    _DoIdle();     // brief night idle
                    return;
                }
                else if (nightRoll < 96)
                {
                    _DoSocial();   // late-night conversation
                    return;
                }
                // remaining 4%: fall through to normal state machine
            }
        }

        uint32 roll = urand(0, 99);
        if      (roll < 38) _DoWander();
        else if (roll < 65) _DoIdle();
        else if (roll < 83) _DoHunt();
        else if (roll < 95) _DoActivity();
        else                _DoSocial();
    }

    // ---- IDLE -----------------------------------------------
    void _DoIdle()
    {
        _state = STATE_IDLE;
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveIdle();

        // B6: Sit near a campfire if one is within 15y (highest immersion priority)
        if (urand(0, 4) == 0)
        {
            static const uint32 FIRE_ENTRIES[] = { 5177, 5178, 5179 };
            for (uint32 fi = 0; fi < 3; ++fi)
            {
                if (GameObject* gobj = me->FindNearestGameObject(FIRE_ENTRIES[fi], 15.f))
                {
                    me->GetMotionMaster()->MovePoint(3,
                        gobj->GetPositionX(), gobj->GetPositionY(), gobj->GetPositionZ());
                    me->HandleEmoteCommand(EMOTE_STATE_SIT);
                    _timer = urand(12000, 28000);
                    return;
                }
            }
        }

        // P1: 15% chance to seek a nearby world object and work at it (forge, anvil, fire)
        if (urand(0, 6) == 0)
        {
            _DoWork();
            if (_state == STATE_WORK)
                return;
        }

        // B1: 20% chance to say a flavour line, gated by per-NPC 2-min cooldown
        if (_speechTimer == 0 && urand(0, 4) == 0)
        {
            const char* line = AmbientSpeech::Roll(me->GetEntry());
            me->Say(line, LANG_UNIVERSAL);
            _speechTimer = 120000;  // 2-minute cooldown
        }

        // B2: Night mode (server 20:00 - 06:00) — NPCs rest instead of idle emotes
        {
            time_t now = time(nullptr);
            int secsInDay = int(now % 86400);
            bool isNight  = (secsInDay >= 72000 || secsInDay < 21600);
            if (isNight && urand(0, 1))
            {
                me->HandleEmoteCommand(urand(0, 1) ? EMOTE_STATE_SLEEP : EMOTE_STATE_SIT);
                _timer = urand(14000, 30000);  // longer rests at night
                return;
            }
        }

        static const uint32 IDLE_EMOTES[] = {
            EMOTE_ONESHOT_WAVE, EMOTE_ONESHOT_CHEER, EMOTE_ONESHOT_LAUGH,
            EMOTE_ONESHOT_BOW,  EMOTE_ONESHOT_POINT, EMOTE_ONESHOT_SALUTE,
            EMOTE_ONESHOT_ROAR, EMOTE_ONESHOT_EAT_NO_SHEATHE,
        };
        me->HandleEmoteCommand(IDLE_EMOTES[urand(0, 7)]);
        _timer = urand(3000, 7000);
    }

    // ---- WANDER ---------------------------------------------
    // Walk to a random point near home using MovePoint so it looks purposeful
    // and won't loop forever like MoveRandom does.
    void _DoWander()
    {
        _state    = STATE_WANDER;
        _moveDone = false;

        float angle  = frand(0.f, float(M_PI) * 2.f);
        float dist   = frand(15.f, SPREAD_RADIUS);
        float destX  = _homeX + std::cos(angle) * dist;
        float destY  = _homeY + std::sin(angle) * dist;
        float destZ  = _homeZ;

        // Sample terrain height so they land on the ground
        if (Map* m = me->GetMap())
        {
            float h = m->GetHeight(me->GetPhaseShift(), destX, destY, _homeZ + 5.f, true, 50.f);
            if (h > INVALID_HEIGHT + 1.f)
                destZ = h;
        }

        me->GetMotionMaster()->Clear();

        // B7: Speed variation — most NPCs walk at slightly different paces;
        //     10% chance to stride briskly (in a hurry feel)
        // B14: Elder archetype walks slower; Young archetype slightly faster
        if (urand(0, 9) == 0)
            me->SetSpeed(MOVE_WALK, frand(3.5f, 5.0f));  // striding fast
        else if (_ageArchetype == 2)  // Elder — slower gait
            me->SetSpeed(MOVE_WALK, frand(1.6f, 1.9f));
        else if (_ageArchetype == 0)  // Young — quicker step
            me->SetSpeed(MOVE_WALK, frand(2.2f, 2.8f));
        else
            me->SetSpeed(MOVE_WALK, frand(1.8f, 2.5f));  // natural variation

        me->GetMotionMaster()->MovePoint(1, destX, destY, destZ);

        // Allow enough time for a walk at ~2.5 m/s across up to 60y (~24s max)
        _timer = urand(6000, 14000);
    }

    // ---- WORK -----------------------------------------------
    // P1: Detect a nearby world object that suggests labour (forge, anvil, cooking fire)
    // and move to it for an immersive crafting/cooking sequence.
    // Only non-companion NPCs enter this state; companions always follow their owner.
    void _DoWork()
    {
        // GO entries known to exist in all Legion builds:
        //   Forge  1685  Anvil  2296 / 3299   Blacksmith bench  2543
        //   Cooking fire / campfire  5177 5178 5179  (already used for B6 campfire sit)
        static const uint32 CRAFT_GO[] = { 1685, 2296, 3299, 2543 };
        static const uint32 FIRE_GO[]  = { 5177, 5178, 5179 };

        // Try forge/anvil first
        for (uint32 ge : CRAFT_GO)
        {
            if (GameObject* go = me->FindNearestGameObject(ge, 18.f))
            {
                _state          = STATE_WORK;
                _workObjectType = 0;            // forge/anvil hammering
                _workEmoteTimer = 1500;
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(2,
                    go->GetPositionX() + frand(-1.5f, 1.5f),
                    go->GetPositionY() + frand(-1.5f, 1.5f),
                    go->GetPositionZ());
                _timer = urand(12000, 22000);
                return;
            }
        }
        // Then cooking fire
        for (uint32 fe : FIRE_GO)
        {
            if (GameObject* go = me->FindNearestGameObject(fe, 18.f))
            {
                _state          = STATE_WORK;
                _workObjectType = 1;            // cooking emotes
                _workEmoteTimer = 1500;
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(2,
                    go->GetPositionX() + frand(-1.5f, 1.5f),
                    go->GetPositionY() + frand(-1.5f, 1.5f),
                    go->GetPositionZ());
                _timer = urand(10000, 18000);
                return;
            }
        }
        // No suitable GO nearby — fall back to idle
        _DoIdle();
    }

    // ---- ACTIVITY -------------------------------------------
    void _DoActivity()
    {
        _state = STATE_ACTIVITY;
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveIdle();

        switch (_role)
        {
            case AMBIENT_MAGE:
            case AMBIENT_HEALER:
                me->HandleEmoteCommand(EMOTE_ONESHOT_SPELL_PRECAST);
                _timer = urand(7000, 11000);
                break;
            case AMBIENT_WARRIOR:
            case AMBIENT_ROGUE:
                me->HandleEmoteCommand(EMOTE_STATE_WORK_SHEATHED);
                _timer = urand(4000, 8000);
                break;
            case AMBIENT_TANK:
                // Tanks train — shield stance / roar war cry
                me->HandleEmoteCommand(urand(0,1) ? EMOTE_ONESHOT_ROAR : EMOTE_ONESHOT_SALUTE);
                _timer = urand(4000, 7000);
                break;
            case AMBIENT_HUNTER:
                me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_BOW);
                _timer = urand(2500, 5000);
                break;
            default:
                me->HandleEmoteCommand(EMOTE_ONESHOT_EAT_NO_SHEATHE);
                _timer = urand(5000, 9000);
                break;
        }
    }

    // ---- HUNT -----------------------------------------------
    // Find and attack any visible hostile creature not currently fighting
    // a real player.  Gives them dynamic, unpredictable looking fights.
    void _DoHunt()
    {
        _state = STATE_HUNT;

        Creature* target = nullptr;

        // Walk through nearby world objects looking for a valid target.
        // Only attack creatures that are ACTIVELY hostile to us (not merely
        // "not friendly") and appear to be wild/combat mobs, never city NPCs.
        std::list<Creature*> nearList;
        me->GetCreatureListInGrid(nearList, 40.f);
        for (Creature* c : nearList)
        {
            if (target)
                break;
            if (c == me || !c->IsAlive())
                continue;
            // Must be genuinely hostile to us — skips neutral city NPCs entirely
            if (!c->IsHostileTo(me))
                continue;
            if (c->IsSummon())
                continue;
            if (c->IsInCombat())
                continue;
            // Skip other ambient NPCs
            if (c->GetEntry() >= 9500080 && c->GetEntry() <= 9500094)
                continue;
            // Skip critters
            if (c->IsCritter())
                continue;
            // Skip city/friendly service NPCs (vendors, trainers, quest givers, gossip)
            if (c->IsVendor() || c->IsTrainer() || c->IsQuestGiver() || c->IsGossip())
                continue;
            // Only attack mobs that the engine has flagged as aggressive
            // (passive/defensive mobs sitting in towns are excluded)
            if (c->GetReactState() != REACT_AGGRESSIVE)
                continue;
            target = c;
        }

        if (target)
        {
            me->GetMotionMaster()->Clear();
            AttackStart(target);
            _timer = urand(10000, 20000);
            return;
        }

        // B5: No hostile mobs — 30% chance to stalk a critter for flavour
        //     (walk toward it and mime aiming/pointing without attacking)
        if (urand(0, 2) == 0)
        {
            std::list<Creature*> critters;
            me->GetCreatureListInGrid(critters, 30.f);
            for (Creature* cr : critters)
            {
                if (!cr || !cr->IsAlive() || !cr->IsCritter()) continue;
                me->SetFacingToObject(cr);
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(2,
                    cr->GetPositionX(), cr->GetPositionY(), cr->GetPositionZ());
                if (_role == AMBIENT_HUNTER)
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_BOW);  // hunter takes aim
                else
                    me->HandleEmoteCommand(EMOTE_ONESHOT_POINT);        // "look there!"
                _timer = urand(4000, 8000);
                return;
            }
        }

        // No hostile mobs and no critters — fall back to wander
        _DoWander();
    }

    // ---- SOCIAL ---------------------------------------------
    void _DoSocial()
    {
        _state = STATE_SOCIAL;
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveIdle();

        Creature* buddy = nullptr;
        std::list<Creature*> socialList;
        me->GetCreatureListInGrid(socialList, 20.f);
        for (Creature* c : socialList)
        {
            if (buddy) break;
            if (c == me || !c->IsAlive()) continue;
            if (c->GetEntry() < 9500080 || c->GetEntry() > 9500094) continue;
            buddy = c;
        }

        if (buddy)
        {
            me->SetFacingToObject(buddy);
            static const uint32 SOCIAL_EMOTES[] = {
                EMOTE_ONESHOT_WAVE, EMOTE_ONESHOT_POINT,
                EMOTE_ONESHOT_LAUGH, EMOTE_ONESHOT_BOW, EMOTE_ONESHOT_CHEER,
            };
            me->HandleEmoteCommand(SOCIAL_EMOTES[urand(0, 4)]);
            if (urand(0, 1))
                buddy->HandleEmoteCommand(EMOTE_ONESHOT_WAVE_NO_SHEATHE);
        }

        _timer = urand(3000, 6000);
    }
};

// ============================================================
//  Party composition helpers
//  (defined after npc_ambient_aiAI so CAST_AI resolves)
// ============================================================
struct PartyCompo { uint32 tanks = 0, healers = 0, dps = 0, total = 0; };

static PartyCompo ComputePartyCompo(Player* player)
{
    PartyCompo out;
    for (uint32 e = 9500080; e <= 9500094; ++e)
    {
        std::list<Creature*> cl;
        GetCreatureListWithEntryInGrid(cl, player, e, 100.f);
        for (Creature* c : cl)
        {
            CompanionData cd;
            if (!GetCompanionData(c->GetGUID(), cd)) continue;
            if (cd.ownerGuid != player->GetGUID()) continue;
            npc_ambient_aiAI* ai = CAST_AI(npc_ambient_aiAI, c->AI());
            if (!ai) continue;
            ++out.total;
            AmbientRole r = ai->GetRole();
            if      (r == AMBIENT_TANK)   ++out.tanks;
            else if (r == AMBIENT_HEALER) ++out.healers;
            else                          ++out.dps;
        }
    }
    return out;
}

// ============================================================
//  CreatureScript  (AI + gossip)
// ============================================================
class npc_ambient_ai : public CreatureScript
{
public:
    npc_ambient_ai() : CreatureScript("npc_ambient_ai") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ambient_aiAI(creature);
    }

    // ------ Gossip: Hello (open menu) -------------------------
    bool OnGossipHello(Player* player, Creature* creature) override
    {
        npc_ambient_aiAI* ai = CAST_AI(npc_ambient_aiAI, creature->AI());
        if (!ai)
            return false;

        if (ai->_isCompanion && ai->_ownerGuid == player->GetGUID())
        {
            // Already companion of this player - show status + options
            CompanionData cd;
            if (GetCompanionData(creature->GetGUID(), cd))
            {
                std::string status = "Level " + std::to_string(cd.currentLevel) +
                    "  \xe2\x80\x94  XP: " + std::to_string(cd.xp) +
                    " / " + std::to_string(cd.xpNeeded) +
                    "  \xe2\x80\x94  Kills: " + std::to_string(cd.killCount);
                if (const char* title = GetCompanionTitle(cd.killCount))
                {
                    status += "  \xe2\x80\x94  ";
                    status += title;
                }
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, status,
                    COMPANION_GOSSIP_SENDER, COMPANION_ACTION_STATUS);
            }
            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                "You are free to go. Return to your own path.",
                COMPANION_GOSSIP_SENDER, COMPANION_ACTION_DISMISS);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                "Goodbye.",
                COMPANION_GOSSIP_SENDER, COMPANION_ACTION_CLOSE);
        }
        else if (!ai->_isCompanion)
        {
            // Show current group composition so the player knows which roles they still need
            PartyCompo compo = ComputePartyCompo(player);
            if (compo.total > 0)
            {
                std::string info = "|cffaaaaaa[Party] ";
                info += std::to_string(compo.tanks)   + "T  ";
                info += std::to_string(compo.healers) + "H  ";
                info += std::to_string(compo.dps)     + " DPS";
                if (compo.tanks == 0 || compo.healers == 0)
                {
                    info += "  \xe2\x80\x94";
                    if (compo.tanks == 0)   info += " need Tank";
                    if (compo.healers == 0) info += " need Healer";
                }
                info += "|r";
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, info,
                    COMPANION_GOSSIP_SENDER, COMPANION_ACTION_STATUS);
            }

            // Label the NPC's role so the player knows what archetype they're hiring
            std::string hireText = std::string("Join my party!  [") + RoleName(ai->GetRole()) + "]";
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, hireText,
                COMPANION_GOSSIP_SENDER, COMPANION_ACTION_HIRE);

            // One-click balanced party fill if there are open slots
            if (compo.total < 4)
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Form a balanced party  (1 Tank, 1 Healer, 2 DPS)",
                    COMPANION_GOSSIP_SENDER, COMPANION_ACTION_AUTOPARTY);

            // ---- Raid roster ----
            uint32 rCount = GetRosterCount(player->GetGUID());
            if (rCount < ROSTER_MAX_SIZE)
            {
                std::string signupText = std::string("Sign up for my raid  [") + RoleName(ai->GetRole()) + "]";
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE, signupText,
                    ROSTER_GOSSIP_SENDER, ROSTER_ACTION_SIGNUP);
            }
            if (rCount > 0)
            {
                std::string viewText = "View raid roster  (" + std::to_string(rCount) + "/" + std::to_string(ROSTER_MAX_SIZE) + ")";
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, viewText,
                    ROSTER_GOSSIP_SENDER, ROSTER_ACTION_VIEW);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Clear raid roster",
                    ROSTER_GOSSIP_SENDER, ROSTER_ACTION_CLEAR);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "+1 extra tank slot  (tank-swap boss override)",
                    ROSTER_GOSSIP_SENDER, ROSTER_ACTION_EXTRA_TANK);
            }

            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                "Goodbye.",
                COMPANION_GOSSIP_SENDER, COMPANION_ACTION_CLOSE);
        }
        else
        {
            // Busy with another player
            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                "I am already assisting someone else.",
                COMPANION_GOSSIP_SENDER, COMPANION_ACTION_STATUS);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                "Goodbye.",
                COMPANION_GOSSIP_SENDER, COMPANION_ACTION_CLOSE);
        }

        SendGossipMenuFor(player, 1, creature->GetGUID());
        return true;
    }

    // ------ Gossip: Select (player chose an option) -----------
    bool OnGossipSelect(Player* player, Creature* creature,
        uint32 /*sender*/, uint32 action) override
    {
        CloseGossipMenuFor(player);
        npc_ambient_aiAI* ai = CAST_AI(npc_ambient_aiAI, creature->AI());
        if (!ai)
            return false;

        switch (action)
        {
            case COMPANION_ACTION_HIRE:
                ai->HireCompanion(player);
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "|cff00ff00[Companion]|r %s joins your party!",
                    creature->GetName().c_str());
                break;
            case COMPANION_ACTION_DISMISS:
                ai->DismissCompanion();
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "|cffaaaaaa[Companion]|r %s has departed.",
                    creature->GetName().c_str());
                break;
            case COMPANION_ACTION_CLOSE:
                // Just close — no action needed (gossip already closed above)
                break;
            case COMPANION_ACTION_AUTOPARTY:
            {
                PartyCompo compo = ComputePartyCompo(player);
                if (compo.total >= 4)
                {
                    ChatHandler(player->GetSession()).PSendSysMessage(
                        "|cffaaaaaa[Party]|r Your party is already full (4/4).");
                    break;
                }

                // Bucket available (unhired) nearby ambient NPCs by role
                std::vector<Creature*> availTanks, availHealers, availDps;
                for (uint32 e = 9500080; e <= 9500094; ++e)
                {
                    std::list<Creature*> cl;
                    GetCreatureListWithEntryInGrid(cl, player, e, 100.f);
                    for (Creature* c : cl)
                    {
                        npc_ambient_aiAI* cai = CAST_AI(npc_ambient_aiAI, c->AI());
                        if (!cai || cai->_isCompanion) continue;
                        AmbientRole r = cai->GetRole();
                        if      (r == AMBIENT_TANK)   availTanks.push_back(c);
                        else if (r == AMBIENT_HEALER)  availHealers.push_back(c);
                        else                           availDps.push_back(c);
                    }
                }

                uint32 hired = 0;
                uint32 slots = 4 - compo.total;
                auto tryHire = [&](Creature* c, const char* roleLabel)
                {
                    if (hired >= slots) return;
                    npc_ambient_aiAI* cai = CAST_AI(npc_ambient_aiAI, c->AI());
                    if (!cai || cai->_isCompanion) return;
                    cai->HireCompanion(player);
                    ChatHandler(player->GetSession()).PSendSysMessage(
                        "|cff00ff00[Party]|r %s (%s) joins your party!",
                        c->GetName().c_str(), roleLabel);
                    ++hired;
                };

                // Fill missing tank first, then healer, then DPS to fill remaining slots
                if (compo.tanks == 0 && !availTanks.empty())
                    tryHire(availTanks[0], "Tank");
                if (compo.healers == 0 && hired < slots && !availHealers.empty())
                    tryHire(availHealers[0], "Healer");
                for (size_t i = 0; i < availDps.size() && hired < slots; ++i)
                    tryHire(availDps[i], "DPS");

                if (hired == 0)
                    ChatHandler(player->GetSession()).PSendSysMessage(
                        "|cffaaaaaa[Party]|r No adventurers found nearby \xe2\x80\x94 explore a bit and they will appear around you.");
                else
                {
                    // Post-fill warning if group is still missing a critical role
                    PartyCompo after = ComputePartyCompo(player);
                    if (after.tanks == 0)
                        ChatHandler(player->GetSession()).PSendSysMessage(
                            "|cffff8800[Party]|r Your group has no tank! Look for a Death Knight, Warrior, or Paladin.");
                    if (after.healers == 0)
                        ChatHandler(player->GetSession()).PSendSysMessage(
                            "|cffff8800[Party]|r Your group has no healer! Look for a Shaman, Druid, Priest, or Paladin.");
                }
                break;
            }
            // ---- Raid roster actions ----
            case ROSTER_ACTION_SIGNUP:
            {
                if (!ai) break;
                std::string npcName = creature->GetName();
                bool alreadyIn = false;
                {
                    std::lock_guard<std::mutex> lk(s_rosterMutex);
                    auto it = s_raidRoster.find(player->GetGUID());
                    if (it != s_raidRoster.end())
                        for (auto const& re : it->second)
                            if (re.displayName == npcName) { alreadyIn = true; break; }
                }
                if (alreadyIn)
                {
                    ChatHandler(player->GetSession()).PSendSysMessage(
                        "|cffff8800[Raid]|r %s is already in your roster.", npcName.c_str());
                    break;
                }
                RosterEntry re;
                re.displayName = npcName;
                re.role        = ai->GetRole();
                re.level       = (uint8)creature->getLevel();
                re.isManual    = true;
                {
                    std::lock_guard<std::mutex> lk(s_rosterMutex);
                    s_raidRoster[player->GetGUID()].push_back(re);
                }
                DB_AddToRoster(player->GetGUID().GetCounter(), npcName, (uint8)re.role, re.level);
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "|cff00ffff[Raid]|r %s (%s Lv%u) signed up. Roster: %u/%u",
                    npcName.c_str(), RoleName(re.role), (uint32)re.level,
                    GetRosterCount(player->GetGUID()), ROSTER_MAX_SIZE);
                break;
            }
            case ROSTER_ACTION_VIEW:
            {
                std::vector<RosterEntry> copy;
                {
                    std::lock_guard<std::mutex> lk(s_rosterMutex);
                    auto it = s_raidRoster.find(player->GetGUID());
                    if (it != s_raidRoster.end()) copy = it->second;
                }
                uint32 rT = 0, rH = 0, rD = 0;
                for (auto const& re : copy)
                    if      (re.role == AMBIENT_TANK)   ++rT;
                    else if (re.role == AMBIENT_HEALER) ++rH;
                    else                                ++rD;
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "|cff00ffff[Raid]|r Roster (%u/%u): %uT  %uH  %u DPS",
                    (uint32)copy.size(), ROSTER_MAX_SIZE, rT, rH, rD);
                uint32 shown = 0;
                for (auto const& re : copy)
                {
                    ChatHandler(player->GetSession()).PSendSysMessage(
                        "  %s  (%s Lv%u)", re.displayName.c_str(), RoleName(re.role), (uint32)re.level);
                    if (++shown >= 10 && copy.size() > 10)
                    {
                        ChatHandler(player->GetSession()).PSendSysMessage(
                            "  ... and %u more", (uint32)copy.size() - shown);
                        break;
                    }
                }
                if (copy.empty())
                    ChatHandler(player->GetSession()).PSendSysMessage("|cffaaaaaa[Raid]|r Roster is empty.");
                break;
            }
            case ROSTER_ACTION_CLEAR:
            {
                uint32 was = GetRosterCount(player->GetGUID());
                {
                    std::lock_guard<std::mutex> lk(s_rosterMutex);
                    s_raidRoster.erase(player->GetGUID());
                    s_extraTankOverride.erase(player->GetGUID());
                }
                DB_ClearRoster(player->GetGUID().GetCounter());
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "|cffaaaaaa[Raid]|r Roster cleared (%u members dismissed). Extra tank overrides reset.", was);
                break;
            }
            case ROSTER_ACTION_EXTRA_TANK:
            {
                uint32 extra = 0;
                {
                    std::lock_guard<std::mutex> lk(s_rosterMutex);
                    extra = ++s_extraTankOverride[player->GetGUID()];
                }
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "|cff00ffff[Raid]|r Extra tank override: +%u slot(s). "
                    "Good for tank-swap bosses \xe2\x80\x94 use \"Clear raid roster\" to reset when done.",
                    extra);
                break;
            }
            default:
                break;
        }
        return true;
    }
};

// ============================================================
//  Spawn helpers
// ============================================================
static uint32 PickAmbientEntry(Player* player)
{
    uint32 mapId = player->GetMapId();
    // Broken Isles / Argus -- neutral only
    if (mapId == 1220 || mapId == 1116)
        return NEUTRAL_POOL[urand(0, 4)];

    // 70% same-faction, 30% neutral for variety
    uint32 roll = urand(0, 9);
    if (player->GetTeam() == ALLIANCE)
        return (roll < 7) ? ALLIANCE_POOL[urand(0, 4)] : NEUTRAL_POOL[urand(0, 4)];
    if (player->GetTeam() == HORDE)
        return (roll < 7) ? HORDE_POOL[urand(0, 4)]    : NEUTRAL_POOL[urand(0, 4)];
    return NEUTRAL_POOL[urand(0, 4)];
}

static uint32 CountNearbyAmbient(Player* player)
{
    uint32 count = 0;
    for (uint32 e = 9500080; e <= 9500094; ++e)
    {
        std::list<Creature*> nl;
        GetCreatureListWithEntryInGrid(nl, player, e, SEARCH_RADIUS);
        count += static_cast<uint32>(nl.size());
    }
    return count;
}

static Position RandomPositionNear(Player* player)
{
    float angle = frand(0.f, float(M_PI) * 2.f);
    float dist  = frand(15.f, SPREAD_RADIUS);
    float x = player->GetPositionX() + std::cos(angle) * dist;
    float y = player->GetPositionY() + std::sin(angle) * dist;
    float z = player->GetPositionZ();

    if (Map* m = player->GetMap())
    {
        float h = m->GetHeight(player->GetPhaseShift(), x, y, z + 5.f, true, 50.f);
        if (h > INVALID_HEIGHT + 1.f)
            z = h;
    }
    return { x, y, z, frand(0.f, float(M_PI) * 2.f) };
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
        // Restore a previously-hired companion that was saved on logout
        if (!player->GetMap()->Instanceable())
            RestoreCompanionForPlayer(player);
        // Reload the persisted raid roster into memory
        RestoreRosterForPlayer(player);
    }

    void OnMapChanged(Player* player) override
    {
        if (!player) return;
        Map* map = player->GetMap();
        if (!map) return;

        if (map->Instanceable())
        {
            // Entering a dungeon or raid — fill the instance with Tier-1 replicas + Tier-2 roster + auto-fill
            HandleInstanceEntry(player, map);
        }
        else
        {
            // Returning to the open world — flush replica XP to DB, clean them from memory, then re-summon
            FlushAndCleanReplicas(player);
            RestoreCompanionForPlayer(player);
        }
    }

    void OnUpdateZone(Player* player, uint32 newZone, uint32 /*newArea*/, uint32 oldZone) override
    {
        if (!player || !player->IsInWorld() || !player->IsAlive()) return;
        if (player->getLevel() == 0) return;

        // C1: Despawn non-companion ambient NPCs that belong to the previous zone.
        // The player may have walked away but those NPCs keep ticking — clean them up now.
        if (oldZone && oldZone != newZone)
        {
            for (uint32 amb = 9500080; amb <= 9500094; ++amb)
            {
                std::list<Creature*> clist;
                GetCreatureListWithEntryInGrid(clist, player, amb, 300.f);
                for (Creature* c : clist)
                {
                    if (!c || !c->IsAlive()) continue;
                    if (c->GetZoneId() != oldZone) continue;
                    // Never despawn companions — they belong to the player, not the zone
                    if (npc_ambient_aiAI* ai = CAST_AI(npc_ambient_aiAI, c->GetAI()))
                        if (ai->_isCompanion) continue;
                    c->DespawnOrUnsummon(5000);
                }
            }
        }

        TrySpawnAmbient(player, newZone);
    }

    // I1: Companion chat commands  |  I3: Party status  (.stay / .follow / .passive / .attack / .dismiss / .cs)
    void OnChat(Player* player, uint32 type, uint32 /*lang*/, std::string& msg) override
    {
        if (type != CHAT_MSG_SAY || msg.empty() || msg[0] != '.') return;

        // Build lowercase copy for comparison
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
        else return;  // not one of our commands — leave msg alone

        msg = "";  // suppress the Say from broadcasting

        // Gather all living companions owned by this player within 100y
        std::vector<Creature*> comp;
        for (uint32 e = 9500080; e <= 9500094; ++e)
        {
            std::list<Creature*> cl;
            GetCreatureListWithEntryInGrid(cl, player, e, 100.f);
            for (Creature* c : cl)
            {
                if (!c || !c->IsAlive()) continue;
                if (npc_ambient_aiAI* ai = CAST_AI(npc_ambient_aiAI, c->GetAI()))
                    if (ai->_isCompanion && ai->_ownerGuid == player->GetGUID())
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
                if (npc_ambient_aiAI* ai = CAST_AI(npc_ambient_aiAI, c->GetAI()))
                    ai->DismissCompanion();
            ch.PSendSysMessage("|cff00ccff[Companions]|r All companions dismissed.");
        }
        else if (doStatus)
        {
            // I3: One-line summary per companion
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
        // Snapshot the companion's current XP/level before it despawns
        SaveCompanionOnLogout(player);
    }

    void OnCreatureKill(Player* killer, Creature* killed) override
    {
        if (!killer || !killed)
            return;

        uint32 xpGain = CompanionXpGain((uint32)killed->getLevel());

        // Award XP to every companion owned by this player that is nearby
        for (uint32 entry = 9500080; entry <= 9500094; ++entry)
        {
            std::list<Creature*> cList;
            GetCreatureListWithEntryInGrid(cList, killer, entry, 50.f);
            for (Creature* companion : cList)
            {
                if (!companion || !companion->IsAlive())
                    continue;
                // E4: single lock acquisition — eliminates TOCTOU window
                CompanionData cd;
                if (!GetCompanionDataIfOwned(companion->GetGUID(), killer->GetGUID(), cd))
                    continue;

                uint32 newLevel = 0;
                bool   leveledUp = AwardCompanionXP(companion->GetGUID(), xpGain, newLevel);

                // Use the cached display name; fall back to GetName() if needed
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

    // ---- Login restore: spawn all saved companions near the player ----
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

            // E8: Reject DB rows with out-of-range entries (guards against manual edits)
            if (entry < 9500080 || entry > 9500094)
            {
                TC_LOG_WARN("scripts", "AmbientNPC: invalid companion entry %u for player %s — skipping",
                    entry, player->GetName().c_str());
                continue;
            }

            if (!sObjectMgr->GetCreatureTemplate(entry))
                continue;

            Position pos = RandomPositionNear(player);
            TempSummon* s = player->SummonCreature(entry, pos,
                TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, DESPAWN_TIME_MS);
            if (!s) continue;

            uint8 slvl = (uint8)std::min(lvl, 110u);
            s->SetLevel(slvl);
            uint32 hp = 100u * slvl * slvl + 500u * slvl + 1000u;
            s->SetMaxHealth(hp);
            s->SetHealth(hp);
            // Keep damage proportional to level
            s->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(slvl) * 0.90f);
            s->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(slvl) * 1.35f);
            s->UpdateDamagePhysical(BASE_ATTACK);
            s->SetName(name);
            if (dispId)
                s->SetDisplayId(dispId);

            // Mount restored companions at appropriate levels
            {
                uint32 mountId = GetMountDisplayId(entry, slvl);
                if (mountId)
                    s->Mount(mountId);
            }

            npc_ambient_aiAI* ai = CAST_AI(npc_ambient_aiAI, s->AI());
            if (!ai) continue;

            ai->HireCompanion(player, slvl, xp);
            // Inject restored kill count into the live companion data
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

    // ---- Instance exit: flush in-memory replica progress to DB, then remove them from tracking ----
    void FlushAndCleanReplicas(Player* player)
    {
        // Collect replica data while holding lock, write to DB outside lock
        std::vector<std::tuple<std::string, uint32, uint32, uint32>> toSave; // (name, level, xp, kills)
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

    // ---- Instance entry: summon Tier-1 replicas + Tier-2 roster + auto-fill ----
    void HandleInstanceEntry(Player* player, Map* map)
    {
        uint32 playerGuid = player->GetGUID().GetCounter();
        uint8  plvl       = player->getLevel();
        InstanceSize inst = GetInstanceSize(map);
        uint32 npcSlots   = inst.total > 0 ? inst.total - 1 : 4; // -1 for the real player
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
                if (!sObjectMgr->GetCreatureTemplate(entry)) continue;
                Position pos = RandomPositionNear(player);
                TempSummon* s = player->SummonCreature(entry, pos,
                    TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 4u * 3600u * 1000u);
                if (!s) continue;
                uint8 slvl = (uint8)std::min(lvl, 110u);
                s->SetLevel(slvl);
                uint32 hp = 100u * slvl * slvl + 500u * slvl + 1000u;
                s->SetMaxHealth(hp); s->SetHealth(hp);
                s->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(slvl) * 0.90f);
                s->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(slvl) * 1.35f);
                s->UpdateDamagePhysical(BASE_ATTACK);
                s->SetName(name);
                if (disp) s->SetDisplayId(disp);
                // Mount instance companion replicas at appropriate levels
                {
                    uint32 mountId = GetMountDisplayId(entry, slvl);
                    if (mountId)
                        s->Mount(mountId);
                }
                npc_ambient_aiAI* ai = CAST_AI(npc_ambient_aiAI, s->AI());
                if (!ai) continue;
                ai->HireCompanion(player, slvl, xp, /*isReplica=*/true);
                if (kc > 0)
                {
                    std::lock_guard<std::mutex> lk(s_companionMutex);
                    auto it = s_companions.find(s->GetGUID());
                    if (it != s_companions.end())
                        it->second.killCount = kc;
                }
                AmbientRole r = ai->GetRole();
                if      (r == AMBIENT_TANK)   ++compTanks;
                else if (r == AMBIENT_HEALER) ++compHealers;
                else                          ++compDps;
            } while (compResult->NextRow());
        }

        // ── Tier 2a: Summon manually rostered NPCs ───────────────────────────────
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
                TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 4u * 3600u * 1000u);
            if (!s) continue;
            uint8 rlvl = re.level > 0 ? re.level : plvl;
            s->SetLevel(rlvl);
            uint32 hp = 100u * rlvl * rlvl + 500u * rlvl + 1000u;
            s->SetMaxHealth(hp); s->SetHealth(hp);
            s->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(rlvl) * 0.90f);
            s->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(rlvl) * 1.35f);
            s->UpdateDamagePhysical(BASE_ATTACK);
            s->SetName(re.displayName);
            s->SetReactState(REACT_DEFENSIVE);
            s->GetMotionMaster()->MoveFollow(player, 3.5f + float(rT+rH+rD) * 0.4f, float(M_PI));
            if      (re.role == AMBIENT_TANK)   ++rT;
            else if (re.role == AMBIENT_HEALER) ++rH;
            else                                ++rD;
        }

        // ── Tier 2b: Auto-fill remaining slots with correct role ratios ──────────
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

        // Rotate through DPS archetypes for visual variety
        static const AmbientRole DPS_ROLES[] = {
            AMBIENT_WARRIOR, AMBIENT_MAGE, AMBIENT_HUNTER, AMBIENT_ROGUE, AMBIENT_DEFAULT
        };
        uint32 autoFilled = 0, dpsIdx = 0;

        auto spawnFill = [&](AmbientRole role)
        {
            uint32 entry = EntryForRole(role);
            if (!sObjectMgr->GetCreatureTemplate(entry)) return;
            Position pos = RandomPositionNear(player);
            TempSummon* s = player->SummonCreature(entry, pos,
                TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 4u * 3600u * 1000u);
            if (!s) return;
            s->SetLevel(plvl);
            uint32 hp = 100u * plvl * plvl + 500u * plvl + 1000u;
            s->SetMaxHealth(hp); s->SetHealth(hp);
            s->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, float(plvl) * 0.90f);
            s->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, float(plvl) * 1.35f);
            s->UpdateDamagePhysical(BASE_ATTACK);
            s->SetName(AmbientNames::Roll(entry));
            // Mount auto-fill NPCs at appropriate levels
            {
                uint32 mountId = GetMountDisplayId(entry, plvl);
                if (mountId)
                    s->Mount(mountId);
            }
            s->SetReactState(REACT_DEFENSIVE);
            s->GetMotionMaster()->MoveFollow(player,
                3.5f + float(autoFilled) * 0.5f,
                float(M_PI) + float(autoFilled) * 0.42f);
            ++autoFilled;
        };

        for (uint32 i = 0; i < needTanks;   ++i) spawnFill(AMBIENT_TANK);
        for (uint32 i = 0; i < needHealers; ++i) spawnFill(AMBIENT_HEALER);
        for (uint32 i = 0; i < needDps;     ++i) { spawnFill(DPS_ROLES[dpsIdx++ % 5]); }

        // ── Summary message ────────────────────────────────────────────────────
        uint32 finalTanks   = haveTanks   + needTanks;
        uint32 finalHealers = haveHealers + needHealers;
        uint32 finalDps     = haveDps     + needDps;
        ChatHandler(player->GetSession()).PSendSysMessage(
            "|cff00ffff[Raid]|r Entering with %uT %uH %u DPS (%u/%u) \xe2\x80\x94 %u auto-filled",
            finalTanks, finalHealers, finalDps,
            1 + haveTotal + autoFilled, inst.total, autoFilled);
        if (finalTanks < inst.minTanks + extraTanks)
            ChatHandler(player->GetSession()).PSendSysMessage(
                "|cffff8800[Raid]|r Warning: insufficient tanks for this content!");
        if (finalHealers < inst.minHealers)
            ChatHandler(player->GetSession()).PSendSysMessage(
                "|cffff8800[Raid]|r Warning: insufficient healers for this content!");
    }

    // ---- Login: reload persisted roster from DB into memory ----
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

    // ---- Logout save: snapshot XP/level for ALL companions before they despawn ----
    void SaveCompanionOnLogout(Player* player)
    {
        for (uint32 entry = 9500080; entry <= 9500094; ++entry)
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

                // Use cached displayName (not GetName() which may return template name)
                std::string savedName = (!cd.displayName.empty()) ? cd.displayName
                                                                  : companion->GetName();
                DB_UpdateCompanionProgress(player->GetGUID().GetCounter(),
                    savedName, cd.currentLevel, cd.xp, cd.killCount);
                // continue - save ALL companions, not just the first one
            }
        }
    }

    void TrySpawnAmbient(Player* player, uint32 zoneId)
    {
        Map* map = player->GetMap();
        if (!map) return;

        // Skip true instances (dungeons/raids)
        if (map->Instanceable())
            return;

        if (player->InBattleground())
            return;

        if (SKIP_ZONES.count(zoneId))
            return;

        // Per-player throttle
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
        uint32 minThreshold = isStarting ? MIN_STARTING_NPCS : MIN_AMBIENT_NPCS;

        if (existing >= minThreshold)
            return;

        uint32 spawnMin = isStarting ? STARTING_SPAWN_MIN : SPAWN_COUNT_MIN;

        // R3: Scale max batch size down when the zone is already busy.
        // Solo/duo players get a full crowd; large groups see fewer extra NPCs.
        uint32 baseCap;
        {
            uint32 nearPlayers = map->GetPlayersCountExceptGMs();
            if      (nearPlayers <= 1) baseCap = SPAWN_COUNT_MAX;
            else if (nearPlayers <= 3) baseCap = SPAWN_COUNT_MAX > 1 ? SPAWN_COUNT_MAX - 1 : SPAWN_COUNT_MAX;
            else if (nearPlayers <= 6) baseCap = SPAWN_COUNT_MAX > 2 ? SPAWN_COUNT_MAX - 2 : spawnMin;
            else                       baseCap = spawnMin;  // lots of real players around — minimal filler
        }
        uint32 spawnMax = isStarting ? STARTING_SPAWN_MAX : std::max(spawnMin, baseCap);
        uint32 needed   = urand(spawnMin, spawnMax);
        // B4: Traveling groups — 25% chance to cluster 2-3 NPCs near a shared home point
        bool   groupActive      = false;
        uint32 groupMembersLeft = 0;
        float  groupX = 0.f, groupY = 0.f, groupZ = 0.f;

        uint32 spawned = 0;

        // R2: Cache time-of-day once per spawn batch
        bool isNightSpawn = false;
        {
            time_t nt     = time(nullptr);
            int    secs   = int(nt % 86400);
            isNightSpawn  = (secs >= 72000 || secs < 21600);  // 20:00 – 06:00
        }

        for (uint32 i = 0; i < needed; ++i)
        {
            uint32 entry = PickAmbientEntry(player);

            // R2: Rogue archetype (stealthy, shadowy) only spawns under cover of darkness.
            // During daytime swap them to the warrior-equivalent of the same faction.
            if (!isNightSpawn && (entry == 9500084 || entry == 9500089))
                entry = (entry == 9500084) ? 9500080 : 9500085;

            if (!sObjectMgr->GetCreatureTemplate(entry))
            {
                TC_LOG_ERROR("scripts", "AmbientNPC: missing template for entry %u", entry);
                continue;
            }

            // B4: Decide spawn position — new group, extend existing group, or random
            Position pos;
            if (!groupActive && urand(0, 3) == 0)  // 25% chance to start a group
            {
                pos          = RandomPositionNear(player);
                groupX       = pos.GetPositionX();
                groupY       = pos.GetPositionY();
                groupZ       = pos.GetPositionZ();
                groupActive      = true;
                groupMembersLeft = urand(1, 2);  // 1 or 2 more companions in this cluster
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

            // Scale level to player's level ±5 for natural variation
            uint8 plvl  = player->getLevel();
            int32 offset = (int32)urand(0, 10) - 5;  // -5 to +5
            uint8 lvl   = (uint8)std::max(1, std::min(110, (int32)plvl + offset));
            s->SetLevel(lvl);

            // Scale HP to level so NPCs aren't trivially one-shot at endgame
            // Formula: 100*L^2 + 500*L + 1000  (~1 M HP at L100, ~1.3 M at L110)
            {
                uint32 hp = uint32(100u * lvl * lvl + 500u * lvl + 1000u);
                s->SetMaxHealth(hp);
                s->SetHealth(hp);
            }

            // Scale weapon damage to level — the template default (BaseAttackTime=0,
            // Legion scaling) can produce wildly wrong values at low levels.
            // Keep damage modest so these NPCs are flavour, not balance-breakers.
            {
                float minDmg = float(lvl) * 0.90f;
                float maxDmg = float(lvl) * 1.35f;
                s->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, minDmg);
                s->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, maxDmg);
                s->UpdateDamagePhysical(BASE_ATTACK);
            }

            // Culture-appropriate random name
            s->SetName(AmbientNames::Roll(entry));

            // Zone-based race appearance: 70 % primary race, 30 % racial variety
            // Faction-gated: Alliance entries never show Horde models; Horde entries never show Alliance models.
            // Neutral entries (9500090-94) follow the spawning player's faction to avoid
            // Night Elf / Human models appearing in Horde starting zones and vice versa.
            {
                auto raceIt = ZONE_RACE_MAP.find(zoneId);
                if (raceIt != ZONE_RACE_MAP.end())
                {
                    const ZoneRacePool& pool = raceIt->second;
                    bool usePrimary = !pool.primary.empty() && (urand(0, 9) < 7);
                    bool isAllianceEntry = (entry >= 9500080 && entry <= 9500084);
                    bool isHordeEntry    = (entry >= 9500085 && entry <= 9500089);
                    // Neutral entries adopt the spawning player's faction for display filtering
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
                    // If primary/secondary pool was entirely wrong-faction, try the alternate pool
                    if (filtered.empty())
                        filtered = FilterPool(usePrimary ? pool.secondary : pool.primary);
                    // Last resort: pick any display from the DB template (don't revert to wrong-faction pool)
                    if (!filtered.empty())
                        s->SetDisplayId(filtered[urand(0, uint32(filtered.size()) - 1u)]);
                }
            }

            // Mount riders at appropriate levels
            {
                uint32 mountId = GetMountDisplayId(entry, lvl);
                if (mountId)
                    s->Mount(mountId);
            }

            ++spawned;
        }

        TC_LOG_DEBUG("scripts", "AmbientNPC: [%s] spawned %u/%u in zone %u (map %u)",
            player->GetName().c_str(), spawned, needed, zoneId, player->GetMapId());
    }
};

std::mutex AmbientWorldPlayerScript::s_mutex;
std::map<ObjectGuid, std::chrono::steady_clock::time_point> AmbientWorldPlayerScript::s_nextSpawnTime;

// ============================================================
//  Registration
// ============================================================
void AddSC_npc_ambient_world()
{
    new npc_ambient_ai();
    new AmbientWorldPlayerScript();
}
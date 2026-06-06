/*
 * npc_ambient_data.cpp
 * Pure data tables: name pools, speech pools, zone race maps, mount display IDs
 */

#include "npc_ambient_world.h"

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
        "Aelindra","Faelyn","Sylara","Varethis","Thalion","Isaeryn","Vaelrin",
        "Celindra","Xarven","Ylarven","Zarven","Alorin","Belorin","Celorin",
        "Elorin","Felorin","Gelorin","Helorin","Ilorin","Jalorin","Kalorin",
        "Morthalun","Yunlan","Drevok",
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

    std::string Roll(uint32 npcEntry)
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
} // namespace AmbientNames

// ============================================================
//  Ambient NPC speech lines
// ============================================================
namespace AmbientSpeech
{
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

    const char* Roll(uint32 npcEntry)
    {
        if (npcEntry >= 9500080 && npcEntry <= 9500084)
            return ALLIANCE_LINES[urand(0, ALLIANCE_COUNT - 1)];
        if (npcEntry >= 9500085 && npcEntry <= 9500089)
            return HORDE_LINES[urand(0, HORDE_COUNT - 1)];
        return NEUTRAL_LINES[urand(0, NEUTRAL_COUNT - 1)];
    }
} // namespace AmbientSpeech

// ============================================================
//  Zone data: skip zones, starting zones, race display pools
// ============================================================
const std::set<uint32> SKIP_ZONES =
{
    14,   3487, 3557,
    4395, 7502, 7563,
};

// Capital cities — heavy ambient population
const std::set<uint32> CAPITAL_ZONES =
{
    // Alliance
    1519,  // Stormwind
    1537,  // Ironforge
    1657,  // Darnassus
    3557,  // Exodar — also allow spawning here
    // Horde
    1637,  // Orgrimmar
    1638,  // Thunder Bluff
    1497,  // Undercity
    3487,  // Silvermoon City
    // Neutral
    4395,  // Dalaran (Northrend)
    7502,  // Dalaran (Broken Isles)
    362,   // Gadgetzan / Tanaris hub
};

const std::set<uint32> STARTING_ZONES =
{
    12, 1, 141, 3524, 4987,
    85, 215, 3430, 5170, 4815,
};

const std::set<uint32> ALLIANCE_ONLY_DISPLAY = {
    3167, 3258, 3257, 1598, 1608, 3524, 4408, 4841, 4842,
    11650, 11652, 16602, 29317, 29318, 30215, 2490, 2891, 3562,
};
const std::set<uint32> HORDE_ONLY_DISPLAY = {
    2858, 2855, 1648, 1678, 3797, 18980, 18982, 18981,
    4573, 4551, 7107, 7109, 7108, 4609, 1882, 15574,
};

const std::map<uint32, ZoneRacePool> ZONE_RACE_MAP =
{
    // ---- Alliance starting zones ----
    { 12,   { {3167,3258,3167,3258,3257}, {1598,1608,4408,4841} } },
    { 1,    { {1598,1608,3524,1598,1608}, {3167,3258,2490,2891} } },
    { 141,  { {4408,4841,4842,4408,4841}, {3167,3258,11650,11652} } },
    { 3524, { {11650,11652,16602,11650},  {4408,4841,3167,3258} } },
    { 4987, { {29317,29318,30215,3167},   {1598,1608,4408,4841} } },
    // ---- Horde starting zones ----
    { 85,   { {2858,2855,1648,2858,2855}, {18980,18982,4573,4551,1678} } },
    { 215,  { {1678,3797,1678,3797},      {4573,4551,2858,2855,18980} } },
    { 3430, { {18980,18982,18981,18980},  {4573,4551,2858,2855,1678} } },
    { 4815, { {7107,7109,4573,4551,7108}, {1678,2858,2855,18980} } },
    // ---- Neutral ----
    { 5170, { {29421,29422,29421,29422},  {3167,3258,4573,4551} } },
    // ---- Eastern Kingdoms ----
    { 10,   { {3167,3258,3257,3167,3258}, {29317,29318,2490,2891} } },
    { 40,   { {3167,3258,3257,3167,3258}, {1598,1608,2490,2891} } },
    { 44,   { {3167,3258,3257,1598,1608}, {4408,4841,11650,11652} } },
    { 45,   { {3167,3258,3257,2858,2855}, {1598,1608,4573,4551} } },
    { 33,   { {3167,3258,3257,4609,1882}, {4573,4551,7107,7109} } },
    { 224,  { {3167,3258,3257,4609,1882}, {4573,4551,7107,7109} } },
    { 267,  { {3167,3258,2858,2855,3257}, {4573,4551,1678,3797} } },
    { 28,   { {2858,2855,1648,2858,2855}, {3167,3258,4408,4841} } },
    { 139,  { {2858,2855,1648,3167,3258}, {4408,4841,11650,11652} } },
    { 46,   { {3167,3258,1598,1608,3257}, {4573,4551,2858,2855} } },
    { 3,    { {1598,1608,3524,1598,1608}, {3167,3258,4573,4551} } },
    { 8,    { {3167,3258,3257,18980,18982},{4573,4551,2858,2855} } },
    { 4,    { {3167,3258,3257,4573,4551}, {2858,2855,1678,3797} } },
    // ---- Kalimdor ----
    { 148,  { {4408,4841,4842,4408,4841}, {3167,3258,11650,11652} } },
    { 331,  { {4408,4841,4842,4408,4841}, {4573,4551,4609,1882} } },
    { 17,   { {4573,4551,1678,3797,4573}, {4609,1882,2858,2855} } },
    { 400,  { {1678,3797,1678,3797},      {4573,4551,2490,2891} } },
    { 440,  { {7107,7109,3167,3258,7108}, {4609,1882,4573,4551} } },
    { 357,  { {4408,4841,4842,1678,3797}, {3167,3258,1598,1608} } },
    { 490,  { {3167,3258,3257,7107,7109}, {4573,4551,4609,1882} } },
    // ---- Outland ----
    { 464,  { {4573,4551,3167,3258,4573}, {11650,11652,2858,2855} } },
    { 478,  { {11650,11652,16602,1678},   {3167,3258,4408,4841} } },
    { 477,  { {18980,18982,11650,18981},  {4573,4551,3167,3258} } },
    { 475,  { {1678,3797,4573,4551,1678}, {11650,11652,3167,3258} } },
    { 473,  { {4573,4551,1678,3797,4573}, {11650,11652,3167,3258} } },
    { 476,  { {18980,18982,11650,11652},  {4573,4551,2858,2855} } },
    { 3523, { {18980,18982,11650,11652},  {2490,2891,3167,3258} } },
    // ---- Northrend ----
    { 3537, { {3167,3258,3257,1598,1608}, {1678,3797,4573,4551} } },
    { 495,  { {3167,3258,3257,2858,2855}, {1598,1608,4573,4551} } },
    { 65,   { {3167,3258,2858,2855,3257}, {1678,3797,4408,4841} } },
    { 394,  { {3167,3258,2858,2855,4573}, {1598,1608,1678,3797} } },
    // ---- Cataclysm ----
    { 616,  { {4408,4841,4842,1678,3797}, {3167,3258,4573,4551} } },
    { 545,  { {1598,1608,3524,4573,4551}, {3167,3258,18980,18982} } },
    { 540,  { {3167,3258,3257,7107,7109}, {11650,11652,4573,4551} } },
    // ---- Pandaria ----
    { 376,  { {29421,29422,3167,3258},    {4408,4841,4573,4551} } },
    { 379,  { {29421,29422,4573,4551},    {1678,3797,3167,3258} } },
    { 371,  { {29421,29422,3167,3258},    {1678,3797,4573,4551} } },
    // ---- Draenor ----
    { 534,  { {4573,4551,3167,3258,4573}, {1598,1608,1678,3797} } },
    { 543,  { {4573,4551,1678,3797,4573}, {3167,3258,11650,11652} } },
    { 539,  { {18980,18982,11650,11652},  {4573,4551,3167,3258} } },
    // ---- Broken Isles ----
    { 630,  { {4408,4841,4842,18980,18982},{3167,3258,11650,11652} } },
    { 641,  { {4408,4841,4842,1678,3797}, {3167,3258,4573,4551} } },
    { 634,  { {1678,3797,4408,4841,1678}, {4573,4551,3167,3258} } },
    { 628,  { {3167,3258,2858,2855,3257}, {4573,4551,1598,1608} } },
    { 708,  { {18980,18982,4408,4841},    {3167,3258,11650,11652} } },
};

// ============================================================
//  Role system functions
// ============================================================
AmbientRole GetRoleForEntry(uint32 entry)
{
    switch (entry)
    {
        case 9500080: return AMBIENT_WARRIOR;
        case 9500081: return (urand(0,1)) ? AMBIENT_TANK : AMBIENT_HEALER;
        case 9500082: return AMBIENT_MAGE;
        case 9500083: return AMBIENT_HUNTER;
        case 9500084: return AMBIENT_ROGUE;
        case 9500085: return AMBIENT_WARRIOR;
        case 9500086: return AMBIENT_HEALER;
        case 9500087: return AMBIENT_HUNTER;
        case 9500088: return AMBIENT_MAGE;
        case 9500089: return AMBIENT_ROGUE;
        case 9500090: return AMBIENT_TANK;
        case 9500091: return AMBIENT_ROGUE;
        case 9500092: return (urand(0,9) < 4) ? AMBIENT_TANK : AMBIENT_HEALER;
        case 9500093: return (urand(0,1)) ? AMBIENT_TANK : AMBIENT_HEALER;
        case 9500094: return AMBIENT_HEALER;
        default:      return AMBIENT_DEFAULT;
    }
}

uint32 GetCombatEmote(AmbientRole role)
{
    switch (role)
    {
        case AMBIENT_WARRIOR: case AMBIENT_ROGUE: return EMOTE_ONESHOT_ATTACK1H;
        case AMBIENT_TANK:    return EMOTE_ONESHOT_ATTACK1H;
        case AMBIENT_HUNTER:  return EMOTE_ONESHOT_ATTACK_BOW;
        case AMBIENT_MAGE:    return EMOTE_ONESHOT_SPELL_CAST_W_SOUND;
        case AMBIENT_HEALER:  return EMOTE_ONESHOT_SPELL_CAST;
        default:              return EMOTE_ONESHOT_ATTACK1H;
    }
}

const char* RoleName(AmbientRole role)
{
    switch (role)
    {
        case AMBIENT_TANK:    return "Tank";
        case AMBIENT_HEALER:  return "Healer";
        default:              return "DPS";
    }
}

uint32 EntryForRole(AmbientRole role)
{
    switch (role)
    {
        case AMBIENT_TANK:    return 9500090;
        case AMBIENT_HEALER:  return 9500086;
        case AMBIENT_MAGE:    return 9500082;
        case AMBIENT_HUNTER:  return 9500083;
        case AMBIENT_ROGUE:   return 9500084;
        case AMBIENT_WARRIOR: return 9500080;
        default:              return 9500085;
    }
}

// ============================================================
//  Mount display ID pools
// ============================================================
static const uint32 MOUNTS_ALLIANCE[] = {
    2411, 2414, 2417, 2418, 4802, 4805, 10045, 10046, 4491, 19481,
};
static constexpr uint32 MOUNTS_ALLIANCE_COUNT = 10;

static const uint32 MOUNTS_HORDE[] = {
    2607, 2610, 2612, 9470, 6467, 6471, 9345, 22723, 9471,
};
static constexpr uint32 MOUNTS_HORDE_COUNT = 9;

static const uint32 MOUNTS_NEUTRAL[] = {
    25162, 4080, 2411, 2607, 10045, 6467,
};
static constexpr uint32 MOUNTS_NEUTRAL_COUNT = 6;

static const uint32 MOUNTS_EPIC[] = {
    14349, 13334, 9470, 2411, 2607, 10051,
};
static constexpr uint32 MOUNTS_EPIC_COUNT = 6;

uint32 GetMountDisplayId(uint32 npcEntry, uint8 level)
{
    if (level < 20) return 0;
    uint32 roll   = urand(0, 99);
    uint32 chance = (level >= 80) ? 75 : (level >= 60) ? 60 : (level >= 40) ? 45 : 30;
    if (roll >= chance) return 0;
    if (level >= 80 && urand(0, 2) == 0)
        return MOUNTS_EPIC[urand(0, MOUNTS_EPIC_COUNT - 1)];
    if (npcEntry >= 9500080 && npcEntry <= 9500084)
        return MOUNTS_ALLIANCE[urand(0, MOUNTS_ALLIANCE_COUNT - 1)];
    if (npcEntry >= 9500085 && npcEntry <= 9500089)
        return MOUNTS_HORDE[urand(0, MOUNTS_HORDE_COUNT - 1)];
    if (npcEntry == 9500090) return 25162;
    return MOUNTS_NEUTRAL[urand(0, MOUNTS_NEUTRAL_COUNT - 1)];
}

// ============================================================
//  Instance sizing
// ============================================================
InstanceSize GetInstanceSize(Map* map)
{
    if (!map->IsRaid())
    {
        InstanceSize s; s.total = 5; s.minTanks = 1; s.minHealers = 1; return s;
    }
    uint32 diff = (uint32)map->GetDifficultyID();
    if (diff == 3 || diff == 5) { InstanceSize s; s.total = 10; s.minTanks = 2; s.minHealers = 2; return s; }
    if (diff == 4 || diff == 6) { InstanceSize s; s.total = 25; s.minTanks = 2; s.minHealers = 6; return s; }
    if (diff == 9)              { InstanceSize s; s.total = 40; s.minTanks = 3; s.minHealers = 8; return s; }
    if (diff == 16)             { InstanceSize s; s.total = 20; s.minTanks = 2; s.minHealers = 4; return s; }
    InstanceSize s; s.total = 25; s.minTanks = 2; s.minHealers = 5; return s;
}

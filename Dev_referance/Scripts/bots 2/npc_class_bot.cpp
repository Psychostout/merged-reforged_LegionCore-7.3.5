/*
 * BotSystem Phase 4 — Class-Based Party Bots
 * npc_class_bot.cpp
 *
 * 20 distinct class archetypes + 1 Bot Captain hireable as companions.
 * Players may hire up to 3 bots simultaneously (full party of 4 w/ player).
 *
 * TANKS  (entries 9500020-9500024)
 *   9500020 Donaveth Ironshield   — Protection Warrior
 *   9500021 Aelindra Dawnthorn    — Protection Paladin
 *   9500022 Morticus Grimholt     — Blood Death Knight
 *   9500023 Grothak Stoneguard    — Guardian Druid
 *   9500024 Vayne Shadowscar      — Vengeance Demon Hunter
 *
 * HEALERS (entries 9500025-9500029)
 *   9500025 Sister Elariene       — Holy Priest
 *   9500026 Wildmist Elowen       — Restoration Druid
 *   9500027 Tidecaller Mazu       — Restoration Shaman
 *   9500028 Dawnlight Caelan      — Holy Paladin
 *   9500029 Spiritweave Mei       — Mistweaver Monk
 *
 * RANGED DPS (entries 9500030-9500034)
 *   9500030 Pyremancer Aldric     — Fire Mage
 *   9500031 Felbound Serath       — Affliction Warlock
 *   9500032 Swiftarrow Kael       — Beast Mastery Hunter
 *   9500033 Moonwarden Syla       — Balance Druid
 *   9500034 Shadowveil Cyrin      — Shadow Priest
 *
 * MELEE DPS (entries 9500035-9500039)
 *   9500035 Daggerfall Rin        — Assassination Rogue
 *   9500036 Stormcaller Korruk    — Enhancement Shaman
 *   9500037 Crusader Arendor      — Retribution Paladin
 *   9500038 Deathblade Razik      — Unholy Death Knight
 *   9500039 Felrend Taelith       — Havoc Demon Hunter
 *
 * BOT CAPTAIN (entry 9500050)
 *   9500050 Commander Talindra    — Any class via submenu
 *
 * Drop into: src/server/scripts/Custom/npc_class_bot.cpp
 * Register in custom_script_loader.cpp:
 *   void AddSC_npc_class_bot();
 *   AddSC_npc_class_bot();
 */

#include <iosfwd>
#include <string>
#include "Chat.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Group.h"
#include "Log.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "SpellMgr.h"
#include "Unit.h"
#include <map>
#include <set>
#include <vector>

// ============================================================
//  Constants
// ============================================================

static const uint32 BOT_CAPTAIN_ENTRY     = 9500050;
static const uint32 MAX_BOTS_PER_PLAYER   = 3;
static const float  BOT_FOLLOW_DIST       = 3.5f;
static const float  BOT_FOLLOW_ANGLE_BACK = float(M_PI);        // behind player
static const float  BOT_FOLLOW_ANGLE_SIDE = float(M_PI * 0.75f);// side-rear
static const uint32 NPC_TEXT_CLASS_BOT    = 91003;
static const uint32 NPC_TEXT_BOT_CAPTAIN  = 91004;
static const uint32 BOT_UPDATE_INTERVAL   = 1500;

// Gossip sender
#define GSENDER 1

// Individual bot actions
#define ACT_HIRE    1
#define ACT_DISMISS 2

// Captain navigation actions
#define CAP_NAV_TANKS   100
#define CAP_NAV_HEALERS 101
#define CAP_NAV_RANGED  102
#define CAP_NAV_MELEE   103
#define CAP_NAV_BACK    104
#define CAP_DISMISS_ALL 105

// Captain hire actions (200+)
#define HIRE_WARRIOR_TANK  201
#define HIRE_PALADIN_TANK  202
#define HIRE_DK_TANK       203
#define HIRE_DRUID_TANK    204
#define HIRE_DH_TANK       205
#define HIRE_PRIEST_HEAL   210
#define HIRE_DRUID_HEAL    211
#define HIRE_SHAMAN_HEAL   212
#define HIRE_PALADIN_HEAL  213
#define HIRE_MONK_HEAL     214
#define HIRE_MAGE          220
#define HIRE_WARLOCK       221
#define HIRE_HUNTER        222
#define HIRE_BOOMKIN       223
#define HIRE_SPRIEST       224
#define HIRE_ROGUE         230
#define HIRE_ENH_SHAMAN    231
#define HIRE_RET_PALADIN   232
#define HIRE_DK_DPS        233
#define HIRE_DH_DPS        234

// ============================================================
//  Bot Role Enum
// ============================================================

enum BotClassRole : uint8
{
    BOT_NONE          = 0,
    // Tanks
    BOT_WARRIOR_TANK  = 1,
    BOT_PALADIN_TANK  = 2,
    BOT_DK_TANK       = 3,
    BOT_DRUID_TANK    = 4,
    BOT_DH_TANK       = 5,
    // Healers
    BOT_PRIEST_HEAL   = 10,
    BOT_DRUID_HEAL    = 11,
    BOT_SHAMAN_HEAL   = 12,
    BOT_PALADIN_HEAL  = 13,
    BOT_MONK_HEAL     = 14,
    // Ranged DPS
    BOT_MAGE          = 20,
    BOT_WARLOCK       = 21,
    BOT_HUNTER        = 22,
    BOT_BOOMKIN       = 23,
    BOT_SPRIEST       = 24,
    // Melee DPS
    BOT_ROGUE         = 30,
    BOT_ENH_SHAMAN    = 31,
    BOT_RET_PALADIN   = 32,
    BOT_DK_DPS        = 33,
    BOT_DH_DPS        = 34,
    // Captain wildcard
    BOT_CAPTAIN       = 99
};

// ============================================================
//  Role → NPC entry mapping
// ============================================================

static BotClassRole GetRoleForEntry(uint32 entry)
{
    switch (entry)
    {
        case 9500020: return BOT_WARRIOR_TANK;
        case 9500021: return BOT_PALADIN_TANK;
        case 9500022: return BOT_DK_TANK;
        case 9500023: return BOT_DRUID_TANK;
        case 9500024: return BOT_DH_TANK;
        case 9500025: return BOT_PRIEST_HEAL;
        case 9500026: return BOT_DRUID_HEAL;
        case 9500027: return BOT_SHAMAN_HEAL;
        case 9500028: return BOT_PALADIN_HEAL;
        case 9500029: return BOT_MONK_HEAL;
        case 9500030: return BOT_MAGE;
        case 9500031: return BOT_WARLOCK;
        case 9500032: return BOT_HUNTER;
        case 9500033: return BOT_BOOMKIN;
        case 9500034: return BOT_SPRIEST;
        case 9500035: return BOT_ROGUE;
        case 9500036: return BOT_ENH_SHAMAN;
        case 9500037: return BOT_RET_PALADIN;
        case 9500038: return BOT_DK_DPS;
        case 9500039: return BOT_DH_DPS;
        case 9500050: return BOT_CAPTAIN;
        default:      return BOT_NONE;
    }
}

static const char* GetClassLabel(BotClassRole role)
{
    switch (role)
    {
        case BOT_WARRIOR_TANK: return "Protection Warrior";
        case BOT_PALADIN_TANK: return "Protection Paladin";
        case BOT_DK_TANK:      return "Blood Death Knight";
        case BOT_DRUID_TANK:   return "Guardian Druid";
        case BOT_DH_TANK:      return "Vengeance Demon Hunter";
        case BOT_PRIEST_HEAL:  return "Holy Priest";
        case BOT_DRUID_HEAL:   return "Restoration Druid";
        case BOT_SHAMAN_HEAL:  return "Restoration Shaman";
        case BOT_PALADIN_HEAL: return "Holy Paladin";
        case BOT_MONK_HEAL:    return "Mistweaver Monk";
        case BOT_MAGE:         return "Fire Mage";
        case BOT_WARLOCK:      return "Affliction Warlock";
        case BOT_HUNTER:       return "Beast Mastery Hunter";
        case BOT_BOOMKIN:      return "Balance Druid";
        case BOT_SPRIEST:      return "Shadow Priest";
        case BOT_ROGUE:        return "Assassination Rogue";
        case BOT_ENH_SHAMAN:   return "Enhancement Shaman";
        case BOT_RET_PALADIN:  return "Retribution Paladin";
        case BOT_DK_DPS:       return "Unholy Death Knight";
        case BOT_DH_DPS:       return "Havoc Demon Hunter";
        case BOT_CAPTAIN:      return "Veteran Mercenary";
        default:               return "Adventurer";
    }
}

static bool IsHealerRole(BotClassRole role)
{
    return role >= BOT_PRIEST_HEAL && role <= BOT_MONK_HEAL;
}

static bool IsTankRole(BotClassRole role)
{
    return role >= BOT_WARRIOR_TANK && role <= BOT_DH_TANK;
}

static bool IsPureCasterRole(BotClassRole role)
{
    // These bots should not chase target in melee
    return role == BOT_MAGE || role == BOT_WARLOCK ||
           role == BOT_BOOMKIN || role == BOT_SPRIEST;
}

static uint32 GetPrimaryHealSpell(BotClassRole role)
{
    switch (role)
    {
        case BOT_PRIEST_HEAL:  return 2061;   // Flash Heal
        case BOT_DRUID_HEAL:   return 8936;   // Regrowth
        case BOT_SHAMAN_HEAL:  return 331;    // Healing Wave
        case BOT_PALADIN_HEAL: return 19750;  // Flash of Light
        case BOT_MONK_HEAL:    return 124682; // Enveloping Mist
        default:               return 0;
    }
}

static uint32 GetOutOfCombatBuff(BotClassRole role)
{
    switch (role)
    {
        case BOT_WARRIOR_TANK:
        case BOT_DK_TANK:
        case BOT_DK_DPS:       return 6673;   // Battle Shout
        case BOT_PALADIN_TANK:
        case BOT_PALADIN_HEAL:
        case BOT_RET_PALADIN:  return 20217;  // Blessing of Kings
        case BOT_PRIEST_HEAL:
        case BOT_SPRIEST:      return 21562;  // Power Word: Fortitude
        case BOT_MAGE:         return 1459;   // Arcane Intellect
        case BOT_DRUID_TANK:
        case BOT_DRUID_HEAL:
        case BOT_BOOMKIN:      return 1126;   // Mark of the Wild
        case BOT_SHAMAN_HEAL:
        case BOT_ENH_SHAMAN:   return 0;      // Shaman buffs complex, skip
        case BOT_MONK_HEAL:    return 0;      // Skip for simplicity
        default:               return 0;
    }
}

// ============================================================
//  Global ownership tracking (single-threaded map updates)
// ============================================================

static std::map<ObjectGuid, std::set<ObjectGuid>> s_ownerToBots;
static std::map<ObjectGuid, ObjectGuid>           s_botToOwner;

static uint32 GetPlayerBotCount(ObjectGuid ownerGuid)
{
    auto it = s_ownerToBots.find(ownerGuid);
    return it == s_ownerToBots.end() ? 0u : (uint32)it->second.size();
}

static void RegisterBot(ObjectGuid ownerGuid, ObjectGuid botGuid)
{
    s_ownerToBots[ownerGuid].insert(botGuid);
    s_botToOwner[botGuid] = ownerGuid;
}

static void UnregisterBot(ObjectGuid botGuid)
{
    auto it = s_botToOwner.find(botGuid);
    if (it == s_botToOwner.end())
        return;
    ObjectGuid ownerGuid = it->second;
    s_ownerToBots[ownerGuid].erase(botGuid);
    if (s_ownerToBots[ownerGuid].empty())
        s_ownerToBots.erase(ownerGuid);
    s_botToOwner.erase(it);
}

// ============================================================
//  Bot AI
// ============================================================

struct npc_class_botAI : public ScriptedAI
{
    npc_class_botAI(Creature* creature) : ScriptedAI(creature)
    {
        _ownerGuid.Clear();
        _role        = GetRoleForEntry(creature->GetEntry());
        _isFollowing = false;
        _updateTimer = BOT_UPDATE_INTERVAL;
        _timer1 = 0;
        _timer2 = 0;
        _timer3 = 0;
        _healTimer  = 0;
        _buffTimer  = 5000; // first buff 5 s after spawn
    }

    // ----------------------------------------------------------
    //  Hire / dismiss
    // ----------------------------------------------------------
    void HireBot(Player* player, BotClassRole role)
    {
        if (_isFollowing)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("This companion is already engaged.");
            return;
        }
        if (GetPlayerBotCount(player->GetGUID()) >= MAX_BOTS_PER_PLAYER)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("You already have a full party of companions. Dismiss one first.");
            return;
        }

        // For Captain: use the chosen role; for individual bots: keep fixed role
        if (me->GetEntry() == BOT_CAPTAIN_ENTRY)
            _role = role;

        _ownerGuid   = player->GetGUID();
        _isFollowing = true;
        RegisterBot(_ownerGuid, me->GetGUID());

        // Scale to player's level
        me->SetLevel(std::max<uint8>(1, player->getLevel()));

        // Adjust combat movement for pure casters
        SetCombatMovement(!IsPureCasterRole(_role));

        // Follow at player's back/side
        float angle = IsTankRole(_role) ? BOT_FOLLOW_ANGLE_SIDE : BOT_FOLLOW_ANGLE_BACK;
        me->GetMotionMaster()->Clear(false);
        me->GetMotionMaster()->MoveFollow(player, BOT_FOLLOW_DIST, angle);

        // Reset spell timers so abilities fire immediately in first fight
        _timer1 = 0; _timer2 = 0; _timer3 = 0; _healTimer = 0;

        ChatHandler(player->GetSession()).PSendSysMessage(
            "%s is ready to fight as your %s!",
            me->GetName().c_str(), GetClassLabel(_role));
    }

    void DismissBot(bool returnToSpawn = true)
    {
        UnregisterBot(me->GetGUID());
        _ownerGuid.Clear();
        _isFollowing = false;

        me->GetMotionMaster()->Clear(false);
        me->CombatStop(true);

        // Restore original level and name
        me->SetLevel(std::max<uint8>(1, (uint8)me->GetCreatureTemplate()->minlevel));

        SetCombatMovement(true); // reset for next hire

        if (returnToSpawn)
            me->GetMotionMaster()->MoveTargetedHome();

        me->SetFullHealth();
        _role = GetRoleForEntry(me->GetEntry()); // reset Captain back to CAPTAIN role
    }

    // ----------------------------------------------------------
    //  Combat AI per class
    // ----------------------------------------------------------
    void TickTimers(uint32 diff)
    {
        if (_timer1 > diff) _timer1 -= diff; else _timer1 = 0;
        if (_timer2 > diff) _timer2 -= diff; else _timer2 = 0;
        if (_timer3 > diff) _timer3 -= diff; else _timer3 = 0;
        if (_healTimer > diff) _healTimer -= diff; else _healTimer = 0;
    }

    void TauntOwnerAttacker(Player* owner, uint32 tauntSpell)
    {
        if (!owner) return;
        for (auto const& ref : owner->getAttackers())
            if (Unit* att = ref) { DoCast(att, tauntSpell); return; }
    }

    void HealOwnerIf(Player* owner, uint32 healSpell, float threshold)
    {
        if (!owner || !owner->IsAlive()) return;
        if (_healTimer > 0) return;
        if (owner->GetHealthPct() < threshold)
        {
            DoCast(owner, healSpell);
            _healTimer = 2800;
        }
    }

    void DoClassAbilities(uint32 diff, Player* owner)
    {
        TickTimers(diff);

        switch (_role)
        {
            // ---- TANKS ----------------------------------------
            case BOT_WARRIOR_TANK:
                if (_timer1 == 0) { DoCast(me, 6343);        _timer1 = 4500; }  // Thunder Clap (AoE)
                if (_timer2 == 0) { TauntOwnerAttacker(owner, 355); _timer2 = 8000; } // Taunt
                if (_timer3 == 0) { DoCastVictim(23922);     _timer3 = 20000; } // Shield Slam
                DoMeleeAttackIfReady();
                break;

            case BOT_PALADIN_TANK:
                if (_timer1 == 0) { DoCast(me, 26573);       _timer1 = 8000; }  // Consecration
                if (_timer2 == 0) { TauntOwnerAttacker(owner, 62124); _timer2 = 8000; } // Hand of Reckoning
                if (_timer3 == 0) { DoCastVictim(20271);     _timer3 = 6000; }  // Judgment
                DoMeleeAttackIfReady();
                break;

            case BOT_DK_TANK:
                if (_timer1 == 0) { DoCastVictim(49576);     _timer1 = 25000; } // Death Grip
                if (_timer2 == 0 && me->GetHealthPct() < 55.0f)
                               { DoCast(me, 49998);          _timer2 = 3500;  } // Death Strike (self-heal)
                if (_timer3 == 0) { TauntOwnerAttacker(owner, 56222); _timer3 = 8000;  } // Rune of Darkness
                DoMeleeAttackIfReady();
                break;

            case BOT_DRUID_TANK:
                if (_timer1 == 0) { DoCast(me, 77758);       _timer1 = 4500; }  // Thrash
                if (_timer2 == 0) { TauntOwnerAttacker(owner, 6795); _timer2 = 8000;  } // Growl
                if (_timer3 == 0 && me->GetHealthPct() < 40.0f)
                               { DoCast(me, 22812);          _timer3 = 60000; } // Barkskin
                DoMeleeAttackIfReady();
                break;

            case BOT_DH_TANK:
                if (_timer1 == 0) { DoCastVictim(203783);    _timer1 = 4500; }  // Shear
                if (_timer2 == 0) { TauntOwnerAttacker(owner, 185245); _timer2 = 8000; } // Torment
                if (_timer3 == 0 && me->GetHealthPct() < 50.0f)
                               { DoCast(me, 203720);         _timer3 = 14000; } // Demon Spikes
                DoMeleeAttackIfReady();
                break;

            // ---- HEALERS -------------------------------------
            case BOT_PRIEST_HEAL:
                HealOwnerIf(owner, 2061, 72.0f);                              // Flash Heal
                if (_timer2 == 0) { DoCast(owner, 139);      _timer2 = 14000; } // Renew (HoT)
                if (_timer3 == 0 && owner && owner->GetHealthPct() < 20.0f)
                               { DoCast(owner, 47788);       _timer3 = 180000; }// Guardian Spirit
                if (me->GetHealthPct() < 50.0f && _healTimer > 100)
                    DoCast(me, 2061); // Self Flash Heal when low
                break;

            case BOT_DRUID_HEAL:
                HealOwnerIf(owner, 8936, 68.0f);                              // Regrowth
                if (_timer2 == 0) { DoCast(owner, 774);      _timer2 = 12000; } // Rejuvenation (HoT)
                if (_timer3 == 0 && owner && owner->GetHealthPct() < 50.0f)
                               { DoCast(owner, 48438);       _timer3 = 10000; } // Wild Growth
                break;

            case BOT_SHAMAN_HEAL:
                HealOwnerIf(owner, 331, 70.0f);                               // Healing Wave
                if (_timer2 == 0 && owner && owner->GetHealthPct() < 50.0f)
                               { DoCast(owner, 1064);        _timer2 = 25000; } // Chain Heal
                break;

            case BOT_PALADIN_HEAL:
                HealOwnerIf(owner, 19750, 72.0f);                             // Flash of Light
                if (_timer2 == 0 && owner && owner->GetHealthPct() < 45.0f)
                               { DoCast(owner, 82326);       _timer2 = 4000;  } // Holy Light
                if (_timer3 == 0 && owner && owner->GetHealthPct() < 12.0f)
                               { DoCast(owner, 633);         _timer3 = 3600000; }// Lay on Hands (1hr CD)
                break;

            case BOT_MONK_HEAL:
                HealOwnerIf(owner, 124682, 72.0f);                            // Enveloping Mist
                if (_timer2 == 0) { DoCast(owner, 119611);   _timer2 = 9000;  } // Renewing Mist (HoT)
                break;

            // ---- RANGED DPS ----------------------------------
            case BOT_MAGE:
                if (_timer1 == 0) { DoCastVictim(133);       _timer1 = 2500; }  // Fireball
                if (_timer2 == 0) { DoCastVictim(2948);      _timer2 = 4000; }  // Scorch
                if (_timer3 == 0) { DoCastVictim(122);       _timer3 = 30000; } // Frost Nova (CC utility)
                break;

            case BOT_WARLOCK:
                if (_timer1 == 0) { DoCastVictim(686);       _timer1 = 3000; }  // Shadow Bolt
                if (_timer2 == 0) { DoCastVictim(172);       _timer2 = 18000; } // Corruption (DoT)
                if (_timer3 == 0) { DoCastVictim(30108);     _timer3 = 8000; }  // Unstable Affliction
                break;

            case BOT_HUNTER:
                if (_timer1 == 0) { DoCastVictim(3044);      _timer1 = 3000; }  // Arcane Shot
                if (_timer2 == 0) { DoCast(me, 2643);        _timer2 = 8000; }  // Multi-Shot (AoE)
                if (_timer3 == 0 && me->GetVictim() && me->GetVictim()->GetHealthPct() < 25.0f)
                               { DoCastVictim(53351);        _timer3 = 10000; } // Kill Shot
                DoMeleeAttackIfReady(); // Hunter keeps melee fallback
                break;

            case BOT_BOOMKIN:
                if (_timer1 == 0) { DoCastVictim(197626);    _timer1 = 6000; }  // Starsurge
                if (_timer2 == 0) { DoCastVictim(8921);      _timer2 = 12000; } // Moonfire (DoT)
                if (_timer3 == 0) { DoCastVictim(93402);     _timer3 = 12000; } // Sunfire (DoT)
                break;

            case BOT_SPRIEST:
                if (_timer1 == 0) { DoCastVictim(8092);      _timer1 = 1500; }  // Mind Blast
                if (_timer2 == 0) { DoCastVictim(589);       _timer2 = 12000; } // Shadow Word: Pain
                if (_timer3 == 0) { DoCastVictim(34914);     _timer3 = 15000; } // Vampiric Touch
                break;

            // ---- MELEE DPS -----------------------------------
            case BOT_ROGUE:
                if (_timer1 == 0) { DoCastVictim(1752);      _timer1 = 2000; }  // Sinister Strike
                if (_timer2 == 0)                                               // Kick (interrupt)
                {
                    if (Unit* v = me->GetVictim())
                        if (v->IsNonMeleeSpellCast(false))
                            DoCast(v, 1766);
                    _timer2 = 8000;
                }
                if (_timer3 == 0) { DoCastVictim(408);       _timer3 = 20000; } // Kidney Shot (stun)
                DoMeleeAttackIfReady();
                break;

            case BOT_ENH_SHAMAN:
                if (_timer1 == 0) { DoCastVictim(17364);     _timer1 = 4500; }  // Stormstrike
                if (_timer2 == 0) { DoCastVictim(60103);     _timer2 = 8000; }  // Lava Lash
                if (_timer3 == 0) { DoCastVictim(8050);      _timer3 = 6000; }  // Flame Shock (DoT)
                DoMeleeAttackIfReady();
                break;

            case BOT_RET_PALADIN:
                if (_timer1 == 0) { DoCastVictim(35395);     _timer1 = 4500; }  // Crusader Strike
                if (_timer2 == 0) { DoCastVictim(20271);     _timer2 = 6000; }  // Judgment
                if (_timer3 == 0) { DoCast(me, 53385);       _timer3 = 12000; } // Divine Storm (AoE)
                DoMeleeAttackIfReady();
                break;

            case BOT_DK_DPS:
                if (_timer1 == 0) { DoCastVictim(55090);     _timer1 = 4500; }  // Scourge Strike
                if (_timer2 == 0) { DoCastVictim(47541);     _timer2 = 8000; }  // Death Coil
                if (_timer3 == 0) { DoCastVictim(85948);     _timer3 = 15000; } // Festering Strike
                DoMeleeAttackIfReady();
                break;

            case BOT_DH_DPS:
                if (_timer1 == 0) { DoCastVictim(162794);    _timer1 = 4500; }  // Chaos Strike
                if (_timer2 == 0) { DoCastVictim(185123);    _timer2 = 6000; }  // Throw Glaive (ranged)
                if (_timer3 == 0 && me->GetHealthPct() < 35.0f)
                               { DoCast(me, 198589);         _timer3 = 60000; } // Blur (DPS defensive)
                DoMeleeAttackIfReady();
                break;

            default:
                DoMeleeAttackIfReady();
                break;
        }
    }

    void ApplyOutOfCombatBuff(Player* owner)
    {
        if (!owner || !owner->IsAlive() || !owner->IsInWorld()) return;
        uint32 buffSpell = GetOutOfCombatBuff(_role);
        if (buffSpell)
            DoCast(owner, buffSpell);
    }

    // ----------------------------------------------------------
    //  ScriptedAI overrides
    // ----------------------------------------------------------
    void Reset() override
    {
        if (_isFollowing)
            DismissBot(false);
        _timer1 = 0; _timer2 = 0; _timer3 = 0;
        _healTimer   = 0;
        _buffTimer   = 30000;
        _updateTimer = BOT_UPDATE_INTERVAL;
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (Player* owner = ObjectAccessor::GetPlayer(*me, _ownerGuid))
            ChatHandler(owner->GetSession()).PSendSysMessage("%s has fallen!", me->GetName().c_str());

        UnregisterBot(me->GetGUID());
        _isFollowing = false;
        _ownerGuid.Clear();
    }

    void UpdateAI(uint32 diff) override
    {
        // Tick heal timer everywhere
        if (_healTimer > diff) _healTimer -= diff; else _healTimer = 0;

        // ─── IN COMBAT ───────────────────────────────────────────
        if (me->IsInCombat())
        {
            if (!_isFollowing)
            {
                // Not hired — just melee if attacked
                DoMeleeAttackIfReady();
                return;
            }

            Player* owner = ObjectAccessor::GetPlayer(*me, _ownerGuid);
            if (!owner || !owner->IsAlive())
            {
                me->CombatStop(true);
                DismissBot(false);
                return;
            }

            // Healers: act regardless of having a melee victim
            if (IsHealerRole(_role))
            {
                DoClassAbilities(diff, owner);
                return;
            }

            // Non-healer: engage the target
            if (!me->GetVictim())
            {
                if (Unit* target = owner->GetVictim())
                    AttackStart(target);
                else
                {
                    me->CombatStop(true);
                    return;
                }
            }

            DoClassAbilities(diff, owner);
            return;
        }

        // ─── OUT OF COMBAT ────────────────────────────────────────
        if (!_isFollowing)
            return;

        // Decrement update throttle
        if (_updateTimer > diff)
        {
            _updateTimer -= diff;
            return;
        }
        _updateTimer = BOT_UPDATE_INTERVAL;

        Player* owner = ObjectAccessor::GetPlayer(*me, _ownerGuid);

        // Auto-dismiss if owner gone, dead, or left map
        if (!owner || !owner->IsAlive() || owner->GetMap() != me->GetMap())
        {
            DismissBot(true);
            return;
        }

        // Healer: top off owner between fights
        if (IsHealerRole(_role) && _healTimer == 0)
        {
            uint32 healSpell = GetPrimaryHealSpell(_role);
            if (healSpell && owner->GetHealthPct() < 90.0f)
            {
                DoCast(owner, healSpell);
                _healTimer = 4000;
                return;
            }
        }

        // Engage owner's target if they enter combat
        if (owner->IsInCombat())
        {
            if (Unit* target = owner->GetVictim())
            {
                if (target->IsAlive() && me->IsWithinDistInMap(target, 40.0f))
                {
                    me->GetMotionMaster()->Clear(false);
                    AttackStart(target);
                    return;
                }
            }
        }

        // Apply out-of-combat buffs every 2 minutes
        if (_buffTimer <= diff)
        {
            _buffTimer = 120000;
            ApplyOutOfCombatBuff(owner);
        }
        else
            _buffTimer -= diff;

        // Re-apply follow motion if needed
        if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
        {
            float angle = IsTankRole(_role) ? BOT_FOLLOW_ANGLE_SIDE : BOT_FOLLOW_ANGLE_BACK;
            me->GetMotionMaster()->MoveFollow(owner, BOT_FOLLOW_DIST, angle);
        }
    }

public:
    ObjectGuid  _ownerGuid;
    BotClassRole _role;
    bool        _isFollowing;

private:
    uint32 _updateTimer;
    uint32 _timer1, _timer2, _timer3;
    uint32 _healTimer;
    uint32 _buffTimer;
};

// ============================================================
//  Helper: dismiss all bots owned by a player
// ============================================================

static void DismissAllPlayerBots(Player* player)
{
    auto it = s_ownerToBots.find(player->GetGUID());
    if (it == s_ownerToBots.end()) return;

    std::vector<ObjectGuid> guids(it->second.begin(), it->second.end());
    for (ObjectGuid g : guids)
    {
        if (Creature* bot = ObjectAccessor::GetCreature(*player, g))
            if (npc_class_botAI* ai = CAST_AI(npc_class_botAI, bot->AI()))
                ai->DismissBot(true);
    }
}

// ============================================================
//  Creature Script (gossip + AI factory)
// ============================================================

class npc_class_bot : public CreatureScript
{
public:
    npc_class_bot() : CreatureScript("npc_class_bot") { }

    // ----------------------------------------------------------
    //  Gossip: Hello
    // ----------------------------------------------------------
    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->GetEntry() == BOT_CAPTAIN_ENTRY)
            return GossipHelloCaptain(player, creature);

        // Individual class bot ─────────────────────────────────
        npc_class_botAI* ai = CAST_AI(npc_class_botAI, creature->AI());
        if (!ai) return false;

        BotClassRole fixedRole = GetRoleForEntry(creature->GetEntry());
        const char*  roleLabel = GetClassLabel(fixedRole);

        bool ownsThisBot = (ai->_isFollowing && ai->_ownerGuid == player->GetGUID());
        bool botIsFree   = !ai->_isFollowing;

        if (ownsThisBot)
        {
            char buf[128];
            snprintf(buf, sizeof(buf), "Return to your post. (Dismiss %s)", roleLabel);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, buf, GSENDER, ACT_DISMISS);
        }
        else if (botIsFree)
        {
            uint32 count = GetPlayerBotCount(player->GetGUID());
            if (count < MAX_BOTS_PER_PLAYER)
            {
                char buf[128];
                snprintf(buf, sizeof(buf),
                    "Fight alongside me as %s.", roleLabel);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE, buf, GSENDER, ACT_HIRE);
            }
            else
            {
                AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                    "Your party is at full capacity. Dismiss a companion first.",
                    GSENDER, 0);
            }
        }
        else
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                "I am currently engaged with another.",
                GSENDER, 0);
        }

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Farewell.", GSENDER, 0);
        SendGossipMenuFor(player, NPC_TEXT_CLASS_BOT, creature->GetGUID());
        return true;
    }

    // ----------------------------------------------------------
    //  Captain gossip: main menu
    // ----------------------------------------------------------
    bool GossipHelloCaptain(Player* player, Creature* creature)
    {
        npc_class_botAI* ai = CAST_AI(npc_class_botAI, creature->AI());
        if (!ai) return false;

        bool ownsIt = (ai->_isFollowing && ai->_ownerGuid == player->GetGUID());
        bool isFree = !ai->_isFollowing;
        uint32 count = GetPlayerBotCount(player->GetGUID());

        if (ownsIt)
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                "Return to base. (Dismiss Captain)",
                GSENDER, ACT_DISMISS);
        }
        else if (isFree && count < MAX_BOTS_PER_PLAYER)
        {
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,
                "[Tank] I need a stalwart front-line defender.",
                GSENDER, CAP_NAV_TANKS);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,
                "[Healer] I need someone to mend my wounds.",
                GSENDER, CAP_NAV_HEALERS);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,
                "[Ranged DPS] I need magical or ranged firepower.",
                GSENDER, CAP_NAV_RANGED);
            AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1,
                "[Melee DPS] I need a fierce close-quarters fighter.",
                GSENDER, CAP_NAV_MELEE);
        }
        else if (!isFree)
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                "I am already engaged with another. Speak to my colleagues.",
                GSENDER, 0);
        }
        else
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                "Your party is at full capacity. Dismiss a companion first.",
                GSENDER, 0);
        }

        if (count > 0)
        {
            char buf[64];
            snprintf(buf, sizeof(buf), "Dismiss all companions (%u active).", count);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, buf, GSENDER, CAP_DISMISS_ALL);
        }

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Farewell.", GSENDER, 0);
        SendGossipMenuFor(player, NPC_TEXT_BOT_CAPTAIN, creature->GetGUID());
        return true;
    }

    // ----------------------------------------------------------
    //  Gossip: Select
    // ----------------------------------------------------------
    bool OnGossipSelect(Player* player, Creature* creature,
                        uint32 /*sender*/, uint32 action) override
    {
        if (creature->GetEntry() == BOT_CAPTAIN_ENTRY)
            return GossipSelectCaptain(player, creature, action);

        // Individual class bot
        npc_class_botAI* ai = CAST_AI(npc_class_botAI, creature->AI());
        if (!ai) { CloseGossipMenuFor(player); return false; }

        CloseGossipMenuFor(player);

        switch (action)
        {
            case ACT_HIRE:
                ai->HireBot(player, GetRoleForEntry(creature->GetEntry()));
                break;
            case ACT_DISMISS:
                ai->DismissBot(true);
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "%s has returned to their post.", creature->GetName().c_str());
                break;
            default:
                break;
        }
        return true;
    }

    // ----------------------------------------------------------
    //  Captain gossip: select / submenu
    // ----------------------------------------------------------
    bool GossipSelectCaptain(Player* player, Creature* creature, uint32 action)
    {
        npc_class_botAI* ai = CAST_AI(npc_class_botAI, creature->AI());
        if (!ai) { CloseGossipMenuFor(player); return false; }

        ClearGossipMenuFor(player);

        switch (action)
        {
            // ── Navigation ────────────────────────────────────
            case CAP_NAV_TANKS:
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Warrior  — Protection  (Thunder Clap, Shield Slam, Taunt)",    GSENDER, HIRE_WARRIOR_TANK);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Paladin  — Protection  (Consecration, Judgment, Taunt)",       GSENDER, HIRE_PALADIN_TANK);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Death Kn — Blood       (Death Grip, Death Strike self-heal)",  GSENDER, HIRE_DK_TANK);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Druid    — Guardian    (Thrash, Growl, Barkskin)",             GSENDER, HIRE_DRUID_TANK);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Demon Hu — Vengeance   (Shear, Torment, Demon Spikes)",        GSENDER, HIRE_DH_TANK);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<- Back", GSENDER, CAP_NAV_BACK);
                SendGossipMenuFor(player, NPC_TEXT_BOT_CAPTAIN, creature->GetGUID());
                break;

            case CAP_NAV_HEALERS:
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Priest   — Holy        (Flash Heal, Renew, Guardian Spirit)",  GSENDER, HIRE_PRIEST_HEAL);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Druid    — Restoration (Regrowth, Rejuvenation, Wild Growth)", GSENDER, HIRE_DRUID_HEAL);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Shaman   — Restoration (Healing Wave, Chain Heal)",            GSENDER, HIRE_SHAMAN_HEAL);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Paladin  — Holy        (Flash of Light, Lay on Hands)",        GSENDER, HIRE_PALADIN_HEAL);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Monk     — Mistweaver  (Enveloping Mist, Renewing Mist)",      GSENDER, HIRE_MONK_HEAL);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<- Back", GSENDER, CAP_NAV_BACK);
                SendGossipMenuFor(player, NPC_TEXT_BOT_CAPTAIN, creature->GetGUID());
                break;

            case CAP_NAV_RANGED:
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Mage     — Fire        (Fireball, Scorch, Frost Nova)",        GSENDER, HIRE_MAGE);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Warlock  — Affliction  (Shadow Bolt, Corruption, UA)",         GSENDER, HIRE_WARLOCK);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Hunter   — Beast Mast  (Arcane Shot, Multi-Shot, Kill Shot)",  GSENDER, HIRE_HUNTER);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Druid    — Balance     (Starsurge, Moonfire, Sunfire)",        GSENDER, HIRE_BOOMKIN);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Priest   — Shadow      (Mind Blast, SW:Pain, Vampiric Touch)", GSENDER, HIRE_SPRIEST);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<- Back", GSENDER, CAP_NAV_BACK);
                SendGossipMenuFor(player, NPC_TEXT_BOT_CAPTAIN, creature->GetGUID());
                break;

            case CAP_NAV_MELEE:
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Rogue    — Assassin    (Sinister Strike, Kick, Kidney Shot)",  GSENDER, HIRE_ROGUE);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Shaman   — Enhancement (Stormstrike, Lava Lash, Flame Shock)", GSENDER, HIRE_ENH_SHAMAN);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Paladin  — Retribution (Crusader Strike, Judgment, D.Storm)",  GSENDER, HIRE_RET_PALADIN);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Death Kn — Unholy      (Scourge Strike, Death Coil, D.Coil)",  GSENDER, HIRE_DK_DPS);
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Demon Hu — Havoc       (Chaos Strike, Throw Glaive, Blur)",    GSENDER, HIRE_DH_DPS);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "<- Back", GSENDER, CAP_NAV_BACK);
                SendGossipMenuFor(player, NPC_TEXT_BOT_CAPTAIN, creature->GetGUID());
                break;

            case CAP_NAV_BACK:
                GossipHelloCaptain(player, creature);
                break;

            case CAP_DISMISS_ALL:
                DismissAllPlayerBots(player);
                CloseGossipMenuFor(player);
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "All companions have returned to their posts.");
                break;

            case ACT_DISMISS:
                ai->DismissBot(true);
                CloseGossipMenuFor(player);
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "Commander Talindra has returned to base.");
                break;

            // ── Hire actions ──────────────────────────────────
            case HIRE_WARRIOR_TANK:  DoHire(player, creature, ai, BOT_WARRIOR_TANK); break;
            case HIRE_PALADIN_TANK:  DoHire(player, creature, ai, BOT_PALADIN_TANK); break;
            case HIRE_DK_TANK:       DoHire(player, creature, ai, BOT_DK_TANK);      break;
            case HIRE_DRUID_TANK:    DoHire(player, creature, ai, BOT_DRUID_TANK);   break;
            case HIRE_DH_TANK:       DoHire(player, creature, ai, BOT_DH_TANK);      break;
            case HIRE_PRIEST_HEAL:   DoHire(player, creature, ai, BOT_PRIEST_HEAL);  break;
            case HIRE_DRUID_HEAL:    DoHire(player, creature, ai, BOT_DRUID_HEAL);   break;
            case HIRE_SHAMAN_HEAL:   DoHire(player, creature, ai, BOT_SHAMAN_HEAL);  break;
            case HIRE_PALADIN_HEAL:  DoHire(player, creature, ai, BOT_PALADIN_HEAL); break;
            case HIRE_MONK_HEAL:     DoHire(player, creature, ai, BOT_MONK_HEAL);    break;
            case HIRE_MAGE:          DoHire(player, creature, ai, BOT_MAGE);         break;
            case HIRE_WARLOCK:       DoHire(player, creature, ai, BOT_WARLOCK);      break;
            case HIRE_HUNTER:        DoHire(player, creature, ai, BOT_HUNTER);       break;
            case HIRE_BOOMKIN:       DoHire(player, creature, ai, BOT_BOOMKIN);      break;
            case HIRE_SPRIEST:       DoHire(player, creature, ai, BOT_SPRIEST);      break;
            case HIRE_ROGUE:         DoHire(player, creature, ai, BOT_ROGUE);        break;
            case HIRE_ENH_SHAMAN:    DoHire(player, creature, ai, BOT_ENH_SHAMAN);   break;
            case HIRE_RET_PALADIN:   DoHire(player, creature, ai, BOT_RET_PALADIN);  break;
            case HIRE_DK_DPS:        DoHire(player, creature, ai, BOT_DK_DPS);       break;
            case HIRE_DH_DPS:        DoHire(player, creature, ai, BOT_DH_DPS);       break;

            default:
                CloseGossipMenuFor(player);
                break;
        }
        return true;
    }

    void DoHire(Player* player, Creature* /*creature*/, npc_class_botAI* ai, BotClassRole role)
    {
        CloseGossipMenuFor(player);
        ai->HireBot(player, role);
    }

    // ----------------------------------------------------------
    //  AI factory
    // ----------------------------------------------------------
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_class_botAI(creature);
    }
};

// ============================================================
//  Registration
// ============================================================

void AddSC_npc_class_bot()
{
    new npc_class_bot();
}

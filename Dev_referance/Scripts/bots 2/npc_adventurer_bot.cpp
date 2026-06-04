/*
 * BotSystem — Phase 2: Adventurer Party Bot
 * npc_adventurer_bot.cpp
 *
 * Drop this file into:
 *   src/server/scripts/Custom/npc_adventurer_bot.cpp
 *
 * Then add to custom_script_loader.cpp:
 *   void AddSC_npc_adventurer_bot();
 *   (and call it inside AddCustomScripts())
 *
 * ENTRY IDs expected (from Phase2 SQL):
 *   9500010 = Tank
 *   9500011 = Healer
 *   9500012 = DPS
 *
 * HOW IT WORKS:
 *   1. Right-click NPC → gossip menu appears
 *   2. Choose Tank / Healer / DPS  →  bot follows you and assists in combat
 *   3. Choose "I no longer need your help"  →  bot returns to spawn
 *   4. If you die, leave the map, or log out the bot auto-dismisses
 */

#include "Chat.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "Group.h"
#include "Log.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "SpellMgr.h"
#include "Unit.h"

/*=============================================================
 *  Constants
 *============================================================*/

// Gossip sender / action codes
#define BOT_GOSSIP_SENDER_MAIN  100
#define BOT_ACTION_HIRE_TANK    1
#define BOT_ACTION_HIRE_HEALER  2
#define BOT_ACTION_HIRE_DPS     3
#define BOT_ACTION_DISMISS      4

// Follow distance & angle (same as pet defaults)
#define BOT_FOLLOW_DIST  3.0f
#define BOT_FOLLOW_ANGLE float(M_PI)   // behind the player

// Combat check interval (ms)
#define BOT_UPDATE_INTERVAL  1000

/*---------- Spell IDs used by bots in combat ----------
 * These are generic cross-class spells that exist in the
 * Legion DB. Adjust if you see "no such spell" warnings.
 * Tank  → Taunt (355)
 * Healer→ Flash Heal (2061), scale with bot level later in Phase 3
 * DPS   → melee only (Phase 2 keeps it simple)
 */
#define SPELL_TAUNT         355
#define SPELL_FLASH_HEAL   2061

enum BotRole : uint8
{
    BOT_ROLE_NONE   = 0,
    BOT_ROLE_TANK   = 1,
    BOT_ROLE_HEALER = 2,
    BOT_ROLE_DPS    = 3
};

/*=============================================================
 *  AI struct
 *============================================================*/
struct npc_adventurer_botAI : public ScriptedAI
{
    npc_adventurer_botAI(Creature* creature) : ScriptedAI(creature)
    {
        _ownerGuid.Clear();
        _role           = BOT_ROLE_NONE;
        _isFollowing    = false;
        _updateTimer    = 0;
        _healCooldown   = 0;
        _tauntCooldown  = 0;
        // Store original spawn position so we can return after dismissal
        _homeX = creature->GetPositionX();
        _homeY = creature->GetPositionY();
        _homeZ = creature->GetPositionZ();
        _homeO = creature->GetOrientation();
        _homeMap = creature->GetMapId();
    }

    // -------------------------------------------------------
    //  Hire / dismiss helpers
    // -------------------------------------------------------
    void HireBot(Player* player, BotRole role)
    {
        if (_isFollowing)
        {
            // Already hired by someone else — send whisper? Just refuse for now.
            ChatHandler(player->GetSession()).PSendSysMessage("This adventurer is already busy.");
            return;
        }

        _ownerGuid   = player->GetGUID();
        _role        = role;
        _isFollowing = true;

        // Scale to the player's level so the bot isn't trivial or unkillable
        me->SetLevel(player->getLevel());

        // Update display name with role
        switch (role)
        {
            case BOT_ROLE_TANK:   me->SetName("Adventurer (Tank)");   break;
            case BOT_ROLE_HEALER: me->SetName("Adventurer (Healer)"); break;
            case BOT_ROLE_DPS:    me->SetName("Adventurer (DPS)");    break;
            default: break;
        }

        // Start following
        me->GetMotionMaster()->Clear(false);
        me->GetMotionMaster()->MoveFollow(player, BOT_FOLLOW_DIST, BOT_FOLLOW_ANGLE);

        // Notify player
        std::string roleStr = (role == BOT_ROLE_TANK) ? "tank" : (role == BOT_ROLE_HEALER) ? "healer" : "DPS";
        ChatHandler(player->GetSession()).PSendSysMessage("Your adventurer is ready to fight as your %s!", roleStr.c_str());
    }

    void DismissBot(bool returnToSpawn = true)
    {
        _ownerGuid.Clear();
        _role        = BOT_ROLE_NONE;
        _isFollowing = false;

        // Stop whatever the bot was doing
        me->GetMotionMaster()->Clear(false);
        me->CombatStop(true);

        // Restore original level and name
        me->SetLevel(me->GetCreatureTemplate()->minlevel > 0 ? me->GetCreatureTemplate()->minlevel : 1);
        me->SetName(me->GetCreatureTemplate()->Name);

        // Walk back to spawn point
        if (returnToSpawn)
            me->GetMotionMaster()->MoveTargetedHome();

        // Restore HP
        me->SetFullHealth();
    }

    // -------------------------------------------------------
    //  ScriptedAI overrides
    // -------------------------------------------------------
    void Reset() override
    {
        // Called on respawn — clear any lingering state
        if (_isFollowing)
            DismissBot(false);

        _updateTimer   = BOT_UPDATE_INTERVAL;
        _healCooldown  = 0;
        _tauntCooldown = 0;
    }

    void JustDied(Unit* /*killer*/) override
    {
        // Notify owner if possible
        if (Player* owner = ObjectAccessor::GetPlayer(*me, _ownerGuid))
            ChatHandler(owner->GetSession()).PSendSysMessage("Your adventurer has fallen!");

        _isFollowing = false;
        _ownerGuid.Clear();
        _role = BOT_ROLE_NONE;
    }

    void UpdateAI(uint32 diff) override
    {
        // ---- Combat: let ScriptedAI handle basic melee ----
        if (me->IsInCombat())
        {
            // Healer: prioritize healing over attacking
            if (_role == BOT_ROLE_HEALER && _isFollowing)
            {
                if (_healCooldown <= diff)
                {
                    if (Player* owner = ObjectAccessor::GetPlayer(*me, _ownerGuid))
                    {
                        if (owner->GetHealthPct() < 60.0f && owner->IsAlive())
                        {
                            me->GetMotionMaster()->Clear(false);
                            DoCast(owner, SPELL_FLASH_HEAL);
                            _healCooldown = 3500; // ~3.5 s cooldown
                            return;             // skip melee this tick
                        }
                    }
                }
                else
                    _healCooldown -= diff;
            }

            // Tank: taunt enemies attacking owner
            if (_role == BOT_ROLE_TANK && _isFollowing)
            {
                if (_tauntCooldown <= diff)
                {
                    if (Player* owner = ObjectAccessor::GetPlayer(*me, _ownerGuid))
                    {
                        // Find a unit attacking the owner and taunt it
                        for (auto const& ref : owner->getAttackers())
                        {
                            if (Unit* attacker = ref)
                            {
                                DoCast(attacker, SPELL_TAUNT);
                                _tauntCooldown = 8000; // 8 s between taunts
                                break;
                            }
                        }
                    }
                }
                else
                    _tauntCooldown -= diff;
            }

            DoMeleeAttackIfReady();
            return;
        }

        // ---- Not in combat ----
        if (!_isFollowing)
            return;

        if (_updateTimer <= diff)
        {
            _updateTimer = BOT_UPDATE_INTERVAL;

            Player* owner = ObjectAccessor::GetPlayer(*me, _ownerGuid);

            // Auto-dismiss if owner gone, dead, or left the map
            if (!owner || !owner->IsAlive() || owner->GetMap() != me->GetMap())
            {
                DismissBot(true);
                return;
            }

            // If owner is in combat and we're not → attack owner's target
            if (owner->IsInCombat())
            {
                if (Unit* target = owner->GetVictim())
                {
                    if (target->IsAlive() && me->IsWithinDistInMap(target, 30.0f))
                    {
                        me->GetMotionMaster()->Clear(false);
                        me->AI()->AttackStart(target);
                        return;
                    }
                }
            }

            // Re-apply follow if we drifted (bot returned from combat)
            if (!me->IsInCombat() && me->IsWithinDistInMap(owner, 40.0f))
            {
                if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
                    me->GetMotionMaster()->MoveFollow(owner, BOT_FOLLOW_DIST, BOT_FOLLOW_ANGLE);
            }
        }
        else
            _updateTimer -= diff;
    }

public:
    ObjectGuid _ownerGuid;
    BotRole    _role;
    bool       _isFollowing;

private:
    uint32     _updateTimer;
    uint32     _healCooldown;
    uint32     _tauntCooldown;

    // home position for return-to-spawn
    float  _homeX, _homeY, _homeZ, _homeO;
    uint32 _homeMap;
};

/*=============================================================
 *  Creature Script (gossip + AI factory)
 *============================================================*/
class npc_adventurer_bot : public CreatureScript
{
public:
    npc_adventurer_bot() : CreatureScript("npc_adventurer_bot") { }

    // -------------------------------------------------------
    //  Gossip: Hello (opens the menu)
    // -------------------------------------------------------
    bool OnGossipHello(Player* player, Creature* creature) override
    {
        npc_adventurer_botAI* botAI = CAST_AI(npc_adventurer_botAI, creature->AI());
        if (!botAI)
            return false;

        // If this bot is already following THIS player, show dismiss option
        if (botAI->_isFollowing && botAI->_ownerGuid == player->GetGUID())
        {
            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                "I no longer need your help. Return to your post.",
                BOT_GOSSIP_SENDER_MAIN, BOT_ACTION_DISMISS);
        }
        else if (!botAI->_isFollowing)
        {
            // Determine which role this NPC template is best suited for
            // (could also let player choose regardless of template)
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                "Come with me as a Tank — defend me in the front line.",
                BOT_GOSSIP_SENDER_MAIN, BOT_ACTION_HIRE_TANK);

            AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                "Come with me as a Healer — keep me alive.",
                BOT_GOSSIP_SENDER_MAIN, BOT_ACTION_HIRE_HEALER);

            AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                "Come with me as DPS — help me deal damage.",
                BOT_GOSSIP_SENDER_MAIN, BOT_ACTION_HIRE_DPS);
        }
        else
        {
            // Bot is busy with someone else
            AddGossipItemFor(player, GOSSIP_ICON_CHAT,
                "I am currently occupied. Come back later.",
                BOT_GOSSIP_SENDER_MAIN, 0);
        }

        SendGossipMenuFor(player, 91001, creature->GetGUID());
        return true;
    }

    // -------------------------------------------------------
    //  Gossip: Select (player picked an option)
    // -------------------------------------------------------
    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        CloseGossipMenuFor(player);

        npc_adventurer_botAI* botAI = CAST_AI(npc_adventurer_botAI, creature->AI());
        if (!botAI)
            return false;

        switch (action)
        {
            case BOT_ACTION_HIRE_TANK:
                botAI->HireBot(player, BOT_ROLE_TANK);
                break;
            case BOT_ACTION_HIRE_HEALER:
                botAI->HireBot(player, BOT_ROLE_HEALER);
                break;
            case BOT_ACTION_HIRE_DPS:
                botAI->HireBot(player, BOT_ROLE_DPS);
                break;
            case BOT_ACTION_DISMISS:
                botAI->DismissBot(true);
                ChatHandler(player->GetSession()).PSendSysMessage("Your adventurer has returned to their post.");
                break;
            default:
                break;
        }

        return true;
    }

    // -------------------------------------------------------
    //  AI factory
    // -------------------------------------------------------
    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_adventurer_botAI(creature);
    }
};

/*=============================================================
 *  Registration (called from custom_script_loader.cpp)
 *============================================================*/
void AddSC_npc_adventurer_bot()
{
    new npc_adventurer_bot();
}

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellPackets.h"
#include "ObjectAccessor.h"
#include "Chat.h"
#include "DB2Stores.h"

// ============================================================================
// Skyreach Beam — Veil Akraz ambient event (repeats every 5 minutes)
// PlayOrphanSpellVisual — position-based, works at any distance
// ============================================================================

enum SkyreachBeamData
{
    NPC_SKYREACH_BEAM_CONTROLLER = 900100,
    NPC_BEAM_SOURCE              = 900101,
    NPC_BEAM_TARGET              = 900102,
    NPC_VEIL_AKRAZ_OUTCAST       = 80320,
    GO_GROUND_FIRE               = 248101,

    SPELL_CAMERA_SHAKE           = 150209,
    SPELL_ENERGIZE_MASS          = 154177,
    SPELL_VISUAL_ENERGIZE        = 37002,

    // SpellVisualID for beam 2 sweep (change once a golden one is found)
    BEAM_VISUAL_ID               = 55186,

    // Beam source events
    EVENT_CHANNEL_REFRESH        = 10,

    // Controller events
    EVENT_SWEEP_TICK             = 2,
    EVENT_NPC_COWER              = 3,
    EVENT_FIRE_DESPAWN_TICK      = 4,
    EVENT_RESTART_CYCLE          = 5,
};

static constexpr uint32 BEAM_CYCLE_INTERVAL    = 5 * 60 * 1000; // 5 min between cycles
static constexpr uint32 SWEEP_INTERVAL         = 120;            // 120ms between fire spawns
static constexpr float  CAMERA_SHAKE_RANGE     = 30.0f;          // shake radius around fire

// Point 1: Skyreach summit — beam originates here (cosmetic only)
static const Position SkyreachTopPos  = { -49.5f, 2306.3f, 566.1f, 5.21f };
// Point 2: Relay — beam arrives here (cosmetic only, no fire)
static const Position BeamRelayPos    = { 130.1f, 2183.0f, 325.5f, 0.0f };
// Point 3: Emitter — fires a sweeping ground beam that creates the breach fire
static const Position BeamEmitterPos  = { 138.0f, 2176.1f, 323.3f, 0.0f };
// Center of breach for distance checks / teleport reference
static const Position BreachCenterPos = { 376.0f, 2060.0f, -7.0f, 0.0f };

// Fire positions along the Veil Akraz breach (curved path through the village)
// Interpolated from 15 GPS reference points, ~4 yard spacing for dense fire line
static const Position FirePositions[] =
{
    // Segment 1→2 (NE end of breach, heading south)
    { 435.1f, 2131.0f, -0.6f, 0.0f },
    { 434.8f, 2127.1f, -1.6f, 0.0f },
    { 434.5f, 2123.2f, -2.6f, 0.0f },
    { 434.2f, 2119.3f, -3.6f, 0.0f },
    // Segment 2→3
    { 433.9f, 2115.4f, -4.6f, 0.0f },
    { 432.3f, 2111.7f, -4.7f, 0.0f },
    { 430.8f, 2108.0f, -4.7f, 0.0f },
    // Segment 3→4
    { 429.2f, 2104.3f, -4.8f, 0.0f },
    { 426.5f, 2101.1f, -5.2f, 0.0f },
    { 423.7f, 2097.8f, -5.5f, 0.0f },
    // Segment 4→5 (curving west)
    { 421.0f, 2094.6f, -5.9f, 0.0f },
    { 417.9f, 2092.6f, -6.1f, 0.0f },
    { 414.8f, 2090.5f, -6.2f, 0.0f },
    { 411.7f, 2088.4f, -6.4f, 0.0f },
    // Segment 5→6
    { 408.6f, 2086.4f, -6.5f, 0.0f },
    { 404.8f, 2084.4f, -6.7f, 0.0f },
    { 401.0f, 2082.5f, -6.9f, 0.0f },
    { 397.3f, 2080.5f, -7.1f, 0.0f },
    // Segment 6→7
    { 393.5f, 2078.5f, -7.3f, 0.0f },
    { 390.0f, 2075.9f, -7.4f, 0.0f },
    { 386.6f, 2073.2f, -7.4f, 0.0f },
    { 383.1f, 2070.6f, -7.5f, 0.0f },
    // Segment 7→8 (long stretch SW)
    { 379.6f, 2067.9f, -7.6f, 0.0f },
    { 376.7f, 2065.3f, -7.6f, 0.0f },
    { 373.7f, 2062.8f, -7.7f, 0.0f },
    { 370.7f, 2060.2f, -7.7f, 0.0f },
    { 367.8f, 2057.6f, -7.8f, 0.0f },
    // Segment 8→9
    { 364.9f, 2055.1f, -7.8f, 0.0f },
    { 362.3f, 2052.0f, -7.9f, 0.0f },
    { 359.6f, 2048.8f, -7.9f, 0.0f },
    { 357.0f, 2045.7f, -8.0f, 0.0f },
    // Segment 9→10 (curving south)
    { 354.4f, 2042.6f, -8.1f, 0.0f },
    { 353.1f, 2039.1f, -7.9f, 0.0f },
    { 351.8f, 2035.6f, -7.6f, 0.0f },
    { 350.5f, 2032.1f, -7.4f, 0.0f },
    // Segment 10→11
    { 349.2f, 2028.6f, -7.1f, 0.0f },
    { 348.7f, 2024.9f, -7.1f, 0.0f },
    { 348.2f, 2021.2f, -7.1f, 0.0f },
    { 347.7f, 2017.5f, -7.1f, 0.0f },
    // Segment 11→12
    { 347.2f, 2013.8f, -7.1f, 0.0f },
    { 345.6f, 2010.7f, -7.1f, 0.0f },
    { 343.9f, 2007.6f, -7.1f, 0.0f },
    // Segment 12→13
    { 342.3f, 2004.5f, -7.1f, 0.0f },
    { 339.3f, 2002.2f, -6.7f, 0.0f },
    { 336.4f, 1999.9f, -6.3f, 0.0f },
    // Segment 13→14
    { 333.4f, 1997.6f, -5.9f, 0.0f },
    { 329.6f, 1994.7f, -5.7f, 0.0f },
    // Segment 14→15 (SW end of breach)
    { 325.8f, 1991.8f, -5.5f, 0.0f },
    { 321.7f, 1990.2f, -5.0f, 0.0f },
    { 317.5f, 1988.5f, -4.4f, 0.0f },
};

static constexpr uint32 FIRE_COUNT = 50;

// ---------------------------------------------------------------------------
// npc_skyreach_beam_source — permanent NPC at Skyreach summit
// Channels spell 154177 to beam target (900102), same as dungeon Skyreach NPCs
// ---------------------------------------------------------------------------

struct npc_skyreach_beam_source : public ScriptedAI
{
    npc_skyreach_beam_source(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
        me->SetCanFly(true);
        channelVisualId = SPELL_VISUAL_ENERGIZE;
    }

    EventMap events;
    int32 channelVisualId;

    void Reset() override
    {
        events.Reset();
        events.RescheduleEvent(EVENT_CHANNEL_REFRESH, 5000);
    }

    void StartChannel()
    {
        Creature* target = me->FindNearestCreature(NPC_BEAM_TARGET, 400.0f);
        if (!target)
            return;

        me->ClearDynamicValue(UNIT_DYNAMIC_FIELD_CHANNEL_OBJECTS);
        me->AddChannelObject(target->GetGUID());
        me->SetChannelSpellId(SPELL_ENERGIZE_MASS);
        me->SetChannelSpellXSpellVisualId(channelVisualId);
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (uint32 eventId = events.ExecuteEvent())
        {
            if (eventId == EVENT_CHANNEL_REFRESH)
            {
                StartChannel();
                events.RescheduleEvent(EVENT_CHANNEL_REFRESH, 60000);
            }
        }
    }
};

// ---------------------------------------------------------------------------
// npc_skyreach_beam_target — permanent NPC at relay point, just floats
// ---------------------------------------------------------------------------

struct npc_skyreach_beam_target : public ScriptedAI
{
    npc_skyreach_beam_target(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
        me->SetCanFly(true);
    }

    void UpdateAI(uint32 /*diff*/) override { }
};

// ---------------------------------------------------------------------------
// npc_skyreach_beam_controller — permanent invisible NPC, loops the fire sweep
// Beam 1 (Skyreach→relay) handled by npc_skyreach_beam_source above
// Beam 2 (emitter→fires) uses PlayOrphanSpellVisual
// ---------------------------------------------------------------------------

class npc_skyreach_beam_controller : public CreatureScript
{
public:
    npc_skyreach_beam_controller() : CreatureScript("npc_skyreach_beam_controller") { }

    struct npc_skyreach_beam_controllerAI : public ScriptedAI
    {
        npc_skyreach_beam_controllerAI(Creature* creature) : ScriptedAI(creature)
        {
            fireGuidCount = 0;
            sweepIndex = 0;
            despawnIndex = 0;
            beamVisualId = BEAM_VISUAL_ID;
        }

        EventMap events;
        ObjectGuid fireGOs[FIRE_COUNT];
        uint32 fireGuidCount;
        uint32 sweepIndex;
        uint32 despawnIndex;
        int32 beamVisualId;

        void Reset() override
        {
            DespawnFires();
            events.Reset();
            fireGuidCount = 0;
            sweepIndex = 0;
            despawnIndex = 0;
            events.RescheduleEvent(EVENT_RESTART_CYCLE, BEAM_CYCLE_INTERVAL);
        }

        void SendBeamVisual(Position const& src, Position const& dst, float speed)
        {
            if (!beamVisualId)
                return;

            WorldPackets::Spells::PlayOrphanSpellVisual pkt;
            pkt.SourceLocation = src;
            pkt.SourceRotation = { 0.0f, 0.0f, 0.0f };
            pkt.TargetLocation = dst;
            pkt.SpellVisualID = beamVisualId;
            pkt.TravelSpeed = speed;
            pkt.SpeedAsTime = true;
            WorldPacket const* data = pkt.Write();

            std::list<Player*> players;
            me->GetPlayerListInGrid(players, 500.0f);
            for (Player* player : players)
                player->SendDirectMessage(data);
        }

        void StartBeamSequence()
        {
            DespawnFires();
            fireGuidCount = 0;
            sweepIndex = 0;
            despawnIndex = 0;

            // T+0s: ground sweep (beam 2 + fires)
            events.RescheduleEvent(EVENT_SWEEP_TICK, 0);
            // T+3.0s: outcasts cower
            events.RescheduleEvent(EVENT_NPC_COWER, 3000);
            // T+25s: fires despawn (reverse)
            events.RescheduleEvent(EVENT_FIRE_DESPAWN_TICK, 25000);
            // T+5min: next cycle
            events.RescheduleEvent(EVENT_RESTART_CYCLE, BEAM_CYCLE_INTERVAL);
        }

        void SpawnFireAt(uint32 index)
        {
            if (index >= FIRE_COUNT)
                return;

            if (GameObject* go = me->SummonGameObject(GO_GROUND_FIRE,
                FirePositions[index], 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0))
            {
                fireGOs[fireGuidCount++] = go->GetGUID();
            }
        }

        void ShakeNearFire(Position const& firePos)
        {
            std::list<Player*> players;
            me->GetPlayerListInGrid(players, 150.0f);
            for (Player* player : players)
            {
                if (player->GetExactDist2d(firePos.GetPositionX(), firePos.GetPositionY()) <= CAMERA_SHAKE_RANGE)
                {
                    player->RemoveSpellCooldown(SPELL_CAMERA_SHAKE, true);
                    player->CastSpell(player, SPELL_CAMERA_SHAKE, true);
                }
            }
        }

        void DespawnFires()
        {
            for (uint32 i = 0; i < fireGuidCount; ++i)
            {
                if (GameObject* go = ObjectAccessor::GetGameObject(*me, fireGOs[i]))
                    go->Delete();
            }
            fireGuidCount = 0;
        }

        void MakeOutcastsCower()
        {
            std::list<Creature*> outcasts;
            GetCreatureListWithEntryInGrid(outcasts, me, NPC_VEIL_AKRAZ_OUTCAST, 80.0f);
            for (Creature* npc : outcasts)
                npc->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, EMOTE_STATE_COWER);
        }

        void ResetOutcasts()
        {
            std::list<Creature*> outcasts;
            GetCreatureListWithEntryInGrid(outcasts, me, NPC_VEIL_AKRAZ_OUTCAST, 80.0f);
            for (Creature* npc : outcasts)
                npc->SetUInt32Value(UNIT_FIELD_EMOTE_STATE, 0);
        }

        void UpdateAI(uint32 diff) override
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SWEEP_TICK:
                    {
                        SendBeamVisual(BeamEmitterPos, FirePositions[sweepIndex], 0.3f);
                        SpawnFireAt(sweepIndex);

                        if (sweepIndex % 8 == 0)
                            ShakeNearFire(FirePositions[sweepIndex]);

                        ++sweepIndex;
                        if (sweepIndex < FIRE_COUNT)
                            events.RescheduleEvent(EVENT_SWEEP_TICK, SWEEP_INTERVAL);
                        break;
                    }

                    case EVENT_NPC_COWER:
                        MakeOutcastsCower();
                        break;

                    case EVENT_FIRE_DESPAWN_TICK:
                    {
                        uint32 reverseIdx = fireGuidCount - 1 - despawnIndex;
                        if (GameObject* go = ObjectAccessor::GetGameObject(*me, fireGOs[reverseIdx]))
                            go->Delete();
                        ++despawnIndex;
                        if (despawnIndex < fireGuidCount)
                            events.RescheduleEvent(EVENT_FIRE_DESPAWN_TICK, SWEEP_INTERVAL);
                        else
                            ResetOutcasts();
                        break;
                    }

                    case EVENT_RESTART_CYCLE:
                        StartBeamSequence();
                        return;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_skyreach_beam_controllerAI(creature);
    }
};

// ============================================================================
// Original Spires of Arak scripts
// ============================================================================

enum eSpells
{
    // ruhamar
    SPELL_SOLAR_RADIATION       = 167708,
    SPELL_SHARP_BEAK            = 167614, // 12
    SPELL_SOLAR_BREATH          = 167679, // 29
    SPELL_LOOSE_QUILLS          = 167647, // 115 (fly)
    SPELL_BLOOD_FEATHER         = 167625, // 15 + script adds
    SPELL_SLEEP_ADDS            = 160641,
    SPELL_FIX_TARGET            = 167757,

    EVENT_GROUP_GROUND          = 1
};

// 83746
class boss_ruhamar : public CreatureScript
{
public:
    boss_ruhamar() : CreatureScript("boss_ruhamar") { }

    struct boss_ruhamarAI : public ScriptedAI
    {
        boss_ruhamarAI(Creature* creature) : ScriptedAI(creature), summons(me), _pathStarted(false) {}

        EventMap events;
        SummonList summons;
        uint32 healthPct;
        Position oldpos;
        bool _pathStarted;

        void Reset()
        {
            me->setActive(true);
            me->SetCanFly(true);
            me->SetDisableGravity(true);
            me->SetSpeed(MOVE_RUN, 3.5f);
            me->SetSpeed(MOVE_FLIGHT, 3.5f);
            me->SetReactState(REACT_AGGRESSIVE);

            // Only create the waypoint path once (first spawn).
            // On evade, the existing generator in MOTION_SLOT_IDLE
            // resumes from _currentNode via DoReset — don't recreate it.
            if (!_pathStarted)
            {
                me->GetMotionMaster()->MovePath(439156, true);
                _pathStarted = true;
            }

            events.Reset();
            summons.DespawnAll();
            healthPct = 98;
        }

        void EnterCombat(Unit* unit)
        {
            me->SetDisableGravity(false);
            me->CastSpell(me, SPELL_SOLAR_RADIATION);
            events.RescheduleEvent(EVENT_1, 12000, EVENT_GROUP_GROUND);
            events.RescheduleEvent(EVENT_2, 29000, EVENT_GROUP_GROUND);
            events.RescheduleEvent(EVENT_3, 85000);
        }

        // No JustReachedHome override — DirectExpire handles waypoint
        // resumption via DoReset on the existing WaypointMovementGenerator.

        void JustSummoned(Creature* summon)
        {
            summons.Summon(summon);
            DoZoneInCombat(summon, 150.0f);
        }

        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
        {
            if (me->HealthBelowPct(healthPct))
            {
                healthPct -= urand(2,4);
                events.RescheduleEvent(EVENT_4, 10, EVENT_GROUP_GROUND);
            }
        }

        void OnApplyOrRemoveAura(uint32 spellId, AuraRemoveMode mode, bool apply)
        {
            if (!apply && spellId == SPELL_LOOSE_QUILLS)
                me->GetMotionMaster()->MovePoint(2, oldpos.GetPositionX(), oldpos.GetPositionY(), oldpos.GetPositionZ());
        }

        void MovementInform(uint32 type, uint32 point)
        {
            if (type == WAYPOINT_MOTION_TYPE)
                me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());

            if (type != POINT_MOTION_TYPE)
                return;

            me->GetMotionMaster()->MovementExpired();
            if (point == 1)
                me->CastSpell(me, SPELL_LOOSE_QUILLS);
            else if (point == 2)
            {
                me->SetCanFly(false);
                me->SetDisableGravity(false);
                me->SetReactState(REACT_AGGRESSIVE);

                events.RescheduleEvent(EVENT_1, 12000, EVENT_GROUP_GROUND);
                events.RescheduleEvent(EVENT_2, 29000, EVENT_GROUP_GROUND);
                events.RescheduleEvent(EVENT_3, 85000);
            }
        }


        void UpdateAI(uint32 diff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STATE_CASTING))
                return;

            events.Update(diff);

            if (uint32 eventId = events.ExecuteEvent())
            {
				switch (eventId)
				{
				case EVENT_1:
					DoCastVictim(SPELL_SHARP_BEAK);
					events.RescheduleEvent(EVENT_1, 12000, EVENT_GROUP_GROUND);
					break;
				case EVENT_2:
					DoCastVictim(SPELL_SOLAR_BREATH);
					events.RescheduleEvent(EVENT_2, 29000, EVENT_GROUP_GROUND);
					break;
				case EVENT_3:
				{
					events.CancelEventGroup(EVENT_GROUP_GROUND);

					me->SetCanFly(true);
					me->SetDisableGravity(true);
					me->SetReactState(REACT_PASSIVE);
					me->AttackStop();
					Position pos;
					pos.Relocate(me);
					oldpos = pos;
					pos.m_positionZ += 10.0f;
					me->GetMotionMaster()->MovePoint(1, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
					break;
				}
                    case EVENT_4:
                        DoCastVictim(SPELL_BLOOD_FEATHER);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_ruhamarAI(creature);
    }
};

// 83769
class npc_energized_phoenix : public CreatureScript
{
public:
    npc_energized_phoenix() : CreatureScript("npc_energized_phoenix") { }

    struct npc_energized_phoenixAI : public ScriptedAI
    {
        npc_energized_phoenixAI(Creature* creature) : ScriptedAI(creature) {}

        EventMap events;
        bool moveComplete;
        uint32 moveTimer;
		ObjectGuid targetMove;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit* unit)
        {
        }


        void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType dmgType)
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;

                me->CastStop();
                me->SetHealth(me->GetMaxHealth());
                me->CastSpell(me, 167629, true);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                me->CastSpell(me, SPELL_SLEEP_ADDS, true);
                me->GetMotionMaster()->MovementExpired(true);
                me->SetDisplayId(66325);
                me->AttackStop();
                me->SetReactState(REACT_PASSIVE);
            }
        }

        void IsSummonedBy(Unit* owner)
        {
            if (owner->GetGUID() != me->GetGUID())
            {
                std::list<Creature*> adds;
                GetCreatureListWithEntryInGrid(adds, me, me->GetEntry(), 5.0f);
                if (!adds.empty())
                    for (std::list<Creature*>::iterator itr = adds.begin(); itr != adds.end(); ++itr)
                    {
                        if ((*itr)->HasAura(SPELL_SLEEP_ADDS))
                        {
                            (*itr)->RemoveAurasDueToSpell(SPELL_SLEEP_ADDS);
                            (*itr)->AI()->IsSummonedBy((*itr));
                        }
                    }
            }

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
            me->SetDisplayId(me->GetNativeDisplayId());

            moveComplete = false;
            moveTimer = 2000;

            std::list<Player*> targets;
            targets.clear();
            GetPlayerListInGrid(targets, me, 70.0f);
            if (!targets.empty())
            {
                std::list<Player*>::const_iterator itr = targets.begin();
                std::advance(itr, urand(0, targets.size() - 1));
                if (*itr)
                    targetMove = (*itr)->GetGUID();

                me->GetMotionMaster()->MoveFollow((*itr), PET_FOLLOW_DIST, (*itr)->GetFollowAngle());
                me->CastSpell((*itr), SPELL_FIX_TARGET);
            }
            else
                me->DespawnOrUnsummon();
        }


        void UpdateAI(uint32 diff)
        {
            if (!moveComplete)
            {
                if (moveTimer <= diff)
                {
                    Player* target = ObjectAccessor::GetPlayer(*me, targetMove);
                    if (!target)
                    {
                        IsSummonedBy(me); // tru refind
                        return;
                    }

                    if (me->GetDistance(target) < 3.0f)
                    {
                        moveComplete = true;

                        me->CastStop();
                        me->SetHealth(me->GetMaxHealth());
                        me->CastSpell(me, 167629, true);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_REMOVE_CLIENT_CONTROL);
                        me->CastSpell(me, SPELL_SLEEP_ADDS, true);
                        me->GetMotionMaster()->MovementExpired(true);
                        me->SetDisplayId(66325);
                        me->AttackStop();
                        me->SetReactState(REACT_PASSIVE);
                    }
                    moveTimer = 1000;
                }
                else
                    moveTimer -= diff;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_energized_phoenixAI(creature);
    }
};

// ---------------------------------------------------------------------------
// GM command: .skybeam — triggers the Veil Akraz beam event on the map
// ---------------------------------------------------------------------------

class skybeam_commandscript : public CommandScript
{
public:
    skybeam_commandscript() : CommandScript("skybeam_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> commandTable =
        {
            { "skybeam", SEC_GAMEMASTER, false, &HandleSkybeamCommand, "" }
        };
        return commandTable;
    }

    static bool HandleSkybeamCommand(ChatHandler* handler, char const* args)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        std::string argStr = args ? args : "";

        // .skybeam test <visualId> — test a visual from emitter to first fire
        if (argStr.substr(0, 4) == "test")
        {
            int32 testId = 0;
            if (argStr.length() > 5)
                testId = atoi(argStr.c_str() + 5);

            if (!testId)
            {
                handler->SendSysMessage("Usage: .skybeam test <visualId>");
                return true;
            }

            player->PlayOrphanSpellVisual(BeamEmitterPos, {0, 0, 0}, FirePositions[0],
                testId, 0.5f, ObjectGuid::Empty, true);
            player->PlayOrphanSpellVisual(SkyreachTopPos, {0, 0, 0}, BeamRelayPos,
                testId, 0.8f, ObjectGuid::Empty, true);

            handler->PSendSysMessage("Fired visual %d: Emitter→Fire[0] + Skyreach→Relay", testId);
            return true;
        }

        // .skybeam lookup <spellId> — find SpellVisualID for a spell
        if (argStr.substr(0, 6) == "lookup")
        {
            int32 spellId = 0;
            if (argStr.length() > 7)
                spellId = atoi(argStr.c_str() + 7);

            if (!spellId)
            {
                handler->SendSysMessage("Usage: .skybeam lookup <spellId>");
                return true;
            }

            bool found = false;
            for (auto const& entry : sSpellXSpellVisualStore)
            {
                if (entry->SpellID == spellId)
                {
                    handler->PSendSysMessage("Spell %d → SpellVisualID %d (CasterPlayerConditionID=%d, ViewerPlayerConditionID=%d)",
                        spellId, entry->SpellVisualID, entry->CasterPlayerConditionID, entry->ViewerPlayerConditionID);
                    found = true;
                }
            }
            if (!found)
                handler->PSendSysMessage("No SpellXSpellVisual entry found for spell %d.", spellId);
            return true;
        }

        // .skybeam channel <visualId> — change the channel beam visual (beam 1)
        if (argStr.substr(0, 7) == "channel")
        {
            int32 newId = 0;
            if (argStr.length() > 8)
                newId = atoi(argStr.c_str() + 8);

            if (!newId)
            {
                handler->SendSysMessage("Usage: .skybeam channel <visualId>");
                return true;
            }

            Creature* source = player->FindNearestCreature(NPC_BEAM_SOURCE, 500.0f);
            if (source)
            {
                if (auto* ai = dynamic_cast<npc_skyreach_beam_source*>(source->AI()))
                {
                    ai->channelVisualId = newId;
                    ai->StartChannel();
                    handler->PSendSysMessage("Channel visual changed to %d and refreshed.", newId);
                    return true;
                }
            }
            handler->SendSysMessage("Beam source NPC not found nearby.");
            return true;
        }

        // .skybeam set <visualId> — change the visual used by the event
        if (argStr.substr(0, 3) == "set")
        {
            int32 newId = 0;
            if (argStr.length() > 4)
                newId = atoi(argStr.c_str() + 4);

            if (!newId)
            {
                handler->SendSysMessage("Usage: .skybeam set <visualId>");
                return true;
            }

            Creature* controller = player->FindNearestCreature(NPC_SKYREACH_BEAM_CONTROLLER, 500.0f);
            if (controller)
            {
                if (auto* ai = dynamic_cast<npc_skyreach_beam_controller::npc_skyreach_beam_controllerAI*>(controller->AI()))
                {
                    ai->beamVisualId = newId;
                    handler->PSendSysMessage("Beam visual changed to %d. Use .skybeam to trigger.", newId);
                    return true;
                }
            }
            handler->SendSysMessage("Controller not found nearby.");
            return true;
        }

        // .skybeam — trigger the event
        Creature* controller = player->FindNearestCreature(NPC_SKYREACH_BEAM_CONTROLLER, 500.0f);
        if (!controller)
        {
            controller = player->SummonCreature(NPC_SKYREACH_BEAM_CONTROLLER,
                BreachCenterPos, TEMPSUMMON_TIMED_DESPAWN, 60000);
        }

        if (!controller)
        {
            handler->SendSysMessage("Failed to find or spawn beam controller.");
            return false;
        }

        if (auto* ai = dynamic_cast<npc_skyreach_beam_controller::npc_skyreach_beam_controllerAI*>(controller->AI()))
        {
            ai->StartBeamSequence();
            handler->PSendSysMessage("Skyreach beam event triggered! (visual=%d)", ai->beamVisualId);
        }
        else
        {
            handler->SendSysMessage("Controller AI not found.");
            return false;
        }

        return true;
    }
};

void AddSC_wod_spires_of_arak()
{
    new boss_ruhamar();
    new npc_energized_phoenix();
    RegisterCreatureAI(npc_skyreach_beam_source);
    RegisterCreatureAI(npc_skyreach_beam_target);
    new npc_skyreach_beam_controller();
    new skybeam_commandscript();
}

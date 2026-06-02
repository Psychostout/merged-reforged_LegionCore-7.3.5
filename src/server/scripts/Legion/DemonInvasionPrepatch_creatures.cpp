/*
 * Demon Invasion Prepatch - Creature AI Scripts
 * Generic demons, Commander/Lieutenants, and Boss
 */

#include "DemonInvasionPrepatch.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "Player.h"
#include "SpellInfo.h"

enum DemonInvasionSpells
{
    // Generic demons
    SPELL_DI_FEL_STRIKE       = 183345,  // Melee instant attack (common Legion spell)
    SPELL_DI_FEL_BOLT         = 183349,  // Shadow bolt variant
    SPELL_DI_IMMOLATION_AURA  = 187727,  // Infernal AoE fire aura

    // Commander/Lieutenants
    SPELL_DI_SHADOW_CLEAVE    = 183542,  // Frontal cleave
    SPELL_DI_FEL_STOMP        = 200261,  // AoE stomp
    SPELL_DI_ENRAGE           = 8599,    // Standard enrage

    // Boss
    SPELL_DI_METEOR_STRIKE    = 177973,  // Large AoE impact
    SPELL_DI_FEL_FIRE_BREATH  = 183465,  // Cone fire
    SPELL_DI_BERSERK          = 26662,   // Berserk timer
};

enum DemonInvasionEvents
{
    EVENT_DI_FEL_STRIKE = 1,
    EVENT_DI_FEL_BOLT,
    EVENT_DI_IMMOLATION,
    EVENT_DI_SHADOW_CLEAVE,
    EVENT_DI_FEL_STOMP,
    EVENT_DI_ENRAGE_CHECK,
    EVENT_DI_METEOR,
    EVENT_DI_BREATH,
    EVENT_DI_BERSERK,
};

// ============================================================================
// npc_demon_invasion_generic — Stage 1/3 trash mobs
// ============================================================================
class npc_demon_invasion_generic : public CreatureScript
{
public:
    npc_demon_invasion_generic() : CreatureScript("npc_demon_invasion_generic") {}

    struct npc_demon_invasion_genericAI : public ScriptedAI
    {
        npc_demon_invasion_genericAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset() override
        {
            events.Reset();
        }

        void EnterCombat(Unit* /*who*/) override
        {
            switch (me->GetEntry())
            {
                case NPC_DI_FEL_INVADER:
                    events.RescheduleEvent(EVENT_DI_FEL_STRIKE, urand(4000, 8000));
                    break;
                case NPC_DI_FELSTALKER:
                    events.RescheduleEvent(EVENT_DI_FEL_STRIKE, urand(3000, 5000));
                    break;
                case NPC_DI_FEL_CASTER:
                    events.RescheduleEvent(EVENT_DI_FEL_BOLT, urand(1000, 3000));
                    break;
                case NPC_DI_INFERNAL:
                    events.RescheduleEvent(EVENT_DI_IMMOLATION, 2000);
                    events.RescheduleEvent(EVENT_DI_FEL_STRIKE, urand(5000, 10000));
                    break;
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_DI_FEL_STRIKE:
                        DoCastVictim(SPELL_DI_FEL_STRIKE);
                        events.RescheduleEvent(EVENT_DI_FEL_STRIKE, urand(6000, 12000));
                        break;
                    case EVENT_DI_FEL_BOLT:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                            DoCast(target, SPELL_DI_FEL_BOLT);
                        events.RescheduleEvent(EVENT_DI_FEL_BOLT, urand(3000, 5000));
                        break;
                    case EVENT_DI_IMMOLATION:
                        DoCast(me, SPELL_DI_IMMOLATION_AURA);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_demon_invasion_genericAI(creature);
    }
};

// ============================================================================
// npc_demon_invasion_commander — Stage 2 Commander + Lieutenants
// ============================================================================
class npc_demon_invasion_commander : public CreatureScript
{
public:
    npc_demon_invasion_commander() : CreatureScript("npc_demon_invasion_commander") {}

    struct npc_demon_invasion_commanderAI : public ScriptedAI
    {
        npc_demon_invasion_commanderAI(Creature* creature) : ScriptedAI(creature)
        {
            enraged = false;
        }

        void Reset() override
        {
            events.Reset();
            enraged = false;
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_DI_SHADOW_CLEAVE, urand(5000, 8000));
            events.RescheduleEvent(EVENT_DI_FEL_STOMP, urand(10000, 15000));
            events.RescheduleEvent(EVENT_DI_ENRAGE_CHECK, 1000);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_DI_SHADOW_CLEAVE:
                        DoCastVictim(SPELL_DI_SHADOW_CLEAVE);
                        events.RescheduleEvent(EVENT_DI_SHADOW_CLEAVE, urand(8000, 12000));
                        break;
                    case EVENT_DI_FEL_STOMP:
                        DoCastAOE(SPELL_DI_FEL_STOMP);
                        events.RescheduleEvent(EVENT_DI_FEL_STOMP, urand(15000, 20000));
                        break;
                    case EVENT_DI_ENRAGE_CHECK:
                        if (!enraged && HealthBelowPct(30))
                        {
                            DoCast(me, SPELL_DI_ENRAGE);
                            enraged = true;
                        }
                        else
                            events.RescheduleEvent(EVENT_DI_ENRAGE_CHECK, 2000);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
        bool enraged;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_demon_invasion_commanderAI(creature);
    }
};

// ============================================================================
// npc_demon_invasion_boss — Stage 4 final boss
// ============================================================================
class npc_demon_invasion_boss : public CreatureScript
{
public:
    npc_demon_invasion_boss() : CreatureScript("npc_demon_invasion_boss") {}

    struct npc_demon_invasion_bossAI : public ScriptedAI
    {
        npc_demon_invasion_bossAI(Creature* creature) : ScriptedAI(creature)
        {
            phase2 = false;
        }

        void Reset() override
        {
            events.Reset();
            phase2 = false;
        }

        void EnterCombat(Unit* /*who*/) override
        {
            events.RescheduleEvent(EVENT_DI_SHADOW_CLEAVE, urand(5000, 8000));
            events.RescheduleEvent(EVENT_DI_FEL_STOMP, urand(12000, 18000));
            events.RescheduleEvent(EVENT_DI_METEOR, urand(20000, 30000));
            events.RescheduleEvent(EVENT_DI_BERSERK, 5 * 60 * 1000); // 5 min berserk
        }

        void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/, DamageEffectType /*dmgType*/) override
        {
            if (!phase2 && HealthBelowPct(40))
            {
                phase2 = true;
                DoCast(me, SPELL_DI_ENRAGE);
                // Speed up abilities in phase 2
                events.RescheduleEvent(EVENT_DI_BREATH, urand(5000, 10000));
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_DI_SHADOW_CLEAVE:
                        DoCastVictim(SPELL_DI_SHADOW_CLEAVE);
                        events.RescheduleEvent(EVENT_DI_SHADOW_CLEAVE, phase2 ? urand(4000, 6000) : urand(8000, 12000));
                        break;
                    case EVENT_DI_FEL_STOMP:
                        DoCastAOE(SPELL_DI_FEL_STOMP);
                        events.RescheduleEvent(EVENT_DI_FEL_STOMP, phase2 ? urand(10000, 14000) : urand(15000, 20000));
                        break;
                    case EVENT_DI_METEOR:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                            DoCast(target, SPELL_DI_METEOR_STRIKE);
                        events.RescheduleEvent(EVENT_DI_METEOR, phase2 ? urand(15000, 20000) : urand(25000, 35000));
                        break;
                    case EVENT_DI_BREATH:
                        DoCastVictim(SPELL_DI_FEL_FIRE_BREATH);
                        events.RescheduleEvent(EVENT_DI_BREATH, urand(12000, 18000));
                        break;
                    case EVENT_DI_BERSERK:
                        DoCast(me, SPELL_DI_BERSERK);
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

    private:
        EventMap events;
        bool phase2;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_demon_invasion_bossAI(creature);
    }
};

// ============================================================================
// Registration — called from AddSC_demon_invasion_prepatch()
// We register them separately but they're loaded via the same script loader
// ============================================================================

void AddSC_demon_invasion_creatures()
{
    new npc_demon_invasion_generic();
    new npc_demon_invasion_commander();
    new npc_demon_invasion_boss();
}

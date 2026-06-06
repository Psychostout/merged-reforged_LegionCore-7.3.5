/*
 * npc_ambient_ai.h
 * AI struct definition for ambient NPC companions.
 * Separated into a header so multiple translation units can use CAST_AI.
 * All methods are defined inline (inside the struct body).
 */

#ifndef NPC_AMBIENT_AI_H
#define NPC_AMBIENT_AI_H

#include "npc_ambient_world.h"

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
        _activeMitTimer(0),
        _hotTimer(0),
        _aoeTimer(0),
        _utilityTimer(urand(0, 15000)),
        _rogueEvasionTimer(0),
        _rogueEvasionActive(false),
        _resTimer(0),
        _combatResUsed(false),
        _regenTimer(0),
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
        me->SetReactState(REACT_PASSIVE);

        if (!_isCompanion)
        {
            uint32 agRoll = urand(0, 5);
            _ageArchetype = (agRoll == 0) ? 2 : (agRoll <= 2) ? 0 : 1;
            if (_ageArchetype == 0)
                me->SetObjectScale(frand(0.90f, 0.95f));
            else if (_ageArchetype == 2)
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
        _combatResUsed = false;
        if (!_isCompanion)
            me->SetReactState(REACT_PASSIVE);
    }

    // ── Damage helper: scales damage to be meaningful against targets ──
    // In instances, companions deal damage as % of TARGET max HP (like a real player).
    // In open world, companions deal damage as % of their OWN max HP (cosmetic).
    void DealScaledDamage(Unit* target, float pctSelf, float pctTargetDungeon, float pctTargetRaid)
    {
        if (!target || !target->IsAlive()) return;
        uint32 dmg;
        if (me->GetMap()->IsRaid())
            dmg = uint32(target->GetMaxHealth() * pctTargetRaid);
        else if (me->GetMap()->IsDungeon())
            dmg = uint32(target->GetMaxHealth() * pctTargetDungeon);
        else
            dmg = uint32(me->GetMaxHealth() * pctSelf);
        if (dmg < 1) dmg = 1;
        if (target->GetHealth() > dmg + 1)
            target->SetHealth(target->GetHealth() - dmg);
    }

    // ── Heal helper: scales healing relative to target max HP ──
    void DoScaledHeal(Unit* target, float pct)
    {
        if (!target || !target->IsAlive()) return;
        uint32 amt = uint32(target->GetMaxHealth() * pct);
        target->SetHealth(std::min(target->GetHealth() + amt, target->GetMaxHealth()));
    }

    // ── Check if the tank companion for our owner is alive ──
    Creature* FindTankCompanion()
    {
        for (uint32 ce = AMBIENT_ENTRY_MIN; ce <= AMBIENT_ENTRY_MAX; ++ce)
        {
            std::list<Creature*> cl;
            GetCreatureListWithEntryInGrid(cl, me, ce, 60.f);
            for (Creature* cm : cl)
            {
                if (!cm || !cm->IsAlive() || cm == me) continue;
                CompanionData ccd;
                if (!GetCompanionData(cm->GetGUID(), ccd) || ccd.ownerGuid != _ownerGuid) continue;
                if (GetRoleForEntry(cm->GetEntry()) == AMBIENT_TANK)
                    return cm;
            }
        }
        return nullptr;
    }

    // ── Find a dead companion for combat res ──
    Creature* FindDeadCompanion()
    {
        for (uint32 ce = AMBIENT_ENTRY_MIN; ce <= AMBIENT_ENTRY_MAX; ++ce)
        {
            std::list<Creature*> cl;
            GetCreatureListWithEntryInGrid(cl, me, ce, 60.f);
            for (Creature* cm : cl)
            {
                if (!cm || cm->IsAlive() || cm == me) continue;
                CompanionData ccd;
                if (!GetCompanionData(cm->GetGUID(), ccd) || ccd.ownerGuid != _ownerGuid) continue;
                return cm;
            }
        }
        return nullptr;
    }

    // ── Get the formation follow distance based on role ──
    float GetRoleFollowDistance() const
    {
        // Ranged classes stay back
        if (_role == AMBIENT_MAGE || _role == AMBIENT_HUNTER)
            return me->GetMap()->IsDungeon() ? 25.0f : _formDist;
        if (_role == AMBIENT_HEALER)
            return me->GetMap()->IsDungeon() ? 18.0f : _formDist;
        return _formDist;
    }

    void EnterCombat(Unit* who) override
    {
        if (!who || !_isCompanion)
            return;
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
            if (_state == STATE_WANDER)
                _timer = urand(800, 2000);
        }
    }

    void UpdateAI(uint32 diff) override;

    // ── Companion state (public so gossip handler can access) ──────────────
    bool        _isCompanion;
    ObjectGuid  _ownerGuid;
    std::string _myCompanionName;
    AmbientRole GetRole() const { return _role; }

    void HireCompanion(Player* player, uint32 restoreLevel = 0, uint32 restoreXp = 0, bool isReplica = false);
    void DismissCompanion();
    void JustDied(Unit* /*killer*/) override;
    void KilledUnit(Unit* killed) override;

private:
    AmbientState _state;
    uint32       _timer;
    AmbientRole  _role;
    float        _homeX, _homeY, _homeZ;
    uint32       _combatEmoteTimer;
    uint32       _healTimer;
    uint32       _healBigCdTimer;
    uint32       _spellTimer;
    uint32       _tauntTimer;
    uint32       _aoeThreatTimer;
    uint32       _shieldWallTimer;
    uint32       _interruptTimer;
    uint32       _interruptCheckTimer;
    uint32       _prioritySwitchTimer;
    uint32       _addScanTimer;
    uint32       _speechTimer;
    bool         _moveDone;
    bool         _isReplica;
    bool         _executeActive;
    bool         _fightingBoss;
    uint8        _ageArchetype;
    float        _formDist;
    float        _formAngle;
    uint32       _leashWhisperTimer;
    uint32       _workEmoteTimer;
    uint8        _workObjectType;
    uint32       _activeMitTimer;
    uint32       _hotTimer;
    uint32       _aoeTimer;
    uint32       _utilityTimer;
    uint32       _rogueEvasionTimer;
    bool         _rogueEvasionActive;
    uint32       _resTimer;
    bool         _combatResUsed;
    uint32       _regenTimer;

    void _SelectNextState();
    void _DoIdle();
    void _DoWander();
    void _DoWork();
    void _DoActivity();
    void _DoHunt();
    void _DoSocial();
};

#endif // NPC_AMBIENT_AI_H

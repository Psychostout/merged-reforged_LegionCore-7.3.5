/*
 * npc_ambient_ai.cpp
 * AI state machine (npc_ambient_aiAI) + CreatureScript (gossip handlers)
 */

#include "npc_ambient_world.h"

// ============================================================
//  AI state machine
// ============================================================
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
        _workObjectType(0),
        _pvpScanTimer(0)
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

            // PvP sync + assist — keep companion's PvP flag in sync with owner
            // and auto-assist when enemy players attack the owner
            if (_pvpScanTimer <= diff)
            {
                _pvpScanTimer = 2000;
                bool ownerPvP = owner->IsPvP();
                if (me->IsPvP() != ownerPvP)
                    me->SetPvP(ownerPvP);
                if (ownerPvP && !me->IsInCombat())
                {
                    for (Unit* att : owner->getAttackers())
                    {
                        if (!att || !att->IsAlive()) continue;
                        if (att->GetTypeId() == TYPEID_PLAYER)
                        {
                            me->GetMotionMaster()->Clear();
                            AttackStart(att);
                            break;
                        }
                    }
                }
            }
            else
                _pvpScanTimer -= diff;

            // Companion in combat — fight
            if (me->IsInCombat())
            {
                if (!me->GetVictim())
                {
                    // DPS/Healer: focus tank's target if possible
                    if (_role != AMBIENT_TANK)
                    {
                        Creature* tank = FindTankCompanion();
                        if (tank && tank->GetVictim())
                        {
                            AttackStart(tank->GetVictim());
                            return;
                        }
                    }
                    if (Unit* t = owner->GetVictim())
                        AttackStart(t);
                    return;
                }

            // H2: Healer — 4-tier priority healing + combat res
            if (_role == AMBIENT_HEALER)
            {
                if (_healBigCdTimer > diff)
                    _healBigCdTimer -= diff;
                else
                    _healBigCdTimer = 0;

                if (_resTimer > diff)
                    _resTimer -= diff;
                else
                    _resTimer = 0;

                // Combat Resurrection — one per combat, 30s cooldown
                if (!_combatResUsed && _resTimer == 0)
                {
                    Creature* dead = FindDeadCompanion();
                    if (dead)
                    {
                        dead->Respawn();
                        DoScaledHeal(dead, 0.50f);
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SPELL_CAST_W_SOUND);
                        CompanionData rcd;
                        std::string rname = (GetCompanionData(dead->GetGUID(), rcd) && !rcd.displayName.empty())
                            ? rcd.displayName : dead->GetName();
                        ChatHandler(owner->GetSession()).PSendSysMessage(
                            "|cff00ff00[Companion]|r %s has been resurrected!", rname.c_str());
                        _combatResUsed = true;
                        _resTimer = 30000;
                        _healTimer = urand(1000, 2000);
                    }
                }

                if (_healTimer <= diff)
                {
                    Unit*  healTarget = nullptr;
                    float  lowestPct  = 100.0f;
                    if (owner->IsAlive() && owner->GetHealthPct() < lowestPct)
                    {
                        lowestPct  = owner->GetHealthPct();
                        healTarget = owner;
                    }
                    for (uint32 ce = AMBIENT_ENTRY_MIN; ce <= AMBIENT_ENTRY_MAX; ++ce)
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

                    // TIER 1: Emergency heal (<20%) — massive heal, fast cast
                    if (healTarget && lowestPct < 20.0f)
                    {
                        DoScaledHeal(healTarget, 0.35f);
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SPELL_CAST);
                        _healTimer = _fightingBoss ? urand(800,  1500) : urand(1500, 2500);
                        return;
                    }
                    // TIER 2: Low HP (<40%) — strong heal
                    else if (healTarget && lowestPct < 40.0f)
                    {
                        DoScaledHeal(healTarget, 0.20f);
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SPELL_CAST);
                        _healTimer = _fightingBoss ? urand(1500, 2500) : urand(2500, 4000);
                        return;
                    }
                    // TIER 3: Medium HP (<65%) — maintenance heal
                    else if (healTarget && lowestPct < 65.0f)
                    {
                        DoScaledHeal(healTarget, 0.12f);
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SPELL_CAST);
                        _healTimer = _fightingBoss ? urand(2500, 4000) : urand(4000, 6000);
                    }
                    else
                        _healTimer = urand(1000, 2500);

                    // TIER 4: Group-heal big CD
                    if (_healBigCdTimer == 0 && healTarget && lowestPct < 65.0f)
                    {
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SPELL_CAST_W_SOUND);
                        DoScaledHeal(owner, 0.15f);
                        for (uint32 ce2 = AMBIENT_ENTRY_MIN; ce2 <= AMBIENT_ENTRY_MAX; ++ce2)
                        {
                            std::list<Creature*> cl2;
                            GetCreatureListWithEntryInGrid(cl2, me, ce2, 60.f);
                            for (Creature* cm2 : cl2)
                            {
                                if (!cm2->IsAlive() || cm2 == me) continue;
                                CompanionData ccd2;
                                if (!GetCompanionData(cm2->GetGUID(), ccd2) || ccd2.ownerGuid != _ownerGuid) continue;
                                DoScaledHeal(cm2, 0.15f);
                            }
                        }
                        _healBigCdTimer = 60000;
                    }
                }
                else
                    _healTimer -= diff;

                // H8: HoT — periodic small heal on the tank
                if (_hotTimer <= diff)
                {
                    Creature* tank = FindTankCompanion();
                    if (tank && tank->GetHealthPct() < 92.0f)
                        DoScaledHeal(tank, 0.03f);
                    _hotTimer = _fightingBoss ? urand(2500, 3500) : urand(3500, 5000);
                }
                else
                    _hotTimer -= diff;
            }

            // H1: Tank — threat + AoE threat + defensive CDs
            if (_role == AMBIENT_TANK && me->GetVictim())
            {
                if (_tauntTimer <= diff)
                {
                    float bonus = _fightingBoss ? 0.20f : 0.10f;
                    me->AddThreat(me->GetVictim(), me->GetMaxHealth() * bonus);
                    me->HandleEmoteCommand(EMOTE_ONESHOT_SALUTE);
                    _tauntTimer = urand(4000, 7000);
                }
                else
                    _tauntTimer -= diff;

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

                if (_shieldWallTimer <= diff)
                {
                    float hpPct = me->GetHealthPct();
                    if (hpPct < 20.0f)
                    {
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

                // H7: Active mitigation — periodic block/parry (self-heal simulation)
                if (_activeMitTimer <= diff)
                {
                    float hpPct = me->GetHealthPct();
                    if (_fightingBoss || hpPct < 85.0f)
                    {
                        uint32 mit = uint32(me->GetMaxHealth() * (_fightingBoss ? 0.06f : 0.04f));
                        me->SetHealth(std::min(me->GetHealth() + mit, me->GetMaxHealth()));
                        me->HandleEmoteCommand(EMOTE_ONESHOT_PARRY_SHIELD);
                    }
                    _activeMitTimer = _fightingBoss ? urand(3000, 5000) : urand(5000, 8000);
                }
                else
                    _activeMitTimer -= diff;
            }

            // A2: Mage — fireball + AoE burst when 3+ hostiles
            // Ranged: stay at 25y in instances
            if (_role == AMBIENT_MAGE && me->GetVictim())
            {
                float distToTarget = me->GetDistance(me->GetVictim());
                if (me->GetMap()->IsDungeon() && distToTarget < 20.0f)
                {
                    me->GetMotionMaster()->Clear();
                    float ang = me->GetAngle(me->GetVictim()) + float(M_PI);
                    float nx = me->GetPositionX() + std::cos(ang) * 10.f;
                    float ny = me->GetPositionY() + std::sin(ang) * 10.f;
                    me->GetMotionMaster()->MovePoint(0, nx, ny, me->GetPositionZ());
                }

                if (_aoeTimer > diff) _aoeTimer -= diff;
                else                  _aoeTimer  = 0;

                if (_spellTimer <= diff)
                {
                    std::list<Creature*> nearH;
                    me->GetCreatureListInGrid(nearH, 10.f);
                    uint32 hCount = 0;
                    for (Creature* nc : nearH)
                        if (nc && nc->IsAlive() && nc->IsHostileTo(me))
                            ++hCount;

                    if (hCount >= 3 && _aoeTimer == 0)
                    {
                        // AoE: Fireball each nearby hostile
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SPELL_CAST_W_SOUND);
                        for (Creature* nc : nearH)
                        {
                            if (nc && nc->IsAlive() && nc->IsHostileTo(me))
                                DealScaledDamage(nc, 0.015f, 0.012f, 0.008f);
                        }
                        _aoeTimer   = _fightingBoss ? urand(8000, 12000) : urand(12000, 18000);
                        _spellTimer = urand(2000, 3000);
                    }
                    else
                    {
                        // Single target: Fireball
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SPELL_CAST_W_SOUND);
                        DealScaledDamage(me->GetVictim(), 0.025f, 0.02f, 0.015f);
                        _spellTimer = _executeActive ? urand(1200, 2000) : urand(2500, 4000);
                    }
                }
                else
                    _spellTimer -= diff;
            }

            // A2: Hunter — aimed shot + multi-shot AoE
            // Ranged: stay at 25y in instances
            if (_role == AMBIENT_HUNTER && me->GetVictim())
            {
                float distToTarget = me->GetDistance(me->GetVictim());
                if (me->GetMap()->IsDungeon() && distToTarget < 20.0f)
                {
                    me->GetMotionMaster()->Clear();
                    float ang = me->GetAngle(me->GetVictim()) + float(M_PI);
                    float nx = me->GetPositionX() + std::cos(ang) * 10.f;
                    float ny = me->GetPositionY() + std::sin(ang) * 10.f;
                    me->GetMotionMaster()->MovePoint(0, nx, ny, me->GetPositionZ());
                }

                if (_aoeTimer > diff) _aoeTimer -= diff;
                else                  _aoeTimer  = 0;

                if (_spellTimer <= diff)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_BOW);
                    std::list<Creature*> nearH;
                    me->GetCreatureListInGrid(nearH, 30.f);
                    uint32 hCount = 0;
                    for (Creature* nc : nearH)
                        if (nc && nc->IsAlive() && nc->IsHostileTo(me))
                            ++hCount;

                    if (hCount >= 3 && _aoeTimer == 0)
                    {
                        // Multi-shot: damage all nearby hostiles
                        for (Creature* nc : nearH)
                        {
                            if (nc && nc->IsAlive() && nc->IsHostileTo(me))
                                DealScaledDamage(nc, 0.015f, 0.01f, 0.007f);
                        }
                        _aoeTimer = _fightingBoss ? urand(6000, 9000) : urand(10000, 15000);
                    }
                    else
                    {
                        // Aimed shot: single-target heavy hit
                        DealScaledDamage(me->GetVictim(), 0.025f, 0.02f, 0.015f);
                    }
                    _spellTimer = _executeActive ? urand(800, 1500) : urand(1800, 3000);
                }
                else
                    _spellTimer -= diff;
            }

            // A3: Rogue — backstab + evasion
            if (_role == AMBIENT_ROGUE && me->GetVictim())
            {
                if (_rogueEvasionTimer > diff) _rogueEvasionTimer -= diff;
                else                           _rogueEvasionTimer  = 0;

                if (_spellTimer <= diff)
                {
                    // Backstab: melee range, heavy single-target
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
                    float bsSelf = _executeActive ? 0.03f : 0.02f;
                    float bsDung = _executeActive ? 0.025f : 0.018f;
                    float bsRaid = _executeActive ? 0.02f  : 0.012f;
                    DealScaledDamage(me->GetVictim(), bsSelf, bsDung, bsRaid);
                    _spellTimer = _executeActive ? urand(1000, 1800) : urand(2000, 3500);
                }
                else
                    _spellTimer -= diff;

                // Evasion: when low HP, activate dodge (simulated as periodic self-heal)
                if (!_rogueEvasionActive && me->GetHealthPct() < 30.0f && _rogueEvasionTimer == 0)
                {
                    _rogueEvasionActive = true;
                    me->HandleEmoteCommand(EMOTE_ONESHOT_DODGE);
                    _rogueEvasionTimer = 45000;
                }
                if (_rogueEvasionActive)
                {
                    DoScaledHeal(me, 0.02f);
                    if (me->GetHealthPct() > 60.0f)
                        _rogueEvasionActive = false;
                }
            }

            // A4: Warrior — cleave + battle shout
            if (_role == AMBIENT_WARRIOR && me->GetVictim())
            {
                if (_spellTimer <= diff)
                {
                    // Cleave: hit all nearby hostiles
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
                    std::list<Creature*> nearH;
                    me->GetCreatureListInGrid(nearH, 8.f);
                    float clvSelf = _executeActive ? 0.025f : 0.015f;
                    float clvDung = _executeActive ? 0.018f : 0.012f;
                    float clvRaid = _executeActive ? 0.012f : 0.008f;
                    for (Creature* nc : nearH)
                    {
                        if (nc && nc->IsAlive() && nc->IsHostileTo(me))
                            DealScaledDamage(nc, clvSelf, clvDung, clvRaid);
                    }
                    _spellTimer = _executeActive ? urand(1500, 2500) : urand(3000, 5000);
                }
                else
                    _spellTimer -= diff;

                // Battle Shout: periodic party buff (simulated as small heal to all companions)
                if (_utilityTimer <= diff)
                {
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                    for (uint32 ce = AMBIENT_ENTRY_MIN; ce <= AMBIENT_ENTRY_MAX; ++ce)
                    {
                        std::list<Creature*> cl;
                        GetCreatureListWithEntryInGrid(cl, me, ce, 30.f);
                        for (Creature* cm : cl)
                        {
                            if (!cm->IsAlive()) continue;
                            CompanionData ccd;
                            if (!GetCompanionData(cm->GetGUID(), ccd) || ccd.ownerGuid != _ownerGuid) continue;
                            DoScaledHeal(cm, 0.02f);
                        }
                    }
                    _utilityTimer = urand(20000, 30000);
                }
                else
                    _utilityTimer -= diff;
            }

            // H3: Interrupt
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

            // H4: Priority target — DPS switches to mob attacking owner or low-HP ally
            if (_role != AMBIENT_TANK && _role != AMBIENT_HEALER)
            {
                if (_prioritySwitchTimer <= diff)
                {
                    _prioritySwitchTimer = _fightingBoss ? 2000 : 3000;
                    std::list<Creature*> nearby;
                    me->GetCreatureListInGrid(nearby, 40.f);

                    // S3: Rescue priority — if owner is critically low, rush to their attacker
                    if (owner->IsAlive() && owner->GetHealthPct() < 20.0f && owner->IsInCombat())
                    {
                        for (Creature* hc : nearby)
                        {
                            if (!hc || !hc->IsAlive() || !hc->IsHostileTo(me)) continue;
                            Unit* hcV = hc->GetVictim();
                            if (hcV && hcV->GetGUID() == _ownerGuid && hc != me->GetVictim())
                            {
                                me->GetMotionMaster()->Clear();
                                AttackStart(hc);
                                break;
                            }
                        }
                    }
                    else
                    {
                        // Normal priority: switch to mob attacking owner
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
                }
                else
                    _prioritySwitchTimer -= diff;
            }

            // H5: Add scan — Tank/Warrior intercepts threats
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

            // H6: Execute phase
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
                _combatEmoteTimer = _executeActive ? urand(800, 1500) : urand(2200, 4000);
            }
            else
                _combatEmoteTimer -= diff;
            DoMeleeAttackIfReady();
            return;
        }

        // ── Auto-engage logic ──
        // In INSTANCES: companions proactively scan for hostiles and engage
        //   - Tank rushes first to establish threat
        //   - DPS focuses tank's target
        //   - All roles auto-assist when any companion is attacked
        // In OPEN WORLD: companions only react when the OWNER starts combat
        bool inInstance = me->GetMap()->IsDungeon();

        if (inInstance)
        {
            // TANK: Proactively scan for hostiles attacking any ally — rush in first
            if (_role == AMBIENT_TANK)
            {
                std::list<Creature*> threats;
                me->GetCreatureListInGrid(threats, 40.f);
                for (Creature* hostile : threats)
                {
                    if (!hostile || !hostile->IsAlive() || !hostile->IsHostileTo(me)) continue;
                    if (!hostile->IsInCombat()) continue;
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ROAR);
                    me->GetMotionMaster()->Clear();
                    me->AddThreat(hostile, me->GetMaxHealth() * 0.30f);
                    AttackStart(hostile);
                    return;
                }
            }

            // ALL ROLES: Auto-assist when any companion or owner is under attack
            if (!me->IsInCombat())
            {
                std::list<Creature*> nearby;
                me->GetCreatureListInGrid(nearby, 40.f);
                for (Creature* hostile : nearby)
                {
                    if (!hostile || !hostile->IsAlive() || !hostile->IsHostileTo(me)) continue;
                    if (!hostile->IsInCombat()) continue;
                    Unit* hVictim = hostile->GetVictim();
                    if (!hVictim) continue;
                    bool isAlly = (hVictim->GetGUID() == _ownerGuid);
                    if (!isAlly)
                    {
                        CompanionData acd;
                        isAlly = (GetCompanionData(hVictim->GetGUID(), acd) && acd.ownerGuid == _ownerGuid);
                    }
                    if (isAlly)
                    {
                        // DPS: focus the tank's target when possible
                        if (_role != AMBIENT_TANK)
                        {
                            Creature* tank = FindTankCompanion();
                            if (tank && tank->GetVictim())
                            {
                                me->GetMotionMaster()->Clear();
                                AttackStart(tank->GetVictim());
                                return;
                            }
                        }
                        me->GetMotionMaster()->Clear();
                        AttackStart(hostile);
                        return;
                    }
                }
            }
        }

        // OPEN WORLD + INSTANCE fallback: assist when owner is directly in combat
        if (owner->IsInCombat())
        {
            if (Unit* t = owner->GetVictim())
            {
                me->GetMotionMaster()->Clear();
                AttackStart(t);
                return;
            }
        }

        // A1: Healer — out-of-combat top up
        if (_role == AMBIENT_HEALER)
        {
            if (_healTimer <= diff)
            {
                Unit*  healTarget = nullptr;
                float  lowestPct  = 95.0f;
                if (owner->IsAlive() && owner->GetHealthPct() < lowestPct)
                {
                    lowestPct  = owner->GetHealthPct();
                    healTarget = owner;
                }
                for (uint32 ce = AMBIENT_ENTRY_MIN; ce <= AMBIENT_ENTRY_MAX; ++ce)
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
                    _healTimer = urand(2000, 3500);
            }
            else
                _healTimer -= diff;
        }

        // Out-of-combat natural regen — ALL companions regen 5% HP every 3s when not fighting
        // Simulates the natural regen players have between pulls
        if (!me->IsInCombat() && me->IsAlive() && me->GetHealthPct() < 100.0f)
        {
            if (_regenTimer <= diff)
            {
                DoScaledHeal(me, 0.05f);
                _regenTimer = 3000;
            }
            else
                _regenTimer -= diff;
        }

        if (_leashWhisperTimer > diff)
            _leashWhisperTimer -= diff;
        else
            _leashWhisperTimer = 0;

        // Leash-back
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

        if (dist2Owner > 5.0f)
        {
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->MoveFollow(owner, _formDist, _formAngle);
        }
        return;
    }
    // ─── End companion mode ───────────────────────────────────────

        // Non-companion melee combat
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

        if (_speechTimer > diff) _speechTimer -= diff;
        else                     _speechTimer  = 0;

        // P1: Work emotes
        if (_state == STATE_WORK)
        {
            if (_workEmoteTimer <= diff)
            {
                if (_workObjectType == 0)
                    me->HandleEmoteCommand(EMOTE_ONESHOT_WORK);
                else
                    me->HandleEmoteCommand(EMOTE_ONESHOT_EAT_NO_SHEATHE);
                _workEmoteTimer = urand(2500, 4500);
            }
            else
                _workEmoteTimer -= diff;
        }

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
    std::string _myCompanionName;
    uint32      _pvpScanTimer;
    AmbientRole GetRole() const { return _role; }

    void HireCompanion(Player* player, uint32 restoreLevel = 0, uint32 restoreXp = 0, bool isReplica = false)
    {
        if (_isCompanion)
            return;

        // A3: Hard cap — 4 companions max
        if (restoreLevel == 0)
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
        _myCompanionName   = me->GetName();
        uint32 lvl         = restoreLevel > 0 ? restoreLevel : (uint32)me->getLevel();

        RegisterCompanion(me->GetGUID(), player->GetGUID(), lvl, _myCompanionName);
        if (isReplica)
        {
            std::lock_guard<std::mutex> lk(s_companionMutex);
            auto ri = s_companions.find(me->GetGUID());
            if (ri != s_companions.end())
                ri->second.isReplica = true;
        }

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

        // Formation follow — role-based distances
        // In instances: ranged/healers stay back, tank up front
        // In open world: tight formation behind player
        {
            static const float FORM_DIST_OW[]  = { 2.5f, 3.1f, 3.1f, 3.7f };
            static const float FORM_ANGLE_OW[] = {
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
            uint32 si = slotIdx < 4u ? slotIdx : slotIdx % 4u;

            if (player->GetMap()->IsDungeon())
            {
                // Instance formation: tank leads, ranged stay back
                if (_role == AMBIENT_TANK)
                {
                    _formDist  = 2.0f;
                    _formAngle = 0.0f; // in front of player
                }
                else if (_role == AMBIENT_HEALER)
                {
                    _formDist  = 15.0f + float(si) * 1.5f;
                    _formAngle = float(M_PI) + float(si) * 0.3f;
                }
                else if (_role == AMBIENT_MAGE || _role == AMBIENT_HUNTER)
                {
                    _formDist  = 18.0f + float(si) * 1.5f;
                    _formAngle = float(M_PI) - 0.4f + float(si) * 0.35f;
                }
                else
                {
                    _formDist  = 3.0f + float(si) * 0.5f;
                    _formAngle = float(M_PI) + float(si) * 0.42f;
                }
            }
            else
            {
                _formDist  = FORM_DIST_OW[si];
                _formAngle = FORM_ANGLE_OW[si];
            }
        }
        me->GetMotionMaster()->MoveFollow(player, _formDist, _formAngle);
        me->SetCreatorGUID(player->GetGUID());
        me->SetPvP(player->IsPvP());

        if (!player->GetGroup())
        {
            Group* grp = new Group();
            if (grp->Create(player))
                sGroupMgr->AddGroup(grp);
            else
                delete grp;
        }

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
        TryDisbandGroupIfLast(owner, myGuid);
        Reset();
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (_isCompanion)
        {
            Player*    owner  = ObjectAccessor::GetPlayer(*me, _ownerGuid);
            ObjectGuid myGuid = me->GetGUID();

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
            me->DespawnOrUnsummon(8000);
        }
    }

    void KilledUnit(Unit* killed) override
    {
        if (!_isCompanion || !killed)
            return;

        Player* owner = ObjectAccessor::GetPlayer(*me, _ownerGuid);
        if (!owner)
            return;

        uint32 xpGain = CompanionXpGain((uint32)killed->getLevel());

        // N2: Auto-loot — companions generate silver for the player on kills
        {
            uint32 copperAmt = urand(1, 3) * killed->getLevel() * 100;
            owner->ModifyMoney(copperAmt);
            std::string looterName = _myCompanionName.empty() ? me->GetName() : _myCompanionName;
            ChatHandler(owner->GetSession()).PSendSysMessage(
                "|cff888888[%s]|r Looted %u silver from the kill.",
                looterName.c_str(), copperAmt / 100);
        }

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
            me->PlayDistanceSound(5906);
            ChatHandler(owner->GetSession()).PSendSysMessage(
                "|cffffd700[Companion]|r %s reached level %u!",
                killerName.c_str(), myNewLevel);
        }
        {
            static const uint32 TITLE_THRESHOLDS[] = { 10, 50, 100, 250, 500 };
            for (uint32 t : TITLE_THRESHOLDS)
                if (myKills == t)
                    ChatHandler(owner->GetSession()).PSendSysMessage(
                        "|cffffd700[Companion]|r %s has earned the title \"%s\"!",
                        killerName.c_str(), GetCompanionTitle(myKills));
        }

        // Award XP to ALL other companions of the same owner
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
                    _state = STATE_IDLE;
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MoveIdle();
                    me->HandleEmoteCommand(urand(0,1) ? EMOTE_STATE_SLEEP : EMOTE_STATE_SIT);
                    if (_speechTimer == 0 && urand(0, 9) == 0)
                    {
                        me->Say(AmbientSpeech::Roll(me->GetEntry()), LANG_UNIVERSAL);
                        _speechTimer = 120000;
                    }
                    _timer = urand(18000, 40000);
                    return;
                }
                else if (nightRoll < 70) { _DoWander(); return; }
                else if (nightRoll < 88) { _DoIdle();   return; }
                else if (nightRoll < 96) { _DoSocial(); return; }
            }
        }

        uint32 roll = urand(0, 99);
        if      (roll < 38) _DoWander();
        else if (roll < 65) _DoIdle();
        else if (roll < 83) _DoHunt();
        else if (roll < 95) _DoActivity();
        else                _DoSocial();
    }

    void _DoIdle()
    {
        _state = STATE_IDLE;
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveIdle();

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

        if (urand(0, 6) == 0)
        {
            _DoWork();
            if (_state == STATE_WORK)
                return;
        }

        if (_speechTimer == 0 && urand(0, 4) == 0)
        {
            const char* line = AmbientSpeech::Roll(me->GetEntry());
            me->Say(line, LANG_UNIVERSAL);
            _speechTimer = 120000;
        }

        {
            time_t now = time(nullptr);
            int secsInDay = int(now % 86400);
            bool isNight  = (secsInDay >= 72000 || secsInDay < 21600);
            if (isNight && urand(0, 1))
            {
                me->HandleEmoteCommand(urand(0, 1) ? EMOTE_STATE_SLEEP : EMOTE_STATE_SIT);
                _timer = urand(14000, 30000);
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

    void _DoWander()
    {
        _state    = STATE_WANDER;
        _moveDone = false;

        float angle  = frand(0.f, float(M_PI) * 2.f);
        float dist   = frand(15.f, SPREAD_RADIUS);
        float destX  = _homeX + std::cos(angle) * dist;
        float destY  = _homeY + std::sin(angle) * dist;
        float destZ  = _homeZ;

        if (Map* m = me->GetMap())
        {
            float h = m->GetHeight(me->GetPhaseShift(), destX, destY, _homeZ + 5.f, true, 50.f);
            if (h > INVALID_HEIGHT + 1.f)
                destZ = h;
        }

        me->GetMotionMaster()->Clear();

        if (urand(0, 9) == 0)
            me->SetSpeed(MOVE_WALK, frand(3.5f, 5.0f));
        else if (_ageArchetype == 2)
            me->SetSpeed(MOVE_WALK, frand(1.6f, 1.9f));
        else if (_ageArchetype == 0)
            me->SetSpeed(MOVE_WALK, frand(2.2f, 2.8f));
        else
            me->SetSpeed(MOVE_WALK, frand(1.8f, 2.5f));

        me->GetMotionMaster()->MovePoint(1, destX, destY, destZ);
        _timer = urand(6000, 14000);
    }

    void _DoWork()
    {
        static const uint32 CRAFT_GO[] = { 1685, 2296, 3299, 2543 };
        static const uint32 FIRE_GO[]  = { 5177, 5178, 5179 };

        for (uint32 ge : CRAFT_GO)
        {
            if (GameObject* go = me->FindNearestGameObject(ge, 18.f))
            {
                _state          = STATE_WORK;
                _workObjectType = 0;
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
        for (uint32 fe : FIRE_GO)
        {
            if (GameObject* go = me->FindNearestGameObject(fe, 18.f))
            {
                _state          = STATE_WORK;
                _workObjectType = 1;
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
        _DoIdle();
    }

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

    void _DoHunt()
    {
        _state = STATE_HUNT;
        Creature* target = nullptr;

        std::list<Creature*> nearList;
        me->GetCreatureListInGrid(nearList, 40.f);
        for (Creature* c : nearList)
        {
            if (target) break;
            if (c == me || !c->IsAlive()) continue;
            if (!c->IsHostileTo(me)) continue;
            if (c->IsSummon()) continue;
            if (c->IsInCombat()) continue;
            if (c->GetEntry() >= AMBIENT_ENTRY_MIN && c->GetEntry() <= AMBIENT_ENTRY_MAX) continue;
            if (c->IsCritter()) continue;
            if (c->IsVendor() || c->IsTrainer() || c->IsQuestGiver() || c->IsGossip()) continue;
            if (c->GetReactState() != REACT_AGGRESSIVE) continue;
            target = c;
        }

        if (target)
        {
            me->GetMotionMaster()->Clear();
            AttackStart(target);
            _timer = urand(10000, 20000);
            return;
        }

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
                    me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK_BOW);
                else
                    me->HandleEmoteCommand(EMOTE_ONESHOT_POINT);
                _timer = urand(4000, 8000);
                return;
            }
        }

        _DoWander();
    }

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
            if (c->GetEntry() < AMBIENT_ENTRY_MIN || c->GetEntry() > AMBIENT_ENTRY_MAX) continue;
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
// ============================================================
PartyCompo ComputePartyCompo(Player* player)
{
    PartyCompo out;
    for (uint32 e = AMBIENT_ENTRY_MIN; e <= AMBIENT_ENTRY_MAX; ++e)
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

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        // T3: Gossip rate limiter
        if (!CheckGossipRateLimit(player->GetGUID()))
            return true;

        npc_ambient_aiAI* ai = CAST_AI(npc_ambient_aiAI, creature->AI());
        if (!ai)
            return false;

        if (ai->_isCompanion && ai->_ownerGuid == player->GetGUID())
        {
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

            std::string hireText = std::string("Join my party!  [") + RoleName(ai->GetRole()) + "]";
            AddGossipItemFor(player, GOSSIP_ICON_BATTLE, hireText,
                COMPANION_GOSSIP_SENDER, COMPANION_ACTION_HIRE);

            if (compo.total < 4)
                AddGossipItemFor(player, GOSSIP_ICON_BATTLE,
                    "Form a balanced party  (1 Tank, 1 Healer, 2 DPS)",
                    COMPANION_GOSSIP_SENDER, COMPANION_ACTION_AUTOPARTY);

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
            {
                uint32 entry = creature->GetEntry();
                bool entryAlliance = (entry >= 9500080 && entry <= 9500084);
                bool entryHorde    = (entry >= 9500085 && entry <= 9500089);
                if ((entryAlliance && player->GetTeam() == HORDE) ||
                    (entryHorde    && player->GetTeam() == ALLIANCE))
                {
                    ChatHandler(player->GetSession()).PSendSysMessage(
                        "|cffff4444[Companion]|r This adventurer is not willing to follow someone of your faction.");
                    break;
                }
                ai->HireCompanion(player);
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "|cff00ff00[Companion]|r %s joins your party!",
                    creature->GetName().c_str());
                break;
            }
            case COMPANION_ACTION_DISMISS:
                ai->DismissCompanion();
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "|cffaaaaaa[Companion]|r %s has departed.",
                    creature->GetName().c_str());
                break;
            case COMPANION_ACTION_CLOSE:
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

                std::vector<Creature*> availTanks, availHealers, availDps;
                bool playerAlliance = (player->GetTeam() == ALLIANCE);
                for (uint32 e = AMBIENT_ENTRY_MIN; e <= AMBIENT_ENTRY_MAX; ++e)
                {
                    // Skip wrong-faction entries
                    bool eAlliance = (e >= 9500080 && e <= 9500084);
                    bool eHorde    = (e >= 9500085 && e <= 9500089);
                    if (eAlliance && !playerAlliance) continue;
                    if (eHorde    &&  playerAlliance) continue;
                    std::list<Creature*> cl;
                    GetCreatureListWithEntryInGrid(cl, player, e, 100.f);
                    for (Creature* c : cl)
                    {
                        if (!c->IsSummon() || c->ToTempSummon()->GetSummonerGUID() != player->GetGUID()) continue;
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

// Called from npc_ambient_world.cpp registration
npc_ambient_ai* CreateAmbientAIScript() { return new npc_ambient_ai(); }

// ============================================================
//  Free-function wrappers — so other .cpp files can call AI
//  methods without needing the full struct definition
// ============================================================
bool AmbientHireCompanion(Creature* creature, Player* player,
    uint32 restoreLevel, uint32 restoreXp, bool isReplica)
{
    npc_ambient_aiAI* ai = CAST_AI(npc_ambient_aiAI, creature->AI());
    if (!ai) return false;
    ai->HireCompanion(player, restoreLevel, restoreXp, isReplica);
    return true;
}

bool AmbientDismissCompanion(Creature* creature)
{
    npc_ambient_aiAI* ai = CAST_AI(npc_ambient_aiAI, creature->AI());
    if (!ai) return false;
    ai->DismissCompanion();
    return true;
}

bool IsAmbientCompanionCreature(Creature* creature)
{
    npc_ambient_aiAI* ai = CAST_AI(npc_ambient_aiAI, creature->AI());
    return ai && ai->_isCompanion;
}

bool IsAmbientCompanionOf(Creature* creature, ObjectGuid playerGuid)
{
    npc_ambient_aiAI* ai = CAST_AI(npc_ambient_aiAI, creature->AI());
    return ai && ai->_isCompanion && ai->_ownerGuid == playerGuid;
}

AmbientRole AmbientGetRole(Creature* creature)
{
    npc_ambient_aiAI* ai = CAST_AI(npc_ambient_aiAI, creature->AI());
    if (!ai) return AMBIENT_DEFAULT;
    return ai->GetRole();
}

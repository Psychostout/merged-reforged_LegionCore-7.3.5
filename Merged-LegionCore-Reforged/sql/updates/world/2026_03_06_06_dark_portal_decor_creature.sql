-- Créature décorative Porte des Ténèbres (map 1265)
-- Entry 424000 : clone visuel de Iron Grunt (78883) — non-attaquable, neutre, statique
-- Unit flags : UNIT_FLAG_NON_ATTACKABLE (0x2) | UNIT_FLAG_NOT_SELECTABLE (0x8000) = 32770
-- Faction 35 = neutre (n'attaque pas les joueurs)
-- Aucun AI ni script (décoration pure, pas de déplacement)

-- Template
INSERT IGNORE INTO creature_template
  (entry, gossip_menu_id, minlevel, maxlevel, HealthScalingExpansion, SandboxScalingID,
   exp, faction, npcflag, npcflag2,
   speed_walk, speed_run, speed_fly, scale,
   mindmg, maxdmg, dmgschool, attackpower, dmg_multiplier, baseattacktime, rangeattacktime,
   unit_class, unit_flags, unit_flags2, unit_flags3, dynamicflags,
   trainer_type, trainer_spell, trainer_class, trainer_race,
   minrangedmg, maxrangedmg, rangedattackpower,
   lootid, pickpocketloot, skinloot,
   resistance1, resistance2, resistance3, resistance4, resistance5, resistance6,
   spell1, spell2, spell3, spell4, spell5, spell6, spell7, spell8,
   PetSpellDataId, VehicleId, mingold, maxgold,
   AIName, MovementType, HoverHeight, Mana_mod_extra, Armor_mod, RegenHealth,
   mechanic_immune_mask, flags_extra, ControllerID,
   WorldEffects, PassiveSpells, StateWorldEffectID, SpellStateVisualID,
   SpellStateAnimID, SpellStateAnimKitID, IgnoreLos, AffixState, MaxVisible, ScriptName)
VALUES
  (424000, 0, 90, 90, 5, 0,
   4, 35, 0, 0,
   1, 1.14286, 1.14286, 1,
   0, 0, 0, 0, 1, 2000, 2000,
   1, 32770, 2048, 0, 0,
   0, 0, 0, 0,
   0, 0, 0,
   0, 0, 0,
   0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0,
   '', 0, 1, 1, 1, 1,
   0, 0, 0,
   '', '', 0, 0,
   0, 0, 0, 0, 0, '');

-- WDB (modèle client — même displayIDs qu'Iron Grunt 78883)
INSERT IGNORE INTO creature_template_wdb
  (Entry, Name1, Title, TypeFlags, TypeFlags2, Type, Family, Classification,
   KillCredit1, KillCredit2, VignetteID,
   Displayid1, Displayid2, Displayid3, Displayid4,
   HpMulti, PowerMulti, Leader,
   QuestItem1, QuestItem2, QuestItem3, QuestItem4, QuestItem5,
   QuestItem6, QuestItem7, QuestItem8, QuestItem9, QuestItem10,
   MovementInfoID, RequiredExpansion, FlagQuest, VerifiedBuild)
VALUES
  (424000, 'Iron Grunt', '', 0, 0, 7, 0, 0,
   0, 0, 0,
   59613, 59614, 59615, 59616,
   1.5, 1, 0,
   0, 0, 0, 0, 0,
   0, 0, 0, 0, 0,
   0, 5, 0, 22566);

-- Spawn — Point 1 : Z=94.6 (niveau haut de la Porte des Ténèbres)
-- map=1265, phaseId=3248, statique, neutre
INSERT IGNORE INTO creature
  (guid, id, map, zoneId, areaId, spawnMask, phaseMask, PhaseId,
   modelid, equipment_id,
   position_x, position_y, position_z, orientation,
   spawntimesecs, spawndist, currentwaypoint, curhealth, curmana,
   MovementType, npcflag, npcflag2, unit_flags, dynamicflags,
   AiID, MovementID, MeleeID, isActive, skipClone, personal_size, isTeemingSpawn, unit_flags3)
VALUES
  (146895479, 424000, 1265, 7025, 7037, 1, 1, '3248',
   0, 0,
   4071.7, -2372.5, 94.6, 1.57,
   120, 0, 0, 0, 0,
   0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0);

-- ==========================================================================
-- Demon Invasion Prepatch - Creature Templates
-- Phase 2: NPC definitions for invasion demons
-- ==========================================================================

-- Clean up previous entries
DELETE FROM `creature_template` WHERE `entry` BETWEEN 900001 AND 900099;
DELETE FROM `creature_template_wdb` WHERE `Entry` BETWEEN 900001 AND 900099;

-- ========================================
-- creature_template_wdb (client-side data: names, models, classification)
-- ========================================
-- Classification: 0=Normal, 1=Elite, 2=Rare Elite, 3=World Boss, 4=Rare
-- Type: 3=Demon
-- DisplayIDs sourced from existing Legion demon creatures
INSERT INTO `creature_template_wdb` (`Entry`, `Name1`, `Name2`, `Name3`, `Name4`, `NameAlt1`, `NameAlt2`, `NameAlt3`, `NameAlt4`, `Title`, `TitleAlt`, `CursorName`, `TypeFlags`, `TypeFlags2`, `Type`, `Family`, `Classification`, `KillCredit1`, `KillCredit2`, `VignetteID`, `Displayid1`, `Displayid2`, `Displayid3`, `Displayid4`, `HpMulti`, `PowerMulti`, `Leader`, `QuestItem1`, `QuestItem2`, `QuestItem3`, `QuestItem4`, `QuestItem5`, `QuestItem6`, `QuestItem7`, `QuestItem8`, `QuestItem9`, `QuestItem10`, `MovementInfoID`, `RequiredExpansion`, `FlagQuest`, `VerifiedBuild`) VALUES
-- Stage 1/3: Generic demons (Normal classification)
(900001, 'Fel Invader',       '', '', '', '', '', '', '', '', '', '', 0, 0, 3, 0, 0, 0, 0, 0, 62277, 0, 0, 0, 4,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 1),  -- Felguard model
(900002, 'Felstalker',        '', '', '', '', '', '', '', '', '', '', 0, 0, 3, 0, 0, 0, 0, 0, 62511, 0, 0, 0, 3,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 1),  -- Felstalker model
(900003, 'Fel Caster',        '', '', '', '', '', '', '', '', '', '', 0, 0, 3, 0, 0, 0, 0, 0, 63524, 0, 0, 0, 2,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 1),  -- Interrogator (caster) model
(900004, 'Dread Infernal',    '', '', '', '', '', '', '', '', '', '', 0, 0, 3, 0, 0, 0, 0, 0, 64169, 0, 0, 0, 8,  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 1),  -- Infernal model
-- Stage 2: Commander + Lieutenants (Elite classification = 1)
(900010, 'Legion Commander',  '', '', '', '', '', '', '', '', '', '', 0, 0, 3, 0, 1, 0, 0, 0, 64027, 0, 0, 0, 30, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 1),  -- Force-Commander model
(900011, 'Fel Lieutenant',    '', '', '', '', '', '', '', '', '', '', 0, 0, 3, 0, 1, 0, 0, 0, 70159, 0, 0, 0, 15, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 1),  -- Wrathguard model
(900012, 'Shadow Lieutenant', '', '', '', '', '', '', '', '', '', '', 0, 0, 3, 0, 1, 0, 0, 0, 68418, 0, 0, 0, 15, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 1),  -- Observer model
-- Stage 4: Boss (Rare Elite classification = 2)
(900020, 'Dread Commander Kazak', '', '', '', '', '', '', '', 'Herald of the Legion', '', '', 0, 0, 3, 0, 2, 0, 0, 0, 65330, 0, 0, 0, 100, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 1),  -- Fel Lord model
-- Vendor
(900050, 'Captive Wyrmtongue', '', '', '', '', '', '', '', 'Nethershard Vendor', '', '', 0, 0, 3, 0, 0, 0, 0, 0, 64485, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 6, 0, 1);  -- Putrid Alchemist model

-- ========================================
-- creature_template (server-side data: stats, faction, spells, scripts)
-- ========================================
-- faction 90 = Burning Legion (hostile to all players)
-- faction 35 = Friendly to all (for vendor)
-- unit_class: 1=Warrior, 2=Paladin(melee+mana), 8=Mage(caster)
-- flags_extra: 0x1=CREATURE_FLAG_EXTRA_INSTANCE_BIND, 0x100=CREATURE_FLAG_EXTRA_NO_XP_AT_KILL, etc.

INSERT INTO `creature_template` (`entry`, `gossip_menu_id`, `minlevel`, `maxlevel`, `HealthScalingExpansion`, `SandboxScalingID`, `exp`, `faction`, `npcflag`, `npcflag2`, `speed_walk`, `speed_run`, `speed_fly`, `scale`, `mindmg`, `maxdmg`, `dmgschool`, `attackpower`, `dmg_multiplier`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `unit_flags2`, `unit_flags3`, `dynamicflags`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `minrangedmg`, `maxrangedmg`, `rangedattackpower`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `Mana_mod_extra`, `Armor_mod`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ControllerID`, `WorldEffects`, `PassiveSpells`, `StateWorldEffectID`, `SpellStateVisualID`, `SpellStateAnimID`, `SpellStateAnimKitID`, `IgnoreLos`, `AffixState`, `MaxVisible`, `ScriptName`) VALUES
-- Stage 1/3: Fel Invader (melee warrior) — 73 values
(900001, 0, 100, 100, 0, 0, 5, 90, 0, 0, 1.0, 1.14286, 1.14286, 1.0, 500, 700, 0, 350, 2.0, 2000, 0, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1500, 2500, '', 0, 1, 1, 1, 1, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 'npc_demon_invasion_generic'),
-- Stage 1/3: Felstalker (fast melee)
(900002, 0, 100, 100, 0, 0, 5, 90, 0, 0, 1.2, 1.42857, 1.14286, 0.9, 400, 550, 0, 280, 1.5, 1500, 0, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1000, 1800, '', 0, 1, 1, 1, 1, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 'npc_demon_invasion_generic'),
-- Stage 1/3: Fel Caster (ranged)
(900003, 0, 100, 100, 0, 0, 5, 90, 0, 0, 1.0, 1.14286, 1.14286, 1.0, 300, 450, 4, 200, 1.5, 2500, 2500, 8, 0, 2048, 0, 0, 0, 0, 0, 0, 300, 450, 200, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1200, 2000, '', 0, 1, 1, 1, 1, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 'npc_demon_invasion_generic'),
-- Stage 1/3: Dread Infernal (tank, slow, high HP)
(900004, 0, 100, 100, 0, 0, 5, 90, 0, 0, 0.8, 1.0, 1.14286, 1.5, 600, 900, 0, 400, 3.0, 2500, 0, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2000, 4000, '', 0, 1, 1, 2, 1, 0, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 'npc_demon_invasion_generic'),
-- Stage 2: Legion Commander (elite, strong)
(900010, 0, 101, 102, 0, 0, 5, 90, 0, 0, 1.0, 1.14286, 1.14286, 1.3, 800, 1200, 0, 600, 5.0, 2000, 0, 2, 0, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5000, 10000, '', 0, 1, 1, 2, 1, 617299967, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 'npc_demon_invasion_commander'),
-- Stage 2: Fel Lieutenant A (elite)
(900011, 0, 101, 102, 0, 0, 5, 90, 0, 0, 1.0, 1.14286, 1.14286, 1.2, 700, 1000, 0, 500, 4.0, 2000, 0, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3000, 6000, '', 0, 1, 1, 2, 1, 617299967, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 'npc_demon_invasion_commander'),
-- Stage 2: Shadow Lieutenant B (elite)
(900012, 0, 101, 102, 0, 0, 5, 90, 0, 0, 1.0, 1.14286, 1.14286, 1.2, 700, 1000, 0, 500, 4.0, 2000, 0, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3000, 6000, '', 0, 1, 1, 2, 1, 617299967, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 'npc_demon_invasion_commander'),
-- Stage 4: Boss (rare elite, very strong, mechanic immune)
(900020, 0, 102, 102, 0, 0, 5, 90, 0, 0, 1.0, 1.14286, 1.14286, 1.8, 1200, 1800, 0, 800, 10.0, 2000, 0, 2, 0, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20000, 50000, '', 0, 1, 1, 3, 1, 617299967, 0, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 'npc_demon_invasion_boss'),
-- Vendor (friendly, npcflag 128 = vendor)
(900050, 0, 100, 100, 0, 0, 5, 35, 128, 0, 1.0, 1.14286, 1.14286, 1.0, 0, 0, 0, 0, 1.0, 2000, 0, 1, 0, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 0, 2, 0, '', '', 0, 0, 0, 0, 0, 0, 0, '');

-- ========================================
-- creature_model_info for our DisplayIDs (if not already present)
-- ========================================
INSERT IGNORE INTO `creature_model_info` (`DisplayID`, `BoundingRadius`, `CombatReach`, `DisplayID_Other_Gender`, `hostileId`) VALUES
(62277, 1.5, 2.0, 0, 0),   -- Felguard Invader
(62511, 1.0, 1.5, 0, 0),   -- Felstalker/Hungering Stalker
(63524, 1.0, 1.5, 0, 0),   -- Interrogator (caster)
(64169, 2.0, 3.0, 0, 0),   -- Hellfire Infernal
(64027, 1.5, 2.5, 0, 0),   -- Force-Commander
(70159, 1.0, 2.0, 0, 0),   -- Wrathguard
(68418, 1.5, 2.0, 0, 0),   -- Observer
(65330, 2.5, 4.0, 0, 0),   -- Fel Lord (boss)
(64485, 0.8, 1.0, 0, 0);   -- Putrid Alchemist (vendor)

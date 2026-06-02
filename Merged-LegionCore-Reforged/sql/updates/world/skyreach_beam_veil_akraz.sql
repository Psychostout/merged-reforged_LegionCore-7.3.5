-- ==========================================================================
-- Skyreach Beam — Veil Akraz ambient event (Spires of Arak)
-- Controller (900100) + Beam Source (900101) + Beam Target (900102) + Fire GO
-- Run against world_legion database
-- ==========================================================================

-- Clean up previous entries
DELETE FROM `creature` WHERE `id` IN (900100, 900101, 900102);
DELETE FROM `creature_template` WHERE `entry` IN (900100, 900101, 900102);
DELETE FROM `creature_template_wdb` WHERE `Entry` IN (900100, 900101, 900102);

-- ========================================
-- creature_template_wdb
-- ========================================
INSERT INTO `creature_template_wdb` (`Entry`, `Name1`, `Name2`, `Name3`, `Name4`, `NameAlt1`, `NameAlt2`, `NameAlt3`, `NameAlt4`, `Title`, `TitleAlt`, `CursorName`, `TypeFlags`, `TypeFlags2`, `Type`, `Family`, `Classification`, `KillCredit1`, `KillCredit2`, `VignetteID`, `Displayid1`, `Displayid2`, `Displayid3`, `Displayid4`, `HpMulti`, `PowerMulti`, `Leader`, `QuestItem1`, `QuestItem2`, `QuestItem3`, `QuestItem4`, `QuestItem5`, `QuestItem6`, `QuestItem7`, `QuestItem8`, `QuestItem9`, `QuestItem10`, `MovementInfoID`, `RequiredExpansion`, `FlagQuest`, `VerifiedBuild`) VALUES
-- Controller (invisible stalker)
(900100, 'Skyreach Beam Controller', '', '', '', '', '', '', '', '', '', '', 0, 0, 10, 0, 0, 0, 0, 0, 11686, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 1),
-- Beam source (displayId 169 = tiny trigger, nearly invisible)
(900101, 'Skyreach Beam Source', '', '', '', '', '', '', '', '', '', '', 0, 0, 10, 0, 0, 0, 0, 0, 169, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 1),
-- Beam target (displayId 169 = tiny trigger, nearly invisible)
(900102, 'Skyreach Beam Target', '', '', '', '', '', '', '', '', '', '', 0, 0, 10, 0, 0, 0, 0, 0, 169, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 1);

-- ========================================
-- creature_template
-- ========================================
INSERT INTO `creature_template` (`entry`, `gossip_menu_id`, `minlevel`, `maxlevel`, `HealthScalingExpansion`, `SandboxScalingID`, `exp`, `faction`, `npcflag`, `npcflag2`, `speed_walk`, `speed_run`, `speed_fly`, `scale`, `mindmg`, `maxdmg`, `dmgschool`, `attackpower`, `dmg_multiplier`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `unit_flags2`, `unit_flags3`, `dynamicflags`, `trainer_type`, `trainer_spell`, `trainer_class`, `trainer_race`, `minrangedmg`, `maxrangedmg`, `rangedattackpower`, `lootid`, `pickpocketloot`, `skinloot`, `resistance1`, `resistance2`, `resistance3`, `resistance4`, `resistance5`, `resistance6`, `spell1`, `spell2`, `spell3`, `spell4`, `spell5`, `spell6`, `spell7`, `spell8`, `PetSpellDataId`, `VehicleId`, `mingold`, `maxgold`, `AIName`, `MovementType`, `HoverHeight`, `Mana_mod_extra`, `Armor_mod`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ControllerID`, `WorldEffects`, `PassiveSpells`, `StateWorldEffectID`, `SpellStateVisualID`, `SpellStateAnimID`, `SpellStateAnimKitID`, `IgnoreLos`, `AffixState`, `MaxVisible`, `ScriptName`) VALUES
-- Controller: invisible, not selectable
(900100, 0, 100, 100, 0, 0, 5, 35, 0, 0, 1.0, 1.14286, 1.14286, 1.0, 0, 0, 0, 0, 1.0, 2000, 0, 1, 33554434, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 0, 2, 0, '', '', 0, 0, 0, 0, 0, 0, 0, 'npc_skyreach_beam_controller'),
-- Beam source: visible, MaxVisible=1, not selectable, flies, scale 5.0
(900101, 0, 100, 100, 0, 0, 5, 35, 0, 0, 1.0, 1.14286, 1.14286, 5.0, 0, 0, 0, 0, 1.0, 2000, 0, 1, 33554434, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 0, 2, 0, '', '', 0, 0, 0, 0, 0, 0, 1, 'npc_skyreach_beam_source'),
-- Beam target: visible, MaxVisible=1, not selectable, flies, scale 5.0
(900102, 0, 100, 100, 0, 0, 5, 35, 0, 0, 1.0, 1.14286, 1.14286, 5.0, 0, 0, 0, 0, 1.0, 2000, 0, 1, 33554434, 2048, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '', 0, 1, 1, 1, 1, 0, 2, 0, '', '', 0, 0, 0, 0, 0, 0, 1, 'npc_skyreach_beam_target');

-- ========================================
-- creature_template_movement — force flight (no gravity)
-- ========================================
DELETE FROM `creature_template_movement` WHERE `CreatureId` IN (900101, 900102);
INSERT INTO `creature_template_movement` (`CreatureId`, `Flight`) VALUES
(900101, 1),
(900102, 1);

-- ========================================
-- Ground Fire (large) — clone of 248100 with size 2.0
-- ========================================
DELETE FROM `gameobject_template` WHERE `entry` = 248101;
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `size`, `VerifiedBuild`) VALUES
(248101, 5, 32556, 'Ground Fire Large', 2.0, 21287);

-- ========================================
-- Permanent spawns
-- ========================================
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnMask`, `phaseMask`, `PhaseId`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `spawndist`, `currentwaypoint`, `curhealth`, `curmana`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags3`) VALUES
-- Controller at breach center
(900100, 900100, 1116, 6722, 0, 1, 1, '', 0, 0, 376, 2060, -7, 0, 120, 0, 0, 1, 0, 0, 0, 0, 0),
-- Beam source at Skyreach summit (pierre de la Flèche d'Arak)
(900101, 900101, 1116, 6722, 0, 1, 1, '', 0, 0, -49.5, 2306.3, 566.1, 5.21, 120, 0, 0, 1, 0, 0, 0, 0, 0),
-- Beam target at relay point
(900102, 900102, 1116, 6722, 0, 1, 1, '', 0, 0, 130.1, 2183, 325.5, 0, 120, 0, 0, 1, 0, 0, 0, 0, 0);

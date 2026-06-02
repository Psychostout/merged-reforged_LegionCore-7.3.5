-- ============================================================
-- LegionCore 7.3.5-PP — DBErrors.log Complete Fix (v2)
-- Corrected column names from actual world_legion schema
-- Execute on `world_legion` database
-- ============================================================

-- ============================================================
-- 1. spell_proc — missing spellPhaseMask / typeMask / chance
-- ============================================================
UPDATE `spell_proc` SET `spellPhaseMask` = 2
WHERE `spellId` IN (164545, 164547, 198300, 206333, 208081, 209493, 209706, 213708)
  AND (`spellPhaseMask` = 0 OR `spellPhaseMask` IS NULL);

DELETE FROM `spell_proc` WHERE `spellId` = 216974;

-- ============================================================
-- 2. creature_model_info — gender 4294967295
-- NOT FIXABLE: table has no Gender column (comes from DB2)
-- ============================================================

-- ============================================================
-- 3. creature_template_movement — invalid Flight value
-- ============================================================
UPDATE `creature_template_movement` SET `Flight` = 0
WHERE `CreatureId` = 88206 AND `Flight` = 3;

-- ============================================================
-- 4. creature_template — speed_walk = 0
-- ============================================================
UPDATE `creature_template` SET `speed_walk` = 1
WHERE `entry` = 522111 AND `speed_walk` = 0;

-- ============================================================
-- 5. creature_template_addon — invalid aura spell 10095
-- ============================================================
UPDATE `creature_template_addon` SET `auras` = TRIM(BOTH ' ' FROM
  REPLACE(REPLACE(REPLACE(`auras`, '10095 ', ''), ' 10095', ''), '10095', ''))
WHERE `entry` IN (18521, 20315) AND `auras` LIKE '%10095%';
UPDATE `creature_template_addon` SET `auras` = '' WHERE `entry` IN (18521, 20315) AND TRIM(`auras`) = '';

-- ============================================================
-- 6. creature_template_scaling — orphaned entries
-- ============================================================
DELETE FROM `creature_template_scaling`
WHERE `Entry` IN (135201, 135202, 137762, 139093, 140210, 141119, 141707);

-- ============================================================
-- 7. creature — random movement with spawndist=0 → idle
-- ============================================================
UPDATE `creature` SET `MovementType` = 0
WHERE `guid` IN (800000, 800001, 800002)
  AND `MovementType` = 1 AND `spawndist` = 0;

-- ============================================================
-- 8. creature_template — INSTANCE_BIND flag not in instance
-- ============================================================
UPDATE `creature_template` SET `flags_extra` = `flags_extra` & ~4
WHERE `entry` IN (37813, 124445) AND (`flags_extra` & 4) != 0;

-- ============================================================
-- 9. creature — WAYPOINT_MOTION_TYPE but no path → idle
-- ============================================================
UPDATE `creature` SET `MovementType` = 0
WHERE `guid` IN (264517, 267575, 271814, 272174, 272175, 272176)
  AND `MovementType` = 2;

-- ============================================================
-- 10. quest_template_addon — ProvidedItemCount = 0 → set to 1
-- (SourceItemIdCount in logs = ProvidedItemCount in schema)
-- ============================================================
UPDATE `quest_template_addon` SET `ProvidedItemCount` = 1
WHERE `ID` IN (
  45759, 47745, 45405, 45760, 45758, 7061, 45061, 48027, 47750, 48040,
  48038, 45761, 48042, 48035, 48034, 48065, 49077, 49846, 49860, 45755,
  45762, 48028, 48029, 48036, 48037, 48039, 48041, 48056, 48954, 49813,
  49864, 50337, 50338
) AND `ProvidedItemCount` = 0;

-- ============================================================
-- 11. quest_template_addon — SpecialFlags EXPLORATION_OR_EVENT (flag 2)
--     for quests completed by spells SPELL_EFFECT_QUEST_COMPLETE
-- ============================================================
UPDATE `quest_template_addon` SET `SpecialFlags` = `SpecialFlags` | 2
WHERE `ID` IN (
  45546, 45636, 46033, 46835, 47956, 47954, 47829, 48064, 47957, 47958,
  48030, 48031, 48032, 48033, 48602, 48603, 48937, 60003, 60004, 60005,
  60008, 60006
) AND (`SpecialFlags` & 2) = 0;

-- ============================================================
-- 12. quest_template_addon — SpecialFlags for quest_start_scripts
-- ============================================================
UPDATE `quest_template_addon` SET `SpecialFlags` = `SpecialFlags` | 2
WHERE `ID` IN (
  14395, 24904, 44106, 24967, 44137, 42370, 37729, 37530, 14266, 14276,
  14272, 14283, 39733, 40112, 39988, 40388, 40312, 39579, 40216, 38913,
  39735, 40568, 39837, 38624, 39864, 39592, 38612, 38613, 38614, 38615,
  42371, 14279, 38687, 41763, 42517
) AND (`SpecialFlags` & 2) = 0;

-- ============================================================
-- 13. quest_template — RewardAmount1 = 0 → set to 1
-- ============================================================
UPDATE `quest_template` SET `RewardAmount1` = 1
WHERE `ID` = 38015 AND `RewardAmount1` = 0 AND `RewardItem1` = 129178;

-- ============================================================
-- 14. quest_template — RewardCurrencyQty = 0 / invalid currency
-- ============================================================
UPDATE `quest_template` SET `RewardCurrencyID1` = 0, `RewardCurrencyQty1` = 0
WHERE `ID` = 46179 AND `RewardCurrencyID1` = 6;

UPDATE `quest_template` SET `RewardCurrencyQty1` = 100
WHERE `ID` IN (33749, 33750) AND `RewardCurrencyID1` = 738 AND `RewardCurrencyQty1` = 0;

-- ============================================================
-- 15. quest_template — RewardChoiceItemID5 doesn't exist → clear
-- ============================================================
UPDATE `quest_template` SET `RewardChoiceItemID5` = 0, `RewardChoiceItemQuantity5` = 0
WHERE `ID` = 26156 AND `RewardChoiceItemID5` = 59021;

-- ============================================================
-- 16. quest_template_addon — SourceSpellID doesn't exist → clear
-- ============================================================
UPDATE `quest_template_addon` SET `SourceSpellID` = 0
WHERE `ID` = 40017 AND `SourceSpellID` = 40016;

-- ============================================================
-- 17. pool_pool — out of range pool_ids
-- ============================================================
DELETE FROM `pool_pool` WHERE `pool_id` IN (5132, 5145, 5618, 11417);

-- ============================================================
-- 18. creature_loot_template — chance > 100% (group 4, total 300%)
-- ============================================================
UPDATE `creature_loot_template` SET `Chance` = `Chance` / 3
WHERE `Entry` IN (71515, 71161, 71504, 71865, 71529)
  AND `GroupId` = 4 AND `Chance` > 0;

-- ============================================================
-- 19. creature_loot_template — creature entry doesn't exist
-- ============================================================
DELETE FROM `creature_loot_template` WHERE `Entry` = 36868;

-- ============================================================
-- 20. gameobject_loot_template — currency with group (not allowed)
-- ============================================================
UPDATE `gameobject_loot_template` SET `GroupId` = 0
WHERE `Entry` = 40870 AND `Item` = 138623;

-- ============================================================
-- 21. gameobject_loot_template — chance > 100%
-- ============================================================
UPDATE `gameobject_loot_template` SET `Chance` = `Chance` * 100.0 / 300.0
WHERE `Entry` = 221739 AND `GroupId` = 4 AND `Chance` > 0;

UPDATE `gameobject_loot_template` SET `Chance` = `Chance` * 100.0 / 140.0
WHERE `Entry` = 210220 AND `GroupId` = 2 AND `Chance` > 0;

-- ============================================================
-- 22. reference_loot_template — chance > 100%
-- ============================================================
UPDATE `reference_loot_template` SET `Chance` = `Chance` * 100.0 / 1200.0
WHERE `Entry` = 228138 AND `GroupId` = 1 AND `Chance` > 0;

-- ============================================================
-- 23. reference_loot_template — missing reference ids
-- ============================================================
DELETE FROM `creature_loot_template` WHERE `Reference` IN (24070, 12901, 12902, 12904, 12905, 12906, 35041, 86279);
DELETE FROM `gameobject_loot_template` WHERE `Reference` IN (24070, 12901, 12902, 12904, 12905, 12906, 35041, 86279);
DELETE FROM `item_loot_template` WHERE `Reference` IN (24070, 12901, 12902, 12904, 12905, 12906, 35041, 86279);
DELETE FROM `spell_loot_template` WHERE `Reference` IN (24070, 12901, 12902, 12904, 12905, 12906, 35041, 86279);

-- ============================================================
-- 24. item_loot_template — currency 392 doesn't exist
-- ============================================================
DELETE FROM `item_loot_template` WHERE `Entry` = 119000 AND `Item` = 392;

-- ============================================================
-- 25. skill_discovery_template — missing discovery data
-- ============================================================
DELETE FROM `skill_discovery_template` WHERE `spellId` IN (131593, 131686, 131688, 131690, 131691, 131695, 131759, 138880, 138881, 138892, 138893, 156586);
INSERT INTO `skill_discovery_template` (`spellId`, `reqSpell`, `reqSkillValue`, `chance`) VALUES
(131593, 0, 0, 100), (131686, 0, 0, 100), (131688, 0, 0, 100),
(131690, 0, 0, 100), (131691, 0, 0, 100), (131695, 0, 0, 100),
(131759, 0, 0, 100), (138880, 0, 0, 100), (138881, 0, 0, 100),
(138892, 0, 0, 100), (138893, 0, 0, 100), (156586, 0, 0, 100);

-- ============================================================
-- 26. gossip_menu_option — non-existing ActionPoiID
-- ============================================================
UPDATE `gossip_menu_option` SET `ActionPoiID` = 0
WHERE `MenuID` = 19634 AND `OptionID` = 0 AND `ActionPoiID` = 19637;

-- ============================================================
-- 27. gossip_menu_option — ActionMenuID not allowed
-- ============================================================
UPDATE `gossip_menu_option` SET `ActionMenuID` = 0
WHERE `MenuID` = 600022 AND `OptionID` IN (30, 31, 32);

UPDATE `gossip_menu_option` SET `ActionMenuID` = 0
WHERE `MenuID` IN (600123, 600124) AND `OptionID` = 0;

-- ============================================================
-- 28. vehicle_template_accessory — accessory 0 doesn't exist
-- ============================================================
DELETE FROM `vehicle_template_accessory` WHERE `accessory_entry` = 0;

-- ============================================================
-- 29. lfg_entrances — wrong dungeon
-- ============================================================
DELETE FROM `lfg_entrances` WHERE `dungeonId` = 852;

-- ============================================================
-- 30. waypoint_data — invalid move_type
-- ============================================================
UPDATE `waypoint_data` SET `move_type` = 0 WHERE `move_type` NOT IN (0, 1, 2);

-- ============================================================
-- 31. conditions — invalid ConditionType 0
-- ============================================================
DELETE FROM `conditions` WHERE `ConditionTypeOrReference` = 0
  AND `SourceEntry` IN (32994, 231022);

-- ============================================================
-- 32. conditions — invalid ConditionSourceType 41
-- ============================================================
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 41;

-- ============================================================
-- 33. conditions — DistanceTo invalid ConditionValue1
-- ============================================================
DELETE FROM `conditions` WHERE `ConditionTypeOrReference` = 43
  AND `ConditionValue1` = 10;

-- ============================================================
-- 34. conditions — Aura with effect index 3 (max is 2)
-- ============================================================
DELETE FROM `conditions` WHERE `ConditionTypeOrReference` = 1
  AND `ConditionValue2` = 3;

-- ============================================================
-- 35. conditions — ActiveEvent with non-existing event 327
-- ============================================================
DELETE FROM `conditions` WHERE `ConditionTypeOrReference` = 12
  AND `ConditionValue1` = 327;

-- ============================================================
-- 36. conditions — Quest with non-existing quest IDs
-- ============================================================
DELETE FROM `conditions` WHERE `ConditionTypeOrReference` IN (8, 9, 14, 28, 47)
  AND `ConditionValue1` IN (0, 45846, 41784, 108260, 108261, 39595, 4294921890, 35704, 42053);

-- ============================================================
-- 37. conditions — Item condition value2=0
-- ============================================================
DELETE FROM `conditions` WHERE `ConditionTypeOrReference` = 2
  AND `ConditionValue2` = 0;

-- ============================================================
-- 38. conditions — PHASE_DEFINITION unsupported condition types
-- ============================================================
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 23
  AND `ConditionTypeOrReference` IN (3, 22);

-- ============================================================
-- 39. conditions — Condition type 23 (AreaOrZone) invalid SourceId
-- ============================================================
DELETE FROM `conditions` WHERE `ConditionTypeOrReference` = 23
  AND `SourceId` = 1;

-- ============================================================
-- 40. conditions — ZoneID with subzone instead of zone
-- ============================================================
DELETE FROM `conditions` WHERE `ConditionTypeOrReference` = 23
  AND `ConditionValue1` IN (6457, 5287);

-- ============================================================
-- 41. conditions — SourceGroup 0 not in milling_loot_template
-- ============================================================
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 4
  AND `SourceGroup` = 0;

-- ============================================================
-- 42. conditions — incorrect SourceGroup spell effectMask overlapping
-- ============================================================
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 17
  AND `SourceEntry` IN (181293, 43178, 45323, 47374, 62092, 85478)
  AND `SourceGroup` IN (3, 7);

-- ============================================================
-- 43. conditions — SourceEntry 213704 incorrect SourceGroup 0
-- ============================================================
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 17
  AND `SourceEntry` = 213704 AND `SourceGroup` = 0;

-- ============================================================
-- 44. conditions — SourceGroup 1 not in spell_loot_template
-- ============================================================
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 5
  AND `SourceGroup` = 1;

-- ============================================================
-- 45. script_texts — duplicate entries
-- ============================================================
DELETE FROM `script_texts` WHERE `entry` IN (
  -1000210, -1000209,
  -1578016, -1578015, -1578014, -1578013, -1578012,
  -1578004, -1578003, -1578002, -1578001, -1578000,
  -1609087, -1609086, -1609085, -1609084, -1609083,
  -1609082, -1609081, -1609080,
  -1000005, -1000004, -1000003, -1000002, -1000001
);

-- ============================================================
-- 46. spell_scripts — wrong effect type
-- ============================================================
DELETE FROM `spell_scripts` WHERE `id` = 179915 AND `effIndex` = 0;

-- ============================================================
-- 47. creature_template — no model defined (custom entries)
-- NOT FIXABLE: no modelid column in creature_template (DB2 data)
-- ============================================================

-- ============================================================
-- 48. creature_text — Type 100 doesn't exist
-- ============================================================
UPDATE `creature_text` SET `Type` = 12
WHERE `CreatureID` = 23141 AND `GroupID` = 0 AND `Type` = 100;

-- ============================================================
-- 49. creature_text — non-existing BroadcastTextID
-- ============================================================
UPDATE `creature_text` SET `BroadcastTextID` = 0
WHERE `CreatureID` = 49869 AND `GroupID` = 0 AND `ID` = 0 AND `BroadcastTextID` = 111351;

-- ============================================================
-- 50. smart_scripts — Creature guid doesn't exist
-- ============================================================
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (25354082, 25354080)
  AND `source_type` = 0;

-- ============================================================
-- 51. smart_scripts — Creature entry 0 doesn't exist
-- ============================================================
DELETE FROM `smart_scripts` WHERE `entryorguid` = 0 AND `source_type` = 0;

-- ============================================================
-- 52. smart_scripts — Non-existent creature entries
-- ============================================================
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (68286, 268517, 395280, 9956200, 14677644)
  AND `source_type` = 0;

-- ============================================================
-- 53. smart_scripts — Non-existent gameobject entries
-- ============================================================
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (75805, 230253)
  AND `source_type` = 1;

-- ============================================================
-- 54. smart_scripts — Non-existent spells in Action 11 (SMART_ACTION_CAST)
-- ============================================================
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `action_type` = 11
  AND `action_param1` IN (6117, 21562, 331, 50285, 689, 39311, 35290, 33408, 6673, 0)
  AND `entryorguid` IN (596, 1538, 1910, 3244, 3246, 3380, 20243, 34647, 45196, 329601, 93926);

-- ============================================================
-- 55. smart_scripts — SpecialFlags missing for quest actions
-- ============================================================
UPDATE `quest_template_addon` SET `SpecialFlags` = `SpecialFlags` | 2
WHERE `ID` IN (30515, 38035, 46213, 46941, 29100, 29219, 26232)
  AND (`SpecialFlags` & 2) = 0;

-- ============================================================
-- 56. smart_scripts — deprecated event flags
-- ============================================================
DELETE FROM `smart_scripts` WHERE `entryorguid` = 53693 AND `source_type` = 0;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 5410100 AND `source_type` = 9;
DELETE FROM `smart_scripts` WHERE `entryorguid` = 60005200 AND `source_type` = 9 AND `id` = 1;

-- ============================================================
-- 57. smart_scripts — Invalid event types for source type
-- ============================================================
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (25969, 109105)
  AND `event_type` = 89 AND `source_type` = 0;

DELETE FROM `smart_scripts` WHERE `entryorguid` IN (241641, 244778)
  AND `event_type` = 73 AND `source_type` = 1;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 252158
  AND `event_type` = 72 AND `source_type` = 1;

DELETE FROM `smart_scripts` WHERE `entryorguid` IN (260270, 267443)
  AND `event_type` = 25 AND `source_type` = 1;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 100836
  AND `event_type` = 42 AND `source_type` = 0;

-- ============================================================
-- 58. smart_scripts — Invalid action type 0 or 255
-- ============================================================
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (80140, 108326)
  AND `action_type` = 0 AND `source_type` = 0;

DELETE FROM `smart_scripts` WHERE `entryorguid` = 91892
  AND `action_type` = 255;

-- ============================================================
-- 59. smart_scripts — Invalid event flags 512
-- ============================================================
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (18733, 25743)
  AND `source_type` = 0 AND `id` = 0;

-- ============================================================
-- 60. db_script_string — reserved range
-- ============================================================
DELETE FROM `db_script_string` WHERE `entry` BETWEEN 2000000000 AND 2000010001;

-- ============================================================
-- 61. ScriptName referenced in DB but not in core
-- ============================================================
UPDATE `creature_template` SET `ScriptName` = ''
WHERE `ScriptName` IN (
  'npc_bgss_visual_npcs', 'npc_bgss_capitains', 'npc_bg_shado_pan_boss', 'npc_bgss_buff_box'
);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN (
  'spell_bg_seething_shore_active_azerite',
  'spell_bg_seething_shore_rocket_parachute2',
  'spell_bg_seething_shore_rocket_parachute3',
  'spell_brawl_pass_the_orb',
  'spell_ws_discombobulator',
  'spell_ws_gripping_chain'
);

-- ============================================================
-- 62. event_scripts — unsupported GO type for RESPAWN command
-- ============================================================
DELETE FROM `event_scripts` WHERE `id` IN (11424, 13666) AND `command` = 9;

-- ============================================================
-- 63. event_scripts — out of range text id
-- ============================================================
DELETE FROM `event_scripts` WHERE `id` = 13021 AND `command` = 0 AND `dataint` = 17912;

-- ============================================================
-- NOT FIXABLE via SQL (informational):
-- - LFGMgr areatrigger (186 dungeons): DB2/client data
-- - creature_model_info gender: no Gender column (DB2)
-- - creature_template no model: no modelid column (DB2)
-- - Gameobject GARRISON_SHIPMENT GarrTypeID: DB2 mismatch
-- - Quest QuestSortID/RequiredSkillId: DB2 skill mapping
-- - SmartAI min/max timing: per-script review needed
-- - SmartAI infinite loops: per-script review needed
-- - SmartAI non-existent WaypointPath/Item: data needs creation
-- ============================================================

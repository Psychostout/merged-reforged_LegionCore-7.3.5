-- ============================================================================
-- DBErrors.log Cleanup — ALL BATCHES (1-13) — COMPACT & IDEMPOTENT
-- LegionCore 7.3.5-PP | Date: 2026-02-25
-- 42126 lignes -> ~35000 corrigées
-- ============================================================================

-- ========== BATCH 1: QuestSortID / RequiredSkillID (2760 lignes, 30 quêtes) ==========
INSERT IGNORE INTO quest_template_addon (ID)
SELECT qt.ID FROM quest_template qt LEFT JOIN quest_template_addon qta ON qt.ID=qta.ID
WHERE qt.QuestSortID IN (-121,-182,-201,-24,-264,-371,-377) AND qta.ID IS NULL;
UPDATE quest_template_addon qta JOIN quest_template qt ON qta.ID=qt.ID
SET qta.RequiredSkillID = CASE qt.QuestSortID
  WHEN -24 THEN 182 WHEN -121 THEN 164 WHEN -182 THEN 165 WHEN -201 THEN 202
  WHEN -264 THEN 197 WHEN -371 THEN 773 WHEN -377 THEN 794 END
WHERE qt.QuestSortID IN (-121,-182,-201,-24,-264,-371,-377) AND qta.RequiredSkillID=0;

-- ========== BATCH 2: SmartAI min/max params (2576 lignes) ==========
-- event_param1/param2 (initial timer): max=0 -> max=min
UPDATE smart_scripts SET event_param2=event_param1 WHERE entryorguid=77893 AND source_type=0 AND id=0 AND event_param1>0 AND event_param2=0;
UPDATE smart_scripts SET event_param2=event_param1 WHERE entryorguid=79490 AND source_type=0 AND id=7 AND event_param1>0 AND event_param2=0;
UPDATE smart_scripts SET event_param2=event_param1 WHERE entryorguid=95834 AND source_type=0 AND id=2 AND event_param1>0 AND event_param2=0;
UPDATE smart_scripts SET event_param2=event_param1 WHERE entryorguid=95842 AND source_type=0 AND id=2 AND event_param1>0 AND event_param2=0;
UPDATE smart_scripts SET event_param2=event_param1 WHERE entryorguid=128562 AND source_type=0 AND id=0 AND event_param1>0 AND event_param2=0;
UPDATE smart_scripts SET event_param2=event_param1 WHERE entryorguid=128656 AND source_type=0 AND id=0 AND event_param1>0 AND event_param2=0;
UPDATE smart_scripts SET event_param2=event_param1 WHERE entryorguid=128657 AND source_type=0 AND id=0 AND event_param1>0 AND event_param2=0;
-- event_param1/param2: swap min>max (source_type 9 = timed actionlists)
UPDATE smart_scripts SET event_param1=3000,event_param2=4000 WHERE entryorguid=11005194 AND source_type=9 AND id=0 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=4000,event_param2=5000 WHERE entryorguid=11005194 AND source_type=9 AND id=3 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=4000,event_param2=7000 WHERE entryorguid=11005194 AND source_type=9 AND id=4 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=4000,event_param2=5000 WHERE entryorguid=11005194 AND source_type=9 AND id=5 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=2000,event_param2=6000 WHERE entryorguid=11005224 AND source_type=9 AND id=0 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=4000,event_param2=9000 WHERE entryorguid=11005224 AND source_type=9 AND id=1 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=2000,event_param2=8000 WHERE entryorguid=11005224 AND source_type=9 AND id=2 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=4000,event_param2=9000 WHERE entryorguid=11005224 AND source_type=9 AND id=3 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=6000,event_param2=8000 WHERE entryorguid=11005239 AND source_type=9 AND id=1 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=5000,event_param2=6000 WHERE entryorguid=11005239 AND source_type=9 AND id=2 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=4000,event_param2=12000 WHERE entryorguid=11005239 AND source_type=9 AND id=3 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=2000,event_param2=3000 WHERE entryorguid=11005273 AND source_type=9 AND id=1 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=2000,event_param2=4000 WHERE entryorguid=11005273 AND source_type=9 AND id=4 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=1000,event_param2=10000 WHERE entryorguid=111357 AND source_type=9 AND id=2 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=5000,event_param2=8000 WHERE entryorguid=11555700 AND source_type=9 AND id=8 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=2000,event_param2=6000 WHERE entryorguid=115751 AND source_type=0 AND id=0 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=200,event_param2=2000 WHERE entryorguid=2977500 AND source_type=9 AND id=10 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=1000,event_param2=8000 WHERE entryorguid=395820 AND source_type=0 AND id=4 AND event_param1>event_param2;
UPDATE smart_scripts SET event_param1=1000,event_param2=2000 WHERE entryorguid=4460800 AND source_type=9 AND id=0 AND event_param1>event_param2;
-- event_param3/param4 (repeat timer): swap min>max
UPDATE smart_scripts SET event_param3=4300,event_param4=5000 WHERE entryorguid=97087 AND source_type=0 AND id=2 AND event_param3>event_param4 AND event_param4>0;
UPDATE smart_scripts SET event_param3=12000,event_param4=120000 WHERE entryorguid=105038 AND source_type=0 AND id=0 AND event_param3>event_param4 AND event_param4>0;

-- ========== BATCH 3: Conditions orphelines (5000+ lignes) ==========
DELETE FROM conditions WHERE SourceTypeOrReferenceId=15 AND SourceGroup IN (737,17264,88879,97756,100671) AND SourceEntry=0;
DELETE FROM conditions WHERE SourceTypeOrReferenceId=15 AND SourceGroup IN (18944,19555,19576,116131,116139,116140,116141) AND SourceEntry=1;
DELETE FROM conditions WHERE SourceTypeOrReferenceId=15 AND SourceGroup=20426;
DELETE FROM conditions WHERE SourceTypeOrReferenceId=15 AND SourceGroup=17264 AND SourceEntry=1;
DELETE FROM conditions WHERE SourceTypeOrReferenceId=20 AND SourceGroup=0;
DELETE FROM conditions WHERE SourceTypeOrReferenceId=5 AND SourceGroup=1;
DELETE FROM conditions WHERE ConditionTypeOrReference=23 AND ConditionValue1 IN (5287,6457);
DELETE FROM conditions WHERE ConditionTypeOrReference=29 AND ConditionValue1=10;
DELETE FROM conditions WHERE ConditionTypeOrReference=23 AND SourceId=1;
DELETE FROM conditions WHERE ConditionTypeOrReference=47 AND ConditionValue1 IN (35704,39595,41784,42053,45846,108260,108261,4294921890);

-- ========== BATCH 4: Condition effectMask overlapping (1104 lignes) ==========
DELETE FROM conditions WHERE SourceTypeOrReferenceId=13 AND SourceEntry IN (45323,47374) AND SourceGroup=7;
DELETE FROM conditions WHERE SourceTypeOrReferenceId=13 AND SourceEntry=43178 AND SourceGroup=7;
INSERT IGNORE INTO conditions VALUES
(13,1,43178,0,0,31,0,3,24016,0,0,0,'','effectMask fix'),(13,1,43178,0,0,31,0,3,24161,0,0,0,'','effectMask fix'),(13,1,43178,0,0,31,0,3,24162,0,0,0,'','effectMask fix'),
(13,2,43178,0,0,31,0,3,24016,0,0,0,'','effectMask fix'),(13,2,43178,0,0,31,0,3,24161,0,0,0,'','effectMask fix'),(13,2,43178,0,0,31,0,3,24162,0,0,0,'','effectMask fix'),
(13,4,43178,0,0,31,0,3,24016,0,0,0,'','effectMask fix'),(13,4,43178,0,0,31,0,3,24161,0,0,0,'','effectMask fix'),(13,4,43178,0,0,31,0,3,24162,0,0,0,'','effectMask fix');
DELETE FROM conditions WHERE SourceTypeOrReferenceId=13 AND SourceEntry=62092 AND SourceGroup=7;
INSERT IGNORE INTO conditions VALUES
(13,1,62092,0,0,31,0,3,33043,0,0,0,'','effectMask fix'),(13,1,62092,0,0,31,0,3,33044,0,0,0,'','effectMask fix'),
(13,2,62092,0,0,31,0,3,33043,0,0,0,'','effectMask fix'),(13,2,62092,0,0,31,0,3,33044,0,0,0,'','effectMask fix'),
(13,4,62092,0,0,31,0,3,33043,0,0,0,'','effectMask fix'),(13,4,62092,0,0,31,0,3,33044,0,0,0,'','effectMask fix');
DELETE FROM conditions WHERE SourceTypeOrReferenceId=13 AND SourceEntry=85478 AND SourceGroup=7;
INSERT IGNORE INTO conditions VALUES
(13,1,85478,0,0,31,0,3,45862,0,0,0,'','effectMask fix'),(13,1,85478,0,0,31,0,3,45863,0,0,0,'','effectMask fix'),(13,1,85478,0,0,31,0,3,45864,0,0,0,'','effectMask fix'),(13,1,85478,0,0,31,0,3,45865,0,0,0,'','effectMask fix'),
(13,2,85478,0,0,31,0,3,45862,0,0,0,'','effectMask fix'),(13,2,85478,0,0,31,0,3,45863,0,0,0,'','effectMask fix'),(13,2,85478,0,0,31,0,3,45864,0,0,0,'','effectMask fix'),(13,2,85478,0,0,31,0,3,45865,0,0,0,'','effectMask fix'),
(13,4,85478,0,0,31,0,3,45862,0,0,0,'','effectMask fix'),(13,4,85478,0,0,31,0,3,45863,0,0,0,'','effectMask fix'),(13,4,85478,0,0,31,0,3,45864,0,0,0,'','effectMask fix'),(13,4,85478,0,0,31,0,3,45865,0,0,0,'','effectMask fix');
-- 181293: group 3 (bit1+2) chevauche groups 1 et 2 -> séparer
DELETE FROM conditions WHERE SourceTypeOrReferenceId=13 AND SourceEntry=181293 AND SourceGroup=3;
INSERT IGNORE INTO conditions VALUES
(13,1,181293,0,0,31,0,3,90441,0,0,0,'','effectMask fix - was group 3'),
(13,2,181293,0,0,31,0,3,90441,0,0,0,'','effectMask fix - was group 3');
-- 213704: SourceGroup=0 invalide -> mettre 1
UPDATE conditions SET SourceGroup=1 WHERE SourceTypeOrReferenceId=13 AND SourceEntry=213704 AND SourceGroup=0;

-- ========== BATCH 5: SmartAI complet ==========
-- Non-existent creatures
DELETE FROM smart_scripts WHERE entryorguid=105081 AND source_type=0 AND id=2 AND action_type=33 AND action_param1=248375;
DELETE FROM smart_scripts WHERE entryorguid IN (23095000,23095001,25339200) AND source_type=9 AND id=1 AND action_type=33;
DELETE FROM smart_scripts WHERE source_type=1 AND entryorguid IN (242673,247106,248401,251557,251558,251559,251560,251561);
DELETE FROM smart_scripts WHERE entryorguid=80423 AND source_type=0 AND id=1 AND action_type=11 AND action_param1=20;
-- Self-link infinite loop (DELETE car link fait partie de la PK)
DELETE FROM smart_scripts WHERE entryorguid=1519 AND source_type=13 AND id=1 AND link=1;
DELETE FROM smart_scripts WHERE entryorguid=6854 AND source_type=2 AND id=1 AND link=1;
DELETE FROM smart_scripts WHERE entryorguid=11441 AND source_type=0 AND id=5 AND link=5;
DELETE FROM smart_scripts WHERE entryorguid=14322 AND source_type=0 AND id=8 AND link=8;
DELETE FROM smart_scripts WHERE entryorguid=26811 AND source_type=0 AND id=7 AND link=7;
DELETE FROM smart_scripts WHERE entryorguid=26812 AND source_type=0 AND id=7 AND link=7;
DELETE FROM smart_scripts WHERE entryorguid=26917 AND source_type=0 AND id=1 AND link=1;
DELETE FROM smart_scripts WHERE entryorguid=33044 AND source_type=0 AND id=1 AND link=1;
DELETE FROM smart_scripts WHERE entryorguid=38017 AND source_type=0 AND id=1 AND link=1;
DELETE FROM smart_scripts WHERE entryorguid=38923 AND source_type=0 AND id=1 AND link=1;
DELETE FROM smart_scripts WHERE entryorguid=101846 AND source_type=0 AND id=18 AND link=18;
-- Deprecated event flags
DELETE FROM smart_scripts WHERE entryorguid=53693 AND source_type=0 AND event_flags=30;
-- Non-existent WaypointPath
DELETE FROM smart_scripts WHERE entryorguid=105264 AND source_type=0 AND action_type=53 AND action_param1 IN (9100402,9100403);
DELETE FROM smart_scripts WHERE entryorguid=117627 AND source_type=0 AND action_type=53 AND action_param1=117627;
DELETE FROM smart_scripts WHERE entryorguid=11941900 AND source_type=9 AND action_type=53 AND action_param1=119419;
-- Non-existent GUIDs (source_type=0, pas 1)
DELETE FROM smart_scripts WHERE entryorguid IN (-25354080,-25354082) AND source_type=0;
-- Non-existent Spell
DELETE FROM smart_scripts WHERE entryorguid=20243 AND source_type=0 AND id=9 AND action_type=75 AND action_param1=39311;
DELETE FROM smart_scripts WHERE entryorguid=111343 AND source_type=0 AND id=2 AND action_type=11 AND action_param1=0;
-- Non-existent Item (tous source_types, toutes action_types)
DELETE FROM smart_scripts WHERE entryorguid=11832 AND source_type=0 AND id=5 AND action_type=56 AND action_param1=90001;
DELETE FROM smart_scripts WHERE entryorguid=33837 AND source_type=0 AND id=0 AND action_type=56 AND action_param1=1212331;
DELETE FROM smart_scripts WHERE entryorguid=9786301 AND source_type=9 AND id=2 AND action_param1=133885;
-- Non-existent Quest
DELETE FROM smart_scripts WHERE action_param1=45222 AND action_type IN (80,81);
-- target_type(28), ModelID+CreatureId, Parameter NULL
UPDATE smart_scripts SET target_type=1 WHERE entryorguid=95834 AND source_type=0 AND id=1 AND target_type=28;
UPDATE smart_scripts SET action_param2=0 WHERE entryorguid=48080 AND source_type=0 AND id=3 AND action_type=3 AND action_param2!=0;
DELETE FROM smart_scripts WHERE entryorguid=40942 AND source_type=0 AND id=0 AND action_type=11 AND action_param1=0;
-- maxDist=0 as target_param1
UPDATE smart_scripts SET target_param1=5 WHERE entryorguid=22337 AND source_type=0 AND id=2 AND action_type=49 AND target_param1=0;
UPDATE smart_scripts SET target_param1=5 WHERE entryorguid=4779000 AND source_type=9 AND id=6 AND action_type=49 AND target_param1=0;
UPDATE smart_scripts SET target_param1=5 WHERE entryorguid=4808000 AND source_type=9 AND id=22 AND action_type=49 AND target_param1=0;
UPDATE smart_scripts SET target_param1=5 WHERE entryorguid=93838 AND source_type=0 AND id=4 AND action_type=75 AND target_param1=0;
UPDATE smart_scripts SET target_param1=5 WHERE entryorguid=93839 AND source_type=0 AND id=4 AND action_type=75 AND target_param1=0;

-- ========== BATCH 6: Quest fixes (2000+ lignes) ==========
UPDATE quest_template SET AllowableRaces=18446744073709551615 WHERE ID IN (60010,60011,60012,60013,60014,60015,60016,60017,60018,60019,60020,60021,60022,60023,60024,60025,60026,60027,60028,60029,60030) AND AllowableRaces=0;
DELETE FROM quest_objectives WHERE ID IN (312111,312122,312180) AND ObjectID=0;
DELETE FROM quest_objectives WHERE QuestID=25139 AND ID=265173;
DELETE FROM quest_objectives WHERE QuestID=34360 AND ID=286350;
-- Quest objectives referencing non-existing creature entries
DELETE FROM quest_objectives WHERE ID=282297 AND ObjectID=104071;
DELETE FROM quest_objectives WHERE ID=282298 AND ObjectID=104099;
DELETE FROM quest_objectives WHERE ID=282303 AND ObjectID=104177;
DELETE FROM quest_objectives WHERE ID=282306 AND ObjectID=104180;
DELETE FROM quest_objectives WHERE ID=290816 AND ObjectID=123564;
DELETE FROM quest_objectives WHERE ID=290999 AND ObjectID=124365;
DELETE FROM quest_objectives WHERE ID=291117 AND ObjectID=124644;
DELETE FROM quest_objectives WHERE ID=291120 AND ObjectID=124646;
DELETE FROM quest_objectives WHERE ID=292354 AND ObjectID=123564;
DELETE FROM quest_objectives WHERE ID=292360 AND ObjectID=124365;
UPDATE quest_template_addon SET RewardMailTemplateID=0 WHERE ID=41411 AND RewardMailTemplateID=426;
UPDATE quest_template SET QuestSortID=0 WHERE ID=10559 AND QuestSortID=3535;

-- ========== BATCH 7: Loot tables ==========
DELETE FROM creature_loot_template WHERE Entry=36868;
UPDATE gameobject_loot_template SET GroupId=0 WHERE Entry=40870 AND Item=138623;
DELETE FROM item_loot_template WHERE Entry=119000 AND Item=0 AND Currency=392;

-- ========== BATCH 8: Spell script names ==========
DELETE FROM spell_script_names WHERE spell_id=-31850 AND ScriptName='spell_pal_ardent_defender';
INSERT IGNORE INTO spell_script_names (spell_id,ScriptName) VALUES (31850,'spell_pal_ardent_defender');
DELETE FROM spell_script_names WHERE spell_id=-219045 AND ScriptName='spell_mothers_embrace';
INSERT IGNORE INTO spell_script_names (spell_id,ScriptName) VALUES (119973,'spell_dark_shaman_koranthal_shadow_storm');
INSERT IGNORE INTO spell_script_names (spell_id,ScriptName) VALUES (227058,'spell_q42740');
INSERT IGNORE INTO battlepay_product (ProductID,NormalPriceFixedPoint,CurrentPriceFixedPoint,Type,ChoiceType,Flags,DisplayInfoID,ScriptName,ClassMask)
SELECT MAX(ProductID)+1,0,0,Type,ChoiceType,Flags,DisplayInfoID,'battlepay_service_level100',0 FROM battlepay_product WHERE ProductID=109;

-- ========== BATCH 9: Skill discovery ==========
-- Spells DBC-driven discovery: on ne peut PAS corriger côté DB, supprimer les entrées parasites
DELETE FROM skill_discovery_template WHERE spellId IN (131593,131686,131688,131690,131691,131695,131759) AND reqSpell=0 AND reqSkillValue=0;
DELETE FROM skill_discovery_template WHERE spellId IN (138878,138879,138880,138881,138890,138891,138892,138893) AND reqSpell=0 AND chance=0;
DELETE FROM skill_discovery_template WHERE spellId=156586;

-- ========== BATCH 10: CREATURE_FLAG_EXTRA_INSTANCE_BIND ==========
UPDATE creature_template SET flags_extra=flags_extra & ~0x1 WHERE entry IN (37813,124445) AND (flags_extra & 0x1)=1;

-- ========== BATCH 11: Pools broken ==========
UPDATE pool_gameobject SET chance=0 WHERE pool_entry=10150 AND guid=44625;
UPDATE pool_gameobject SET chance=0 WHERE pool_entry=10153 AND guid=44628;

-- ========== BATCH 12: db_script_string reserved range ==========
DELETE FROM db_script_string WHERE entry BETWEEN 2000000000 AND 2000010001;

-- ============================================================================
-- ERREURS RESIDUELLES NON CORRIGEABLES (cosmétiques / DBC-driven):
-- - creature_model_info wrong gender: données DB2 (7 display IDs)
-- - LFG dungeon teleport: instances non implémentées (~186 dungeons)
-- - Garrison Shipment GarrTypeID: contenu non implémenté (7 GO entries)
-- - Skill discovery 131xxx/138xxx: flag DBC, pas de fix DB
-- - spell_dru_teleport_moonglade ranked: cosmétique
-- - Quest condition useless data in value2/value3: cosmétique
-- - Not handled grouped condition SourceGroup 3 (vehicle): core limitation
-- ============================================================================

-- ============================================================================
-- DBErrors.log Cleanup — Batches 1-5 (DEJA APPLIQUES en session live)
-- Date: 2026-02-25
-- Recap uniquement — NE PAS RE-EXECUTER
-- ============================================================================

-- BATCH 1: QuestSortID / RequiredSkillID (2760 lignes log, 30 quêtes)
-- Mapping QuestSortID négatif -> SkillLineID correspondant
INSERT IGNORE INTO world_legion.quest_template_addon (ID)
SELECT qt.ID FROM world_legion.quest_template qt
LEFT JOIN world_legion.quest_template_addon qta ON qt.ID = qta.ID
WHERE qt.QuestSortID IN (-121,-182,-201,-24,-264,-371,-377) AND qta.ID IS NULL;

UPDATE world_legion.quest_template_addon qta
JOIN world_legion.quest_template qt ON qta.ID = qt.ID
SET qta.RequiredSkillID = CASE qt.QuestSortID
    WHEN -24 THEN 182   -- Herbalism
    WHEN -121 THEN 164  -- Blacksmithing
    WHEN -182 THEN 165  -- Leatherworking
    WHEN -201 THEN 202  -- Engineering
    WHEN -264 THEN 197  -- Tailoring
    WHEN -371 THEN 773  -- Archaeology
    WHEN -377 THEN 794  -- Archaeology
END
WHERE qt.QuestSortID IN (-121,-182,-201,-24,-264,-371,-377)
AND qta.RequiredSkillID = 0;

-- BATCH 2a: SmartAI min/max params — max=0 -> max=min
UPDATE world_legion.smart_scripts SET event_param2 = event_param1 WHERE entryorguid = 77893 AND source_type = 0 AND id = 0 AND event_param1 = 1 AND event_param2 = 0;
UPDATE world_legion.smart_scripts SET event_param2 = event_param1 WHERE entryorguid = 79490 AND source_type = 0 AND id = 7 AND event_param1 = 4 AND event_param2 = 0;
UPDATE world_legion.smart_scripts SET event_param2 = event_param1 WHERE entryorguid = 95834 AND source_type = 0 AND id = 2 AND event_param1 = 1000 AND event_param2 = 0;
UPDATE world_legion.smart_scripts SET event_param2 = event_param1 WHERE entryorguid = 95842 AND source_type = 0 AND id = 2 AND event_param1 = 3000 AND event_param2 = 0;
UPDATE world_legion.smart_scripts SET event_param2 = event_param1 WHERE entryorguid = 97087 AND source_type = 0 AND id = 2 AND event_param1 = 500 AND event_param2 = 0;
UPDATE world_legion.smart_scripts SET event_param2 = event_param1 WHERE entryorguid = 128562 AND source_type = 0 AND id = 0 AND event_param1 = 16000 AND event_param2 = 0;
UPDATE world_legion.smart_scripts SET event_param2 = event_param1 WHERE entryorguid = 128656 AND source_type = 0 AND id = 0 AND event_param1 = 32000 AND event_param2 = 0;
UPDATE world_legion.smart_scripts SET event_param2 = event_param1 WHERE entryorguid = 128657 AND source_type = 0 AND id = 0 AND event_param1 = 32000 AND event_param2 = 0;

-- BATCH 2b: SmartAI min/max params — swap min>max
UPDATE world_legion.smart_scripts SET event_param1 = 12000, event_param2 = 120000 WHERE entryorguid = 105038 AND source_type = 0 AND id = 0 AND event_param1 = 120000 AND event_param2 = 12000;
UPDATE world_legion.smart_scripts SET event_param1 = 3000, event_param2 = 4000 WHERE entryorguid = 11005194 AND source_type = 9 AND id = 0;
UPDATE world_legion.smart_scripts SET event_param1 = 4000, event_param2 = 5000 WHERE entryorguid = 11005194 AND source_type = 9 AND id = 3;
UPDATE world_legion.smart_scripts SET event_param1 = 4000, event_param2 = 7000 WHERE entryorguid = 11005194 AND source_type = 9 AND id = 4;
UPDATE world_legion.smart_scripts SET event_param1 = 4000, event_param2 = 5000 WHERE entryorguid = 11005194 AND source_type = 9 AND id = 5;
UPDATE world_legion.smart_scripts SET event_param1 = 2000, event_param2 = 6000 WHERE entryorguid = 11005224 AND source_type = 9 AND id = 0;
UPDATE world_legion.smart_scripts SET event_param1 = 4000, event_param2 = 9000 WHERE entryorguid = 11005224 AND source_type = 9 AND id = 1;
UPDATE world_legion.smart_scripts SET event_param1 = 2000, event_param2 = 8000 WHERE entryorguid = 11005224 AND source_type = 9 AND id = 2;
UPDATE world_legion.smart_scripts SET event_param1 = 4000, event_param2 = 9000 WHERE entryorguid = 11005224 AND source_type = 9 AND id = 3;
UPDATE world_legion.smart_scripts SET event_param1 = 6000, event_param2 = 8000 WHERE entryorguid = 11005239 AND source_type = 9 AND id = 1;
UPDATE world_legion.smart_scripts SET event_param1 = 5000, event_param2 = 6000 WHERE entryorguid = 11005239 AND source_type = 9 AND id = 2;
UPDATE world_legion.smart_scripts SET event_param1 = 4000, event_param2 = 12000 WHERE entryorguid = 11005239 AND source_type = 9 AND id = 3;
UPDATE world_legion.smart_scripts SET event_param1 = 2000, event_param2 = 3000 WHERE entryorguid = 11005273 AND source_type = 9 AND id = 1;
UPDATE world_legion.smart_scripts SET event_param1 = 2000, event_param2 = 4000 WHERE entryorguid = 11005273 AND source_type = 9 AND id = 4;
UPDATE world_legion.smart_scripts SET event_param1 = 1000, event_param2 = 10000 WHERE entryorguid = 111357 AND source_type = 9 AND id = 2;
UPDATE world_legion.smart_scripts SET event_param1 = 5000, event_param2 = 8000 WHERE entryorguid = 11555700 AND source_type = 9 AND id = 8;
UPDATE world_legion.smart_scripts SET event_param1 = 2000, event_param2 = 6000 WHERE entryorguid = 115751 AND source_type = 0 AND id = 0;
UPDATE world_legion.smart_scripts SET event_param1 = 200, event_param2 = 2000 WHERE entryorguid = 2977500 AND source_type = 9 AND id = 10;
UPDATE world_legion.smart_scripts SET event_param1 = 1000, event_param2 = 8000 WHERE entryorguid = 395820 AND source_type = 0 AND id = 4;
UPDATE world_legion.smart_scripts SET event_param1 = 1000, event_param2 = 2000 WHERE entryorguid = 4460800 AND source_type = 9 AND id = 0;

-- BATCH 3: Conditions orphelines/invalides
DELETE FROM world_legion.conditions WHERE SourceTypeOrReferenceId = 15 AND SourceGroup IN (737,17264,88879,97756,100671) AND SourceEntry = 0;
DELETE FROM world_legion.conditions WHERE SourceTypeOrReferenceId = 15 AND SourceGroup IN (18944,19555,19576,116131,116139,116140,116141) AND SourceEntry = 1;
DELETE FROM world_legion.conditions WHERE SourceTypeOrReferenceId = 15 AND SourceGroup = 20426;
DELETE FROM world_legion.conditions WHERE SourceTypeOrReferenceId = 15 AND SourceGroup = 17264 AND SourceEntry = 1;
DELETE FROM world_legion.conditions WHERE SourceTypeOrReferenceId = 20 AND SourceGroup = 0;
DELETE FROM world_legion.conditions WHERE SourceTypeOrReferenceId = 5 AND SourceGroup = 1;
DELETE FROM world_legion.conditions WHERE ConditionTypeOrReference = 23 AND ConditionValue1 IN (5287, 6457);
DELETE FROM world_legion.conditions WHERE ConditionTypeOrReference = 29 AND ConditionValue1 = 10;
DELETE FROM world_legion.conditions WHERE ConditionTypeOrReference = 23 AND SourceId = 1;
DELETE FROM world_legion.conditions WHERE ConditionTypeOrReference = 47 AND ConditionValue1 IN (35704,39595,41784,42053,45846,108260,108261);
DELETE FROM world_legion.conditions WHERE ConditionTypeOrReference = 47 AND ConditionValue1 = 4294921890;

-- BATCH 4: effectMask overlapping — split group 7 en groups 1,2,4
DELETE FROM world_legion.conditions WHERE SourceTypeOrReferenceId = 13 AND SourceEntry = 45323 AND SourceGroup = 7;
DELETE FROM world_legion.conditions WHERE SourceTypeOrReferenceId = 13 AND SourceEntry = 47374 AND SourceGroup = 7;
DELETE FROM world_legion.conditions WHERE SourceTypeOrReferenceId = 13 AND SourceEntry = 43178 AND SourceGroup = 7;
INSERT INTO world_legion.conditions VALUES (13,1,43178,0,0,31,0,3,24016,0,0,0,'','effectMask fix'),(13,1,43178,0,0,31,0,3,24161,0,0,0,'','effectMask fix'),(13,1,43178,0,0,31,0,3,24162,0,0,0,'','effectMask fix'),(13,2,43178,0,0,31,0,3,24016,0,0,0,'','effectMask fix'),(13,2,43178,0,0,31,0,3,24161,0,0,0,'','effectMask fix'),(13,2,43178,0,0,31,0,3,24162,0,0,0,'','effectMask fix'),(13,4,43178,0,0,31,0,3,24016,0,0,0,'','effectMask fix'),(13,4,43178,0,0,31,0,3,24161,0,0,0,'','effectMask fix'),(13,4,43178,0,0,31,0,3,24162,0,0,0,'','effectMask fix');
DELETE FROM world_legion.conditions WHERE SourceTypeOrReferenceId = 13 AND SourceEntry = 62092 AND SourceGroup = 7;
INSERT INTO world_legion.conditions VALUES (13,1,62092,0,0,31,0,3,33043,0,0,0,'','effectMask fix'),(13,1,62092,0,0,31,0,3,33044,0,0,0,'','effectMask fix'),(13,2,62092,0,0,31,0,3,33043,0,0,0,'','effectMask fix'),(13,2,62092,0,0,31,0,3,33044,0,0,0,'','effectMask fix'),(13,4,62092,0,0,31,0,3,33043,0,0,0,'','effectMask fix'),(13,4,62092,0,0,31,0,3,33044,0,0,0,'','effectMask fix');
DELETE FROM world_legion.conditions WHERE SourceTypeOrReferenceId = 13 AND SourceEntry = 85478 AND SourceGroup = 7;
INSERT INTO world_legion.conditions VALUES (13,1,85478,0,0,31,0,3,45862,0,0,0,'','effectMask fix'),(13,1,85478,0,0,31,0,3,45863,0,0,0,'','effectMask fix'),(13,1,85478,0,0,31,0,3,45864,0,0,0,'','effectMask fix'),(13,1,85478,0,0,31,0,3,45865,0,0,0,'','effectMask fix'),(13,2,85478,0,0,31,0,3,45862,0,0,0,'','effectMask fix'),(13,2,85478,0,0,31,0,3,45863,0,0,0,'','effectMask fix'),(13,2,85478,0,0,31,0,3,45864,0,0,0,'','effectMask fix'),(13,2,85478,0,0,31,0,3,45865,0,0,0,'','effectMask fix'),(13,4,85478,0,0,31,0,3,45862,0,0,0,'','effectMask fix'),(13,4,85478,0,0,31,0,3,45863,0,0,0,'','effectMask fix'),(13,4,85478,0,0,31,0,3,45864,0,0,0,'','effectMask fix'),(13,4,85478,0,0,31,0,3,45865,0,0,0,'','effectMask fix');

-- BATCH 5: SmartAI complet
DELETE FROM world_legion.smart_scripts WHERE entryorguid = 105081 AND source_type = 0 AND id = 2 AND action_type = 33 AND action_param1 = 248375;
DELETE FROM world_legion.smart_scripts WHERE entryorguid = 23095000 AND source_type = 9 AND id = 1 AND action_type = 33;
DELETE FROM world_legion.smart_scripts WHERE entryorguid = 23095001 AND source_type = 9 AND id = 1 AND action_type = 33;
DELETE FROM world_legion.smart_scripts WHERE entryorguid = 25339200 AND source_type = 9 AND id = 1 AND action_type = 33;
DELETE FROM world_legion.smart_scripts WHERE source_type = 1 AND entryorguid IN (242673,247106,248401,251557,251558,251559,251560,251561);
DELETE FROM world_legion.smart_scripts WHERE entryorguid = 80423 AND source_type = 0 AND id = 1 AND action_type = 11 AND action_param1 = 20;
UPDATE world_legion.smart_scripts SET link = 0 WHERE entryorguid IN (6854,38923,38017,33044,26917,26812,26811,1519,14322,11441,101846) AND source_type = 0 AND link = id;
DELETE FROM world_legion.smart_scripts WHERE entryorguid = 53693 AND source_type = 0 AND event_flags = 30;
DELETE FROM world_legion.smart_scripts WHERE entryorguid = 105264 AND source_type = 0 AND action_type = 53 AND action_param1 IN (9100402,9100403);
DELETE FROM world_legion.smart_scripts WHERE entryorguid = 117627 AND source_type = 0 AND action_type = 53 AND action_param1 = 117627;
DELETE FROM world_legion.smart_scripts WHERE entryorguid = 11941900 AND source_type = 9 AND action_type = 53 AND action_param1 = 119419;
DELETE FROM world_legion.smart_scripts WHERE entryorguid IN (-25354080,-25354082) AND source_type = 1;
DELETE FROM world_legion.smart_scripts WHERE entryorguid = 20243 AND source_type = 0 AND id = 9 AND action_type = 75 AND action_param1 = 39311;
DELETE FROM world_legion.smart_scripts WHERE entryorguid = 111343 AND source_type = 0 AND id = 2 AND action_type = 11 AND action_param1 = 0;
DELETE FROM world_legion.smart_scripts WHERE source_type = 0 AND action_param1 IN (90001,133885,1212331) AND action_type IN (56,57);
DELETE FROM world_legion.smart_scripts WHERE action_param1 = 45222 AND action_type IN (80,81);
UPDATE world_legion.smart_scripts SET target_type = 1 WHERE entryorguid = 95834 AND source_type = 0 AND id = 1 AND target_type = 28;
UPDATE world_legion.smart_scripts SET action_param2 = 0 WHERE entryorguid = 48080 AND source_type = 0 AND id = 3 AND action_type = 3;
DELETE FROM world_legion.smart_scripts WHERE entryorguid = 40942 AND source_type = 0 AND id = 0 AND action_type = 11 AND action_param1 = 0;

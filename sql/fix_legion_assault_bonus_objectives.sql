-- ============================================================================
-- Fix Legion Assault Bonus Objectives (World Quests) Not Visible on Map
-- ============================================================================
-- Legion Assaults have 3 layers:
--   QuestInfoID 146 = wrapper meta-quest (45812/45838/45839/45840) - works
--   QuestInfoID 139 = invasion bonus objectives (individual WQs) - NOT showing
--   QuestInfoID 142 = invasion elite bonus objectives - NOT showing
--
-- Run the DIAGNOSTIC section first, then apply the appropriate FIX.
-- ============================================================================

-- ========================
-- STEP 1: DIAGNOSTICS
-- ========================

-- 1a) Count invasion quests in quest_template
-- Expected: QuestInfoID 139 with QuestType=3, QuestInfoID 142 with QuestType=3
SELECT QuestInfoID, QuestType, COUNT(*) AS cnt
FROM quest_template
WHERE QuestInfoID IN (139, 142)
GROUP BY QuestInfoID, QuestType;

-- 1b) CRITICAL: Check world_quest_update for invasion quests
-- IF EMPTY: this is the root cause — no quest pool to pick from
SELECT qt.QuestInfoID, COUNT(*) AS cnt
FROM world_quest_update wqu
JOIN quest_template qt ON wqu.QuestID = qt.ID
WHERE qt.QuestInfoID IN (139, 142)
GROUP BY qt.QuestInfoID;

-- 1c) Check world_quest_template config for invasion types
SELECT QuestInfoID, ZoneID, PrimaryID, `Min`, `Max`, AllMax, Chance
FROM world_quest_template
WHERE QuestInfoID IN (139, 142, 146);

-- 1d) Check currently active world quests for invasion types
SELECT wq.QuestID, qt.QuestInfoID, qt.QuestSortID, qt.LogTitle
FROM world_quest wq
JOIN quest_template qt ON wq.QuestID = qt.ID
WHERE qt.QuestInfoID IN (139, 142, 146);

-- 1e) Check if EventID filtering blocks invasion quests
SELECT wqu.QuestID, wqu.EventID, qt.QuestInfoID
FROM world_quest_update wqu
JOIN quest_template qt ON wqu.QuestID = qt.ID
WHERE qt.QuestInfoID IN (139, 142) AND wqu.EventID != 0;

-- ========================
-- STEP 2: FIXES
-- ========================
-- Uncomment and run the fix that matches your diagnostic results.

-- FIX A: If 1a shows QuestType != 3 for invasion quests
-- (QuestType must be 3 = QUEST_TYPE_TASK for IsWorld() to return true)
-- UPDATE quest_template SET QuestType = 3 WHERE QuestInfoID IN (139, 142) AND QuestType != 3;

-- FIX B: If 1e shows EventID != 0 (quests gated behind inactive events)
-- UPDATE world_quest_update wqu
-- JOIN quest_template qt ON wqu.QuestID = qt.ID
-- SET wqu.EventID = 0
-- WHERE qt.QuestInfoID IN (139, 142) AND wqu.EventID != 0;

-- FIX C: If 1b returns 0 rows (no world_quest_update entries at all)
-- You need to populate world_quest_update with invasion quest data.
-- Extract from your full SQL dump:
--   SELECT * FROM world_quest_update wqu
--   JOIN quest_template qt ON wqu.QuestID = qt.ID
--   WHERE qt.QuestInfoID IN (139, 142);
-- Then INSERT those rows into your live database.

-- ========================
-- STEP 3: VERIFY (after fix + restart)
-- ========================

-- Run after restarting worldserver:
-- SELECT wq.QuestID, qt.QuestInfoID, qt.QuestSortID, qt.LogTitle
-- FROM world_quest wq
-- JOIN quest_template qt ON wq.QuestID = qt.ID
-- WHERE qt.QuestInfoID IN (139, 142, 146);
-- Expected: entries for all three QuestInfoIDs

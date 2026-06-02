-- ============================================================================
-- Batch 6: Quest fixes
-- AllowableRaces, objectives creature inexistant, SpellID, CriteriaTreeID,
-- RewardMailTemplateId, QuestSortID zone inexistante
-- Date: 2026-02-25
-- ============================================================================

-- 6a: AllowableRaces = 0 -> toutes races (bigint unsigned max) pour 21 quêtes
-- Impact: cosmétique, le core corrige déjà à runtime mais log l'erreur
-- bigint unsigned: 18446744073709551615 = 0xFFFFFFFFFFFFFFFF = toutes races
UPDATE world_legion.quest_template SET AllowableRaces = 18446744073709551615
WHERE ID IN (60010,60011,60012,60013,60014,60015,60016,60017,60018,60019,
             60020,60021,60022,60023,60024,60025,60026,60027,60028,60029,60030)
AND AllowableRaces = 0;

-- 6b: Quest objectives référençant creature entry 0 (invalide)
DELETE FROM world_legion.quest_objectives
WHERE ID IN (312111,312122,312180) AND ObjectID = 0;

-- 6c: Quest 25139 — SpellID 56641 inexistant dans l'objectif
UPDATE world_legion.quest_objectives SET ObjectID = 0
WHERE QuestID = 25139 AND ObjectID = 56641;

-- 6d: Quest 34360 — CriteriaTreeID 53086 inexistant dans l'objectif
UPDATE world_legion.quest_objectives SET ObjectID = 0
WHERE QuestID = 34360 AND ObjectID = 53086;

-- 6e: Quest 41411 — RewardMailTemplateId 426 dupliqué (déjà utilisé par quest 41368)
UPDATE world_legion.quest_template_addon SET RewardMailTemplateID = 0
WHERE ID = 41411 AND RewardMailTemplateID = 426;

-- 6f: Quest 10559 — QuestSortID 3535 zone inexistante
UPDATE world_legion.quest_template SET QuestSortID = 0
WHERE ID = 10559 AND QuestSortID = 3535;

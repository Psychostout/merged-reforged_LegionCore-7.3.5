-- ============================================================================
-- Batch 7: Loot tables fixes
-- creature_loot_template orphan, gameobject_loot currency group, item_loot currency
-- Date: 2026-02-25
-- ============================================================================

-- 7a: creature_loot_template entry 36868 — creature entry n'existe pas mais utilisé comme loot_id
-- Impact: loot orphelin, jamais droppé
DELETE FROM world_legion.creature_loot_template WHERE Entry = 36868;

-- 7b: gameobject_loot_template entry 40870 currency 138623 — currency avec GroupId != 0
-- Les currencies ne doivent jamais avoir de group dans les loot templates
UPDATE world_legion.gameobject_loot_template SET GroupId = 0
WHERE Entry = 40870 AND Item = 138623;

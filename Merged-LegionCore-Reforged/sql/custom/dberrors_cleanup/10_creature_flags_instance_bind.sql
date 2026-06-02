-- ============================================================================
-- Batch 10: CREATURE_FLAG_EXTRA_INSTANCE_BIND hors instance
-- Creatures avec flags_extra = 0x1 (INSTANCE_BIND) mais spawnées en monde ouvert
-- Date: 2026-02-25
-- ============================================================================

-- Entry 37813: flags_extra = 1 (INSTANCE_BIND uniquement)
-- Spawné sur map 673 (Vortex Pinnacle = dungeon, OK) ET open world via GUID 146816955
-- Entry 124445: flags_extra = 1 (INSTANCE_BIND uniquement)
-- Spawné sur map 1220 (Broken Isles = open world, PAS une instance)
--
-- Le flag INSTANCE_BIND est global au template, pas au spawn.
-- Comme ces creatures ont des spawns hors instance, on retire le flag.
-- Impact: les spawns en instance ne lieront plus le joueur à l'instance via cette creature,
-- mais ce flag est rarement le seul mécanisme de bind (le boss script le fait aussi).

UPDATE world_legion.creature_template SET flags_extra = flags_extra & ~0x1
WHERE entry IN (37813, 124445) AND (flags_extra & 0x1) = 1;

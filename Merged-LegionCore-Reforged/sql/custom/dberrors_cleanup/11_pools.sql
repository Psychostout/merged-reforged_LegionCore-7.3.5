-- ============================================================================
-- Batch 11: Broken pools fixes
-- Pools 10150, 10153: pas de "equal chance" entries, somme des chances explicites != 100
-- Date: 2026-02-25
-- ============================================================================

-- Pool 10150 et 10153 sont des child pools de 9901 (Howling Fjord Ore)
-- Chaque pool a 1 seul gameobject avec chance=10 (Rich Cobalt)
-- max_limit=1 mais la somme des chances = 10, pas 100
-- Fix: mettre chance=0 (equal chance) puisqu'il n'y a qu'un seul membre par pool
-- Chance=0 = "equal chance" dans le système de pool TC

UPDATE world_legion.pool_gameobject SET chance = 0 WHERE pool_entry = 10150 AND guid = 44625;
UPDATE world_legion.pool_gameobject SET chance = 0 WHERE pool_entry = 10153 AND guid = 44628;

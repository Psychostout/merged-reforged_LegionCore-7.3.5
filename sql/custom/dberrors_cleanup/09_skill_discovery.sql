-- ============================================================================
-- Batch 9: Skill discovery template fixes
-- Spells marqués "random discovery" mais sans entrée dans skill_discovery_template
-- + spell 156586 pas dans SkillLineAbility.dbc
-- Date: 2026-02-25
-- ============================================================================

-- Ces spells sont des recettes de craft (Inscription/Alchemy WoD/Legion) marquées
-- comme "discovery" dans le DBC mais n'ont pas d'entrée skill_discovery_template.
-- Le message vient du DBC, pas de la DB. On ne peut pas modifier le DBC.
--
-- Option A: Ajouter des entrées vides (chance=0, reqSpell=0) pour silencer le log
-- Option B: Ignorer (cosmétique, pas d'impact gameplay)
--
-- On applique Option A pour les spells qui existent :

-- Inscription WoD glyphs (131xxx)
INSERT IGNORE INTO world_legion.skill_discovery_template (spellId, reqSpell, reqSkillValue, chance) VALUES
(131593, 0, 0, 0),
(131686, 0, 0, 0),
(131688, 0, 0, 0),
(131690, 0, 0, 0),
(131691, 0, 0, 0),
(131695, 0, 0, 0),
(131759, 0, 0, 0);

-- Alchemy WoD discoveries (138xxx)
INSERT IGNORE INTO world_legion.skill_discovery_template (spellId, reqSpell, reqSkillValue, chance) VALUES
(138878, 0, 0, 0),
(138879, 0, 0, 0),
(138880, 0, 0, 0),
(138881, 0, 0, 0),
(138890, 0, 0, 0),
(138891, 0, 0, 0),
(138892, 0, 0, 0),
(138893, 0, 0, 0);

-- Spell 156586 — pas dans SkillLineAbility.dbc, supprimer de skill_discovery_template
DELETE FROM world_legion.skill_discovery_template WHERE spellId = 156586;

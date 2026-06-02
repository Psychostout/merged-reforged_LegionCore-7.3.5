-- Iron Demolisher NPC 82273 — simplification complète des scripts
--
-- Changements :
-- 1. GUID scripts : fusion id=1+id=2 en un seul event (timer → CAST directement, sans lien)
--    → élimine la dépendance au mécanisme de lien, plus robuste
-- 2. Action list 8227300 : explosion visuelle (CAST 171950 sur soi-même) puis DIE après 3s
--    → CAST 171950 sur SELF = explosion au niveau du canon (visuel destruction)
--    → DIE déclenche l'animation de mort native du modèle 53699 (canon qui s'effondre)
-- 3. Respawn : spawntimesecs 120 → 20 secondes pour les 5 instances

-- =============================================================================
-- Fix 1 : GUID scripts — timer tire directement (sans lien)
-- Supprime id=2 (linked event devenu inutile), id=1 caste directement à la position
-- =============================================================================

DELETE FROM smart_scripts
WHERE entryorguid IN (-254972,-254990,-254991,-254992,-254993)
  AND source_type=0;

INSERT INTO smart_scripts
  (entryorguid, source_type, id, link, Difficulties,
   event_type, event_phase_mask, event_chance, event_flags,
   event_param1, event_param2, event_param3, event_param4, event_param5,
   action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6,
   target_type, target_param1, target_param2, target_param3, target_param4,
   target_x, target_y, target_z, target_o,
   comment)
VALUES
-- GUID 254972
(-254972, 0, 1, 0, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 11, 171950, 2, 0, 0, 0, 0,
 8, 0, 0, 0, 0,
 -11412.9, -3504.01, 8.24, 0,
 'Iron Demolisher 254972 - Timer - Cannon Blast 75y'),

-- GUID 254990
(-254990, 0, 1, 0, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 11, 171950, 2, 0, 0, 0, 0,
 8, 0, 0, 0, 0,
 -11549.2, -3593.77, 14.75, 0,
 'Iron Demolisher 254990 - Timer - Cannon Blast 75y'),

-- GUID 254991
(-254991, 0, 1, 0, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 11, 171950, 2, 0, 0, 0, 0,
 8, 0, 0, 0, 0,
 -11285.2, -3599.18, 18.11, 0,
 'Iron Demolisher 254991 - Timer - Cannon Blast 75y'),

-- GUID 254992
(-254992, 0, 1, 0, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 11, 171950, 2, 0, 0, 0, 0,
 8, 0, 0, 0, 0,
 -11464.3, -3623.18, 14.26, 0,
 'Iron Demolisher 254992 - Timer - Cannon Blast 75y'),

-- GUID 254993
(-254993, 0, 1, 0, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 11, 171950, 2, 0, 0, 0, 0,
 8, 0, 0, 0, 0,
 -11367.7, -3599.06, 11.32, 0,
 'Iron Demolisher 254993 - Timer - Cannon Blast 75y');

-- =============================================================================
-- Fix 2 : Action list 8227300 — explosion puis mort
-- SpellClick → CAST 171950 sur soi-même (explosion au niveau du canon)
--           → DIE 3s plus tard (animation de mort du modèle 53699)
-- =============================================================================

DELETE FROM smart_scripts
WHERE entryorguid=8227300 AND source_type=9;

INSERT INTO smart_scripts
  (entryorguid, source_type, id, link, Difficulties,
   event_type, event_phase_mask, event_chance, event_flags,
   event_param1, event_param2, event_param3, event_param4, event_param5,
   action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6,
   target_type, target_param1, target_param2, target_param3, target_param4,
   target_x, target_y, target_z, target_o,
   comment)
VALUES
-- id=0 : explosion immédiate au niveau du canon (CAST 171950 triggered sur soi-même)
(8227300, 9, 0, 0, '',
 0, 0, 100, 0,
 0, 0, 0, 0, 0,
 11, 171950, 2, 0, 0, 0, 0,
 1, 0, 0, 0, 0,
 0, 0, 0, 0,
 'Iron Demolisher - Destroy - Explosion au canon (self-cast 171950)'),

-- id=1 : mort après 3s (animation de mort native du modèle)
(8227300, 9, 1, 0, '',
 0, 0, 100, 0,
 3000, 0, 0, 0, 0,
 37, 0, 0, 0, 0, 0, 0,
 1, 0, 0, 0, 0,
 0, 0, 0, 0,
 'Iron Demolisher - Destroy - Die (animation mort modele 53699)');

-- =============================================================================
-- Fix 3 : Respawn 20 secondes (au lieu de 2 minutes)
-- =============================================================================

UPDATE creature
SET spawntimesecs = 20
WHERE guid IN (254972, 254990, 254991, 254992, 254993);

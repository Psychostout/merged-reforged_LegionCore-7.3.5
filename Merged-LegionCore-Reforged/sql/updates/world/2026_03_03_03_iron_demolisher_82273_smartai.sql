-- Iron Demolisher (NPC 82273) — SmartAI
-- Ajoute :
--   1. Boulet de canon (spell 171950 "Cannon Blast") toutes les 6-10s sur le joueur le plus proche (35y)
--   2. Séquence de destruction au SpellClick : animkit 4169 (explosion Iron Horde) puis mort 2s après
-- Sort de boulet : spell 171950 = "Hurls a massive boulder... Fire damage + 1000 siege dmg in 10y"
-- AnimKit  : 4169 = Iron Horde War Machine destruction animation (NPC 81181)

-- ─── 1. Activer SmartAI ───────────────────────────────────────────────────────
UPDATE creature_template
    SET AIName = 'SmartAI'
WHERE entry = 82273;

-- ─── 2. Scripts NPC 82273 (source_type = 0) ──────────────────────────────────
DELETE FROM smart_scripts WHERE entryorguid = 82273 AND source_type = 0;

INSERT INTO smart_scripts
    (entryorguid, source_type, id, link, Difficulties,
     event_type, event_phase_mask, event_chance, event_flags,
     event_param1, event_param2, event_param3, event_param4, event_param5,
     action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6,
     target_type, target_param1, target_param2, target_param3, target_param4,
     target_x, target_y, target_z, target_o,
     comment)
VALUES
-- SpellClick joueur → déclenche la séquence de destruction (action list 8227300)
(82273, 0, 0, 0, '',
 73, 0, 100, 0,
 0, 0, 0, 0, 0,
 80, 8227300, 2, 0, 0, 0, 0,
 1, 0, 0, 0, 0,
 0, 0, 0, 0,
 'Iron Demolisher - SpellClick - Déclenche destruction'),

-- Timer : tire un boulet de canon (spell 171950) sur le joueur le plus proche (35y), toutes les 6-10s
-- event_type 60 = UPDATE (déclenche en et hors combat)
(82273, 0, 1, 0, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 11, 171950, 0, 0, 0, 0, 0,
 17, 35, 0, 0, 0,
 0, 0, 0, 0,
 'Iron Demolisher - Timer - Cannon Blast (171950) sur joueur le plus proche');

-- ─── 3. Action list 8227300 : séquence de destruction ────────────────────────
DELETE FROM smart_scripts WHERE entryorguid = 8227300 AND source_type = 9;

INSERT INTO smart_scripts
    (entryorguid, source_type, id, link, Difficulties,
     event_type, event_phase_mask, event_chance, event_flags,
     event_param1, event_param2, event_param3, event_param4, event_param5,
     action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6,
     target_type, target_param1, target_param2, target_param3, target_param4,
     target_x, target_y, target_z, target_o,
     comment)
VALUES
-- À 0ms : joue l'animkit d'explosion Iron Horde War Machine (4169)
(8227300, 9, 0, 0, '',
 0, 0, 100, 0,
 0, 0, 0, 0, 0,
 128, 4169, 1, 0, 0, 0, 0,
 1, 0, 0, 0, 0,
 0, 0, 0, 0,
 'Iron Demolisher - Destroy - AnimKit explosion (4169)'),

-- À 2000ms : la machine meurt (fin visible avec l'animation)
(8227300, 9, 1, 0, '',
 0, 0, 100, 0,
 2000, 0, 0, 0, 0,
 37, 0, 0, 0, 0, 0, 0,
 1, 0, 0, 0, 0,
 0, 0, 0, 0,
 'Iron Demolisher - Destroy - Die after explosion');

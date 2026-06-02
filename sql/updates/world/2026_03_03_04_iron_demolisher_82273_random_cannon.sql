-- Iron Demolisher (NPC 82273) — Boulet de canon à position aléatoire
-- Remplace le tir sur le joueur (id=1) par une invocation de bunny invisible
-- à une position aléatoire dans un rayon de 20y autour de la machine.
--
-- Architecture :
--   NPC 82273 (timer 6-10s) → SUMMON_CREATURE(600400) à SMART_TARGET_RANDOM_POSITION (range=20y)
--   NPC 600400 (bunny invisible, trigger) → JUST_SUMMONED → CAST 171950 sur soi-même (triggered)
--
-- Spell 171950 "Cannon Blast" crée déjà la texture de zone + dégâts AoE 10y.
-- Le bunny est invisible aux joueurs (CREATURE_FLAG_EXTRA_TRIGGER + display=11686).

-- ─── 1. Créer la créature bunny 600400 ───────────────────────────────────────

-- Entrée WDB (nom + modèle invisible "stalker")
INSERT IGNORE INTO creature_template_wdb
    (Entry, Name1, Displayid1)
VALUES
    (600400, 'Iron Demolisher Cannon Impact', 11686);

-- Entrée template : invisible, pas sélectionnable, immunisé joueurs, SmartAI
INSERT IGNORE INTO creature_template
    (entry, gossip_menu_id, minlevel, maxlevel,
     faction, unit_flags, flags_extra, AIName)
VALUES
    (600400, 0, 1, 1,
     35, 33554688, 128, 'SmartAI');

-- ─── 2. SmartAI du bunny 600400 : lance le sort dès l'invocation ─────────────

DELETE FROM smart_scripts WHERE entryorguid = 600400 AND source_type = 0;

INSERT INTO smart_scripts
    (entryorguid, source_type, id, link, Difficulties,
     event_type, event_phase_mask, event_chance, event_flags,
     event_param1, event_param2, event_param3, event_param4, event_param5,
     action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6,
     target_type, target_param1, target_param2, target_param3, target_param4,
     target_x, target_y, target_z, target_o,
     comment)
VALUES
-- JUST_SUMMONED : lance spell 171950 (triggered) sur soi → explosion à la position du bunny
(600400, 0, 0, 0, '',
 54, 0, 100, 0,
 0, 0, 0, 0, 0,
 11, 171950, 2, 0, 0, 0, 0,
 1, 0, 0, 0, 0,
 0, 0, 0, 0,
 'Cannon Impact Bunny - Just Summoned - Cast Cannon Blast (171950) on self (triggered)');

-- ─── 3. Mettre à jour NPC 82273 : tir aléatoire via bunny ─────────────────────

-- Supprimer l'ancienne entrée (tir sur joueur le plus proche)
DELETE FROM smart_scripts WHERE entryorguid = 82273 AND source_type = 0 AND id = 1;

INSERT INTO smart_scripts
    (entryorguid, source_type, id, link, Difficulties,
     event_type, event_phase_mask, event_chance, event_flags,
     event_param1, event_param2, event_param3, event_param4, event_param5,
     action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6,
     target_type, target_param1, target_param2, target_param3, target_param4,
     target_x, target_y, target_z, target_o,
     comment)
VALUES
-- Timer 6-10s : invoque bunny invisible (600400) à une position aléatoire dans 20y
-- SMART_ACTION_SUMMON_CREATURE (12) :
--   param1 = entry (600400)
--   param2 = TempSummonType (3 = TEMPSUMMON_TIMED_DESPAWN)
--   param3 = durée avant dépop en ms (5000 = 5s, largement le temps pour l'animation)
-- SMART_TARGET_RANDOM_POSITION (128) :
--   target_param1 = range (20 = rayon max de dispersion en yards)
--   target_param2 = angle (0 = vers l'avant du NPC, en degrés)
--   target_param3 = distance (0 = depuis la position du NPC, pas de décalage avant)
(82273, 0, 1, 0, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 12, 600400, 3, 5000, 0, 0, 0,
 128, 20, 0, 0, 0,
 0, 0, 0, 0,
 'Iron Demolisher - Timer - Invoque bunny impact (600400) a position aleatoire (20y)');

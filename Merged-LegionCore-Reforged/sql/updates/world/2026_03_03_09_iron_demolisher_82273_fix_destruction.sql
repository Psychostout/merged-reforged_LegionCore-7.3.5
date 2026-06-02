-- Iron Demolisher (NPC 82273) — Fix séquence de destruction
-- Problème : AnimKit 4169 est conçu pour le modèle 55226 (NPC 81181),
--            pas pour le modèle 53699 (NPC 82273/82298).
--            Résultat visuel : un "saut" au lieu d'une animation d'explosion.
--            De plus, event_param2=0 → délai aléatoire urand(0,2000) au lieu de fixe.
--
-- Référence : NPC 82298 (même modèle 53699) — action list 8229800 :
--   id=0 : DIE après 3000ms fixe, aucun animkit → fonctionne correctement.
--
-- Fix : supprimer animkit 4169, garder seulement DIE avec délai fixe 2s.
--       event_param1 = event_param2 = 2000 → RecalcTimer(2000,2000) = délai fixe 2000ms.

DELETE FROM smart_scripts
WHERE entryorguid = 8227300 AND source_type = 9;

INSERT INTO smart_scripts
    (entryorguid, source_type, id, link, Difficulties,
     event_type, event_phase_mask, event_chance, event_flags,
     event_param1, event_param2, event_param3, event_param4, event_param5,
     action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6,
     target_type, target_param1, target_param2, target_param3, target_param4,
     target_x, target_y, target_z, target_o,
     comment)
VALUES
(8227300, 9, 0, 0, '',
 0, 0, 100, 0,
 2000, 2000, 0, 0, 0,
 37, 0, 0, 0, 0, 0, 0,
 1, 0, 0, 0, 0,
 0, 0, 0, 0,
 'Iron Demolisher - Destroy - Die after 2s (natural model death anim)');

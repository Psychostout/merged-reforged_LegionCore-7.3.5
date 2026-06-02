-- Iron Demolisher (NPC 82273) — Mise à jour point d'impact à 75y (au lieu de 30y)
-- Positions recalculées : pos + 75 * (cos(ori), sin(ori)) pour chaque instance.

UPDATE smart_scripts SET
    target_x = -11412.90, target_y = -3504.01, target_z = 8.24,
    comment  = 'Iron Demolisher 254972 - Timer - Cannon Blast 75y devant'
WHERE entryorguid = -254972 AND source_type = 0 AND id = 1;

UPDATE smart_scripts SET
    target_x = -11549.18, target_y = -3593.77, target_z = 14.75,
    comment  = 'Iron Demolisher 254990 - Timer - Cannon Blast 75y devant'
WHERE entryorguid = -254990 AND source_type = 0 AND id = 1;

UPDATE smart_scripts SET
    target_x = -11285.15, target_y = -3599.18, target_z = 18.11,
    comment  = 'Iron Demolisher 254991 - Timer - Cannon Blast 75y devant'
WHERE entryorguid = -254991 AND source_type = 0 AND id = 1;

UPDATE smart_scripts SET
    target_x = -11464.33, target_y = -3623.18, target_z = 14.26,
    comment  = 'Iron Demolisher 254992 - Timer - Cannon Blast 75y devant'
WHERE entryorguid = -254992 AND source_type = 0 AND id = 1;

UPDATE smart_scripts SET
    target_x = -11367.65, target_y = -3599.06, target_z = 11.32,
    comment  = 'Iron Demolisher 254993 - Timer - Cannon Blast 75y devant'
WHERE entryorguid = -254993 AND source_type = 0 AND id = 1;

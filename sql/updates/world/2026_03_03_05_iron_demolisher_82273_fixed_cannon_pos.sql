-- Iron Demolisher (NPC 82273) — Tir de canon à position fixe 30y devant
-- Remplace l'approche bunny (SQL 04) : chaque machine tire à un point fixe absolu
-- calculé comme pos + 30y * (cos(ori), sin(ori)) pour chaque instance.
--
-- Architecture :
--   Script générique (entryorguid=82273) : SpellClick → destruction (inchangé)
--   Scripts GUID (-guid)                 : Timer 6-10s → CAST 171950 au point fixe 30y devant
--
-- SMART_ACTION_CAST (11) + SMART_TARGET_POSITION (8) + target_x/y/z absolus
-- CastFlags = 2 (SMARTCAST_TRIGGERED, pas de temps de cast)

-- ─── 1. Annuler le SQL précédent (approche bunny) ─────────────────────────────

DELETE FROM creature_template     WHERE entry = 600400;
DELETE FROM creature_template_wdb WHERE Entry = 600400;
DELETE FROM smart_scripts         WHERE entryorguid = 600400;

-- Supprimer l'entrée id=1 générique sur entry 82273 (tir bunny / joueur)
DELETE FROM smart_scripts WHERE entryorguid = 82273 AND source_type = 0 AND id = 1;

-- ─── 2. Scripts GUID-spécifiques : tir à position fixe ───────────────────────
-- Format entryorguid = -guid pour les scripts d'instance unique

DELETE FROM smart_scripts
WHERE entryorguid IN (-254972, -254990, -254991, -254992, -254993)
  AND source_type = 0 AND id = 1;

INSERT INTO smart_scripts
    (entryorguid, source_type, id, link, Difficulties,
     event_type, event_phase_mask, event_chance, event_flags,
     event_param1, event_param2, event_param3, event_param4, event_param5,
     action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6,
     target_type, target_param1, target_param2, target_param3, target_param4,
     target_x, target_y, target_z, target_o,
     comment)
VALUES
-- guid 254972 → point d'impact : (-11413.32, -3459.01, 8.24)
(-254972, 0, 1, 0, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 11, 171950, 2, 0, 0, 0, 0,
 8, 0, 0, 0, 0,
 -11413.32, -3459.01, 8.24, 0,
 'Iron Demolisher 254972 - Timer - Cannon Blast 30y devant'),

-- guid 254990 → point d'impact : (-11538.21, -3550.12, 14.75)
(-254990, 0, 1, 0, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 11, 171950, 2, 0, 0, 0, 0,
 8, 0, 0, 0, 0,
 -11538.21, -3550.12, 14.75, 0,
 'Iron Demolisher 254990 - Timer - Cannon Blast 30y devant'),

-- guid 254991 → point d'impact : (-11299.76, -3556.61, 18.11)
(-254991, 0, 1, 0, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 11, 171950, 2, 0, 0, 0, 0,
 8, 0, 0, 0, 0,
 -11299.76, -3556.61, 18.11, 0,
 'Iron Demolisher 254991 - Timer - Cannon Blast 30y devant'),

-- guid 254992 → point d'impact : (-11449.61, -3580.65, 14.26)
(-254992, 0, 1, 0, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 11, 171950, 2, 0, 0, 0, 0,
 8, 0, 0, 0, 0,
 -11449.61, -3580.65, 14.26, 0,
 'Iron Demolisher 254992 - Timer - Cannon Blast 30y devant'),

-- guid 254993 → point d'impact : (-11378.54, -3555.40, 11.32)
(-254993, 0, 1, 0, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 11, 171950, 2, 0, 0, 0, 0,
 8, 0, 0, 0, 0,
 -11378.54, -3555.40, 11.32, 0,
 'Iron Demolisher 254993 - Timer - Cannon Blast 30y devant');

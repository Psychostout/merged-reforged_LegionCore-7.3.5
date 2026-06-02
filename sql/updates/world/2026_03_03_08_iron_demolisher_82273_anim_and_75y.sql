-- Iron Demolisher (NPC 82273) — Animation d'attaque + distance 75y
-- Restructure les scripts GUID :
--   id=1 (timer 6-10s) : PLAY_EMOTE(20 = EMOTE_ONESHOT_SHOOT) sur SELF, link=2
--   id=2 (lié)         : CAST spell 171950 (triggered) au point fixe 75y devant
--
-- Emote 20 = EMOTE_ONESHOT_SHOOT : animation "tir" OneShot, appropriée pour un canon.
-- Si l'animation ne convient pas visuellement, tester avec emote 26 (ATTACK1H)
-- ou emote 44 (ATTACK2HTIGHT) via .npc playemote en jeu.

-- ─── Supprimer anciens scripts GUID ──────────────────────────────────────────
DELETE FROM smart_scripts
WHERE entryorguid IN (-254972, -254990, -254991, -254992, -254993)
  AND source_type = 0;

-- ─── Insérer les nouveaux scripts (id=1 animkit, id=2 cast lié) ──────────────
INSERT INTO smart_scripts
    (entryorguid, source_type, id, link, Difficulties,
     event_type, event_phase_mask, event_chance, event_flags,
     event_param1, event_param2, event_param3, event_param4, event_param5,
     action_type, action_param1, action_param2, action_param3, action_param4, action_param5, action_param6,
     target_type, target_param1, target_param2, target_param3, target_param4,
     target_x, target_y, target_z, target_o,
     comment)
VALUES

-- ══ guid 254972 ══════════════════════════════════════════════════════════════

-- id=1 : timer 6-10s → EMOTE_ONESHOT_SHOOT (20) sur soi, puis lié à id=2
(-254972, 0, 1, 2, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 5, 20, 0, 0, 0, 0, 0,
 1, 0, 0, 0, 0,
 0, 0, 0, 0,
 'Iron Demolisher 254972 - Timer - Emote shoot, puis canon'),

-- id=2 : lié — CAST 171950 (triggered) au point 75y devant
(-254972, 0, 2, 0, '',
 0, 0, 100, 0,
 0, 0, 0, 0, 0,
 11, 171950, 2, 0, 0, 0, 0,
 8, 0, 0, 0, 0,
 -11412.90, -3504.01, 8.24, 0,
 'Iron Demolisher 254972 - Linked - Cannon Blast 75y devant'),

-- ══ guid 254990 ══════════════════════════════════════════════════════════════

(-254990, 0, 1, 2, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 5, 20, 0, 0, 0, 0, 0,
 1, 0, 0, 0, 0,
 0, 0, 0, 0,
 'Iron Demolisher 254990 - Timer - Emote shoot, puis canon'),

(-254990, 0, 2, 0, '',
 0, 0, 100, 0,
 0, 0, 0, 0, 0,
 11, 171950, 2, 0, 0, 0, 0,
 8, 0, 0, 0, 0,
 -11549.18, -3593.77, 14.75, 0,
 'Iron Demolisher 254990 - Linked - Cannon Blast 75y devant'),

-- ══ guid 254991 ══════════════════════════════════════════════════════════════

(-254991, 0, 1, 2, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 5, 20, 0, 0, 0, 0, 0,
 1, 0, 0, 0, 0,
 0, 0, 0, 0,
 'Iron Demolisher 254991 - Timer - Emote shoot, puis canon'),

(-254991, 0, 2, 0, '',
 0, 0, 100, 0,
 0, 0, 0, 0, 0,
 11, 171950, 2, 0, 0, 0, 0,
 8, 0, 0, 0, 0,
 -11285.15, -3599.18, 18.11, 0,
 'Iron Demolisher 254991 - Linked - Cannon Blast 75y devant'),

-- ══ guid 254992 ══════════════════════════════════════════════════════════════

(-254992, 0, 1, 2, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 5, 20, 0, 0, 0, 0, 0,
 1, 0, 0, 0, 0,
 0, 0, 0, 0,
 'Iron Demolisher 254992 - Timer - Emote shoot, puis canon'),

(-254992, 0, 2, 0, '',
 0, 0, 100, 0,
 0, 0, 0, 0, 0,
 11, 171950, 2, 0, 0, 0, 0,
 8, 0, 0, 0, 0,
 -11464.33, -3623.18, 14.26, 0,
 'Iron Demolisher 254992 - Linked - Cannon Blast 75y devant'),

-- ══ guid 254993 ══════════════════════════════════════════════════════════════

(-254993, 0, 1, 2, '',
 60, 0, 100, 0,
 6000, 10000, 6000, 10000, 0,
 5, 20, 0, 0, 0, 0, 0,
 1, 0, 0, 0, 0,
 0, 0, 0, 0,
 'Iron Demolisher 254993 - Timer - Emote shoot, puis canon'),

(-254993, 0, 2, 0, '',
 0, 0, 100, 0,
 0, 0, 0, 0, 0,
 11, 171950, 2, 0, 0, 0, 0,
 8, 0, 0, 0, 0,
 -11367.65, -3599.06, 11.32, 0,
 'Iron Demolisher 254993 - Linked - Cannon Blast 75y devant');

-- Fix animations for Iron Demolisher NPC 82273
--
-- Problems fixed:
-- 1. Entry id=1 : legacy timer that cast 171950 at nearest player — conflicts with GUID scripts
--    (GUID scripts already handle cannon firing at fixed positions per instance)
-- 2. GUID scripts id=1 : PLAY_EMOTE(20) = EMOTE_ONESHOT_SHOOT — humanoid emote, does nothing on
--    mechanical model 53699. Replaced with SMART_ACTION_NONE (timer still fires and links to cast).
-- 3. Action list 8227300 id=0 : PLAY_ANIMKIT(4169, type=1) — animkit 4169 is NOT in AnimKit.db2
--    (sAnimKitStore) so SetAnimKitId() returns early without sending any packet. Replaced with DIE.
--    The model's natural death animation plays automatically when DIE is called, showing the
--    cannon falling apart / broken state (AnimID Dead for model 53699).

-- Fix 1: Remove conflicting entry-level timer (was also casting 171950 at nearest player)
DELETE FROM smart_scripts
WHERE entryorguid = 82273
  AND source_type = 0
  AND id = 1;

-- Fix 2: GUID scripts id=1 — replace PLAY_EMOTE(20) with SMART_ACTION_NONE
-- The SMART_EVENT_UPDATE timer still fires and the link=2 still triggers the cannon cast (id=2).
UPDATE smart_scripts
SET action_type = 0,
    action_param1 = 0,
    comment = 'Iron Demolisher - Timer - linked to cannon blast'
WHERE entryorguid IN (-254972, -254990, -254991, -254992, -254993)
  AND source_type = 0
  AND id = 1;

-- Fix 3: Action list 8227300 — replace non-working PLAY_ANIMKIT(4169) with DIE
-- Natural death animation of model 53699 shows the cannon in destroyed/broken state.
-- 500ms initial delay gives the spellclick spell visual time to play before the cannon dies.
DELETE FROM smart_scripts
WHERE entryorguid = 8227300
  AND source_type = 9;

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
   500, 0, 0, 0, 0,
   37, 0, 0, 0, 0, 0, 0,
   1, 0, 0, 0, 0,
   0, 0, 0, 0,
   'Iron Demolisher - Destroy - Die (natural death anim = cannon breaks apart)');

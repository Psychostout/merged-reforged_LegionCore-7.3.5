-- Disable pre-WoD Iron Horde Invasion event quests (Alliance + Horde)
-- These quests were part of the patch 6.0.1 pre-launch event and are no longer available.
-- Quest 36498: Invasion de la Horde de Fer (Alliance) — given by Hero's Call Board
-- Quest 36499: Invasion de la Horde de Fer (Horde) — given by Warlord's Command Board
INSERT INTO disables (sourceType, entry, flags, comment) VALUES
(1, 36498, 0, 'Pre-WoD Iron Horde Invasion event quest (Alliance) — no longer available since 6.0.2'),
(1, 36499, 0, 'Pre-WoD Iron Horde Invasion event quest (Horde) — no longer available since 6.0.2')
ON DUPLICATE KEY UPDATE comment = VALUES(comment);

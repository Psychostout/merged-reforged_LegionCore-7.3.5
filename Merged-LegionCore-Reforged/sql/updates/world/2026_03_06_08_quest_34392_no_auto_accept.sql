-- Quest 34392 "Onslaught's End" : retirer QUEST_FLAGS_AUTO_ACCEPT (0x80000)
-- Flags avant : 38273024 (0x247F400) | attendu après : 37748736 (0x2400000)
UPDATE quest_template SET Flags = Flags & ~0x80000 WHERE ID = 34392;

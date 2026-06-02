-- ==========================================================================
-- Demon Invasion Prepatch Framework
-- Phase 1: OutdoorPvP template + GameEvents (no creatures yet)
-- ==========================================================================

-- OutdoorPvP template: 2 entries (one per continent)
DELETE FROM `outdoorpvp_template` WHERE `TypeId` IN (16, 17);
INSERT INTO `outdoorpvp_template` (`TypeId`, `ScriptName`, `ZoneList`, `MapList`, `comment`) VALUES
(16, 'outdoorpvp_demon_invasion_ek',  '1 267 40',  '0', 'Demon Invasion Pre-patch EK'),
(17, 'outdoorpvp_demon_invasion_kal', '16 10 440', '1', 'Demon Invasion Pre-patch Kalimdor');

-- GameEvents: 6 zones x 4 stages = 24 events (IDs 900-923)
-- All GAMEEVENT_INTERNAL (world_event=5) — controlled exclusively by code
-- occurence/length set to 60 days so they never auto-trigger
DELETE FROM `game_event` WHERE `eventEntry` BETWEEN 900 AND 923;
INSERT INTO `game_event` (`eventEntry`, `start_time`, `end_time`, `occurence`, `length`, `holiday`, `description`, `world_event`) VALUES
-- Zone 0: Azshara (ZoneID 16, Map 1 Kalimdor) — events 900-903
(900, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Azshara S1 Defend', 5),
(901, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Azshara S2 Commander', 5),
(902, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Azshara S3 Repel', 5),
(903, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Azshara S4 Boss', 5),
-- Zone 1: Dun Morogh (ZoneID 1, Map 0 EK) — events 904-907
(904, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Dun Morogh S1 Defend', 5),
(905, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Dun Morogh S2 Commander', 5),
(906, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Dun Morogh S3 Repel', 5),
(907, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Dun Morogh S4 Boss', 5),
-- Zone 2: Hillsbrad (ZoneID 267, Map 0 EK) — events 908-911
(908, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Hillsbrad S1 Defend', 5),
(909, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Hillsbrad S2 Commander', 5),
(910, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Hillsbrad S3 Repel', 5),
(911, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Hillsbrad S4 Boss', 5),
-- Zone 3: Northern Barrens (ZoneID 10, Map 1 Kalimdor) — events 912-915
(912, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI N.Barrens S1 Defend', 5),
(913, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI N.Barrens S2 Commander', 5),
(914, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI N.Barrens S3 Repel', 5),
(915, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI N.Barrens S4 Boss', 5),
-- Zone 4: Tanaris (ZoneID 440, Map 1 Kalimdor) — events 916-919
(916, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Tanaris S1 Defend', 5),
(917, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Tanaris S2 Commander', 5),
(918, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Tanaris S3 Repel', 5),
(919, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Tanaris S4 Boss', 5),
-- Zone 5: Westfall (ZoneID 40, Map 0 EK) — events 920-923
(920, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Westfall S1 Defend', 5),
(921, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Westfall S2 Commander', 5),
(922, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Westfall S3 Repel', 5),
(923, '2000-01-01 00:00:00', '2035-01-01 00:00:00', 5184000, 5184000, 0, 'DI Westfall S4 Boss', 5);

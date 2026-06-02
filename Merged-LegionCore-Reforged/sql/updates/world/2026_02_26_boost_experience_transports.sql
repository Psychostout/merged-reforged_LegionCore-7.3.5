-- Boost Experience maps: spawn transports server-side
-- Map 1554 (Alliance - Sword of Dawn) : Alliance Gunship, taxiPathID 2222
-- Map 1557 (Horde - Tempest's Roar)   : Orc Gunship,      taxiPathID 2337
--
-- Note: Instance transports are auto-spawned by TransportMgr::CreateInstanceTransports()
-- from gameobject_template type=15. The `transports` table entries below serve as
-- documentation and are skipped by SpawnContinentTransports() for instanced maps.

REPLACE INTO `transports` (`guid`, `entry`, `name`, `ScriptName`) VALUES
(23, 204018, 'Boost Experience Alliance Gunship', ''),
(24, 204423, 'Boost Experience Horde Gunship', '');

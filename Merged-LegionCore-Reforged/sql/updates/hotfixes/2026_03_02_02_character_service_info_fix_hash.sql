-- Fix: Wrong TableHash in hotfix_data for CharacterServiceInfo
-- 0xADE120EF is the LayoutHash (DB2Meta, used for file format validation)
-- 0x56AA902B is the real TableHash from CharacterServiceInfo.db2 header
--   → used to key _stores[] → used by LoadHotfixData to find the store
SET NAMES utf8mb4;

UPDATE `hotfix_data`
SET `TableHash` = 0x56AA902B
WHERE `TableHash` = 0xADE120EF;

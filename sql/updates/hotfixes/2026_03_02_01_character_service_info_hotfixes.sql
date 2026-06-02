-- Fix: CharacterServiceInfo missing from hotfix_data
-- Without these entries, the server never advertises CharacterServiceInfo to clients
-- via SMSG_AVAILABLE_HOTFIXES, so GetCharacterServiceDisplayOrder() always returns []
-- TableHash 0xADE120EF = CharacterServiceInfoMeta (DB2Metadata.h line 975)
SET NAMES utf8mb4;

DELETE FROM `hotfix_data` WHERE `TableHash` = 0xADE120EF;
INSERT INTO `hotfix_data` (`Id`, `TableHash`, `RecordID`, `Timestamp`, `Deleted`) VALUES
(9000212, 0xADE120EF, 1, UNIX_TIMESTAMP(), 0),
(9000213, 0xADE120EF, 6, UNIX_TIMESTAMP(), 0);

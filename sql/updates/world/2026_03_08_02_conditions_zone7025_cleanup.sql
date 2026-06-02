-- Suppression des conditions SmartAI orphelines pour zone 7025.
-- Ces 125 entrées référencent des smart_scripts qui n'existent plus (entryorguid=7025
-- a 0 entrées dans smart_scripts). Reliquat de l'ancien système de phases DB.
DELETE FROM `world_legion`.`conditions`
WHERE `SourceGroup` = 7025 AND `SourceTypeOrReferenceId` = 23;

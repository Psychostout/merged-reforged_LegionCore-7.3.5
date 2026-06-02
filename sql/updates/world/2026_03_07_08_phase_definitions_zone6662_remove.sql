-- Phase definitions zone 6662 — suppression entrées obsolètes
-- Map 1265 (Draenor intro) : phases gérées en C++ par playerscript_wod_portal_phases
-- Entrées supprimées : entry 1 (phase 3000), entry 11 (phase 4201), entry 12 (phase 4204)
-- Sans ces DELETE, phase 4201 (portail détruit) s'applique sans condition à tout joueur en zone 6662

-- Sauvegarde de référence :
-- entry 1  : phase 3000, flags=24 (Base Phase)
-- entry 11 : phase 4201, comment="Quest=35045" — aucune condition en DB → appliquée à tous
-- entry 12 : phase 4204, comment="quest 34980" — aucune condition en DB → appliquée à tous

DELETE FROM `world_legion`.`phase_definitions` WHERE `zoneId` = 6662;

-- Suppression des entrées spell_area du spell 164678 pour zone 7025 (Dark Portal intro).
-- Ce spell s'auto-castait quand Q_35933 était INCOMPLETE, ajoutant phase 4201
-- indépendamment du script C++ playerscript_wod_portal_phases.
-- Les phases de map 1265 sont désormais entièrement gérées en C++.
DELETE FROM `world_legion`.`spell_area` WHERE `spell` = 164678 AND `area` = 7025;

-- Vérification : lister les autres spell_area pour zone 7025 avec autocast=1
-- (à lire dans le retour pour identifier d'éventuels spells résiduels avec effets de phase)
SELECT spell, area, quest_start, quest_end, quest_start_status, quest_end_status, autocast
FROM world_legion.spell_area WHERE area = 7025 AND autocast = 1 ORDER BY spell;

-- Fix 4 : Investigation phase 4201 dans phase_definitions
-- Étape 1 : lister toutes les zones qui contiennent phase 4201
SELECT zoneId, entry, phaseId, flags, comment
FROM world_legion.phase_definitions
WHERE phaseId = 4201
ORDER BY zoneId;

-- Étape 2 : résumé des grandes zones suspectes (sous-zones potentielles de map 1265)
SELECT zoneId, COUNT(*) cnt, GROUP_CONCAT(DISTINCT phaseId ORDER BY phaseId) phases
FROM world_legion.phase_definitions
WHERE zoneId IN (7637, 7503, 7502, 7558, 7025)
GROUP BY zoneId;

-- Si phase 4201 apparaît dans des sous-zones de map 1265, les supprimer :
-- DELETE FROM world_legion.phase_definitions WHERE zoneId IN (...) AND phaseId = 4201;
-- (décommenter et adapter après lecture des résultats ci-dessus)

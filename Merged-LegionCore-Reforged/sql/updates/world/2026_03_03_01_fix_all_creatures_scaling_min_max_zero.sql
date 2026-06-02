-- Fix: Toutes les créatures avec LevelScalingMin=0 et LevelScalingMax=0 mouraient instantanément
-- Cause: La simple existence d'une entrée dans creature_template_scaling active HasScalableLevels()=true.
--        Avec Min=Max=0, GetLevelForTarget() retourne toujours 0 pour tout attaquant.
--        SetHealthScal() divise par scaleStat->healthMax (niveau 0 = 0) → UB/HP corrompu → mort instantanée.
-- Fix: Supprimer toutes ces entrées invalides. Les créatures concernées utilisent leur niveau réel.
-- Nombre d'entrées supprimées : ~9971
-- RÈGLE : toute entrée creature_template_scaling avec LevelScalingMin=LevelScalingMax=0
--         pour une créature de niveau > 0 est invalide et doit être supprimée ou corrigée.
DELETE FROM creature_template_scaling WHERE LevelScalingMin = 0 AND LevelScalingMax = 0;

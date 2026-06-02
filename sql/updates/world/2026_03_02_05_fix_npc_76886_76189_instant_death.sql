-- Fix: NPCs 76886, 76189 et 76524 meurent instantanément dès le premier dégât
-- Cause: creature_template_scaling avec LevelScalingMin=0/LevelScalingMax=0 activait
--        HasScalableLevels()=true, faisant retourner GetLevelForTarget()=0 pour tout joueur,
--        et causant une division par zéro dans SetHealthScal() (healthMax du niveau 0 = 0).
--        Résultat : HP corrompu à 0 → mort instantanée au premier dégât.
-- Fix: Supprimer les entrées de scaling incorrectes → ces créatures utilisent leur niveau normalement.
-- RÈGLE : toute entrée creature_template_scaling avec LevelScalingMin=LevelScalingMax=0
--         pour une créature de niveau > 0 doit être supprimée ou corrigée.
DELETE FROM creature_template_scaling WHERE Entry IN (76886, 76189, 76524);

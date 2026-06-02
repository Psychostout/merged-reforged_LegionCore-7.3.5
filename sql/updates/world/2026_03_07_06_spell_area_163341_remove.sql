-- Supprime la règle spell_area qui retirait automatiquement le sort 163341 à la récompense de Q34393.
-- La gestion est désormais entièrement en C++ (playerscript_wod_portal_ambient dans dark_portal.cpp) :
--   - sort appliqué à l'arrivée sur map 1265
--   - sort retiré à la récompense de Q34420 (Into the Portal)
--
-- Pour remettre la règle automatique (si le PlayerScript est retiré) :
-- INSERT INTO spell_area (spell, area, quest_start, quest_end, aura_spell, racemask, classmask, active_event, gender, autocast, quest_start_status, quest_end_status)
-- VALUES (163341, 7025, 0, 34393, 0, 0, 0, 0, 2, 1, 64, 66);

DELETE FROM world_legion.spell_area WHERE spell = 163341 AND area = 7025;

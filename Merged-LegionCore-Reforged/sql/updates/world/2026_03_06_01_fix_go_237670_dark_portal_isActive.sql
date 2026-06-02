-- GO 237670 (Dark Portal, map 1265, guid 105799)
-- isActive=1 : force la grille à rester chargée (évite le déchargement après le spawn)
-- Complément du fix C++ Map.cpp : if (id == 1265) LoadGrid(4064.08f, -2338.02f)
UPDATE `gameobject` SET `isActive`=1 WHERE `guid`=105799;

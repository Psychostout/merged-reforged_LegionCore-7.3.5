-- Quest 34392 "Onslaught's End" : ajouter NPC 78554 (Khadgar) comme queststarter
-- NPC 78554 est présent dans les phases 3248/3569/3264/4011 (toutes les phases principales)
-- permettant au joueur de reprendre la quête s'il l'abandonne
INSERT IGNORE INTO creature_queststarter (id, quest) VALUES (78554, 34392);

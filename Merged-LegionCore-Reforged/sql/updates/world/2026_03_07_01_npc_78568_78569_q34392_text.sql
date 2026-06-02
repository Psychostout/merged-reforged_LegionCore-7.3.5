-- creature_text pour dialogues de proximité Q34392 "Onslaught's End"
-- NPC 78568 (Thaelin Darkanvil)   GroupID=1 : "Ne vous inquiétez pas. Nous sommes là pour vous couvrir !"
-- NPC 78569 (Hansel Lourdemains)  GroupID=0 : "Allez-y !" (son 45699)
-- Textes stockés en hex UTF-8 pour éviter les problèmes d'encodage Windows

DELETE FROM creature_text WHERE CreatureID IN (78568, 78569) AND GroupID IN (0, 1) AND ID = 0;

INSERT INTO creature_text (CreatureID, GroupID, ID, Text, Type, Language, Probability, Emote, Duration, Sound, BroadcastTextID, MinTimer, MaxTimer, SpellID, comment)
VALUES
-- "Ne vous inquiétez pas. Nous sommes là pour vous couvrir !"
(78568, 1, 0, X'4E6520766F757320696E717569C3A974657A207061732E204E6F757320736F6D6D6573206CC3A020706F757220766F757320636F75767269722021', 12, 0, 100, 0, 0, 0, 0, 0, 0, 0, 'Thaelin Darkanvil - Q34392 proximity'),
-- "Allez-y !"
(78569, 0, 0, 'Allez-y !', 12, 0, 100, 0, 0, 45699, 0, 0, 0, 0, 'Hansel Lourdemains - Q34392 proximity');

-- ============================================================================
-- Batch 13: Correctifs des erreurs persistantes après batches 1-12
-- Date: 2026-02-25
-- ============================================================================

-- ========== 13a: SmartAI self-link — DELETE les lignes avec link=id ==========
-- link fait partie de la PK, on ne peut pas UPDATE vers link=0 si une row link=0 existe déjà
DELETE FROM smart_scripts WHERE entryorguid=1519 AND source_type=13 AND id=1 AND link=1;
DELETE FROM smart_scripts WHERE entryorguid=6854 AND source_type=2 AND id=1 AND link=1;
DELETE FROM smart_scripts WHERE entryorguid=11441 AND source_type=0 AND id=5 AND link=5;
DELETE FROM smart_scripts WHERE entryorguid=14322 AND source_type=0 AND id=8 AND link=8;
DELETE FROM smart_scripts WHERE entryorguid=26811 AND source_type=0 AND id=7 AND link=7;
DELETE FROM smart_scripts WHERE entryorguid=26812 AND source_type=0 AND id=7 AND link=7;
DELETE FROM smart_scripts WHERE entryorguid=26917 AND source_type=0 AND id=1 AND link=1;
DELETE FROM smart_scripts WHERE entryorguid=33044 AND source_type=0 AND id=1 AND link=1;
DELETE FROM smart_scripts WHERE entryorguid=38017 AND source_type=0 AND id=1 AND link=1;
DELETE FROM smart_scripts WHERE entryorguid=38923 AND source_type=0 AND id=1 AND link=1;
DELETE FROM smart_scripts WHERE entryorguid=101846 AND source_type=0 AND id=18 AND link=18;

-- ========== 13b: SmartAI min/max — c'est event_param3/param4 (repeat timers) ==========
-- 97087 id=2: event_param3=5000 > event_param4=4300 -> swap
UPDATE smart_scripts SET event_param3=4300, event_param4=5000 WHERE entryorguid=97087 AND source_type=0 AND id=2 AND event_param3=5000 AND event_param4=4300;
-- 105038 id=0: event_param3=120000 > event_param4=12000 -> swap
UPDATE smart_scripts SET event_param3=12000, event_param4=120000 WHERE entryorguid=105038 AND source_type=0 AND id=0 AND event_param3=120000 AND event_param4=12000;
-- Restaurer event_param1/param2 de 105038 (le batch 2b les a mis a 12000/120000 par erreur)
-- Actuellement 0/0 dans la DB, l'event_type=60 (SMART_EVENT_UPDATE) n'utilise pas param1/2
-- 77893 id=0: déjà fixé (1/1 OK), 115751: déjà fixé (2000/6000 OK)

-- ========== 13c: SmartAI creature GUIDs — source_type=0 pas 1 ==========
DELETE FROM smart_scripts WHERE entryorguid IN (-25354080,-25354082) AND source_type=0;

-- ========== 13d: SmartAI maxDist=0 as target_param1 ==========
-- action_type 49 = SMART_ACTION_MELEE_ATTACK (needs maxDist), 75 = SMART_ACTION_ATTACK
-- target_param1=0 pour maxDist est invalide; mettre une valeur par défaut raisonnable (5)
UPDATE smart_scripts SET target_param1=5 WHERE entryorguid=22337 AND source_type=0 AND id=2 AND action_type=49 AND target_param1=0;
UPDATE smart_scripts SET target_param1=5 WHERE entryorguid=4779000 AND source_type=9 AND id=6 AND action_type=49 AND target_param1=0;
UPDATE smart_scripts SET target_param1=5 WHERE entryorguid=4808000 AND source_type=9 AND id=22 AND action_type=49 AND target_param1=0;
UPDATE smart_scripts SET target_param1=5 WHERE entryorguid=93838 AND source_type=0 AND id=4 AND action_type=75 AND target_param1=0;
UPDATE smart_scripts SET target_param1=5 WHERE entryorguid=93839 AND source_type=0 AND id=4 AND action_type=75 AND target_param1=0;

-- ========== 13e: SmartAI non-existent Item 133885 source_type=9 ==========
DELETE FROM smart_scripts WHERE entryorguid=9786301 AND source_type=9 AND id=2 AND action_param1=133885;

-- ========== 13f: skill_discovery_template — retirer les entrées chance=0 (créent nouvelle erreur) ==========
DELETE FROM skill_discovery_template WHERE spellId IN (131593,131686,131688,131690,131691,131695,131759) AND chance=100;
DELETE FROM skill_discovery_template WHERE spellId IN (138878,138879,138880,138881,138890,138891,138892,138893) AND chance=0 AND reqSpell=0;
-- Les spells 131xxx/138xxx sont des erreurs DBC (spell marqué discovery dans le DBC).
-- On ne peut pas les corriger côté DB. Supprimer les entrées qu'on a ajoutées.

-- ========== 13g: Quest objectives — DELETE au lieu de ObjectID=0 ==========
DELETE FROM quest_objectives WHERE QuestID=25139 AND ID=265173 AND ObjectID=0;
DELETE FROM quest_objectives WHERE QuestID=34360 AND ID=286350 AND ObjectID=0;

-- ========== 13h: Condition effectMask 181293 group 3 -> séparer en 1 et 2 ==========
-- group 3 = bit1 + bit2. Les conditions sur group 1 (bit1) et group 2 (bit2) existent déjà.
-- group 3 contient creature 90441 qui doit être appliqué aux effects 0 ET 1
DELETE FROM conditions WHERE SourceTypeOrReferenceId=13 AND SourceEntry=181293 AND SourceGroup=3;
INSERT INTO conditions VALUES
(13,1,181293,0,0,31,0,3,90441,0,0,0,'','effectMask fix - was group 3'),
(13,2,181293,0,0,31,0,3,90441,0,0,0,'','effectMask fix - was group 3');

-- ========== 13i: Condition effectMask 213704 SourceGroup=0 (invalide) ==========
-- SourceGroup=0 = pas d'effect mask = invalide pour SourceType 13 (spell condition)
-- Remplacer par SourceGroup=1 (effect 0) qui est le plus courant
UPDATE conditions SET SourceGroup=1 WHERE SourceTypeOrReferenceId=13 AND SourceEntry=213704 AND SourceGroup=0;

-- ========== 13j: item_loot_template currency 392 inexistante ==========
DELETE FROM item_loot_template WHERE Entry=119000 AND Item=0 AND Currency=392;

-- ========== 13k: "Not handled grouped condition, SourceGroup 3" ==========
-- Ceci vient de SourceType 22 (CONDITION_SOURCE_TYPE_VEHICLE_SPELL) avec SourceGroup=3
-- C'est un warning du core pour un type de condition non supporté. Cosmétique.
-- Pas de fix DB sûr (supprimer casserait les conditions de vehicules).

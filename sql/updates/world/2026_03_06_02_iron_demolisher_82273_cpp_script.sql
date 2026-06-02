-- Iron Demolisher NPC 82273 — passage au script C++ npc_iron_demolisher
--
-- Le script C++ gère :
--   - Timer canon (EVENT_CANNON_FIRE, 6-10s, pos fixe par GUID)
--   - OnSpellClick → explosion spell 171950 + mort après 3s (animation native modèle 53699)
--   - Flags UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE (cannon passif)
--
-- Suppression de tous les smart_scripts (plus besoin)
-- AIName='' car ScriptName prend le dessus
-- ScriptName = npc_iron_demolisher

-- =====================================================
-- 1. Suppression des smart_scripts devenus inutiles
-- =====================================================

-- GUID scripts (timer canon)
DELETE FROM smart_scripts
WHERE entryorguid IN (-254972,-254990,-254991,-254992,-254993)
  AND source_type=0;

-- Entry script (SpellClick → action list)
DELETE FROM smart_scripts
WHERE entryorguid=82273 AND source_type=0;

-- Action list (explosion + mort)
DELETE FROM smart_scripts
WHERE entryorguid=8227300 AND source_type=9;

-- =====================================================
-- 2. Brancher le script C++ sur la créature
-- =====================================================

UPDATE creature_template
SET AIName    = '',
    ScriptName = 'npc_iron_demolisher'
WHERE entry = 82273;

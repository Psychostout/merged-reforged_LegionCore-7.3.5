-- Spell 171950 "Cannon Blast" — Iron Demolisher SmartAI
-- Enregistre le SpellScript C++ qui réduit les dégâts de 90%.
-- La classe C++ est dans src/server/scripts/Spells/spell_generic.cpp.

DELETE FROM spell_script_names WHERE spell_id = 171950;
INSERT INTO spell_script_names (spell_id, ScriptName) VALUES (171950, 'spell_iron_demolisher_cannon_blast');

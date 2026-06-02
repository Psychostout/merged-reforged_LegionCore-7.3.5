-- Fix visual missile travel speed for spell 171950 (Cannon Blast, Iron Demolisher NPC 82273)
-- DB2 chain: Spell 171950 → SpellXSpellVisual → SpellVisualID=69633
-- Without this entry, Unit::SendSpellCreateVisual() returns early → no PlaySpellVisual packet
-- → client animates missile using BaseMissileSpeed=0.5 from DB2 → extremely slow
-- TravelSpeed=30.0 matches the server-side SetSpeed(30.0f) in spell_generic.cpp SpellScript
INSERT INTO world_legion.spell_visual
  (spellId, SpellVisualID, TravelSpeed, SpeedAsTime, MissReason, ReflectStatus, type, temp, HasPosition, comment)
VALUES
  (171950, 69633, 30.0, 0, 0, 0, 0, 0, 1, 'Iron Demolisher cannon blast visual travel speed fix')
ON DUPLICATE KEY UPDATE
  SpellVisualID = 69633,
  TravelSpeed   = 30.0,
  HasPosition   = 1;

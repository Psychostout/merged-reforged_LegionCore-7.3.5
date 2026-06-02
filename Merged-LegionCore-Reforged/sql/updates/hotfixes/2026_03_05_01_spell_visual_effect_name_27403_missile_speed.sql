-- Fix visual missile speed for spell 171950 (Cannon Blast, Iron Demolisher NPC 82273)
-- DB2 chain: Spell 171950 → SpellXSpellVisual → SpellVisualID=69633
--            → SpellVisualMissileSetID=8855 → SpellVisualMissile.ID=8855
--            → SpellVisualEffectNameID=27403
-- Current BaseMissileSpeed = 0.5 (extremely slow, missile appears frozen)
-- New BaseMissileSpeed = 30.0 (matches server-side SetSpeed(30.0f) in spell_generic.cpp)
INSERT INTO hotfixes_legion.spell_visual_effect_name
  (ID, ModelFileDataID, EffectRadius, BaseMissileSpeed, Scale,
   MinAllowedScale, MaxAllowedScale, Alpha, Flags, Type,
   GenericID, TextureFileDataID, RibbonQualityID, DissolveEffectID, Unknown13, VerifiedBuild)
VALUES
  (27403, 0, 0.0, 30.0, 0.25,
   0.75, 1.0, 0, 0, 0,
   0, 1327489, 0, 0, 0, 26972)
ON DUPLICATE KEY UPDATE
  BaseMissileSpeed = 30.0,
  VerifiedBuild    = 26972;

-- hotfix_data: needed so the server sends this hotfix to connecting clients
-- SpellVisualEffectName table_hash = 0x02E18F32 = 48336690
INSERT INTO hotfixes_legion.hotfix_data (Id, TableHash, RecordID, Timestamp, Deleted)
VALUES (9000215, 48336690, 27403, 0, 0)
ON DUPLICATE KEY UPDATE Deleted=0;

-- Spell 171950 (Iron Demolisher Cannon Blast) — Vitesse missile
--
-- Problème : Speed=0.0 dans SpellMisc.db2 (impact instantané côté serveur).
--   Le client animait le projectile à sa propre vitesse cosmétique (non synchronisée).
--   Notre précédent ApplySpellFix(Speed=30) ne modifiait que le timing serveur →
--   le client utilisait toujours Speed=0 depuis son cache DB2 local →
--   explosion avant que le projectile ne touche visuellement le sol.
--
-- Fix : hotfix spell_misc ID=110729 (SpellMisc record principal de spell 171950,
--   DifficultyID=0). Le mécanisme hotfix envoie ce changement au client via
--   SMSG_DB_REPLY (manifest via hotfix_data) → client et serveur utilisent Speed=40.0
--   → projectile visuel et explosion synchronisés. TravelTime ≈ 75y / 40 = 1.875s.
--
-- Données champs tirées du parse WDC1 de SpellMisc.db2 (build 26972) :
--   CastingTimeIndex=1, DurationIndex=0, RangeIndex=74, SchoolMask=1 (Physical),
--   SpellIconFileDataID=136243, ActiveIconFileDataID=0, LaunchDelay=0.0,
--   Attributes1=0x09000180, Attributes2=0x00000400, Attributes3=0x10480080,
--   Attributes4=0x00030200, Attributes5=0x00800000, Attributes6=0x00060008,
--   Attributes7=0x00000200, Attributes8-14=0.
--
-- Note : ApplySpellFix({171950}, Speed=30) supprimé de SpellMgr.cpp (ce hotfix
--   le remplace et le serveur chargerait la DB2/hotfix AVANT LoadSpellCustomAttr,
--   ApplySpellFix aurait overridé ce hotfix côté serveur à 30.0f au lieu de 20.0f).
--
-- SpellMisc.db2 TableHash = 0x0002BFFE (180222) — lu depuis le header WDC1.
-- L'entrée hotfix_data est requise pour que le CLIENT reçoive la vitesse mise à jour
-- via SendHotfixList → HandleDBQueryBulk → SMSG_DB_REPLY.
-- Sans hotfix_data, le serveur utilise Speed=40 mais le client garde Speed=0 (DB2 local)
-- → missile visuel toujours instantané côté client.

INSERT INTO spell_misc
    (ID, CastingTimeIndex, DurationIndex, RangeIndex, SchoolMask, SpellIconFileDataID,
     Speed, ActiveIconFileDataID, LaunchDelay, DifficultyID,
     Attributes1, Attributes2, Attributes3, Attributes4, Attributes5, Attributes6, Attributes7,
     Attributes8, Attributes9, Attributes10, Attributes11, Attributes12, Attributes13, Attributes14,
     SpellID, VerifiedBuild)
VALUES
    (110729, 1, 0, 74, 1, 136243,
     40.0, 0, 0.0, 0,
     0x09000180, 0x00000400, 0x10480080, 0x00030200, 0x00800000, 0x00060008, 0x00000200,
     0, 0, 0, 0, 0, 0, 0,
     171950, 26972)
ON DUPLICATE KEY UPDATE
    Speed = 40.0,
    VerifiedBuild = 26972;

-- Manifest client : informe le client que SpellMisc record 110729 a changé.
-- TableHash 3322146344 (0xC603EE28) = SpellMisc.db2 (lu à l'offset 20 du header WDC1,
-- build 26972). NE PAS confondre avec RecordCount (offset 4 = 180222).
-- Confirmé : 225 entrées SpellMisc existantes utilisent déjà ce TableHash.
INSERT INTO hotfix_data (Id, TableHash, RecordId, Deleted)
VALUES (9000214, 3322146344, 110729, 0)
ON DUPLICATE KEY UPDATE TableHash=3322146344, RecordId=110729, Deleted=0;

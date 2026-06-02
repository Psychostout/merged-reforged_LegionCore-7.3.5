-- Fix double-encoded UTF-8 in creature/item/quest names
-- Cause : noms importés depuis locales ptBR/frFR avec double-encodage UTF-8
-- (ex: "é" = U+00E9 = C3A9 en UTF-8, mais stocké comme C383C2A9 = Ã©)
-- Fix   : CONVERT(CONVERT(col USING latin1) USING utf8mb4)
--   → réinterprète les bytes UTF-8 comme Latin-1, puis re-encode en UTF-8 correct

-- creature_template_wdb (14 entrées)
UPDATE creature_template_wdb
SET
  Name1    = CONVERT(CONVERT(Name1    USING latin1) USING utf8mb4),
  Name2    = CONVERT(CONVERT(Name2    USING latin1) USING utf8mb4),
  Name3    = CONVERT(CONVERT(Name3    USING latin1) USING utf8mb4),
  Name4    = CONVERT(CONVERT(Name4    USING latin1) USING utf8mb4),
  NameAlt1 = CONVERT(CONVERT(NameAlt1 USING latin1) USING utf8mb4),
  NameAlt2 = CONVERT(CONVERT(NameAlt2 USING latin1) USING utf8mb4),
  NameAlt3 = CONVERT(CONVERT(NameAlt3 USING latin1) USING utf8mb4),
  NameAlt4 = CONVERT(CONVERT(NameAlt4 USING latin1) USING utf8mb4),
  Title    = CONVERT(CONVERT(Title    USING latin1) USING utf8mb4),
  TitleAlt = CONVERT(CONVERT(TitleAlt USING latin1) USING utf8mb4)
WHERE LENGTH(Name1) != CHAR_LENGTH(Name1)
   OR LENGTH(Name2) != CHAR_LENGTH(Name2)
   OR LENGTH(Title)  != CHAR_LENGTH(Title);

-- item_template (18 entrées)
UPDATE item_template
SET
  name        = CONVERT(CONVERT(name        USING latin1) USING utf8mb4),
  description = CONVERT(CONVERT(description USING latin1) USING utf8mb4)
WHERE LENGTH(name) != CHAR_LENGTH(name)
   OR LENGTH(description) != CHAR_LENGTH(description);

-- quest_template (91 entrées)
UPDATE quest_template
SET
  LogTitle       = CONVERT(CONVERT(LogTitle       USING latin1) USING utf8mb4),
  LogDescription = CONVERT(CONVERT(LogDescription USING latin1) USING utf8mb4),
  AreaDescription= CONVERT(CONVERT(AreaDescription USING latin1) USING utf8mb4),
  PortraitGiverName = CONVERT(CONVERT(PortraitGiverName USING latin1) USING utf8mb4),
  PortraitGiverText = CONVERT(CONVERT(PortraitGiverText USING latin1) USING utf8mb4)
WHERE LENGTH(LogTitle) != CHAR_LENGTH(LogTitle)
   OR LENGTH(LogDescription) != CHAR_LENGTH(LogDescription);

-- character_service_info data from CharacterServiceInfo.db2 (frFR, WoW 7.3.5)
-- Generated from DB2 file: build/bin/Release/ClientData/dbc/frFR/CharacterServiceInfo.db2

SET NAMES utf8mb4;

-- Main table
DELETE FROM `character_service_info` WHERE ID IN (1, 6);
INSERT INTO `character_service_info` (ID, FlowTitle, PopupTitle, PopupDescription, BoostType, IconFileDataID, Priority, Flags, ProfessionLevel, BoostLevel, Expansion, PopupUITextureKitID, VerifiedBuild) VALUES (1, 'Sésame pour le niveau 100', 'Sésame gratuit pour le niveau 100', 'World of Warcraft: Legion comprend un Sésame pour le niveau 100. Vous pouvez choisir de propulser au niveau 100 un personnage existant, essayer une nouvelle classe avant d’utiliser votre Sésame, ou fermer la fenêtre pour utiliser le Sésame plus tard.', 2, 1033987, 20, 1, 700, 100, 6, 260, 0);
INSERT INTO `character_service_info` (ID, FlowTitle, PopupTitle, PopupDescription, BoostType, IconFileDataID, Priority, Flags, ProfessionLevel, BoostLevel, Expansion, PopupUITextureKitID, VerifiedBuild) VALUES (6, 'Sésame pour le niveau 90', 'Sésame gratuit pour le niveau 90', 'Vous avez obtenu ce sésame en cadeau pour l’achat de Warlords of Draenor.', 1, 614740, 30, 1, 625, 90, 5, 260, 0);

-- Locale table (frFR)
DELETE FROM `character_service_info_locale` WHERE ID IN (1, 6);
INSERT INTO `character_service_info_locale` (ID, FlowTitle_lang, PopupTitle_lang, PopupDescription_lang, locale, VerifiedBuild) VALUES (1, 'Sésame pour le niveau 100', 'Sésame gratuit pour le niveau 100', 'World of Warcraft: Legion comprend un Sésame pour le niveau 100. Vous pouvez choisir de propulser au niveau 100 un personnage existant, essayer une nouvelle classe avant d’utiliser votre Sésame, ou fermer la fenêtre pour utiliser le Sésame plus tard.', 'frFR', 0);
INSERT INTO `character_service_info_locale` (ID, FlowTitle_lang, PopupTitle_lang, PopupDescription_lang, locale, VerifiedBuild) VALUES (6, 'Sésame pour le niveau 90', 'Sésame gratuit pour le niveau 90', 'Vous avez obtenu ce sésame en cadeau pour l’achat de Warlords of Draenor.', 'frFR', 0);

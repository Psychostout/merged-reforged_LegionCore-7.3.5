-- ==========================================================================
-- Demon Invasion Prepatch - Phase 4: Nethershards, Vendor, Achievements
-- ==========================================================================

-- ========================================
-- 1. Nethershard item (ID 900001) — hotfix tables
-- ========================================
-- TableHash for Item: 4030871717
-- TableHash for ItemSparse: 2442913102

-- item table (base item data)
DELETE FROM `hotfixes_legion`.`item` WHERE `ID` = 900001;
INSERT INTO `hotfixes_legion`.`item` (`ID`, `IconFileDataID`, `ClassID`, `SubclassID`, `SoundOverrideSubclass`, `Material`, `InventoryType`, `SheatheType`, `ItemGroupSoundsID`, `VerifiedBuild`) VALUES
(900001, 1380306, 15, 0, -1, 0, 0, 0, 0, 1);
-- ClassID 15 = Miscellaneous, SubclassID 0 = Junk, InventoryType 0 = Non-equippable
-- IconFileDataID 1380306 = crystal/shard icon (Legion)

-- item_sparse table (full item data)
DELETE FROM `hotfixes_legion`.`item_sparse` WHERE `ID` = 900001;
INSERT INTO `hotfixes_legion`.`item_sparse` (`ID`, `AllowableRace`, `Display`, `Display1`, `Display2`, `Display3`, `Description`,
    `Flags1`, `Flags2`, `Flags3`, `Flags4`,
    `PriceRandomValue`, `PriceVariance`, `VendorStackCount`, `BuyPrice`, `SellPrice`,
    `RequiredAbility`, `MaxCount`, `Stackable`,
    `StatPercentEditor1`, `StatPercentEditor2`, `StatPercentEditor3`, `StatPercentEditor4`, `StatPercentEditor5`,
    `StatPercentEditor6`, `StatPercentEditor7`, `StatPercentEditor8`, `StatPercentEditor9`, `StatPercentEditor10`,
    `StatPercentageOfSocket1`, `StatPercentageOfSocket2`, `StatPercentageOfSocket3`, `StatPercentageOfSocket4`, `StatPercentageOfSocket5`,
    `StatPercentageOfSocket6`, `StatPercentageOfSocket7`, `StatPercentageOfSocket8`, `StatPercentageOfSocket9`, `StatPercentageOfSocket10`,
    `ItemRange`, `BagFamily`, `QualityModifier`, `DurationInInventory`, `DmgVariance`,
    `AllowableClass`, `ItemLevel`, `RequiredSkill`, `RequiredSkillRank`, `MinFactionID`,
    `ItemStatValue1`, `ItemStatValue2`, `ItemStatValue3`, `ItemStatValue4`, `ItemStatValue5`,
    `ItemStatValue6`, `ItemStatValue7`, `ItemStatValue8`, `ItemStatValue9`, `ItemStatValue10`,
    `ScalingStatDistributionID`, `ItemDelay`, `PageID`, `StartQuestID`, `LockID`,
    `RandomSelect`, `ItemRandomSuffixGroupID`, `ItemSet`,
    `ZoneBound`, `InstanceBound`, `TotemCategoryID`, `SocketMatch_enchantment_id`, `GemProperties`,
    `LimitCategory`, `RequiredHoliday`, `RequiredTransmogHoliday`, `ItemNameDescriptionID`,
    `OverallQualityID`, `InventoryType`, `RequiredLevel`, `RequiredPVPRank`, `RequiredPVPMedal`,
    `MinReputation`, `ContainerSlots`,
    `StatModifierBonusStat1`, `StatModifierBonusStat2`, `StatModifierBonusStat3`, `StatModifierBonusStat4`, `StatModifierBonusStat5`,
    `StatModifierBonusStat6`, `StatModifierBonusStat7`, `StatModifierBonusStat8`, `StatModifierBonusStat9`, `StatModifierBonusStat10`,
    `DamageDamageType`, `Bonding`, `LanguageID`, `PageMaterialID`, `Material`, `SheatheType`,
    `SocketType1`, `SocketType2`, `SocketType3`, `SpellWeightCategory`, `SpellWeight`,
    `ArtifactID`, `ExpansionID`, `VerifiedBuild`) VALUES
(900001, -1, 'Nethershard', '', '', '', 'A crystallized fragment of Nether energy, dropped by demons during the Legion invasion.',
    0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 2000,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    -1, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0,
    0, 0, 0, 0, 0,
    0, 0, 0, 0,
    3, 0, 0, 0, 0,
    0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0,
    0, 6, 1);
-- OverallQualityID 3 = Rare (blue), Stackable 2000, Bonding 0 = no bind, ExpansionID 6 = Legion

-- item_search_name (for search functionality)
DELETE FROM `hotfixes_legion`.`item_search_name` WHERE `ID` = 900001;
INSERT IGNORE INTO `hotfixes_legion`.`item_search_name` (`ID`, `Display`, `OverallQualityID`, `ExpansionID`, `MinFactionID`, `MinReputation`, `AllowableClass`, `AllowableRace`, `RequiredLevel`, `RequiredSkill`, `RequiredSkillRank`, `RequiredAbility`, `ItemLevel`, `Flags1`, `Flags2`, `Flags3`, `VerifiedBuild`) VALUES
(900001, 'Nethershard', 3, 6, 0, 0, -1, -1, 0, 0, 0, 0, 1, 0, 0, 0, 1);

-- hotfix_data entries so the server processes these custom items
-- Using Id range 9000001+ to avoid conflicts
DELETE FROM `hotfixes_legion`.`hotfix_data` WHERE `RecordID` = 900001 AND `TableHash` IN (4030871717, 2442913102);
INSERT INTO `hotfixes_legion`.`hotfix_data` (`Id`, `TableHash`, `RecordID`, `Timestamp`, `Deleted`) VALUES
(9000001, 4030871717, 900001, UNIX_TIMESTAMP(), 0),   -- Item table
(9000002, 2442913102, 900001, UNIX_TIMESTAMP(), 0);    -- ItemSparse table

-- ========================================
-- 2. Vendor items (IDs 900101-900111)
-- ========================================

-- Vendor equipment items (ilvl 700)
DELETE FROM `hotfixes_legion`.`item` WHERE `ID` BETWEEN 900101 AND 900111;
INSERT INTO `hotfixes_legion`.`item` (`ID`, `IconFileDataID`, `ClassID`, `SubclassID`, `SoundOverrideSubclass`, `Material`, `InventoryType`, `SheatheType`, `ItemGroupSoundsID`, `VerifiedBuild`) VALUES
-- Transmog sets (armor tokens — non-equippable tokens for simplicity)
(900101, 132741, 15, 0, -1, 7, 0, 0, 0, 1),   -- Cloth set token
(900102, 132742, 15, 0, -1, 8, 0, 0, 0, 1),   -- Leather set token
(900103, 132743, 15, 0, -1, 5, 0, 0, 0, 1),   -- Mail set token
(900104, 132744, 15, 0, -1, 6, 0, 0, 0, 1),   -- Plate set token
-- Equipment (ilvl 700 accessories)
(900105, 133770, 4, 0, -1, 7, 16, 0, 0, 1),   -- Cloak (InventoryType 16=Cloak, Class 4=Armor)
(900106, 133345, 4, 0, -1, 0, 11, 0, 0, 1),   -- Ring (InventoryType 11=Finger)
(900107, 133346, 4, 0, -1, 0, 2, 0, 0, 1),    -- Neck (InventoryType 2=Neck)
(900108, 133347, 4, 0, -1, 0, 12, 0, 0, 1),   -- Trinket (InventoryType 12=Trinket)
-- Consumables
(900109, 237268, 0, 0, -1, 0, 0, 0, 0, 1),    -- Felstone (Class 0=Consumable)
(900110, 237265, 0, 0, -1, 0, 0, 0, 0, 1),    -- Protection Potion
(900111, 237266, 0, 0, -1, 7, 0, 0, 0, 1);    -- Combat Bandage

DELETE FROM `hotfixes_legion`.`item_sparse` WHERE `ID` BETWEEN 900101 AND 900111;
-- Simplified inserts for vendor items — only key fields differ, rest are defaults
-- Transmog tokens
INSERT INTO `hotfixes_legion`.`item_sparse` (`ID`, `AllowableRace`, `Display`, `Description`,
    `Flags1`, `Flags2`, `Flags3`, `Flags4`, `PriceRandomValue`, `PriceVariance`, `VendorStackCount`, `BuyPrice`, `SellPrice`,
    `RequiredAbility`, `MaxCount`, `Stackable`,
    `StatPercentEditor1`, `StatPercentEditor2`, `StatPercentEditor3`, `StatPercentEditor4`, `StatPercentEditor5`,
    `StatPercentEditor6`, `StatPercentEditor7`, `StatPercentEditor8`, `StatPercentEditor9`, `StatPercentEditor10`,
    `StatPercentageOfSocket1`, `StatPercentageOfSocket2`, `StatPercentageOfSocket3`, `StatPercentageOfSocket4`, `StatPercentageOfSocket5`,
    `StatPercentageOfSocket6`, `StatPercentageOfSocket7`, `StatPercentageOfSocket8`, `StatPercentageOfSocket9`, `StatPercentageOfSocket10`,
    `ItemRange`, `BagFamily`, `QualityModifier`, `DurationInInventory`, `DmgVariance`,
    `AllowableClass`, `ItemLevel`, `RequiredSkill`, `RequiredSkillRank`, `MinFactionID`,
    `ItemStatValue1`, `ItemStatValue2`, `ItemStatValue3`, `ItemStatValue4`, `ItemStatValue5`,
    `ItemStatValue6`, `ItemStatValue7`, `ItemStatValue8`, `ItemStatValue9`, `ItemStatValue10`,
    `ScalingStatDistributionID`, `ItemDelay`, `PageID`, `StartQuestID`, `LockID`,
    `RandomSelect`, `ItemRandomSuffixGroupID`, `ItemSet`,
    `ZoneBound`, `InstanceBound`, `TotemCategoryID`, `SocketMatch_enchantment_id`, `GemProperties`,
    `LimitCategory`, `RequiredHoliday`, `RequiredTransmogHoliday`, `ItemNameDescriptionID`,
    `OverallQualityID`, `InventoryType`, `RequiredLevel`, `RequiredPVPRank`, `RequiredPVPMedal`,
    `MinReputation`, `ContainerSlots`,
    `StatModifierBonusStat1`, `StatModifierBonusStat2`, `StatModifierBonusStat3`, `StatModifierBonusStat4`, `StatModifierBonusStat5`,
    `StatModifierBonusStat6`, `StatModifierBonusStat7`, `StatModifierBonusStat8`, `StatModifierBonusStat9`, `StatModifierBonusStat10`,
    `DamageDamageType`, `Bonding`, `LanguageID`, `PageMaterialID`, `Material`, `SheatheType`,
    `SocketType1`, `SocketType2`, `SocketType3`, `SpellWeightCategory`, `SpellWeight`,
    `ArtifactID`, `ExpansionID`, `VerifiedBuild`) VALUES
(900101, -1, 'Fel-Touched Cloth Set', 'A set of fel-infused cloth armor for transmogrification.',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,
    -1, 1, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,
    3, 0, 0, 0, 0, 0, 0,
    0,0,0,0,0,0,0,0,0,0, 0, 1, 0, 0, 7, 0, 0,0,0, 0,0, 0, 6, 1),
(900102, -1, 'Fel-Touched Leather Set', 'A set of fel-infused leather armor for transmogrification.',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,
    -1, 1, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,
    3, 0, 0, 0, 0, 0, 0,
    0,0,0,0,0,0,0,0,0,0, 0, 1, 0, 0, 8, 0, 0,0,0, 0,0, 0, 6, 1),
(900103, -1, 'Fel-Touched Mail Set', 'A set of fel-infused mail armor for transmogrification.',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,
    -1, 1, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,
    3, 0, 0, 0, 0, 0, 0,
    0,0,0,0,0,0,0,0,0,0, 0, 1, 0, 0, 5, 0, 0,0,0, 0,0, 0, 6, 1),
(900104, -1, 'Fel-Touched Plate Set', 'A set of fel-infused plate armor for transmogrification.',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,
    -1, 1, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,
    3, 0, 0, 0, 0, 0, 0,
    0,0,0,0,0,0,0,0,0,0, 0, 1, 0, 0, 6, 0, 0,0,0, 0,0, 0, 6, 1);
-- Equipment (ilvl 700)
INSERT INTO `hotfixes_legion`.`item_sparse` (`ID`, `AllowableRace`, `Display`, `Description`,
    `Flags1`, `Flags2`, `Flags3`, `Flags4`, `PriceRandomValue`, `PriceVariance`, `VendorStackCount`, `BuyPrice`, `SellPrice`,
    `RequiredAbility`, `MaxCount`, `Stackable`,
    `StatPercentEditor1`, `StatPercentEditor2`, `StatPercentEditor3`, `StatPercentEditor4`, `StatPercentEditor5`,
    `StatPercentEditor6`, `StatPercentEditor7`, `StatPercentEditor8`, `StatPercentEditor9`, `StatPercentEditor10`,
    `StatPercentageOfSocket1`, `StatPercentageOfSocket2`, `StatPercentageOfSocket3`, `StatPercentageOfSocket4`, `StatPercentageOfSocket5`,
    `StatPercentageOfSocket6`, `StatPercentageOfSocket7`, `StatPercentageOfSocket8`, `StatPercentageOfSocket9`, `StatPercentageOfSocket10`,
    `ItemRange`, `BagFamily`, `QualityModifier`, `DurationInInventory`, `DmgVariance`,
    `AllowableClass`, `ItemLevel`, `RequiredSkill`, `RequiredSkillRank`, `MinFactionID`,
    `ItemStatValue1`, `ItemStatValue2`, `ItemStatValue3`, `ItemStatValue4`, `ItemStatValue5`,
    `ItemStatValue6`, `ItemStatValue7`, `ItemStatValue8`, `ItemStatValue9`, `ItemStatValue10`,
    `ScalingStatDistributionID`, `ItemDelay`, `PageID`, `StartQuestID`, `LockID`,
    `RandomSelect`, `ItemRandomSuffixGroupID`, `ItemSet`,
    `ZoneBound`, `InstanceBound`, `TotemCategoryID`, `SocketMatch_enchantment_id`, `GemProperties`,
    `LimitCategory`, `RequiredHoliday`, `RequiredTransmogHoliday`, `ItemNameDescriptionID`,
    `OverallQualityID`, `InventoryType`, `RequiredLevel`, `RequiredPVPRank`, `RequiredPVPMedal`,
    `MinReputation`, `ContainerSlots`,
    `StatModifierBonusStat1`, `StatModifierBonusStat2`, `StatModifierBonusStat3`, `StatModifierBonusStat4`, `StatModifierBonusStat5`,
    `StatModifierBonusStat6`, `StatModifierBonusStat7`, `StatModifierBonusStat8`, `StatModifierBonusStat9`, `StatModifierBonusStat10`,
    `DamageDamageType`, `Bonding`, `LanguageID`, `PageMaterialID`, `Material`, `SheatheType`,
    `SocketType1`, `SocketType2`, `SocketType3`, `SpellWeightCategory`, `SpellWeight`,
    `ArtifactID`, `ExpansionID`, `VerifiedBuild`) VALUES
(900105, -1, 'Demon Commander\'s Cloak', 'Torn from the back of a defeated demon commander.',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    5333, 6666, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,
    -1, 700, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,
    3, 16, 98, 0, 0, 0, 0,
    32, 40, 0,0,0,0,0,0,0,0, 0, 2, 0, 0, 7, 0, 0,0,0, 0,0, 0, 6, 1),
-- StatModifierBonusStat1=32(Crit), StatModifierBonusStat2=40(Versatility)
(900106, -1, 'Infernal Signet', 'A ring forged in felfire.',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    5333, 6666, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,
    -1, 700, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,
    3, 11, 98, 0, 0, 0, 0,
    32, 36, 0,0,0,0,0,0,0,0, 0, 2, 0, 0, 0, 0, 0,0,0, 0,0, 0, 6, 1),
-- StatModifierBonusStat1=32(Crit), StatModifierBonusStat2=36(Haste)
(900107, -1, 'Felstalker\'s Pendant', 'Ripped from the neck of a felstalker alpha.',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    5333, 6666, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,
    -1, 700, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,
    3, 2, 98, 0, 0, 0, 0,
    49, 40, 0,0,0,0,0,0,0,0, 0, 2, 0, 0, 0, 0, 0,0,0, 0,0, 0, 6, 1),
-- StatModifierBonusStat1=49(Mastery), StatModifierBonusStat2=40(Versatility)
(900108, -1, 'Legion Doomguard Trinket', 'A dark trinket pulsing with demonic energy.',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    5333, 0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,
    -1, 700, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,
    3, 12, 98, 0, 0, 0, 0,
    32, 0, 0,0,0,0,0,0,0,0, 0, 2, 0, 0, 0, 0, 0,0,0, 0,0, 0, 6, 1);
-- Consumables
INSERT INTO `hotfixes_legion`.`item_sparse` (`ID`, `AllowableRace`, `Display`, `Description`,
    `Flags1`, `Flags2`, `Flags3`, `Flags4`, `PriceRandomValue`, `PriceVariance`, `VendorStackCount`, `BuyPrice`, `SellPrice`,
    `RequiredAbility`, `MaxCount`, `Stackable`,
    `StatPercentEditor1`, `StatPercentEditor2`, `StatPercentEditor3`, `StatPercentEditor4`, `StatPercentEditor5`,
    `StatPercentEditor6`, `StatPercentEditor7`, `StatPercentEditor8`, `StatPercentEditor9`, `StatPercentEditor10`,
    `StatPercentageOfSocket1`, `StatPercentageOfSocket2`, `StatPercentageOfSocket3`, `StatPercentageOfSocket4`, `StatPercentageOfSocket5`,
    `StatPercentageOfSocket6`, `StatPercentageOfSocket7`, `StatPercentageOfSocket8`, `StatPercentageOfSocket9`, `StatPercentageOfSocket10`,
    `ItemRange`, `BagFamily`, `QualityModifier`, `DurationInInventory`, `DmgVariance`,
    `AllowableClass`, `ItemLevel`, `RequiredSkill`, `RequiredSkillRank`, `MinFactionID`,
    `ItemStatValue1`, `ItemStatValue2`, `ItemStatValue3`, `ItemStatValue4`, `ItemStatValue5`,
    `ItemStatValue6`, `ItemStatValue7`, `ItemStatValue8`, `ItemStatValue9`, `ItemStatValue10`,
    `ScalingStatDistributionID`, `ItemDelay`, `PageID`, `StartQuestID`, `LockID`,
    `RandomSelect`, `ItemRandomSuffixGroupID`, `ItemSet`,
    `ZoneBound`, `InstanceBound`, `TotemCategoryID`, `SocketMatch_enchantment_id`, `GemProperties`,
    `LimitCategory`, `RequiredHoliday`, `RequiredTransmogHoliday`, `ItemNameDescriptionID`,
    `OverallQualityID`, `InventoryType`, `RequiredLevel`, `RequiredPVPRank`, `RequiredPVPMedal`,
    `MinReputation`, `ContainerSlots`,
    `StatModifierBonusStat1`, `StatModifierBonusStat2`, `StatModifierBonusStat3`, `StatModifierBonusStat4`, `StatModifierBonusStat5`,
    `StatModifierBonusStat6`, `StatModifierBonusStat7`, `StatModifierBonusStat8`, `StatModifierBonusStat9`, `StatModifierBonusStat10`,
    `DamageDamageType`, `Bonding`, `LanguageID`, `PageMaterialID`, `Material`, `SheatheType`,
    `SocketType1`, `SocketType2`, `SocketType3`, `SpellWeightCategory`, `SpellWeight`,
    `ArtifactID`, `ExpansionID`, `VerifiedBuild`) VALUES
(900109, -1, 'Felstone', 'Use: Your attacks have a chance to deal additional Shadow damage for 30 min.',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,
    -1, 1, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,
    2, 0, 0, 0, 0, 0, 0,
    0,0,0,0,0,0,0,0,0,0, 0, 0, 0, 0, 0, 0, 0,0,0, 0,0, 0, 6, 1),
(900110, -1, 'Potion of Demonic Protection', 'Use: Reduces damage taken by 10% for 15 sec.',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,
    -1, 1, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,
    1, 0, 0, 0, 0, 0, 0,
    0,0,0,0,0,0,0,0,0,0, 0, 0, 0, 0, 0, 0, 0,0,0, 0,0, 0, 6, 1),
(900111, -1, 'Fel-Mended Bandage', 'Use: Heals the target for a moderate amount over 8 sec.',
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20,
    0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,
    -1, 1, 0, 0, 0, 0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0, 0,0,0, 0,0,0,0,0, 0,0,0,0,
    1, 0, 0, 0, 0, 0, 0,
    0,0,0,0,0,0,0,0,0,0, 0, 0, 0, 0, 7, 0, 0,0,0, 0,0, 0, 6, 1);

-- hotfix_data for vendor items
DELETE FROM `hotfixes_legion`.`hotfix_data` WHERE `RecordID` BETWEEN 900101 AND 900111 AND `TableHash` IN (4030871717, 2442913102);
INSERT INTO `hotfixes_legion`.`hotfix_data` (`Id`, `TableHash`, `RecordID`, `Timestamp`, `Deleted`) VALUES
-- Item table entries
(9000101, 4030871717, 900101, UNIX_TIMESTAMP(), 0),
(9000102, 4030871717, 900102, UNIX_TIMESTAMP(), 0),
(9000103, 4030871717, 900103, UNIX_TIMESTAMP(), 0),
(9000104, 4030871717, 900104, UNIX_TIMESTAMP(), 0),
(9000105, 4030871717, 900105, UNIX_TIMESTAMP(), 0),
(9000106, 4030871717, 900106, UNIX_TIMESTAMP(), 0),
(9000107, 4030871717, 900107, UNIX_TIMESTAMP(), 0),
(9000108, 4030871717, 900108, UNIX_TIMESTAMP(), 0),
(9000109, 4030871717, 900109, UNIX_TIMESTAMP(), 0),
(9000110, 4030871717, 900110, UNIX_TIMESTAMP(), 0),
(9000111, 4030871717, 900111, UNIX_TIMESTAMP(), 0),
-- ItemSparse table entries
(9000201, 2442913102, 900101, UNIX_TIMESTAMP(), 0),
(9000202, 2442913102, 900102, UNIX_TIMESTAMP(), 0),
(9000203, 2442913102, 900103, UNIX_TIMESTAMP(), 0),
(9000204, 2442913102, 900104, UNIX_TIMESTAMP(), 0),
(9000205, 2442913102, 900105, UNIX_TIMESTAMP(), 0),
(9000206, 2442913102, 900106, UNIX_TIMESTAMP(), 0),
(9000207, 2442913102, 900107, UNIX_TIMESTAMP(), 0),
(9000208, 2442913102, 900108, UNIX_TIMESTAMP(), 0),
(9000209, 2442913102, 900109, UNIX_TIMESTAMP(), 0),
(9000210, 2442913102, 900110, UNIX_TIMESTAMP(), 0),
(9000211, 2442913102, 900111, UNIX_TIMESTAMP(), 0);

-- ========================================
-- 3. Update vendor creature_template ScriptName
-- ========================================
UPDATE `creature_template` SET `ScriptName` = 'npc_demon_invasion_vendor', `npcflag` = 1 WHERE `entry` = 900050;
-- npcflag=1 (Gossip) instead of 128 (Vendor) since we use a gossip script, not npc_vendor

-- ========================================
-- 4. Achievement tracking table (characters DB)
-- ========================================
-- NOTE: Run this against the characters database (characters_legion)
-- CREATE TABLE IF NOT EXISTS `character_demon_invasion_progress` (
--     `guid` INT UNSIGNED NOT NULL,
--     `zone_index` TINYINT UNSIGNED NOT NULL,
--     `completed_time` INT UNSIGNED NOT NULL DEFAULT 0,
--     PRIMARY KEY (`guid`, `zone_index`)
-- ) ENGINE=InnoDB DEFAULT CHARSET=utf8;

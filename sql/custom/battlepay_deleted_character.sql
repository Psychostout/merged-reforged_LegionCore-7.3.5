-- BattlePay: Deleted Character Restoration (cooldown bypass)
-- The free restoration (6-month cooldown) is handled in C++ (CharacterHandler.cpp).
-- This BattlePay product allows players to bypass the cooldown for a token cost.

-- Display info
INSERT IGNORE INTO `battlepay_display_info` (`DisplayInfoId`, `CreatureDisplayInfoID`, `FileDataID`, `Flags`, `Name1`, `Name2`, `Name3`, `Name4`) VALUES
(200, 0, 0, 0, 'Character Restoration', '', 'Restore a previously deleted character immediately, bypassing the 6-month cooldown. Your character will be returned exactly as it was when deleted.', '');

-- Product (WebsiteType 15 = DeletedCharacter, ChoiceType 0 = no character selection needed at purchase)
INSERT IGNORE INTO `battlepay_product` (`ProductID`, `NormalPriceFixedPoint`, `CurrentPriceFixedPoint`, `Type`, `ChoiceType`, `Flags`, `DisplayInfoID`, `ScriptName`, `ClassMask`, `WebsiteType`) VALUES
(200, 50, 50, 0, 0, 0, 200, '', 0, 15);

-- Shop entry (GroupID 3 = Services)
INSERT IGNORE INTO `battlepay_shop_entry` (`EntryID`, `GroupID`, `ProductID`, `Ordering`, `Flags`, `BannerType`, `DisplayInfoID`) VALUES
(200, 3, 200, 10, 0, 0, 0);

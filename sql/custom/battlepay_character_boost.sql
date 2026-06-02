-- ==========================================================
-- Character Boost: products 109 (lvl90), 110 (lvl100), 111 (lvl110)
-- ==========================================================

-- Fix product 110: set price, WebsiteType=29 (CharacterBoost), ScriptName
UPDATE battlepay_product SET NormalPriceFixedPoint=150, CurrentPriceFixedPoint=150, WebsiteType=29, ScriptName='battlepay_service_level100', DisplayInfoID=110 WHERE ProductID=110;

-- Add shop entry for product 110
INSERT INTO battlepay_shop_entry (EntryID, GroupID, ProductID, Ordering, Flags, BannerType, DisplayInfoID)
VALUES (110, 3, 110, 6, 0, 0, 0) ON DUPLICATE KEY UPDATE GroupID=3;

-- Product 111: Level 110 Boost (new)
INSERT INTO battlepay_product (ProductID, NormalPriceFixedPoint, CurrentPriceFixedPoint, Type, ChoiceType, Flags, DisplayInfoID, ScriptName, ClassMask, WebsiteType)
VALUES (111, 200, 200, 0, 0, 0, 111, 'battlepay_service_level110', 0, 29)
ON DUPLICATE KEY UPDATE ScriptName='battlepay_service_level110', NormalPriceFixedPoint=200, CurrentPriceFixedPoint=200, WebsiteType=29, DisplayInfoID=111;

INSERT INTO battlepay_shop_entry (EntryID, GroupID, ProductID, Ordering, Flags, BannerType, DisplayInfoID)
VALUES (111, 3, 111, 7, 0, 0, 0) ON DUPLICATE KEY UPDATE GroupID=3;

-- Display info
INSERT INTO battlepay_display_info (DisplayInfoId, CreatureDisplayInfoID, FileDataID, Flags, Name1, Name2, Name3, Name4)
VALUES (110, 614740, 614740, 0, 'Level 100 Character Boost', '', 'Boost one character to level 100 with appropriate gear and flying skills.', '')
ON DUPLICATE KEY UPDATE Name1='Level 100 Character Boost';

INSERT INTO battlepay_display_info (DisplayInfoId, CreatureDisplayInfoID, FileDataID, Flags, Name1, Name2, Name3, Name4)
VALUES (111, 614740, 614740, 0, 'Level 110 Character Boost', '', 'Boost one character to level 110 with appropriate gear and flying skills. Teleport to Dalaran.', '')
ON DUPLICATE KEY UPDATE Name1='Level 110 Character Boost';

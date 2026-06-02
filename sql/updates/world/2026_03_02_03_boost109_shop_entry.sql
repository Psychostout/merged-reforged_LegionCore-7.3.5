-- ==========================================================
-- Fix: Product 109 (Level 90 Character Boost) was missing from
-- battlepay_shop_entry, causing it to be excluded from
-- SMSG_BATTLE_PAY_GET_PRODUCT_LIST_RESPONSE.
--
-- Without a shop entry, GetProductGroupForProductId(109) returns null
-- → product 109 is skipped in SendProductList()
-- → ProductListResponse never includes product 109
-- → client's GetUpgradeDistributions() can't match the distribution
--   to a BoostType → returns {} → boost button never appears
-- ==========================================================

-- Display info for product 109
INSERT INTO battlepay_display_info (DisplayInfoId, CreatureDisplayInfoID, FileDataID, Flags, Name1, Name2, Name3, Name4)
VALUES (109, 614740, 614740, 0, 'Level 90 Character Boost', '', 'Boost one character to level 90 with appropriate gear.', '')
ON DUPLICATE KEY UPDATE Name1='Level 90 Character Boost';

-- Ensure product 109 has correct WebsiteType=29 (CharacterBoost), ScriptName, DisplayInfoID
UPDATE battlepay_product
SET WebsiteType=29, ScriptName='battlepay_service_level90', DisplayInfoID=109
WHERE ProductID=109;

-- Add shop entry for product 109 (GroupID=3, consistent with products 110 and 111)
INSERT INTO battlepay_shop_entry (EntryID, GroupID, ProductID, Ordering, Flags, BannerType, DisplayInfoID)
VALUES (109, 3, 109, 5, 0, 0, 0)
ON DUPLICATE KEY UPDATE GroupID=3, ProductID=109;

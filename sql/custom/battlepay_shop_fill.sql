-- =============================================
-- PRODUCT GROUPS
-- =============================================
INSERT INTO battlepay_product_group (GroupID, Name, IconFileDataID, DisplayType, Ordering, Flags, TokenType, IngameOnly, OwnsTokensOnly) VALUES
(1, 'Montures', 841541, 0, 1, 0, 1, 0, 0),
(2, 'Mascottes', 132599, 0, 2, 0, 1, 0, 0),
(3, 'Jouets', 237178, 0, 4, 0, 1, 0, 0),
(4, 'Transmogrification', 132820, 0, 5, 0, 1, 0, 0)
ON DUPLICATE KEY UPDATE Name=VALUES(Name), IconFileDataID=VALUES(IconFileDataID), Ordering=VALUES(Ordering);

-- Update Services group ordering to be last
UPDATE battlepay_product_group SET Ordering = 10 WHERE GroupID = 22;

-- =============================================
-- DISPLAY INFO - Mounts
-- =============================================
INSERT INTO battlepay_display_info (DisplayInfoId, CreatureDisplayInfoID, FileDataID, Flags, Name1, Name2, Name3, Name4) VALUES
(1,  0, 0, 0, 'Gardien aile',          '', 'Ce lion majestueux emportera vos ennemis dans un tourbillon de plumes et de griffes.', ''),
(2,  0, 0, 0, 'Destrier celeste',      '', 'Une monture etheree venue des etoiles. Vole et nage.', ''),
(3,  0, 0, 0, 'Coeur des Aspects',     '', 'Un drake dore beni par les Aspects du Vol.', ''),
(4,  0, 0, 0, 'Destrier du vent agile','', 'Une monture gracieuse venue de Pandarie.', ''),
(5,  0, 0, 0, 'Aile de sang blindee',  '', 'Une chauve-souris de guerre en armure de plaques.', ''),
(6,  0, 0, 0, 'Kilin imperial',        '', 'Un kilin antique dote d\'ailes de foudre.', ''),
(7,  0, 0, 0, 'Dragon feerique enchante','','Un dragon capricieux et colore des bois enchantes.', ''),
(8,  0, 0, 0, 'Eventreur du ciel de fer','','Une chimere mecanique forgee par les Forgerons noirs.', ''),
(9,  0, 0, 0, 'Faucheur ricanant',     '', 'Une monture cauchemardesque au sourire eternel.', ''),
(10, 0, 0, 0, 'Cauchemar forge',       '', 'Un destrier de flammes et d\'acier forge dans les tenebres.', ''),
(11, 0, 0, 0, 'Tranchesabre mystique', '', 'Un felin runique de la nuit.', ''),
(12, 0, 0, 0, 'Chercheuse d\'etoiles lumineuse','','Une creature astrale qui illumine les cieux.', '')
ON DUPLICATE KEY UPDATE Name1=VALUES(Name1), Name3=VALUES(Name3);

-- =============================================
-- DISPLAY INFO - Pets
-- =============================================
INSERT INTO battlepay_display_info (DisplayInfoId, CreatureDisplayInfoID, FileDataID, Flags, Name1, Name2, Name3, Name4) VALUES
(21, 0, 0, 0, 'Mini K.T.',             '', 'Une version miniature du terrible Kel\'Thuzad.', ''),
(22, 0, 0, 0, 'Moine pandaren',        '', 'Un petit pandaren qui medite a vos cotes.', ''),
(23, 0, 0, 0, 'Mini XT',               '', 'Le coeur de Deconstructor, en format de poche.', ''),
(25, 0, 0, 0, 'Mini Ragnaros',         '', 'Le Seigneur du Feu, version adorable.', ''),
(26, 0, 0, 0, 'Ame des Aspects',       '', 'Un petit dragon ne de l\'essence des cinq Aspects.', ''),
(27, 0, 0, 0, 'Chaton de braise',      '', 'Un chaton de feu elementaire.', ''),
(28, 0, 0, 0, 'Petit du Kilin chanceux','','Un bebe Kilin qui porte bonheur.', ''),
(29, 0, 0, 0, 'Ancien florissant',     '', 'Un treant miniature en fleurs perpetuelles.', ''),
(30, 0, 0, 0, 'Chiot Alterac',         '', 'Un adorable chiot de brasserie Alterac.', ''),
(31, 0, 0, 0, 'Argi',                  '', 'Un bebe talbuk de l\'Exodar.', ''),
(32, 0, 0, 0, 'Brillepatte',           '', 'Un chat magique aux pattes lumineuses.', ''),
(33, 0, 0, 0, 'Croque-mignon',         '', 'Un crocilisque qui n\'a d\'yeux que pour vous.', ''),
(34, 0, 0, 0, 'Ombre',                 '', 'Un renard des tenebres, loyal et mysterieux.', ''),
(35, 0, 0, 0, 'Crepuscule',            '', 'Un jeune dragon du crepuscule.', '')
ON DUPLICATE KEY UPDATE Name1=VALUES(Name1), Name3=VALUES(Name3);

-- =============================================
-- DISPLAY INFO - Toys
-- =============================================
INSERT INTO battlepay_display_info (DisplayInfoId, CreatureDisplayInfoID, FileDataID, Flags, Name1, Name2, Name3, Name4) VALUES
(41, 0, 0, 0, 'Poteau d\'attache cauchemardesque','','Invoque un poteau qui transforme les montures alliees en cauchemars enflammes pendant 20 min.', '')
ON DUPLICATE KEY UPDATE Name1=VALUES(Name1), Name3=VALUES(Name3);

-- =============================================
-- DISPLAY INFO - Transmog
-- =============================================
INSERT INTO battlepay_display_info (DisplayInfoId, CreatureDisplayInfoID, FileDataID, Flags, Name1, Name2, Name3, Name4) VALUES
(51, 0, 0, 0, 'Couronne de l\'hiver eternel','','Un casque de glace eternelle.', ''),
(52, 0, 0, 0, 'Capuche de l\'obscurite devorante','','Un masque de tenebres vivantes.', ''),
(53, 0, 0, 0, 'Joyau du Seigneur du Feu','','La couronne enflammee de Ragnaros.', '')
ON DUPLICATE KEY UPDATE Name1=VALUES(Name1), Name3=VALUES(Name3);

-- =============================================
-- PRODUCTS - Mounts (WebsiteType=0 = Item)
-- =============================================
INSERT INTO battlepay_product (ProductID, NormalPriceFixedPoint, CurrentPriceFixedPoint, Type, ChoiceType, Flags, DisplayInfoID, ScriptName, ClassMask, WebsiteType) VALUES
(1,  250, 250, 0, 0, 0, 1,  '', 0, 0),
(2,  250, 250, 0, 0, 0, 2,  '', 0, 0),
(3,  250, 250, 0, 0, 0, 3,  '', 0, 0),
(4,  250, 250, 0, 0, 0, 4,  '', 0, 0),
(5,  250, 250, 0, 0, 0, 5,  '', 0, 0),
(6,  250, 250, 0, 0, 0, 6,  '', 0, 0),
(7,  250, 250, 0, 0, 0, 7,  '', 0, 0),
(8,  250, 250, 0, 0, 0, 8,  '', 0, 0),
(9,  250, 250, 0, 0, 0, 9,  '', 0, 0),
(10, 250, 250, 0, 0, 0, 10, '', 0, 0),
(11, 250, 250, 0, 0, 0, 11, '', 0, 0),
(12, 250, 250, 0, 0, 0, 12, '', 0, 0)
ON DUPLICATE KEY UPDATE NormalPriceFixedPoint=VALUES(NormalPriceFixedPoint);

-- PRODUCTS - Pets (WebsiteType=1 = BattlePet)
INSERT INTO battlepay_product (ProductID, NormalPriceFixedPoint, CurrentPriceFixedPoint, Type, ChoiceType, Flags, DisplayInfoID, ScriptName, ClassMask, WebsiteType) VALUES
(21, 100, 100, 0, 0, 0, 21, '', 0, 1),
(22, 100, 100, 0, 0, 0, 22, '', 0, 1),
(23, 100, 100, 0, 0, 0, 23, '', 0, 1),
(25, 100, 100, 0, 0, 0, 25, '', 0, 1),
(26, 100, 100, 0, 0, 0, 26, '', 0, 1),
(27, 100, 100, 0, 0, 0, 27, '', 0, 1),
(28, 100, 100, 0, 0, 0, 28, '', 0, 1),
(29, 100, 100, 0, 0, 0, 29, '', 0, 1),
(30, 100, 100, 0, 0, 0, 30, '', 0, 1),
(31, 100, 100, 0, 0, 0, 31, '', 0, 1),
(32, 100, 100, 0, 0, 0, 32, '', 0, 1),
(33, 100, 100, 0, 0, 0, 33, '', 0, 1),
(34, 100, 100, 0, 0, 0, 34, '', 0, 1),
(35, 100, 100, 0, 0, 0, 35, '', 0, 1)
ON DUPLICATE KEY UPDATE NormalPriceFixedPoint=VALUES(NormalPriceFixedPoint);

-- PRODUCTS - Toys (WebsiteType=0 = Item)
INSERT INTO battlepay_product (ProductID, NormalPriceFixedPoint, CurrentPriceFixedPoint, Type, ChoiceType, Flags, DisplayInfoID, ScriptName, ClassMask, WebsiteType) VALUES
(41, 100, 100, 0, 0, 0, 41, '', 0, 0)
ON DUPLICATE KEY UPDATE NormalPriceFixedPoint=VALUES(NormalPriceFixedPoint);

-- PRODUCTS - Transmog (WebsiteType=0 = Item)
INSERT INTO battlepay_product (ProductID, NormalPriceFixedPoint, CurrentPriceFixedPoint, Type, ChoiceType, Flags, DisplayInfoID, ScriptName, ClassMask, WebsiteType) VALUES
(51, 150, 150, 0, 0, 0, 51, '', 0, 0),
(52, 150, 150, 0, 0, 0, 52, '', 0, 0),
(53, 150, 150, 0, 0, 0, 53, '', 0, 0)
ON DUPLICATE KEY UPDATE NormalPriceFixedPoint=VALUES(NormalPriceFixedPoint);

-- =============================================
-- PRODUCT ITEMS (link product -> actual item ID)
-- =============================================
INSERT INTO battlepay_product_item (ID, ProductID, ItemID, Quantity, DisplayID, PetResult) VALUES
(1,  1,  69846,  1, 0, 0),
(2,  2,  54811,  1, 0, 0),
(3,  3,  78924,  1, 0, 0),
(4,  4,  92724,  1, 0, 0),
(5,  5,  95341,  1, 0, 0),
(6,  6,  85870,  1, 0, 0),
(7,  7,  97989,  1, 0, 0),
(8,  8,  107951, 1, 0, 0),
(9,  9,  112327, 1, 0, 0),
(10, 10, 112326, 1, 0, 0),
(11, 11, 122469, 1, 0, 0),
(12, 12, 147901, 1, 0, 0),
(21, 21, 49693,  1, 0, 0),
(22, 22, 49665,  1, 0, 0),
(23, 23, 54847,  1, 0, 0),
(25, 25, 68385,  1, 0, 0),
(26, 26, 78916,  1, 0, 0),
(27, 27, 92707,  1, 0, 0),
(28, 28, 85871,  1, 0, 0),
(29, 29, 98550,  1, 0, 0),
(30, 30, 106240, 1, 0, 0),
(31, 31, 118516, 1, 0, 0),
(32, 32, 128424, 1, 0, 0),
(33, 33, 128426, 1, 0, 0),
(34, 34, 151234, 1, 0, 0),
(35, 35, 147900, 1, 0, 0),
(41, 41, 112324, 1, 0, 0),
(51, 51, 95475,  1, 0, 0),
(52, 52, 97213,  1, 0, 0),
(53, 53, 95474,  1, 0, 0)
ON DUPLICATE KEY UPDATE ItemID=VALUES(ItemID);

-- =============================================
-- SHOP ENTRIES (link product -> group, ordering)
-- =============================================
INSERT INTO battlepay_shop_entry (EntryID, GroupID, ProductID, Ordering, Flags, BannerType, DisplayInfoID) VALUES
(1,  1, 1,  1,  0, 0, 0),
(2,  1, 2,  2,  0, 0, 0),
(3,  1, 3,  3,  0, 0, 0),
(4,  1, 4,  4,  0, 0, 0),
(5,  1, 5,  5,  0, 0, 0),
(6,  1, 6,  6,  0, 0, 0),
(7,  1, 7,  7,  0, 0, 0),
(8,  1, 8,  8,  0, 0, 0),
(9,  1, 9,  9,  0, 0, 0),
(10, 1, 10, 10, 0, 0, 0),
(11, 1, 11, 11, 0, 0, 0),
(12, 1, 12, 12, 0, 0, 0),
(21, 2, 21, 1,  0, 0, 0),
(22, 2, 22, 2,  0, 0, 0),
(23, 2, 23, 3,  0, 0, 0),
(25, 2, 25, 4,  0, 0, 0),
(26, 2, 26, 5,  0, 0, 0),
(27, 2, 27, 6,  0, 0, 0),
(28, 2, 28, 7,  0, 0, 0),
(29, 2, 29, 8,  0, 0, 0),
(30, 2, 30, 9,  0, 0, 0),
(31, 2, 31, 10, 0, 0, 0),
(32, 2, 32, 11, 0, 0, 0),
(33, 2, 33, 12, 0, 0, 0),
(34, 2, 34, 13, 0, 0, 0),
(35, 2, 35, 14, 0, 0, 0),
(41, 3, 41, 1,  0, 0, 0),
(51, 4, 51, 1,  0, 0, 0),
(52, 4, 52, 2,  0, 0, 0),
(53, 4, 53, 3,  0, 0, 0)
ON DUPLICATE KEY UPDATE GroupID=VALUES(GroupID);

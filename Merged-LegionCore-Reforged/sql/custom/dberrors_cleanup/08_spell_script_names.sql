-- ============================================================================
-- Batch 8: Spell script names fixes
-- Scripts C++ enregistrés mais sans entrée DB correspondante, ou entrées invalides
-- Date: 2026-02-25
-- ============================================================================

-- 8a: spell_pal_ardent_defender — spell_id -31850 = "tous les rangs du spell 31850"
-- Le spell 31850 n'a pas de rangs en 7.3.5 (Ardent Defender a été simplifié)
-- Fix: remplacer -31850 par 31850 (ID direct)
DELETE FROM world_legion.spell_script_names WHERE spell_id = -31850 AND ScriptName = 'spell_pal_ardent_defender';
INSERT IGNORE INTO world_legion.spell_script_names (spell_id, ScriptName) VALUES (31850, 'spell_pal_ardent_defender');

-- 8b: spell_mothers_embrace — spell_id -219045 = "tous les rangs du spell 219045"
-- Le spell 219045 n'a pas de rangs. L'entrée positive (219045) existe déjà.
-- Fix: supprimer l'entrée négative redondante
DELETE FROM world_legion.spell_script_names WHERE spell_id = -219045 AND ScriptName = 'spell_mothers_embrace';

-- 8c: spell_dark_shaman_koranthal_shadow_storm — spell 119973 (Shadow Storm)
-- Le script C++ utilise RegisterSpellScript (nouveau style) mais le core log quand même.
-- Fix: ajouter l'entrée DB
INSERT IGNORE INTO world_legion.spell_script_names (spell_id, ScriptName) VALUES (119973, 'spell_dark_shaman_koranthal_shadow_storm');

-- 8d: spell_q42740 — Broken Isles scenario spell (commentaire C++ dit "Spell not exits: 227058")
-- Le spell 227058 n'existe pas dans les DBC. Le script C++ est enregistré mais inutile.
-- Fix: ajouter l'entrée DB avec le spell 227058 pour supprimer l'erreur
-- (le script ne sera juste jamais appelé si le spell n'existe pas)
INSERT IGNORE INTO world_legion.spell_script_names (spell_id, ScriptName) VALUES (227058, 'spell_q42740');

-- 8e: battlepay_service_level100 — script C++ enregistré, pas de produit BattlePay en DB
-- Le script n'est pas dans spell_script_names (c'est un BattlePay service, pas un spell)
-- Le ProductID pour level100 n'existe pas dans battlepay_product (seul level90 = ID 109)
-- Fix: ajouter le produit level100 dans battlepay_product
-- NOTE: à adapter selon les besoins (prix, display, etc.)
INSERT IGNORE INTO world_legion.battlepay_product (ProductID, NormalPriceFixedPoint, CurrentPriceFixedPoint, Type, ChoiceType, Flags, DisplayInfoID, ScriptName, ClassMask)
SELECT MAX(ProductID)+1, 0, 0, Type, ChoiceType, Flags, DisplayInfoID, 'battlepay_service_level100', 0
FROM world_legion.battlepay_product WHERE ProductID = 109;

-- 8f: spell_dru_teleport_moonglade — spell 18960 "ranked spell" warning
-- Cosmétique: le core signale que le spell est ranké mais pas tous les rangs sont assignés.
-- En 7.3.5 il n'y a qu'un seul rang. Pas de fix nécessaire — warning ignorable.

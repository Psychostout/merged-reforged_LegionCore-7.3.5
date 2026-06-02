-- ============================================================================
-- Batch 12: db_script_string reserved range
-- Date: 2026-02-25
-- ============================================================================

-- La table db_script_string contient des entrées dans la plage 2000000000-2000010001
-- qui est réservée par TrinityCore. Ces strings ne sont pas chargées.
-- Fix: reID les strings hors de la plage réservée, ou supprimer si inutilisées.

-- Vérifier d'abord combien d'entrées sont concernées:
-- SELECT entry, content_default FROM db_script_string WHERE entry BETWEEN 2000000000 AND 2000010001;

-- Option A: Supprimer (si les scripts qui les utilisent n'existent pas)
DELETE FROM world_legion.db_script_string WHERE entry BETWEEN 2000000000 AND 2000010001;

-- Option B (alternative): Si les strings sont utilisées par des scripts,
-- il faudrait les re-numéroter hors de la plage réservée et mettre à jour
-- les références dans les tables de scripts correspondantes.
-- À vérifier manuellement si le DELETE ci-dessus cause des régressions.

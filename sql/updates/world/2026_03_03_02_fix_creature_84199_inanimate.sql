-- Fix: Creature 84199 est un objet inanimé mais attaquait les joueurs et leurs invocations.
-- Cause: faction=14 ("Monster") est hostile envers les joueurs → l'IA par défaut initiait le combat.
-- La créature avait déjà UNIT_FLAG_NOT_SELECTABLE (0x2000000) mais pas les flags d'immunité.
--
-- Fix:
--   faction   : 14 (Monster/hostile) → 35 (Friendly, neutre envers tout le monde)
--   unit_flags : ajout UNIT_FLAG_NON_ATTACKABLE (0x2) + UNIT_FLAG_IMMUNE_TO_PC (0x100) + UNIT_FLAG_IMMUNE_TO_NPC (0x200)
--              → la créature n'attaque pas et ne peut être attaquée (ni par joueurs, ni par invocations)

UPDATE creature_template
SET
    faction    = 35,
    unit_flags = unit_flags | 0x00000302
WHERE entry = 84199;

-- Fix spell 85692 (Doom Bolt / Trait Funeste) — Doomguard warlock pet
-- The spell costs 35 Energy in SpellPower.db2 (record ID 4947), but the
-- Doomguard uses Mana and has no energy bar, so it can never cast.
-- Setting ManaCost=0 makes the spell free (0 energy), which any unit can afford.
--
-- SpellPower.db2 record ID: 4947
-- SpellID: 85692
-- Original: ManaCost=35, PowerType=3 (Energy)
-- Fix:      ManaCost=0,  PowerType=3 (Energy, cost = 0)

INSERT INTO spell_power
    (ID, SpellID, ManaCost, PowerCostPct, PowerPctPerSecond, RequiredAuraSpellID,
     PowerCostMaxPct, OrderIndex, PowerType, ManaCostPerLevel, ManaPerSecond,
     OptionalCost, PowerDisplayID, AltPowerBarID, VerifiedBuild)
VALUES
    (4947, 85692, 0, 0.0, 0.0, 0, 0.0, 0, 3, 0, 0, 0, 0, 0, 26972)
ON DUPLICATE KEY UPDATE
    ManaCost       = 0,
    VerifiedBuild  = 26972;

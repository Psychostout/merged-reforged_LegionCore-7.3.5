-- Fix: Iron Horde creatures with MovementType=2 (WAYPOINT) but no waypoint data
-- Result: suppress WaypointMovementGenerator LoadPath warnings on startup
-- Affected entries: Ironmarch + Dreadmaul + Stormwind alliance mobs in Tanaan Jungle intro (map 1265)

UPDATE creature
SET MovementType = 0
WHERE id IN (
    77724, -- Ironmarch Sentinel
    77767, -- Ironmarch Shaman
    77723, -- Ironmarch Pillager
    78667, -- Ironmarch Legionnaire
    76886, -- Ironmarch Scout
    76189, -- Ironmarch Grunt
    78670, -- Ironmarch Warcaster
    78674, -- Ironmarch Scorcher
    78489, -- Snickerfang Pup
    78488, -- Dreadmaul Packmaster
    77640, -- Ironmarch Stone-Singer
    77643, -- Ironmarch Flame-Touched
    78142, -- Ironmarch Raider (1)
    77790, -- Ironmarch Raider (2)
    77090, -- Ironmarch Forager
    78348, -- Dreadmaul Flamebelcher
    76651, -- Ironmarch Leadspitter
    5983,  -- Bonepicker Felfeeder
    76108, -- "Stitches" Solderbolt
    76085, -- Stormwind Sea Wolf
    76098  -- Stormwind Sailor
)
AND MovementType = 2;

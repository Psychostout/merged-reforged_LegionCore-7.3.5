-- ==========================================================================
-- Rukhmar (83746) — Smooth flight patrol path around Skyreach
-- 19 waypoints for fluid movement without sharp angles
-- Path ID 439156 (referenced in boss_ruhamar script via MovePath)
-- NOTE: Must be in waypoint_data_script (not waypoint_data) because
--       MovePath(pathId) with explicit pathId uses GetPathScript()
-- ==========================================================================

-- Clean previous waypoints from both tables
DELETE FROM `waypoint_data` WHERE `id` = 439156;
DELETE FROM `waypoint_data_script` WHERE `id` = 439156;

-- Update spawn position
UPDATE `creature` SET
  `position_x` = -169.0,
  `position_y` = 2438.0,
  `position_z` = 231.0,
  `orientation` = 4.0,
  `MovementType` = 0
WHERE `guid` = 323055 AND `id` = 83746;

-- Smooth flight path: 19 waypoints, arcs at every turn
-- move_type 1 = RUN, speed 0 = use creature speed (SetSpeed 3.5)
INSERT INTO `waypoint_data_script` (`id`, `point`, `position_x`, `position_y`, `position_z`, `orientation`, `move_type`, `speed`, `delay`, `action`, `action_chance`) VALUES
-- Arc: spawn → WP1 (southwest descent)
(439156,  1,  -198.0, 2375.0, 210.0, 0, 1, 0, 0, 0, 100),
(439156,  2,  -222.0, 2338.0, 195.0, 0, 1, 0, 0, 0, 100),
(439156,  3,  -233.0, 2304.0, 183.0, 0, 1, 0, 0, 0, 100),  -- key point
-- Arc: WP1 → WP2 (south, descending to low pass)
(439156,  4,  -236.0, 2238.0, 158.0, 0, 1, 0, 0, 0, 100),
(439156,  5,  -232.0, 2167.0, 129.0, 0, 1, 0, 0, 0, 100),  -- key point — low pass
-- Arc: WP2 → WP3 (curving SSE, climbing)
(439156,  6,  -210.0, 2118.0, 137.0, 0, 1, 0, 0, 0, 100),
(439156,  7,  -174.0, 2077.0, 151.0, 0, 1, 0, 0, 0, 100),  -- key point
-- Arc: WP3 → WP4 (heading east)
(439156,  8,  -112.0, 2060.0, 164.0, 0, 1, 0, 0, 0, 100),
(439156,  9,   -43.0, 2054.0, 179.0, 0, 1, 0, 0, 0, 100),  -- key point
-- Arc: WP4 → WP5 (curving NE toward platform — extra points for sharp turn)
(439156, 10,    -8.0, 2090.0, 189.0, 0, 1, 0, 0, 0, 100),
(439156, 11,    20.0, 2145.0, 201.0, 0, 1, 0, 0, 0, 100),
(439156, 12,    45.0, 2205.0, 212.0, 0, 1, 0, 0, 0, 100),  -- key point — platform
-- Arc: WP5 → WP6 (heading north)
(439156, 13,    62.0, 2310.0, 202.0, 0, 1, 0, 0, 0, 100),
(439156, 14,    78.0, 2440.0, 188.0, 0, 1, 0, 0, 0, 100),
(439156, 15,    87.0, 2579.0, 176.0, 0, 1, 0, 0, 0, 100),  -- key point — north
-- Arc: WP6 → spawn (curving SW — extra points for sharpest turn)
(439156, 16,    25.0, 2570.0, 190.0, 0, 1, 0, 0, 0, 100),
(439156, 17,   -50.0, 2530.0, 205.0, 0, 1, 0, 0, 0, 100),
(439156, 18,  -120.0, 2480.0, 220.0, 0, 1, 0, 0, 0, 100),
-- Close the loop
(439156, 19,  -169.0, 2438.0, 231.0, 0, 1, 0, 0, 0, 100);  -- back to spawn

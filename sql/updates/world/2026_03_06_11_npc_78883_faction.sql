-- NPC 78883 (mob_wod_intro_enemy_at_portal) : faction 14 (Friendly) → 16 (Monster, hostile à tous)
-- Rend le NPC hostile aux joueurs et aux alliés (Thrall, Khadgar, Ariok faction 35)
UPDATE creature_template SET faction = 16 WHERE entry = 78883;

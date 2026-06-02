-- Brawlers Guild: activer le script C++ npc_brawlers_guild_queue
-- NPC 68408 = queue manager Alliance (Stormwind)
-- NPC 67267 = queue manager Horde (Orgrimmar)
-- Sans ce ScriptName, les joueurs ne peuvent pas s'inscrire au Brawlers Guild.
UPDATE `creature_template` SET `ScriptName` = 'npc_brawlers_guild_queue'
  WHERE `entry` IN (68408, 67267);

-- Brawlers Guild: corriger les ScriptName vides sur les variantes de boss
-- NPC 115044 = boss_brawguild_rayd (2e variante, 115040 est correcte)
-- NPC 114976 = npc_brawguild_burnstachio_ground (2e variante, 114975 est correcte)
-- Sans ce correctif, ces variantes utilisent l'AI par défaut (statique).
UPDATE `creature_template` SET `ScriptName` = 'boss_brawguild_rayd'
  WHERE `entry` = 115044;

UPDATE `creature_template` SET `ScriptName` = 'npc_brawguild_burnstachio_ground'
  WHERE `entry` = 114976;

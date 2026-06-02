-- Fix gameobject_template: replace invisible displayId 39 with 32290
-- 2951 entries affected

UPDATE `gameobject_template` SET `displayId` = 32290 WHERE `displayId` = 39;

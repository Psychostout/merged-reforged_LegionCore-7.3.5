-- Fix GO 237670 (Dark Portal, map 1265, guid 105799)
-- Remove PhaseId so the GO is visible to all phases (creatures + players)
-- Without this fix, creatures (empty phases {}) vs GO (phases {4200}) → InSamePhaseId returns false
-- → DynamicTree skips the GO for LOS and GetHeight checks
UPDATE gameobject SET PhaseId='' WHERE guid=105799;

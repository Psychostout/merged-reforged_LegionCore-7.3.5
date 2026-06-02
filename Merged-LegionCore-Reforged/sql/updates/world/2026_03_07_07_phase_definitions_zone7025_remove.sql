-- ================================================================
-- 2026_03_07_07 — Suppression des phase_definitions zone 7025
--
-- La logique de phases du scénario intro WoD (Map 1265, zone 7025)
-- est désormais entièrement gérée en C++ dans :
--   playerscript_wod_portal_phases::UpdatePhaseForPlayer()
--   (src/server/scripts/Draenor/dark_portal.cpp)
--
-- Pour RESTAURER les entrées originales (annuler ce script) :
-- décommenter et exécuter le bloc INSERT ci-dessous.
-- ================================================================

DELETE FROM world_legion.phase_definitions WHERE zoneId = 7025;

-- ================================================================
-- RESTAURATION — 45 entrées originales zone 7025
-- (phasemask=0, PreloadMapID=0, VisibleMapID=0, UiWorldMapAreaID=0, flags=0)
-- Décommenter pour restaurer :
-- ================================================================
/*
INSERT INTO world_legion.phase_definitions
    (zoneId, entry, phasemask, phaseId, PreloadMapID, VisibleMapID, UiWorldMapAreaID, flags, comment)
VALUES
-- Entrée 1 : base (avant toute quête)
(7025, 1, 0, '3248 3249 3250 3251 3263 3480 3563 3568 3605 3693 3712 3763 3764 3824 3833 3834 3880 3946 4142 4143 4200', 0, 0, 0, 0, 'Draenor Dark Portal Intro'),
-- Entrée 2 : (phaseId vide — condition faction/spéciale)
(7025, 2, 0, '', 0, 0, 0, 0, 'Draenor Dark Portal Intro'),
-- Entrée 3 : (phaseId vide — condition faction/spéciale)
(7025, 3, 0, '', 0, 0, 0, 0, 'Draenor Dark Portal Intro'),
-- Entrée 4 : Q34392 démarré (pas de phase 3880)
(7025, 4, 0, '3248 3249 3250 3251 3263 3480 3563 3568 3605 3693 3712 3763 3764 3824 3833 3834 3946 4142 4143 4200', 0, 0, 0, 0, 'DraenorIntro: Q34392 started'),
-- Entrée 5 : Q34393 démarré
(7025, 5, 0, '3248 3249 3250 3251 3263 3480 3563 3568 3693 3712 3824 3833 3834 3948 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: Q34393 started'),
-- Entrée 6 : Q34393 complet
(7025, 6, 0, '3263 3480 3569 3604 3693 3712 3824 3833 3834 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: Q34393 completed.'),
-- Entrée 7 : Q34420 démarré ou Q34393 récompensé
(7025, 7, 0, '3480 3604 3693 3712 3824 3833 3834 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: Q34420 started or 34393 rewarded.'),
-- Entrée 8 : Q34420 incomplet + SMSG_EXPLORATION_EXPERIENCE zone 7041
(7025, 8, 0, '3236 3480 3626 3670 3693 3712 3794 3824 3833 3834 3856 3857 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: Q34420 at SMSG_EXPLORATION_EXPERIENCE 7041'),
-- Entrée 9 : Q34420 incomplet + CMSG_SCENE_PLAYBACK_COMPLETE sceneID 621
(7025, 9, 0, '3236 3264 3394 3395 3396 3480 3626 3670 3693 3712 3794 3824 3833 3834 3856 3857 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: Q34420 at CMSG_SCENE_PLAYBACK_COMPLETE sceneID 621'),
-- Entrée 10 : Q34420 récompensé
(7025, 10, 0, '3236 3394 3395 3396 3480 3626 3670 3693 3712 3794 3824 3833 3834 3856 3857 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: Q34420 rewarded'),
-- Entrée 11 : (phaseId vide — Horde custom, Q34420 récompensé)
(7025, 11, 0, '', 0, 0, 0, 0, 'DraenorIntro: cunstom for horde after Q34420 rewarded'),
-- Entrée 12 : (phaseId vide — Alliance custom, Q34420 récompensé)
(7025, 12, 0, '', 0, 0, 0, 0, 'DraenorIntro: custom for alliance Q34420 rewarded'),
-- Entrée 13 : Q34421 accepté (additive — phases 3209, 3210)
(7025, 13, 0, '3209 3210', 0, 0, 0, 0, 'DraenorIntro: Q34421 take'),
-- Entrée 14 : CMSG_SCENE_PLAYBACK_COMPLETE sceneID 770 (additive — phase 4011)
(7025, 14, 0, '4011', 0, 0, 0, 0, 'DraenorIntro: at CMSG_SCENE_PLAYBACK_COMPLETE sceneID 770'),
-- Entrée 15 : Q34422 récompensé
(7025, 15, 0, '3237 3265 3394 3395 3396 3480 3626 3655 3670 3693 3712 3794 3824 3833 3834 3856 3857 3911 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: at reward 34422'),
-- Entrée 16 : Q34423 incomplet + objectif 273678 = 3
(7025, 16, 0, '3237 3266 3394 3395 3396 3414 3480 3626 3693 3712 3794 3824 3833 3834 3856 3857 4006 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: Q34423 complete objective 273678 = 3'),
-- Entrée 17 : Q34425 démarré
(7025, 17, 0, '3266 3394 3395 3396 3480 3693 3694 3712 3794 3824 3833 3834 4006 4017 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: Q34425'),
-- Entrée 18 : Q34425 + CMSG_SCENE_TRIGGER_EVENT Bridge
(7025, 18, 0, '3266 3394 3395 3396 3481 3693 3694 3712 3824 3833 3834 4006 4017 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: Q34425 after CMSG_SCENE_TRIGGER_EVENT Bridge'),
-- Entrée 19 : Q34425 complet ou récompensé
(7025, 19, 0, '3266 3317 3349 3358 3359 3394 3395 3396 3416 3481 3693 3694 3712 3824 3833 3834 4006 4017 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: Q34425 complete & rewarded.'),
-- Entrée 20 : CMSG_SCENE_PLAYBACK_COMPLETE (scène pont)
(7025, 20, 0, '3266 3278 3317 3349 3350 3358 3359 3394 3395 3396 3416 3481 3693 3694 3712 3824 3833 3834 4006 4017 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: CMSG_SCENE_PLAYBACK_COMPLETE.'),
-- Entrée 21 : Q34429 démarré
(7025, 21, 0, '3317 3349 3350 3358 3359 3394 3395 3396 3481 3712 3824 3833 3834 4017 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: start 34429'),
-- Entrée 22 : Q34429 objectif complet
(7025, 22, 0, '3317 3349 3350 3358 3359 3394 3395 3396 3481 3692 3693 3824 3833 3834 4017 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: objectibe complet on q34429'),
-- Entrée 23 : Q34429 complet
(7025, 23, 0, '3267 3317 3330 3394 3395 3396 3481 3693 3712 3720 3790 3824 3833 3834 4015 4017 4150 4151 4200 3752', 0, 0, 0, 0, 'DraenorIntro: q34429 complete'),
-- Entrée 24 : Q34429 complet (additive — phase 3752)
(7025, 24, 0, '3752', 0, 0, 0, 0, 'DraenorIntro: q34429 complete'),
-- Entrée 25 : Q34429 récompensé
(7025, 25, 0, '3267 3317 3334 3356 3394 3395 3396 3481 3693 3712 3720 3752 3790 3824 3833 3834 3936 4015 4017 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: q34429 rewarded'),
-- Entrée 26 : avant Q34434/34740 (additive — phase 3352)
(7025, 26, 0, '3352', 0, 0, 0, 0, 'DraenorIntro: before 34434, 34740'),
-- Entrée 27 : après Q34434/34740 (additive — phase 3351)
(7025, 27, 0, '3351', 0, 0, 0, 0, 'DraenorIntro: after 34434, 34740'),
-- Entrée 28 : Q34741/Q34436 démarré
(7025, 28, 0, '3267 3317 3334 3394 3395 3396 3481 3693 3712 3720 3752 3790 3824 3833 3834 3936 4015 4017 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: q34741 34436'),
-- Entrée 29 : Q34436/Q34741 complet (variante A)
(7025, 29, 0, '3268 3317 3334 3394 3395 3396 3481 3497 3498 3499 3693 3712 3752 3824 3833 3834 3936 4017 4019 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: q34436 q34741 completed'),
-- Entrée 30 : Q34436/Q34741 complet (variante B)
(7025, 30, 0, '3268 3317 3334 3394 3395 3396 3481 3497 3498 3499 3594 3693 3712 3752 3824 3833 3834 3936 4019 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: q34436 q34741 completed'),
-- Entrée 31 : (intermédiaire)
(7025, 31, 0, '3268 3334 3394 3395 3396 3481 3498 3499 3594 3597 3693 3712 3752 3824 3833 3834 3936 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: '),
-- Entrée 32 : Q34439 démarré
(7025, 32, 0, '3334 3394 3395 3396 3481 3498 3594 3693 3712 3752 3824 3833 3834 3936 4022 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: stsrt q34439'),
-- Entrée 33 : Q34439 (additive — phase 3500)
(7025, 33, 0, '3500', 0, 0, 0, 0, 'DraenorIntro: stsrt q34439 (phase additive)'),
-- Entrée 34 : Q34439 objectifs avancés
(7025, 34, 0, '3269 3334 3394 3395 3396 3423 3481 3498 3505 3594 3693 3712 3752 3833 3834 3936 4026 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: stsrt q34439 (objectifs avances)'),
-- Entrée 35 : (phase 0 — condition inconnue)
(7025, 35, 0, '0', 0, 0, 0, 0, 'DraenorIntro: (phase 0)'),
-- Entrée 36 : Q34442 démarré
(7025, 36, 0, '3269 3334 3394 3395 3396 3423 3481 3498 3505 3551 3594 3693 3712 3752 3833 3834 3936 4026 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: stsrt q34442 start'),
-- Entrée 37 : (additive — phase 3579)
(7025, 37, 0, '3579', 0, 0, 0, 0, 'DraenorIntro: (phase additive 3579)'),
-- Entrée 38 : Q34437 démarré
(7025, 38, 0, '3269 3394 3395 3396 3423 3481 3498 3505 3579 3581 3594 3693 3712 3752 3833 3834 3936 4026 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: stsrt q34437 start'),
-- Entrée 39 : Q35747 créature tuée (additive — phase 3508)
(7025, 39, 0, '3508', 0, 0, 0, 0, 'DraenorIntro: q35747 creature killed (NPC 80880 = 1)'),
-- Entrée 40 : Q35747 objectif ou récompensé
(7025, 40, 0, '3269 3394 3395 3396 3481 3498 3542 3583 3604 3693 3712 3752 3833 3834 3936 4150 4151 4200', 0, 0, 0, 0, 'DraenorIntro: stsrt q35747 o:80887 or reward'),
-- Entrée 41 : Q35747 non démarré, Q34445 non pris (additive — phase 3519)
(7025, 41, 0, '3519', 0, 0, 0, 0, 'DraenorIntro: q35747 not started and not Q34445'),
-- Entrée 42 : Q34445 complet ou récompensé
(7025, 42, 0, '3394 3395 3396 3481 3498 3519 3583 3604 3693 3712 3752 3833 3834 3936 4028 4150 4151 4201', 0, 0, 0, 0, 'DraenorIntro: q34445 complete or rew'),
-- Entrée 43 : Q34446/Q35884 complet ou scène complète (variante A — +3889)
(7025, 43, 0, '3394 3395 3396 3481 3498 3693 3712 3752 3833 3834 3889 3936 4028 4072 4150 4151 4201', 0, 0, 0, 0, 'DraenorIntro: q34446 35884 complete or scene complete'),
-- Entrée 44 : Q34446/Q35884 complet ou scène complète (variante B — sans 3833/3889)
(7025, 44, 0, '3394 3395 3396 3481 3498 3693 3712 3752 3834 3936 4028 4072 4150 4151 4201', 0, 0, 0, 0, 'DraenorIntro: q34446 35884 complete or scene complete'),
-- Entrée 45 : quest=35747 (additive — phase 3680)
(7025, 45, 0, '3680', 0, 0, 0, 0, 'quest=35747');
*/

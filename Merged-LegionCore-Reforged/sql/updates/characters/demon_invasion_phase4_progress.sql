-- ==========================================================================
-- Demon Invasion Prepatch - Phase 4: Achievement tracking table
-- Run against characters_legion database
-- ==========================================================================

CREATE TABLE IF NOT EXISTS `character_demon_invasion_progress` (
    `guid` INT UNSIGNED NOT NULL,
    `zone_index` TINYINT UNSIGNED NOT NULL,
    `completed_time` INT UNSIGNED NOT NULL DEFAULT 0,
    PRIMARY KEY (`guid`, `zone_index`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

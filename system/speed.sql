-- Raw speed telemetry from ESP (PhysicalComputing/api/load.php)
-- Import before system/highscores.sql

CREATE TABLE IF NOT EXISTS `speed` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `velo_id` TINYINT NOT NULL COMMENT '1 = Bike A, 2 = Bike B',
  `speed` DECIMAL(10, 1) NOT NULL COMMENT 'km/h',
  `time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_velo_time` (`velo_id`, `time`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

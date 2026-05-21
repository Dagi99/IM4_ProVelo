-- Highscores for Bike Challenge leaderboard
-- Import after speed table exists in your database.

CREATE TABLE IF NOT EXISTS `highscores` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `player_name` VARCHAR(32) NOT NULL,
  `velo_id` TINYINT NOT NULL COMMENT '1 = Bike A, 2 = Bike B',
  `distance_km` DECIMAL(8, 3) NOT NULL,
  `created_at` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_created` (`created_at`),
  KEY `idx_player_velo` (`player_name`, `velo_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

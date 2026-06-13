-- Random player names for Bike Challenge (api/assign-name.php)
-- Import after system/speed.sql, before system/highscores.sql

CREATE TABLE IF NOT EXISTS `assigned_names` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `funky_name` VARCHAR(32) NOT NULL,
  `is_assigned` TINYINT(1) NOT NULL DEFAULT 0,
  `assigned_at` TIMESTAMP NULL DEFAULT NULL,
  `velo_id` TINYINT NULL DEFAULT NULL COMMENT '1 = Bike A, 2 = Bike B when assigned',
  PRIMARY KEY (`id`),
  KEY `idx_available` (`is_assigned`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

INSERT INTO `assigned_names` (`funky_name`) VALUES
  ('Blitz-Rad'),
  ('Turbo-Tret'),
  ('Sprint-Safe'),
  ('Pedal-Power'),
  ('Rad-Rakete'),
  ('Velo-Viper'),
  ('Chain-Champion'),
  ('Wheel-Wizard'),
  ('Gear-Guru'),
  ('Spin-Storm');

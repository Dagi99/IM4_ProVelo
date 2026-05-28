-- Optional: detailed per-challenge results (distance + top speed)
CREATE TABLE IF NOT EXISTS challenge_results (
  id INT AUTO_INCREMENT PRIMARY KEY,
  started_at TIMESTAMP NOT NULL,
  velo_id TINYINT NOT NULL,
  player_name VARCHAR(100) NOT NULL,
  distance_km DECIMAL(10,3) NOT NULL,
  top_speed_kmh DECIMAL(10,1) NOT NULL,
  created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
  UNIQUE KEY uniq_result (started_at, velo_id),
  KEY idx_player_velo (player_name, velo_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;


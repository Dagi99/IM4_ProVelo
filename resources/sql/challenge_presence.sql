-- Presence heartbeats for Bike A (1) and Bike B (2)
CREATE TABLE IF NOT EXISTS challenge_presence (
  velo_id TINYINT PRIMARY KEY,
  last_seen_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
    ON UPDATE CURRENT_TIMESTAMP
);

INSERT IGNORE INTO challenge_presence (velo_id) VALUES (1), (2);


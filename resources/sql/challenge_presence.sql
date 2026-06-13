-- challenge_presence.sql — Heartbeat-Zeitstempel pro Bike für Challenge-Start.
-- Import: Schritt 6.

CREATE TABLE IF NOT EXISTS challenge_presence (
  velo_id TINYINT PRIMARY KEY,
  last_seen_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
    ON UPDATE CURRENT_TIMESTAMP
);

INSERT IGNORE INTO challenge_presence (velo_id) VALUES (1), (2);


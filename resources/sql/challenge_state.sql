-- Shared challenge state (single row id=1)
CREATE TABLE IF NOT EXISTS challenge_state (
  id TINYINT PRIMARY KEY DEFAULT 1,
  started_at TIMESTAMP NULL DEFAULT NULL,
  a_assigned_at TIMESTAMP NULL DEFAULT NULL,
  b_assigned_at TIMESTAMP NULL DEFAULT NULL,
  results_saved_for_started_at TIMESTAMP NULL DEFAULT NULL,
  updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
    ON UPDATE CURRENT_TIMESTAMP
);

INSERT IGNORE INTO challenge_state (id) VALUES (1);


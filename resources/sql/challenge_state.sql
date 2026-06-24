-- challenge_state.sql — Singleton-Zeile (id=1) für Challenge-Timer und Ergebnis-Status.
-- Import: Schritt 5.

CREATE TABLE IF NOT EXISTS challenge_state (
  id TINYINT PRIMARY KEY DEFAULT 1,
  started_at TIMESTAMP NULL DEFAULT NULL,
  a_assigned_at TIMESTAMP NULL DEFAULT NULL,
  b_assigned_at TIMESTAMP NULL DEFAULT NULL,
  results_saved_for_started_at TIMESTAMP NULL DEFAULT NULL,
  opponent_wait_started_at TIMESTAMP NULL DEFAULT NULL,
  mode VARCHAR(8) NOT NULL DEFAULT 'duel',
  solo_velo_id TINYINT NULL DEFAULT NULL,
  updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
    ON UPDATE CURRENT_TIMESTAMP
);

INSERT IGNORE INTO challenge_state (id) VALUES (1);


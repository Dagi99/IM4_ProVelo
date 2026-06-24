-- Solo-Modus: 20s Gegner-Wartezeit (bestehende DB migrieren).
-- Einmal ausführen wenn challenge_state bereits ohne diese Spalten existiert.

ALTER TABLE challenge_state
  ADD COLUMN opponent_wait_started_at TIMESTAMP NULL DEFAULT NULL,
  ADD COLUMN mode VARCHAR(8) NOT NULL DEFAULT 'duel',
  ADD COLUMN solo_velo_id TINYINT NULL DEFAULT NULL;

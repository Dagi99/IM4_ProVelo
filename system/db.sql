-- db.sql — Tabelle users für Admin-Login (E-Mail, gehashtes Passwort).
-- Import: Schritt 1 der Installationsanleitung.

CREATE TABLE IF NOT EXISTS `users` (
  `id` INT NOT NULL AUTO_INCREMENT,
  `email` VARCHAR(100) NOT NULL,
  `password` VARCHAR(255) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY (`email`)
);

<?php
/**
 * load.php — Empfängt JSON vom ESP (velo_id, wert) und schreibt in Tabelle speed.
 * Löscht Einträge älter als 15 Minuten.
 */

require_once("../../system/config.php");


###################################### JSON empfangen

$inputJSON = file_get_contents('php://input');
$input = json_decode($inputJSON, true);


###################################### Daten auslesen

$wert = $input["wert"];
$velo_id = $input["velo_id"]; // z.B. "velo_1" oder "velo_2"


###################################### Datenbankeintrag

$sql = "INSERT INTO speed (velo_id, speed) VALUES (?, ?)";

$stmt = $pdo->prepare($sql);
$stmt->execute([$velo_id, $wert]);

$pdo->query('DELETE FROM speed WHERE time < NOW() - INTERVAL 15 MINUTE');

?>
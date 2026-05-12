<?php
/*****************************************************
 * Kapitel 12: Website2DB > Schritt 2: Website -> DB
 * load.php
 * Empfängt JSON-Daten und speichert sie in die DB
 *****************************************************/

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

?>
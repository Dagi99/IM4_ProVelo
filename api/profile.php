<?php
/**
 * profile.php — Gibt Profildaten des eingeloggten Benutzers als JSON zurück.
 */
session_start();
 
include_once("../system/config.php");

$userID = $_SESSION ['user_id'];

$stmt = $pdo ->prepare("SELECT * FROM users WHERE id = :userID");
$stmt->execute([':userID' => $userID]);
$user = $stmt->fetch(PDO::FETCH_ASSOC);

echo json_encode([
    "status" => "success",
    "user" => $user ['id'],
    "email" => $user ['email'],
    "vorname" => $user ['firstname'],
    "nachname" => $user ['lastname']
]);
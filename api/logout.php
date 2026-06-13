<?php
/**
 * logout.php — Beendet die PHP-Session und gibt JSON-Erfolg zurück.
 */
session_start();
$_SESSION = [];
session_destroy();

// Return a success response instead of redirecting
header('Content-Type: application/json');
echo json_encode(["status" => "success"]);
exit;
?>
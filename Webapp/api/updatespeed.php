<?php
 
include_once("../system/config.php");

$velo_id = "1";

$stmt = $pdo ->prepare("SELECT * FROM speed WHERE id = :velo_id");
$stmt->execute([':velo_id' => $velo_id]);
$user = $stmt->fetch(PDO::FETCH_ASSOC);

echo json_encode([
    "status" => "success",
    "speed" => $user ['speed']
]);
<?php
header('Content-Type: application/json');
include_once("../../system/config.php");

$velo_id = "2";

try {
    $stmt = $pdo ->prepare("SELECT speed FROM speed WHERE velo_id = :velo_id ORDER BY time DESC LIMIT 1");
    $stmt->execute([':velo_id' => $velo_id]);
    $data = $stmt->fetch(PDO::FETCH_ASSOC);
    if ($data){
        echo json_encode([
            "status" => "success",
            "speed" => number_format($data['speed'], 1 )
        ]);
    }else {
        echo json_encode([
            "status" => "error",
            "message" => "No data"
        ]);
    }
} catch (PDOException $e) {
    echo json_encode([
        "status" => "error",
        "message" => $e->getMessage()
    ]);
}
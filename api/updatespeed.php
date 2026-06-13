<?php
/**
 * updatespeed.php — Legacy: liefert neueste Speed-Werte pro Bike und löscht Einträge älter als 15 Min.
 */
header('Content-Type: application/json');
include_once("../system/config.php");

$displayVeloId = (string) ($_GET['velo_id'] ?? '2');
if (!in_array($displayVeloId, ['1', '2'], true)) {
    $displayVeloId = '2';
}

try {
    $pdo->query("DELETE FROM speed WHERE time < NOW() - INTERVAL 15 MINUTE");

    $speeds = ["1" => "0.0", "2" => "0.0"];

    $stmt = $pdo->prepare("
        SELECT s.velo_id, s.speed
        FROM speed s
        INNER JOIN (
            SELECT velo_id, MAX(time) AS max_time
            FROM speed
            WHERE velo_id IN (1, 2)
            GROUP BY velo_id
        ) latest ON s.velo_id = latest.velo_id AND s.time = latest.max_time
    ");
    $stmt->execute();

    while ($row = $stmt->fetch(PDO::FETCH_ASSOC)) {
        $speeds[(string) $row['velo_id']] = number_format($row['speed'], 1);
    }

    echo json_encode([
        "status" => "success",
        "speeds" => $speeds,
        "speed" => $speeds[$displayVeloId],
    ]);
} catch (PDOException $e) {
    echo json_encode([
        "status" => "error",
        "message" => $e->getMessage(),
    ]);
}

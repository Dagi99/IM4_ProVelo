<?php
/**
 * simulate-speed.php — Entwicklung: simulierte Speed-Ticks für Bike A/B (race.html, Test ohne ESP).
 */
header('Content-Type: application/json');
require_once('../system/config.php');

const MIN_SPEED = 0;
const MAX_SPEED = 30;

function clampSpeed(float $speed): float
{
    return round(max(MIN_SPEED, min(MAX_SPEED, $speed)), 1);
}

try {
    $t = time();
    $speedA = clampSpeed(12 + 8 * sin($t / 3));
    $speedB = clampSpeed(14 + 8 * sin($t / 3 + M_PI));

    $stmt = $pdo->prepare('INSERT INTO speed (velo_id, speed) VALUES (?, ?)');
    $stmt->execute([1, $speedA]);
    $stmt->execute([2, $speedB]);

    echo json_encode([
        'status' => 'success',
        'speeds' => [
            '1' => number_format($speedA, 1),
            '2' => number_format($speedB, 1),
        ],
    ]);
} catch (PDOException $e) {
    echo json_encode([
        'status' => 'error',
        'message' => $e->getMessage(),
    ]);
}

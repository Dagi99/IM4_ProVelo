<?php
/**
 * get-opponent-name.php — Gibt den Gegner-Namen für das jeweilige Bike zurück (GET ?velo_id).
 */
header('Content-Type: application/json');
require_once('../system/config.php');

$veloId = (int) ($_GET['velo_id'] ?? 0);
if (!in_array($veloId, [1, 2], true)) {
    echo json_encode([
        'status' => 'error',
        'message' => 'velo_id must be 1 or 2',
    ]);
    exit;
}

$opponentVeloId = $veloId === 1 ? 2 : 1;

try {
    $stmt = $pdo->prepare(
        'SELECT funky_name FROM assigned_names
         WHERE velo_id = ? AND is_assigned = 1
         ORDER BY assigned_at DESC
         LIMIT 1'
    );
    $stmt->execute([$opponentVeloId]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);

    if ($row) {
        echo json_encode([
            'status' => 'success',
            'name' => $row['funky_name'],
            'velo_id' => $opponentVeloId,
        ]);
        exit;
    }

    echo json_encode([
        'status' => 'success',
        'name' => $opponentVeloId === 1 ? 'Bike A' : 'Bike B',
        'velo_id' => $opponentVeloId,
    ]);
} catch (PDOException $e) {
    echo json_encode([
        'status' => 'error',
        'message' => $e->getMessage(),
    ]);
}

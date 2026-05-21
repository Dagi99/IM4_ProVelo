<?php
session_start();
header('Content-Type: application/json');

$veloId = (int) ($_GET['velo_id'] ?? 0);
if (!in_array($veloId, [1, 2], true)) {
    echo json_encode([
        'status' => 'error',
        'message' => 'velo_id must be 1 or 2',
    ]);
    exit;
}

$name = $_SESSION['players'][$veloId]['name'] ?? null;

if ($name) {
    echo json_encode([
        'status' => 'success',
        'name' => $name,
        'velo_id' => $veloId,
    ]);
    exit;
}

echo json_encode([
    'status' => 'error',
    'message' => 'No player name assigned for this bike',
]);

<?php
header('Content-Type: application/json');
require_once '../system/config.php';

$veloId = (int) ($_GET['velo_id'] ?? 0);
if (!in_array($veloId, [1, 2], true)) {
    echo json_encode(['status' => 'error', 'message' => 'velo_id must be 1 or 2']);
    exit;
}

try {
    $stmt = $pdo->prepare(
        'INSERT INTO challenge_presence (velo_id, last_seen_at)
         VALUES (?, NOW())
         ON DUPLICATE KEY UPDATE last_seen_at = NOW()'
    );
    $stmt->execute([$veloId]);

    echo json_encode(['status' => 'success']);
} catch (PDOException $e) {
    echo json_encode(['status' => 'error', 'message' => $e->getMessage()]);
}


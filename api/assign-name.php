<?php
/**
 * assign-name.php — Weist zufälligen Namen aus assigned_names zu (GET ?velo_id=1|2), speichert in Session.
 */
session_start();
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

if (!isset($_SESSION['players'])) {
    $_SESSION['players'] = [];
}

if (!empty($_SESSION['players'][$veloId]['name'])) {
    $name = $_SESSION['players'][$veloId]['name'];
    $_SESSION['velo_id'] = $veloId;
    $_SESSION['player_name'] = $name;
    $_SESSION['funky_name'] = $name;

    echo json_encode([
        'status' => 'success',
        'name' => $name,
        'velo_id' => $veloId,
    ]);
    exit;
}

try {
    $pdo->beginTransaction();

    $stmt = $pdo->query(
        'SELECT id, funky_name FROM assigned_names WHERE is_assigned = 0 ORDER BY RAND() LIMIT 1'
    );
    $row = $stmt->fetch(PDO::FETCH_ASSOC);

    if (!$row) {
        $pdo->rollBack();
        echo json_encode([
            'status' => 'error',
            'message' => 'No names available',
        ]);
        exit;
    }

    $update = $pdo->prepare(
        'UPDATE assigned_names SET is_assigned = 1, assigned_at = NOW(), velo_id = ? WHERE id = ? AND is_assigned = 0'
    );
    $update->execute([$veloId, $row['id']]);

    if ($update->rowCount() !== 1) {
        $pdo->rollBack();
        echo json_encode([
            'status' => 'error',
            'message' => 'Name assignment failed, please try again',
        ]);
        exit;
    }

    $pdo->commit();

    $name = $row['funky_name'];
    $_SESSION['players'][$veloId] = [
        'name' => $name,
        'velo_id' => $veloId,
    ];
    $_SESSION['velo_id'] = $veloId;
    $_SESSION['player_name'] = $name;
    $_SESSION['funky_name'] = $name;

    echo json_encode([
        'status' => 'success',
        'name' => $name,
        'velo_id' => $veloId,
        'assigned_at' => (new DateTime('now'))->format('Y-m-d H:i:s'),
    ]);
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    echo json_encode([
        'status' => 'error',
        'message' => $e->getMessage(),
    ]);
}

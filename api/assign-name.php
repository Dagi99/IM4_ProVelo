<?php
session_start();
header('Content-Type: application/json');
require_once('../system/config.php');

if (!empty($_SESSION['funky_name'])) {
    echo json_encode([
        'status' => 'success',
        'name' => $_SESSION['funky_name'],
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
        'UPDATE assigned_names SET is_assigned = 1, assigned_at = NOW() WHERE id = ? AND is_assigned = 0'
    );
    $update->execute([$row['id']]);

    if ($update->rowCount() !== 1) {
        $pdo->rollBack();
        echo json_encode([
            'status' => 'error',
            'message' => 'Name assignment failed, please try again',
        ]);
        exit;
    }

    $pdo->commit();
    $_SESSION['funky_name'] = $row['funky_name'];

    echo json_encode([
        'status' => 'success',
        'name' => $row['funky_name'],
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

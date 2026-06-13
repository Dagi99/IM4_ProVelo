<?php
/**
 * Delete a single highscores row (admin: any logged-in user).
 */
ini_set('session.cookie_httponly', 1);
session_start();
header('Content-Type: application/json');
require_once '../../system/config.php';

if (!isset($_SESSION['user_id'])) {
    http_response_code(401);
    echo json_encode(['status' => 'error', 'message' => 'Unauthorized']);
    exit;
}

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['status' => 'error', 'message' => 'Invalid request method']);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?? [];
$id = (int) ($data['id'] ?? 0);

if ($id <= 0) {
    echo json_encode(['status' => 'error', 'message' => 'Invalid id']);
    exit;
}

try {
    $stmt = $pdo->prepare('DELETE FROM highscores WHERE id = ?');
    $stmt->execute([$id]);

    echo json_encode([
        'status' => 'success',
        'deleted' => $stmt->rowCount() > 0,
    ]);
} catch (PDOException $e) {
    echo json_encode([
        'status' => 'error',
        'message' => $e->getMessage(),
    ]);
}

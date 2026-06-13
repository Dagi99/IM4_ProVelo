<?php
/**
 * save-highscore.php — Speichert Challenge-Ergebnis nur bei neuem Personalbest (Spieler + Bike).
 */
ini_set('session.cookie_httponly', 1);
session_start();
header('Content-Type: application/json');
require_once '../system/config.php';

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    echo json_encode(['status' => 'error', 'message' => 'Invalid request method']);
    exit;
}

$playerName = trim($_SESSION['player_name'] ?? '');
if ($playerName === '') {
    echo json_encode(['status' => 'error', 'message' => 'No player session. Set a name first.']);
    exit;
}

$data = json_decode(file_get_contents('php://input'), true) ?? [];
$distanceKm = (float) ($data['distance_km'] ?? 0);
$veloId = (int) ($data['velo_id'] ?? 2);

if ($distanceKm <= 0) {
    echo json_encode(['status' => 'skipped', 'message' => 'No distance to save']);
    exit;
}

if (!in_array($veloId, [1, 2], true)) {
    $veloId = 2;
}

$playerName = mb_substr($playerName, 0, 32);

try {
    $bestStmt = $pdo->prepare(
        'SELECT MAX(distance_km) AS best FROM highscores WHERE player_name = ? AND velo_id = ?'
    );
    $bestStmt->execute([$playerName, $veloId]);
    $previousBest = $bestStmt->fetchColumn();
    $previousBest = $previousBest !== false && $previousBest !== null
        ? (float) $previousBest
        : 0.0;

    if ($distanceKm <= $previousBest) {
        echo json_encode([
            'status' => 'skipped',
            'message' => 'Not a new personal best',
            'previous_best_km' => $previousBest,
            'distance_km' => $distanceKm,
        ]);
        exit;
    }

    $insert = $pdo->prepare(
        'INSERT INTO highscores (player_name, velo_id, distance_km) VALUES (?, ?, ?)'
    );
    $insert->execute([$playerName, $veloId, round($distanceKm, 3)]);

    echo json_encode([
        'status' => 'success',
        'saved' => true,
        'name' => $playerName,
        'velo_id' => $veloId,
        'distance_km' => round($distanceKm, 3),
        'previous_best_km' => $previousBest,
    ]);
} catch (PDOException $e) {
    echo json_encode([
        'status' => 'error',
        'message' => $e->getMessage(),
    ]);
}

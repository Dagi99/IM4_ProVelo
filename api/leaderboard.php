<?php
/**
 * Leaderboard entries (best distance per player per bike).
 * GET ?period=today|alltime
 */
header('Content-Type: application/json');
require_once '../system/config.php';

$period = $_GET['period'] ?? 'today';
if (!in_array($period, ['today', 'alltime'], true)) {
    $period = 'today';
}

try {
    $dateFilter = $period === 'today'
        ? 'AND DATE(created_at) = CURDATE()'
        : '';

    $sql = "
        SELECT
            player_name,
            velo_id,
            MAX(distance_km) AS distance_km
        FROM highscores
        WHERE 1 = 1 {$dateFilter}
        GROUP BY player_name, velo_id
        ORDER BY distance_km DESC
        LIMIT 50
    ";

    $stmt = $pdo->query($sql);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $entries = array_map(static function (array $row): array {
        return [
            'name' => $row['player_name'],
            'bike' => (int) $row['velo_id'] === 2 ? 'Bike B' : 'Bike A',
            'velo_id' => (int) $row['velo_id'],
            'distance_km' => (float) $row['distance_km'],
        ];
    }, $rows);

    echo json_encode([
        'status' => 'success',
        'period' => $period,
        'entries' => $entries,
    ]);
} catch (PDOException $e) {
    echo json_encode([
        'status' => 'error',
        'message' => $e->getMessage(),
    ]);
}

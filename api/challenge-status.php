<?php
/**
 * Server-authoritative race status:
 * - starts when both bikes have assigned names AND both are present
 * - computes live speed/top speed/distance (trapezoidal integration) per bike
 * - saves results once when finished (idempotent)
 */
header('Content-Type: application/json');
require_once '../system/config.php';

const DURATION_S = 90;
const PRESENCE_WINDOW_S = 5;

$veloId = (int) ($_GET['velo_id'] ?? 0);
if (!in_array($veloId, [1, 2], true)) {
    echo json_encode(['status' => 'error', 'message' => 'velo_id must be 1 or 2']);
    exit;
}

function isoUtc(?string $mysqlTs): ?string
{
    if (!$mysqlTs) return null;
    $dt = new DateTime($mysqlTs, new DateTimeZone('UTC'));
    return $dt->format('Y-m-d\TH:i:s\Z');
}

function fetchLatestAssigned(PDO $pdo, int $veloId): ?array
{
    $stmt = $pdo->prepare(
        'SELECT funky_name, assigned_at
         FROM assigned_names
         WHERE velo_id = ? AND is_assigned = 1
         ORDER BY assigned_at DESC
         LIMIT 1'
    );
    $stmt->execute([$veloId]);
    $row = $stmt->fetch(PDO::FETCH_ASSOC);
    return $row ?: null;
}

function isPresent(PDO $pdo, int $veloId): bool
{
    $stmt = $pdo->prepare(
        'SELECT last_seen_at
         FROM challenge_presence
         WHERE velo_id = ?
         LIMIT 1'
    );
    $stmt->execute([$veloId]);
    $ts = $stmt->fetchColumn();
    if (!$ts) return false;
    $last = new DateTime((string) $ts);
    $cutoff = new DateTime('-' . PRESENCE_WINDOW_S . ' seconds');
    return $last >= $cutoff;
}

/**
 * @return array{speed_kmh: float, top_speed_kmh: float, distance_km: float}
 */
function computeMetrics(PDO $pdo, int $veloId, DateTime $start, DateTime $end): array
{
    $stmt = $pdo->prepare(
        'SELECT time, speed
         FROM speed
         WHERE velo_id = ?
           AND time >= ?
           AND time <= ?
         ORDER BY time ASC'
    );
    $stmt->execute([
        $veloId,
        $start->format('Y-m-d H:i:s'),
        $end->format('Y-m-d H:i:s'),
    ]);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);

    $top = 0.0;
    $distanceKm = 0.0;
    $current = 0.0;

    $prevT = null;
    $prevV = null;

    foreach ($rows as $row) {
        $t = new DateTime($row['time']);
        $v = (float) $row['speed'];
        if ($v > $top) $top = $v;
        $current = $v;

        if ($prevT !== null && $prevV !== null) {
            $dt = $t->getTimestamp() - $prevT->getTimestamp();
            if ($dt > 0 && $dt < 60) { // ignore huge gaps
                $avgV = ($prevV + $v) / 2.0; // km/h
                $distanceKm += $avgV * ($dt / 3600.0);
            }
        }
        $prevT = $t;
        $prevV = $v;
    }

    return [
        'speed_kmh' => round($current, 1),
        'top_speed_kmh' => round($top, 1),
        'distance_km' => round($distanceKm, 3),
    ];
}

try {
    $pdo->query('DELETE FROM speed WHERE time < NOW() - INTERVAL 15 MINUTE');

    $nameA = fetchLatestAssigned($pdo, 1);
    $nameB = fetchLatestAssigned($pdo, 2);

    $presentA = isPresent($pdo, 1);
    $presentB = isPresent($pdo, 2);

    $namesReady = $nameA && $nameB;
    $presenceReady = $presentA && $presentB;
    $ready = (bool) ($namesReady && $presenceReady);

    $pdo->beginTransaction();
    $stateStmt = $pdo->query('SELECT * FROM challenge_state WHERE id = 1 FOR UPDATE');
    $state = $stateStmt->fetch(PDO::FETCH_ASSOC) ?: [];

    $startedAt = $state['started_at'] ?? null;
    $aAssignedAt = $state['a_assigned_at'] ?? null;
    $bAssignedAt = $state['b_assigned_at'] ?? null;
    $savedFor = $state['results_saved_for_started_at'] ?? null;

    if (!$ready) {
        // Don't start until both are ready. If a challenge already started, keep it running.
        // (Presence can flap due to Wi-Fi; the timer remains server-authoritative once started.)
    } else {
        if ($startedAt === null) {
            $pdo->prepare(
                'UPDATE challenge_state
                 SET started_at = NOW(),
                     a_assigned_at = ?,
                     b_assigned_at = ?,
                     results_saved_for_started_at = NULL
                 WHERE id = 1'
            )->execute([$nameA['assigned_at'], $nameB['assigned_at']]);
            $startedAt = (new DateTime('now'))->format('Y-m-d H:i:s');
            $aAssignedAt = $nameA['assigned_at'];
            $bAssignedAt = $nameB['assigned_at'];
            $savedFor = null;
        } else {
            // Auto-reset once finished and BOTH bikes have NEW names.
            $startDt = new DateTime($startedAt);
            $finishDt = (clone $startDt)->modify('+' . DURATION_S . ' seconds');
            $now = new DateTime('now');
            $finished = $now >= $finishDt;

            $newNames = $nameA['assigned_at'] > (string) $aAssignedAt && $nameB['assigned_at'] > (string) $bAssignedAt;
            if ($finished && $newNames) {
                $pdo->prepare(
                    'UPDATE challenge_state
                     SET started_at = NOW(),
                         a_assigned_at = ?,
                         b_assigned_at = ?,
                         results_saved_for_started_at = NULL
                     WHERE id = 1'
                )->execute([$nameA['assigned_at'], $nameB['assigned_at']]);
                $startedAt = (new DateTime('now'))->format('Y-m-d H:i:s');
                $aAssignedAt = $nameA['assigned_at'];
                $bAssignedAt = $nameB['assigned_at'];
                $savedFor = null;
            }
        }
    }

    $pdo->commit();

    $stateStr = 'waiting';
    $remaining = null;
    $metricsA = ['speed_kmh' => 0.0, 'top_speed_kmh' => 0.0, 'distance_km' => 0.0];
    $metricsB = ['speed_kmh' => 0.0, 'top_speed_kmh' => 0.0, 'distance_km' => 0.0];

    if ($startedAt !== null) {
        $start = new DateTime($startedAt);
        $now = new DateTime('now');
        $end = (clone $start)->modify('+' . DURATION_S . ' seconds');
        $windowEnd = $now < $end ? $now : $end;

        $elapsed = $now->getTimestamp() - $start->getTimestamp();
        $remaining = max(0.0, DURATION_S - $elapsed);
        $stateStr = $remaining <= 0 ? 'finished' : 'running';

        $metricsA = computeMetrics($pdo, 1, $start, $windowEnd);
        $metricsB = computeMetrics($pdo, 2, $start, $windowEnd);

        // Save results once when finished.
        if ($stateStr === 'finished') {
            try {
                $pdo->beginTransaction();
                $stateStmt = $pdo->query('SELECT started_at, results_saved_for_started_at FROM challenge_state WHERE id = 1 FOR UPDATE');
                $st = $stateStmt->fetch(PDO::FETCH_ASSOC) ?: [];
                $stStarted = $st['started_at'] ?? null;
                $stSavedFor = $st['results_saved_for_started_at'] ?? null;

                if ($stStarted !== null && $stSavedFor !== $stStarted) {
                    // 1) Save detailed results if table exists (idempotent by UNIQUE).
                    try {
                        $ins = $pdo->prepare(
                            'INSERT INTO challenge_results (started_at, velo_id, player_name, distance_km, top_speed_kmh)
                             VALUES (?, ?, ?, ?, ?)'
                        );
                        $ins->execute([$stStarted, 1, $nameA['funky_name'], $metricsA['distance_km'], $metricsA['top_speed_kmh']]);
                        $ins->execute([$stStarted, 2, $nameB['funky_name'], $metricsB['distance_km'], $metricsB['top_speed_kmh']]);
                    } catch (PDOException $e) {
                        // challenge_results may not exist; ignore if not used.
                    }

                    // 2) Save distance leaderboard using existing highscores table (best-only logic handled in save-highscore.php,
                    // but we replicate minimal best logic here to avoid client-side posts).
                    $bestStmt = $pdo->prepare('SELECT MAX(distance_km) AS best FROM highscores WHERE player_name = ? AND velo_id = ?');
                    $insertHs = $pdo->prepare('INSERT INTO highscores (player_name, velo_id, distance_km) VALUES (?, ?, ?)');

                    foreach ([[1, $nameA['funky_name'], $metricsA['distance_km']], [2, $nameB['funky_name'], $metricsB['distance_km']]] as $row) {
                        [$vId, $pName, $dist] = $row;
                        if ($dist <= 0) continue;
                        $bestStmt->execute([$pName, $vId]);
                        $prev = $bestStmt->fetchColumn();
                        $prev = $prev !== false && $prev !== null ? (float) $prev : 0.0;
                        if ($dist > $prev) {
                            $insertHs->execute([$pName, $vId, $dist]);
                        }
                    }

                    $pdo->prepare('UPDATE challenge_state SET results_saved_for_started_at = ? WHERE id = 1')->execute([$stStarted]);
                }

                $pdo->commit();
            } catch (PDOException $e) {
                if ($pdo->inTransaction()) $pdo->rollBack();
            }
        }
    }

    $player = $veloId === 1 ? $metricsA : $metricsB;
    $opponent = $veloId === 1 ? $metricsB : $metricsA;

    echo json_encode([
        'status' => 'success',
        'state' => $stateStr,
        'duration_s' => DURATION_S,
        'started_at' => isoUtc($startedAt),
        'remaining_s' => $remaining !== null ? round($remaining, 1) : null,
        'ready' => $ready,
        'presence' => ['1' => $presentA, '2' => $presentB],
        'names' => [
            '1' => $nameA['funky_name'] ?? null,
            '2' => $nameB['funky_name'] ?? null,
        ],
        'player' => array_merge(['velo_id' => $veloId], $player),
        'opponent' => array_merge(['velo_id' => $veloId === 1 ? 2 : 1], $opponent),
    ]);
} catch (PDOException $e) {
    if ($pdo->inTransaction()) {
        $pdo->rollBack();
    }
    echo json_encode(['status' => 'error', 'message' => $e->getMessage()]);
}


<?php
/**
 * player-session.php — Spielername in PHP-Session (GET: lesen/erzeugen, POST: setzen, max. 16 Zeichen).
 */
ini_set('session.cookie_httponly', 1);
session_start();
header('Content-Type: application/json');

const MAX_NAME_LENGTH = 16;

$randomNames = [
    'Lina', 'Max', 'Jules', 'Nova', 'Kai', 'Mira', 'Leo', 'Zoe',
    'Finn', 'Ava', 'Ben', 'Luz', 'Nico', 'Sara', 'Tim', 'Ella',
    'Alex', 'Mia', 'Noah', 'Lia', 'Emil', 'Nina', 'Jan', 'Romy',
];

function randomPlayerName(array $pool): string
{
    return $pool[array_rand($pool)];
}

function sanitizeName(string $name): string
{
    $name = trim($name);
    $name = preg_replace('/\s+/', ' ', $name);
    return mb_substr($name, 0, MAX_NAME_LENGTH);
}

if ($_SERVER['REQUEST_METHOD'] === 'GET') {
    if (empty($_SESSION['player_name'])) {
        $_SESSION['player_name'] = randomPlayerName($randomNames);
    }

    echo json_encode([
        'status' => 'success',
        'name' => $_SESSION['player_name'],
    ]);
    exit;
}

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $data = json_decode(file_get_contents('php://input'), true) ?? [];
    $name = sanitizeName((string) ($data['name'] ?? ''));

    if ($name === '') {
        echo json_encode(['status' => 'error', 'message' => 'Name is required']);
        exit;
    }

    $_SESSION['player_name'] = $name;

    echo json_encode([
        'status' => 'success',
        'name' => $name,
    ]);
    exit;
}

echo json_encode(['status' => 'error', 'message' => 'Invalid request method']);

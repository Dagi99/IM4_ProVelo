<?php
session_start();
header('Content-Type: application/json');

if (!empty($_SESSION['funky_name'])) {
    echo json_encode([
        'status' => 'success',
        'name' => $_SESSION['funky_name'],
    ]);
    exit;
}

echo json_encode([
    'status' => 'error',
    'message' => 'No player name assigned',
]);

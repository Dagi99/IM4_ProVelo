const MAX_ANGLE = 45;
const MAX_DIFF_KM = 0.05; // 50m gap -> full needle deflection

function parseVeloId() {
    const velo = parseInt(new URLSearchParams(window.location.search).get("velo"), 10);
    return velo === 1 || velo === 2 ? velo : null;
}

const veloId = parseVeloId();
if (!veloId) {
    window.location.replace("infoBikeB.html");
    throw new Error("Missing velo parameter");
}

const PLAYER_VELO_ID = veloId;
const OPPONENT_VELO_ID = veloId === 1 ? 2 : 1;

const needle = document.querySelector("#gauge-needle");
const timerEl = document.getElementById("timer");
const bikePos = document.getElementById("bikePos");
const raceBarFill = document.getElementById("race-bar-fill");

const playerBadgeLabel = document.getElementById("player-badge-label");
const playerNameBadge = document.getElementById("player-name-badge");

const playerLabel = document.getElementById("player-label");
const playerName = document.getElementById("player-name");
const playerDistanceEl = document.getElementById("duel-player-distance");
const playerBarEl = document.getElementById("duel-bar-player");

const opponentLabel = document.getElementById("opponent-label");
const opponentName = document.getElementById("opponent-name");
const opponentDistanceEl = document.getElementById("duel-opponent-distance");
const opponentBarEl = document.getElementById("duel-bar-opponent");
const duelLead = document.getElementById("duel-lead");
const duelCard = document.getElementById("duel-card");

const statSpeed = document.getElementById("speed");
const statDistance = document.getElementById("stat-distance");
const statTopSpeed = document.getElementById("stat-top-speed");
const raceStatusEl = document.getElementById("race-status");

let redirectedToLeaderboard = false;
let lastState = null;
let lastStartedAt = null;

function clamp(value, min, max) {
    return Math.max(min, Math.min(max, value));
}

function updateGaugeByLead(diffKmOpponentMinusPlayer) {
    if (!needle) return;
    const angle = clamp((diffKmOpponentMinusPlayer / MAX_DIFF_KM) * MAX_ANGLE, -MAX_ANGLE, MAX_ANGLE);
    needle.style.transform = `translateX(-50%) rotate(${angle}deg)`;
}

function setTimerAndProgress(remainingS, durationS) {
    if (timerEl && remainingS !== null && remainingS !== undefined) {
        timerEl.textContent = remainingS.toFixed(1) + "s";
    }

    const progressPercent = remainingS === null || remainingS === undefined
        ? 0
        : clamp(((durationS - remainingS) / durationS) * 100, 0, 100);
    const bikeLeft = remainingS !== null && remainingS <= 0 ? 100 : 2 + progressPercent * 0.96;

    if (raceBarFill) raceBarFill.style.width = progressPercent + "%";
    if (bikePos) bikePos.style.left = bikeLeft + "%";
}

function formatDistance(km) {
    return (km || 0).toFixed(2) + " km";
}

function renderStatus(status) {
    const player = status.player;
    const opponent = status.opponent;

    if (raceStatusEl) {
        if (status.state === "waiting") {
            raceStatusEl.textContent = "Warte auf den Gegner…";
        } else if (status.state === "running") {
            raceStatusEl.textContent = "Challenge läuft";
        } else if (status.state === "finished") {
            raceStatusEl.textContent = "Challenge beendet";
        }
    }

    // Labels
    const playerBikeLabel = PLAYER_VELO_ID === 1 ? "BIKE A" : "BIKE B";
    const opponentBikeLabel = OPPONENT_VELO_ID === 1 ? "Bike A" : "Bike B";
    if (playerBadgeLabel) playerBadgeLabel.textContent = playerBikeLabel;
    if (playerLabel) playerLabel.textContent = opponentBikeLabel === "Bike A" ? "Bike B" : "Bike A";
    if (opponentLabel) opponentLabel.textContent = opponentBikeLabel;

    // Names
    if (playerNameBadge) playerNameBadge.textContent = status.names?.[String(PLAYER_VELO_ID)] || "";
    if (playerName) playerName.textContent = status.names?.[String(PLAYER_VELO_ID)] || "…";
    if (opponentName) opponentName.textContent = status.names?.[String(OPPONENT_VELO_ID)] || "…";

    // Timer/progress
    setTimerAndProgress(status.remaining_s, status.duration_s);

    // Reset redirect guard when a new challenge starts
    if (status.started_at && status.started_at !== lastStartedAt) {
        lastStartedAt = status.started_at;
        redirectedToLeaderboard = false;
    }

    // Stat card (player)
    if (statSpeed) statSpeed.textContent = (player.speed_kmh || 0).toFixed(1);
    if (statDistance) statDistance.textContent = formatDistance(player.distance_km || 0);
    if (statTopSpeed) statTopSpeed.textContent = (player.top_speed_kmh || 0).toFixed(1) + " km/h";

    // Duel card (both)
    const distPlayer = player.distance_km || 0;
    const distOpponent = opponent.distance_km || 0;

    if (playerDistanceEl) playerDistanceEl.textContent = formatDistance(distPlayer);
    if (opponentDistanceEl) opponentDistanceEl.textContent = formatDistance(distOpponent);

    const maxDist = Math.max(distPlayer, distOpponent, 0.0001);
    if (playerBarEl) playerBarEl.style.width = (distPlayer / maxDist) * 100 + "%";
    if (opponentBarEl) opponentBarEl.style.width = (distOpponent / maxDist) * 100 + "%";

    if (duelLead) {
        if (distPlayer > distOpponent) duelLead.textContent = "Du führst!";
        else if (distOpponent > distPlayer) duelLead.textContent = "Du liegst zurück!";
        else duelLead.textContent = "Gleichstand!";
    }

    if (duelCard) {
        duelCard.classList.toggle("duel-card--opponent-a", OPPONENT_VELO_ID === 1);
        duelCard.classList.toggle("duel-card--opponent-b", OPPONENT_VELO_ID === 2);
    }

    // Gauge should always point to the global leader side:
    // - left/orange when Bike A (velo_id=1) leads
    // - right/blue when Bike B (velo_id=2) leads
    const distanceA = (player.velo_id === 1 ? distPlayer : distOpponent);
    const distanceB = (player.velo_id === 2 ? distPlayer : distOpponent);
    updateGaugeByLead(distanceB - distanceA);

    // Only redirect when we observe a transition to finished (avoid immediate redirect
    // when loading a page after a previous challenge already ended).
    if (
        !redirectedToLeaderboard &&
        lastState === "running" &&
        status.state === "finished"
    ) {
        redirectedToLeaderboard = true;
        // Give the server a moment to persist results
        setTimeout(() => {
            window.location.href = "leaderboard.html";
        }, 1200);
    }

    lastState = status.state;
}

async function sendHeartbeat() {
    try {
        await fetch(`api/challenge-heartbeat.php?velo_id=${PLAYER_VELO_ID}&t=${Date.now()}`);
    } catch (e) {
        // ignore
    }
}

async function pollStatus() {
    try {
        const res = await fetch(`api/challenge-status.php?velo_id=${PLAYER_VELO_ID}&t=${Date.now()}`);
        const json = await res.json();
        if (json.status === "success") {
            renderStatus(json);
        }
    } catch (e) {
        console.error("Status poll failed:", e);
    }
}

let simulateInterval = null;

async function simulateTick() {
    try {
        await fetch("api/simulate-speed.php?t=" + Date.now());
    } catch (error) {
        console.error("Error simulating speed:", error);
    }
}

function toggleSimulation() {
    const btn = document.getElementById("simulate-btn");
    if (!btn) return;

    if (simulateInterval) {
        clearInterval(simulateInterval);
        simulateInterval = null;
        btn.textContent = "Simulate data";
        return;
    }

    simulateTick();
    simulateInterval = setInterval(simulateTick, 1000);
    btn.textContent = "Stop simulation";
}

function init() {
    document.body.dataset.veloId = String(PLAYER_VELO_ID);
    document.getElementById("simulate-btn")?.addEventListener("click", toggleSimulation);

    // Heartbeat 1s, status poll 1s (as requested)
    sendHeartbeat();
    pollStatus();
    setInterval(sendHeartbeat, 1000);
    setInterval(pollStatus, 1000);
}

init();

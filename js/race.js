const MAX_ANGLE = 45;
const MAX_SPEED_DIFF = 10;
const CHALLENGE_DURATION = 90;

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
const PLAYER_BIKE_LABEL = veloId === 1 ? "BIKE A" : "BIKE B";
const OPPONENT_BIKE_LABEL = veloId === 1 ? "Bike B" : "Bike A";

const needle = document.querySelector("#gauge-needle");
const timerEl = document.getElementById("timer");
const bikePos = document.getElementById("bikePos");
const raceBarFill = document.getElementById("race-bar-fill");
const playerBadgeLabel = document.getElementById("player-badge-label");

let distanceA = 0;
let distanceB = 0;
let topSpeedPlayer = 0;
let lastPollTime = null;
let challengeActive = true;
let remaining = CHALLENGE_DURATION;
let highscoreSaved = false;

function getDistancePlayer() {
    return PLAYER_VELO_ID === 1 ? distanceA : distanceB;
}

function getDistanceOpponent() {
    return OPPONENT_VELO_ID === 1 ? distanceA : distanceB;
}

function clamp(value, min, max) {
    return Math.max(min, Math.min(max, value));
}

function formatDistance(km) {
    return km.toFixed(2) + " km";
}

function updateTopSpeedDisplay() {
    const statTopSpeed = document.getElementById("stat-top-speed");
    if (statTopSpeed) {
        statTopSpeed.textContent = topSpeedPlayer.toFixed(1) + " km/h";
    }
}

function updateGauge(speedPlayer, speedOpponent) {
    if (!needle) return;

    const diff = speedOpponent - speedPlayer;
    const angle = clamp((diff / MAX_SPEED_DIFF) * MAX_ANGLE, -MAX_ANGLE, MAX_ANGLE);
    needle.style.transform = `translateX(-50%) rotate(${angle}deg)`;
}

function updateDistances() {
    const duelOpponentDistance = document.getElementById("duel-opponent-distance");
    const duelBarOpponent = document.getElementById("duel-bar-opponent");
    const duelLead = document.getElementById("duel-lead");
    const statDistance = document.getElementById("stat-distance");

    const distPlayer = getDistancePlayer();
    const distOpponent = getDistanceOpponent();

    if (statDistance) statDistance.textContent = formatDistance(distPlayer);
    if (duelOpponentDistance) duelOpponentDistance.textContent = formatDistance(distOpponent);

    const maxDist = Math.max(distPlayer, distOpponent, 0.0001);
    if (duelBarOpponent) {
        duelBarOpponent.style.width = (distOpponent / maxDist) * 100 + "%";
    }

    if (duelLead) {
        if (distPlayer > distOpponent) {
            duelLead.textContent = "Du führst!";
        } else if (distOpponent > distPlayer) {
            duelLead.textContent = "Du liegst zurück!";
        } else {
            duelLead.textContent = "Gleichstand!";
        }
    }
}

function integrateDistances(speedA, speedB) {
    if (!challengeActive) return;

    const now = Date.now();
    if (lastPollTime === null) {
        lastPollTime = now;
        return;
    }

    const elapsedSeconds = (now - lastPollTime) / 1000;
    lastPollTime = now;

    distanceA += speedA * (elapsedSeconds / 3600);
    distanceB += speedB * (elapsedSeconds / 3600);
    updateDistances();
}

function updateChallengeProgress() {
    const elapsed = CHALLENGE_DURATION - remaining;
    const progressPercent = Math.min(100, (elapsed / CHALLENGE_DURATION) * 100);
    const bikeLeft = remaining <= 0 ? 100 : 2 + progressPercent * 0.96;

    if (raceBarFill) {
        raceBarFill.style.width = progressPercent + "%";
    }
    if (bikePos) {
        bikePos.style.left = bikeLeft + "%";
    }
}

async function saveHighscore() {
    const distPlayer = getDistancePlayer();
    if (highscoreSaved || distPlayer <= 0) return;
    highscoreSaved = true;

    try {
        const response = await fetch("api/save-highscore.php", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({
                distance_km: distPlayer,
                velo_id: PLAYER_VELO_ID,
            }),
        });
        const result = await response.json();
        console.log("Highscore save:", result);
    } catch (error) {
        console.error("Error saving highscore:", error);
        highscoreSaved = false;
    }
}

function startChallengeTimer() {
    if (!timerEl) return;

    timerEl.textContent = remaining.toFixed(1) + "s";
    updateChallengeProgress();

    setInterval(() => {
        remaining -= 0.1;
        if (remaining <= 0) {
            remaining = 0;
            challengeActive = false;
            timerEl.textContent = "0.0s";
            updateChallengeProgress();
            saveHighscore();
            return;
        }
        timerEl.textContent = remaining.toFixed(1) + "s";
        updateChallengeProgress();
    }, 100);
}

async function loadPlayerName() {
    try {
        const response = await fetch(`api/get-player-name.php?velo_id=${PLAYER_VELO_ID}`);
        const result = await response.json();

        if (result.status === "success") {
            const playerNameBadge = document.getElementById("player-name-badge");
            if (playerNameBadge) playerNameBadge.textContent = result.name;
        }
    } catch (error) {
        console.error("Error loading player name:", error);
    }
}

async function loadOpponentName() {
    const opponentLabel = document.getElementById("opponent-label");
    const opponentNameEl = document.getElementById("opponent-name");
    const duelCard = document.getElementById("duel-card");

    try {
        const response = await fetch(`api/get-opponent-name.php?velo_id=${PLAYER_VELO_ID}`);
        const result = await response.json();

        if (result.status === "success") {
            const label = OPPONENT_BIKE_LABEL;
            if (opponentLabel) opponentLabel.textContent = label;
            if (opponentNameEl) opponentNameEl.textContent = result.name;
            if (duelCard) {
                duelCard.classList.toggle("duel-card--opponent-a", OPPONENT_VELO_ID === 1);
                duelCard.classList.toggle("duel-card--opponent-b", OPPONENT_VELO_ID === 2);
            }
        }
    } catch (error) {
        console.error("Error loading opponent name:", error);
    }
}

async function loadSpeed() {
    try {
        const response = await fetch(
            `api/updatespeed.php?velo_id=${PLAYER_VELO_ID}&t=${Date.now()}`
        );
        const result = await response.json();

        if (result.status === "success") {
            document.querySelector("#speed").textContent = result.speed;

            const speedA = parseFloat(result.speeds["1"] || 0);
            const speedB = parseFloat(result.speeds["2"] || 0);
            const speedPlayer = PLAYER_VELO_ID === 1 ? speedA : speedB;
            const speedOpponent = OPPONENT_VELO_ID === 1 ? speedA : speedB;

            if (challengeActive && speedPlayer > topSpeedPlayer) {
                topSpeedPlayer = speedPlayer;
                updateTopSpeedDisplay();
            }
            integrateDistances(speedA, speedB);
            updateGauge(speedPlayer, speedOpponent);
        }
    } catch (error) {
        console.error("Error fetching speed:", error);
    }
}

let simulateInterval = null;

async function simulateTick() {
    try {
        const response = await fetch("api/simulate-speed.php?t=" + Date.now());
        const result = await response.json();
        if (result.status === "success") {
            loadSpeed();
        }
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

function initRacePage() {
    document.body.dataset.veloId = String(PLAYER_VELO_ID);

    if (playerBadgeLabel) {
        playerBadgeLabel.textContent = PLAYER_BIKE_LABEL;
    }

    document.getElementById("simulate-btn")?.addEventListener("click", toggleSimulation);

    loadPlayerName();
    loadOpponentName();
    startChallengeTimer();
    updateDistances();
    updateTopSpeedDisplay();
    loadSpeed();
    setInterval(loadSpeed, 1000);
}

initRacePage();

const MAX_ANGLE = 45;
const MAX_SPEED_DIFF = 10;
const CHALLENGE_DURATION = 90;

const needle = document.querySelector("#gauge-needle");
const timerEl = document.getElementById("timer");
const bikePos = document.getElementById("bikePos");
const raceBarFill = document.getElementById("race-bar-fill");

let distanceA = 0;
let distanceB = 0;
let topSpeedB = 0;
let lastPollTime = null;
let challengeActive = true;
let remaining = CHALLENGE_DURATION;

function clamp(value, min, max) {
    return Math.max(min, Math.min(max, value));
}

function formatDistance(km) {
    return km.toFixed(2) + " km";
}

function updateTopSpeedDisplay() {
    const statTopSpeed = document.getElementById("stat-top-speed");
    if (statTopSpeed) {
        statTopSpeed.textContent = topSpeedB.toFixed(1) + " km/h";
    }
}

function updateGauge(speedA, speedB) {
    if (!needle) return;

    const diff = speedB - speedA;
    const angle = clamp((diff / MAX_SPEED_DIFF) * MAX_ANGLE, -MAX_ANGLE, MAX_ANGLE);
    needle.style.transform = `translateX(-50%) rotate(${angle}deg)`;
}

function updateDistances() {
    const duelDistanceA = document.getElementById("duel-distance-a");
    const duelDistanceB = document.getElementById("duel-distance-b");
    const duelBarA = document.getElementById("duel-bar-a");
    const duelBarB = document.getElementById("duel-bar-b");
    const duelLead = document.getElementById("duel-lead");
    const statDistance = document.getElementById("stat-distance");

    const distTextA = formatDistance(distanceA);
    const distTextB = formatDistance(distanceB);

    if (duelDistanceA) duelDistanceA.textContent = distTextA;
    if (duelDistanceB) duelDistanceB.textContent = distTextB;
    if (statDistance) statDistance.textContent = distTextB;

    const maxDist = Math.max(distanceA, distanceB, 0.0001);
    if (duelBarA) duelBarA.style.width = (distanceA / maxDist) * 100 + "%";
    if (duelBarB) duelBarB.style.width = (distanceB / maxDist) * 100 + "%";

    if (duelLead) {
        if (distanceB > distanceA) {
            duelLead.textContent = "Du führst!";
        } else if (distanceA > distanceB) {
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
            return;
        }
        timerEl.textContent = remaining.toFixed(1) + "s";
        updateChallengeProgress();
    }, 100);
}

async function loadSpeed() {
    try {
        const response = await fetch("api/updatespeed.php?t=" + Date.now());
        const result = await response.json();

        if (result.status === "success") {
            console.log("DEBUG - Received from DB:", result);
            document.querySelector("#speed").textContent = result.speed;

            const speedA = parseFloat(result.speeds["1"] || 0);
            const speedB = parseFloat(result.speeds["2"] || 0);
            if (challengeActive && speedB > topSpeedB) {
                topSpeedB = speedB;
                updateTopSpeedDisplay();
            }
            integrateDistances(speedA, speedB);
            updateGauge(speedA, speedB);
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

document.getElementById("simulate-btn")?.addEventListener("click", toggleSimulation);

async function loadPlayerName() {
    try {
        const response = await fetch("api/get-player-name.php");
        const result = await response.json();

        if (result.status === "success") {
            const playerName = document.getElementById("player-name");
            const playerNameBadge = document.getElementById("player-name-badge");
            if (playerName) playerName.textContent = result.name;
            if (playerNameBadge) playerNameBadge.textContent = result.name;
        }
    } catch (error) {
        console.error("Error loading player name:", error);
    }
}

loadPlayerName();
startChallengeTimer();
updateDistances();
updateTopSpeedDisplay();
loadSpeed();
setInterval(loadSpeed, 1000);

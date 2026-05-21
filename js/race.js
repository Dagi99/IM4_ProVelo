const MAX_ANGLE = 45;
const MAX_SPEED_DIFF = 10;

const needle = document.querySelector("#gauge-needle");

function clamp(value, min, max) {
    return Math.max(min, Math.min(max, value));
}

function updateGauge(speedA, speedB) {
    if (!needle) return;

    const diff = speedB - speedA;
    const angle = clamp((diff / MAX_SPEED_DIFF) * MAX_ANGLE, -MAX_ANGLE, MAX_ANGLE);
    needle.style.transform = `translateX(-50%) rotate(${angle}deg)`;
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
            updateGauge(speedA, speedB);
        }
    } catch (error) {
        console.error("Error fetching speed:", error);
    }
}

loadSpeed();
setInterval(loadSpeed, 1000);

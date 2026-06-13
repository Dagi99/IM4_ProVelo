/**
 * challengeName.js — Holt Zufallsnamen von api/assign-name.php, Weiterleitung zu race.html (?velo=1|2).
 */
const assignedNameEl = document.getElementById("assignedName");
const nameErrorEl = document.getElementById("nameError");
const continueBtn = document.getElementById("continueBtn");
const bikeBadgeEl = document.getElementById("bikeBadge");
const backLinkEl = document.getElementById("backLink");

function parseVeloId() {
    const velo = parseInt(new URLSearchParams(window.location.search).get("velo"), 10);
    return velo === 1 || velo === 2 ? velo : null;
}

const veloId = parseVeloId();

if (!veloId) {
    window.location.replace("infoBikeB.html");
} else {
    const bikeLabel = veloId === 1 ? "BIKE A" : "BIKE B";
    const infoPage = veloId === 1 ? "infoBikeA.html" : "infoBikeB.html";

    if (bikeBadgeEl) bikeBadgeEl.textContent = bikeLabel;
    if (backLinkEl) backLinkEl.href = infoPage;
    if (continueBtn) continueBtn.href = `race.html?velo=${veloId}`;

    async function assignName() {
        try {
            const response = await fetch(`api/assign-name.php?velo_id=${veloId}`);
            const result = await response.json();

            if (result.status === "success") {
                assignedNameEl.textContent = result.name;
                if (continueBtn) {
                    continueBtn.removeAttribute("aria-disabled");
                    continueBtn.classList.remove("challenge-btn--disabled");
                }
                return;
            }

            assignedNameEl.textContent = "—";
            if (nameErrorEl) {
                nameErrorEl.hidden = false;
                nameErrorEl.textContent = result.message || "Kein Name verfügbar.";
            }
        } catch (error) {
            assignedNameEl.textContent = "—";
            if (nameErrorEl) {
                nameErrorEl.hidden = false;
                nameErrorEl.textContent = "Fehler beim Laden des Namens.";
            }
            console.error("Error assigning name:", error);
        }
    }

    if (continueBtn) {
        continueBtn.addEventListener("click", (e) => {
            if (continueBtn.getAttribute("aria-disabled") === "true") {
                e.preventDefault();
            }
        });
    }

    assignName();
}

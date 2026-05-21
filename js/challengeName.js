const assignedNameEl = document.getElementById("assignedName");
const nameErrorEl = document.getElementById("nameError");
const continueBtn = document.getElementById("continueBtn");
async function assignName() {
    try {
        const response = await fetch("api/assign-name.php");
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
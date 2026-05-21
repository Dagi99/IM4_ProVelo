const input = document.getElementById('nameInput');
const count = document.getElementById('count');
const continueBtn = document.querySelector('.challenge-btn');

async function loadSessionName() {
    try {
        const response = await fetch('api/player-session.php?t=' + Date.now());
        const result = await response.json();
        if (result.status === 'success' && result.name && input) {
            input.value = result.name;
            if (count) count.textContent = String(result.name.length);
        }
    } catch (error) {
        console.error('Failed to load session name:', error);
    }
}

async function saveNameAndContinue() {
    if (!input) return;

    const name = input.value.trim();
    if (!name) return;

    try {
        const response = await fetch('api/player-session.php', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ name }),
        });
        const result = await response.json();
        if (result.status === 'success') {
            window.location.href = 'race.html';
        }
    } catch (error) {
        console.error('Failed to save name:', error);
    }
}

if (input && count) {
    input.addEventListener('input', () => {
        count.textContent = String(input.value.length);
    });
}

continueBtn?.addEventListener('click', saveNameAndContinue);
loadSessionName();

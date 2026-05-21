const MEDAL_SVG = `<svg width="18" height="18" viewBox="0 0 24 24" fill="none"
    stroke="currentColor" stroke-width="2"
    stroke-linecap="round" stroke-linejoin="round" aria-hidden="true">
    <circle cx="12" cy="8" r="3"/>
    <path d="M8 21h8"/>
    <path d="M12 11v10"/>
    <path d="M7 4H5a2 2 0 0 0-2 2v2"/>
    <path d="M17 4h2a2 2 0 0 1 2 2v2"/>
    <path d="M7 20H5a2 2 0 0 1-2-2v-2"/>
    <path d="M17 20h2a2 2 0 0 0 2-2v-2"/>
</svg>`;

const listEl = document.querySelector('.leaderboard-list');
const tabs = document.querySelectorAll('.leaderboard-tab');
let activePeriod = 'today';

function formatDistance(km) {
    if (km >= 1) {
        return km.toFixed(2) + ' km';
    }
    return (km * 1000).toFixed(0) + ' m';
}

function setActiveTab(period) {
    activePeriod = period;
    tabs.forEach((tab) => {
        tab.classList.toggle(
            'leaderboard-tab--active',
            tab.dataset.period === period
        );
    });
}

function renderCard(entry, rank) {
    const card = document.createElement('div');
    card.className = 'leaderboard-card';
    card.innerHTML = `
        <div class="leaderboard-card__left">
            <div class="leaderboard-card__icon" aria-hidden="true">
                ${MEDAL_SVG}
            </div>
            <div class="leaderboard-card__info">
                <div class="leaderboard-card__name"></div>
                <div class="leaderboard-card__bike"></div>
            </div>
        </div>
        <div class="leaderboard-card__distance"></div>
    `;

    card.querySelector('.leaderboard-card__name').textContent = entry.name;
    card.querySelector('.leaderboard-card__bike').textContent = entry.bike;
    card.querySelector('.leaderboard-card__distance').textContent = formatDistance(
        entry.distance_km
    );
    card.querySelector('.leaderboard-card__icon').setAttribute(
        'aria-label',
        'Platz ' + rank
    );

    return card;
}

function renderEmpty(message) {
    listEl.innerHTML = '';
    const empty = document.createElement('p');
    empty.className = 'leaderboard-empty';
    empty.textContent = message;
    listEl.appendChild(empty);
}

async function loadLeaderboard(period) {
    if (!listEl) return;

    listEl.innerHTML = '<p class="leaderboard-empty">Laden…</p>';

    try {
        const response = await fetch(
            'api/leaderboard.php?period=' + encodeURIComponent(period) + '&t=' + Date.now()
        );
        const result = await response.json();

        if (result.status !== 'success') {
            renderEmpty('Rangliste konnte nicht geladen werden.');
            return;
        }

        if (!result.entries || result.entries.length === 0) {
            renderEmpty(
                period === 'today'
                    ? 'Heute noch keine Highscores.'
                    : 'Noch keine Highscores vorhanden.'
            );
            return;
        }

        listEl.innerHTML = '';
        result.entries.forEach((entry, index) => {
            listEl.appendChild(renderCard(entry, index + 1));
        });
    } catch (error) {
        console.error('Leaderboard load error:', error);
        renderEmpty('Rangliste konnte nicht geladen werden.');
    }
}

tabs.forEach((tab) => {
    tab.addEventListener('click', () => {
        const period = tab.dataset.period;
        if (!period || period === activePeriod) return;
        setActiveTab(period);
        loadLeaderboard(period);
    });
});

setActiveTab('today');
loadLeaderboard('today');

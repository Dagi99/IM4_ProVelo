/**
 * admin.js — Lädt Highscore-Liste, löscht Einträge via api/admin/delete-highscore.php (admin.html).
 */
const listEl = document.querySelector('.admin-list');
const tabs = document.querySelectorAll('.leaderboard-tab');
let activePeriod = 'today';

function formatDistance(km) {
    if (km >= 1) {
        return km.toFixed(2) + ' km';
    }
    return (km * 1000).toFixed(0) + ' m';
}

function formatDate(iso) {
    if (!iso) return '';
    const date = new Date(iso.replace(' ', 'T'));
    if (Number.isNaN(date.getTime())) return iso;
    return date.toLocaleString('de-CH', {
        day: '2-digit',
        month: '2-digit',
        year: 'numeric',
        hour: '2-digit',
        minute: '2-digit',
    });
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

function renderEmpty(message) {
    listEl.innerHTML = '';
    const empty = document.createElement('p');
    empty.className = 'leaderboard-empty';
    empty.textContent = message;
    listEl.appendChild(empty);
}

function renderCard(entry) {
    const card = document.createElement('div');
    card.className = 'leaderboard-card';
    card.dataset.id = String(entry.id);

    card.innerHTML = `
        <div class="leaderboard-card__left">
            <div class="leaderboard-card__info">
                <div class="leaderboard-card__name"></div>
                <div class="leaderboard-card__bike"></div>
                <div class="admin-meta">ID ${entry.id} · <span class="admin-date"></span></div>
            </div>
        </div>
        <div class="admin-card__right">
            <div class="leaderboard-card__distance"></div>
            <button type="button" class="btn btn-danger-outline admin-delete-btn">Löschen</button>
        </div>
    `;

    card.querySelector('.leaderboard-card__name').textContent = entry.name;
    card.querySelector('.leaderboard-card__bike').textContent = entry.bike;
    card.querySelector('.leaderboard-card__distance').textContent = formatDistance(entry.distance_km);
    card.querySelector('.admin-date').textContent = formatDate(entry.created_at);

    card.querySelector('.admin-delete-btn').addEventListener('click', () => {
        deleteEntry(entry.id, entry.name);
    });

    return card;
}

async function loadEntries(period) {
    if (!listEl) return;

    listEl.innerHTML = '<p class="leaderboard-empty">Laden…</p>';

    try {
        const response = await fetch(
            'api/leaderboard.php?period=' + encodeURIComponent(period) + '&t=' + Date.now(),
            { credentials: 'include' }
        );
        const result = await response.json();

        if (result.status !== 'success') {
            renderEmpty('Einträge konnten nicht geladen werden.');
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
        result.entries.forEach((entry) => {
            listEl.appendChild(renderCard(entry));
        });
    } catch (error) {
        console.error('Admin load error:', error);
        renderEmpty('Einträge konnten nicht geladen werden.');
    }
}

async function deleteEntry(id, name) {
    const confirmed = window.confirm(
        'Eintrag für "' + name + '" (ID ' + id + ') wirklich löschen?'
    );
    if (!confirmed) return;

    try {
        const response = await fetch('api/admin/delete-highscore.php', {
            method: 'POST',
            credentials: 'include',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ id }),
        });

        if (response.status === 401) {
            window.location.href = 'login.html';
            return;
        }

        const result = await response.json();

        if (result.status === 'success' && result.deleted) {
            await loadEntries(activePeriod);
            return;
        }

        window.alert('Löschen fehlgeschlagen.');
    } catch (error) {
        console.error('Delete error:', error);
        window.alert('Löschen fehlgeschlagen.');
    }
}

tabs.forEach((tab) => {
    tab.addEventListener('click', () => {
        const period = tab.dataset.period;
        if (!period || period === activePeriod) return;
        setActiveTab(period);
        loadEntries(period);
    });
});

window.addEventListener('load', async () => {
    const ok = await checkAuth();
    if (!ok) return;

    setActiveTab('today');
    loadEntries('today');
});

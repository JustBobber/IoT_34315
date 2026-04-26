// ── GenStraek · Session Bar Chart ────────────────────────────────────────────

// ?? Colours (matching GenStraek palette) ??????????????????????????????
const C = {
    bar:        '#f7931a',
    barHover:   '#f57c00',
    barStroke:  '#f57c00',
    fill:       'rgba(247, 147, 26, 0.12)',
    grid:       '#d7d7d7',
    axis:       '#a5a5a5',
    label:      '#555555',
    arrow:      '#a5a5a5',
};

// ── Date & duration helpers (matching original) ────────────────────────────
function parseDate(str) {
    // Handles "2026-04-24 00:13:32" and ISO formats
    return new Date(str.replace(' ', 'T'));
}
function formatDateShort(str) {
    const d = parseDate(str);
    if (isNaN(d)) return str;
    return `${d.getDate()}/${d.getMonth() + 1}`;
}
function formatDateFull(str) {
    const d = parseDate(str);
    if (isNaN(d)) return str;
    const pad = n => String(n).padStart(2, '0');
    return `${d.getFullYear()}-${pad(d.getMonth()+1)}-${pad(d.getDate())}  ${pad(d.getHours())}:${pad(d.getMinutes())}`;
}

// ── Main ───────────────────────────────────────────────────────────────────
(function () {
    const canvas  = document.getElementById('sessionChart');
    const tooltip = document.getElementById('chartTooltip');
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    const data = (typeof sessionData !== 'undefined' && sessionData.length)
        ? sessionData
        : [];

    // ── Layout constants ──────────────────────────────────────────────────
    const PAD   = { top: 28, right: 24, bottom: 52, left: 52 };
    const BAR_RADIUS = 5;
    const ANIM_MS    = 520;

    let W, H, chartW, chartH;
    let hoveredIndex = -1;
    let animProgress = 0;
    let animStart    = null;

    function resize() {
        const dpr = window.devicePixelRatio || 1;
        const rect = canvas.parentElement.getBoundingClientRect();
        W = rect.width  || 600;
        H = rect.height || 300;
        canvas.width  = W * dpr;
        canvas.height = H * dpr;
        canvas.style.width  = W + 'px';
        canvas.style.height = H + 'px';
        ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
        chartW = W - PAD.left - PAD.right;
        chartH = H - PAD.top  - PAD.bottom;
    }

    // ── Rounded-top rectangle helper ─────────────────────────────────────
    function roundedBar(x, y, w, h, r) {
        r = Math.min(r, h / 2, w / 2);
        ctx.beginPath();
        ctx.moveTo(x + r, y);
        ctx.lineTo(x + w - r, y);
        ctx.quadraticCurveTo(x + w, y, x + w, y + r);
        ctx.lineTo(x + w, y + h);
        ctx.lineTo(x, y + h);
        ctx.lineTo(x, y + r);
        ctx.quadraticCurveTo(x, y, x + r, y);
        ctx.closePath();
    }

    // ── Draw ──────────────────────────────────────────────────────────────
    function draw(progress) {
        ctx.clearRect(0, 0, W, H);

        if (!data.length) {
            ctx.fillStyle = C.label;
            ctx.font = '14px sans-serif';
            ctx.textAlign = 'center';
            ctx.fillText('No session data yet', W / 2, H / 2);
            return;
        }

        const maxVal = Math.max(...data.map(d => d.max_distance || 0), 1);
        const niceMax = Math.ceil(maxVal / 10) * 10;

        // Y-axis ticks
        const TICK_COUNT = 5;
        const tickStep   = niceMax / TICK_COUNT;

        ctx.save();
        ctx.translate(PAD.left, PAD.top);

        // ── Grid lines ────────────────────────────────────────────────────
        ctx.strokeStyle = C.grid;
        ctx.lineWidth   = 1;
        ctx.setLineDash([4, 4]);
        for (let i = 0; i <= TICK_COUNT; i++) {
            const y = chartH - (i / TICK_COUNT) * chartH;
            ctx.beginPath();
            ctx.moveTo(0, y);
            ctx.lineTo(chartW, y);
            ctx.stroke();
        }
        ctx.setLineDash([]);

        // ── Y-axis labels ─────────────────────────────────────────────────
        ctx.fillStyle  = C.label;
        ctx.font       = '11px sans-serif';
        ctx.textAlign  = 'right';
        ctx.textBaseline = 'middle';
        for (let i = 0; i <= TICK_COUNT; i++) {
            const val = Math.round(i * tickStep);
            const y   = chartH - (i / TICK_COUNT) * chartH;
            ctx.fillText(val + ' cm', -8, y);
        }

        // ── Y-axis label ("Max distance") ─────────────────────────────────
        ctx.save();
        ctx.translate(-38, chartH / 2);
        ctx.rotate(-Math.PI / 2);
        ctx.fillStyle = C.axis;
        ctx.font = '11px sans-serif';
        ctx.textAlign = 'center';
        ctx.fillText('Max distance (cm)', 0, 0);
        ctx.restore();

        // ── Bars ──────────────────────────────────────────────────────────
        const n        = data.length;
        const BAR_GAP  = Math.max(4, chartW * 0.02);
        const barW     = (chartW - BAR_GAP * (n + 1)) / n;

        data.forEach((session, i) => {
            const val      = session.max_distance || 0;
            const fullH    = (val / niceMax) * chartH;
            const animH    = fullH * progress;
            const x        = BAR_GAP + i * (barW + BAR_GAP);
            const y        = chartH - animH;
            const isHover  = i === hoveredIndex;

            // Bar fill
            ctx.fillStyle = isHover ? C.barHover : C.bar;
            ctx.globalAlpha = isHover ? 1 : 0.88;
            roundedBar(x, y, barW, animH, BAR_RADIUS);
            ctx.fill();

            // Bar stroke
            ctx.globalAlpha = 1;
            ctx.strokeStyle = C.barStroke;
            ctx.lineWidth   = isHover ? 2 : 1;
            roundedBar(x, y, barW, animH, BAR_RADIUS);
            ctx.stroke();

            // Value label on top of bar (only when animation is near done)
            if (progress > 0.85 && animH > 18) {
                ctx.fillStyle    = C.barHover;
                ctx.font         = `${isHover ? 'bold ' : ''}11px sans-serif`;
                ctx.textAlign    = 'center';
                ctx.textBaseline = 'bottom';
                ctx.fillText(val.toFixed(1), x + barW / 2, y - 3);
            }

            // X-axis date label
            if (progress > 0.6) {
                ctx.fillStyle    = C.label;
                ctx.font         = '10px sans-serif';
                ctx.textAlign    = 'center';
                ctx.textBaseline = 'top';
                ctx.fillText(formatDateShort(session.start_time), x + barW / 2, chartH + 8);
            }
        });

        ctx.restore();
    }

    // ── Tooltip ───────────────────────────────────────────────────────────
    function showTooltip(e, idx) {
        if (!tooltip) return;
        const s    = data[idx];
        const diff = s.avg_difficulty > 0 ? s.avg_difficulty.toFixed(2) : '?';
        tooltip.innerHTML =
            `<strong>${formatDateFull(s.start_time)}</strong><br>` +
            `Distance: ${s.max_distance} cm<br>` +
            (s.duration ? `Duration: ${s.duration}<br>` : '') +
            `Avg difficulty: ${diff}`;
        tooltip.classList.add('visible');
        moveTooltip(e);
    }
    function moveTooltip(e) {
        const tw = tooltip.offsetWidth  || 200;
        const th = tooltip.offsetHeight || 80;
        let x = e.clientX + 14;
        let y = e.clientY - th / 2;
        if (x + tw > window.innerWidth  - 12) x = e.clientX - tw - 14;
        if (y < 8)                             y = 8;
        if (y + th > window.innerHeight - 8)   y = window.innerHeight - th - 8;
        tooltip.style.left = x + 'px';
        tooltip.style.top  = y + 'px';
    }
    function hideTooltip() {
        if (tooltip) tooltip.classList.remove('visible');
    }

    // ── Hit test ──────────────────────────────────────────────────────────
    function hitIndex(mx, my) {
        if (!data.length) return -1;
        const n       = data.length;
        const BAR_GAP = Math.max(4, chartW * 0.02);
        const barW    = (chartW - BAR_GAP * (n + 1)) / n;
        const ox = PAD.left, oy = PAD.top;
        const maxVal  = Math.max(...data.map(d => d.max_distance || 0), 1);
        const niceMax = Math.ceil(maxVal / 10) * 10;

        for (let i = 0; i < n; i++) {
            const x    = ox + BAR_GAP + i * (barW + BAR_GAP);
            const val  = data[i].max_distance || 0;
            const barH = (val / niceMax) * chartH;
            const y    = oy + chartH - barH;
            if (mx >= x && mx <= x + barW && my >= y && my <= oy + chartH) return i;
        }
        return -1;
    }

    // ── Mouse events ──────────────────────────────────────────────────────
    canvas.addEventListener('mousemove', e => {
        const rect = canvas.getBoundingClientRect();
        const mx   = e.clientX - rect.left;
        const my   = e.clientY - rect.top;
        const idx  = hitIndex(mx, my);
        if (idx !== hoveredIndex) {
            hoveredIndex = idx;
            draw(1);
        }
        if (idx >= 0) {
            showTooltip(e, idx);
            canvas.style.cursor = 'pointer';
        } else {
            hideTooltip();
            canvas.style.cursor = 'default';
        }
    });

    canvas.addEventListener('mouseleave', () => {
        hoveredIndex = -1;
        hideTooltip();
        draw(1);
    });

    // ── Animation loop ────────────────────────────────────────────────────
    function animate(ts) {
        if (!animStart) animStart = ts;
        const elapsed  = ts - animStart;
        const progress = Math.min(elapsed / ANIM_MS, 1);
        // Ease out cubic
        const eased = 1 - Math.pow(1 - progress, 3);
        draw(eased);
        if (progress < 1) requestAnimationFrame(animate);
    }

    // ── Init ──────────────────────────────────────────────────────────────
    resize();
    requestAnimationFrame(animate);
    window.addEventListener('resize', () => {
        resize();
        animStart = null;
        requestAnimationFrame(animate);
    });
})();
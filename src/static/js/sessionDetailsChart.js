(function () {
    const canvas  = document.getElementById('sessionDetailsChart');
    const tooltip = document.getElementById('chartTooltip');

    if (!canvas || !sessionData || sessionData.length === 0) return;

    const C = {
        line:      '#f57c00',
        dot:       '#f7931a',
        dotStroke: '#ffffff',
        fill:      'rgba(247, 147, 26, 0.12)',
        grid:      '#d7d7d7',
        axis:      '#a5a5a5',
        label:     '#555555',
        arrow:     '#a5a5a5',
    };

    const W      = 700;
    const H      = 330;
    const PAD    = { top: 30, right: 30, bottom: 60, left: 72 };
    const PLOT_W = W - PAD.left - PAD.right;
    const PLOT_H = H - PAD.top  - PAD.bottom;

    canvas.width  = W;
    canvas.height = H;

    const ctx = canvas.getContext('2d');

    // Pre-compute running maximum on raw data
    const rawDistances   = sessionData.map(d => d.max_distance);
    const runningMaxData = sessionData.map((d, i) => Object.assign({}, d, {
        max_distance: rawDistances.reduce((max, v, j) => j <= i ? Math.max(max, v) : max, 0)
    }));

    let displayData      = [];
    let displayDistances = [];

    function xOf(i) {
        if (displayData.length === 1) return PAD.left + PLOT_W / 2;
        return PAD.left + (i / (displayData.length - 1)) * PLOT_W;
    }

    function yOf(val, yMin, yMax) {
        return PAD.top + PLOT_H - ((val - yMin) / (yMax - yMin)) * PLOT_H;
    }

    function draw() {
        displayData      = runningMaxData;
        displayDistances = displayData.map(d => d.max_distance);

        const yMin = Math.max(0, Math.min(...displayDistances) * 0.85);
        const yMax = Math.max(...displayDistances) * 1.08;

        const DOT_R = displayData.length > 80 ? 0
                    : displayData.length > 30 ? 2.5
                    : displayData.length > 12 ? 3.5
                    : 5;

        const showFill = displayData.length < 200;

        ctx.clearRect(0, 0, W, H);

        const gridCount = 4;
        ctx.font      = '13px "Courier New", monospace';
        ctx.fillStyle = C.label;
        ctx.textAlign = 'right';

        for (let i = 0; i <= gridCount; i++) {
            const val = yMin + (i / gridCount) * (yMax - yMin);
            const y   = yOf(val, yMin, yMax);
            ctx.strokeStyle = i === 0 ? C.axis : C.grid;
            ctx.lineWidth   = i === 0 ? 2 : 1;
            ctx.beginPath();
            ctx.moveTo(PAD.left, y);
            ctx.lineTo(PAD.left + PLOT_W, y);
            ctx.stroke();
            if (i > 0) {
                ctx.fillText(Math.round(val), PAD.left - 8, y + 4);
            }
        }

        ctx.strokeStyle = C.axis;
        ctx.lineWidth   = 2;
        ctx.beginPath();
        ctx.moveTo(PAD.left, PAD.top + PLOT_H);
        ctx.lineTo(PAD.left + PLOT_W + 10, PAD.top + PLOT_H);
        ctx.stroke();

        ctx.beginPath();
        ctx.moveTo(PAD.left, PAD.top + PLOT_H + 10);
        ctx.lineTo(PAD.left, PAD.top - 10);
        ctx.stroke();

        ctx.fillStyle = C.arrow;
        ctx.beginPath();
        ctx.moveTo(PAD.left,     PAD.top - 18);
        ctx.lineTo(PAD.left - 6, PAD.top - 8);
        ctx.lineTo(PAD.left + 6, PAD.top - 8);
        ctx.closePath();
        ctx.fill();

        ctx.save();
        ctx.translate(18, PAD.top + PLOT_H / 2);
        ctx.rotate(-Math.PI / 2);
        ctx.fillStyle = '#111';
        ctx.font      = '14px "Courier New", monospace';
        ctx.textAlign = 'center';
        ctx.fillText('Distance (cm)', 0, 0);
        ctx.restore();

        ctx.fillStyle = '#555';
        ctx.font      = '13px "Courier New", monospace';
        ctx.textAlign = 'center';
        ctx.fillText('Session', PAD.left + PLOT_W / 2, H - 10);

        if (displayData.length < 2) return;

        const maxTicks = Math.min(displayData.length, 6);
        ctx.fillStyle = C.label;
        ctx.font      = '11px "Courier New", monospace';
        ctx.textAlign = 'center';
        for (let t = 0; t < maxTicks; t++) {
            const idx  = Math.round(t * (displayData.length - 1) / (maxTicks - 1));
            const x    = xOf(idx);
            const date = formatTime(displayData[idx].start_time);
            ctx.fillText(date, x, PAD.top + PLOT_H + 20);
        }

        if (showFill) {
            ctx.beginPath();
            ctx.moveTo(xOf(0), yOf(displayDistances[0], yMin, yMax));
            displayData.forEach((_, i) => ctx.lineTo(xOf(i), yOf(displayDistances[i], yMin, yMax)));
            ctx.lineTo(xOf(displayData.length - 1), PAD.top + PLOT_H);
            ctx.lineTo(xOf(0), PAD.top + PLOT_H);
            ctx.closePath();
            ctx.fillStyle = C.fill;
            ctx.fill();
        }

        ctx.beginPath();
        ctx.moveTo(xOf(0), yOf(displayDistances[0], yMin, yMax));
        displayData.forEach((_, i) => ctx.lineTo(xOf(i), yOf(displayDistances[i], yMin, yMax)));
        ctx.strokeStyle = C.line;
        ctx.lineWidth   = displayData.length > 100 ? 1.5 : 2.5;
        ctx.lineJoin    = 'round';
        ctx.stroke();

        if (DOT_R > 0) {
            displayData.forEach((_, i) => {
                ctx.beginPath();
                ctx.arc(xOf(i), yOf(displayDistances[i], yMin, yMax), DOT_R, 0, Math.PI * 2);
                ctx.fillStyle   = C.dot;
                ctx.fill();
                ctx.strokeStyle = C.dotStroke;
                ctx.lineWidth   = 1.5;
                ctx.stroke();
            });
        }

        draw._yMin = yMin;
        draw._yMax = yMax;
    }

    draw();

    let activeIdx = -1;

    function getHoveredIndex(mouseX, mouseY) {
        const rect   = canvas.getBoundingClientRect();
        const scaleX = W / rect.width;
        const scaleY = H / rect.height;
        const cx     = (mouseX - rect.left) * scaleX;
        const cy     = (mouseY - rect.top)  * scaleY;

        const DOT_R  = displayData.length > 80 ? 0
                     : displayData.length > 30 ? 2.5
                     : displayData.length > 12 ? 3.5
                     : 5;
        const hitR   = Math.max(DOT_R + 6, 10);

        for (let i = 0; i < displayData.length; i++) {
            const dx = cx - xOf(i);
            const dy = cy - yOf(displayDistances[i], draw._yMin, draw._yMax);
            if (Math.sqrt(dx * dx + dy * dy) <= hitR) return i;
        }
        return -1;
    }

    canvas.addEventListener('mousemove', (e) => {
        const idx = getHoveredIndex(e.clientX, e.clientY);

        if (idx !== -1) {
            canvas.style.cursor = 'pointer';
            if (idx !== activeIdx) {
                activeIdx = idx;
                redrawHighlight(idx);
                showTooltip(e, idx);
            } else {
                moveTooltip(e);
            }
        } else {
            if (activeIdx !== -1) {
                activeIdx = -1;
                draw();
                hideTooltip();
            }
            canvas.style.cursor = 'default';
        }
    });

    canvas.addEventListener('mouseleave', () => {
        activeIdx = -1;
        draw();
        hideTooltip();
        canvas.style.cursor = 'default';
    });

    function redrawHighlight(idx) {
        draw();

        const DOT_R = displayData.length > 80 ? 0
                    : displayData.length > 30 ? 2.5
                    : displayData.length > 12 ? 3.5
                    : 5;
        const highlightR = Math.max(DOT_R + 3, 6);

        ctx.beginPath();
        ctx.arc(xOf(idx), yOf(displayDistances[idx], draw._yMin, draw._yMax), highlightR, 0, Math.PI * 2);
        ctx.fillStyle   = C.dot;
        ctx.fill();
        ctx.strokeStyle = '#ffffff';
        ctx.lineWidth   = 2;
        ctx.stroke();

        ctx.beginPath();
        ctx.moveTo(xOf(idx), PAD.top + PLOT_H);
        ctx.lineTo(xOf(idx), yOf(displayDistances[idx], draw._yMin, draw._yMax));
        ctx.strokeStyle = 'rgba(245, 124, 0, 0.3)';
        ctx.lineWidth   = 1.5;
        ctx.setLineDash([4, 4]);
        ctx.stroke();
        ctx.setLineDash([]);
    }

    function showTooltip(e, idx) {
        const s    = displayData[idx];
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
        tooltip.classList.remove('visible');
    }

    function parseDate(str) {
        return new Date(str.replace(' ', 'T'));
    }

    function formatTime(str) {
        const d = parseDate(str);
        if (isNaN(d)) return str;
        const pad = n => String(n).padStart(2, '0');
        return `${pad(d.getHours())}:${pad(d.getMinutes())}:${pad(d.getSeconds())}`;
    }

    function formatDateFull(str) {
        const d = parseDate(str);
        if (isNaN(d)) return str;
        const pad = n => String(n).padStart(2, '0');
        return `${d.getFullYear()}-${pad(d.getMonth()+1)}-${pad(d.getDate())}  ${pad(d.getHours())}:${pad(d.getMinutes())}:${pad(d.getSeconds())}`;
    }

    window.addEventListener('resize', draw);
})();
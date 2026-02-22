<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>

<xsl:template match="/mcaster1stats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Mcaster1DNAS Player - <xsl:value-of select="source/@mount"/></title>

    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css"
          integrity="sha512-iecdLmaskl7CVkqkXNQ/ZH/XLlvWZOJyj7Yy7tcenmpD1ypASozpmT/E0iPtmFIB46ZmdtAc9eNBvH0H/ZpiBw=="
          crossorigin="anonymous" referrerpolicy="no-referrer"/>

    <style>
        :root {
            --mcaster-blue: #0891b2;
            --mcaster-teal: #14b8a6;
            --dnas-green: #10b981;
            --bg-dark: #0f172a;
            --bg-medium: #1e293b;
            --bg-light: #334155;
            --text-primary: #f1f5f9;
            --text-secondary: #94a3b8;
            --border-light: #475569;
            --accent-yellow: #fbbf24;
            --status-active: #10b981;
            --status-error: #ef4444;
            --radius-md: 0.5rem;
            --radius-sm: 0.25rem;
        }

        * { margin: 0; padding: 0; box-sizing: border-box; }

        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            background: linear-gradient(135deg, var(--bg-dark) 0%, #0c1426 100%);
            color: var(--text-primary);
            min-height: 100vh;
            padding: 1rem;
            overflow-x: hidden;
        }

        /* ── Two-column page layout ── */
        .page-layout {
            display: flex;
            gap: 0.75rem;
            align-items: flex-start;
            max-width: 900px;
            margin: 0 auto;
        }

        /* ── Player window (left column) ── */
        .player-window {
            flex: 1;
            min-width: 0;
            background: rgba(30, 41, 59, 0.95);
            border-radius: var(--radius-md);
            box-shadow: 0 10px 40px rgba(0, 0, 0, 0.5);
            overflow: hidden;
        }

        .player-header {
            background: linear-gradient(135deg, var(--mcaster-blue) 0%, var(--mcaster-teal) 100%);
            padding: 1.5rem;
            text-align: center;
            position: relative;
        }

        .player-header h1 {
            font-size: 1.5rem;
            margin-bottom: 0.5rem;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 0.5rem;
        }

        .brand-mcaster { color: #ffffff; font-weight: 700; }
        .brand-dnas    { color: var(--accent-yellow); font-weight: 600; }

        .station-name { font-size: 0.875rem; opacity: 0.9; }

        .bookmark-hint {
            position: absolute;
            top: 0.5rem;
            right: 0.5rem;
            background: rgba(0, 0, 0, 0.3);
            padding: 0.25rem 0.75rem;
            border-radius: 1rem;
            font-size: 0.75rem;
            display: flex;
            align-items: center;
            gap: 0.25rem;
            cursor: pointer;
            transition: all 0.2s;
        }
        .bookmark-hint:hover { background: rgba(0, 0, 0, 0.5); transform: scale(1.05); }

        .player-body { padding: 1.5rem; }

        /* ── Now Playing ── */
        .now-playing {
            background: rgba(0, 0, 0, 0.3);
            padding: 1.25rem;
            border-radius: var(--radius-md);
            margin-bottom: 1rem;
            border-left: 4px solid var(--mcaster-blue);
        }

        .playing-indicator {
            display: inline-block;
            color: var(--status-active);
            font-weight: bold;
            margin-bottom: 0.5rem;
            animation: pulse 2s infinite;
        }

        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }

        .np-topbar {
            display: flex;
            align-items: center;
            justify-content: space-between;
            margin-bottom: 0.3rem;
        }

        /* ── VFD Ticker Display (cyan — admin monitor) ── */
        .media-display {
            position: relative;
            background: #00060e;
            border: 1px solid rgba(20, 184, 166, 0.4);
            border-radius: 5px;
            overflow: hidden;
            margin: 0.75rem 0 0.4rem;
            box-shadow:
                inset 0 0 30px rgba(0,0,0,0.95),
                0 0 10px rgba(20, 184, 166, 0.12),
                0 2px 6px rgba(0,0,0,0.55);
        }

        /* VFD phosphor scanline texture */
        .media-display::before {
            content: '';
            position: absolute;
            inset: 0;
            background: repeating-linear-gradient(
                0deg, transparent, transparent 2px,
                rgba(0,0,0,0.14) 2px, rgba(0,0,0,0.14) 4px
            );
            pointer-events: none;
            z-index: 2;
        }

        .display-header {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 0.28rem 0.75rem;
            border-bottom: 1px solid rgba(20, 184, 166, 0.14);
        }

        .display-label {
            font-size: 0.58rem;
            font-family: 'Courier New', monospace;
            color: var(--mcaster-blue);
            text-shadow: 0 0 7px var(--mcaster-blue);
            letter-spacing: 0.18em;
        }

        .display-clock {
            font-size: 0.58rem;
            font-family: 'Courier New', monospace;
            color: rgba(8, 145, 178, 0.5);
            letter-spacing: 0.06em;
        }

        .ticker-wrap { overflow: hidden; padding: 0.45rem 0; }

        /* Cyan VFD glow — admin monitor style */
        .ticker-text {
            display: inline-block;
            white-space: nowrap;
            font-family: 'Courier New', Courier, monospace;
            font-size: 1.05rem;
            font-weight: 700;
            color: var(--mcaster-teal);
            text-shadow:
                0 0 14px rgba(20,184,166,0.95),
                0 0  6px rgba(20,184,166,0.6),
                0 0 28px rgba(20,184,166,0.25);
            letter-spacing: 0.07em;
            padding-left: 100%;
            animation: ticker-scroll 18s linear infinite;
        }

        @keyframes ticker-scroll {
            0%   { transform: translateX(0%); }
            100% { transform: translateX(-100%); }
        }

        .display-footer {
            display: flex;
            align-items: center;
            gap: 0.5rem;
            padding: 0.25rem 0.75rem;
            border-top: 1px solid rgba(20, 184, 166, 0.1);
        }

        .eq-dots { display: flex; gap: 0.3rem; align-items: center; }
        .eq-dot {
            width: 4px; height: 4px; border-radius: 50%;
            animation: eq-bounce 1.2s ease-in-out infinite;
        }
        .eq-dot:nth-child(1) { background: #0891b2; box-shadow: 0 0 5px #0891b2; }
        .eq-dot:nth-child(2) { background: #14b8a6; box-shadow: 0 0 5px #14b8a6; animation-delay: 0.15s; }
        .eq-dot:nth-child(3) { background: #6366f1; box-shadow: 0 0 5px #6366f1; animation-delay: 0.30s; }
        .eq-dot:nth-child(4) { background: #10b981; box-shadow: 0 0 5px #10b981; animation-delay: 0.45s; }

        @keyframes eq-bounce {
            0%, 100% { opacity: 0.2; transform: scale(0.7); }
            50%       { opacity: 1;   transform: scale(1.35); }
        }

        .display-meta {
            font-size: 0.56rem;
            font-family: 'Courier New', monospace;
            color: rgba(20, 184, 166, 0.38);
            letter-spacing: 0.1em;
        }

        /* ── Music service lookup links (under ticker) ── */
        .np-music-links {
            display: flex;
            gap: 0.3rem;
            align-items: center;
            flex-wrap: wrap;
            margin-top: 0.55rem;
            min-height: 1.9rem;
        }

        .np-music-link {
            display: inline-flex;
            align-items: center;
            gap: 0.25rem;
            padding: 0.18rem 0.5rem;
            border-radius: var(--radius-sm);
            text-decoration: none;
            font-size: 0.7rem;
            font-weight: 600;
            transition: all 0.15s ease;
            border: 1px solid transparent;
        }
        .np-music-link:hover { transform: translateY(-1px); box-shadow: 0 3px 8px rgba(0,0,0,0.3); }

        /* MusicBrainz — orange */
        .npm-mb  { color:#f97316; background:rgba(249,115,22,0.12); border-color:rgba(249,115,22,0.3); }
        .npm-mb:hover  { background:#f97316; color:#fff; border-color:#f97316; }
        /* Last.fm — red */
        .npm-lfm { color:#ef4444; background:rgba(239,68,68,0.12); border-color:rgba(239,68,68,0.3); }
        .npm-lfm:hover { background:#ef4444; color:#fff; border-color:#ef4444; }
        /* Discogs — slate */
        .npm-dc  { color:#94a3b8; background:rgba(148,163,184,0.12); border-color:rgba(148,163,184,0.3); }
        .npm-dc:hover  { background:#94a3b8; color:#0f172a; border-color:#94a3b8; }
        /* AllMusic — teal */
        .npm-am  { color:#14b8a6; background:rgba(20,184,166,0.12); border-color:rgba(20,184,166,0.3); }
        .npm-am:hover  { background:#14b8a6; color:#fff; border-color:#14b8a6; }

        .current-artist {
            font-size: 1rem;
            color: var(--text-secondary);
            margin-top: 0.5rem;
        }

        .stream-description {
            font-size: 0.875rem;
            color: var(--text-secondary);
            font-style: italic;
            margin-top: 0.35rem;
        }

        /* ── Audio Controls ── */
        .audio-controls {
            display: flex;
            align-items: center;
            gap: 1rem;
            margin-bottom: 1rem;
            padding: 1rem;
            background: rgba(0, 0, 0, 0.2);
            border-radius: var(--radius-md);
        }

        .control-btn {
            background: linear-gradient(135deg, var(--mcaster-blue), var(--mcaster-teal));
            color: white;
            border: none;
            width: 50px; height: 50px;
            border-radius: 50%;
            display: flex; align-items: center; justify-content: center;
            cursor: pointer;
            font-size: 1.25rem;
            transition: all 0.2s;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.3);
        }
        .control-btn:hover  { transform: scale(1.1); box-shadow: 0 6px 12px rgba(0,0,0,0.4); }
        .control-btn:active { transform: scale(0.95); }
        .control-btn.stop   { background: linear-gradient(135deg, #ef4444, #dc2626); }

        .volume-control {
            flex: 1;
            display: flex;
            align-items: center;
            gap: 0.75rem;
        }

        .volume-icon { color: var(--mcaster-blue); font-size: 1.25rem; }

        .volume-slider {
            flex: 1; height: 6px;
            -webkit-appearance: none; appearance: none;
            background: rgba(0, 0, 0, 0.3);
            border-radius: 3px; outline: none;
        }
        .volume-slider::-webkit-slider-thumb {
            -webkit-appearance: none; appearance: none;
            width: 18px; height: 18px; border-radius: 50%;
            background: var(--mcaster-blue); cursor: pointer;
            box-shadow: 0 0 5px rgba(8,145,178,0.5); transition: all 0.2s;
        }
        .volume-slider::-webkit-slider-thumb:hover { transform: scale(1.2); box-shadow: 0 0 10px rgba(8,145,178,0.8); }
        .volume-slider::-moz-range-thumb {
            width: 18px; height: 18px; border-radius: 50%;
            background: var(--mcaster-blue); cursor: pointer; border: none;
            box-shadow: 0 0 5px rgba(8,145,178,0.5); transition: all 0.2s;
        }

        .volume-level { color: var(--accent-yellow); font-weight: bold; min-width: 45px; text-align: center; }

        /* ── VU Meters ── */
        .vu-meters { margin-bottom: 1rem; }

        .vu-meter {
            position: relative; height: 30px;
            background: #000; border-radius: 4px;
            overflow: hidden; margin-bottom: 0.5rem;
            box-shadow: inset 0 0 10px rgba(0,0,0,0.7);
        }

        .vu-meter-label {
            position: absolute; left: 0.5rem; top: 50%;
            transform: translateY(-50%);
            font-size: 0.75rem; color: rgba(255,255,255,0.7);
            z-index: 2; font-weight: bold;
        }

        .vu-meter-fill {
            position: absolute; left: 0; top: 0;
            height: 100%; width: 0%;
            background: linear-gradient(to right,
                #00aa00 0%, #00dd00 70%,
                #dddd00 70%, #dddd00 85%,
                #dd0000 85%, #dd0000 100%);
            transition: width 0.1s linear;
        }

        .vu-meter-peak {
            position: absolute; top: 0; height: 100%;
            width: 2px; background-color: #ff0000;
            z-index: 3; transition: left 0.1s ease-out;
        }

        /* ── Tabbed section ── */
        .player-tabs { margin-bottom: 0.5rem; }

        .tab-bar {
            display: flex;
            border-bottom: 2px solid rgba(71,85,105,0.5);
            margin-bottom: 0.75rem;
        }

        .tab-btn {
            background: none;
            border: none;
            border-bottom: 2px solid transparent;
            margin-bottom: -2px;
            padding: 0.45rem 0.85rem;
            color: var(--text-secondary);
            font-size: 0.75rem;
            font-weight: 600;
            cursor: pointer;
            display: flex;
            align-items: center;
            gap: 0.3rem;
            transition: all 0.15s;
            white-space: nowrap;
        }
        .tab-btn:hover { color: var(--text-primary); }
        .tab-btn.active { color: var(--mcaster-blue); border-bottom-color: var(--mcaster-blue); }

        .tab-panel { display: none; }
        .tab-panel.active { display: block; }

        /* ── Stats grid (tab 1) ── */
        .stream-info {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(130px, 1fr));
            gap: 0.6rem;
            margin-bottom: 0.5rem;
        }

        .info-card {
            background: rgba(0,0,0,0.2);
            padding: 0.65rem;
            border-radius: var(--radius-md);
            border: 1px solid var(--border-light);
        }

        .info-label {
            font-size: 0.7rem;
            color: var(--text-secondary);
            text-transform: uppercase;
            margin-bottom: 0.2rem;
        }

        .info-value {
            font-size: 0.95rem;
            font-weight: 600;
            color: var(--text-primary);
            display: flex;
            align-items: center;
            gap: 0.4rem;
        }
        .info-value i { color: var(--mcaster-blue); }

        /* ── Song History (tab 2) ── */
        .history-list {
            display: flex;
            flex-direction: column;
            gap: 0.35rem;
            max-height: 240px;
            overflow-y: auto;
        }

        .history-list::-webkit-scrollbar { width: 4px; }
        .history-list::-webkit-scrollbar-track { background: rgba(0,0,0,0.2); border-radius: 2px; }
        .history-list::-webkit-scrollbar-thumb { background: var(--border-light); border-radius: 2px; }

        .history-entry {
            display: flex;
            align-items: flex-start;
            gap: 0.5rem;
            padding: 0.45rem 0.55rem;
            background: rgba(0,0,0,0.2);
            border-radius: 5px;
            border-left: 3px solid var(--border-light);
            font-size: 0.77rem;
        }
        .history-entry:first-child { border-left-color: var(--dnas-green); }

        .history-num {
            color: var(--text-secondary);
            font-size: 0.62rem;
            min-width: 1.1rem;
            text-align: right;
            padding-top: 0.12rem;
            flex-shrink: 0;
        }

        .history-info { flex: 1; min-width: 0; }

        .history-title {
            font-weight: 600;
            color: var(--text-primary);
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
        }

        .history-artist {
            color: var(--text-secondary);
            font-size: 0.68rem;
            margin-top: 0.08rem;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
        }

        .history-links {
            display: flex;
            gap: 0.18rem;
            align-items: center;
            flex-shrink: 0;
        }

        .history-link {
            display: inline-flex;
            align-items: center;
            justify-content: center;
            width: 1.3rem; height: 1.3rem;
            border-radius: 3px;
            text-decoration: none;
            font-size: 0.6rem;
            transition: all 0.13s;
            border: 1px solid transparent;
        }
        .history-link:hover { transform: translateY(-1px); }

        .hl-mb  { color:#f97316; background:rgba(249,115,22,0.1);  border-color:rgba(249,115,22,0.25); }
        .hl-mb:hover  { background:#f97316; color:#fff; }
        .hl-lfm { color:#ef4444; background:rgba(239,68,68,0.1);   border-color:rgba(239,68,68,0.25); }
        .hl-lfm:hover { background:#ef4444; color:#fff; }
        .hl-dc  { color:#94a3b8; background:rgba(148,163,184,0.1); border-color:rgba(148,163,184,0.25); }
        .hl-dc:hover  { background:#94a3b8; color:#0f172a; }
        .hl-am  { color:#14b8a6; background:rgba(20,184,166,0.1);  border-color:rgba(20,184,166,0.25); }
        .hl-am:hover  { background:#14b8a6; color:#fff; }

        .history-empty {
            text-align: center;
            padding: 1.5rem 1rem;
            color: var(--text-secondary);
            font-size: 0.8rem;
        }

        .history-now-dot {
            display: inline-block;
            width: 6px; height: 6px;
            background: var(--dnas-green);
            border-radius: 50%;
            margin-right: 0.3rem;
            animation: pulse 2s infinite;
            vertical-align: middle;
        }

        /* ── Credits tab ── */
        .credits-content {
            font-size: 0.78rem;
            color: var(--text-secondary);
            line-height: 1.65;
            padding: 0.25rem 0;
        }

        .credits-section { margin-bottom: 0.85rem; }

        .credits-heading {
            font-size: 0.72rem;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 0.1em;
            color: var(--mcaster-blue);
            margin-bottom: 0.35rem;
            display: flex;
            align-items: center;
            gap: 0.35rem;
        }

        .credits-badge {
            display: inline-flex;
            align-items: center;
            gap: 0.3rem;
            background: rgba(8,145,178,0.12);
            border: 1px solid rgba(8,145,178,0.3);
            color: var(--mcaster-blue);
            padding: 0.15rem 0.45rem;
            border-radius: 1rem;
            font-size: 0.68rem;
            font-weight: 600;
            margin: 0.15rem 0.15rem 0.15rem 0;
        }

        .credits-link { color: var(--mcaster-teal); text-decoration: none; }
        .credits-link:hover { color: var(--mcaster-blue); text-decoration: underline; }

        /* ── ICY2 Notice Board (right column) ── */
        .notice-panel {
            width: 210px;
            flex-shrink: 0;
            background: rgba(30, 41, 59, 0.95);
            border-radius: var(--radius-md);
            box-shadow: 0 10px 40px rgba(0, 0, 0, 0.5);
            overflow: hidden;
            position: sticky;
            top: 1rem;
            max-height: calc(100vh - 2rem);
            display: flex;
            flex-direction: column;
        }

        .notice-panel-header {
            background: linear-gradient(135deg, rgba(8,145,178,0.25), rgba(20,184,166,0.25));
            padding: 0.65rem 0.85rem;
            font-size: 0.68rem;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 0.1em;
            color: var(--mcaster-teal);
            border-bottom: 1px solid rgba(20,184,166,0.2);
            display: flex;
            align-items: center;
            gap: 0.4rem;
            flex-shrink: 0;
        }

        .notice-scroll {
            overflow-y: auto;
            flex: 1;
            padding: 0.65rem;
        }

        .notice-scroll::-webkit-scrollbar { width: 3px; }
        .notice-scroll::-webkit-scrollbar-track { background: rgba(0,0,0,0.2); }
        .notice-scroll::-webkit-scrollbar-thumb { background: var(--border-light); border-radius: 2px; }

        .notice-item {
            background: rgba(0,0,0,0.25);
            border-radius: 5px;
            border-left: 3px solid var(--mcaster-blue);
            padding: 0.5rem 0.6rem;
            margin-bottom: 0.5rem;
            font-size: 0.73rem;
        }

        .notice-tag {
            font-size: 0.57rem;
            text-transform: uppercase;
            letter-spacing: 0.1em;
            color: var(--mcaster-teal);
            margin-bottom: 0.25rem;
            font-weight: 700;
        }

        .notice-text {
            color: var(--text-primary);
            line-height: 1.45;
            word-break: break-word;
        }

        .notice-text a { color: var(--mcaster-blue); text-decoration: none; }
        .notice-text a:hover { color: var(--mcaster-teal); text-decoration: underline; }

        .notice-loading {
            text-align: center;
            color: var(--text-secondary);
            padding: 1rem 0;
            font-size: 0.75rem;
        }

        .notice-icy2-badge {
            display: inline-flex;
            align-items: center;
            gap: 0.2rem;
            background: rgba(251,191,36,0.12);
            border: 1px solid rgba(251,191,36,0.3);
            color: var(--accent-yellow);
            padding: 0.1rem 0.35rem;
            border-radius: 0.75rem;
            font-size: 0.57rem;
            font-weight: 700;
            margin-bottom: 0.3rem;
        }

        /* Admin-only: mount path indicator */
        .admin-mount-badge {
            display: inline-flex;
            align-items: center;
            gap: 0.3rem;
            background: rgba(99,102,241,0.12);
            border: 1px solid rgba(99,102,241,0.3);
            color: #a5b4fc;
            padding: 0.12rem 0.4rem;
            border-radius: var(--radius-sm);
            font-family: monospace;
            font-size: 0.65rem;
            margin-top: 0.4rem;
        }

        /* ── Buffering overlay ── */
        .buffering-overlay {
            position: fixed; top: 0; left: 0;
            width: 100%; height: 100%;
            background: rgba(0,0,0,0.8);
            display: none;
            align-items: center; justify-content: center;
            z-index: 9999;
        }
        .buffering-overlay.active { display: flex; }
        .buffering-spinner { font-size: 3rem; color: var(--mcaster-blue); animation: spin 1s linear infinite; }
        @keyframes spin { from { transform: rotate(0deg); } to { transform: rotate(360deg); } }

        /* ── Status badge ── */
        .status-live {
            display: inline-flex; align-items: center; gap: 0.5rem;
            background: rgba(16,185,129,0.2); color: var(--status-active);
            padding: 0.25rem 0.75rem; border-radius: 1rem;
            font-size: 0.75rem; font-weight: 600;
            border: 1px solid var(--status-active);
        }
        .live-dot {
            width: 8px; height: 8px;
            background: var(--status-active);
            border-radius: 50%;
            animation: pulse 2s infinite;
        }

        a { color: var(--mcaster-blue); text-decoration: none; transition: color 0.2s; }
        a:hover { color: var(--mcaster-teal); }

        /* ── Mobile responsive ── */
        @media (max-width: 720px) {
            .page-layout { flex-direction: column; gap: 0.5rem; }
            .notice-panel { width: 100%; position: static; max-height: 220px; }
        }

        @media (max-width: 640px) {
            body { padding: 0; }
            .player-window { border-radius: 0; min-height: 100vh; }
            .notice-panel  { border-radius: 0; }
            .player-header { padding: 1rem; }
            .player-header h1 { font-size: 1.25rem; }
            .player-body { padding: 0.75rem; }
            .now-playing { padding: 0.85rem; margin-bottom: 0.75rem; }
            .audio-controls { flex-wrap: wrap; gap: 0.75rem; padding: 0.75rem; }
            .control-btn { width: 56px; height: 56px; font-size: 1.4rem; }
            .volume-control { width: 100%; flex-basis: 100%; }
            .volume-slider { height: 10px; }
            .volume-slider::-webkit-slider-thumb { width: 26px; height: 26px; }
            .volume-slider::-moz-range-thumb { width: 26px; height: 26px; }
            .stream-info { grid-template-columns: repeat(2, 1fr); gap: 0.4rem; }
            .info-card { padding: 0.5rem; }
            .info-value { font-size: 0.85rem; }
            .vu-meter { height: 24px; }
            .bookmark-hint { font-size: 0.65rem; padding: 0.2rem 0.5rem; }
            .tab-btn { padding: 0.4rem 0.6rem; font-size: 0.7rem; }
        }

        @media (max-width: 380px) {
            .stream-info { grid-template-columns: 1fr 1fr; }
            .control-btn { width: 50px; height: 50px; }
        }
    </style>
</head>
<body>

    <!-- Buffering overlay -->
    <div class="buffering-overlay" id="bufferingOverlay">
        <div>
            <i class="fas fa-spinner buffering-spinner"></i>
            <p style="text-align:center;margin-top:1rem;">Buffering stream...</p>
        </div>
    </div>

    <!-- Two-column page layout -->
    <div class="page-layout">

        <!-- ══ LEFT: Player ══ -->
        <div class="player-window">
            <div class="player-header">
                <div class="bookmark-hint" onclick="showBookmarkHint()" title="Bookmark this player">
                    <i class="fas fa-bookmark"></i>
                    <span>Bookmark Me</span>
                </div>

                <h1>
                    <i class="fas fa-broadcast-tower"></i>
                    <span class="brand-mcaster">Mcaster1</span>
                    <span class="brand-dnas">DNAS</span>
                </h1>

                <div class="station-name">
                    <xsl:choose>
                        <xsl:when test="source/server_name">
                            <xsl:value-of select="source/server_name"/>
                        </xsl:when>
                        <xsl:otherwise>Streaming Server</xsl:otherwise>
                    </xsl:choose>
                </div>
            </div><!-- /player-header -->

            <div class="player-body">

                <!-- ── Now Playing ── -->
                <div class="now-playing">
                    <div class="np-topbar">
                        <div class="playing-indicator" id="playingIndicator">
                            <i class="fas fa-circle"></i> <span id="statusText">READY</span>
                        </div>
                        <div class="status-live">
                            <span class="live-dot"></span>
                            LIVE
                        </div>
                    </div>

                    <!-- VFD Ticker Display (admin) -->
                    <div class="media-display">
                        <div class="display-header">
                            <span class="display-label">&#9670; ADMIN MONITOR</span>
                            <span class="display-clock" id="displayClock">--:--:--</span>
                        </div>
                        <div class="ticker-wrap">
                            <span class="ticker-text" id="currentTitle"><xsl:choose>
                                <xsl:when test="source/title"><xsl:value-of select="source/title"/></xsl:when>
                                <xsl:otherwise>Connecting to Stream&#x2026;</xsl:otherwise>
                            </xsl:choose></span>
                        </div>
                        <div class="display-footer">
                            <div class="eq-dots">
                                <span class="eq-dot"></span>
                                <span class="eq-dot"></span>
                                <span class="eq-dot"></span>
                                <span class="eq-dot"></span>
                            </div>
                            <span class="display-meta">MCASTER1DNAS &#183; ADMIN</span>
                        </div>
                    </div>

                    <!-- Music service lookup links (updated live by JS) -->
                    <div class="np-music-links" id="musicLinks">
                        <!-- Populated by JS on load and on title change -->
                    </div>

                    <!-- Admin mount badge -->
                    <div class="admin-mount-badge">
                        <i class="fas fa-stream" style="font-size:0.6rem;"></i>
                        <xsl:value-of select="source/@mount"/>
                    </div>

                    <xsl:if test="source/artist">
                        <div class="current-artist">
                            <i class="fas fa-user" style="font-size:0.8rem;margin-right:0.25rem;opacity:0.7;"></i>
                            <xsl:value-of select="source/artist"/>
                        </div>
                    </xsl:if>

                    <xsl:if test="source/server_description">
                        <div class="stream-description">
                            <xsl:value-of select="source/server_description"/>
                        </div>
                    </xsl:if>
                </div><!-- /now-playing -->

                <!-- ── Audio Controls ── -->
                <div class="audio-controls">
                    <button class="control-btn" onclick="playStream()" title="Play" id="playBtn">
                        <i class="fas fa-play"></i>
                    </button>
                    <button class="control-btn" onclick="pauseStream()" title="Pause" id="pauseBtn">
                        <i class="fas fa-pause"></i>
                    </button>
                    <button class="control-btn stop" onclick="stopStream()" title="Stop">
                        <i class="fas fa-stop"></i>
                    </button>

                    <div class="volume-control">
                        <i class="fas fa-volume-up volume-icon" id="volumeIcon"></i>
                        <input type="range" class="volume-slider" id="volumeSlider"
                               min="0" max="100" value="80" oninput="adjustVolume()"/>
                        <span class="volume-level" id="volumeLevel">80%</span>
                    </div>
                </div>

                <!-- ── VU Meters ── -->
                <div class="vu-meters">
                    <div class="vu-meter">
                        <div class="vu-meter-label">L</div>
                        <div class="vu-meter-fill" id="vuMeterLeft"></div>
                        <div class="vu-meter-peak" id="vuPeakLeft"></div>
                    </div>
                    <div class="vu-meter">
                        <div class="vu-meter-label">R</div>
                        <div class="vu-meter-fill" id="vuMeterRight"></div>
                        <div class="vu-meter-peak" id="vuPeakRight"></div>
                    </div>
                </div>

                <!-- ── Tabbed section ── -->
                <div class="player-tabs">
                    <div class="tab-bar">
                        <button class="tab-btn active" id="tbtn-stats"
                                onclick="switchTab('stats')">
                            <i class="fas fa-chart-bar"></i> Stats
                        </button>
                        <button class="tab-btn" id="tbtn-history"
                                onclick="switchTab('history')">
                            <i class="fas fa-history"></i> History
                        </button>
                        <button class="tab-btn" id="tbtn-credits"
                                onclick="switchTab('credits')">
                            <i class="fas fa-info-circle"></i> Credits
                        </button>
                    </div>

                    <!-- Tab 1: Stream Stats -->
                    <div class="tab-panel active" id="tab-stats">
                        <div class="stream-info">
                            <div class="info-card">
                                <div class="info-label">Format</div>
                                <div class="info-value">
                                    <i class="fas fa-file-audio"></i>
                                    <xsl:choose>
                                        <xsl:when test="source/server_type = 'audio/mpeg'">MP3</xsl:when>
                                        <xsl:when test="source/server_type = 'audio/aacp'">AAC+</xsl:when>
                                        <xsl:when test="contains(source/server_type, 'aac')">AAC</xsl:when>
                                        <xsl:when test="contains(source/server_type, 'ogg')">
                                            <xsl:choose>
                                                <xsl:when test="source/subtype = 'Vorbis'">Ogg Vorbis</xsl:when>
                                                <xsl:when test="source/subtype">Ogg <xsl:value-of select="source/subtype"/></xsl:when>
                                                <xsl:otherwise>Opus</xsl:otherwise>
                                            </xsl:choose>
                                        </xsl:when>
                                        <xsl:otherwise><xsl:value-of select="source/server_type"/></xsl:otherwise>
                                    </xsl:choose>
                                </div>
                            </div>

                            <div class="info-card">
                                <div class="info-label">Bitrate</div>
                                <div class="info-value">
                                    <i class="fas fa-tachometer-alt"></i>
                                    <xsl:choose>
                                        <xsl:when test="source/bitrate"><xsl:value-of select="source/bitrate"/> kbps</xsl:when>
                                        <xsl:when test="source/ice-bitrate"><xsl:value-of select="source/ice-bitrate"/> kbps</xsl:when>
                                        <xsl:otherwise>N/A</xsl:otherwise>
                                    </xsl:choose>
                                </div>
                            </div>

                            <div class="info-card">
                                <div class="info-label">Sample Rate</div>
                                <div class="info-value">
                                    <i class="fas fa-wave-square"></i>
                                    <xsl:choose>
                                        <xsl:when test="source/mpeg_samplerate"><xsl:value-of select="source/mpeg_samplerate"/> Hz</xsl:when>
                                        <xsl:when test="source/audio_samplerate"><xsl:value-of select="source/audio_samplerate"/> Hz</xsl:when>
                                        <xsl:otherwise>N/A</xsl:otherwise>
                                    </xsl:choose>
                                </div>
                            </div>

                            <div class="info-card">
                                <div class="info-label">Channels</div>
                                <div class="info-value">
                                    <i class="fas fa-volume-up"></i>
                                    <xsl:choose>
                                        <xsl:when test="source/mpeg_channels = '2' or source/audio_channels = '2'">Stereo</xsl:when>
                                        <xsl:when test="source/mpeg_channels = '1' or source/audio_channels = '1'">Mono</xsl:when>
                                        <xsl:when test="source/mpeg_channels"><xsl:value-of select="source/mpeg_channels"/> ch</xsl:when>
                                        <xsl:when test="source/audio_channels"><xsl:value-of select="source/audio_channels"/> ch</xsl:when>
                                        <xsl:otherwise>N/A</xsl:otherwise>
                                    </xsl:choose>
                                </div>
                            </div>

                            <div class="info-card">
                                <div class="info-label">Listeners</div>
                                <div class="info-value" id="listenersValue">
                                    <i class="fas fa-users"></i>
                                    <xsl:choose>
                                        <xsl:when test="source/listeners"><xsl:value-of select="source/listeners"/></xsl:when>
                                        <xsl:otherwise>0</xsl:otherwise>
                                    </xsl:choose>
                                </div>
                            </div>

                            <div class="info-card">
                                <div class="info-label">Peak</div>
                                <div class="info-value" id="peakListenersValue">
                                    <i class="fas fa-chart-line"></i>
                                    <xsl:choose>
                                        <xsl:when test="source/listener_peak"><xsl:value-of select="source/listener_peak"/></xsl:when>
                                        <xsl:otherwise>0</xsl:otherwise>
                                    </xsl:choose>
                                </div>
                            </div>
                        </div><!-- /stream-info -->
                    </div><!-- /tab-stats -->

                    <!-- Tab 2: Song History (this mount only, no duplicates) -->
                    <div class="tab-panel" id="tab-history">
                        <div id="historyList" class="history-list">
                            <div class="history-empty">
                                <i class="fas fa-circle-notch fa-spin" style="display:block;font-size:1.5rem;margin-bottom:0.5rem;opacity:0.4;"></i>
                                Loading history&#x2026;
                            </div>
                        </div>
                    </div><!-- /tab-history -->

                    <!-- Tab 3: Credits -->
                    <div class="tab-panel" id="tab-credits">
                        <div class="credits-content">
                            <div class="credits-section">
                                <div class="credits-heading">
                                    <i class="fas fa-broadcast-tower"></i> Mcaster1DNAS Web Player
                                </div>
                                <p>
                                    A modern, browser-based streaming audio player built for the
                                    <strong style="color:var(--text-primary)">Mcaster1DNAS</strong> platform —
                                    the MediaCast1 Digital Network Audio Server. Designed to give listeners
                                    an immersive, bookmarkable experience without leaving the browser.
                                </p>
                            </div>

                            <div class="credits-section">
                                <div class="credits-heading">
                                    <i class="fas fa-lightbulb"></i> Concept &amp; Vision
                                </div>
                                <p>
                                    Built on the belief that internet radio deserves a first-class web experience —
                                    live metadata, real VU metering, track lookup, and station messaging all in
                                    one self-contained player window. No apps, no plugins, just the open web.
                                </p>
                            </div>

                            <div class="credits-section">
                                <div class="credits-heading">
                                    <i class="fas fa-code"></i> Built With
                                </div>
                                <div>
                                    <span class="credits-badge"><i class="fas fa-code"></i> XSLT 1.0</span>
                                    <span class="credits-badge"><i class="fab fa-html5"></i> HTML5 Audio</span>
                                    <span class="credits-badge"><i class="fas fa-wave-square"></i> Web Audio API</span>
                                    <span class="credits-badge"><i class="fas fa-satellite-dish"></i> ICY2 Protocol</span>
                                    <span class="credits-badge"><i class="fas fa-database"></i> MusicBrainz</span>
                                    <span class="credits-badge"><i class="fab fa-lastfm"></i> Last.fm</span>
                                    <span class="credits-badge"><i class="fas fa-record-vinyl"></i> Discogs</span>
                                </div>
                            </div>

                            <div class="credits-section">
                                <div class="credits-heading">
                                    <i class="fas fa-code-branch"></i> Lineage
                                </div>
                                <p style="font-family:monospace;font-size:0.72rem;color:var(--text-secondary);">
                                    Icecast2 (Xiph.Org) &#x2192; Icecast-KH (Karl Heyes) &#x2192; Mcaster1DNAS
                                </p>
                            </div>

                            <div class="credits-section">
                                <div class="credits-heading">
                                    <i class="fas fa-user"></i> Developer
                                </div>
                                <p>
                                    <strong style="color:var(--text-primary)">David St. John</strong><br/>
                                    MediaCast1 / Mcaster1 Platform<br/>
                                    <a class="credits-link" href="https://mcaster1.com" target="_blank" rel="noopener noreferrer">
                                        <i class="fas fa-globe" style="font-size:0.7rem;"></i> mcaster1.com
                                    </a>
                                    &#160;&#8226;&#160;
                                    <a class="credits-link" href="https://github.com/davestj/mcaster1dnas" target="_blank" rel="noopener noreferrer">
                                        <i class="fab fa-github" style="font-size:0.7rem;"></i> GitHub
                                    </a>
                                </p>
                                <p style="margin-top:0.45rem;font-style:italic;opacity:0.7;">
                                    "The drive to build something your community can call their own —
                                    open, independent, and built to last."
                                </p>
                            </div>

                            <div style="font-size:0.65rem;color:var(--text-secondary);opacity:0.5;margin-top:0.5rem;">
                                Mcaster1DNAS v2.5.1-rc1 &#183; GNU GPL v2 &#183; 2026
                            </div>
                        </div>
                    </div><!-- /tab-credits -->

                </div><!-- /player-tabs -->

                <!-- Hidden audio element -->
                <audio id="streamAudio" preload="none" crossorigin="anonymous" style="display:none;">
                    <source src="{source/@mount}" type="{source/server_type}"/>
                    Your browser does not support the audio element.
                </audio>

            </div><!-- /player-body -->
        </div><!-- /player-window -->

        <!-- ══ RIGHT: ICY2 Station Notice Board ══ -->
        <div class="notice-panel">
            <div class="notice-panel-header">
                <i class="fas fa-satellite-dish"></i> Station Board
            </div>
            <div class="notice-scroll" id="noticeBoard">
                <div class="notice-loading">
                    <i class="fas fa-circle-notch fa-spin"></i>
                </div>
            </div>
        </div><!-- /notice-panel -->

    </div><!-- /page-layout -->

    <!-- XSLT config vars: outside CDATA so XSLT processor evaluates them -->
    <script>
        var streamUrl  = '<xsl:value-of select="source/@mount"/>';
        var streamType = '<xsl:value-of select="source/server_type"/>';
    </script>
    <script>
    //<![CDATA[
        // streamUrl / streamType defined above (outside CDATA)
        // Title / station read from DOM to avoid JS string-escaping issues with apostrophes
        let streamTitle  = '';
        let streamArtist = '';
        let stationName  = '';

        // Player state
        let audioElement = document.getElementById('streamAudio');
        let isPlaying    = false;
        let audioContext = null;
        let analyser     = null;
        let source       = null;
        let vuMeterInterval = null;

        // VU peak state
        let peakLeft = 0, peakRight = 0;
        let peakHoldLeft = 0, peakHoldRight = 0;
        let peakHoldCountLeft = 0, peakHoldCountRight = 0;
        const PEAK_DECAY       = 0.92;
        const PEAK_HOLD_FRAMES = 20;

        // Song history tracking
        let historyLoaded = false;

        // ── Init ─────────────────────────────────────────────────────────────
        window.addEventListener('load', function() {
            var titleEl   = document.getElementById('currentTitle');
            var stationEl = document.querySelector('.station-name');
            streamTitle  = titleEl   ? titleEl.textContent.trim()   : '';
            stationName  = stationEl ? stationEl.textContent.trim() : 'Mcaster1DNAS';

            restartTicker(streamTitle || 'Connecting to Stream\u2026');
            updateDisplayClock();
            setInterval(updateDisplayClock, 1000);

            var savedVol = localStorage.getItem('mcaster1_volume');
            if (savedVol) {
                document.getElementById('volumeSlider').value = savedVol;
                adjustVolume();
            } else {
                audioElement.volume = 0.8;
            }

            setupAudioEvents();

            // Build initial music links from whatever XSLT gave us
            updateMusicLinks(streamTitle, streamArtist);

            // Start metadata polling
            updateMetadata();
            setInterval(updateMetadata, 2500);

            // Preload history in background
            setTimeout(loadSongHistory, 1500);
        });

        // ── Audio event handlers ──────────────────────────────────────────────
        function setupAudioEvents() {
            audioElement.addEventListener('play', function() {
                document.getElementById('statusText').textContent = 'PLAYING';
                document.getElementById('playingIndicator').style.color = '#10b981';
                hideBuffering();
                startVUMeters();
                if (streamTitle) startTabTicker(streamTitle);
            });

            audioElement.addEventListener('pause', function() {
                document.getElementById('statusText').textContent = 'PAUSED';
                document.getElementById('playingIndicator').style.color = '#fbbf24';
                stopVUMeters();
                stopTabTicker();
            });

            audioElement.addEventListener('ended', function() {
                document.getElementById('statusText').textContent = 'STOPPED';
                stopVUMeters();
                stopTabTicker();
            });

            audioElement.addEventListener('waiting', showBuffering);
            audioElement.addEventListener('canplay', hideBuffering);

            audioElement.addEventListener('error', function() {
                hideBuffering();
                document.getElementById('statusText').textContent = 'ERROR';
                alert('Stream playback error. Please try again.');
            });
        }

        // ── Playback controls ─────────────────────────────────────────────────
        function playStream() {
            showBuffering();
            if (audioContext && audioContext.state === 'suspended') audioContext.resume();
            audioElement.load();
            audioElement.play().catch(function(err) {
                hideBuffering();
                console.error('Playback failed:', err);
                alert('Unable to play stream. Please check your browser settings.');
            });
            isPlaying = true;
            if (streamTitle) startTabTicker(streamTitle);
            else updateDocumentTitle();
        }

        function pauseStream() { audioElement.pause(); isPlaying = false; }

        function stopStream() {
            audioElement.pause();
            audioElement.currentTime = 0;
            isPlaying = false;
            document.getElementById('statusText').textContent = 'STOPPED';
            stopVUMeters();
        }

        function adjustVolume() {
            var slider = document.getElementById('volumeSlider');
            var level  = document.getElementById('volumeLevel');
            var icon   = document.getElementById('volumeIcon');
            var v = slider.value;
            audioElement.volume = v / 100;
            level.textContent = v + '%';
            icon.className = v == 0 ? 'fas fa-volume-mute volume-icon'
                           : v  < 50 ? 'fas fa-volume-down volume-icon'
                           : 'fas fa-volume-up volume-icon';
            localStorage.setItem('mcaster1_volume', v);
        }

        function showBuffering() { document.getElementById('bufferingOverlay').classList.add('active'); }
        function hideBuffering() { document.getElementById('bufferingOverlay').classList.remove('active'); }

        // ── Tab management ────────────────────────────────────────────────────
        function switchTab(name) {
            ['stats', 'history', 'credits'].forEach(function(t) {
                document.getElementById('tab-' + t).classList.toggle('active', t === name);
                document.getElementById('tbtn-' + t).classList.toggle('active', t === name);
            });
            if (name === 'history') {
                loadSongHistory();
            }
        }

        // ── Music service links ───────────────────────────────────────────────
        function buildMusicLinksHTML(title, artist) {
            if (!title || title === 'Connecting to Stream\u2026') return '';
            var q  = encodeURIComponent((artist ? artist + ' ' : '') + title);
            var qp = ((artist ? artist + ' ' : '') + title).replace(/\s+/g, '+');
            return '<a class="np-music-link npm-mb" href="https://musicbrainz.org/search?query=' + qp + '&amp;type=recording"'
                 + ' target="_blank" rel="noopener noreferrer" title="Search MusicBrainz for this track">'
                 + '<i class="fas fa-database"></i> MusicBrainz</a>'
                 + '<a class="np-music-link npm-lfm" href="https://www.last.fm/search?q=' + q + '"'
                 + ' target="_blank" rel="noopener noreferrer" title="Search Last.fm for this track">'
                 + '<i class="fab fa-lastfm"></i> Last.fm</a>'
                 + '<a class="np-music-link npm-dc" href="https://www.discogs.com/search/?q=' + qp + '&amp;type=release"'
                 + ' target="_blank" rel="noopener noreferrer" title="Search Discogs for this track">'
                 + '<i class="fas fa-record-vinyl"></i> Discogs</a>'
                 + '<a class="np-music-link npm-am" href="https://www.allmusic.com/search/songs/' + qp + '"'
                 + ' target="_blank" rel="noopener noreferrer" title="Search AllMusic for this track">'
                 + '<i class="fas fa-music"></i> AllMusic</a>';
        }

        function buildHistoryLinksHTML(title, artist) {
            var qp = ((artist ? artist + ' ' : '') + title).replace(/\s+/g, '+');
            var q  = encodeURIComponent((artist ? artist + ' ' : '') + title);
            return '<a class="history-link hl-mb" href="https://musicbrainz.org/search?query=' + qp + '&amp;type=recording"'
                 + ' target="_blank" rel="noopener noreferrer" title="MusicBrainz"><i class="fas fa-database"></i></a>'
                 + '<a class="history-link hl-lfm" href="https://www.last.fm/search?q=' + q + '"'
                 + ' target="_blank" rel="noopener noreferrer" title="Last.fm"><i class="fab fa-lastfm"></i></a>'
                 + '<a class="history-link hl-dc" href="https://www.discogs.com/search/?q=' + qp + '&amp;type=release"'
                 + ' target="_blank" rel="noopener noreferrer" title="Discogs"><i class="fas fa-record-vinyl"></i></a>'
                 + '<a class="history-link hl-am" href="https://www.allmusic.com/search/songs/' + qp + '"'
                 + ' target="_blank" rel="noopener noreferrer" title="AllMusic"><i class="fas fa-music"></i></a>';
        }

        function updateMusicLinks(title, artist) {
            var el = document.getElementById('musicLinks');
            if (el) el.innerHTML = buildMusicLinksHTML(title, artist);
        }

        // ── Song history loader (admin: fetches /admin/songdata.xsl) ──────────
        function loadSongHistory() {
            fetch('/admin/songdata.xsl')
                .then(function(r) {
                    if (!r.ok) throw new Error('HTTP ' + r.status);
                    return r.text();
                })
                .then(function(html) {
                    var doc  = new DOMParser().parseFromString(html, 'text/html');
                    var rows = doc.querySelectorAll('.track-history-table tbody tr');
                    var seen = new Set();
                    var entries = [];

                    rows.forEach(function(row) {
                        var mountEl  = row.querySelector('.track-mount-code');
                        var titleEl  = row.querySelector('.track-title-text');
                        var artistEl = row.querySelector('.track-artist-text');
                        if (!mountEl || !titleEl) return;

                        var mount  = mountEl.textContent.trim();
                        if (mount !== streamUrl) return;

                        // Strip out the now-playing-dot text node if any
                        var title  = titleEl.textContent.replace(/^\s*\u25cf?\s*/, '').trim();
                        var artist = artistEl ? artistEl.textContent.replace(/^\s*[\u{1F464}\s]*/, '').trim() : '';

                        // De-duplicate: same title + artist = same track
                        var key = (title + '|' + artist).toLowerCase();
                        if (seen.has(key) || !title || title === 'Unknown Title') return;
                        seen.add(key);

                        entries.push({ title: title, artist: artist });
                    });

                    renderHistory(entries);
                    historyLoaded = true;
                })
                .catch(function(err) {
                    console.warn('Song history fetch failed:', err);
                    var el = document.getElementById('historyList');
                    if (el) el.innerHTML = '<div class="history-empty">'
                        + '<i class="fas fa-exclamation-circle" style="display:block;font-size:1.25rem;margin-bottom:0.4rem;opacity:0.4;"></i>'
                        + 'History unavailable</div>';
                });
        }

        function renderHistory(entries) {
            var el = document.getElementById('historyList');
            if (!el) return;
            if (entries.length === 0) {
                el.innerHTML = '<div class="history-empty">'
                    + '<i class="fas fa-history" style="display:block;font-size:1.5rem;margin-bottom:0.4rem;opacity:0.3;"></i>'
                    + 'No history for this stream yet.</div>';
                return;
            }
            var html = '';
            entries.forEach(function(e, i) {
                html += '<div class="history-entry">'
                      + '<span class="history-num">' + (i + 1) + '</span>'
                      + '<div class="history-info">'
                      +   '<div class="history-title">'
                      +     (i === 0 ? '<span class="history-now-dot"></span>' : '')
                      +     escapeHtml(e.title)
                      +   '</div>'
                      +   (e.artist ? '<div class="history-artist"><i class="fas fa-user" style="font-size:0.58rem;margin-right:0.2rem;opacity:0.55;"></i>' + escapeHtml(e.artist) + '</div>' : '')
                      + '</div>'
                      + '<div class="history-links">' + buildHistoryLinksHTML(e.title, e.artist) + '</div>'
                      + '</div>';
            });
            el.innerHTML = html;
        }

        function escapeHtml(str) {
            return str.replace(/&/g, '&amp;')
                      .replace(/</g, '&lt;')
                      .replace(/>/g, '&gt;')
                      .replace(/"/g, '&quot;');
        }

        // ── Notice board ──────────────────────────────────────────────────────
        function updateNoticeBoard(src) {
            var board = document.getElementById('noticeBoard');
            if (!board) return;
            var items = [];

            // Station info block
            if (src.server_name) {
                items.push({
                    tag: 'Station',
                    text: '<strong>' + escapeHtml(src.server_name) + '</strong>'
                        + (src.genre ? '<br/><i class="fas fa-music" style="font-size:0.6rem;opacity:0.6;margin-right:0.2rem;"></i>' + escapeHtml(src.genre) : '')
                        + (src.server_url ? '<br/><a href="' + escapeHtml(src.server_url) + '" target="_blank" rel="noopener noreferrer"><i class="fas fa-globe" style="font-size:0.6rem;margin-right:0.2rem;"></i>' + escapeHtml(src.server_url) + '</a>' : ''),
                    border: '#0891b2'
                });
            }

            // Live listeners
            var listenersStr = (src.listeners !== undefined ? src.listeners : '\u2014') + ' listening';
            if (src.listener_peak) listenersStr += ' &bull; peak ' + src.listener_peak;
            items.push({
                tag: 'Live Now',
                text: '<i class="fas fa-users" style="font-size:0.6rem;margin-right:0.2rem;opacity:0.7;"></i>' + listenersStr,
                border: '#10b981'
            });

            // Description / ICY2 notice
            if (src.server_description && src.server_description.trim()) {
                items.push({
                    tag: 'About This Stream',
                    text: escapeHtml(src.server_description),
                    border: '#14b8a6'
                });
            }

            // ICY2-specific fields — shown if server sends them
            var icy2Fields = [
                { key: 'icy_notice1',     label: 'Notice' },
                { key: 'icy_notice2',     label: 'Notice' },
                { key: 'icy_upcoming',    label: 'Coming Up' },
                { key: 'icy_show',        label: 'Show' },
                { key: 'icy_episode',     label: 'Episode' },
                { key: 'icy_hashtags',    label: 'Hashtags' },
                { key: 'icy_description', label: 'Info' },
            ];
            icy2Fields.forEach(function(f) {
                if (src[f.key] && src[f.key].trim()) {
                    items.push({
                        tag: f.label,
                        text: escapeHtml(src[f.key]),
                        border: '#fbbf24',
                        icy2: true
                    });
                }
            });

            if (items.length === 0) {
                board.innerHTML = '<div class="notice-loading" style="opacity:0.5;">No notices.</div>';
                return;
            }

            var html = '';
            items.forEach(function(item) {
                html += '<div class="notice-item" style="border-left-color:' + item.border + '">'
                      + (item.icy2 ? '<div class="notice-icy2-badge"><i class="fas fa-satellite-dish"></i> ICY2</div>' : '')
                      + '<div class="notice-tag">' + item.tag + '</div>'
                      + '<div class="notice-text">' + item.text + '</div>'
                      + '</div>';
            });
            board.innerHTML = html;
        }

        // ── Metadata poll ─────────────────────────────────────────────────────
        function updateMetadata() {
            fetch('/status-json.xsl')
                .then(function(r) { return r.json(); })
                .then(function(data) {
                    var stats = (data && (data.mcaster1stats || data.icestats)) || null;
                    if (!stats || !stats.source) return;

                    var sources = Array.isArray(stats.source) ? stats.source : [stats.source];
                    var src = sources.find(function(s) { return s.mount === streamUrl; });
                    if (!src) return;

                    // Update song title + ticker + music links
                    var newTitle  = src.title || src.yp_currently_playing || '';
                    var newArtist = src.artist || '';
                    var titleEl   = document.getElementById('currentTitle');

                    if (titleEl && newTitle && titleEl.textContent !== newTitle) {
                        titleEl.textContent = newTitle;
                        streamTitle  = newTitle;
                        streamArtist = newArtist;
                        restartTicker(newTitle);
                        updateMusicLinks(newTitle, newArtist);
                        if (isPlaying) startTabTicker(newTitle);
                        // Refresh history silently on title change
                        loadSongHistory();
                    }
                    if (newTitle && !streamTitle) {
                        streamTitle  = newTitle;
                        streamArtist = newArtist;
                        updateMusicLinks(newTitle, newArtist);
                    }
                    updateDocumentTitle();

                    // Update listeners
                    var lEl = document.getElementById('listenersValue');
                    if (lEl && src.listeners !== undefined) {
                        lEl.innerHTML = '<i class="fas fa-users"></i> ' + (src.listeners || 0);
                    }

                    // Update peak
                    var pEl = document.getElementById('peakListenersValue');
                    if (pEl && src.listener_peak !== undefined) {
                        pEl.innerHTML = '<i class="fas fa-chart-line"></i> ' + (src.listener_peak || 0);
                    }

                    // Update notice board
                    updateNoticeBoard(src);
                })
                .catch(function(err) { console.warn('Metadata poll failed:', err); });
        }

        // ── VU Meters ─────────────────────────────────────────────────────────
        function startVUMeters() {
            if (!audioContext) {
                try {
                    audioContext = new (window.AudioContext || window.webkitAudioContext)();
                    analyser = audioContext.createAnalyser();
                    analyser.fftSize = 2048;
                    analyser.smoothingTimeConstant = 0.4;
                    if (!source) {
                        source = audioContext.createMediaElementSource(audioElement);
                        source.connect(analyser);
                        analyser.connect(audioContext.destination);
                    }
                } catch (e) {
                    console.error('Web Audio API not supported:', e);
                    return;
                }
            }

            if (audioContext.state === 'suspended') {
                audioContext.resume().catch(function(e) { console.warn('AudioContext resume failed:', e); });
            }

            var bufferLength = analyser.fftSize;
            var timeData     = new Float32Array(bufferLength);
            peakLeft = 0; peakRight = 0;

            vuMeterInterval = setInterval(function() {
                analyser.getFloatTimeDomainData(timeData);
                var sumSqL = 0, sumSqR = 0;
                var half = bufferLength / 2;
                for (var i = 0; i < half; i++) {
                    sumSqL += timeData[i]        * timeData[i];
                    sumSqR += timeData[i + half] * timeData[i + half];
                }
                var rmsL = Math.min(1.0, Math.sqrt(sumSqL / half) * 3.0);
                var rmsR = Math.min(1.0, Math.sqrt(sumSqR / half) * 3.0);
                var pctL = rmsL * 100;
                var pctR = rmsR * 100;

                peakLeft  = Math.max(pctL, peakLeft  * PEAK_DECAY);
                peakRight = Math.max(pctR, peakRight * PEAK_DECAY);

                document.getElementById('vuMeterLeft').style.width  = peakLeft  + '%';
                document.getElementById('vuMeterRight').style.width = peakRight + '%';

                if (pctL >= peakHoldLeft)  { peakHoldLeft  = pctL;  peakHoldCountLeft  = PEAK_HOLD_FRAMES; }
                else if (peakHoldCountLeft  > 0) peakHoldCountLeft--;
                else peakHoldLeft  = Math.max(0, peakHoldLeft  - 1.5);

                if (pctR >= peakHoldRight) { peakHoldRight = pctR;  peakHoldCountRight = PEAK_HOLD_FRAMES; }
                else if (peakHoldCountRight > 0) peakHoldCountRight--;
                else peakHoldRight = Math.max(0, peakHoldRight - 1.5);

                document.getElementById('vuPeakLeft').style.left  = Math.min(98, peakHoldLeft)  + '%';
                document.getElementById('vuPeakRight').style.left = Math.min(98, peakHoldRight) + '%';
            }, 60);
        }

        function stopVUMeters() {
            if (vuMeterInterval) { clearInterval(vuMeterInterval); vuMeterInterval = null; }
            peakLeft = 0; peakRight = 0; peakHoldLeft = 0; peakHoldRight = 0;
            document.getElementById('vuMeterLeft').style.width   = '0%';
            document.getElementById('vuMeterRight').style.width  = '0%';
            document.getElementById('vuPeakLeft').style.left     = '0%';
            document.getElementById('vuPeakRight').style.left    = '0%';
        }

        // ── Ticker helpers ────────────────────────────────────────────────────
        function restartTicker(text) {
            var el = document.getElementById('currentTitle');
            if (!el || !text) return;
            var dur = Math.max(10, Math.round(text.length * 0.28 + 9));
            el.style.animation = 'none';
            void el.offsetWidth;
            el.style.animation = 'ticker-scroll ' + dur + 's linear infinite';
        }

        function updateDisplayClock() {
            var cl = document.getElementById('displayClock');
            if (!cl) return;
            var d = new Date();
            cl.textContent =
                String(d.getHours()).padStart(2, '0') + ':' +
                String(d.getMinutes()).padStart(2, '0') + ':' +
                String(d.getSeconds()).padStart(2, '0');
        }

        // Browser tab title ticker
        var _titlePos = 0, _titleStr = '', _titleInterval = null;

        function startTabTicker(text) {
            if (_titleInterval) clearInterval(_titleInterval);
            _titleStr = text + '   \u25cf   ';
            _titlePos = 0;
            _titleInterval = setInterval(function() {
                var r = _titleStr.slice(_titlePos) + _titleStr.slice(0, _titlePos);
                document.title = '\u25b6 ' + r.slice(0, 60);
                _titlePos = (_titlePos + 1) % _titleStr.length;
            }, 160);
        }

        function stopTabTicker() {
            if (_titleInterval) { clearInterval(_titleInterval); _titleInterval = null; }
            updateDocumentTitle();
        }

        function updateDocumentTitle() {
            if (_titleInterval) return;
            document.title = streamTitle
                ? (isPlaying ? '\u25b6 ' : '') + streamTitle + ' \u2014 ' + stationName
                : 'Mcaster1DNAS Player \u2014 ' + stationName;
        }

        // ── Bookmark hint ─────────────────────────────────────────────────────
        function showBookmarkHint() {
            alert('To bookmark this player:\n\n'
                + '\u2022 Desktop: Press Ctrl+D (Windows/Linux) or Cmd+D (Mac)\n'
                + '\u2022 Mobile: Tap the browser menu and select "Add to Home Screen"\n\n'
                + 'This allows you to quickly access this stream anytime!');
        }

        // ── Keyboard shortcuts ────────────────────────────────────────────────
        document.addEventListener('keydown', function(e) {
            if (e.code === 'Space') {
                e.preventDefault();
                isPlaying ? pauseStream() : playStream();
            } else if (e.code === 'Escape') {
                stopStream();
            }
        });
    //]]>
    </script>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

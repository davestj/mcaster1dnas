<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>
<xsl:include href="header.xsl"/>
<xsl:include href="footer.xsl"/>

<xsl:template match="/mcaster1songdata">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Track History - Mcaster1DNAS</title>

    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css"
          integrity="sha512-iecdLmaskl7CVkqkXNQ/ZH/XLlvWZOJyj7Yy7tcenmpD1ypASozpmT/E0iPtmFIB46ZmdtAc9eNBvH0H/ZpiBw=="
          crossorigin="anonymous" referrerpolicy="no-referrer"/>

    <link rel="stylesheet" type="text/css" href="/style.css"/>
    <link rel="icon" type="image/x-icon" href="/favicon.ico"/>
    <link rel="icon" type="image/png" sizes="32x32" href="/favicon-32x32.png"/>
    <link rel="icon" type="image/png" sizes="16x16" href="/favicon-16x16.png"/>
    <link rel="apple-touch-icon" sizes="180x180" href="/apple-touch-icon.png"/>
    <script src="/mcaster-utils.js"></script>

    <style>
        .track-history-table {
            width: 100%;
            border-collapse: collapse;
            font-size: 0.875rem;
        }
        .track-history-table th {
            background: var(--bg-light);
            padding: 0.65rem 0.875rem;
            text-align: left;
            font-weight: 600;
            color: var(--text-secondary);
            font-size: 0.7rem;
            text-transform: uppercase;
            letter-spacing: 0.05em;
            border-bottom: 2px solid var(--border-medium);
            white-space: nowrap;
        }
        .track-history-table td {
            padding: 0.65rem 0.875rem;
            border-bottom: 1px solid var(--border-light);
            vertical-align: middle;
        }
        .track-history-table tbody tr:hover td {
            background: var(--bg-light);
        }
        .track-num {
            width: 2.5rem;
            text-align: center;
            font-weight: 600;
            color: var(--text-secondary);
        }
        .track-row-latest td {
            background: rgba(14, 165, 233, 0.04);
        }
        .now-playing-dot {
            display: inline-block;
            width: 8px;
            height: 8px;
            background: var(--dnas-green);
            border-radius: 50%;
            margin-right: 0.35rem;
            animation: pulse-dot 2s ease-in-out infinite;
        }
        @keyframes pulse-dot {
            0%, 100% { opacity: 1; }
            50%       { opacity: 0.25; }
        }
        .track-title-text {
            font-weight: 600;
            color: var(--text-primary);
            line-height: 1.3;
        }
        .track-artist-text {
            font-size: 0.78rem;
            color: var(--text-secondary);
            margin-top: 0.15rem;
        }
        .track-mount-code {
            font-family: monospace;
            font-size: 0.78rem;
            background: var(--bg-light);
            padding: 0.15rem 0.4rem;
            border-radius: var(--radius-sm);
            color: var(--mcaster-blue);
            white-space: nowrap;
            border: 1px solid var(--border-light);
        }
        .quality-badge {
            display: inline-block;
            font-size: 0.72rem;
            background: var(--bg-light);
            border: 1px solid var(--border-light);
            border-radius: var(--radius-sm);
            padding: 0.18rem 0.45rem;
            white-space: nowrap;
            color: var(--text-secondary);
        }
        .track-time {
            white-space: nowrap;
            color: var(--text-secondary);
            font-size: 0.8rem;
        }
        .on-air-time {
            font-weight: 600;
            color: var(--text-primary);
        }
        .listeners-count {
            text-align: center;
            font-weight: 600;
            color: var(--text-primary);
        }
        .music-links {
            display: flex;
            gap: 0.35rem;
            align-items: center;
        }
        .music-link {
            display: inline-flex;
            align-items: center;
            justify-content: center;
            width: 1.7rem;
            height: 1.7rem;
            border-radius: var(--radius-sm);
            text-decoration: none;
            font-size: 0.78rem;
            transition: all 0.15s ease;
            border: 1px solid transparent;
        }
        .music-link:hover {
            transform: translateY(-2px);
            box-shadow: 0 3px 8px rgba(0,0,0,0.18);
        }
        .ml-mb  { color:#f97316; background:rgba(249,115,22,0.1); border-color:rgba(249,115,22,0.25); }
        .ml-mb:hover  { background:#f97316; color:#fff; border-color:#f97316; }
        .ml-lfm { color:#ef4444; background:rgba(239,68,68,0.1); border-color:rgba(239,68,68,0.25); }
        .ml-lfm:hover { background:#ef4444; color:#fff; border-color:#ef4444; }
        .ml-dc  { color:#475569; background:rgba(71,85,105,0.1); border-color:rgba(71,85,105,0.25); }
        .ml-dc:hover  { background:#475569; color:#fff; border-color:#475569; }
        .ml-am  { color:#14b8a6; background:rgba(20,184,166,0.1); border-color:rgba(20,184,166,0.25); }
        .ml-am:hover  { background:#14b8a6; color:#fff; border-color:#14b8a6; }
        .empty-history {
            text-align: center;
            padding: 3rem 2rem;
            color: var(--text-secondary);
        }
        .empty-history i {
            font-size: 3rem;
            display: block;
            margin-bottom: 1rem;
            opacity: 0.25;
        }
        @media (max-width: 768px) {
            .track-history-table th.hide-mobile,
            .track-history-table td.hide-mobile { display: none; }
        }
    </style>
</head>
<body>
    <xsl:call-template name="web-header">
        <xsl:with-param name="active-page" select="'songdata'"/>
        <xsl:with-param name="title" select="'Track History - Mcaster1DNAS'"/>
    </xsl:call-template>

    <div class="mcaster-main">
        <div class="mcaster-container">

            <div class="mcaster-card">
                <h2>
                    <i class="fas fa-history"></i> Track History
                    <span class="tooltip">
                        <span class="help-icon">?</span>
                        <span class="tooltiptext">
                            Recently played tracks across all active stream mounts, newest first.
                            Click the service icons on each row to look up track and album
                            information on external music databases.
                        </span>
                    </span>
                </h2>

                <!-- Summary bar -->
                <div class="stat-grid" style="margin-bottom:1.25rem;">
                    <div class="stat-box">
                        <span class="stat-value"><xsl:value-of select="total_entries"/></span>
                        <span class="stat-label">
                            <i class="fas fa-music"></i> Songs Tracked
                        </span>
                    </div>
                    <div class="stat-box">
                        <span class="stat-value" style="font-size:0.875rem;font-weight:500;">
                            <xsl:value-of select="generated"/>
                        </span>
                        <span class="stat-label">
                            <i class="fas fa-sync-alt"></i> Page Generated
                        </span>
                    </div>
                </div>

                <!-- Refresh link -->
                <div style="margin-bottom:1rem;">
                    <a href="/songdata.xsl" style="
                        display:inline-flex; align-items:center; gap:0.4rem;
                        padding:0.45rem 0.9rem; background:var(--bg-white);
                        border:2px solid var(--border-medium); border-radius:var(--radius-md);
                        color:var(--text-primary); text-decoration:none;
                        font-size:0.8rem; font-weight:500; transition:all var(--transition-fast);">
                        <i class="fas fa-sync-alt"></i> Refresh
                    </a>
                    <a href="/status.xsl" style="
                        display:inline-flex; align-items:center; gap:0.4rem;
                        padding:0.45rem 0.9rem; margin-left:0.4rem; background:var(--bg-white);
                        border:2px solid var(--border-medium); border-radius:var(--radius-md);
                        color:var(--text-primary); text-decoration:none;
                        font-size:0.8rem; font-weight:500; transition:all var(--transition-fast);">
                        <i class="fas fa-home"></i> Status
                    </a>
                </div>

                <!-- Track table -->
                <xsl:choose>
                    <xsl:when test="entry">
                        <div style="overflow-x:auto;">
                        <table class="track-history-table">
                            <thead>
                                <tr>
                                    <th class="track-num">#</th>
                                    <th>Mount</th>
                                    <th>Artist / Title</th>
                                    <th class="hide-mobile">Quality</th>
                                    <th class="hide-mobile">Played At</th>
                                    <th>On Air</th>
                                    <th style="text-align:center;" title="Listeners at play time">
                                        <i class="fas fa-headphones"></i>
                                    </th>
                                    <th title="Look up on music services">Lookup</th>
                                </tr>
                            </thead>
                            <tbody>
                                <xsl:for-each select="entry">
                                    <xsl:variable name="sq">
                                        <xsl:value-of select="translate(concat(artist,' ',title),' ','+')"/>
                                    </xsl:variable>
                                    <tr>
                                        <xsl:if test="position() = 1">
                                            <xsl:attribute name="class">track-row-latest</xsl:attribute>
                                        </xsl:if>
                                        <td class="track-num">
                                            <xsl:value-of select="position()"/>
                                        </td>
                                        <td>
                                            <span class="track-mount-code">
                                                <xsl:value-of select="mount"/>
                                            </span>
                                        </td>
                                        <td>
                                            <div class="track-title-text">
                                                <xsl:if test="position() = 1">
                                                    <span class="now-playing-dot"></span>
                                                </xsl:if>
                                                <xsl:choose>
                                                    <xsl:when test="title != ''">
                                                        <xsl:value-of select="title"/>
                                                    </xsl:when>
                                                    <xsl:otherwise>
                                                        <em>Unknown Title</em>
                                                    </xsl:otherwise>
                                                </xsl:choose>
                                            </div>
                                            <xsl:if test="artist != ''">
                                                <div class="track-artist-text">
                                                    <i class="fas fa-user" style="font-size:0.65rem;margin-right:0.2rem;opacity:0.6;"></i>
                                                    <xsl:value-of select="artist"/>
                                                </div>
                                            </xsl:if>
                                        </td>
                                        <td class="hide-mobile">
                                            <span class="quality-badge">
                                                <xsl:value-of select="codec"/>
                                                <xsl:if test="number(bitrate) &gt; 0">
                                                    <xsl:text> </xsl:text><xsl:value-of select="bitrate"/>kbps
                                                </xsl:if>
                                                <xsl:if test="number(samplerate) &gt; 0">
                                                    <xsl:text> </xsl:text><xsl:value-of
                                                        select="floor(samplerate div 1000)"/>.<xsl:value-of
                                                        select="floor((samplerate mod 1000) div 100)"/>kHz
                                                </xsl:if>
                                                <xsl:choose>
                                                    <xsl:when test="number(channels) = 1"> Mono</xsl:when>
                                                    <xsl:when test="number(channels) = 2"> Stereo</xsl:when>
                                                    <xsl:when test="number(channels) &gt; 2">
                                                        <xsl:text> </xsl:text><xsl:value-of select="channels"/>ch
                                                    </xsl:when>
                                                </xsl:choose>
                                            </span>
                                        </td>
                                        <td class="track-time hide-mobile">
                                            <xsl:value-of select="played_at"/>
                                        </td>
                                        <td class="track-time">
                                            <span class="on-air-time">
                                                <xsl:choose>
                                                    <xsl:when test="number(on_air_seconds) &gt;= 3600">
                                                        <xsl:value-of select="floor(on_air_seconds div 3600)"/>h
                                                        <xsl:text> </xsl:text>
                                                        <xsl:value-of select="floor((on_air_seconds mod 3600) div 60)"/>m
                                                    </xsl:when>
                                                    <xsl:when test="number(on_air_seconds) &gt;= 60">
                                                        <xsl:value-of select="floor(on_air_seconds div 60)"/>m
                                                        <xsl:text> </xsl:text>
                                                        <xsl:value-of select="on_air_seconds mod 60"/>s
                                                    </xsl:when>
                                                    <xsl:otherwise>
                                                        <xsl:value-of select="on_air_seconds"/>s
                                                    </xsl:otherwise>
                                                </xsl:choose>
                                            </span>
                                        </td>
                                        <td class="listeners-count">
                                            <xsl:value-of select="listeners"/>
                                        </td>
                                        <td>
                                            <div class="music-links">
                                                <a class="music-link ml-mb" target="_blank" rel="noopener noreferrer"
                                                   title="Look up track &amp; album on MusicBrainz">
                                                    <xsl:attribute name="href">https://musicbrainz.org/search?query=<xsl:value-of select="$sq"/>&amp;type=recording</xsl:attribute>
                                                    <i class="fas fa-database"></i>
                                                </a>
                                                <a class="music-link ml-lfm" target="_blank" rel="noopener noreferrer"
                                                   title="Look up track &amp; album on Last.fm">
                                                    <xsl:attribute name="href">https://www.last.fm/search?q=<xsl:value-of select="$sq"/></xsl:attribute>
                                                    <i class="fab fa-lastfm"></i>
                                                </a>
                                                <a class="music-link ml-dc" target="_blank" rel="noopener noreferrer"
                                                   title="Look up track &amp; album on Discogs">
                                                    <xsl:attribute name="href">https://www.discogs.com/search/?q=<xsl:value-of select="$sq"/>&amp;type=release</xsl:attribute>
                                                    <i class="fas fa-record-vinyl"></i>
                                                </a>
                                                <a class="music-link ml-am" target="_blank" rel="noopener noreferrer"
                                                   title="Look up track &amp; album on AllMusic">
                                                    <xsl:attribute name="href">https://www.allmusic.com/search/songs/<xsl:value-of select="$sq"/></xsl:attribute>
                                                    <i class="fas fa-music"></i>
                                                </a>
                                            </div>
                                        </td>
                                    </tr>
                                </xsl:for-each>
                            </tbody>
                        </table>
                        </div>
                    </xsl:when>
                    <xsl:otherwise>
                        <div class="empty-history">
                            <i class="fas fa-history"></i>
                            <p>No track history yet.</p>
                            <p style="font-size:0.875rem;margin-top:0.5rem;">
                                Track history appears here once sources start streaming.
                            </p>
                        </div>
                    </xsl:otherwise>
                </xsl:choose>

            </div>
        </div>
    </div>

    <xsl:call-template name="web-footer"/>
</body>
</html>
</xsl:template>

</xsl:stylesheet>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>

<xsl:template match="/mcaster1stats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Stream Status - Mcaster1DNAS</title>

    <!-- FontAwesome 6.x for professional icons -->
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css"
          integrity="sha512-iecdLmaskl7CVkqkXNQ/ZH/XLlvWZOJyj7Yy7tcenmpD1ypASozpmT/E0iPtmFIB46ZmdtAc9eNBvH0H/ZpiBw=="
          crossorigin="anonymous" referrerpolicy="no-referrer"/>

    <link rel="stylesheet" type="text/css" href="/style.css"/>
    <script src="/mcaster-utils.js"></script>
</head>
<body>
    <div class="mcaster-header">
        <div class="mcaster-container">
            <div class="mcaster-header-top">
                <div class="mcaster-brand">
                    <div class="brand-icon">M1</div>
                    <div class="brand-text">
                        <h1 style="margin: 0; font-size: 1.75rem;">
                            <span class="brand-mcaster">Mcaster1</span>
                            <span class="brand-dnas">DNAS</span>
                        </h1>
                    </div>
                </div>
                <div class="mcaster-clock">
                    <div class="mcaster-clock-time">
                        <i class="fas fa-clock mcaster-clock-icon"></i>
                        <span id="live-time">Loading...</span>
                    </div>
                    <div class="mcaster-clock-date" id="live-date">Loading...</div>
                </div>
            </div>
            <div class="mcaster-nav">
                <a href="status.xsl" class="active"><i class="fas fa-home"></i> Status</a>
                <a href="server_version.xsl"><i class="fas fa-info-circle"></i> Server Info</a>
                <a href="credits.xsl"><i class="fas fa-award"></i> Credits</a>
                <a href="/admin/stats.xsl"><i class="fas fa-shield-alt"></i> Admin</a>
            </div>
        </div>
    </div>

    <div class="mcaster-main">
        <div class="mcaster-container">

            <!-- Server Stats Overview -->
            <div class="mcaster-card">
                <h2>
                    <i class="fas fa-chart-bar"></i> Server Statistics
                    <span class="tooltip">
                        <span class="help-icon">?</span>
                        <span class="tooltiptext">
                            Live streaming statistics showing current server activity and listener counts across all streams.
                        </span>
                    </span>
                </h2>
                <div class="stat-grid">
                    <div class="stat-box">
                        <span class="stat-value"><xsl:value-of select="listeners"/></span>
                        <span class="stat-label">
                            <i class="fas fa-headphones"></i> Total Listeners
                            <span class="tooltip">
                                <span class="help-icon">?</span>
                                <span class="tooltiptext">
                                    Currently connected listeners across all active streams
                                </span>
                            </span>
                        </span>
                    </div>
                    <div class="stat-box">
                        <span class="stat-value"><xsl:value-of select="sources"/></span>
                        <span class="stat-label">
                            <i class="fas fa-broadcast-tower"></i> Active Sources
                            <span class="tooltip">
                                <span class="help-icon">?</span>
                                <span class="tooltiptext">
                                    Number of live broadcasts currently streaming to the server
                                </span>
                            </span>
                        </span>
                    </div>
                    <div class="stat-box">
                        <span class="stat-value"><xsl:value-of select="clients"/></span>
                        <span class="stat-label">
                            <i class="fas fa-users"></i> Total Clients
                        </span>
                    </div>
                    <div class="stat-box">
                        <span class="stat-value"><xsl:value-of select="connections"/></span>
                        <span class="stat-label">
                            <i class="fas fa-plug"></i> Connections
                        </span>
                    </div>
                </div>
            </div>

            <!-- What are Mount Points? -->
            <xsl:if test="source">
                <div class="info-box">
                    <div class="info-box-title">
                        <i class="fas fa-question-circle"></i> What are Mount Points?
                    </div>
                    <div class="info-box-content">
                        Mount points are URLs where individual streams are available. Each stream can have different audio quality, format (MP3, AAC, Ogg, etc.), or content. Listeners connect to mount points to hear the broadcast.
                    </div>
                </div>
            </xsl:if>

            <!-- Active Mount Points -->
            <xsl:for-each select="source">
                <div class="mcaster-card">
                    <h2>
                        <i class="fas fa-stream"></i> <xsl:value-of select="@mount"/>
                        <xsl:if test="listeners">
                            <span class="status-badge status-active">
                                <i class="fas fa-circle" style="animation: pulse 2s infinite;"></i> LIVE - <xsl:value-of select="listeners"/> listeners
                            </span>
                        </xsl:if>
                        <span class="tooltip">
                            <span class="help-icon">?</span>
                            <span class="tooltiptext">
                                This mount point is the URL endpoint where listeners can connect to this specific stream. Copy the listen link below to share with your audience.
                            </span>
                        </span>
                    </h2>

                    <!-- Now Playing Section (Always Visible) -->
                    <div style="margin: 1.5rem 0;">
                        <h3 style="margin-bottom: 1rem;"><i class="fas fa-music"></i> Now Playing</h3>
                        <div style="background: var(--bg-light); padding: 1.5rem; border-radius: var(--radius-md); border-left: 4px solid var(--mcaster-blue);">
                            <div style="font-size: 1.25rem; font-weight: 600; color: var(--text-primary); margin-bottom: 0.5rem;">
                                <xsl:choose>
                                    <xsl:when test="title">
                                        <xsl:value-of select="title"/>
                                    </xsl:when>
                                    <xsl:otherwise>
                                        <span style="color: var(--text-secondary); font-style: italic;">No track information available</span>
                                    </xsl:otherwise>
                                </xsl:choose>
                            </div>
                            <xsl:if test="artist">
                                <div style="font-size: 1rem; color: var(--text-secondary); margin-bottom: 0.5rem;">
                                    <i class="fas fa-user"></i> <xsl:value-of select="artist"/>
                                </div>
                            </xsl:if>
                            <xsl:if test="server_description">
                                <div style="font-size: 0.875rem; color: var(--text-secondary); margin-top: 0.75rem;">
                                    <i class="fas fa-info-circle"></i> <xsl:value-of select="server_description"/>
                                </div>
                            </xsl:if>
                        </div>
                    </div>

                    <!-- Stream Information -->
                    <div style="margin: 1.5rem 0;">
                        <h3 style="margin-bottom: 1rem;"><i class="fas fa-info-circle"></i> Stream Information</h3>
                        <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 1rem;">
                            <!-- Format/Codec -->
                            <div style="background: white; padding: 1rem; border-radius: var(--radius-md); border: 1px solid var(--border-light);">
                                <div style="font-size: 0.75rem; color: var(--text-secondary); text-transform: uppercase; margin-bottom: 0.25rem;">Audio Format</div>
                                <div style="font-weight: 600; color: var(--text-primary);">
                                    <xsl:choose>
                                        <xsl:when test="server_type = 'audio/mpeg'">
                                            <i class="fas fa-file-audio" style="color: var(--mcaster-blue);"></i> MP3
                                        </xsl:when>
                                        <xsl:when test="server_type = 'application/ogg'">
                                            <i class="fas fa-file-audio" style="color: var(--dnas-green);"></i>
                                            <xsl:choose>
                                                <xsl:when test="subtype">
                                                    <xsl:value-of select="subtype"/>
                                                </xsl:when>
                                                <xsl:otherwise>Ogg Vorbis</xsl:otherwise>
                                            </xsl:choose>
                                        </xsl:when>
                                        <xsl:when test="server_type = 'audio/aac' or server_type = 'audio/aacp'">
                                            <i class="fas fa-file-audio" style="color: #f59e0b;"></i> AAC
                                        </xsl:when>
                                        <xsl:otherwise>
                                            <xsl:value-of select="server_type"/>
                                        </xsl:otherwise>
                                    </xsl:choose>
                                </div>
                            </div>

                            <!-- Bitrate -->
                            <div style="background: white; padding: 1rem; border-radius: var(--radius-md); border: 1px solid var(--border-light);">
                                <div style="font-size: 0.75rem; color: var(--text-secondary); text-transform: uppercase; margin-bottom: 0.25rem;">Audio Quality</div>
                                <div style="font-weight: 600; color: var(--text-primary);">
                                    <xsl:choose>
                                        <xsl:when test="ice-bitrate">
                                            <xsl:value-of select="ice-bitrate"/> kbps
                                        </xsl:when>
                                        <xsl:when test="bitrate">
                                            <xsl:value-of select="bitrate"/> kbps
                                        </xsl:when>
                                        <xsl:otherwise>Unknown</xsl:otherwise>
                                    </xsl:choose>
                                </div>
                            </div>

                            <!-- Sample Rate -->
                            <div style="background: white; padding: 1rem; border-radius: var(--radius-md); border: 1px solid var(--border-light);">
                                <div style="font-size: 0.75rem; color: var(--text-secondary); text-transform: uppercase; margin-bottom: 0.25rem;">Sample Rate</div>
                                <div style="font-weight: 600; color: var(--text-primary);">
                                    <xsl:choose>
                                        <xsl:when test="ice-samplerate">
                                            <xsl:value-of select="ice-samplerate"/> Hz
                                        </xsl:when>
                                        <xsl:when test="audio_samplerate">
                                            <xsl:value-of select="audio_samplerate"/> Hz
                                        </xsl:when>
                                        <xsl:otherwise>Unknown</xsl:otherwise>
                                    </xsl:choose>
                                </div>
                            </div>

                            <!-- Channels -->
                            <div style="background: white; padding: 1rem; border-radius: var(--radius-md); border: 1px solid var(--border-light);">
                                <div style="font-size: 0.75rem; color: var(--text-secondary); text-transform: uppercase; margin-bottom: 0.25rem;">Channels</div>
                                <div style="font-weight: 600; color: var(--text-primary);">
                                    <xsl:choose>
                                        <xsl:when test="ice-channels = '2'">
                                            <i class="fas fa-volume-up"></i> Stereo
                                        </xsl:when>
                                        <xsl:when test="ice-channels = '1'">
                                            <i class="fas fa-volume-down"></i> Mono
                                        </xsl:when>
                                        <xsl:when test="ice-channels">
                                            <xsl:value-of select="ice-channels"/> ch
                                        </xsl:when>
                                        <xsl:otherwise>Unknown</xsl:otherwise>
                                    </xsl:choose>
                                </div>
                            </div>

                            <!-- Stream Uptime -->
                            <div style="background: white; padding: 1rem; border-radius: var(--radius-md); border: 1px solid var(--border-light);">
                                <div style="font-size: 0.75rem; color: var(--text-secondary); text-transform: uppercase; margin-bottom: 0.25rem;">Stream Uptime</div>
                                <div style="font-weight: 600; color: var(--text-primary);">
                                    <xsl:choose>
                                        <xsl:when test="connected">
                                            <xsl:value-of select="connected"/>s
                                        </xsl:when>
                                        <xsl:otherwise>N/A</xsl:otherwise>
                                    </xsl:choose>
                                </div>
                            </div>

                            <!-- Peak Listeners -->
                            <div style="background: white; padding: 1rem; border-radius: var(--radius-md); border: 1px solid var(--border-light);">
                                <div style="font-size: 0.75rem; color: var(--text-secondary); text-transform: uppercase; margin-bottom: 0.25rem;">Peak Listeners</div>
                                <div style="font-weight: 600; color: var(--text-primary);">
                                    <xsl:choose>
                                        <xsl:when test="listener_peak">
                                            <i class="fas fa-chart-line" style="color: var(--dnas-green);"></i> <xsl:value-of select="listener_peak"/>
                                        </xsl:when>
                                        <xsl:otherwise>0</xsl:otherwise>
                                    </xsl:choose>
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- Current Listeners Section -->
                    <h3 style="margin-top: 1.5rem; margin-bottom: 1rem;">
                        <i class="fas fa-headphones"></i> Current Listeners
                        <xsl:if test="listeners">
                            <span class="status-badge status-active" style="font-size: 0.875rem; margin-left: 0.5rem;">
                                <xsl:value-of select="listeners"/> listening now
                            </span>
                        </xsl:if>
                    </h3>

                    <table>
                        <xsl:if test="server_name">
                            <tr>
                                <th style="width: 200px">Stream Title</th>
                                <td><xsl:value-of select="server_name"/></td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="server_description">
                            <tr>
                                <th>Description</th>
                                <td><xsl:value-of select="server_description"/></td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="title">
                            <tr>
                                <th>Now Playing</th>
                                <td>
                                    <strong><xsl:if test="artist"><xsl:value-of select="artist"/> - </xsl:if><xsl:value-of select="title"/></strong>
                                </td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="server_type">
                            <tr>
                                <th>Content Type</th>
                                <td><xsl:value-of select="server_type"/></td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="bitrate">
                            <tr>
                                <th>Bitrate</th>
                                <td><xsl:value-of select="bitrate"/> kbps</td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="quality">
                            <tr>
                                <th>Quality</th>
                                <td><xsl:value-of select="quality"/></td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="genre">
                            <tr>
                                <th>Genre</th>
                                <td><xsl:value-of select="genre"/></td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="listener_peak">
                            <tr>
                                <th>Peak Listeners</th>
                                <td><xsl:value-of select="listener_peak"/></td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="stream_start">
                            <tr>
                                <th>Stream Started</th>
                                <td><xsl:value-of select="stream_start"/></td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="server_url">
                            <tr>
                                <th>Stream URL</th>
                                <td><a href="{server_url}" target="_blank"><xsl:value-of select="server_url"/></a></td>
                            </tr>
                        </xsl:if>
                    </table>

                    <div style="margin-top: 1rem;">
                        <xsl:choose>
                            <xsl:when test="authenticator">
                                <a href="./auth.xsl" class="btn btn-primary">Login to Listen</a>
                            </xsl:when>
                            <xsl:otherwise>
                                <a href="javascript:void(0);" onclick="window.open('/webplayer.xsl?mount={@mount}', 'mcaster1player', 'width=650,height=800,resizable=yes,scrollbars=yes'); return false;" class="btn btn-primary" title="Open web player in new window - bookmark for quick access" style="margin-right: 0.5rem;">
                                    <i class="fas fa-play-circle"></i> Web Player
                                </a>
                                <a href=".{@mount}.m3u" class="btn btn-secondary">Listen (M3U)</a>
                                <a href=".{@mount}.xspf" class="btn btn-secondary">Listen (XSPF)</a>
                            </xsl:otherwise>
                        </xsl:choose>
                    </div>
                </div>
            </xsl:for-each>

            <!-- No Active Streams Message -->
            <xsl:if test="not(source)">
                <div class="mcaster-card text-center">
                    <h2>No Active Streams</h2>
                    <p>There are currently no active broadcasts. Check back later!</p>
                </div>
            </xsl:if>

        </div>
    </div>

    <div class="mcaster-footer">
        <div class="mcaster-container">
            <p>Powered by <a href="https://mcaster1.com">Mcaster1DNAS</a> - Digital Network Audio Server
                <span class="page-load-time" id="page-load-time">
                    <i class="fas fa-spinner fa-spin"></i> Loading...
                </span>
            </p>
        </div>
    </div>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

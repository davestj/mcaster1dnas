<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>
<xsl:include href="header.xsl"/>
<xsl:include href="footer.xsl"/>

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
    <link rel="icon" type="image/x-icon" href="/favicon.ico"/>
    <link rel="icon" type="image/png" sizes="32x32" href="/favicon-32x32.png"/>
    <link rel="icon" type="image/png" sizes="16x16" href="/favicon-16x16.png"/>
    <link rel="apple-touch-icon" sizes="180x180" href="/apple-touch-icon.png"/>
    <script src="/mcaster-utils.js"></script>
</head>
<body>
    <xsl:call-template name="web-header">
        <xsl:with-param name="active-page" select="'status'"/>
        <xsl:with-param name="title" select="'Stream Status - Mcaster1DNAS'"/>
    </xsl:call-template>

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
                        <xsl:choose>
                            <xsl:when test="mount_type = 'podcast'">
                                <span class="status-badge" style="background:rgba(139,92,246,0.15);color:#7c3aed;border:1px solid rgba(139,92,246,0.3);">
                                    <i class="fas fa-podcast"></i> PODCAST
                                </span>
                            </xsl:when>
                            <xsl:when test="mount_type = 'socialmedia' or mount_type = 'socialcast'">
                                <span class="status-badge" style="background:rgba(236,72,153,0.15);color:#db2777;border:1px solid rgba(236,72,153,0.3);">
                                    <i class="fas fa-share-alt"></i> SOCIAL MEDIA
                                </span>
                            </xsl:when>
                            <xsl:when test="mount_type = 'ondemand'">
                                <span class="status-badge" style="background:rgba(245,158,11,0.15);color:#d97706;border:1px solid rgba(245,158,11,0.3);">
                                    <i class="fas fa-archive"></i> ON DEMAND
                                </span>
                            </xsl:when>
                            <xsl:when test="listeners and number(listeners) > 0">
                                <span class="status-badge status-active">
                                    <i class="fas fa-circle" style="animation: pulse 2s infinite;"></i> LIVE &#8212; <xsl:value-of select="listeners"/> listeners
                                </span>
                            </xsl:when>
                        </xsl:choose>
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
                                        <xsl:when test="server_type = 'audio/aacp'">
                                            <i class="fas fa-file-audio" style="color: #f59e0b;"></i> AAC+
                                        </xsl:when>
                                        <xsl:when test="server_type = 'audio/aac'">
                                            <i class="fas fa-file-audio" style="color: #f59e0b;"></i> AAC
                                        </xsl:when>
                                        <xsl:when test="server_type = 'audio/ogg' or server_type = 'application/ogg'">
                                            <i class="fas fa-file-audio" style="color: var(--dnas-green);"></i>
                                            <xsl:choose>
                                                <xsl:when test="subtype = 'Vorbis'">Ogg Vorbis</xsl:when>
                                                <xsl:when test="subtype">Ogg <xsl:value-of select="subtype"/></xsl:when>
                                                
                                                <xsl:otherwise>Opus</xsl:otherwise>
                                            </xsl:choose>
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
                                        <xsl:when test="mpeg_samplerate">
                                            <xsl:value-of select="mpeg_samplerate"/> Hz
                                        </xsl:when>
                                        <xsl:when test="audio_samplerate">
                                            <xsl:value-of select="audio_samplerate"/> Hz
                                        </xsl:when>
                                        <xsl:otherwise>N/A</xsl:otherwise>
                                    </xsl:choose>
                                </div>
                            </div>

                            <!-- Channels -->
                            <div style="background: white; padding: 1rem; border-radius: var(--radius-md); border: 1px solid var(--border-light);">
                                <div style="font-size: 0.75rem; color: var(--text-secondary); text-transform: uppercase; margin-bottom: 0.25rem;">Channels</div>
                                <div style="font-weight: 600; color: var(--text-primary);">
                                    <xsl:choose>
                                        <xsl:when test="mpeg_channels = '2' or audio_channels = '2'">
                                            <i class="fas fa-volume-up"></i> Stereo
                                        </xsl:when>
                                        <xsl:when test="mpeg_channels = '1' or audio_channels = '1'">
                                            <i class="fas fa-volume-down"></i> Mono
                                        </xsl:when>
                                        <xsl:when test="mpeg_channels">
                                            <xsl:value-of select="mpeg_channels"/> ch
                                        </xsl:when>
                                        <xsl:when test="audio_channels">
                                            <xsl:value-of select="audio_channels"/> ch
                                        </xsl:when>
                                        <xsl:otherwise>N/A</xsl:otherwise>
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

                        <!-- ICY2 v2.2 Extended Metadata -->
                        <xsl:if test="*[local-name()='icy2-version']">
                            <tr><th colspan="2" style="background:rgba(8,145,178,0.1);color:#0891b2;font-size:0.75rem;text-transform:uppercase;letter-spacing:0.08em;">ICY2 v2.2 Extended Metadata</th></tr>
                        </xsl:if>
                        <xsl:if test="*[local-name()='icy2-show-title']">
                            <tr><th>Show</th><td><xsl:value-of select="*[local-name()='icy2-show-title']"/></td></tr>
                        </xsl:if>
                        <xsl:if test="*[local-name()='icy2-show-episode']">
                            <tr><th>Episode</th><td><xsl:value-of select="*[local-name()='icy2-show-episode']"/></td></tr>
                        </xsl:if>
                        <xsl:if test="*[local-name()='icy2-dj-handle']">
                            <tr><th>DJ</th><td><xsl:value-of select="*[local-name()='icy2-dj-handle']"/></td></tr>
                        </xsl:if>
                        <xsl:if test="*[local-name()='icy2-track-artist']">
                            <tr><th>Artist</th><td><xsl:value-of select="*[local-name()='icy2-track-artist']"/></td></tr>
                        </xsl:if>
                        <xsl:if test="*[local-name()='icy2-track-title']">
                            <tr><th>Track</th><td><xsl:value-of select="*[local-name()='icy2-track-title']"/></td></tr>
                        </xsl:if>
                        <xsl:if test="*[local-name()='icy2-track-artwork']">
                            <tr><th>Artwork</th><td><a href="{*[local-name()='icy2-track-artwork']}" target="_blank"><xsl:value-of select="*[local-name()='icy2-track-artwork']"/></a></td></tr>
                        </xsl:if>
                        <xsl:if test="*[local-name()='icy2-audio-codec']">
                            <tr><th>Codec</th><td><xsl:value-of select="*[local-name()='icy2-audio-codec']"/></td></tr>
                        </xsl:if>
                        <xsl:if test="*[local-name()='icy2-notice-board']">
                            <tr><th>Notice</th><td><xsl:value-of select="*[local-name()='icy2-notice-board']"/></td></tr>
                        </xsl:if>
                        <xsl:if test="*[local-name()='icy2-upcoming-show']">
                            <tr><th>Coming Up</th><td><xsl:value-of select="*[local-name()='icy2-upcoming-show']"/></td></tr>
                        </xsl:if>
                        <xsl:if test="*[local-name()='icy2-tip-url']">
                            <tr><th>Tip Jar</th><td><a href="{*[local-name()='icy2-tip-url']}" target="_blank" rel="noopener noreferrer"><xsl:value-of select="*[local-name()='icy2-tip-url']"/></a></td></tr>
                        </xsl:if>
                        <xsl:if test="*[local-name()='icy2-chat-url']">
                            <tr><th>Chat</th><td><a href="{*[local-name()='icy2-chat-url']}" target="_blank" rel="noopener noreferrer"><xsl:value-of select="*[local-name()='icy2-chat-url']"/></a></td></tr>
                        </xsl:if>
                        <xsl:if test="*[local-name()='icy2-social-twitter']">
                            <tr><th>Twitter</th><td><xsl:value-of select="*[local-name()='icy2-social-twitter']"/></td></tr>
                        </xsl:if>
                        <xsl:if test="*[local-name()='icy2-hashtags']">
                            <tr><th>Hashtags</th><td><xsl:value-of select="*[local-name()='icy2-hashtags']"/></td></tr>
                        </xsl:if>
                    </table>

                    <div style="margin-top: 1rem;">
                        <xsl:choose>
                            <xsl:when test="authenticator">
                                <a href="./auth.xsl" class="btn btn-primary">Login to Listen</a>
                            </xsl:when>
                            <xsl:otherwise>
                                <a href="javascript:void(0);" onclick="window.open('/webplayer.xsl?mount={@mount}', 'mcaster1player', 'width=960,height=880,resizable=yes,scrollbars=yes'); return false;" class="btn btn-primary" title="Open web player in new window - bookmark for quick access" style="margin-right: 0.5rem;">
                                    <i class="fas fa-play-circle"></i> Web Player
                                </a>
                                <a href=".{@mount}.m3u" class="btn btn-secondary">Listen (M3U)</a>
                                <a href=".{@mount}.xspf" class="btn btn-secondary">Listen (XSPF)</a>
                            </xsl:otherwise>
                        </xsl:choose>
                    </div>
                </div>
            </xsl:for-each>

            <!-- No Streams Message -->
            <xsl:if test="not(source)">
                <div class="mcaster-card text-center">
                    <h2>No Streams Available</h2>
                    <p>There are currently no streams or mount points configured. Check back later!</p>
                </div>
            </xsl:if>

        </div>
    </div>

    <xsl:call-template name="web-footer"/>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

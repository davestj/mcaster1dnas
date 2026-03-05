<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>
<xsl:include href="header.xsl"/>
<xsl:include href="footer.xsl"/>

<xsl:template match="/mcaster1stats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Admin Statistics - Mcaster1DNAS</title>

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
    <style>
        .admin-action-btn {
            display: inline-flex;
            align-items: center;
            gap: 0.5rem;
            padding: 0.5rem 1rem;
            margin: 0.25rem;
            background: var(--bg-white);
            border: 2px solid var(--border-medium);
            border-radius: var(--radius-md);
            color: var(--text-primary);
            text-decoration: none;
            font-size: 0.875rem;
            font-weight: 500;
            transition: all var(--transition-fast);
        }
        .admin-action-btn:hover {
            background: var(--gradient-brand);
            color: white;
            border-color: var(--mcaster-blue);
            transform: translateY(-2px);
        }
        .admin-action-btn.danger:hover {
            background: linear-gradient(135deg, #ef4444, #dc2626);
            border-color: #dc2626;
        }
    </style>
</head>
<body>
    <xsl:call-template name="admin-header">
        <xsl:with-param name="active-page" select="'stats'"/>
        <xsl:with-param name="title" select="'Server Statistics - Mcaster1DNAS'"/>
    </xsl:call-template>

    <div class="mcaster-main">
        <div class="mcaster-container">

            <!-- Global Server Stats -->
            <div class="mcaster-card">
                <h2>
                    <i class="fas fa-server"></i> Global Server Statistics
                    <span class="tooltip">
                        <span class="help-icon">?</span>
                        <span class="tooltiptext">
                            Real-time server metrics showing current activity. These numbers update automatically as listeners connect and disconnect from your streams.
                        </span>
                    </span>
                </h2>

                <div class="stat-grid">
                    <div class="stat-box">
                        <span class="stat-value"><xsl:value-of select="listeners"/></span>
                        <span class="stat-label">
                            <i class="fas fa-headphones"></i> Listeners
                            <span class="tooltip">
                                <span class="help-icon">?</span>
                                <span class="tooltiptext">
                                    Total number of active listeners currently connected and streaming audio from all mount points.
                                </span>
                            </span>
                        </span>
                    </div>
                    <div class="stat-box">
                        <span class="stat-value"><xsl:value-of select="sources"/></span>
                        <span class="stat-label">
                            <i class="fas fa-microphone"></i> Sources
                            <span class="tooltip">
                                <span class="help-icon">?</span>
                                <span class="tooltiptext">
                                    Number of broadcast sources currently streaming to the server (e.g., OBS, SAM Broadcaster, Mixxx, etc.).
                                </span>
                            </span>
                        </span>
                    </div>
                    <div class="stat-box">
                        <span class="stat-value"><xsl:value-of select="clients"/></span>
                        <span class="stat-label">
                            <i class="fas fa-users"></i> Clients
                            <span class="tooltip">
                                <span class="help-icon">?</span>
                                <span class="tooltiptext">
                                    Total connected clients including listeners, sources, and web interface users.
                                </span>
                            </span>
                        </span>
                    </div>
                    <div class="stat-box">
                        <span class="stat-value"><xsl:value-of select="outgoing_kbitrate"/> kbps</span>
                        <span class="stat-label">
                            <i class="fas fa-tachometer-alt"></i> Bandwidth
                            <span class="tooltip">
                                <span class="help-icon">?</span>
                                <span class="tooltiptext">
                                    Current outgoing bandwidth in kilobits per second. This is the total data being streamed to all listeners.
                                </span>
                            </span>
                        </span>
                    </div>
                </div>

                <table style="margin-top: 1.5rem">
                    <tr>
                        <th style="width: 250px">Server ID</th>
                        <td><strong><xsl:value-of select="server_id"/></strong></td>
                    </tr>
                    <tr>
                        <th>Build</th>
                        <td class="font-mono"><xsl:value-of select="build"/></td>
                    </tr>
                    <tr>
                        <th>Server Start Time</th>
                        <td><xsl:value-of select="server_start"/></td>
                    </tr>
                    <tr>
                        <th>Location</th>
                        <td><xsl:value-of select="location"/></td>
                    </tr>
                    <tr>
                        <th>Admin Email</th>
                        <td><xsl:value-of select="admin"/></td>
                    </tr>
                    <tr>
                        <th>Total Connections</th>
                        <td><xsl:value-of select="connections"/></td>
                    </tr>
                    <tr>
                        <th>Client Connections</th>
                        <td><xsl:value-of select="client_connections"/></td>
                    </tr>
                    <tr>
                        <th>Source Connections</th>
                        <td><xsl:value-of select="source_client_connections"/></td>
                    </tr>
                    <tr>
                        <th>Data Sent</th>
                        <td><xsl:value-of select="stream_kbytes_sent"/> KB</td>
                    </tr>
                    <tr>
                        <th>Data Read</th>
                        <td><xsl:value-of select="stream_kbytes_read"/> KB</td>
                    </tr>
                </table>
            </div>

            <!-- Mount Point Stats -->
            <xsl:for-each select="source">
                <div class="mcaster-card">
                    <h2>
                        <i class="fas fa-stream"></i> Mount: <xsl:value-of select="@mount"/>
                        <xsl:if test="listeners">
                            <span class="status-badge status-active">
                                <i class="fas fa-circle"></i> LIVE - <xsl:value-of select="listeners"/> listening
                            </span>
                        </xsl:if>
                    </h2>

                    <!-- Admin Actions -->
                    <div style="margin: 1rem 0; padding: 1rem; background: var(--bg-light); border-radius: var(--radius-md);">
                        <strong style="display: block; margin-bottom: 0.5rem;">Admin Actions:</strong>
                        <a href="listclients.xsl?mount={@mount}" class="admin-action-btn">
                            <i class="fas fa-users"></i> List Clients
                        </a>
                        <a href="moveclients.xsl?mount={@mount}" class="admin-action-btn">
                            <i class="fas fa-exchange-alt"></i> Move Listeners
                        </a>
                        <a href="updatemetadata.xsl?mount={@mount}" class="admin-action-btn">
                            <i class="fas fa-edit"></i> Update Metadata
                        </a>
                        <xsl:if test="authenticator">
                            <a href="manageauth.xsl?mount={@mount}" class="admin-action-btn">
                                <i class="fas fa-key"></i> Manage Auth
                            </a>
                        </xsl:if>
                        <a href="javascript:void(0);" onclick="window.open('webplayer.xsl?mount={@mount}', 'mcaster1player', 'width=650,height=800,resizable=yes,scrollbars=yes'); return false;" class="admin-action-btn" title="Open web player in new window - bookmark for quick access">
                            <i class="fas fa-play-circle"></i> Player
                        </a>
                        <a href="killsource.xsl?mount={@mount}" class="admin-action-btn danger">
                            <i class="fas fa-stop-circle"></i> Kill Source
                        </a>
                    </div>

                    <!-- Stream Information (Always Visible) -->
                    <div style="margin: 1.5rem 0;">
                        <h3 style="margin-bottom: 1rem;"><i class="fas fa-music"></i> Now Playing</h3>
                        <div style="background: var(--bg-light); padding: 1.5rem; border-radius: var(--radius-md); border-left: 4px solid var(--mcaster-blue);">
                            <div style="font-size: 1.25rem; font-weight: 600; color: var(--text-primary); margin-bottom: 0.5rem;">
                                <xsl:choose>
                                    <xsl:when test="title">
                                        <xsl:value-of select="title"/>
                                    </xsl:when>
                                    <xsl:otherwise>
                                        <span style="color: var(--text-secondary); font-style: italic;">No metadata available</span>
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

                    <!-- Stream Technical Details -->
                    <div style="margin: 1.5rem 0;">
                        <h3 style="margin-bottom: 1rem;"><i class="fas fa-cog"></i> Stream Technical Details</h3>
                        <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 1rem;">
                            <!-- Format/Codec -->
                            <div style="background: white; padding: 1rem; border-radius: var(--radius-md); border: 1px solid var(--border-light);">
                                <div style="font-size: 0.75rem; color: var(--text-secondary); text-transform: uppercase; margin-bottom: 0.25rem;">Format/Codec</div>
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
                                <div style="font-size: 0.75rem; color: var(--text-secondary); text-transform: uppercase; margin-bottom: 0.25rem;">Bitrate</div>
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

                    <!-- Mount Stats Table -->
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
                                <th>Current Song</th>
                                <td><strong><xsl:if test="artist"><xsl:value-of select="artist"/> - </xsl:if><xsl:value-of select="title"/></strong></td>
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
                        <xsl:if test="listeners">
                            <tr>
                                <th>Current Listeners</th>
                                <td><xsl:value-of select="listeners"/></td>
                            </tr>
                        </xsl:if>
                        <xsl:if test="listener_peak">
                            <tr>
                                <th>Peak Listeners</th>
                                <td><xsl:value-of select="listener_peak"/></td>
                            </tr>
                        </xsl:if>
                        <xsl:if test="genre">
                            <tr>
                                <th>Genre</th>
                                <td><xsl:value-of select="genre"/></td>
                            </tr>
                        </xsl:if>
                        <xsl:if test="stream_start">
                            <tr>
                                <th>Stream Started</th>
                                <td><xsl:value-of select="stream_start"/></td>
                            </tr>
                        </xsl:if>
                        <xsl:if test="total_bytes_read">
                            <tr>
                                <th>Data Read</th>
                                <td><xsl:value-of select="total_bytes_read"/> bytes</td>
                            </tr>
                        </xsl:if>
                        <xsl:if test="total_bytes_sent">
                            <tr>
                                <th>Data Sent</th>
                                <td><xsl:value-of select="total_bytes_sent"/> bytes</td>
                            </tr>
                        </xsl:if>
                    </table>

                    <!-- Playlist Links -->
                    <div style="margin-top: 1rem;">
                        <xsl:choose>
                            <xsl:when test="authenticator">
                                <a href="../auth.xsl" class="btn btn-primary"><i class="fas fa-lock"></i> Login Required</a>
                            </xsl:when>
                            <xsl:otherwise>
                                <a href="{@mount}.m3u" class="btn btn-primary"><i class="fas fa-play"></i> M3U Playlist</a>
                                <a href="{@mount}.xspf" class="btn btn-secondary"><i class="fas fa-list-ul"></i> XSPF Playlist</a>
                            </xsl:otherwise>
                        </xsl:choose>
                    </div>
                </div>
            </xsl:for-each>

            <!-- No Active Sources Message -->
            <xsl:if test="not(source)">
                <div class="mcaster-card text-center">
                    <h2><i class="fas fa-info-circle"></i> No Active Sources</h2>
                    <p>There are currently no active broadcast sources connected to the server.</p>
                </div>
            </xsl:if>

        </div>
    </div>

    <xsl:call-template name="admin-footer"/>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

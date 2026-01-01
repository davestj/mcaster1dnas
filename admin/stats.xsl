<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>

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
    <div class="mcaster-header">
        <div class="mcaster-container">
            <div class="mcaster-header-top">
                <div class="mcaster-brand">
                    <div class="brand-icon"><i class="fas fa-broadcast-tower"></i></div>
                    <div class="brand-text">
                        <h1 style="margin: 0; font-size: 1.75rem;">
                            <span class="brand-mcaster">Mcaster1</span>
                            <span class="brand-dnas">DNAS Admin</span>
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
                <a href="stats.xsl" class="active"><i class="fas fa-chart-line"></i> Stats</a>
                <a href="listmounts.xsl"><i class="fas fa-stream"></i> Mounts</a>
                <a href="managerelays.xsl"><i class="fas fa-project-diagram"></i> Relays</a>
                <a href="logs.xsl"><i class="fas fa-file-alt"></i> Logs</a>
                <a href="credits.xsl"><i class="fas fa-info-circle"></i> Credits</a>
                <a href="../status.xsl" target="_blank"><i class="fas fa-globe"></i> Public</a>
            </div>
        </div>
    </div>

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
                        <a href="killsource.xsl?mount={@mount}" class="admin-action-btn danger">
                            <i class="fas fa-stop-circle"></i> Kill Source
                        </a>
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

    <div class="mcaster-footer">
        <div class="mcaster-container">
            <p>
                <i class="fas fa-server"></i> Powered by <a href="https://mcaster1.com">Mcaster1DNAS</a> - Digital Network Audio Server
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

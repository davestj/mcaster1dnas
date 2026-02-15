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
                                <a href=".{@mount}.m3u" class="btn btn-primary">Listen (M3U)</a>
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

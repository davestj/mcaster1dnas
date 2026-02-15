<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>

<xsl:template match="/mcaster1stats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Mount Points - Mcaster1DNAS Admin</title>

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
                <a href="stats.xsl"><i class="fas fa-chart-line"></i> Stats</a>
                <a href="listmounts.xsl" class="active"><i class="fas fa-stream"></i> Mounts</a>
                <a href="managerelays.xsl"><i class="fas fa-project-diagram"></i> Relays</a>
                <a href="logs.xsl"><i class="fas fa-file-alt"></i> Logs</a>
                <a href="credits.xsl"><i class="fas fa-info-circle"></i> Credits</a>
                <a href="../status.xsl" target="_blank"><i class="fas fa-globe"></i> Public</a>
            </div>
        </div>
    </div>

    <div class="mcaster-main">
        <div class="mcaster-container">

            <!-- Mount Points Explanation -->
            <div class="info-box">
                <div class="info-box-title">
                    <i class="fas fa-lightbulb"></i> Understanding Mount Points
                </div>
                <div class="info-box-content">
                    <strong>What are Mount Points?</strong> Each mount point is a unique URL endpoint where a broadcast stream is available. Think of them as individual "radio stations" on your server.
                    <br/><br/>
                    <strong>Common Uses:</strong>
                    <ul style="margin: 0.5rem 0 0 1.5rem;">
                        <li>Multiple quality levels (e.g., /live-high.mp3, /live-low.mp3)</li>
                        <li>Different audio formats (e.g., /stream.mp3, /stream.aac, /stream.ogg)</li>
                        <li>Separate shows or channels (e.g., /jazz, /rock, /talk)</li>
                    </ul>
                </div>
            </div>

            <xsl:for-each select="source">
                <div class="mcaster-card">
                    <h2>
                        <i class="fas fa-stream"></i> Mount: <xsl:value-of select="@mount"/>
                        <xsl:if test="listeners">
                            <span class="status-badge status-active">
                                <i class="fas fa-circle"></i> <xsl:value-of select="listeners"/> listening
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
                        <a href="killsource.xsl?mount={@mount}" class="admin-action-btn danger">
                            <i class="fas fa-stop-circle"></i> Kill Source
                        </a>
                        <xsl:if test="authenticator">
                            <a href="manageauth.xsl?mount={@mount}" class="admin-action-btn">
                                <i class="fas fa-key"></i> Manage Auth
                            </a>
                        </xsl:if>
                    </div>

                    <!-- Listener List -->
                    <xsl:choose>
                        <xsl:when test="listeners &gt; 0">
                            <h3><i class="fas fa-headphones"></i> Connected Listeners (<xsl:value-of select="listeners"/>)</h3>
                            <table>
                                <thead>
                                    <tr>
                                        <th>IP Address</th>
                                        <th>Connected (seconds)</th>
                                        <th>Lag (bytes)</th>
                                        <th>User Agent</th>
                                        <th style="text-align: center;">Action</th>
                                    </tr>
                                </thead>
                                <tbody>
                                    <xsl:variable name="themount"><xsl:value-of select="@mount"/></xsl:variable>
                                    <xsl:for-each select="listener">
                                        <tr>
                                            <td>
                                                <xsl:value-of select="IP"/>
                                                <xsl:if test="username">
                                                    <span style="color: var(--text-secondary); font-size: 0.875rem;">
                                                        (<xsl:value-of select="username"/>)
                                                    </span>
                                                </xsl:if>
                                            </td>
                                            <td><xsl:value-of select="Connected"/></td>
                                            <td><xsl:value-of select="lag"/></td>
                                            <td style="font-family: var(--font-mono); font-size: 0.875rem;">
                                                <xsl:value-of select="UserAgent"/>
                                            </td>
                                            <td style="text-align: center;">
                                                <a href="killclient.xsl?mount={$themount}&amp;id={@id}" class="btn btn-secondary" style="font-size: 0.75rem; padding: 0.375rem 0.75rem;">
                                                    <i class="fas fa-user-times"></i> Kick
                                                </a>
                                            </td>
                                        </tr>
                                    </xsl:for-each>
                                </tbody>
                            </table>
                        </xsl:when>
                        <xsl:otherwise>
                            <p style="text-align: center; padding: 2rem; color: var(--text-secondary);">
                                <i class="fas fa-info-circle"></i> No listeners currently connected to this mount.
                            </p>
                        </xsl:otherwise>
                    </xsl:choose>

                    <!-- Playlist Links -->
                    <div style="margin-top: 1rem;">
                        <xsl:choose>
                            <xsl:when test="authenticator">
                                <a href="../auth.xsl" class="btn btn-primary">
                                    <i class="fas fa-lock"></i> Login Required
                                </a>
                            </xsl:when>
                            <xsl:otherwise>
                                <a href="{@mount}.m3u" class="btn btn-primary">
                                    <i class="fas fa-play"></i> M3U Playlist
                                </a>
                                <a href="{@mount}.xspf" class="btn btn-secondary">
                                    <i class="fas fa-list-ul"></i> XSPF Playlist
                                </a>
                            </xsl:otherwise>
                        </xsl:choose>
                    </div>
                </div>
            </xsl:for-each>

            <!-- No Active Sources Message -->
            <xsl:if test="not(source)">
                <div class="mcaster-card text-center">
                    <h2><i class="fas fa-info-circle"></i> No Active Mount Points</h2>
                    <p>There are currently no active broadcast sources connected to the server.</p>
                </div>
            </xsl:if>

        </div>
    </div>

    <div class="mcaster-footer">
        <div class="mcaster-container">
            <p><i class="fas fa-server"></i> Powered by <a href="https://mcaster1.com">Mcaster1DNAS</a> - Digital Network Audio Server
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

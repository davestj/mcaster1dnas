<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>

<xsl:template match="/mcaster1stats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Update Metadata - Mcaster1DNAS Admin</title>

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
            <div class="mcaster-nav">
                <a href="stats.xsl"><i class="fas fa-chart-line"></i> Stats</a>
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

            <!-- What is Metadata? -->
            <div class="info-box">
                <div class="info-box-title">
                    <i class="fas fa-question-circle"></i> What is Stream Metadata?
                </div>
                <div class="info-box-content">
                    Metadata is the "Now Playing" information displayed in listeners' media players. When you update metadata, it shows the current song title, artist, or show information to everyone tuned in to your stream. This is especially useful for pre-recorded content or DJ mixes where track information isn't automatically detected.
                </div>
            </div>

            <xsl:for-each select="source">
                <div class="mcaster-card">
                    <h2>
                        <i class="fas fa-edit"></i> Update Metadata
                        <xsl:if test="server_name">
                            - <xsl:value-of select="server_name"/>
                        </xsl:if>
                        <span class="tooltip">
                            <span class="help-icon">?</span>
                            <span class="tooltiptext">
                                Manually update the "Now Playing" information that appears in your listeners' media players
                            </span>
                        </span>
                    </h2>
                    <p style="color: var(--text-secondary); margin-bottom: 1.5rem;">
                        Mount Point: <strong><xsl:value-of select="@mount"/></strong>
                    </p>

                    <form method="GET" action="/admin/metadata.xsl">
                        <div style="margin-bottom: 1.5rem;">
                            <label style="display: block; margin-bottom: 0.5rem; font-weight: 600; color: var(--text-primary);">
                                <i class="fas fa-music"></i> Song/Metadata Information
                                <span class="tooltip">
                                    <span class="help-icon">?</span>
                                    <span class="tooltiptext">
                                        Enter song details in format: "Artist - Title" or "Show Name - Episode Title". This will appear in listeners' players immediately.
                                    </span>
                                </span>
                            </label>
                            <input type="text" name="song" placeholder="Artist - Title" style="width: 100%; max-width: 600px;"/>
                            <p style="font-size: 0.875rem; color: var(--text-secondary); margin-top: 0.5rem;">
                                <strong>Examples:</strong> "The Beatles - Hey Jude" or "Morning Show - Episode 42"
                            </p>
                        </div>

                        <input type="hidden" name="mount" value="{@mount}"/>
                        <input type="hidden" name="mode" value="updinfo"/>
                        <input type="hidden" name="charset" value="UTF-8"/>

                        <button type="submit" class="btn btn-primary" style="padding: 0.75rem 2rem;">
                            <i class="fas fa-save"></i> Update Metadata
                        </button>
                    </form>
                </div>
            </xsl:for-each>

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

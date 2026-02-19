<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>
<xsl:include href="header.xsl"/>
<xsl:include href="footer.xsl"/>

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
    <link rel="icon" type="image/x-icon" href="/favicon.ico"/>
    <link rel="icon" type="image/png" sizes="32x32" href="/favicon-32x32.png"/>
    <link rel="icon" type="image/png" sizes="16x16" href="/favicon-16x16.png"/>
    <link rel="apple-touch-icon" sizes="180x180" href="/apple-touch-icon.png"/>
    <script src="/mcaster-utils.js"></script>
</head>
<body>
    <xsl:call-template name="admin-header">
        <xsl:with-param name="active-page" select="''"/>
    </xsl:call-template>

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

    <xsl:call-template name="admin-footer"/>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

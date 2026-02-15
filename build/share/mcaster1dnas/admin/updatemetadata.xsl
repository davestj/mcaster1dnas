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

    <link rel="stylesheet" type="text/css" href="mcaster1-modern.css"/>
</head>
<body>
    <div class="mcaster-header">
        <div class="mcaster-container">
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
                <a href="stats.xsl"><i class="fas fa-chart-line"></i> Statistics</a>
                <a href="listmounts.xsl"><i class="fas fa-list"></i> Mounts</a>
                <a href="logs.xsl"><i class="fas fa-file-alt"></i> Logs</a>
                <a href="../status.xsl" target="_blank"><i class="fas fa-globe"></i> Public Page</a>
            </div>
        </div>
    </div>

    <div class="mcaster-main">
        <div class="mcaster-container">

            <xsl:for-each select="source">
                <div class="mcaster-card">
                    <h2>
                        <i class="fas fa-edit"></i> Update Metadata
                        <xsl:if test="server_name">
                            - <xsl:value-of select="server_name"/>
                        </xsl:if>
                    </h2>
                    <p style="color: var(--text-secondary); margin-bottom: 1.5rem;">
                        Mount Point: <strong><xsl:value-of select="@mount"/></strong>
                    </p>

                    <form method="GET" action="/admin/metadata.xsl">
                        <div style="margin-bottom: 1.5rem;">
                            <label style="display: block; margin-bottom: 0.5rem; font-weight: 600; color: var(--text-primary);">
                                <i class="fas fa-music"></i> Song/Metadata Information
                            </label>
                            <input type="text" name="song" placeholder="Artist - Title" style="width: 100%; max-width: 600px;"/>
                            <p style="font-size: 0.875rem; color: var(--text-secondary); margin-top: 0.5rem;">
                                Enter the current song information in the format: Artist - Title
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
            <p><i class="fas fa-server"></i> Powered by <a href="https://mcaster1.com">Mcaster1DNAS</a> - Digital Network Audio Server</p>
        </div>
    </div>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

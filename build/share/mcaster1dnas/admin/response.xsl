<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>

<xsl:template match="/iceresponse">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Server Response - Mcaster1DNAS Admin</title>

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

            <div class="mcaster-card">
                <h2><i class="fas fa-comment-dots"></i> Server Response</h2>

                <xsl:for-each select="/iceresponse">
                    <div style="background: var(--bg-light); padding: 1.5rem; border-radius: var(--radius-md); margin: 1rem 0;">
                        <div style="margin-bottom: 1rem;">
                            <strong style="color: var(--text-secondary); display: block; margin-bottom: 0.5rem;">
                                <i class="fas fa-info-circle"></i> Message:
                            </strong>
                            <p style="font-size: 1.125rem; margin: 0; color: var(--text-primary);">
                                <xsl:value-of select="message"/>
                            </p>
                        </div>
                        <div>
                            <strong style="color: var(--text-secondary); display: block; margin-bottom: 0.5rem;">
                                <i class="fas fa-code"></i> Return Code:
                            </strong>
                            <p style="font-family: var(--font-mono); font-size: 1rem; margin: 0;">
                                <xsl:value-of select="return"/>
                            </p>
                        </div>
                    </div>
                </xsl:for-each>

                <div style="margin-top: 1.5rem;">
                    <a href="stats.xsl" class="btn btn-primary">
                        <i class="fas fa-arrow-left"></i> Back to Statistics
                    </a>
                </div>
            </div>

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

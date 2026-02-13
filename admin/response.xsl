<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>
<xsl:include href="header.xsl"/>
<xsl:include href="footer.xsl"/>

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

    <xsl:call-template name="admin-footer"/>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

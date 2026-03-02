<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>
<xsl:include href="header.xsl"/>
<xsl:include href="footer.xsl"/>

<xsl:template match="/mcaster1stats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Move Listeners - Mcaster1DNAS Admin</title>

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

            <xsl:variable name="currentmount"><xsl:value-of select="current_source"/></xsl:variable>

            <div class="mcaster-card">
                <h2><i class="fas fa-exchange-alt"></i> Move Listeners from <xsl:copy-of select="$currentmount"/></h2>
                <p style="color: var(--text-secondary); margin-bottom: 1.5rem;">
                    Select a destination mount point to move all listeners from the current mount.
                </p>

                <xsl:for-each select="source">
                    <div style="margin-bottom: 1rem; padding: 1rem; background: var(--bg-light); border-radius: var(--radius-md); display: flex; align-items: center; justify-content: space-between;">
                        <div>
                            <strong style="font-size: 1.125rem;">
                                <i class="fas fa-stream"></i> <xsl:value-of select="@mount"/>
                            </strong>
                            <span style="margin-left: 1rem; color: var(--text-secondary);">
                                <i class="fas fa-headphones"></i> <xsl:value-of select="listeners"/> listener(s)
                            </span>
                        </div>
                        <a href="moveclients.xsl?mount={$currentmount}&amp;destination={@mount}" class="btn btn-primary">
                            <i class="fas fa-arrow-right"></i> Move Clients Here
                        </a>
                    </div>
                </xsl:for-each>

                <xsl:if test="not(source)">
                    <p style="text-align: center; padding: 2rem; color: var(--text-secondary);">
                        <i class="fas fa-info-circle"></i> No destination mount points available.
                    </p>
                </xsl:if>
            </div>

        </div>
    </div>

    <xsl:call-template name="admin-footer"/>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

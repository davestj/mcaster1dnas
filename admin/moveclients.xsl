<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>

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

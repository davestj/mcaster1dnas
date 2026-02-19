<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!-- Shared Admin Header Template
     Usage: <xsl:call-template name="admin-header">
                <xsl:with-param name="active-page" select="'stats'"/>
                <xsl:with-param name="title" select="'Page Title - Mcaster1DNAS'"/>
            </xsl:call-template>
     active-page values: stats, mounts, relays, logs, credits, songdata
-->
<xsl:template name="admin-header">
    <xsl:param name="active-page" select="'none'"/>
    <xsl:param name="title" select="'Admin - Mcaster1DNAS'"/>

    <div class="mcaster-header" role="banner">
        <div class="mcaster-container">

            <!-- Left group: brand icon + title + nav links all in one row -->
            <div class="header-left">
                <div class="mcaster-brand">
                    <div class="brand-icon" aria-hidden="true"><i class="fas fa-broadcast-tower"></i></div>
                    <div class="brand-text">
                        <h1><span class="brand-mcaster">Mcaster1</span><span class="brand-dnas">DNAS Admin</span></h1>
                    </div>
                </div>

                <nav class="mcaster-nav" aria-label="Admin navigation">
                    <a href="stats.xsl">
                        <xsl:if test="$active-page = 'stats'">
                            <xsl:attribute name="class">active</xsl:attribute>
                        </xsl:if>
                        <i class="fas fa-chart-line" aria-hidden="true"></i> Stats
                    </a>
                    <a href="listmounts.xsl">
                        <xsl:if test="$active-page = 'mounts'">
                            <xsl:attribute name="class">active</xsl:attribute>
                        </xsl:if>
                        <i class="fas fa-stream" aria-hidden="true"></i> Mounts
                    </a>
                    <a href="managerelays.xsl">
                        <xsl:if test="$active-page = 'relays'">
                            <xsl:attribute name="class">active</xsl:attribute>
                        </xsl:if>
                        <i class="fas fa-project-diagram" aria-hidden="true"></i> Relays
                    </a>
                    <a href="logs.xsl">
                        <xsl:if test="$active-page = 'logs'">
                            <xsl:attribute name="class">active</xsl:attribute>
                        </xsl:if>
                        <i class="fas fa-file-alt" aria-hidden="true"></i> Logs
                    </a>
                    <a href="songdata.xsl">
                        <xsl:if test="$active-page = 'songdata'">
                            <xsl:attribute name="class">active</xsl:attribute>
                        </xsl:if>
                        <i class="fas fa-history" aria-hidden="true"></i> Track History
                    </a>
                    <a href="credits.xsl">
                        <xsl:if test="$active-page = 'credits'">
                            <xsl:attribute name="class">active</xsl:attribute>
                        </xsl:if>
                        <i class="fas fa-info-circle" aria-hidden="true"></i> Credits
                    </a>
                    <a href="../status.xsl" target="_blank" rel="noopener">
                        <i class="fas fa-globe" aria-hidden="true"></i> Public
                    </a>
                </nav>
            </div>

            <!-- Right: compact clock -->
            <div class="mcaster-clock" aria-label="Current server time" aria-live="polite">
                <div class="mcaster-clock-time">
                    <i class="fas fa-clock mcaster-clock-icon" aria-hidden="true"></i>
                    <span id="live-time">--:--:--</span>
                </div>
                <div class="mcaster-clock-date" id="live-date" aria-label="Current date"></div>
            </div>

        </div>
    </div>
</xsl:template>

</xsl:stylesheet>

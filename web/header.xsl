<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!-- Shared Public Web Header Template
     Usage: <xsl:call-template name="web-header">
                <xsl:with-param name="active-page" select="'status'"/>
                <xsl:with-param name="title" select="'Page Title - Mcaster1DNAS'"/>
            </xsl:call-template>
     active-page values: status, version, credits, songdata
-->
<xsl:template name="web-header">
    <xsl:param name="active-page" select="'none'"/>
    <xsl:param name="title" select="'Mcaster1DNAS'"/>

    <div class="mcaster-header" role="banner">
        <div class="mcaster-container">

            <!-- Left group: brand icon + title + nav links all in one row -->
            <div class="header-left">
                <div class="mcaster-brand">
                    <div class="brand-icon" aria-hidden="true"><i class="fas fa-broadcast-tower"></i></div>
                    <div class="brand-text">
                        <h1><span class="brand-mcaster">Mcaster1</span><span class="brand-dnas">DNAS</span></h1>
                    </div>
                </div>

                <nav class="mcaster-nav" aria-label="Site navigation">
                    <a href="/status.xsl">
                        <xsl:if test="$active-page = 'status'">
                            <xsl:attribute name="class">active</xsl:attribute>
                        </xsl:if>
                        <i class="fas fa-home" aria-hidden="true"></i> Status
                    </a>
                    <a href="/server_version.xsl">
                        <xsl:if test="$active-page = 'version'">
                            <xsl:attribute name="class">active</xsl:attribute>
                        </xsl:if>
                        <i class="fas fa-info-circle" aria-hidden="true"></i> Server Info
                    </a>
                    <a href="/songdata.xsl">
                        <xsl:if test="$active-page = 'songdata'">
                            <xsl:attribute name="class">active</xsl:attribute>
                        </xsl:if>
                        <i class="fas fa-history" aria-hidden="true"></i> Track History
                    </a>
                    <a href="/credits.xsl">
                        <xsl:if test="$active-page = 'credits'">
                            <xsl:attribute name="class">active</xsl:attribute>
                        </xsl:if>
                        <i class="fas fa-award" aria-hidden="true"></i> Credits
                    </a>
                    <a href="/admin/stats.xsl">
                        <i class="fas fa-shield-alt" aria-hidden="true"></i> Admin
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

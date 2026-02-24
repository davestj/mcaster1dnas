<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!-- Shared Admin Footer Toolbar Template
     Usage: <xsl:call-template name="admin-footer"/>
-->
<xsl:template name="admin-footer">
    <div class="mcaster-footer" role="contentinfo" aria-label="Site footer">
        <div class="mcaster-container">
            <div class="footer-toolbar">
                <span class="footer-brand">
                    <i class="fas fa-broadcast-tower" aria-hidden="true"></i>
                    <strong>Mcaster1DNAS</strong>
                </span>
                <span class="footer-links" role="navigation" aria-label="Footer navigation">
                    <a href="stats.xsl"><i class="fas fa-chart-line" aria-hidden="true"></i> <strong>Stats</strong></a>
                    <a href="listmounts.xsl"><i class="fas fa-stream" aria-hidden="true"></i> Mounts</a>
                    <a href="songdata.xsl"><i class="fas fa-history" aria-hidden="true"></i> Track History</a>
                    <a href="logs.xsl"><i class="fas fa-file-alt" aria-hidden="true"></i> Logs</a>
                    <a href="../status.xsl" target="_blank" rel="noopener"><i class="fas fa-globe" aria-hidden="true"></i> Public</a>
                </span>
                <span class="footer-status">
                    <xsl:choose>
                        <xsl:when test="number(sources) &gt; 0">
                            <i class="fas fa-circle footer-online-dot" aria-hidden="true"></i>
                            <strong>Online</strong>
                            <span class="footer-metrics" aria-label="Server statistics">
                                <span class="footer-metric">
                                    <strong><xsl:value-of select="sources"/></strong>
                                    <span class="metric-label">mts</span>
                                    <i class="fas fa-info-circle footer-metric-info" aria-hidden="true"
                                       title="Active source mounts: number of live stream sources currently connected and broadcasting"></i>
                                </span>
                                <span class="footer-metric-sep" aria-hidden="true">&#183;</span>
                                <span class="footer-metric">
                                    <strong><xsl:value-of select="listeners"/></strong>
                                    <span class="metric-label">lst</span>
                                    <i class="fas fa-info-circle footer-metric-info" aria-hidden="true"
                                       title="Total listeners: number of people currently tuned in across all streams"></i>
                                </span>
                                <span class="footer-metric-sep" aria-hidden="true">&#183;</span>
                                <span class="footer-metric">
                                    <strong><xsl:value-of select="max_listeners"/></strong>
                                    <span class="metric-label">max</span>
                                    <i class="fas fa-info-circle footer-metric-info" aria-hidden="true"
                                       title="Max listeners: server-configured maximum concurrent listener capacity from config"></i>
                                </span>
                                <span class="footer-metric-sep" aria-hidden="true">&#183;</span>
                                <span class="footer-metric">
                                    <strong><xsl:value-of select="sum(source/listener_peak)"/></strong>
                                    <span class="metric-label">peak</span>
                                    <i class="fas fa-info-circle footer-metric-info" aria-hidden="true"
                                       title="Peak listeners: highest concurrent listener count reached across all mounts since server start"></i>
                                </span>
                            </span>
                        </xsl:when>
                        <xsl:otherwise>
                            <i class="fas fa-circle footer-offline-dot" aria-hidden="true"></i>
                            <strong>Offline</strong>
                            <span class="footer-offline-msg" title="No source mounts are currently connected or broadcasting">
                                <i class="fas fa-info-circle" aria-hidden="true"></i>
                                No live streams connected
                            </span>
                        </xsl:otherwise>
                    </xsl:choose>
                    <span class="page-load-time" id="page-load-time" aria-live="polite">
                        <i class="fas fa-spinner fa-spin" aria-hidden="true"></i> Loading...
                    </span>
                </span>
            </div>
        </div>
    </div>
</xsl:template>

</xsl:stylesheet>

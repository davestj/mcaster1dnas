<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<!-- Shared Public Web Footer Toolbar Template
     Usage: <xsl:call-template name="web-footer"/>
     Features: Fixed bottom bar with nav links + stats popup modal
-->
<xsl:template name="web-footer">
    <div class="mcaster-footer" role="contentinfo" aria-label="Site footer">
        <div class="mcaster-container">
            <div class="footer-toolbar">
                <span class="footer-brand">
                    <i class="fas fa-broadcast-tower" aria-hidden="true"></i>
                    <strong>Mcaster1DNAS</strong>
                </span>
                <span class="footer-links" role="navigation" aria-label="Footer navigation">
                    <a href="/status.xsl"><i class="fas fa-home" aria-hidden="true"></i> Status</a>
                    <a href="/songdata.xsl"><i class="fas fa-history" aria-hidden="true"></i> Track History</a>
                    <a href="/server_version.xsl"><i class="fas fa-info-circle" aria-hidden="true"></i> Server Info</a>
                    <a href="/credits.xsl"><i class="fas fa-award" aria-hidden="true"></i> Credits</a>
                    <a href="/admin/stats.xsl"><i class="fas fa-shield-alt" aria-hidden="true"></i> Admin</a>
                </span>
                <span class="footer-status">
                    <xsl:choose>
                        <xsl:when test="number(sources) &gt; 0">
                            <button class="footer-stats-btn" onclick="toggleStatsModal()" aria-label="Open server stats">
                                <i class="fas fa-circle footer-online-dot" aria-hidden="true" style="animation:pulse 2s infinite;font-size:0.5rem;"></i>
                                Online
                                <span class="btn-count"><xsl:value-of select="sources"/> mts</span>
                                <span class="btn-count"><xsl:value-of select="listeners"/> lst</span>
                            </button>
                        </xsl:when>
                        <xsl:otherwise>
                            <button class="footer-stats-btn" onclick="toggleStatsModal()" aria-label="Open server stats" style="background:#94a3b8;">
                                <i class="fas fa-circle" aria-hidden="true" style="font-size:0.5rem;"></i>
                                Offline
                            </button>
                        </xsl:otherwise>
                    </xsl:choose>
                    <span class="page-load-time" id="page-load-time" aria-live="polite">
                        <i class="fas fa-spinner fa-spin" aria-hidden="true"></i>
                    </span>
                </span>
            </div>
        </div>
    </div>

    <!-- Stats Popup Modal -->
    <div class="stats-modal-overlay" id="statsModal">
        <div class="stats-modal">
            <div class="stats-modal-header">
                <h3><i class="fas fa-chart-bar"></i> Server Stats</h3>
                <button class="stats-modal-close" onclick="toggleStatsModal()" aria-label="Close stats">&#215;</button>
            </div>
            <div class="stats-modal-body">
                <div class="stats-modal-grid">
                    <div class="stats-modal-item online">
                        <div class="stat-value"><xsl:value-of select="sources"/></div>
                        <div class="stat-label">Active Mounts</div>
                    </div>
                    <div class="stats-modal-item">
                        <div class="stat-value"><xsl:value-of select="listeners"/></div>
                        <div class="stat-label">Listeners</div>
                    </div>
                    <div class="stats-modal-item highlight">
                        <div class="stat-value"><xsl:value-of select="sum(source/listener_peak)"/></div>
                        <div class="stat-label">Peak Listeners</div>
                    </div>
                    <div class="stats-modal-item">
                        <div class="stat-value"><xsl:value-of select="max_listeners"/></div>
                        <div class="stat-label">Max Capacity</div>
                    </div>
                </div>
                <hr class="stats-modal-divider"/>
                <div class="stats-modal-row">
                    <span class="row-label">Server ID</span>
                    <span class="row-value"><xsl:value-of select="server_id"/></span>
                </div>
                <div class="stats-modal-row">
                    <span class="row-label">Host</span>
                    <span class="row-value"><xsl:value-of select="host"/></span>
                </div>
                <xsl:if test="number(sources) &gt; 0">
                <hr class="stats-modal-divider"/>
                <xsl:for-each select="source">
                    <div class="stats-modal-row">
                        <span class="row-label">
                            <i class="fas fa-music" style="color:var(--mcaster-teal);margin-right:0.3rem;" aria-hidden="true"></i>
                            <xsl:value-of select="@mount"/>
                        </span>
                        <span class="row-value">
                            <xsl:value-of select="listeners"/> / <xsl:value-of select="listener_peak"/> pk
                        </span>
                    </div>
                </xsl:for-each>
                </xsl:if>
            </div>
        </div>
    </div>

    <script><![CDATA[
    function toggleStatsModal() {
        var m = document.getElementById('statsModal');
        if (m) m.classList.toggle('active');
    }
    document.addEventListener('click', function(e) {
        var m = document.getElementById('statsModal');
        var btn = document.querySelector('.footer-stats-btn');
        if (m && m.classList.contains('active') &&
            !m.contains(e.target) && btn && !btn.contains(e.target)) {
            m.classList.remove('active');
        }
    });
    document.addEventListener('keydown', function(e) {
        if (e.key === 'Escape') {
            var m = document.getElementById('statsModal');
            if (m) m.classList.remove('active');
        }
    });
    ]]></script>
</xsl:template>

</xsl:stylesheet>

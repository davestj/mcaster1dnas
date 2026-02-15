<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>

<xsl:template match="/mcaster1stats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Credits &amp; Fork Information - Mcaster1DNAS Admin</title>

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
                <a href="stats.xsl"><i class="fas fa-chart-line"></i> Statistics</a>
                <a href="listmounts.xsl"><i class="fas fa-list"></i> Mounts</a>
                <a href="logs.xsl"><i class="fas fa-file-alt"></i> Logs</a>
                <a href="credits.xsl" class="active"><i class="fas fa-info-circle"></i> Credits</a>
                <a href="../status.xsl" target="_blank"><i class="fas fa-globe"></i> Public Page</a>
            </div>
        </div>
    </div>

    <div class="mcaster-main">
        <div class="mcaster-container">

            <!-- Header -->
            <div class="mcaster-card">
                <h1 style="margin-bottom: 0.5rem;">
                    <i class="fas fa-award"></i> Credits &amp; Fork Information
                </h1>
                <p style="color: var(--text-secondary); font-size: 1.125rem;">
                    Mcaster1DNAS stands on the shoulders of giants - acknowledging our open source heritage.
                </p>
            </div>

            <!-- Project Lineage -->
            <div class="credits-section">
                <h3><i class="fas fa-code-branch"></i> Project Lineage</h3>
                <p style="color: var(--text-secondary); margin-bottom: 1rem;">
                    Mcaster1DNAS is derived from Icecast-KH by Karl Heyes, which itself is a fork of Icecast2 by the Xiph.Org Foundation.
                </p>
                <div class="lineage-tree">
                    Icecast2 (Xiph.Org Foundation)<br/>
                    <span style="color: var(--mcaster-blue);">    ↓</span><br/>
                    Icecast-KH (Karl Heyes)<br/>
                    <span style="color: var(--dnas-green);">    ↓</span><br/>
                    <strong style="color: var(--mcaster-blue);">Mcaster1DNAS (Saint John / MediaCast1)</strong>
                </div>
            </div>

            <!-- Original Projects -->
            <div class="credits-section">
                <h3><i class="fas fa-book"></i> Original Projects</h3>

                <div class="project-info">
                    <h4><i class="fas fa-music"></i> Icecast2</h4>
                    <ul>
                        <li><strong>Copyright:</strong> Xiph.Org Foundation</li>
                        <li><strong>License:</strong> GNU GPL v2</li>
                        <li><strong>Repository:</strong> <a href="https://gitlab.xiph.org/xiph/icecast-server" target="_blank">gitlab.xiph.org/xiph/icecast-server</a></li>
                        <li><strong>Website:</strong> <a href="https://icecast.org" target="_blank">icecast.org</a></li>
                    </ul>
                </div>

                <div class="project-info">
                    <h4><i class="fas fa-code"></i> Icecast-KH</h4>
                    <ul>
                        <li><strong>Copyright:</strong> Karl Heyes</li>
                        <li><strong>License:</strong> GNU GPL v2</li>
                        <li><strong>Repository:</strong> <a href="https://github.com/karlheyes/icecast-kh" target="_blank">github.com/karlheyes/icecast-kh</a></li>
                        <li><strong>Maintainer:</strong> Karl Heyes</li>
                    </ul>
                </div>
            </div>

            <!-- Mcaster1DNAS Modifications -->
            <div class="credits-section">
                <h3><i class="fas fa-wrench"></i> Mcaster1DNAS Modifications</h3>
                <p style="color: var(--text-secondary); margin-bottom: 1.5rem;">
                    Mcaster1DNAS represents a complete rebrand and continued development based on Karl Heyes' excellent Icecast-KH improvements. We maintain all original credits and licensing while establishing a distinct identity for the MediaCast1 streaming platform.
                </p>

                <div class="info-box">
                    <div class="info-box-title">
                        <i class="fas fa-star"></i> Key Changes from Icecast-KH
                    </div>
                    <div class="info-box-content">
                        <ul style="margin-left: 1.5rem; margin-top: 0.5rem;">
                            <li>Complete rebrand to Mcaster1DNAS naming</li>
                            <li>Modern HTML5/CSS3 web interface with FontAwesome 6.x icons</li>
                            <li>Enhanced help tooltips and user guidance</li>
                            <li>HTTPS/SSL streaming by default (ports 9330 HTTP, 9443 SSL)</li>
                            <li>MediaCast1 ecosystem integration</li>
                            <li>Updated configuration defaults</li>
                            <li>Enhanced logging and monitoring</li>
                            <li>Modern build system improvements</li>
                        </ul>
                    </div>
                </div>

                <div class="success-box">
                    <div class="success-box-title">
                        <i class="fas fa-check-circle"></i> Major Features Inherited from Icecast-KH
                    </div>
                    <div style="margin-top: 0.5rem;">
                        <ul style="margin-left: 1.5rem;">
                            <li>Enhanced authentication system</li>
                            <li>Improved relay handling</li>
                            <li>Better memory management</li>
                            <li>Advanced source client support</li>
                            <li>Performance optimizations</li>
                        </ul>
                    </div>
                </div>
            </div>

            <!-- License Information -->
            <div class="credits-section">
                <h3><i class="fas fa-balance-scale"></i> License</h3>
                <p style="color: var(--text-primary);">
                    Mcaster1DNAS is licensed under <strong style="color: var(--mcaster-blue);">GNU GPL v2</strong>, maintaining compatibility with all upstream projects.
                </p>
            </div>

            <!-- Acknowledgments -->
            <div class="credits-section">
                <h3><i class="fas fa-heart"></i> Acknowledgments</h3>
                <p style="color: var(--text-secondary); margin-bottom: 1rem;">
                    We acknowledge and thank:
                </p>
                <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 1rem;">
                    <div style="background: var(--bg-light); padding: 1.25rem; border-radius: var(--radius-md);">
                        <h4 style="color: var(--mcaster-blue); margin-bottom: 0.5rem;">
                            <i class="fas fa-users"></i> Xiph.Org Foundation
                        </h4>
                        <p style="font-size: 0.9375rem; color: var(--text-secondary);">
                            Original Icecast2 development and the foundation of modern streaming audio servers
                        </p>
                    </div>
                    <div style="background: var(--bg-light); padding: 1.25rem; border-radius: var(--radius-md);">
                        <h4 style="color: var(--mcaster-blue); margin-bottom: 0.5rem;">
                            <i class="fas fa-user"></i> Karl Heyes
                        </h4>
                        <p style="font-size: 0.9375rem; color: var(--text-secondary);">
                            Icecast-KH improvements, maintenance, and excellent enhancements to the streaming server
                        </p>
                    </div>
                    <div style="background: var(--bg-light); padding: 1.25rem; border-radius: var(--radius-md);">
                        <h4 style="color: var(--mcaster-blue); margin-bottom: 0.5rem;">
                            <i class="fas fa-code"></i> All Contributors
                        </h4>
                        <p style="font-size: 0.9375rem; color: var(--text-secondary);">
                            All Icecast2 and Icecast-KH contributors for the foundational streaming server technology
                        </p>
                    </div>
                </div>
            </div>

            <!-- Contact Information -->
            <div class="credits-section">
                <h3><i class="fas fa-envelope"></i> Mcaster1DNAS Project Contact</h3>
                <div style="background: linear-gradient(135deg, var(--bg-light), var(--bg-accent)); padding: 1.5rem; border-radius: var(--radius-md);">
                    <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 1.5rem;">
                        <div>
                            <strong style="color: var(--text-secondary); display: block; margin-bottom: 0.375rem;">Maintainer</strong>
                            <p style="color: var(--text-primary); font-size: 1.0625rem;">Saint John (David St John)</p>
                        </div>
                        <div>
                            <strong style="color: var(--text-secondary); display: block; margin-bottom: 0.375rem;">Email</strong>
                            <p style="color: var(--text-primary); font-size: 1.0625rem;">
                                <a href="mailto:davestj@gmail.com">davestj@gmail.com</a>
                            </p>
                        </div>
                        <div>
                            <strong style="color: var(--text-secondary); display: block; margin-bottom: 0.375rem;">Website</strong>
                            <p style="color: var(--text-primary); font-size: 1.0625rem;">
                                <a href="https://mcaster1.com" target="_blank">mcaster1.com</a>
                            </p>
                        </div>
                        <div>
                            <strong style="color: var(--text-secondary); display: block; margin-bottom: 0.375rem;">Repository</strong>
                            <p style="color: var(--text-primary); font-size: 1.0625rem;">
                                <a href="https://github.com/davestj/mcaster1dnas" target="_blank">github.com/davestj/mcaster1dnas</a>
                            </p>
                        </div>
                    </div>
                </div>
            </div>

            <!-- Footer Note -->
            <div class="warning-box">
                <div class="warning-box-title">
                    <i class="fas fa-exclamation-triangle"></i> Important Note
                </div>
                <p style="margin: 0;">
                    This is a fork project. We respect and maintain all original licensing and credits. If you're looking for the original Icecast2 or Icecast-KH, please visit the repositories linked above.
                </p>
            </div>

        </div>
    </div>

    <div class="mcaster-footer">
        <div class="mcaster-container">
            <p><i class="fas fa-server"></i> Powered by <a href="https://mcaster1.com">Mcaster1DNAS</a> - Digital Network Audio Server | <a href="credits.xsl">Credits</a> | GNU GPL v2</p>
        </div>
    </div>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

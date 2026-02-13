<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>
<xsl:include href="header.xsl"/>
<xsl:include href="footer.xsl"/>

<xsl:template match="/mcaster1stats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Credits &amp; About - Mcaster1DNAS</title>

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
    <xsl:call-template name="web-header">
        <xsl:with-param name="active-page" select="'credits'"/>
        <xsl:with-param name="title" select="'Credits &amp; About - Mcaster1DNAS'"/>
    </xsl:call-template>

    <div class="mcaster-main">
        <div class="mcaster-container">

            <!-- Header -->
            <div class="mcaster-card">
                <h1 style="margin-bottom: 0.5rem;">
                    <i class="fas fa-award"></i> About Mcaster1DNAS
                </h1>
                <p style="color: var(--text-secondary); font-size: 1.125rem;">
                    Professional streaming server built on open source excellence
                </p>
            </div>

            <!-- What is Mcaster1DNAS -->
            <div class="credits-section">
                <h3><i class="fas fa-broadcast-tower"></i> What is Mcaster1DNAS?</h3>
                <p style="font-size: 1.0625rem; line-height: 1.8; color: var(--text-primary);">
                    Mcaster1DNAS (Digital Network Audio Server) is a powerful, enterprise-grade streaming media server designed for professional audio broadcasting. It supports multiple audio formats including MP3, AAC, Ogg Vorbis, Opus, and FLAC, providing high-quality streaming with low latency and excellent reliability.
                </p>
            </div>

            <!-- Project Heritage -->
            <div class="credits-section">
                <h3><i class="fas fa-code-branch"></i> Project Heritage</h3>
                <p style="color: var(--text-secondary); margin-bottom: 1rem;">
                    Mcaster1DNAS is built on a foundation of proven open source technology:
                </p>
                <div class="lineage-tree">
                    Icecast2 (Xiph.Org Foundation)<br/>
                    <span style="color: var(--mcaster-blue);">    ↓</span><br/>
                    Icecast-KH (Karl Heyes)<br/>
                    <span style="color: var(--dnas-green);">    ↓</span><br/>
                    <strong style="color: var(--mcaster-blue);">Mcaster1DNAS (MediaCast1)</strong>
                </div>
            </div>

            <!-- Key Features -->
            <div class="credits-section">
                <h3><i class="fas fa-star"></i> Key Features</h3>
                <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 1.25rem; margin-top: 1.5rem;">
                    <div style="background: linear-gradient(135deg, #dbeafe, #bfdbfe); padding: 1.5rem; border-radius: var(--radius-md); border-left: 4px solid var(--mcaster-blue);">
                        <h4 style="color: var(--mcaster-blue); margin-bottom: 0.75rem;">
                            <i class="fas fa-shield-alt"></i> Secure by Default
                        </h4>
                        <p style="color: var(--text-primary); font-size: 0.9375rem;">
                            HTTPS/SSL streaming enabled by default with modern encryption standards
                        </p>
                    </div>
                    <div style="background: linear-gradient(135deg, #d1fae5, #a7f3d0); padding: 1.5rem; border-radius: var(--radius-md); border-left: 4px solid var(--dnas-green);">
                        <h4 style="color: var(--dnas-accent); margin-bottom: 0.75rem;">
                            <i class="fas fa-tachometer-alt"></i> High Performance
                        </h4>
                        <p style="color: var(--text-primary); font-size: 0.9375rem;">
                            Optimized for low latency and high concurrent listener capacity
                        </p>
                    </div>
                    <div style="background: linear-gradient(135deg, #fce7f3, #fbcfe8); padding: 1.5rem; border-radius: var(--radius-md); border-left: 4px solid #ec4899;">
                        <h4 style="color: #be185d; margin-bottom: 0.75rem;">
                            <i class="fas fa-music"></i> Multi-Format Support
                        </h4>
                        <p style="color: var(--text-primary); font-size: 0.9375rem;">
                            MP3, AAC, Ogg Vorbis, Opus, FLAC, and more audio codecs supported
                        </p>
                    </div>
                    <div style="background: linear-gradient(135deg, #e9d5ff, #d8b4fe); padding: 1.5rem; border-radius: var(--radius-md); border-left: 4px solid #a855f7;">
                        <h4 style="color: #7e22ce; margin-bottom: 0.75rem;">
                            <i class="fas fa-cogs"></i> Modern Web UI
                        </h4>
                        <p style="color: var(--text-primary); font-size: 0.9375rem;">
                            Beautiful, responsive interface with real-time statistics and controls
                        </p>
                    </div>
                </div>
            </div>

            <!-- Credits -->
            <div class="credits-section">
                <h3><i class="fas fa-heart"></i> Built With</h3>
                <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 1rem;">
                    <div style="background: var(--bg-light); padding: 1.25rem; border-radius: var(--radius-md);">
                        <h4 style="color: var(--mcaster-blue); margin-bottom: 0.5rem;">
                            <i class="fas fa-music"></i> Icecast2
                        </h4>
                        <p style="font-size: 0.9375rem; color: var(--text-secondary);">
                            By Xiph.Org Foundation - The original streaming server
                        </p>
                    </div>
                    <div style="background: var(--bg-light); padding: 1.25rem; border-radius: var(--radius-md);">
                        <h4 style="color: var(--mcaster-blue); margin-bottom: 0.5rem;">
                            <i class="fas fa-code"></i> Icecast-KH
                        </h4>
                        <p style="font-size: 0.9375rem; color: var(--text-secondary);">
                            By Karl Heyes - Enhanced performance and features
                        </p>
                    </div>
                    <div style="background: var(--bg-light); padding: 1.25rem; border-radius: var(--radius-md);">
                        <h4 style="color: var(--mcaster-blue); margin-bottom: 0.5rem;">
                            <i class="fab fa-font-awesome"></i> FontAwesome
                        </h4>
                        <p style="font-size: 0.9375rem; color: var(--text-secondary);">
                            Professional icon library for modern web interfaces
                        </p>
                    </div>
                </div>
            </div>

            <!-- License -->
            <div class="credits-section">
                <h3><i class="fas fa-balance-scale"></i> Open Source License</h3>
                <div class="info-box">
                    <div class="info-box-title">
                        <i class="fab fa-osi"></i> GNU General Public License v2
                    </div>
                    <div class="info-box-content">
                        Mcaster1DNAS is free and open source software licensed under GNU GPL v2, ensuring it remains free for everyone to use, modify, and distribute.
                    </div>
                </div>
            </div>

            <!-- Contact -->
            <div class="credits-section">
                <h3><i class="fas fa-envelope"></i> Get Involved</h3>
                <div style="background: linear-gradient(135deg, var(--bg-light), var(--bg-accent)); padding: 2rem; border-radius: var(--radius-md); text-align: center;">
                    <p style="font-size: 1.0625rem; color: var(--text-primary); margin-bottom: 1.5rem;">
                        Want to contribute or learn more about Mcaster1DNAS?
                    </p>
                    <div style="display: flex; gap: 1rem; justify-content: center; flex-wrap: wrap;">
                        <a href="https://mcaster1.com" target="_blank" class="btn btn-primary">
                            <i class="fas fa-globe"></i> Visit Mcaster1.com
                        </a>
                        <a href="https://github.com/davestj/mcaster1dnas" target="_blank" class="btn btn-secondary">
                            <i class="fab fa-github"></i> View on GitHub
                        </a>
                        <a href="mailto:davestj@gmail.com" class="btn btn-secondary">
                            <i class="fas fa-envelope"></i> Contact Us
                        </a>
                    </div>
                </div>
            </div>

        </div>
    </div>

    <xsl:call-template name="web-footer"/>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>

<xsl:template match="/mcaster1stats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Server Logs - Mcaster1DNAS Admin</title>

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
                <a href="logs.xsl" class="active"><i class="fas fa-file-alt"></i> Logs</a>
                <a href="../status.xsl" target="_blank"><i class="fas fa-globe"></i> Public Page</a>
            </div>
        </div>
    </div>

    <div class="mcaster-main">
        <div class="mcaster-container">

            <!-- Access Log -->
            <div class="mcaster-card">
                <h2><i class="fas fa-file-alt"></i> Access Log</h2>
                <iframe frameborder="0" width="100%" height="400" src="showlog.xsl?log=accesslog" style="border: 2px solid var(--border-light); border-radius: var(--radius-md); background: white;">
                    No frame support - contents can be found <a href="showlog.xsl?log=accesslog">here</a>
                </iframe>
            </div>

            <!-- Error Log -->
            <div class="mcaster-card">
                <h2><i class="fas fa-exclamation-triangle"></i> Error Log</h2>
                <iframe frameborder="0" width="100%" height="400" src="showlog.xsl?log=errorlog" style="border: 2px solid var(--border-light); border-radius: var(--radius-md); background: white;">
                    No frame support - contents can be found <a href="showlog.xsl?log=errorlog">here</a>
                </iframe>
            </div>

            <!-- Playlist Log -->
            <div class="mcaster-card">
                <h2><i class="fas fa-list-ol"></i> Playlist Log</h2>
                <iframe frameborder="0" width="100%" height="300" src="showlog.xsl?log=playlistlog" style="border: 2px solid var(--border-light); border-radius: var(--radius-md); background: white;">
                    No frame support - contents can be found <a href="showlog.xsl?log=playlistlog">here</a>
                </iframe>
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

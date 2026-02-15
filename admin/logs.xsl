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

    <link rel="stylesheet" type="text/css" href="/style.css"/>
    <script src="/mcaster-utils.js"></script>

    <style>
        .log-tabs {
            display: flex;
            gap: 0.5rem;
            margin-bottom: 1.5rem;
            border-bottom: 2px solid var(--border-light);
            flex-wrap: wrap;
        }

        .log-tab {
            padding: 0.75rem 1.5rem;
            background: var(--bg-light);
            border: 2px solid var(--border-light);
            border-bottom: none;
            border-radius: var(--radius-md) var(--radius-md) 0 0;
            cursor: pointer;
            font-weight: 500;
            color: var(--text-secondary);
            transition: all var(--transition-fast);
            display: flex;
            align-items: center;
            gap: 0.5rem;
            position: relative;
            bottom: -2px;
        }

        .log-tab:hover {
            background: var(--bg-white);
            color: var(--text-primary);
            transform: translateY(-2px);
        }

        .log-tab.active {
            background: white;
            color: var(--mcaster-blue);
            border-color: var(--mcaster-blue);
            border-bottom: 2px solid white;
            transform: translateY(0);
        }

        .log-tab i {
            font-size: 1.1rem;
        }

        .log-content {
            display: none;
        }

        .log-content.active {
            display: block;
            animation: fadeIn 0.3s ease-in;
        }

        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(10px); }
            to { opacity: 1; transform: translateY(0); }
        }

        .log-iframe-container {
            background: white;
            border: 2px solid var(--border-light);
            border-radius: var(--radius-md);
            overflow: hidden;
        }

        .log-iframe {
            width: 100%;
            height: 500px;
            border: none;
        }

        .log-info {
            padding: 1rem;
            background: var(--bg-light);
            border-left: 4px solid var(--mcaster-blue);
            border-radius: var(--radius-md);
            margin-bottom: 1rem;
        }

        .log-info h3 {
            margin: 0 0 0.5rem 0;
            color: var(--mcaster-blue);
            font-size: 1.1rem;
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }

        .log-info p {
            margin: 0;
            color: var(--text-secondary);
            font-size: 0.9rem;
        }
    </style>

    <script type="text/javascript">
    //<![CDATA[
    function switchTab(tabName) {
        // Hide all tab contents
        var contents = document.getElementsByClassName('log-content');
        for (var i = 0; i < contents.length; i++) {
            contents[i].classList.remove('active');
        }

        // Remove active class from all tabs
        var tabs = document.getElementsByClassName('log-tab');
        for (var i = 0; i < tabs.length; i++) {
            tabs[i].classList.remove('active');
        }

        // Show selected tab content
        document.getElementById(tabName + '-content').classList.add('active');
        document.getElementById(tabName + '-tab').classList.add('active');

        // Store the active tab in sessionStorage
        sessionStorage.setItem('activeLogTab', tabName);
    }

    window.onload = function() {
        // Restore previously active tab or default to access log
        var activeTab = sessionStorage.getItem('activeLogTab') || 'access';
        switchTab(activeTab);
    };
    //]]>
    </script>
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
                <div class="mcaster-clock">
                    <div class="mcaster-clock-time">
                        <i class="fas fa-clock mcaster-clock-icon"></i>
                        <span id="live-time">Loading...</span>
                    </div>
                    <div class="mcaster-clock-date" id="live-date">Loading...</div>
                </div>
            </div>
            <div class="mcaster-nav">
                <a href="stats.xsl"><i class="fas fa-chart-line"></i> Stats</a>
                <a href="listmounts.xsl"><i class="fas fa-stream"></i> Mounts</a>
                <a href="managerelays.xsl"><i class="fas fa-project-diagram"></i> Relays</a>
                <a href="logs.xsl" class="active"><i class="fas fa-file-alt"></i> Logs</a>
                <a href="credits.xsl"><i class="fas fa-info-circle"></i> Credits</a>
                <a href="../status.xsl" target="_blank"><i class="fas fa-globe"></i> Public</a>
            </div>
        </div>
    </div>

    <div class="mcaster-main">
        <div class="mcaster-container">

            <div class="mcaster-card">
                <h2 style="margin-bottom: 1.5rem;">
                    <i class="fas fa-file-alt"></i> Server Logs
                </h2>

                <!-- Tab Navigation -->
                <div class="log-tabs">
                    <div class="log-tab active" id="access-tab" onclick="switchTab('access')">
                        <i class="fas fa-file-alt"></i>
                        <span>Access Log</span>
                    </div>
                    <div class="log-tab" id="error-tab" onclick="switchTab('error')">
                        <i class="fas fa-exclamation-triangle"></i>
                        <span>Error Log</span>
                    </div>
                    <div class="log-tab" id="playlist-tab" onclick="switchTab('playlist')">
                        <i class="fas fa-list-ol"></i>
                        <span>Playlist Log</span>
                    </div>
                    <div class="log-tab" id="yp-tab" onclick="switchTab('yp')">
                        <i class="fas fa-globe"></i>
                        <span>YP Connections</span>
                    </div>
                </div>

                <!-- Access Log Content -->
                <div class="log-content active" id="access-content">
                    <div class="log-info">
                        <h3>
                            <i class="fas fa-info-circle"></i>
                            Access Log
                        </h3>
                        <p>
                            Records all HTTP requests to the server including listener connections, admin page access, and file requests.
                            Use this log to monitor traffic patterns and identify connection issues.
                        </p>
                    </div>
                    <div class="log-iframe-container">
                        <iframe class="log-iframe" src="showlog.xsl?log=accesslog">
                            No frame support - contents can be found <a href="showlog.xsl?log=accesslog">here</a>
                        </iframe>
                    </div>
                </div>

                <!-- Error Log Content -->
                <div class="log-content" id="error-content">
                    <div class="log-info">
                        <h3>
                            <i class="fas fa-info-circle"></i>
                            Error Log
                        </h3>
                        <p>
                            Contains error messages, warnings, and informational messages from the server.
                            Check this log when troubleshooting configuration issues, source connection problems, or unexpected behavior.
                        </p>
                    </div>
                    <div class="log-iframe-container">
                        <iframe class="log-iframe" src="showlog.xsl?log=errorlog">
                            No frame support - contents can be found <a href="showlog.xsl?log=errorlog">here</a>
                        </iframe>
                    </div>
                </div>

                <!-- Playlist Log Content -->
                <div class="log-content" id="playlist-content">
                    <div class="log-info">
                        <h3>
                            <i class="fas fa-info-circle"></i>
                            Playlist Log
                        </h3>
                        <p>
                            Tracks metadata updates and song information for each mount point.
                            View this log to see what tracks have been played and when metadata was updated.
                        </p>
                    </div>
                    <div class="log-iframe-container">
                        <iframe class="log-iframe" src="showlog.xsl?log=playlistlog">
                            No frame support - contents can be found <a href="showlog.xsl?log=playlistlog">here</a>
                        </iframe>
                    </div>
                </div>

                <!-- YP Connections Log Content -->
                <div class="log-content" id="yp-content">
                    <div class="log-info">
                        <h3>
                            <i class="fas fa-info-circle"></i>
                            YP Directory Connections
                        </h3>
                        <p>
                            Shows all communication with YP (Yellow Pages) directory servers including add, remove, and touch operations.
                            Use this to debug YP listing issues. Note: YP logging must be configured per directory server in your config file using the <code>yp-logfile</code> option.
                        </p>
                    </div>
                    <div class="log-iframe-container">
                        <iframe class="log-iframe" src="showlog.xsl?log=yplog">
                            No frame support - contents can be found <a href="showlog.xsl?log=yplog">here</a>
                        </iframe>
                    </div>
                </div>

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

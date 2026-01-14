<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>

<xsl:template match="/mcaster1stats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Mcaster1DNAS Player - <xsl:value-of select="source/@mount"/></title>

    <!-- FontAwesome 6.x for icons -->
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css"
          integrity="sha512-iecdLmaskl7CVkqkXNQ/ZH/XLlvWZOJyj7Yy7tcenmpD1ypASozpmT/E0iPtmFIB46ZmdtAc9eNBvH0H/ZpiBw=="
          crossorigin="anonymous" referrerpolicy="no-referrer"/>

    <style>
        :root {
            --mcaster-blue: #0891b2;
            --mcaster-teal: #14b8a6;
            --dnas-green: #10b981;
            --bg-dark: #0f172a;
            --bg-medium: #1e293b;
            --bg-light: #334155;
            --text-primary: #f1f5f9;
            --text-secondary: #94a3b8;
            --border-light: #475569;
            --accent-yellow: #fbbf24;
            --status-active: #10b981;
            --status-error: #ef4444;
            --radius-md: 0.5rem;
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, sans-serif;
            background: linear-gradient(135deg, var(--bg-dark) 0%, #0c1426 100%);
            color: var(--text-primary);
            min-height: 100vh;
            padding: 1rem;
            overflow-x: hidden;
        }

        .player-window {
            max-width: 600px;
            margin: 0 auto;
            background: rgba(30, 41, 59, 0.95);
            border-radius: var(--radius-md);
            box-shadow: 0 10px 40px rgba(0, 0, 0, 0.5);
            overflow: hidden;
        }

        .player-header {
            background: linear-gradient(135deg, var(--mcaster-blue) 0%, var(--mcaster-teal) 100%);
            padding: 1.5rem;
            text-align: center;
            position: relative;
        }

        .player-header h1 {
            font-size: 1.5rem;
            margin-bottom: 0.5rem;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 0.5rem;
        }

        .brand-mcaster {
            color: #ffffff;
            font-weight: 700;
        }

        .brand-dnas {
            color: var(--accent-yellow);
            font-weight: 600;
        }

        .station-name {
            font-size: 0.875rem;
            opacity: 0.9;
        }

        .bookmark-hint {
            position: absolute;
            top: 0.5rem;
            right: 0.5rem;
            background: rgba(0, 0, 0, 0.3);
            padding: 0.25rem 0.75rem;
            border-radius: 1rem;
            font-size: 0.75rem;
            display: flex;
            align-items: center;
            gap: 0.25rem;
            cursor: pointer;
            transition: all 0.2s;
        }

        .bookmark-hint:hover {
            background: rgba(0, 0, 0, 0.5);
            transform: scale(1.05);
        }

        .player-body {
            padding: 1.5rem;
        }

        /* Now Playing Section */
        .now-playing {
            background: rgba(0, 0, 0, 0.3);
            padding: 1.5rem;
            border-radius: var(--radius-md);
            margin-bottom: 1.5rem;
            border-left: 4px solid var(--mcaster-blue);
        }

        .playing-indicator {
            display: inline-block;
            color: var(--status-active);
            font-weight: bold;
            margin-bottom: 0.5rem;
            animation: pulse 2s infinite;
        }

        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }

        .current-title {
            font-size: 1.25rem;
            font-weight: 600;
            margin-bottom: 0.5rem;
            color: #ffffff;
            overflow: hidden;
            text-overflow: ellipsis;
            white-space: nowrap;
        }

        .current-artist {
            font-size: 1rem;
            color: var(--text-secondary);
            margin-bottom: 0.5rem;
        }

        .stream-description {
            font-size: 0.875rem;
            color: var(--text-secondary);
            font-style: italic;
        }

        /* Audio Controls */
        .audio-controls {
            display: flex;
            align-items: center;
            gap: 1rem;
            margin-bottom: 1.5rem;
            padding: 1rem;
            background: rgba(0, 0, 0, 0.2);
            border-radius: var(--radius-md);
        }

        .control-btn {
            background: linear-gradient(135deg, var(--mcaster-blue), var(--mcaster-teal));
            color: white;
            border: none;
            width: 50px;
            height: 50px;
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            font-size: 1.25rem;
            transition: all 0.2s;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.3);
        }

        .control-btn:hover {
            transform: scale(1.1);
            box-shadow: 0 6px 12px rgba(0, 0, 0, 0.4);
        }

        .control-btn:active {
            transform: scale(0.95);
        }

        .control-btn.stop {
            background: linear-gradient(135deg, #ef4444, #dc2626);
        }

        .volume-control {
            flex: 1;
            display: flex;
            align-items: center;
            gap: 0.75rem;
        }

        .volume-icon {
            color: var(--mcaster-blue);
            font-size: 1.25rem;
        }

        .volume-slider {
            flex: 1;
            height: 6px;
            -webkit-appearance: none;
            appearance: none;
            background: rgba(0, 0, 0, 0.3);
            border-radius: 3px;
            outline: none;
        }

        .volume-slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 18px;
            height: 18px;
            border-radius: 50%;
            background: var(--mcaster-blue);
            cursor: pointer;
            box-shadow: 0 0 5px rgba(8, 145, 178, 0.5);
            transition: all 0.2s;
        }

        .volume-slider::-webkit-slider-thumb:hover {
            transform: scale(1.2);
            box-shadow: 0 0 10px rgba(8, 145, 178, 0.8);
        }

        .volume-slider::-moz-range-thumb {
            width: 18px;
            height: 18px;
            border-radius: 50%;
            background: var(--mcaster-blue);
            cursor: pointer;
            border: none;
            box-shadow: 0 0 5px rgba(8, 145, 178, 0.5);
            transition: all 0.2s;
        }

        .volume-level {
            color: var(--accent-yellow);
            font-weight: bold;
            min-width: 45px;
            text-align: center;
        }

        /* VU Meters */
        .vu-meters {
            margin-bottom: 1.5rem;
        }

        .vu-meter {
            position: relative;
            height: 30px;
            background: #000;
            border-radius: 4px;
            overflow: hidden;
            margin-bottom: 0.5rem;
            box-shadow: inset 0 0 10px rgba(0, 0, 0, 0.7);
        }

        .vu-meter-label {
            position: absolute;
            left: 0.5rem;
            top: 50%;
            transform: translateY(-50%);
            font-size: 0.75rem;
            color: rgba(255, 255, 255, 0.7);
            z-index: 2;
            font-weight: bold;
        }

        .vu-meter-fill {
            position: absolute;
            left: 0;
            top: 0;
            height: 100%;
            width: 0%;
            background: linear-gradient(to right,
                #00aa00 0%, #00dd00 70%,
                #dddd00 70%, #dddd00 85%,
                #dd0000 85%, #dd0000 100%);
            transition: width 0.1s linear;
        }

        .vu-meter-peak {
            position: absolute;
            top: 0;
            height: 100%;
            width: 2px;
            background-color: #ff0000;
            z-index: 3;
            transition: left 0.1s ease-out;
        }

        /* Stream Info Cards */
        .stream-info {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
            gap: 0.75rem;
            margin-bottom: 1.5rem;
        }

        .info-card {
            background: rgba(0, 0, 0, 0.2);
            padding: 0.75rem;
            border-radius: var(--radius-md);
            border: 1px solid var(--border-light);
        }

        .info-label {
            font-size: 0.75rem;
            color: var(--text-secondary);
            text-transform: uppercase;
            margin-bottom: 0.25rem;
        }

        .info-value {
            font-size: 1rem;
            font-weight: 600;
            color: var(--text-primary);
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }

        .info-value i {
            color: var(--mcaster-blue);
        }

        /* Buffering Indicator */
        .buffering-overlay {
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background: rgba(0, 0, 0, 0.8);
            display: none;
            align-items: center;
            justify-content: center;
            z-index: 9999;
        }

        .buffering-overlay.active {
            display: flex;
        }

        .buffering-spinner {
            font-size: 3rem;
            color: var(--mcaster-blue);
            animation: spin 1s linear infinite;
        }

        @keyframes spin {
            from { transform: rotate(0deg); }
            to { transform: rotate(360deg); }
        }

        /* Mobile Responsive */
        @media (max-width: 640px) {
            .player-window {
                margin: 0;
                border-radius: 0;
            }

            .player-body {
                padding: 1rem;
            }

            .stream-info {
                grid-template-columns: repeat(2, 1fr);
            }

            .current-title {
                font-size: 1rem;
            }
        }

        /* Link Styling */
        a {
            color: var(--mcaster-blue);
            text-decoration: none;
            transition: color 0.2s;
        }

        a:hover {
            color: var(--mcaster-teal);
        }

        /* Status Badge */
        .status-live {
            display: inline-flex;
            align-items: center;
            gap: 0.5rem;
            background: rgba(16, 185, 129, 0.2);
            color: var(--status-active);
            padding: 0.25rem 0.75rem;
            border-radius: 1rem;
            font-size: 0.75rem;
            font-weight: 600;
            border: 1px solid var(--status-active);
        }

        .live-dot {
            width: 8px;
            height: 8px;
            background: var(--status-active);
            border-radius: 50%;
            animation: pulse 2s infinite;
        }
    </style>
</head>
<body>
    <div class="buffering-overlay" id="bufferingOverlay">
        <div>
            <i class="fas fa-spinner buffering-spinner"></i>
            <p style="text-align: center; margin-top: 1rem;">Buffering stream...</p>
        </div>
    </div>

    <div class="player-window">
        <div class="player-header">
            <div class="bookmark-hint" onclick="showBookmarkHint()" title="Bookmark this player">
                <i class="fas fa-bookmark"></i>
                <span>Bookmark Me</span>
            </div>

            <h1>
                <i class="fas fa-broadcast-tower"></i>
                <span class="brand-mcaster">Mcaster1</span>
                <span class="brand-dnas">DNAS</span>
            </h1>

            <div class="station-name">
                <xsl:choose>
                    <xsl:when test="source/server_name">
                        <xsl:value-of select="source/server_name"/>
                    </xsl:when>
                    <xsl:otherwise>Streaming Server</xsl:otherwise>
                </xsl:choose>
            </div>
        </div>

        <div class="player-body">
            <!-- Now Playing -->
            <div class="now-playing">
                <div class="playing-indicator" id="playingIndicator">
                    <i class="fas fa-circle"></i> <span id="statusText">READY</span>
                </div>

                <div class="status-live">
                    <span class="live-dot"></span>
                    LIVE
                </div>

                <div class="current-title" id="currentTitle">
                    <xsl:choose>
                        <xsl:when test="source/title">
                            <xsl:value-of select="source/title"/>
                        </xsl:when>
                        <xsl:otherwise>No Title Available</xsl:otherwise>
                    </xsl:choose>
                </div>

                <xsl:if test="source/artist">
                    <div class="current-artist">
                        <i class="fas fa-user"></i>
                        <xsl:value-of select="source/artist"/>
                    </div>
                </xsl:if>

                <xsl:if test="source/server_description">
                    <div class="stream-description">
                        <xsl:value-of select="source/server_description"/>
                    </div>
                </xsl:if>
            </div>

            <!-- Audio Controls -->
            <div class="audio-controls">
                <button class="control-btn" onclick="playStream()" title="Play" id="playBtn">
                    <i class="fas fa-play"></i>
                </button>
                <button class="control-btn" onclick="pauseStream()" title="Pause" id="pauseBtn">
                    <i class="fas fa-pause"></i>
                </button>
                <button class="control-btn stop" onclick="stopStream()" title="Stop">
                    <i class="fas fa-stop"></i>
                </button>

                <div class="volume-control">
                    <i class="fas fa-volume-up volume-icon" id="volumeIcon"></i>
                    <input type="range" class="volume-slider" id="volumeSlider" min="0" max="100" value="80" oninput="adjustVolume()"/>
                    <span class="volume-level" id="volumeLevel">80%</span>
                </div>
            </div>

            <!-- VU Meters -->
            <div class="vu-meters">
                <div class="vu-meter">
                    <div class="vu-meter-label">L</div>
                    <div class="vu-meter-fill" id="vuMeterLeft"></div>
                    <div class="vu-meter-peak" id="vuPeakLeft"></div>
                </div>
                <div class="vu-meter">
                    <div class="vu-meter-label">R</div>
                    <div class="vu-meter-fill" id="vuMeterRight"></div>
                    <div class="vu-meter-peak" id="vuPeakRight"></div>
                </div>
            </div>

            <!-- Stream Info -->
            <div class="stream-info">
                <div class="info-card">
                    <div class="info-label">Format</div>
                    <div class="info-value">
                        <i class="fas fa-file-audio"></i>
                        <xsl:choose>
                            <xsl:when test="contains(source/server_type, 'mpeg')">MP3</xsl:when>
                            <xsl:when test="contains(source/server_type, 'ogg')">Vorbis</xsl:when>
                            <xsl:when test="contains(source/server_type, 'aac')">AAC</xsl:when>
                            <xsl:when test="contains(source/server_type, 'opus')">Opus</xsl:when>
                            <xsl:otherwise>Unknown</xsl:otherwise>
                        </xsl:choose>
                    </div>
                </div>

                <div class="info-card">
                    <div class="info-label">Bitrate</div>
                    <div class="info-value">
                        <i class="fas fa-tachometer-alt"></i>
                        <xsl:choose>
                            <xsl:when test="source/bitrate">
                                <xsl:value-of select="source/bitrate"/> kbps
                            </xsl:when>
                            <xsl:when test="source/ice-bitrate">
                                <xsl:value-of select="source/ice-bitrate"/> kbps
                            </xsl:when>
                            <xsl:otherwise>N/A</xsl:otherwise>
                        </xsl:choose>
                    </div>
                </div>

                <div class="info-card">
                    <div class="info-label">Sample Rate</div>
                    <div class="info-value">
                        <i class="fas fa-wave-square"></i>
                        <xsl:choose>
                            <xsl:when test="source/samplerate">
                                <xsl:value-of select="source/samplerate"/> Hz
                            </xsl:when>
                            <xsl:when test="source/ice-samplerate">
                                <xsl:value-of select="source/ice-samplerate"/> Hz
                            </xsl:when>
                            <xsl:otherwise>N/A</xsl:otherwise>
                        </xsl:choose>
                    </div>
                </div>

                <div class="info-card">
                    <div class="info-label">Channels</div>
                    <div class="info-value">
                        <i class="fas fa-volume-up"></i>
                        <xsl:choose>
                            <xsl:when test="source/channels = 2">Stereo</xsl:when>
                            <xsl:when test="source/channels = 1">Mono</xsl:when>
                            <xsl:when test="source/ice-channels = 2">Stereo</xsl:when>
                            <xsl:when test="source/ice-channels = 1">Mono</xsl:when>
                            <xsl:otherwise>N/A</xsl:otherwise>
                        </xsl:choose>
                    </div>
                </div>

                <div class="info-card">
                    <div class="info-label">Listeners</div>
                    <div class="info-value">
                        <i class="fas fa-users"></i>
                        <xsl:choose>
                            <xsl:when test="source/listeners">
                                <xsl:value-of select="source/listeners"/>
                            </xsl:when>
                            <xsl:otherwise>0</xsl:otherwise>
                        </xsl:choose>
                    </div>
                </div>

                <div class="info-card">
                    <div class="info-label">Peak</div>
                    <div class="info-value">
                        <i class="fas fa-chart-line"></i>
                        <xsl:choose>
                            <xsl:when test="source/listener_peak">
                                <xsl:value-of select="source/listener_peak"/>
                            </xsl:when>
                            <xsl:otherwise>0</xsl:otherwise>
                        </xsl:choose>
                    </div>
                </div>
            </div>

            <!-- Hidden Audio Element -->
            <audio id="streamAudio" preload="none" style="display: none;">
                <source src="{source/@mount}" type="{source/server_type}"/>
                Your browser does not support the audio element.
            </audio>
        </div>
    </div>

    <script>
    //<![CDATA[
        // Stream configuration from XSLT
        const streamUrl = '<xsl:value-of select="source/@mount"/>';
        const streamTitle = '<xsl:value-of select="source/title"/>';
        const stationName = '<xsl:value-of select="source/server_name"/>';
        const streamType = '<xsl:value-of select="source/server_type"/>';

        // Player state
        let audioElement = document.getElementById('streamAudio');
        let isPlaying = false;
        let audioContext = null;
        let analyser = null;
        let source = null;
        let vuMeterInterval = null;

        // Initialize volume from localStorage or default to 80%
        window.addEventListener('load', function() {
            const savedVolume = localStorage.getItem('mcaster1_volume');
            if (savedVolume) {
                document.getElementById('volumeSlider').value = savedVolume;
                adjustVolume();
            } else {
                audioElement.volume = 0.8;
            }

            // Set up audio event listeners
            setupAudioEvents();

            // Update title every 5 seconds
            setInterval(updateMetadata, 5000);
        });

        function setupAudioEvents() {
            audioElement.addEventListener('play', function() {
                document.getElementById('statusText').textContent = 'PLAYING';
                document.getElementById('playingIndicator').style.color = '#10b981';
                hideBuffering();
                startVUMeters();
            });

            audioElement.addEventListener('pause', function() {
                document.getElementById('statusText').textContent = 'PAUSED';
                document.getElementById('playingIndicator').style.color = '#fbbf24';
                stopVUMeters();
            });

            audioElement.addEventListener('ended', function() {
                document.getElementById('statusText').textContent = 'STOPPED';
                stopVUMeters();
            });

            audioElement.addEventListener('waiting', function() {
                showBuffering();
            });

            audioElement.addEventListener('canplay', function() {
                hideBuffering();
            });

            audioElement.addEventListener('error', function(e) {
                hideBuffering();
                document.getElementById('statusText').textContent = 'ERROR';
                alert('Stream playback error. Please try again.');
            });
        }

        function playStream() {
            showBuffering();
            audioElement.load();
            audioElement.play().catch(function(error) {
                hideBuffering();
                console.error('Playback failed:', error);
                alert('Unable to play stream. Please check your browser settings.');
            });
            isPlaying = true;
            updateDocumentTitle();
        }

        function pauseStream() {
            audioElement.pause();
            isPlaying = false;
        }

        function stopStream() {
            audioElement.pause();
            audioElement.currentTime = 0;
            isPlaying = false;
            document.getElementById('statusText').textContent = 'STOPPED';
            stopVUMeters();
        }

        function adjustVolume() {
            const slider = document.getElementById('volumeSlider');
            const level = document.getElementById('volumeLevel');
            const icon = document.getElementById('volumeIcon');

            const volume = slider.value;
            audioElement.volume = volume / 100;
            level.textContent = volume + '%';

            // Update icon
            if (volume == 0) {
                icon.className = 'fas fa-volume-mute volume-icon';
            } else if (volume < 50) {
                icon.className = 'fas fa-volume-down volume-icon';
            } else {
                icon.className = 'fas fa-volume-up volume-icon';
            }

            // Save to localStorage
            localStorage.setItem('mcaster1_volume', volume);
        }

        function showBuffering() {
            document.getElementById('bufferingOverlay').classList.add('active');
        }

        function hideBuffering() {
            document.getElementById('bufferingOverlay').classList.remove('active');
        }

        function updateDocumentTitle() {
            if (isPlaying) {
                document.title = '▶ ' + streamTitle + ' - ' + stationName;
            } else {
                document.title = 'Mcaster1DNAS Player - ' + stationName;
            }
        }

        function updateMetadata() {
            // Poll the stats endpoint for updated metadata
            if (!isPlaying) return;

            fetch('../status-json.xsl')
                .then(response => response.json())
                .then(data => {
                    if (data && data.icestats && data.icestats.source) {
                        const sources = Array.isArray(data.icestats.source)
                            ? data.icestats.source
                            : [data.icestats.source];

                        const currentSource = sources.find(s => s.listenurl && s.listenurl.includes(streamUrl));
                        if (currentSource && currentSource.title) {
                            const titleElement = document.getElementById('currentTitle');
                            if (titleElement.textContent !== currentSource.title) {
                                titleElement.textContent = currentSource.title;
                                updateDocumentTitle();
                            }
                        }
                    }
                })
                .catch(err => console.error('Metadata update failed:', err));
        }

        function startVUMeters() {
            if (!audioContext) {
                try {
                    audioContext = new (window.AudioContext || window.webkitAudioContext)();
                    analyser = audioContext.createAnalyser();
                    analyser.fftSize = 256;

                    if (!source) {
                        source = audioContext.createMediaElementSource(audioElement);
                        source.connect(analyser);
                        analyser.connect(audioContext.destination);
                    }
                } catch (e) {
                    console.error('Web Audio API not supported:', e);
                    return;
                }
            }

            const bufferLength = analyser.frequencyBinCount;
            const dataArray = new Uint8Array(bufferLength);

            vuMeterInterval = setInterval(function() {
                analyser.getByteFrequencyData(dataArray);

                // Calculate average amplitude
                let sum = 0;
                for (let i = 0; i < bufferLength; i++) {
                    sum += dataArray[i];
                }
                const average = sum / bufferLength;
                const percentage = (average / 255) * 100;

                // Update both channels with slight variation
                const leftPercentage = Math.min(100, percentage + (Math.random() * 5 - 2.5));
                const rightPercentage = Math.min(100, percentage + (Math.random() * 5 - 2.5));

                document.getElementById('vuMeterLeft').style.width = leftPercentage + '%';
                document.getElementById('vuMeterRight').style.width = rightPercentage + '%';

                // Update peaks
                document.getElementById('vuPeakLeft').style.left = leftPercentage + '%';
                document.getElementById('vuPeakRight').style.left = rightPercentage + '%';
            }, 100);
        }

        function stopVUMeters() {
            if (vuMeterInterval) {
                clearInterval(vuMeterInterval);
                vuMeterInterval = null;
            }
            document.getElementById('vuMeterLeft').style.width = '0%';
            document.getElementById('vuMeterRight').style.width = '0%';
        }

        function showBookmarkHint() {
            const msg = 'To bookmark this player:\n\n' +
                       '• Desktop: Press Ctrl+D (Windows/Linux) or Cmd+D (Mac)\n' +
                       '• Mobile: Tap the browser menu and select "Add to Home Screen"\n\n' +
                       'This allows you to quickly access this stream anytime!';
            alert(msg);
        }

        // Keyboard shortcuts
        document.addEventListener('keydown', function(e) {
            if (e.code === 'Space') {
                e.preventDefault();
                if (isPlaying) {
                    pauseStream();
                } else {
                    playStream();
                }
            } else if (e.code === 'Escape') {
                stopStream();
            }
        });
    //]]>
    </script>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

# Mcaster1DNAS - Digital Network Audio Server

[![License: GPL v2](https://img.shields.io/badge/License-GPL_v2-blue.svg)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
[![Version](https://img.shields.io/badge/version-2.5.1--rc1-brightgreen.svg)](https://github.com/davestj/mcaster1dnas/releases)
[![Build Status](https://img.shields.io/badge/build-passing-success.svg)](https://github.com/davestj/mcaster1dnas)
[![Last Commit](https://img.shields.io/github/last-commit/davestj/mcaster1dnas)](https://github.com/davestj/mcaster1dnas/commits/main)
[![Language](https://img.shields.io/badge/language-C-blue.svg)](https://github.com/davestj/mcaster1dnas)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20BSD%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)](https://github.com/davestj/mcaster1dnas)
[![ICY-META](https://img.shields.io/badge/ICY--META-v2.2-purple.svg)](ICY2_PROTOCOL_SPEC.md)
[![YAML Config](https://img.shields.io/badge/config-YAML%20%7C%20XML-yellow.svg)](YAML_IMPLEMENTATION.md)
[![SSL Gen](https://img.shields.io/badge/SSL-built--in%20cert%20gen-green.svg)](docs/SSL_CERT_GENERATION.md)

### Audio Format Support
[![MP3](https://img.shields.io/badge/codec-MP3-red.svg)](https://en.wikipedia.org/wiki/MP3)
[![AAC](https://img.shields.io/badge/codec-AAC-blue.svg)](https://en.wikipedia.org/wiki/Advanced_Audio_Coding)
[![Ogg Vorbis](https://img.shields.io/badge/codec-Ogg_Vorbis-orange.svg)](https://xiph.org/vorbis/)
[![Opus](https://img.shields.io/badge/codec-Opus-purple.svg)](https://opus-codec.org/)
[![FLAC](https://img.shields.io/badge/codec-FLAC-green.svg)](https://xiph.org/flac/)
[![Speex](https://img.shields.io/badge/codec-Speex-teal.svg)](https://www.speex.org/)
[![Theora](https://img.shields.io/badge/codec-Theora-brown.svg)](https://www.theora.org/)

### Protocol Support
[![HTTP](https://img.shields.io/badge/protocol-HTTP-blue.svg)](https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol)
[![HTTPS/SSL](https://img.shields.io/badge/protocol-HTTPS%2FSSL-green.svg)](https://en.wikipedia.org/wiki/HTTPS)
[![Icecast](https://img.shields.io/badge/protocol-Icecast-orange.svg)](https://icecast.org/)
[![Shoutcast](https://img.shields.io/badge/protocol-Shoutcast-red.svg)](https://en.wikipedia.org/wiki/Shoutcast)
[![ICY-META v2.2](https://img.shields.io/badge/protocol-ICY--META_v2.2-purple.svg)](ICY2_PROTOCOL_SPEC.md)

---

## 📻 Overview

**Mcaster1DNAS** (Digital Network Audio Server) is a powerful, enterprise-grade streaming media server designed for professional audio broadcasting. Built on the proven foundation of Icecast-KH and Icecast2, Mcaster1DNAS provides high-quality audio streaming with low latency, modern web interface, and excellent reliability.

### 🎯 Key Features

- 🔒 **Secure by Default** - HTTPS/SSL streaming enabled out of the box
- 🆕 **ICY-META v2.2 Protocol** - Extended metadata with podcast, video, social media, track data, and live programming support ([spec](ICY2_PROTOCOL_SPEC.md) | [guide](docs/ICY2_PROTOCOL.md))
- 🔑 **Built-in SSL Cert Generator** - Generate self-signed certs or CSRs directly from `mcaster1` CLI or `mcaster1win.exe` GUI — no separate openssl install needed ([guide](docs/SSL_CERT_GENERATION.md))
- 🎛️ **Per-Listener SSL Enforcement** - Mark each listen-socket as `ssl: true` (TLS only) or `ssl: false` (plain HTTP only) in YAML or XML config
- 📻 **Static Mount Types** - podcast, socialcast, and on-demand mount types alongside live streaming ([guide](docs/STATIC_MOUNTS.md))
- 📝 **YAML Configuration** - Modern YAML config alongside traditional XML ([guide](YAML_IMPLEMENTATION.md))
- 🎵 **Multi-Format Audio** - MP3, AAC, Ogg Vorbis, Opus, FLAC, Speex, and Theora
- 🚀 **High Performance** - Optimized for low latency and high concurrent listener capacity
- 💻 **Modern Web Interface** - Beautiful, responsive HTML5/CSS3 admin and public interfaces
- 📊 **Real-Time Statistics** - Live dashboard with updating metrics
- ⏰ **Live System Clock** - Real-time clock display on all pages
- 💡 **Interactive Help** - Contextual tooltips explaining streaming concepts
- 🔐 **Advanced Authentication** - User management and stream protection
- 🔄 **Relay Support** - Distribute streams across multiple servers
- 📈 **Comprehensive Logging** - Detailed access, error, playlist, and YP connection logs
- 🎶 **Song History API** - In-memory track play log exposed as XML at `/mcaster1songdata`
- 📻 **Track History UI** - Browsable track history with music service lookup icons (MusicBrainz, Last.fm, Discogs, AllMusic)
- 🎧 **Browser Audio Player** - Built-in HTML5 player with VU meters, volume control, and real-time metadata ([docs](WEBPLAYER_FEATURE.md))
- 🔊 **Full Codec Stats** - Public exposure of bitrate, samplerate, channels, and codec for all formats including Opus

---

## 🌳 Project Lineage

Mcaster1DNAS is a fork of the excellent streaming server projects that came before it:

```
Icecast2 (Xiph.Org Foundation)
    ↓
Icecast-KH (Karl Heyes)
    ↓
Mcaster1DNAS (MediaCast1)
```

### Credits to Upstream Projects

- **[Icecast2](https://icecast.org/)** by Xiph.Org Foundation - The original streaming media server
- **[Icecast-KH](https://github.com/karlheyes/icecast-kh)** by Karl Heyes - Performance enhancements and advanced features

We maintain all original credits, licensing (GNU GPL v2), and acknowledgments while establishing our own identity within the MediaCast1 ecosystem.

---

## ✨ What Makes Mcaster1DNAS Different?

### Modern UI Enhancements

- **🎨 Professional Web Interface** - Complete HTML5/CSS3 redesign with FontAwesome 6.4.0 icons
- **📱 Responsive Design** - Mobile-friendly interface that works on all devices
- **💡 Interactive Help System** - Hover-activated tooltips explaining every feature
- **⏰ Live Clock** - Real-time clock and date display in header (updates every second)
- **⚡ Page Load Metrics** - Performance tracking showing page load times
- **🎯 Intuitive Navigation** - Seamless navigation between admin and public interfaces with shared header/footer templates
- **🏆 Credits Page** - Comprehensive fork history and acknowledgments
- **🎧 Integrated Browser Player** - One-click streaming from any page; VU meters, volume control, keyboard shortcuts ([docs](WEBPLAYER_FEATURE.md))

### Song History & Track Lookup _(New in 2.5.1-rc1)_

- **🎶 `/mcaster1songdata` XML API** - In-memory ring buffer of the last N played tracks (default: 25, configurable, 0 = unlimited) exposed as a machine-readable XML endpoint for third-party apps and dashboards
- **📋 Track History Pages** - `/admin/songdata.xsl` (admin) and `/songdata.xsl` (public) show a rich table of recently played tracks with:
  - Artist, title, mount point, and codec quality badge (codec / kbps / kHz / mono or stereo)
  - Played-at timestamp and live "on air" duration counter (Xm Ys)
  - Listener count at the moment the track played
  - Animated pulsing green dot on the most recently added entry
- **🔍 Music Service Lookup** - Every track row carries four one-click lookup icons linking directly to search results on:
  - 🟠 **MusicBrainz** (`fa-database`) — open music encyclopedia
  - 🔴 **Last.fm** (`fab fa-lastfm`) — scrobbling and artist radio
  - ⚫ **Discogs** (`fa-record-vinyl`) — release/label database
  - 🟢 **AllMusic** (`fa-music`) — editorial reviews and discographies
- **🔁 Deduplication** - Consecutive identical titles on the same mount are silently ignored; `ended_at` is back-filled when the next track starts for accurate time-on-air calculation
- **⚙️ Configurable limit** - Set `song-history-limit` in YAML or XML config; 0 means keep everything (memory-backed, no disk I/O)

### Codec & Metadata Enhancements _(New in 2.5.1-rc1)_

- **🔊 Full Public Codec Stats** - `audio_codecid`, `mpeg_samplerate`, `mpeg_channels` promoted from `STATS_HIDDEN` → `STATS_COUNTERS` so public status pages and third-party clients can read them
- **🎼 Opus Now Playing** - `format_opus.c` now parses `OpusHead` to expose `audio_channels` and `audio_samplerate` stats; `opus_set_tag()` added to capture ICY title/artist metadata from Opus streams (previously untracked — Opus streams had no song-change events)
- **📡 Max Listeners in Stats** - `max_listeners` from server config is now exposed in the global stats XML so public dashboards can show server capacity

### Enhanced Features

- **🔒 HTTPS/SSL by Default** - Ports 9330 (HTTP) and 9443 (HTTPS) pre-configured
- **📊 Enhanced Statistics** - Modern dashboard with visual stats and graphs
- **🎛️ Mount Point Management** - Easy-to-use interface for managing streams
- **👥 User Authentication** - Advanced user and stream authentication system
- **📝 Metadata Updates** - Simple interface for updating "Now Playing" information
- **🔄 Listener Migration** - Move listeners between mount points seamlessly
- **📋 Comprehensive Logging** - Real-time log viewing in the web interface

### Enterprise Features

- **🏢 Production-Ready** - Built for professional broadcasting environments
- **⚡ High Concurrency** - Optimized for thousands of concurrent listeners
- **🔧 Dual Configuration** - YAML and XML config with sensible defaults
- **📚 Complete Documentation** - Comprehensive guides and help system
- **🛠️ Admin Tools** - Full-featured admin interface for server management

---

## 🚀 Quick Start

### Prerequisites

Mcaster1DNAS requires the following packages:

- **libxml2** - XML parsing library ([download](http://xmlsoft.org/downloads.html))
- **libxslt** - XSLT transformation library ([download](http://xmlsoft.org/XSLT/downloads.html))
- **libcurl** - HTTP client library - version 7.10+ ([download](http://curl.haxx.se/download.html))
- **OpenSSL** - SSL/TLS library for HTTPS support ([download](https://www.openssl.org/source/))
- **Ogg/Vorbis** - Ogg Vorbis codec support - version 1.0+ ([download](http://www.vorbis.com/files))

**Optional codecs:**
- **Speex** - Voice codec support
- **Theora** - Video codec support
- **FLAC** - Lossless audio codec

### Installation (Linux/Unix/macOS)

```bash
# 1. Clone the repository
git clone https://github.com/davestj/mcaster1dnas.git
cd mcaster1dnas

# 2. Generate build system
./autogen.sh

# 3. Configure with all features
./configure \
  --prefix=/usr/local/mcaster1dnas \
  --with-openssl \
  --with-ogg \
  --with-vorbis \
  --with-theora \
  --with-speex \
  --with-curl

# 4. Compile
make -j$(nproc)

# 5. Install
sudo make install

# 6. Create log directory
mkdir -p /usr/local/mcaster1dnas/logs

# 7. Copy and edit configuration
cp mcaster1-production.xml /usr/local/mcaster1dnas/etc/mcaster1.xml
nano /usr/local/mcaster1dnas/etc/mcaster1.xml

# IMPORTANT: Change default passwords in the config file!
```

### Quick Build Script

For convenience, use the automated build script:

```bash
./build.sh
```

This script handles all build steps automatically and provides detailed progress feedback.

### Running the Server

```bash
# Start in foreground (for testing)
/usr/local/mcaster1dnas/bin/mcaster1 -c /usr/local/mcaster1dnas/etc/mcaster1.xml

# Start as background daemon
/usr/local/mcaster1dnas/bin/mcaster1 -c /usr/local/mcaster1dnas/etc/mcaster1.xml -b

# Using the control script
./mcaster1-control.sh start   # Start server
./mcaster1-control.sh stop    # Stop server
./mcaster1-control.sh restart # Restart server
./mcaster1-control.sh status  # Check status
./mcaster1-control.sh logs    # View logs
```

### Default Access URLs

After starting the server, access the web interfaces:

- **HTTP:** `http://your-server:9330/`
- **HTTPS:** `https://your-server:9443/`
- **Public Status:** `https://your-server:9443/status.xsl`
- **Track History (public):** `https://your-server:9443/songdata.xsl`
- **Song Data API (raw XML):** `https://your-server:9443/admin/songdata`
- **Admin Interface:** `https://your-server:9443/admin/stats.xsl`
  - Default credentials: `admin` / `hackme` (⚠️ **CHANGE THESE!**)
- **Admin Track History:** `https://your-server:9443/admin/songdata.xsl`
- **Admin Web Player:** `https://your-server:9443/admin/webplayer.xsl`

---

## 🎯 ICY-META v2.2 Protocol

Mcaster1DNAS implements the **ICY-META v2.2 extended metadata protocol**, providing rich metadata support beyond legacy ICY 1.x.

### 🌟 Features

- **Auto-Detection** - Automatically detects ICY2-compliant encoders via `icy-metadata-version: 2.x` header
- **Backward Compatible** - Seamlessly falls back to ICY 1.x for legacy encoders
- **Zero Configuration** - No server config changes needed, works automatically
- **50+ Metadata Fields** - Comprehensive metadata across 9 categories:
  - **Core**: station-id, name, url, genre, bitrate, public
  - **Track**: title, artist, album, ISRC, BPM, label, composer
  - **Show/Programming**: show-title, show-start/end, DJ handle, venue, language
  - **Podcast**: host, rss, episode, season, duration
  - **Video**: type, link, title, platform, resolution
  - **Social Media**: dj-handle, twitter, instagram, tiktok, emoji, hashtags
  - **Audio Technical**: codec, samplerate, channels, bitdepth
  - **Listener Engagement**: requests-url, chat-url, donate-url
  - **Content Classification**: nsfw, ai-generated, geo-region, content-rating

### 📡 Example ICY2 Request

```http
PUT /my-stream.mp3 HTTP/1.1
Host: server.example.com:9443
Authorization: Basic c291cmNlOnBhc3N3b3Jk
Content-Type: audio/mpeg

icy-metadata-version: 2.1
icy-station-id: unique-station-id
icy-name: My Radio Station
icy-genre: Electronic/House
icy-url: https://myradio.com
icy-br: 320
icy-pub: 1

icy-podcast-host: DJ Name
icy-podcast-rss: https://myradio.com/podcast.rss
icy-video-type: live
icy-video-platform: youtube
icy-dj-handle: @mydj
icy-social-twitter: @myradio
icy-emoji: 🎵🔥
icy-hashtags: #electronic #house #live
```

### 🧪 Test with cURL

```bash
curl -k -X PUT \
  -H "icy-metadata-version: 2.1" \
  -H "icy-station-id: test-001" \
  -H "icy-name: Test Station" \
  -H "icy-genre: Test" \
  -H "icy-url: https://example.com" \
  -H "icy-br: 128" \
  -H "Authorization: Basic c291cmNlOmhhY2ttZQ==" \
  -H "Content-Type: audio/mpeg" \
  --data-binary @audio.mp3 \
  https://your-server:9443/test.mp3
```

### 📖 Full Specification

See **[ICY2_PROTOCOL_SPEC.md](ICY2_PROTOCOL_SPEC.md)** for the complete protocol specification and
**[docs/ICY2_PROTOCOL.md](docs/ICY2_PROTOCOL.md)** for the implementation guide, including:
- All metadata field definitions (50+ fields)
- Static mount metadata push (podcast, socialcast, on-demand)
- Admin API metadata update examples
- Config-level ICY2 defaults per mount
- Stats XML exposure of ICY2 fields

---

## 📝 YAML Configuration Support

Mcaster1DNAS v2.5.0+ supports **YAML configuration** alongside traditional XML, providing a modern, human-readable alternative.

### ✨ YAML Features

- **Full Feature Parity** - 100% of XML features available in YAML
- **Auto-Detection** - Automatically detects YAML vs XML format
- **Easy to Read** - Clean, indented structure without angle brackets
- **Comments** - Native support for inline comments
- **Four Templates Included**:
  - `mcaster1.yaml.in` - Full-featured template with examples
  - `mcaster1_minimal.yaml.in` - Bare minimum configuration
  - `mcaster1_shoutcast_compat.yaml.in` - Shoutcast compatibility
  - `mcaster1_advanced.yaml.in` - All advanced features

### 📋 YAML Example

```yaml
# Mcaster1DNAS Configuration
server:
  location: "Earth"
  admin: "admin@example.com"
  hostname: "stream.example.com"

  limits:
    clients: 100
    sources: 10
    workers: 2
    song-history-limit: 25   # tracks kept in memory for /songdata API (0 = unlimited)

listen-sockets:
  - port: 9330
    bind-address: "0.0.0.0"

  - port: 9443
    bind-address: "0.0.0.0"
    ssl: true

authentication:
  source-password: "hackme"
  admin-username: "admin"
  admin-password: "hackme"

paths:
  basedir: "/usr/local/mcaster1dnas"
  logdir: "/var/log/mcaster1dnas"
  webroot: "/usr/local/mcaster1dnas/web"
  adminroot: "/usr/local/mcaster1dnas/admin"
  ssl-certificate: "/etc/ssl/mcaster1dnas.pem"

mounts:
  - mount-name: "/stream.mp3"
    max-listeners: 100
    public: true
```

### 🔧 Using YAML Configuration

```bash
# Build with YAML support
./configure --with-yaml --prefix=/usr/local/mcaster1dnas
make && sudo make install

# Use YAML config file
./mcaster1 -c mcaster1-production.yaml
```

### 📚 YAML Documentation

See **[YAML_IMPLEMENTATION.md](YAML_IMPLEMENTATION.md)** for complete YAML usage guide, including:
- All configuration options
- Migration from XML to YAML
- Advanced features and examples
- Troubleshooting

---

## 🪟 Windows — Native GUI (Beta)

Mcaster1DNAS ships a full native Windows GUI application (`mcaster1win.exe`) built with
**Visual Studio 2022 (version 17 / MSBuild v17)** targeting Windows 10/11 x64.
This is a significant upgrade from the original Icecast2 Windows port and is actively
developed on the `windows-dev` branch.

### What's New in the Windows Beta

#### Resizable Window (ResizableLib)
The GUI now integrates **ResizableLib by Paolo Messina** (Artistic License 2.0), replacing
brittle hand-coded pixel math with declarative anchor-based layout:
- Drag any edge or corner — all controls reflow correctly
- Maximize/restore works seamlessly across all tab pages
- Window position and size persist between sessions (stored in the Windows registry)
- All four tab pages (Status, Stats, Config, Log) resize independently with proper anchoring

#### Real-Time Clocks
- **Uptime clock** — shows `Up: HH:MM:SS` next to the server status indicator, incrementing every 500ms while the server is running
- **System time status bar** — always-on clock at the bottom of the main window (`System Time: Mon Feb 22 2026  06:45:12 PM`), updates every second via a dedicated WM_TIMER

#### Real-Time Log Viewer
The Log tab tails all server log files (access, error, playlist) in real time with
color-coded lines (errors red, info amber, debug grey). Uses `_fsopen` with `_SH_DENYNO`
share mode so reading works even while the server holds the files open for writing.

#### HTTP Admin Authentication Fixed
A critical Windows-only bug was fixed in `src/params.c` where MSVC struct positional
initializers in `mc_http_printf` / `mc_params_printf` were mapping fields incorrectly,
causing `name=NULL` and silently dropping **all** HTTP response headers (including
`WWW-Authenticate`) on Windows. The fix uses `memset + explicit field assignment`.

#### Command-Line Interface
The GUI application supports command-line flags for scripted and automated use:

```
mcaster1win.exe [options]

  -c <file>                          Use specified config file (YAML or XML)
  -s                                 Auto-start the server on launch
  -m                                 Start minimised to the system tray
  -v                                 Print version and exit
  -h                                 Print help and exit
  --ssl-gencert                      Generate SSL certificate/CSR then exit
  --ssl-gentype=selfsigned|csr       Output type
  --subj="<subject>"                 X.509 subject string
  --ssl-gencert-savepath=<dir>       Output directory
  --ssl-gencert-addtoconfig=true     Patch -c config file after gen
```

Example — start with a specific YAML config, auto-start the server, minimised:
```
mcaster1win.exe -c C:\mcaster1\mcaster1dnas.yaml -s -m
```

#### Auto-Versioned Builds
Every build stamps the git commit hash and branch into the window title at compile time:
```
Mcaster1DNAS v2.5.1-dev.b73b861 — windows-dev
```

#### YAML Configuration on Windows
YAML config (`mcaster1dnas.yaml`) is fully supported. A Windows-specific CRT DLL boundary
crash in libyaml was identified and fixed — the config file is now read into memory by the
exe's own CRT before being handed to `yaml.dll`, eliminating the incompatible `FILE*` struct
layout issue. Both YAML and XML configs start the server identically on Windows.

See **[docs/WINDOWS_GUI.md](docs/WINDOWS_GUI.md)** for the complete Windows GUI user guide.

### Building on Windows

#### Prerequisites

1. **Visual Studio 2022** (Community, Professional, or Enterprise) with:
   - "Desktop development with C++" workload
   - MSVC v143 toolset
   - Windows 10/11 SDK

2. **vcpkg** (package manager) with the following packages installed for `x64-windows`:
   ```
   vcpkg install libxml2 libxslt curl openssl libyaml ogg vorbis theora speex pthreads
   ```
   Set `VCPKG_ROOT` environment variable to your vcpkg install path.

3. **Git** — required for the PreBuildEvent version stamping script.

#### Build Steps

```
# Open in Visual Studio 2022:
windows\Mcaster1DNAS.sln

# Select configuration: Debug | x64  (or Release | x64)
# Build → Build Solution   (or Ctrl+Shift+B)

# Output:
windows\x64\Debug\mcaster1win.exe
```

Or from a Developer Command Prompt / PowerShell:
```powershell
cd windows
msbuild Mcaster1Win.vcxproj /p:Configuration=Debug /p:Platform=x64
```

#### Config Files

Place either `mcaster1.yaml` or `mcaster1.xml` in the same directory as `mcaster1win.exe`.
The server auto-detects the format. Sample configs are included in `windows\x64\Debug\`.

### Windows Roadmap

| Feature | Status |
|---------|--------|
| Resizable GUI (ResizableLib) | **Done** |
| CLI flags (-c -s -m -v -h) | **Done** |
| YAML config support (CRT fix) | **Done** |
| Auto-version stamping | **Done** |
| Mcaster1DNAS visual rebrand | **Done** |
| Real-time log viewer tabs (_SH_DENYNO) | **Done** |
| Uptime clock + system time status bar | **Done** |
| HTTP admin authentication fix (WWW-Authenticate) | **Done** |
| Per-listener SSL enforcement (ssl: true/false) | In Progress |
| SSL cert generation (--ssl-gencert CLI flags) | In Progress |
| Config file auto-discovery + GetOpenFileName fallback | In Progress |
| Config dialog editor (GUI) | Planned |
| Podcast & On-Demand File Manager | Planned |
| RSS Podcast Feed Generator | Planned |
| Windows Installer (Inno Setup) | Planned |
| Windows Service integration | Planned |
| Dark mode (Win10/11) | Planned |

> For full Windows-specific change history see [CHANGELOG-WIN.md](CHANGELOG-WIN.md).

---

## 📚 Documentation

Comprehensive documentation is available:

### Core Documentation
- **[BUILD_AND_RUN.md](BUILD_AND_RUN.md)** - Complete build and deployment guide
- **[ChangeLog](ChangeLog)** - Version history and changes (Linux/macOS)
- **[CHANGELOG-WIN.md](CHANGELOG-WIN.md)** - ⭐ Windows build change history
- **[COPYING](COPYING)** - GNU GPL v2 license text
- **[FORK.md](FORK.md)** - Fork information and lineage

### Feature Guides (`docs/` directory)
- **[docs/SSL_CERT_GENERATION.md](docs/SSL_CERT_GENERATION.md)** - ⭐ Built-in SSL cert/CSR generation (all platforms)
- **[docs/ICY2_PROTOCOL.md](docs/ICY2_PROTOCOL.md)** - ⭐ ICY2 protocol implementation guide (v2.2)
- **[docs/STATIC_MOUNTS.md](docs/STATIC_MOUNTS.md)** - ⭐ Podcast, socialcast, on-demand mount types
- **[docs/SONG_HISTORY_API.md](docs/SONG_HISTORY_API.md)** - ⭐ Song history ring buffer and XML API
- **[docs/WINDOWS_GUI.md](docs/WINDOWS_GUI.md)** - ⭐ Windows GUI user guide (mcaster1win.exe)

### Protocol & Config Reference
- **[ICY2_PROTOCOL_SPEC.md](ICY2_PROTOCOL_SPEC.md)** - ICY-META v2.2 full normative specification
- **[YAML_IMPLEMENTATION.md](YAML_IMPLEMENTATION.md)** - YAML configuration guide
- **[YP_LOGGING_FEATURE.md](YP_LOGGING_FEATURE.md)** - YP directory logging implementation

### UI & Player Documentation
- **[WEBPLAYER_FEATURE.md](WEBPLAYER_FEATURE.md)** - Integrated browser audio player documentation
- **[ENTERPRISE_UI_ENHANCEMENTS.md](ENTERPRISE_UI_ENHANCEMENTS.md)** - UI modernization details
- **[CLOCK_AND_LOADTIME.md](CLOCK_AND_LOADTIME.md)** - Live clock and performance metrics

### Windows Development
- **[CLAUDE-RESUME-WINDOWS-DEV.md](CLAUDE-RESUME-WINDOWS-DEV.md)** - Windows dev branch state, file index, build commands

### Implementation Plans
- **[ICY2_SIMPLIFIED_PLAN.md](ICY2_SIMPLIFIED_PLAN.md)** - ICY2 implementation architecture
- **[PODCAST_PLANNING.md](PODCAST_PLANNING.md)** - Podcast feature roadmap
- **[TODO.md](TODO.md)** - Future features and roadmap

### Online Documentation

Browse the full documentation by opening `doc/index.html` in your browser after installation.

---

## ⚙️ Configuration

### Basic Configuration

Edit the XML configuration file to customize your server:

```xml
<mcaster1>
    <!-- Server identification -->
    <hostname>your-server.com</hostname>
    <location>Your Location</location>
    <admin>admin@your-server.com</admin>

    <!-- Authentication (CHANGE THESE!) -->
    <authentication>
        <source-password>YOUR_SECURE_PASSWORD</source-password>
        <relay-password>YOUR_SECURE_PASSWORD</relay-password>
        <admin-user>admin</admin-user>
        <admin-password>YOUR_SECURE_ADMIN_PASSWORD</admin-password>
    </authentication>

    <!-- Listeners -->
    <listen-socket>
        <port>9330</port>
        <bind-address>0.0.0.0</bind-address>
    </listen-socket>

    <!-- HTTPS/SSL Listener -->
    <listen-socket>
        <port>9443</port>
        <bind-address>0.0.0.0</bind-address>
        <ssl>1</ssl>
    </listen-socket>

    <!-- Paths -->
    <paths>
        <ssl-certificate>/path/to/cert.pem</ssl-certificate>
        <webroot>/path/to/mcaster1dnas/web</webroot>
        <adminroot>/path/to/mcaster1dnas/admin</adminroot>
        <logdir>/var/log/mcaster1dnas</logdir>
    </paths>
</mcaster1>
```

### SSL Certificate

**Option A — Built-in cert generator (all platforms, no separate openssl needed):**

```bash
# Linux / macOS console
./mcaster1 --ssl-gencert --ssl-gentype=selfsigned \
    --subj="/C=US/ST=CA/O=My Station/CN=stream.example.com" \
    --ssl-gencert-savepath=/etc/mcaster1dnas/ssl \
    --ssl-gencert-addtoconfig=true -c mcaster1dnas.yaml
```

```powershell
# Windows console
mcaster1.exe --ssl-gencert --ssl-gentype=selfsigned `
    --subj="/C=US/ST=TX/O=My Station/CN=stream.example.com" `
    --ssl-gencert-savepath=ssl\mycert `
    --ssl-gencert-addtoconfig=true -c windows\mcaster1dnas.yaml
```

**Option B — External openssl:**
```bash
openssl req -x509 -newkey rsa:4096 -nodes \
  -keyout mcaster1dnas.pem \
  -out mcaster1dnas.pem \
  -days 365 \
  -subj "/CN=your-server.com"
```

See **[docs/SSL_CERT_GENERATION.md](docs/SSL_CERT_GENERATION.md)** for full options including CSR
generation for commercial CAs (Let's Encrypt, DigiCert, etc.).

---

## 🎵 Broadcasting to Mcaster1DNAS

### Using OBS Studio

1. Add an audio source
2. Go to Settings → Stream
3. Set Service to "Custom"
4. Server: `icecast://source:YOUR_PASSWORD@your-server:9330/stream.mp3`
5. Click "Start Streaming"

### Using Mixxx

1. Go to Preferences → Live Broadcasting
2. Type: Icecast 2
3. Host: `your-server`
4. Port: `9330`
5. Mount: `/stream.mp3`
6. Login: `source`
7. Password: `YOUR_PASSWORD`
8. Click "Enable Live Broadcasting"

### Using ffmpeg

```bash
# Stream an audio file
ffmpeg -re -i input.mp3 -acodec libmp3lame -ab 128k \
  -f mp3 icecast://source:YOUR_PASSWORD@your-server:9330/stream.mp3

# Stream from microphone
ffmpeg -f alsa -i default -acodec libmp3lame -ab 128k \
  -f mp3 icecast://source:YOUR_PASSWORD@your-server:9330/live.mp3
```

---

## 🛠️ Development

### Building from Source

```bash
# Install development dependencies (Debian/Ubuntu)
sudo apt-get install build-essential autoconf automake libtool \
  libxml2-dev libxslt1-dev libcurl4-openssl-dev libssl-dev \
  libvorbis-dev libogg-dev libtheora-dev libspeex-dev

# Build
./autogen.sh
./configure --prefix=/usr/local/mcaster1dnas
make
sudo make install
```

### Contributing

We welcome contributions! Please:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Code Style

- C code follows K&R style
- Use tabs for indentation (width: 4 spaces)
- Maximum line length: 120 characters
- Add comments for complex logic

---

## 📊 System Requirements

### Minimum Requirements

- **CPU:** 1 GHz single-core processor
- **RAM:** 256 MB
- **Disk:** 50 MB for installation
- **Network:** 100 Mbps for moderate streaming

### Recommended for Production

- **CPU:** 2+ GHz multi-core processor
- **RAM:** 1 GB or more
- **Disk:** 1 GB+ for logs and buffers
- **Network:** 1 Gbps for high-capacity streaming
- **OS:** Linux (Debian, Ubuntu, CentOS, RHEL), BSD, or macOS

### Supported Platforms

- ✅ Linux (Debian, Ubuntu, CentOS, RHEL, Fedora)
- ✅ BSD (FreeBSD, OpenBSD, NetBSD)
- ✅ macOS (10.13+)
- ✅ Windows 10/11 — **native GUI via Visual Studio 2022 (v17)** (`mcaster1win.exe`)

---

## 🔒 Security

### Best Practices

1. **Change Default Passwords** - Immediately change default admin and source passwords
2. **Use HTTPS** - Always use SSL/TLS for admin interface
3. **Firewall Rules** - Restrict admin port access to trusted IPs
4. **Regular Updates** - Keep Mcaster1DNAS and dependencies up to date
5. **Strong Passwords** - Use complex passwords for all accounts
6. **Disable Unused Features** - Comment out unused mount points and relays

### Security Advisories

Check the [GitHub Issues](https://github.com/davestj/mcaster1dnas/issues) page for security advisories and updates.

---

## 📝 License

Mcaster1DNAS is licensed under the **GNU General Public License v2.0** - see the [COPYING](COPYING) file for details.

This ensures that Mcaster1DNAS remains free and open source software, compatible with all upstream projects (Icecast2 and Icecast-KH).

---

## 🙏 Acknowledgments

Mcaster1DNAS stands on the shoulders of giants. We thank:

- **[Xiph.Org Foundation](https://xiph.org/)** - Original Icecast2 development and the foundation of open source streaming
- **[Karl Heyes](https://github.com/karlheyes)** - Icecast-KH improvements, performance enhancements, and maintenance
- **All Icecast contributors** - For decades of development on the foundational streaming server technology
- **[FontAwesome](https://fontawesome.com/)** - Professional icon library used in our web interface

---

## 🔗 Links

- **Website:** [https://mcaster1.com](https://mcaster1.com)
- **GitHub:** [https://github.com/davestj/mcaster1dnas](https://github.com/davestj/mcaster1dnas)
- **Issues:** [https://github.com/davestj/mcaster1dnas/issues](https://github.com/davestj/mcaster1dnas/issues)
- **Discussions:** [https://github.com/davestj/mcaster1dnas/discussions](https://github.com/davestj/mcaster1dnas/discussions)

### Upstream Projects

- **Icecast2:** [https://icecast.org](https://icecast.org)
- **Icecast-KH:** [https://github.com/karlheyes/icecast-kh](https://github.com/karlheyes/icecast-kh)

---

## 📧 Contact

- **Maintainer:** Saint John (David St John)
- **Email:** [davestj@gmail.com](mailto:davestj@gmail.com)
- **Website:** [https://mcaster1.com](https://mcaster1.com)

---

## 🌟 Features Roadmap

### Current Version (2.5.1-beta.2-win)
✅ Modern HTML5/CSS3 web interface with shared header/footer templates
✅ Interactive help tooltips
✅ Live clock and page load metrics
✅ HTTPS/SSL by default
✅ Enhanced admin dashboard
✅ Credits and fork information pages
✅ ICY-META v2.2 extended metadata protocol — 50+ fields, static mount support
✅ YAML configuration support (alongside XML)
✅ Per-listener `ssl:` enforcement — `ssl: true` (TLS only) or `ssl: false` (plain HTTP only)
✅ Built-in SSL cert generator — `--ssl-gencert` CLI flags, all platforms
✅ Song History API — in-memory ring buffer at `/mcaster1songdata`
✅ Track History pages — admin + public with music service lookup icons
✅ Static mount types — podcast, socialcast, on-demand (alongside live)
✅ Integrated browser audio player — VU meters, volume control, keyboard shortcuts
✅ Full public codec stats — bitrate, samplerate, channels, codec for all formats
✅ Opus ICY metadata support — now playing, artist/title, song history tracking
✅ Windows native GUI (`mcaster1win.exe`) — Visual Studio 2022 (v17), fully resizable via ResizableLib
✅ Windows YAML config CRT crash fixed — in-memory parse, no DLL FILE* boundary crossing
✅ Windows auto-versioning — git commit hash stamped into window title at build time
✅ Windows uptime clock + system time status bar — real-time display in GUI
✅ Windows real-time log viewer — color-coded, shared-mode file reading (_SH_DENYNO)
✅ Windows HTTP admin auth fixed — WWW-Authenticate header now correctly sent (MSVC struct bug fix)
✅ Complete Mcaster1DNAS visual rebrand — new bitmaps, icons, installer branding

### Upcoming Features (Cross-Platform)
- [ ] Real-time statistics dashboard (WebSocket/SSE)
- [ ] Dark mode toggle
- [ ] Stream health monitoring with visual indicators
- [ ] Historical listener analytics with charts
- [ ] Mobile app integration
- [ ] Bulk operations for listener management
- [ ] Automated stream scheduling system
- [ ] Interactive API documentation
- [ ] Plugin architecture for extensibility

### Upcoming Features (Windows GUI)
- [ ] **Config Dialog Editor** — visual YAML/XML editor built into the GUI
- [ ] **Enhanced Windows documentation** — dependency setup guide, config wizard
- [ ] **Podcast & On-Demand File Manager** — manage episode files and on-demand assets from the GUI
- [ ] **RSS Podcast Feed Generator** — built-in RSS 2.0 / Apple Podcasts feed publisher
- [ ] **Podcast core server features** — episode scheduling, chapter markers, per-episode listener analytics
- [ ] **Windows Installer (Inno Setup / MSI)** — automated installer with DLL bundling
- [ ] **Windows Service integration** — install/start/stop as a Windows Service from the GUI
- [ ] **Dark mode** — Windows 10/11 dark mode support

---

## 📈 Statistics

![GitHub stars](https://img.shields.io/github/stars/davestj/mcaster1dnas?style=social)
![GitHub forks](https://img.shields.io/github/forks/davestj/mcaster1dnas?style=social)
![GitHub watchers](https://img.shields.io/github/watchers/davestj/mcaster1dnas?style=social)

---

<div align="center">

**Made with ❤️ by the MediaCast1 Team**

**GNU GPL v2 Licensed** | **Free and Open Source Forever**

[⬆ Back to Top](#mcaster1dnas---digital-network-audio-server)

</div>

# Changelog

All notable changes to **Mcaster1DNAS** are documented in this file.

Format follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
Versions follow [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [2.5.3-sec] - 2026-04-11

### Security

#### CRITICAL Fixes
- **Constant-time password comparison** (`src/connection.c`) -- replaced `strcmp()` with
  `CRYPTO_memcmp()` on all three password check functions to prevent timing side-channel
  attacks (CWE-208)
- **Buffer overflow in ssl_gen.c** (`src/ssl_gen.c`) -- replaced unbounded `strcat()` with
  tracked-length `memcpy()` in `patch_yaml_config()` and `patch_xml_config()` (CWE-120)
- **Command injection hardening** (`src/source.c`) -- added NULL-termination, malloc failure
  handling, and explicit cleanup in `execvp` event script execution (CWE-78)
- **Config file permissions** -- `mcaster1-production.yaml` changed from 0644 to 0640 to
  prevent credential exposure to other system users
- **Admin over HTTP blocked** (`src/admin.c`) -- admin endpoints now return 403 on non-SSL
  connections, preventing cleartext credential transmission (CWE-319)

#### HIGH Fixes
- **HSTS header** (`src/params.c`) -- added `Strict-Transport-Security: max-age=31536000;
  includeSubDomains` to all HTTPS responses
- **Binary hardening** -- rebuilt with `-fstack-protector-strong`, `-D_FORTIFY_SOURCE=2`,
  `-Wformat -Wformat-security`, `-Wl,-z,relro,-z,now` (full RELRO)
- **Unchecked realloc** (`src/connection.c`) -- added NULL return check before overwriting
  pointer in `connection_bufs_append()` (CWE-690)
- **EBML NULL dereference** (`src/format_ebml.c`) -- added NULL checks on all four `calloc()`
  returns in `ebml_create()`, including the 1MB header allocation (CWE-476)
- **TOCTOU race** (`src/auth_htpasswd.c`) -- replaced `stat()+fopen()` with atomic
  `open(O_CREAT|O_EXCL)` to prevent symlink attacks (CWE-367)

#### MEDIUM Fixes
- **X.509 serial number** (`src/ssl_gen.c`) -- replaced hardcoded serial `1` with random
  128-bit value via `BN_rand()` per RFC 5280 (CWE-330)
- **PEM file permissions** (`src/ssl_gen.c`) -- new `fopen_secure()` function creates private
  key files with mode 0600 via `open(O_CREAT, 0600)` (CWE-732)
- **alloca stack overflow** (`src/xslt.c`) -- replaced `alloca()` with bounded `malloc()` and
  8192-byte length cap on query parameters (CWE-770)
- **Integer truncation** (`src/global.c`) -- changed `int len = strlen(s)` to `size_t` in
  `my_strdup()` to prevent wrap on large strings (CWE-190)
- **calloc NULL checks** (`src/global.c`) -- added NULL checks on both allocation paths in
  `my_calloc()` to prevent NULL dereference on memory pressure

#### LOW Fixes
- **Server version disclosure** (`src/params.c`, `src/cfgfile.c`) -- stripped version number
  from `Server:` header (now just `Mcaster1DNAS`) and `WWW-Authenticate` realm
- **X-Frame-Options** (`src/client.c`) -- added `X-Frame-Options: SAMEORIGIN` to admin 401
  responses to prevent clickjacking
- **Referrer-Policy** (`src/params.c`) -- added `strict-origin-when-cross-origin` header
- **Debug trace files** (`src/main.c`, `src/cfgfile.c`, `src/cfgfile_yaml.c`) -- guarded
  `mcaster1win_start.log` trace macros behind `#if defined(WIN32) && defined(_DEBUG)`

### Changed
- Version bumped to 2.5.3-sec
- Default `server_id` no longer includes version string

---

## [2.5.2-dev] - 2026-03-31

### Added
- VP9/WebM video streaming support (end-to-end verified)
- `FORMAT_FL_VIDEO` flag for video content-type detection
- Cross-boundary EBML Cluster ID detection
- Video queue/burst sizing (512KB queue, 128KB burst floor)
- Source timeout fix for video streams (`last_read` update on data consumption)

---

## [2.5.1-rc1] - 2026-02-18

### Added

#### Song History API (`src/songdata_api.c`, `src/songdata_api.h`)
- In-memory ring buffer storing the last N played tracks (default: 25; 0 = unlimited)
- Configurable via `song-history-limit` in YAML or XML config
- Deduplication: consecutive identical titles on the same mount are silently ignored
- Back-fills `ended_at` timestamp when the next track starts for accurate time-on-air
- Exposed as machine-readable XML at `/admin/songdata` (admin-authenticated)
- Public (no-auth) version served via `/songdata.xsl` intercept in `stats.c`

#### Track History Pages (`admin/songdata.xsl`, `web/songdata.xsl`)
- Rich table showing recently played tracks across all mount points
- Columns: #, Mount, Artist / Title, Quality badge (codec / kbps / kHz / stereo|mono),
  Played At, On-Air duration (live counter: Xm Ys / Xh Ym), Listeners, Lookup
- Row #1 highlighted with animated pulsing green "Now Playing" dot
- Four one-click music service lookup icons per row:
  - 🟠 **MusicBrainz** — open music encyclopedia
  - 🔴 **Last.fm** — scrobbling and artist radio
  - ⚫ **Discogs** — release and label database
  - 🟢 **AllMusic** — editorial reviews and discographies
- URL query strings built entirely in XSLT 1.0 (`translate(concat(...))`)
- Admin version at `/admin/songdata.xsl`; public version at `/songdata.xsl`

#### Integrated Browser Audio Player (`admin/webplayer.xsl`, `web/webplayer.xsl`)
- Full HTML5 audio player built into the admin and public web interfaces
- Real-time metadata polling every 5 seconds via the JSON status API
- VU meters implemented with the Web Audio API (AnalyserNode)
- Volume control with `localStorage` persistence across sessions
- Responsive design — works on desktop and mobile
- Keyboard shortcuts: `Space` = play/pause, `Esc` = stop
- Player button added to stats, listmounts, and public status pages
- Bookmarkable player window (`/admin/webplayer.xsl?mount=/your-mount`)

#### Opus ICY Metadata & Now Playing (`src/format_opus.c`)
- Added `opus_set_tag()` to capture ICY `StreamTitle` from Opus source connections
- Calls `logging_playlist()` on tag commit so Opus tracks appear in the song history
  (previously Opus streams produced zero song-change events)
- Parses `OpusHead` packet to expose `audio_channels` and `audio_samplerate` stats
- `audio_codecid` stat exposed for Opus streams (value: `opus`)

#### Full Public Codec Stats (`src/format_mp3.c`)
- `mpeg_samplerate`, `mpeg_channels`, and `audio_codecid` promoted from
  `STATS_HIDDEN` → `STATS_COUNTERS` so public status pages and third-party clients
  can read them without admin authentication

#### Shared Header / Footer XSL Templates
- `admin/header.xsl` — named template `admin-header`; `active-page` param
  accepts: `stats`, `mounts`, `relays`, `logs`, `songdata`, `credits`
- `admin/footer.xsl` — named template `admin-footer`; fixed toolbar at bottom
- `web/header.xsl` — named template `web-header`; `active-page` param
  accepts: `status`, `songdata`, `version`, `credits`
- `web/footer.xsl` — named template `web-footer`
- All admin and public pages converted to use shared include templates for
  consistent navigation, branding, and styling

#### Windows Build Support
- Added MSBuild project files for Visual Studio 2022 (`windows/` directory)
- `windows/Makefile` provides `nmake` targets for MSVC builds
- CI pipeline extended with Windows build job

#### YAML Configuration Enhancements
- `song-history-limit` key added under `limits:` block (YAML) / `<song-history-limit>` (XML)
- Production config now uses domain name `dnas.mcaster1.com` instead of bare IP
- `workers` increased to 4 to match CPU count on the reference server
- `webroot` and `adminroot` now point directly to source directories (no copy step)

### Changed

- **Admin command routing** (`src/admin.c`): `command_list_mounts` now calls
  `stats_get_xml(STATS_ALL, ...)` so `listmounts.xsl` receives all 30+ metadata
  fields (previously used `admin_build_sourcelist()` which omitted codec data)
- **Public XSL routing** (`src/stats.c`): Added intercept for `/songdata.xsl`
  URI to serve `songdata_get_xml()` instead of the standard `mcaster1stats` XML,
  enabling a no-auth public Track History page
- **Admin header nav**: Track History link added between Logs and Credits
- **Web header nav**: Track History link added between Server Info and Credits
- **Footer toolbars**: Track History shortcut added to both admin and web footers

### Fixed

- Metadata display in `listmounts.xsl` was showing "No metadata available"
  despite correct XSLT; root cause was the admin command returning a minimal
  XML document — fixed by switching to full stats XML source
- Footer online/offline indicator correctly reflects live mount state using
  `number(sources) > 0` test against the root XML element

### Repository Hygiene

- **`.gitignore` expanded** to cover:
  - `build/` — `make install` output (auto-generated; not source)
  - `ssl/`, `*.pem`, `*.crt`, `*.key` — SSL certificates and private keys
  - `/logs/` and `build/logs/` — runtime log files
  - `mcaster1-production.yaml` / `.xml` — production configs with credentials
  - `Makefile` (exact match) — `./configure` output; `Makefile.am` / `Makefile.in` unaffected
  - `/config.sub`, `/config.h.in` — autotools-generated files
  - `test-*.yaml`, `YAML_TEST_REPORT.txt` — CI test artifacts
- **Removed from git tracking** (files remain on disk locally):
  - All 50+ files under `build/`
  - `ssl/` directory (including private key — rotate if repo goes public)
  - Root-level `logs/access.log` and `logs/error.log`
  - `mcaster1-production.yaml` and `mcaster1-production.xml`
  - All generated `Makefile` files (14 files across subdirectories)
  - `config.h.in`, `config.sub`

---

## [2.5.0] - 2026-02-15

### Added

- **ICY-META v2.1+ Protocol** — Extended metadata with 29+ fields across 6 categories
  (Core, Podcast, Video, Social Media, Access Control, Verification)
- **YAML Configuration** — Full-featured YAML config support alongside traditional XML;
  auto-detects format on startup; four template files included
- **Modern Web Interface** — Complete HTML5/CSS3 redesign with FontAwesome 6.4.0 icons
- **Interactive Help System** — Hover-activated tooltips explaining every UI element
- **Live Clock** — Real-time clock and date display in header (updates every second)
- **Page Load Metrics** — Performance tracking showing page load times in footer
- **Responsive Design** — Mobile-friendly layout for all admin and public pages
- **Credits Page** — Comprehensive fork history and upstream acknowledgments
- **YP Connection Logging** — Detailed Yellow Pages directory connection logs
- **Max Listeners in Stats** — Server-configured `max_listeners` exposed in global XML

### Changed

- Admin dashboard redesigned with card-based layout and visual metrics
- Public status page redesigned to match new brand identity
- Server binary renamed from `icecast` / `icecast2` to `mcaster1`
- Default ports changed to 9330 (HTTP) and 9443 (HTTPS)
- Project rebranded from Icecast-KH to **Mcaster1DNAS** (Digital Network Audio Server)

---

## [Upstream: Icecast-KH] — Prior to 2.5.0

Mcaster1DNAS is a fork of [Icecast-KH](https://github.com/karlheyes/icecast-kh) by Karl Heyes,
which is itself a fork of [Icecast2](https://icecast.org/) by the Xiph.Org Foundation.

All prior changes are documented in the upstream projects:
- [Icecast-KH ChangeLog](https://github.com/karlheyes/icecast-kh/blob/master/ChangeLog)
- [Icecast2 ChangeLog](https://gitlab.xiph.org/xiph/icecast-server/-/blob/master/ChangeLog)

---

*Mcaster1DNAS is licensed under the GNU General Public License v2.0.*
*See [COPYING](COPYING) for the full license text.*

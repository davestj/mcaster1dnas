# Mcaster1DNAS - News & Release Notes

Official release announcements and changelog for Mcaster1DNAS.

---

## Version 2.5.0 - "Enterprise Launch" (February 14, 2026)

**First official release of Mcaster1DNAS fork!** 🎉

### 🎯 Major Features

#### Complete Rebranding
- Forked from Icecast-KH to create Mcaster1DNAS
- Complete rename of all binaries, configs, and documentation
- New branding: **Mcaster1DNAS** (Digital Network Audio Server)
- MediaCast1 ecosystem integration

#### Modern Web Interface
- **Complete HTML5/CSS3 redesign** with professional UI
- **FontAwesome 6.4.0 icons** throughout admin and public interfaces
- **Responsive design** - Mobile-friendly on all devices
- **Modern card-based layout** with gradients and shadows
- **Enhanced color scheme** - Mcaster1 blue + DNAS green branding

#### Interactive Help System
- **Hover-activated tooltips** explaining every feature
- **Info boxes** with streaming concepts explained
- **"What are Mount Points?"** educational content
- **"What is Metadata?"** explanations for users
- **Help icons (?)** throughout the interface

#### Live Clock & Metrics
- **Real-time clock** in header (updates every second)
- **Full date display** - Day, Month, Date, Year
- **Page load time** metrics in footer
- **Performance tracking** using Performance API
- **Pulsing clock icon** for visual appeal

#### Enhanced Admin Dashboard
- **Real-time statistics** - Listeners, Sources, Bandwidth, Clients
- **Mount point management** - Easy stream control
- **Listener management** - View, kick, move listeners
- **Metadata updates** - Simple "Now Playing" editor
- **User authentication** - Manage stream access
- **Relay management** - Configure stream relays
- **Log viewer** - Real-time access/error/playlist logs

#### Credits & Attribution
- **Credits pages** (admin + public) with complete fork history
- **Project lineage** - Icecast2 → Icecast-KH → Mcaster1DNAS
- **Upstream acknowledgments** - Xiph.Org, Karl Heyes, contributors
- **License information** - GNU GPL v2 compliance

#### Comprehensive Documentation
- **README.md** with 15+ GitHub badges
- **BUILD_AND_RUN.md** - Complete build guide
- **FORK.md** - Fork information and lineage
- **ENTERPRISE_UI_ENHANCEMENTS.md** - UI modernization details
- **CLOCK_AND_LOADTIME.md** - Live clock implementation
- **AUTHORS.md** - Complete contributor attribution
- **CONTRIBUTING.md** - Developer guidelines
- **TODO.md** - Project roadmap

### 🔒 Security & Configuration

- **HTTPS/SSL by default** - Ports 9330 (HTTP) and 9443 (HTTPS)
- **Modern SSL configuration** - Strong ciphers and protocols
- **Production-ready config** - mcaster1-production.xml template
- **Secure defaults** - Password protection, authentication

### 🛠️ Build System Improvements

- **build.sh** - Automated build script (6 steps)
- **mcaster1-control.sh** - Server management (start/stop/restart/status/logs)
- **Modern autotools** - Updated configure.ac and Makefile.am
- **Visual Studio 2022 projects** - Windows build support

### 🎨 Visual Enhancements

- **Gradient header** - Professional branding
- **Status badges** - LIVE indicators with animations
- **Format badges** - MP3, AAC, Ogg, Opus, FLAC color-coded
- **Stat grids** - Large numbers with icons
- **Professional tables** - Clean, modern table styling
- **Smooth animations** - Pulse, fade, slide effects

### 📊 Statistics & Monitoring

- **Global server stats** - Total listeners, sources, bandwidth
- **Per-mount statistics** - Individual stream metrics
- **Connection tracking** - Client, source, listener counts
- **Bandwidth monitoring** - Real-time bitrate display
- **Uptime tracking** - Server start time and duration

### 🌐 Navigation & UX

- **Unified navigation** - Consistent across all pages
- **Admin navigation** - Stats | Mounts | Relays | Logs | Credits | Public
- **Public navigation** - Status | Server Info | Credits | Admin
- **Active page indicators** - Highlighted current page
- **Back-to-top links** - Easy navigation on long pages

### 📝 Additional Features

- **Streaming format explanations** - Help users understand codecs
- **Mount point guidance** - Educational content for broadcasters
- **Broadcasting examples** - OBS, Mixxx, ffmpeg instructions
- **Responsive footer** - Clean, informative page footers
- **Professional error pages** - Branded error messages

### 🔄 Inherited from Icecast-KH

All features from Icecast-KH 2.4.0.kh22:
- Worker thread implementation (1-N threads)
- Advanced authentication system
- Relay and mount wildcards
- Bandwidth limiting
- HTTP 302 redirects for max listeners
- FLV wrapping for MP3/AAC
- Enhanced logging
- URL authenticator handlers
- Multiple ICY sources on single port

### 📦 Package Information

- **Version:** 2.5.0
- **Codename:** "Enterprise Launch"
- **Release Date:** February 14, 2026
- **License:** GNU GPL v2
- **Platforms:** Linux, BSD, macOS, Windows
- **Build:** 20260214173327

### 🙏 Credits

Special thanks to:
- **Karl Heyes** - Icecast-KH foundation
- **Xiph.Org Foundation** - Icecast2 and open codecs
- **Jack Montgomery (Monty)** - Original Icecast creator
- **All contributors** - Decades of streaming server development

### 📋 Breaking Changes

None - Fully compatible with Icecast-KH configurations (after renaming).

### 🔗 Resources

- **Website:** https://mcaster1.com
- **Repository:** https://github.com/davestj/mcaster1dnas
- **Documentation:** See doc/index.html
- **Issues:** https://github.com/davestj/mcaster1dnas/issues

---

## Previous Releases

For Icecast-KH release history, see [NEWS.archive](NEWS.archive)

---

## Upcoming Releases

### Version 2.6.0 (Planned Q2 2026)

**Focus:** UI/UX Enhancements

Planned features:
- Dark mode toggle
- Real-time dashboard updates (WebSocket/SSE)
- Enhanced stream cards with visualizations
- Listener analytics dashboard
- Mobile app deep linking
- Accessibility improvements (WCAG 2.1 AA)

See [TODO.md](TODO.md) for complete roadmap.

---

## Stay Updated

- **GitHub:** Watch the repository for updates
- **Website:** https://mcaster1.com for announcements
- **Email:** davestj@gmail.com for direct contact

---

**Maintained by:** David St John (Saint John)
**License:** GNU GPL v2
**Project:** Mcaster1DNAS - Digital Network Audio Server

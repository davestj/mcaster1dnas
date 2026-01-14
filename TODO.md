# Mcaster1DNAS - TODO & Roadmap

Features, improvements, and tasks planned for future releases.

---

## 🎯 Current Version: 2.5.1-rc1 (Pre-Release)

### 🧪 In Testing - v2.6.0 Features (Alpha/RC1)

**ICY-META v2.1+ Protocol:**
- [x] Complete ICY-META v2.1+ protocol implementation
- [x] Auto-detection via icy-metadata-version header
- [x] Seamless failover to ICY 1.x legacy support
- [x] 29+ metadata fields (core, podcast, video, social, access control, verification)
- [x] Dedicated icy2_meta.c/h module for clean integration
- [x] Stats system integration for admin interface display
- [x] Complete protocol specification (ICY2_PROTOCOL_SPEC.md)
- [x] curl testing and validation
- [ ] **Needs testing with Mcaster1Encoder and TagStack clients**

**HTTPS Source Support:**
- [x] SSL/TLS auto-detection for source encoders
- [x] HTTPS source connections on any port
- [x] Compatible with FFmpeg, Liquidsoap, Butt, OBS plugins
- [ ] **Needs production testing**

**Build System:**
- [x] ICY2 module integrated into Makefile.am
- [x] Successful compilation and installation

### ✅ Completed in 2.5.1-rc1 (February 15, 2026)

**Web Player Feature:**
- [x] Full-featured browser-based audio player
- [x] Real-time metadata polling (updates every 5 seconds)
- [x] VU meters with Web Audio API visualization
- [x] Volume control with localStorage persistence
- [x] Responsive design for desktop and mobile
- [x] Keyboard shortcuts (Space = play/pause, Esc = stop)
- [x] Bookmarkable player windows for easy station access
- [x] Player buttons integrated into stats.xsl, listmounts.xsl, status.xsl
- [x] Admin player at `/admin/webplayer.xsl?mount=/stream.mp3`
- [x] Public player at `/webplayer.xsl?mount=/stream.mp3`
- [x] Complete documentation (WEBPLAYER_FEATURE.md)
- [x] XSLT endpoint registration in src/admin.c

**Metadata Display Fix:**
- [x] Fixed listmounts.xsl showing "No metadata available"
- [x] Changed command_list_mounts to use stats_get_xml(STATS_ALL)
- [x] Now displays all 30+ metadata fields correctly

### ✅ Completed in 2.5.0 (February 14, 2026)

**YAML Configuration:**
- [x] Full YAML configuration file support
- [x] Automatic format detection (XML vs YAML)
- [x] 100% feature parity with XML
- [x] Four comprehensive YAML templates
- [x] Complete documentation (YAML_IMPLEMENTATION.md)

**YP Connection Logging:**
- [x] Per-YP-server logging capability
- [x] Timestamped transaction logs (add/remove/touch)
- [x] Optional yp-logfile parameter
- [x] Admin interface YP log display tab

**UI/UX Enhancements:**
- [x] Tabbed logs interface (4 tabs: Access, Error, Playlist, YP)
- [x] Session persistence for active tab
- [x] Fixed admin page XML parsing errors
- [x] Live clock on all admin pages

**Core Features:**
- [x] Complete rebrand from Icecast-KH to Mcaster1DNAS
- [x] Modern HTML5/CSS3 web interface
- [x] FontAwesome 6.4.0 icon integration
- [x] Interactive help tooltip system
- [x] HTTPS/SSL by default (ports 9330 HTTP, 9443 HTTPS)
- [x] Enhanced admin dashboard with real-time stats
- [x] Professional navigation system
- [x] Comprehensive documentation

---

## 📅 Version 2.6.1 - Podcast Features (Next Release)

**Target:** Q2 2026
**Focus:** Empowering podcasters and podcast hosting companies

### Phase 1: Basic RSS Generation (Week 1-2)
- [ ] **Podcast Directory Scanning** - Auto-detect audio files in configured directory
  - Support for .mp3, .ogg, .m4a file formats
  - Recursive directory scanning
  - Modification date sorting (newest first)
  - File change detection and auto-refresh

- [ ] **Audio Metadata Parsing** - Extract ID3/Vorbis tags from audio files
  - ID3v2 tag parsing for MP3 files (libid3tag)
  - Vorbis comment parsing for .ogg files (libvorbisfile)
  - AAC metadata for .m4a files (libavformat optional)
  - Extract: title, artist, album, duration, cover art
  - Duration detection for all formats

- [ ] **RSS 2.0 Feed Generation** - Auto-generate podcast feeds
  - Valid RSS 2.0 format with iTunes podcast tags
  - Episode-level metadata (enclosure, duration, description)
  - Show-level metadata (title, description, author, category)
  - Auto-calculate file sizes and URLs
  - W3C Feed Validator compliance

- [ ] **Podcast Configuration Block** - YAML and XML config support
  - `<podcasting>` block in XML config
  - `podcasting:` block in YAML config
  - Configure: media directory, RSS path, show metadata
  - Auto-generate RSS option
  - Show title, description, author, category, language, cover art

- [ ] **Core Podcast Module** - New source files
  - `src/podcasting.c` - Core podcast management
  - `src/podcasting.h` - Public API and structures
  - `src/id3_parser.c` - Audio metadata parser
  - `src/id3_parser.h` - Parser interface
  - Integration into Makefile.am build system
  - Unit tests for metadata parsing

### Phase 2: Web UI for Podcast Management (Week 3-4)
- [ ] **Admin Podcast Page** - Episode management interface
  - `admin/podcasts.xsl` - Podcast admin page
  - Table view of all episodes (title, duration, size, format, date)
  - Download links for each episode
  - Play buttons (integrate with webplayer.xsl)
  - RSS feed link with copy-to-clipboard
  - Statistics: total episodes, storage used, latest episode

- [ ] **Public Podcast Page** - Public-facing episode listing
  - `web/podcasts.xsl` - Public podcast page
  - Episode listing with embedded players
  - Subscribe buttons (Apple Podcasts, Spotify, RSS)
  - Show notes display (from .txt sidecar files)
  - Responsive mobile design

- [ ] **Episode Details Modal** - Full metadata display
  - All ID3/Vorbis tags shown
  - Cover art preview
  - Technical info (bitrate, sample rate, channels)
  - Download counter (optional)
  - File path and size

- [ ] **Admin Command Registration** - Register podcasts.xsl endpoint
  - Add `command_podcast_list()` to src/admin.c
  - XML response format: `<mcaster1podcasts>`
  - Episode data structure in XML
  - RSS validation status

### Phase 3: Live Stream Recording (Week 5-6)
- [ ] **Recording Engine** - Capture live streams to files
  - Hook into format.c stream handlers
  - Write stream data to file while broadcasting
  - Handle source disconnects gracefully
  - Atomic file writes (temp → rename on complete)
  - Support for MP3 and Ogg Vorbis recording

- [ ] **Per-Mount Recording Config** - Configure recording per mount point
  - `podcast-recording:` config block in mount settings
  - Enable/disable recording per mount
  - Auto-publish to podcast directory option
  - Output directory and filename pattern
  - Episode title and description templates
  - Max/min duration settings

- [ ] **Auto-Publishing Workflow** - Publish recorded streams as episodes
  - Validate recorded file after completion
  - Extract metadata using id3_parser
  - Move to podcast directory
  - Update RSS feed automatically
  - Notification system (log entry or webhook)

- [ ] **Metadata Injection** - Embed metadata in recorded files
  - Capture ICY metadata during recording
  - Embed as ID3 tags in MP3 files
  - Create show notes from metadata log
  - Extract chapter markers from metadata changes

- [ ] **Recording Controls** - Admin UI for manual recording
  - Start/Stop recording buttons in stats.xsl
  - Recording status indicator (red dot when active)
  - Recording duration counter
  - Estimated file size display
  - Link to recorded episode once published

- [ ] **Hybrid Mount Points (KILLER FEATURE!)** - Live + on-demand fallback
  - New mount type: `hybrid`
  - Stream live source when connected
  - Fallback to podcast episode when disconnected
  - Play modes: latest, random, sequential
  - Loop option for continuous playback
  - Seamless transitions between live and on-demand

### Documentation
- [ ] **PODCAST_QUICKSTART.md** - 5-minute setup guide
- [ ] **PODCAST_WEB_UI.md** - Admin interface walkthrough
- [ ] **PODCAST_RECORDING.md** - Live recording setup guide
- [ ] **PODCAST_API.md** - Developer API reference
- [ ] Update main README.md with podcast features

### Testing
- [ ] Test RSS feed in Apple Podcasts, Spotify, Pocket Casts
- [ ] W3C Feed Validator compliance
- [ ] Test recording with various stream formats
- [ ] Test hybrid mode transitions (live ↔ podcast)
- [ ] Load testing with 100+ episodes
- [ ] Mobile browser compatibility

---

## 📅 Version 2.6.2 - Security & Authentication

**Target:** Q3 2026
**Focus:** Enhanced security and authentication methods

### High Priority
- [ ] **SSH Key Authentication for ICY2** - Public/private key auth for HTTPS
  - Client-side key pair generation
  - Server-side public key configuration
  - Works alongside username/password
  - Specifically for ICY2 client authentication
  - OpenSSL integration for key verification

- [ ] **ICY2 Metadata Validation** - Input validation and sanitization
  - URL format validation
  - Language code validation (ISO 639-1)
  - Bitrate range checking
  - Field length limits
  - XSS prevention for emoji/hashtags

- [ ] **ICY2 Admin UI Display** - Show ICY2 metadata in web interface
  - Enhanced mount point cards with ICY2 fields
  - Podcast metadata display
  - Video stream indicators
  - Social media links
  - Content flags (NSFW, AI-generated)

### Medium Priority
- [ ] **ICY2 Config Overrides** - Optional config-based metadata defaults
  - Global ICY2 field defaults in config
  - Per-mount ICY2 overrides
  - Encoder values take precedence
  - Useful for managed hosting environments

- [ ] **ICY2 Rate Limiting** - Prevent metadata update abuse
  - Limit metadata update frequency
  - Per-mount and global limits
  - Configurable thresholds

### Low Priority
- [ ] **Dynamic Metadata Updates (ICY 2.2)** - Update metadata during stream
  - Send metadata updates without reconnecting
  - HTTP POST endpoint for metadata changes
  - Authentication required for updates
  - Backward compatible with ICY 2.1

---

## 📅 Version 2.7.0 - UI/UX Enhancements

**Target:** Q4 2026
**Focus:** User experience and interface improvements

### High Priority
- [ ] **Dark mode toggle** - User preference for light/dark theme
  - CSS variables for theme switching
  - LocalStorage persistence
  - Toggle button in header
  - Smooth transitions

- [ ] **Real-time dashboard updates** - WebSocket or Server-Sent Events
  - Live listener count updates
  - Real-time bandwidth graphs
  - Connection/disconnection notifications
  - No page refresh needed

- [ ] **Enhanced stream cards** - Better visual presentation
  - Album art/cover image support
  - Waveform visualization
  - Bitrate quality indicators
  - Format badges (MP3, AAC, etc.)

### Medium Priority
- [ ] **Listener analytics dashboard** - Historical data visualization
  - Charts.js integration
  - Daily/weekly/monthly listener graphs
  - Peak times visualization
  - Geographic distribution (if available)

- [ ] **Mobile app deep linking** - Better mobile integration
  - Custom URL schemes (mcaster1dnas://)
  - Stream sharing buttons
  - QR code generation for streams
  - Mobile-optimized player pages

- [ ] **Accessibility improvements** - WCAG 2.1 AA compliance
  - Screen reader optimization
  - Keyboard navigation
  - ARIA labels throughout
  - High contrast mode

### Low Priority
- [ ] **Customizable admin dashboard** - Widget-based layout
  - Drag-and-drop widgets
  - Save user preferences
  - Custom metric displays

---

## 📅 Version 2.7.5 - Feature Enhancements

**Target:** Q4 2026
**Focus:** New functionality and capabilities

### High Priority
- [ ] **Stream preview with embedded players** - Test before you broadcast
  - HTML5 audio player integration
  - HLS/DASH support for modern browsers
  - Multiple format playback
  - Volume and playback controls

- [ ] **Bulk listener operations** - Manage multiple listeners
  - Multi-select interface
  - Bulk kick/ban functionality
  - Mass migration between mounts
  - Export listener lists

- [ ] **Advanced playlist management** - Web-based playlist editor
  - Drag-and-drop ordering
  - Upload audio files
  - Scheduled playlist switching
  - Loop/shuffle modes

### Medium Priority
- [ ] **Stream scheduling system** - Automated stream management
  - Time-based stream switching
  - Automated relay fallback
  - Scheduled mount point activation
  - Cron-like scheduling interface

- [ ] **Multi-language support** - Internationalization (i18n)
  - English (default)
  - Spanish
  - French
  - German
  - Language selector in UI

- [ ] **API documentation interface** - Interactive API explorer
  - Swagger/OpenAPI spec
  - Try-it-now functionality
  - Code examples in multiple languages
  - Authentication testing

### Low Priority
- [ ] **Plugin system** - Extensibility framework
  - Hook API for custom code
  - Plugin discovery and management
  - Example plugins (auth, metadata, etc.)
  - Plugin marketplace

---

## 📅 Version 2.8.0 - Performance & Scalability

**Target:** Q1 2027
**Focus:** Speed, efficiency, and scale

### High Priority
- [ ] **Performance optimizations** - Speed improvements
  - Memory usage reduction
  - CPU optimization for high concurrency
  - I/O buffering improvements
  - Connection pooling

- [ ] **Improved caching** - Faster response times
  - Metadata caching
  - Static file caching
  - CDN integration support
  - Cache invalidation strategies

- [ ] **Database integration** - Optional persistent storage
  - Listener history database
  - Statistics archival
  - User database (SQLite/PostgreSQL)
  - Query interface for analytics

### Medium Priority
- [ ] **Load balancing support** - High availability setup
  - Built-in load balancer awareness
  - Health check endpoints
  - Graceful degradation
  - Cluster management

- [ ] **Enhanced relay system** - Better stream distribution
  - Automatic relay failover
  - Relay health monitoring
  - Bandwidth-aware relay selection
  - Master-slave sync improvements

- [ ] **HTTP/2 and HTTP/3 support** - Modern protocols
  - Server push for metadata
  - Multiplexing support
  - Reduced latency

---

## 🌐 Mcaster1 Ecosystem Development

### 🎙️ Mcaster1Encoder (DSP Encoder Platform)

**Status:** Alpha Development - Q1-Q4 2026
**Repository:** https://github.com/davestj/mcaster1encoder

#### Phase 1: Core Architecture (Q1 2026)
- [ ] Basic DSP encoder functionality
- [ ] Live audio capture from sound cards
- [ ] File source support (MP3, Ogg Vorbis, WAV)
- [ ] Initial integration with Mcaster1DNAS using ICY protocol
- [ ] Integration with SAM Broadcaster, Winamp, Liquidsoap

#### Phase 2: Podcast Automation (Q2 2026)
- [ ] Podcast show creation interface
- [ ] Folder-based content management
- [ ] Automated playlist generation
- [ ] Integration with Mcaster1 TagStack for metadata management
- [ ] Episode scheduling and rotation

#### Phase 3: Twitch Integration (Q3 2026)
- [ ] **Twitch stream rebroadcasting**
- [ ] **Relay Twitch streams to Mcaster1DNAS**
- [ ] Multi-platform streaming (Twitch + Mcaster1DNAS simultaneously)
- [ ] Real-time chat integration
- [ ] Twitch API integration for stream metadata

#### Phase 4: Video Support (Q4 2026)
- [ ] Full video encoding support (MP4, Ogg Theora, WebM)
- [ ] Video podcast automation
- [ ] Live video capture
- [ ] Multi-bitrate adaptive streaming
- [ ] Video blog (vlog) support

#### Phase 5: Influencer Platform (2027)
- [ ] Integration with OnlyFans, Patreon
- [ ] Social media platform connections
- [ ] E-commerce for branded merchandise
- [ ] Subscription management
- [ ] Fan engagement tools
- [ ] In-stream promos and callouts middleware
- [ ] Shoutout and sponsorship management

### 🏷️ Mcaster1 TagStack (ICY 2.0 Metadata Manager)

**Status:** Beta Development - Q2-Q3 2026
**Repository:** https://github.com/davestj/mcaster1-tagstack

#### Core Features (Q2 2026)
- [ ] ICY 2.0 protocol compliance (29+ fields)
- [ ] Hashtag composer with auto-suggestions
- [ ] Emoji indicator support
- [ ] Social media header integration (Twitter, Twitch, YouTube, Instagram, TikTok)
- [ ] Real-time metadata preview
- [ ] Field validation and character counting

#### Podcast & Playlist Management (Q2 2026)
- [ ] Podcast show manager
- [ ] Episode metadata editor
- [ ] Show notes and chapter markers
- [ ] Playlist generation for Mcaster1Encoder
- [ ] Drag-and-drop playlist interface
- [ ] Smart shuffle and crossfade settings
- [ ] Automated rotation schedules

#### Influencer Tools (Q3 2026)
- [ ] Promo and callout scheduling
- [ ] Sponsored content manager
- [ ] In-stream advertisement insertion points
- [ ] Merchandise link embedding
- [ ] Subscription platform integration (OnlyFans, Patreon, Ko-fi)
- [ ] Supporter shoutout automation

#### Analytics & Integration (Q3 2026)
- [ ] Hashtag performance tracking
- [ ] Social media engagement metrics
- [ ] Listener demographics
- [ ] Export reports for sponsors
- [ ] Live sync with Mcaster1DNAS server
- [ ] Real-time metadata updates during broadcasts

### 📡 YP Directory Modernization (yp.mcaster1.com)

**Status:** Planning - Q3-Q4 2026
**Repository:** https://github.com/davestj/mcaster1-yp

#### Core Features
- [ ] Modern web interface for stream directory
- [ ] Search and discovery engine
- [ ] Genre categorization and filtering
- [ ] ICY 2.0 metadata display (hashtags, social links)
- [ ] Podcast directory integration
- [ ] Video stream support
- [ ] Geographic search
- [ ] Language filtering

#### Server Integration
- [ ] Automatic stream registration from Mcaster1DNAS
- [ ] Health check and monitoring
- [ ] Stream quality scoring
- [ ] Uptime statistics
- [ ] Listener count aggregation

#### User Features
- [ ] User accounts for broadcasters
- [ ] Stream ownership verification
- [ ] Custom stream pages
- [ ] Analytics dashboard for broadcasters
- [ ] Featured streams and promotion
- [ ] Community ratings and reviews

---

## 📅 Future Versions (2.9.0+)

### Advanced Features
- [ ] **AI-powered metadata** - Automatic song recognition
  - ACRCloud or AudD integration
  - Automatic metadata updates
  - Cover art fetching
  - Genre classification

- [ ] **Blockchain integration** - Decentralized features
  - Listener verification
  - Micropayments for premium streams
  - Content certification

- [ ] **WebRTC support** - Ultra-low latency streaming
  - Sub-second latency
  - Browser-to-browser relaying
  - Interactive streaming

- [ ] **Cloud storage integration** - Remote media storage
  - S3-compatible storage
  - Google Cloud Storage
  - Azure Blob Storage
  - Automatic backup

### Enterprise Features
- [ ] **Role-based access control (RBAC)** - Granular permissions
  - User roles (admin, operator, viewer)
  - Permission matrix
  - Audit logging
  - LDAP/Active Directory integration

- [ ] **SLA monitoring** - Service level agreements
  - Uptime tracking
  - Performance SLAs
  - Automated reporting
  - Alert thresholds

- [ ] **Multi-tenancy support** - Hosting multiple stations
  - Isolated configurations
  - Resource quotas per tenant
  - Separate admin interfaces
  - Billing integration

### Developer Tools
- [ ] **REST API v2** - Modern API design
  - GraphQL support
  - Versioned endpoints
  - Rate limiting
  - OAuth2 authentication

- [ ] **SDK development** - Client libraries
  - Python SDK
  - JavaScript/Node.js SDK
  - PHP SDK
  - Ruby SDK

- [ ] **Terraform/Ansible modules** - Infrastructure as code
  - Automated deployment
  - Configuration management
  - Cloud provider integration

---

## 🐛 Bug Fixes & Technical Debt

### Known Issues to Address
- [ ] Memory leak in long-running relay connections
- [ ] Occasional race condition in client disconnect
- [ ] SSL certificate reload without restart
- [ ] Large playlist handling optimization
- [ ] Windows service installation improvements

### Code Quality
- [ ] Increase unit test coverage (target: 80%)
- [ ] Static analysis with Coverity/Clang Analyzer
- [ ] Modernize autotools configuration
- [ ] CMake build system option
- [ ] Refactor legacy code from Icecast2 era

### Documentation
- [ ] API reference documentation
- [ ] Video tutorials
- [ ] Troubleshooting guide expansion
- [ ] Best practices guide
- [ ] Security hardening guide

---

## 🎨 UI/UX Wishlist

### Nice to Have
- [ ] Stream visualizer with audio spectrum
- [ ] Emoji reactions for listeners
- [ ] Chat integration for interactive streams
- [ ] Listener voting/polling system
- [ ] Social media integration (share buttons)
- [ ] Custom theme builder
- [ ] Animated transitions and effects
- [ ] Notification system for admin events

---

## 🔐 Security Enhancements

### Planned Improvements
- [ ] Two-factor authentication (2FA/TOTP)
- [ ] IP allowlist/denylist management UI
- [ ] Rate limiting per IP/user
- [ ] DDoS protection mechanisms
- [ ] Security headers (CSP, HSTS, etc.)
- [ ] Automated security scanning
- [ ] Vulnerability disclosure program

---

## 📊 Analytics & Reporting

### Future Analytics
- [ ] Listener retention metrics
- [ ] Geographic analytics (GeoIP)
- [ ] Device/platform detection
- [ ] Popular listening times heatmap
- [ ] Stream health scoring
- [ ] Bandwidth usage forecasting
- [ ] Exportable reports (PDF, CSV)

---

## 🌍 Community Requests

### User-Requested Features
- [ ] Icecast2 configuration migration tool
- [ ] Stream recording functionality
- [ ] Automated podcast generation from recordings
- [ ] RTMP input support for video streaming
- [ ] Discord bot integration
- [ ] Prometheus metrics exporter
- [ ] Docker official image
- [ ] Kubernetes Helm chart

---

## 📝 How to Contribute

Want to work on any of these items?

1. **Comment on the related GitHub issue** (or create one)
2. **Discuss the approach** with maintainers
3. **Fork and create a branch** for your work
4. **Submit a pull request** when ready
5. **Get your feature merged!** 🎉

See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed guidelines.

---

## 🏆 Completed Features Archive

Features completed in previous versions:

### Version 2.5.0 (February 2026)
- Complete Mcaster1DNAS rebranding
- Modern web interface with FontAwesome 6.4.0
- Interactive help tooltips
- Live clock and performance metrics
- Enhanced admin dashboard
- Comprehensive documentation

### Version 2.4.0.kh22 (Icecast-KH)
- Enhanced authentication processing
- Worker thread improvements
- Range handling updates
- FLV metadata resize support

### Version 2.4.0.kh21 (Icecast-KH)
- Dual queue worker implementation
- Fast/slow queue processing

*(See NEWS.archive for complete Icecast-KH history)*

---

## 📞 Contact

Have ideas for features? Contact us:

- **GitHub Issues:** https://github.com/davestj/mcaster1dnas/issues
- **Email:** davestj@gmail.com
- **Website:** https://mcaster1.com

---

**Last Updated:** February 15, 2026
**Current Version:** 2.5.1-rc1 (Pre-Release)
**Next Release:** 2.6.1 (Q2 2026 - Podcast Features)
**Following Release:** 2.6.2 (Q3 2026 - Security & Authentication)

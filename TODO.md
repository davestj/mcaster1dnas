# Mcaster1DNAS - TODO & Roadmap

Features, improvements, and tasks planned for future releases.

---

## 🎯 Current Version: 2.5.0

### ✅ Completed in 2.5.0

- [x] Complete rebrand from Icecast-KH to Mcaster1DNAS
- [x] Modern HTML5/CSS3 web interface
- [x] FontAwesome 6.4.0 icon integration
- [x] Interactive help tooltip system
- [x] Live clock and page load metrics
- [x] HTTPS/SSL by default (ports 9330 HTTP, 9443 HTTPS)
- [x] Enhanced admin dashboard with real-time stats
- [x] Credits and fork attribution pages
- [x] Comprehensive documentation (README.md, FORK.md, etc.)
- [x] Responsive mobile-friendly design
- [x] Professional navigation system
- [x] Build automation scripts (build.sh, mcaster1-control.sh)

---

## 📅 Version 2.6.0 - UI/UX Enhancements (Next Release)

**Target:** Q2 2026
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

## 📅 Version 2.7.0 - Feature Enhancements

**Target:** Q3 2026
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

**Target:** Q4 2026
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

**Last Updated:** February 14, 2026
**Current Version:** 2.5.0
**Next Release:** 2.6.0 (Q2 2026)

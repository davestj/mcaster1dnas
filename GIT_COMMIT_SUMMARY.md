# Git Commit Message Summary
**Mcaster1DNAS v2.5.0 → v2.6.0**
**Date:** February 14-15, 2026

---

## Recommended Commit Message (Concise)

```
feat: Add ICY-META v2.1+ protocol, YAML config, HTTPS encoders, and enhanced UI

Major features in this release:
- ICY-META v2.1+ extended metadata protocol with 29+ fields
- YAML configuration support alongside XML (full feature parity)
- HTTPS/SSL source encoder support with auto-detection
- YP connection logging for directory debugging
- Tabbed logs interface with session persistence
- Complete protocol specification and documentation

Implementation:
- New icy2_meta.c/h module for ICY2 parsing
- YAML parser (cfgfile_yaml.c) with libyaml support
- SSL auto-detection in connection handler
- Enhanced admin interface with 4-tab log viewer
- Per-YP-directory logging capability

Documentation:
- ICY2_PROTOCOL_SPEC.md - Complete ICY-META v2.1+ specification
- YAML_IMPLEMENTATION.md - YAML configuration guide
- YP_LOGGING_FEATURE.md - YP logging implementation
- Updated README.md, ChangeLog, TODO.md

Breaking Changes: None (100% backward compatible)

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
```

---

## Alternative: Detailed Commit Message

```
feat(protocol): Implement ICY-META v2.1+ extended metadata protocol

ICY-META v2.1+ provides rich metadata support for modern streaming:

Protocol Features:
- Auto-detection via icy-metadata-version header
- Seamless failover to ICY 1.x legacy support
- 29+ metadata fields across 6 categories:
  * Core: station-id, name, url, genre, bitrate, public
  * Podcast: host, rss, episode, duration, language
  * Video: type, link, title, platform, resolution
  * Social Media: dj-handle, twitter, instagram, tiktok, emoji, hashtags
  * Access Control: nsfw, ai-generated, geo-region, auth-token
  * Verification: certificate-verify, verification-status

Implementation:
- New icy2_meta.c/h module with dedicated parser
- Stats system integration for admin display
- Zero configuration required - auto-detection
- Forward compatible with ICY 2.2, 2.3

Testing:
- curl-based validation framework
- Verified all 29+ metadata fields
- Confirmed ICY 1.x fallback behavior

Documentation:
- Complete protocol specification (ICY2_PROTOCOL_SPEC.md)
- Implementation architecture (ICY2_SIMPLIFIED_PLAN.md)
- Client/server guidelines and examples

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
```

```
feat(config): Add YAML configuration support with full XML parity

YAML configuration provides modern, readable alternative to XML:

Features:
- Automatic format detection (YAML vs XML)
- 100% feature parity with XML configuration
- Four comprehensive templates included:
  * mcaster1.yaml.in - Full-featured with examples
  * mcaster1_minimal.yaml.in - Minimal config
  * mcaster1_shoutcast_compat.yaml.in - Shoutcast optimized
  * mcaster1_advanced.yaml.in - Advanced features

Implementation:
- New cfgfile_yaml.c parser (~2000 lines)
- libyaml >= 0.1.7 dependency (optional)
- Conditional compilation (--with-yaml flag)
- Complete error reporting with line numbers

Supports all features:
- Server settings and limits
- Listen sockets (HTTP, HTTPS, IPv6)
- Mount points with authentication
- Relays with multi-host failover
- YP directories with logging
- Master/slave replication

Documentation:
- Complete usage guide (YAML_IMPLEMENTATION.md)
- Migration from XML instructions
- All templates with inline comments

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
```

```
feat(ssl): Add HTTPS/SSL source encoder support with auto-detection

Encoders can now send streams via HTTPS for enhanced security:

Features:
- SSL/TLS auto-detection on any port
- Works with both ICY 1.x and ICY2 protocols
- No configuration changes required
- Automatic protocol negotiation

Detection:
- Checks for SSL ClientHello handshake pattern (0x16 0x03 ... 0x01)
- Seamlessly upgrades connection to SSL/TLS
- Falls back to plaintext HTTP if no SSL detected

Compatibility:
- FFmpeg with SSL support
- Liquidsoap
- Butt (v0.1.30+)
- OBS with streaming plugins
- curl with --data-binary

Testing:
- Verified HTTPS source connections on port 9443
- Confirmed certificate handling
- Validated source authentication over HTTPS

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
```

```
feat(logging): Add YP connection logging for directory debugging

Per-YP-server logging capability for troubleshooting directory issues:

Features:
- Optional yp-logfile parameter in directory config
- Logs all YP transactions: add, remove, touch
- Timestamped entries with success/failure status
- Separate log file for each YP directory server
- Health check system (pings at touch-interval)

Format:
[YYYY-MM-DD HH:MM:SS] YP_URL - ACTION: DETAILS

Implementation:
- Added yp_logfile field to yp_server struct
- Direct disk writes via log_write_direct()
- Fixed admin.c to read YP logs from disk (not in-memory)

Admin Interface:
- New "YP Connections" tab in logs.xsl
- Session persistence for active tab
- Clean tabbed design with smooth transitions

Use Cases:
- Debug why streams don't appear in directories
- Monitor YP server health proactively
- Track historical YP transactions

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
```

```
feat(ui): Add tabbed logs interface with enhanced admin pages

Modern tabbed logs interface with session persistence:

Features:
- Four tabs: Access Log, Error Log, Playlist Log, YP Connections
- Session persistence (remembers active tab)
- Smooth animations and transitions
- Helpful descriptions for each log type
- Professional card-based layout

Fixes:
- Fixed XML parsing errors in listmounts.xsl
- Fixed managerelays.xsl tag structure
- Added missing live clock to admin pages
- Corrected formatting issues

Admin Interface:
- Enhanced navigation
- Improved mobile responsiveness
- Better error handling
- Consistent styling across pages

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>
```

---

## Alternative: Separate Commits (Multiple)

If you prefer atomic commits per feature:

```bash
# Commit 1: ICY-META v2.1+ Protocol
git add src/icy2_meta.* src/source.c src/Makefile.am ICY2_PROTOCOL_SPEC.md ICY2_SIMPLIFIED_PLAN.md
git commit -m "feat(protocol): Implement ICY-META v2.1+ extended metadata protocol

Add complete ICY-META v2.1+ protocol support with auto-detection,
29+ metadata fields, and seamless ICY 1.x fallback. Includes dedicated
icy2_meta.c/h module and complete specification documentation.

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>"

# Commit 2: YAML Configuration
git add src/cfgfile_yaml.* conf/mcaster1*.yaml.in YAML_IMPLEMENTATION.md configure.ac
git commit -m "feat(config): Add YAML configuration support with full XML parity

Add complete YAML configuration parser with auto-detection, libyaml
integration, and four comprehensive templates. 100% feature parity
with XML configuration.

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>"

# Commit 3: HTTPS Encoder Support
git add src/connection.c
git commit -m "feat(ssl): Add HTTPS/SSL source encoder support with auto-detection

Enable encoders to send streams via HTTPS with automatic SSL/TLS
detection. Works on any port with no configuration changes required.

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>"

# Commit 4: YP Connection Logging
git add src/yp.c src/admin.c YP_LOGGING_FEATURE.md admin/logs.xsl
git commit -m "feat(logging): Add YP connection logging and tabbed logs interface

Add per-YP-server logging capability for directory debugging. Includes
new tabbed logs interface with 4 tabs and session persistence.

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>"

# Commit 5: Documentation Updates
git add README.md ChangeLog TODO.md
git commit -m "docs: Update documentation for v2.6.0 features

Update README, ChangeLog, and TODO with ICY-META v2.1+ protocol,
YAML configuration, HTTPS encoders, and enhanced UI features.

Co-Authored-By: Claude Sonnet 4.5 <noreply@anthropic.com>"
```

---

## Quick Summary (One-Line)

For quick commits or pull requests:

```
feat: ICY-META v2.1+ protocol, YAML config, HTTPS encoders, YP logging, enhanced UI
```

```
v2.6.0: Add ICY2 protocol, YAML support, HTTPS encoders, and tabbed logs UI
```

```
Release v2.6.0: ICY-META v2.1+, YAML configuration, enhanced logging and UI
```

---

## Pull Request Title

If creating a GitHub Pull Request:

```
🎉 Release v2.6.0: ICY-META v2.1+ Protocol, YAML Config, HTTPS Encoders & Enhanced UI
```

---

## Git Tag

For versioning:

```bash
git tag -a v2.6.0 -m "Mcaster1DNAS v2.6.0 - ICY-META v2.1+ Protocol Support

Major Features:
- ICY-META v2.1+ extended metadata protocol (29+ fields)
- YAML configuration support (full XML parity)
- HTTPS/SSL source encoder support
- YP connection logging and health monitoring
- Tabbed logs interface with session persistence

Breaking Changes: None
Backward Compatible: 100%

Release Date: February 15, 2026"
```

---

## Conventional Commits Format

Following Conventional Commits specification:

```
feat!: major release v2.6.0 with ICY2 protocol and YAML config

BREAKING CHANGE: None - fully backward compatible

Features:
- ICY-META v2.1+ protocol (icy2_meta.c/h)
- YAML configuration (cfgfile_yaml.c)
- HTTPS source encoders (SSL auto-detection)
- YP connection logging (yp.c, admin.c)
- Tabbed logs interface (admin/logs.xsl)

Docs:
- ICY2_PROTOCOL_SPEC.md
- YAML_IMPLEMENTATION.md
- YP_LOGGING_FEATURE.md
- Updated README, ChangeLog, TODO

Refs: #1, #2, #3
```

---

## Recommended Approach

**For Single Commit:**
Use the "Recommended Commit Message (Concise)" - it's comprehensive yet readable.

**For Multiple Commits:**
Use the "Separate Commits" approach - each feature gets its own atomic commit for better git history.

**For GitHub Release:**
Use the "Pull Request Title" format with the detailed commit message as PR description.

**For Git Tag:**
Use the provided tag message format for version tracking.

---

**Generated:** February 15, 2026
**Version:** 2.6.0
**Author:** Mcaster1DNAS Development Team

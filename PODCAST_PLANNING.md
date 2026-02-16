# Mcaster1DNAS Podcast Feature - Implementation Plan

**Version:** 2.6.1+
**Target:** Q2-Q3 2026
**Status:** Planning Phase
**Last Updated:** February 15, 2026

---

## 🎯 Project Vision

Empower podcast hosting companies and self-hosted podcasters with an open-source, feature-rich podcast management platform integrated into Mcaster1DNAS. We are NOT becoming a podcast hosting service - we're providing the free, honest tools for others to host.

**Core Philosophy:**
- 100% Open Source - No paid tiers, no subscriptions
- Self-hosted first - Full control of your data
- Hybrid approach - Bridge live streaming AND podcasting
- Simple baby steps - Ship small, useful features quickly

---

## 🎧 Target Users

1. **Self-hosted podcasters** - Complete control, zero monthly fees
2. **Podcast hosting companies** - White-label solution for their customers
3. **Radio stations** - Record live shows as podcast episodes automatically
4. **Indie broadcasters** - Run 24/7 podcast "radio stations" from their catalog

---

## 📅 6-Week Implementation Roadmap

### **Week 1-2: Basic RSS Generation**

**Goal:** Auto-generate podcast RSS feeds from a directory of audio files.

#### Files to Create:
```
src/podcasting.c          - Core podcast management module
src/podcasting.h          - Public API and structures
src/id3_parser.c          - ID3v2/Vorbis comment parser
src/id3_parser.h          - Audio metadata parser interface
```

#### Core Functionality:
1. **Directory Scanning**
   - Watch configured podcast directory for .mp3, .ogg, .m4a files
   - Detect new files and changes
   - Sort by modification date (newest first)

2. **Metadata Extraction**
   - Parse ID3v2 tags from MP3 files (title, artist, album, duration, etc.)
   - Parse Vorbis comments from .ogg files
   - Parse AAC metadata from .m4a files
   - Extract cover art if embedded

3. **RSS 2.0 Feed Generation**
   - Create valid RSS 2.0 podcast feed
   - Include iTunes podcast tags (required for Apple Podcasts)
   - Support episode-level metadata
   - Include enclosure tags with file URLs
   - Auto-calculate file sizes and durations

4. **Configuration Support (YAML & XML)**
```yaml
# mcaster1dnas-production.yaml
podcasting:
  enabled: true
  media-directory: /var/podcasts/myshow
  rss-feed-path: /podcasts/feed.rss
  auto-generate-rss: true

  # RSS Feed Metadata
  title: "My Awesome Podcast"
  description: "Weekly episodes about tech and life"
  author: "John Podcaster"
  email: "john@example.com"
  category: Technology
  language: en-us
  cover-art: /podcasts/cover.jpg
  website: https://mypodcast.example.com
```

```xml
<!-- mcaster1dnas-production.xml -->
<podcasting>
    <enabled>true</enabled>
    <media-directory>/var/podcasts/myshow</media-directory>
    <rss-feed-path>/podcasts/feed.rss</rss-feed-path>
    <auto-generate-rss>true</auto-generate-rss>

    <rss-metadata>
        <title>My Awesome Podcast</title>
        <description>Weekly episodes about tech and life</description>
        <author>John Podcaster</author>
        <email>john@example.com</email>
        <category>Technology</category>
        <language>en-us</language>
        <cover-art>/podcasts/cover.jpg</cover-art>
        <website>https://mypodcast.example.com</website>
    </rss-metadata>
</podcasting>
```

#### Dependencies:
- **libid3tag** - ID3v2 tag parsing (already in many distros)
- **libvorbisfile** - Vorbis comment parsing (likely already installed)
- **libavformat** (optional) - For .m4a and duration detection

#### Deliverables:
- [ ] `podcasting.c/h` module integrated into build system
- [ ] `id3_parser.c/h` with support for MP3, Ogg Vorbis, M4A
- [ ] YAML/XML config parser updates for `<podcasting>` block
- [ ] RSS 2.0 feed generator with iTunes tags
- [ ] Basic unit tests for metadata parsing
- [ ] Documentation: PODCAST_QUICKSTART.md

---

### **Week 3-4: Web UI for Podcast Management**

**Goal:** Create admin interface to view, manage, and download podcast episodes.

#### Files to Create:
```
admin/podcasts.xsl         - Podcast episode listing page
web/podcasts.xsl           - Public podcast page (optional)
src/admin.c                - Register podcasts.xsl endpoint
```

#### Core Functionality:
1. **Episode Listing Page (`/admin/podcasts.xsl`)**
   - Table view of all episodes in podcast directory
   - Display: Title, Duration, File Size, Format, Upload Date
   - Download links for each episode
   - Play buttons (use existing webplayer.xsl integration)
   - RSS feed link (copy-paste for podcast apps)

2. **Statistics Dashboard**
   - Total episodes count
   - Total storage used
   - Most recent episode info
   - RSS feed validation status

3. **Episode Details Modal**
   - Full metadata display (all ID3/Vorbis tags)
   - Cover art preview
   - File path and technical info (bitrate, sample rate, channels)
   - Download count (if tracking enabled)

4. **RSS Feed Preview**
   - Live preview of generated RSS feed
   - Validation status (W3C Feed Validator integration)
   - Copy-to-clipboard button for feed URL
   - Test in podcast app instructions

5. **Public Podcast Page (`/podcasts.xsl`)**
   - Publicly accessible episode listing
   - Subscribe buttons (Apple Podcasts, Spotify, RSS)
   - Embedded player for each episode
   - Show notes display (from .txt sidecar files)

#### UI Design:
- Use existing Mcaster1DNAS design system
- CSS variables: `--mcaster-blue`, `--dnas-green`, etc.
- FontAwesome 6.x icons
- Responsive design (mobile-friendly)
- Match stats.xsl and listmounts.xsl styling

#### Admin Command Registration:
```c
// src/admin.c
static struct admin_command admin_general[] = {
    { "podcasts.xsl", XSLT, { command_podcast_list } },
    // ... existing commands
};
```

#### XML Response Format:
```xml
<mcaster1stats>
    <podcasts>
        <show>
            <title>My Awesome Podcast</title>
            <description>Weekly episodes about tech and life</description>
            <rss-feed>/podcasts/feed.rss</rss-feed>
            <total-episodes>42</total-episodes>
            <total-size>1.2GB</total-size>
            <latest-episode>2026-02-15T10:30:00Z</latest-episode>
        </show>
        <episode>
            <filename>episode-042-ai-revolution.mp3</filename>
            <title>Episode 42: The AI Revolution</title>
            <description>We discuss the latest in AI technology</description>
            <author>John Podcaster</author>
            <duration>3600</duration> <!-- seconds -->
            <file-size>48000000</file-size> <!-- bytes -->
            <format>MP3</format>
            <bitrate>128</bitrate>
            <sample-rate>44100</sample-rate>
            <channels>2</channels>
            <publish-date>2026-02-15T10:30:00Z</publish-date>
            <download-url>/podcasts/episode-042-ai-revolution.mp3</download-url>
            <stream-url>/podcasts/episode-042-ai-revolution.mp3</stream-url>
            <cover-art>/podcasts/episode-042-cover.jpg</cover-art>
        </episode>
        <!-- ... more episodes -->
    </podcasts>
</mcaster1stats>
```

#### Deliverables:
- [ ] `admin/podcasts.xsl` with full episode management UI
- [ ] `web/podcasts.xsl` for public podcast page
- [ ] `command_podcast_list()` in src/admin.c
- [ ] XML output for podcast data (mcaster1podcasts format)
- [ ] RSS feed validation integration
- [ ] Download statistics (optional, basic counter)
- [ ] Documentation: PODCAST_WEB_UI.md

---

### **Week 5-6: Live Stream Recording (The Killer Feature!)**

**Goal:** Automatically record live streams and publish them as podcast episodes.

#### Files to Modify/Create:
```
src/format.c               - Add recording hooks to stream handlers
src/format.h               - Recording API definitions
src/podcasting.c           - Add episode auto-publishing
conf/mcaster1dnas-production.yaml - Recording config
```

#### Core Functionality:
1. **Per-Mount Recording Configuration**
```yaml
mounts:
  - name: /live.mp3
    username: source
    password: hackme

    # NEW: Podcast Recording Options
    podcast-recording:
      enabled: true
      auto-publish: true
      output-directory: /var/podcasts/liveshow
      filename-pattern: "live-{date}-{time}.mp3"
      episode-title-pattern: "Live Show - {date}"
      episode-description: "Recorded live stream from {date} at {time}"
      split-on-silence: false  # Future: auto-split long streams
      max-duration: 7200       # Stop after 2 hours
      min-duration: 300        # Don't save if < 5 minutes
```

2. **Recording Engine**
   - Hook into existing format handlers (MP3, Ogg)
   - Write raw stream data to file while streaming
   - Handle source disconnects gracefully
   - Resume recording if source reconnects (optional)
   - Atomic file writes (temp file → rename when complete)

3. **Metadata Injection**
   - Capture ICY metadata during recording
   - Embed as ID3 tags in final MP3 file
   - Create show notes from metadata log
   - Extract chapter markers from metadata changes

4. **Auto-Publishing Workflow**
   - After recording completes, validate file
   - Extract metadata using id3_parser
   - Move to podcast directory
   - Update RSS feed automatically
   - Send notification (log entry or webhook)

5. **Manual Recording Control**
   - Admin UI buttons: "Start Recording", "Stop Recording"
   - Recording status indicator on stats.xsl
   - Recording duration counter
   - Estimated file size display

#### Admin UI Integration:
```xml
<!-- In stats.xsl mount point display -->
<mount>
    <name>/live.mp3</name>
    <listeners>42</listeners>

    <!-- NEW: Recording status -->
    <recording>
        <enabled>true</enabled>
        <active>true</active>
        <duration>1847</duration> <!-- seconds -->
        <file-size>24567890</file-size>
        <output-file>/var/podcasts/liveshow/live-2026-02-15-1430.mp3</output-file>
    </recording>
</mount>
```

Add to `stats.xsl` and `listmounts.xsl`:
- Recording indicator badge (red dot when recording)
- Recording duration timer
- Manual controls (start/stop buttons)
- Link to recorded episode once published

#### Hybrid Streaming Feature:
```yaml
mounts:
  - name: /247radio
    type: hybrid  # NEW mount type!

    # If live source connected, stream it
    live-source:
      enabled: true
      username: source
      password: hackme

    # If no live source, play latest podcast episode on loop
    fallback-mode:
      enabled: true
      podcast-directory: /var/podcasts/liveshow
      play-mode: latest  # or 'random', 'sequential'
      loop: true
```

This is UNIQUE - no other streaming server does this!

#### Deliverables:
- [ ] Recording engine integrated into format.c
- [ ] Per-mount recording configuration
- [ ] Auto-publish workflow to podcast directory
- [ ] Metadata injection during recording
- [ ] Admin UI recording controls
- [ ] Hybrid mount point support (live + fallback)
- [ ] Recording statistics and history
- [ ] Documentation: PODCAST_RECORDING.md

---

## 🏗️ Build System Integration

### Makefile.am Updates
```makefile
# Add new source files
mcaster1_SOURCES = \
    ... existing sources ... \
    podcasting.c \
    id3_parser.c

# Add new headers
noinst_HEADERS = \
    ... existing headers ... \
    podcasting.h \
    id3_parser.h

# Add podcast admin XSL files
dist_admindir_DATA = \
    ... existing XSL files ... \
    podcasts.xsl

# Add podcast web XSL files (optional)
dist_webdir_DATA = \
    ... existing XSL files ... \
    podcasts.xsl
```

### Dependency Detection (configure.ac)
```autoconf
# Check for libid3tag
PKG_CHECK_MODULES([ID3TAG], [id3tag], [
    AC_DEFINE([HAVE_ID3TAG], 1, [Define if libid3tag is available])
], [
    AC_MSG_WARN([libid3tag not found - podcast features will be limited])
])

# Check for libvorbisfile
PKG_CHECK_MODULES([VORBISFILE], [vorbisfile], [
    AC_DEFINE([HAVE_VORBISFILE], 1, [Define if libvorbisfile is available])
], [
    AC_MSG_WARN([libvorbisfile not found - Ogg podcast support disabled])
])
```

---

## 📊 Data Structures

### Podcast Episode Structure (src/podcasting.h)
```c
typedef struct _podcast_episode {
    char *filename;           // episode-042.mp3
    char *filepath;           // /var/podcasts/myshow/episode-042.mp3
    char *title;              // Episode 42: The AI Revolution
    char *description;        // Episode description or show notes
    char *author;             // John Podcaster
    char *album;              // Podcast show name
    char *genre;              // Technology

    time_t publish_date;      // Unix timestamp
    unsigned int duration;    // Seconds
    size_t file_size;         // Bytes

    char *format;             // MP3, Ogg Vorbis, M4A
    unsigned int bitrate;     // kbps
    unsigned int sample_rate; // Hz (e.g., 44100)
    unsigned int channels;    // 1 (mono) or 2 (stereo)

    char *cover_art_path;     // Path to cover image
    char *stream_url;         // HTTP URL to stream episode
    char *download_url;       // HTTP URL to download episode

    unsigned long downloads;  // Download counter (optional)

    struct _podcast_episode *next;  // Linked list
} podcast_episode_t;

typedef struct {
    char *title;              // Podcast show title
    char *description;        // Show description
    char *author;             // Show author/host
    char *email;              // Contact email
    char *category;           // iTunes category
    char *language;           // en-us, es, etc.
    char *cover_art;          // Show cover art URL
    char *website;            // Podcast website URL
    char *rss_feed_path;      // /podcasts/feed.rss

    char *media_directory;    // /var/podcasts/myshow
    int auto_generate_rss;    // Boolean flag

    podcast_episode_t *episodes;  // Linked list of episodes
    unsigned int episode_count;
    size_t total_size;        // Total bytes of all episodes

    time_t last_scan;         // Last directory scan timestamp
} podcast_show_t;
```

### Recording Configuration (src/podcasting.h)
```c
typedef struct {
    int enabled;              // Recording enabled for this mount
    int auto_publish;         // Auto-add to podcast feed

    char *output_directory;   // /var/podcasts/liveshow
    char *filename_pattern;   // live-{date}-{time}.mp3
    char *title_pattern;      // Live Show - {date}
    char *description;        // Episode description template

    unsigned int max_duration;    // Max recording length (seconds)
    unsigned int min_duration;    // Min recording to save (seconds)
    int split_on_silence;         // Auto-split feature (future)

    // Runtime state
    int currently_recording;  // Boolean
    time_t recording_start;   // Start timestamp
    size_t bytes_recorded;    // Current file size
    FILE *recording_file;     // File handle
    char *temp_filepath;      // Temp file path
} podcast_recording_t;
```

---

## 🧪 Testing Plan

### Week 1-2 Testing:
1. **Directory Scanning**
   - Create test directory with 10 MP3 files
   - Verify all files detected
   - Test with subdirectories
   - Test with mixed formats (MP3, Ogg, M4A)

2. **Metadata Parsing**
   - Test files with ID3v2.3 and ID3v2.4 tags
   - Test Vorbis comments
   - Test files with no metadata
   - Test Unicode characters in titles
   - Test embedded cover art extraction

3. **RSS Generation**
   - Validate RSS with W3C Feed Validator
   - Test in Apple Podcasts (iOS)
   - Test in Spotify
   - Test in Pocket Casts
   - Verify enclosure URLs work

### Week 3-4 Testing:
1. **Admin UI**
   - Test on desktop browsers (Chrome, Firefox, Safari)
   - Test on mobile (iOS Safari, Android Chrome)
   - Verify all buttons and links work
   - Test with 0 episodes, 1 episode, 100+ episodes

2. **Web Player Integration**
   - Click "Play" on episode, verify webplayer.xsl opens
   - Test playback of MP3 episodes
   - Test playback of Ogg episodes
   - Verify metadata displays correctly

### Week 5-6 Testing:
1. **Recording**
   - Start live stream, verify recording starts
   - Stop stream, verify file saved correctly
   - Verify metadata embedded in recorded file
   - Test max_duration cutoff
   - Test min_duration filtering
   - Test disk full scenario

2. **Auto-Publishing**
   - Verify recorded file appears in RSS feed
   - Check episode metadata accuracy
   - Test with concurrent recordings (multiple mounts)

3. **Hybrid Mode**
   - Test fallback to podcast when source disconnects
   - Test switching back to live when source reconnects
   - Verify smooth transitions (no gaps/glitches)

---

## 📚 Documentation Deliverables

### User Documentation:
1. **PODCAST_QUICKSTART.md**
   - 5-minute setup guide
   - Basic configuration examples
   - RSS feed testing instructions

2. **PODCAST_WEB_UI.md**
   - Admin interface walkthrough
   - Episode management guide
   - RSS feed distribution guide (Apple, Spotify, etc.)

3. **PODCAST_RECORDING.md**
   - Live stream recording setup
   - Hybrid mount configuration
   - Troubleshooting guide

### Developer Documentation:
4. **PODCAST_API.md**
   - C API reference for podcasting.c
   - XML response format documentation
   - Extension points for future features

---

## 🚀 Future Enhancements (Post-Week 6)

### Phase 2 Features (Q3 2026):
- [ ] **Show Notes Support** - Read .txt or .md sidecar files
- [ ] **Chapter Markers** - ID3 CHAP frame support
- [ ] **Multiple Podcast Shows** - Support multiple shows per server
- [ ] **Episode Scheduling** - Publish episodes at specific times
- [ ] **Download Analytics** - Track downloads per episode, per country
- [ ] **Podcast Playlist Creator** - Feed to Mcaster1Encoder for 24/7 radio

### Phase 3 Features (Q4 2026):
- [ ] **Video Podcast Support** - MP4, WebM for video podcasting
- [ ] **Dynamic Ad Insertion** - Pre-roll, mid-roll, post-roll ads
- [ ] **Transcription Integration** - Whisper API for show notes
- [ ] **Cross-Promotion** - Link live streams and podcast episodes
- [ ] **YP Directory Integration** - List podcasts on yp.mcaster1.com

---

## 🔗 Integration with Mcaster1 Ecosystem

### Mcaster1Encoder Integration:
- Feed podcast playlists to encoder
- Create 24/7 podcast "radio station"
- Auto-crossfade between episodes
- Dynamic metadata updates per episode

### Mcaster1 TagStack Integration:
- Manage podcast metadata before upload
- Batch edit episode information
- Generate show notes templates
- Schedule episode releases

### YP Directory (yp.mcaster1.com):
- Separate podcast directory
- Search and discovery
- Genre categorization
- Subscription links for all podcast apps

---

## ✅ Success Criteria

By the end of Week 6, we should have:
1. ✅ Auto-generated RSS feeds from podcast directories
2. ✅ Admin UI to view and manage episodes
3. ✅ Live stream recording with auto-publishing
4. ✅ Hybrid mount points (live + fallback to podcast)
5. ✅ Full documentation for users and developers
6. ✅ Tested on real podcast apps (Apple, Spotify, Pocket Casts)
7. ✅ Zero bugs, production-ready code

---

## 📞 Questions to Resolve

Before starting Week 1:
- [ ] Which metadata library? libid3tag vs TagLib vs custom parser?
- [ ] RSS hosting: Same server or separate web server option?
- [ ] File upload UI: Do we need web-based upload or just filesystem monitoring?
- [ ] Multi-show support: One podcast per server or multiple shows?
- [ ] Storage limits: Should we add disk quota warnings?

---

**Ready to start Week 1?** Let's build something amazing! 🎧🚀

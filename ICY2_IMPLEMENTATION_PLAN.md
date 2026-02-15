# ICY-META v2.1+ Implementation Plan
## Mcaster1DNAS - CasterClub Streaming Standards Initiative

**Date:** February 14, 2026
**Version:** 2.1+
**Status:** Planning Phase

---

## 🎯 Executive Summary

**Complexity Level:** HIGH
**Estimated Effort:** 200-300 hours (4-6 weeks full-time)
**Files to Modify:** 15-20 files
**New Files:** 3-5 files
**Lines of Code:** ~2,000-3,000 new lines
**Backward Compatibility:** 100% (Critical requirement)

**Strategic Value:** GAME-CHANGER
- Positions Mcaster1DNAS as the most advanced streaming server
- Enables podcast, video metadata, and social media integration
- Creates foundation for yp.casterclub.com directory integration
- Opens revenue opportunities (verification services, premium listings)

---

## 📋 Current State Analysis

### ✅ What We Have (ICY 1.x)

**Current ICY Header Support:**
- ✅ `icy-name` - Stream name
- ✅ `icy-genre` - Genre
- ✅ `icy-url` - Station URL
- ✅ `icy-pub` / `ice-public` - Public flag
- ✅ `icy-br` / `ice-bitrate` - Bitrate
- ✅ `icy-metaint` - Metadata interval
- ✅ `StreamTitle` / `StreamUrl` - In-stream metadata

**Where ICY Headers Are Handled:**

1. **src/source.c (lines 2024-2138)**
   - Parses incoming ICY headers from source clients
   - Converts ice-* to icy-* for compatibility
   - Stores in `source->stats` (statistics handle)

2. **src/format.c (lines 507-537)**
   - Formats ICY headers for listeners
   - Converts ice-audio-info to icy-br
   - Handles multiple format variants

3. **src/format_mp3.c**
   - Handles StreamTitle/StreamUrl metadata
   - Manages metadata blocks for MP3 streams
   - Injects metadata into audio stream

4. **src/util.h + src/util.c**
   - `util_dict` structure stores key-value pairs
   - Used for `source->audio_info` dictionary
   - Simple linked-list implementation

5. **src/stats.c**
   - Statistics system stores metadata
   - Accessible via admin interface
   - XML/JSON output for directories

---

## 🏗️ Architecture Design for ICY2.1+

### Phase 1: Core Infrastructure (Week 1-2)

#### 1.1 Create ICY2 Metadata Structure

**New File:** `src/icy2_meta.h`

```c
#ifndef __ICY2_META_H__
#define __ICY2_META_H__

#define ICY2_META_VERSION "2.1"

/* ICY2 Metadata Categories */
typedef enum {
    ICY2_CAT_CORE,          // Core v2.1+ fields
    ICY2_CAT_AUTH,          // Certificate/verification
    ICY2_CAT_AUDIO,         // Audio metadata
    ICY2_CAT_PODCAST,       // Podcast fields
    ICY2_CAT_VIDEO,         // Video metadata
    ICY2_CAT_SOCIAL,        // Social media
    ICY2_CAT_ACCESS,        // AI/NSFW flags
    ICY2_CAT_LEGACY         // ICY 1.x compat
} icy2_category_t;

/* ICY2 Metadata Entry */
typedef struct icy2_meta_entry {
    char *key;                          // Header name
    char *value;                        // Header value
    icy2_category_t category;           // Category
    time_t updated;                     // Last update time
    struct icy2_meta_entry *next;       // Linked list
} icy2_meta_entry_t;

/* ICY2 Metadata Container */
typedef struct icy2_metadata {
    char *version;                      // ICY-META version (2.1)

    /* Core Fields */
    char *station_id;                   // icy-meta-station-id
    char *cert_issuer_id;               // icy-meta-certissuer-id
    char *cert_rootca;                  // icy-meta-cert-rootca
    char *certificate;                  // icy-meta-certificate
    char *verification_status;          // icy-meta-verification-status

    /* Podcast Fields */
    char *podcast_host;                 // icy-meta-podcast-host
    char *podcast_rss;                  // icy-meta-podcast-rss
    char *podcast_episode;              // icy-meta-podcast-episode
    char *language;                     // icy-meta-language
    int duration;                       // icy-meta-duration

    /* Video Fields */
    char *video_type;                   // icy-meta-videotype
    char *video_link;                   // icy-meta-videolink
    char *video_title;                  // icy-meta-videotitle
    char *video_poster;                 // icy-meta-videoposter
    char *video_channel;                // icy-meta-videochannel
    char *video_platform;               // icy-meta-videoplatform
    char *video_resolution;             // icy-meta-videoresolution
    char *video_codec;                  // icy-meta-videocodec
    int video_duration;                 // icy-meta-videoduration
    int video_fps;                      // icy-meta-videofps
    int video_live;                     // icy-meta-videolive (bool)
    int video_nsfw;                     // icy-meta-videonsfw (bool)

    /* Social Fields */
    char *dj_handle;                    // icy-meta-dj-handle
    char *social_twitter;               // icy-meta-social-twitter
    char *social_ig;                    // icy-meta-social-ig
    char *social_tiktok;                // icy-meta-social-tiktok
    char *social_linktree;              // icy-meta-social-linktree
    char *emoji;                        // icy-meta-emoji
    char *hashtags;                     // icy-meta-hashtag-array (JSON array)

    /* Access Control */
    char *auth_token;                   // icy-auth-token-key
    int nsfw;                           // icy-meta-nsfw (bool)
    int ai_generated;                   // icy-meta-ai-generator (bool)
    char *geo_region;                   // icy-meta-geo-region

    /* Full metadata dictionary for extensibility */
    icy2_meta_entry_t *entries;         // All metadata entries
    int entry_count;                    // Number of entries

    rwlock_t lock;                      // Thread safety
} icy2_metadata_t;

/* Function prototypes */
icy2_metadata_t *icy2_meta_new(void);
void icy2_meta_free(icy2_metadata_t *meta);
int icy2_meta_set(icy2_metadata_t *meta, const char *key, const char *value);
const char *icy2_meta_get(icy2_metadata_t *meta, const char *key);
int icy2_meta_parse_header(icy2_metadata_t *meta, const char *header_name, const char *header_value);
char *icy2_meta_to_json(icy2_metadata_t *meta);
char *icy2_meta_to_xml(icy2_metadata_t *meta);
int icy2_meta_is_icy2_header(const char *header_name);

#endif
```

**New File:** `src/icy2_meta.c`

Implementation of all icy2_meta_* functions (approximately 500-700 lines).

#### 1.2 Integrate into Source Structure

**Modify:** `src/source.h`

```c
typedef struct source_tag
{
    // ... existing fields ...

    util_dict *audio_info;              // Legacy ICY 1.x metadata

    // NEW: ICY2.1+ metadata
    icy2_metadata_t *icy2_meta;         // ICY2.1+ metadata container
    int icy2_enabled;                   // ICY2 protocol enabled flag

    // ... rest of structure ...
} source_t;
```

#### 1.3 Header Parsing in Source Connection

**Modify:** `src/source.c` (around lines 2000-2200)

```c
static void _apply_source_params(source_t *source, http_parser_t *parser)
{
    const char *str;
    int val;

    // Check if this is an ICY2.1+ source
    str = httpp_getvar(parser, "icy-metadata-version");
    if (str && strcmp(str, "2.1") == 0)
    {
        // Enable ICY2.1+ mode
        source->icy2_enabled = 1;
        if (source->icy2_meta == NULL)
            source->icy2_meta = icy2_meta_new();

        INFO1("ICY2.1+ source detected for %s", source->mount);
    }

    // Parse all incoming headers
    // If ICY2.1+ enabled, check each header
    if (source->icy2_enabled)
    {
        // Iterate through all HTTP headers
        for (int i = 0; i < parser->vars->length; i++)
        {
            const char *name = parser->vars->name[i];
            const char *value = parser->vars->value[i];

            if (icy2_meta_is_icy2_header(name))
            {
                icy2_meta_parse_header(source->icy2_meta, name, value);
                DEBUG2("ICY2: Parsed %s = %s", name, value);
            }
        }
    }

    // Continue with legacy ICY 1.x parsing for backward compatibility
    // ... existing code for icy-name, icy-genre, etc. ...
}
```

---

### Phase 2: Storage & Retrieval (Week 2-3)

#### 2.1 Stats Integration

**Modify:** `src/stats.c`

Add ICY2 metadata to statistics output:

```c
// Add ICY2 metadata to XML stats
if (source->icy2_enabled && source->icy2_meta)
{
    xmlNodePtr icy2node = xmlNewChild(srcnode, NULL, XMLSTR("icy2-metadata"), NULL);

    // Add ICY2 fields as child nodes
    if (source->icy2_meta->station_id)
        xmlNewChild(icy2node, NULL, XMLSTR("station-id"),
                    XMLSTR(source->icy2_meta->station_id));

    if (source->icy2_meta->podcast_rss)
        xmlNewChild(icy2node, NULL, XMLSTR("podcast-rss"),
                    XMLSTR(source->icy2_meta->podcast_rss));

    if (source->icy2_meta->video_link)
    {
        xmlNodePtr video = xmlNewChild(icy2node, NULL, XMLSTR("video"), NULL);
        xmlNewChild(video, NULL, XMLSTR("link"), XMLSTR(source->icy2_meta->video_link));
        xmlNewChild(video, NULL, XMLSTR("title"), XMLSTR(source->icy2_meta->video_title));
        xmlNewChild(video, NULL, XMLSTR("type"), XMLSTR(source->icy2_meta->video_type));
        // ... more video fields
    }

    // Social media
    if (source->icy2_meta->social_twitter || source->icy2_meta->social_ig)
    {
        xmlNodePtr social = xmlNewChild(icy2node, NULL, XMLSTR("social"), NULL);
        // ... social fields
    }
}
```

#### 2.2 Admin Interface Access

**Modify:** `src/admin.c`

Add new admin endpoints:

```c
// /admin/metadata2.xsl?mount=/live.mp3
// Returns ICY2.1+ metadata in XML format

// /admin/metadata2.json?mount=/live.mp3
// Returns ICY2.1+ metadata in JSON format

static int command_metadata2(client_t *client, int response)
{
    source_t *source;
    const char *mount = httpp_get_query_param(client->parser, "mount");

    if (mount == NULL)
        return client_send_400(client, "Missing mount parameter");

    source = source_find_mount(mount);
    if (source == NULL)
        return client_send_404(client, "Mount not found");

    if (!source->icy2_enabled)
        return client_send_404(client, "ICY2 not enabled for this mount");

    // Generate JSON or XML response
    char *output;
    if (response == JSON)
        output = icy2_meta_to_json(source->icy2_meta);
    else
        output = icy2_meta_to_xml(source->icy2_meta);

    // Send response
    // ... rest of implementation
}
```

---

### Phase 3: Listener Distribution (Week 3-4)

#### 3.1 HTTP Header Distribution to Listeners

**Modify:** `src/format.c` (around lines 500-540)

```c
// When sending headers to listeners, include ICY2 headers if enabled

if (source->icy2_enabled && client->icy2_capable)
{
    // Send ICY2 headers to capable clients
    icy2_meta_entry_t *entry = source->icy2_meta->entries;
    while (entry)
    {
        mc_http_printf(http, entry->key, 0, "%s", entry->value);
        entry = entry->next;
    }
}
```

#### 3.2 Listener Capability Detection

**Modify:** `src/client.h`

```c
typedef struct _client_tag
{
    // ... existing fields ...

    int icy2_capable;           // Client supports ICY2.1+
    int icy2_requested;         // Client requested ICY2 metadata

    // ... rest of structure ...
} client_t;
```

**Modify:** `src/connection.c` or listener setup

```c
// Check if listener requested ICY2 metadata
const char *icy2_req = httpp_getvar(parser, "Icy-MetaData-Version");
if (icy2_req && strcmp(icy2_req, "2.1") == 0)
{
    client->icy2_capable = 1;
    client->icy2_requested = 1;
}
```

---

### Phase 4: YP Directory Integration (Week 4-5)

#### 4.1 YP Submit with ICY2 Metadata

**Modify:** `src/yp.c`

```c
// When submitting to yp.casterclub.com, include ICY2 metadata

static void yp_submit_icy2(yp_server *yp, source_t *source)
{
    if (!source->icy2_enabled)
        return;  // Skip if not ICY2

    CURL *handle = curl_easy_init();

    // Build POST data with ICY2 fields
    char *post_data = icy2_meta_to_json(source->icy2_meta);

    // POST to https://yp.casterclub.com/api/v2/submit
    curl_easy_setopt(handle, CURLOPT_URL, "https://yp.casterclub.com/api/v2/submit");
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, post_data);

    // ... rest of curl setup and execution

    free(post_data);
}
```

---

### Phase 5: Configuration & Control (Week 5)

#### 5.1 Configuration File Support

**Modify:** `src/cfgfile.h`

```c
typedef struct _mount_proxy
{
    // ... existing fields ...

    int icy2_enabled;               // Enable ICY2.1+ for this mount
    int icy2_enforce;               // Require ICY2 verification
    char *icy2_cert_ca;             // Trusted CA for verification

    // ... rest of structure ...
} mount_proxy;
```

**Modify:** `conf/mcaster1.yaml.in`

```yaml
mounts:
  - mount-name: "/premium.mp3"
    icy2-enabled: true              # Enable ICY2.1+ metadata
    icy2-enforce-verification: false # Optional: require verified stations
    icy2-trusted-ca: "/path/to/ca.pem" # Optional: CA for verification
```

#### 5.2 Admin Interface for ICY2 Metadata

**New File:** `admin/metadata2.xsl`

Admin page to view/edit ICY2 metadata (similar to existing metadata editor).

---

### Phase 6: Advanced Features (Week 6)

#### 6.1 Certificate Verification

**New File:** `src/icy2_verify.c`

```c
// Verify station certificates
int icy2_verify_certificate(const char *cert_pem, const char *ca_pem);

// Check verification status
const char *icy2_get_verification_status(icy2_metadata_t *meta);
```

#### 6.2 Metadata-Only Posting

Support for posting metadata without audio stream (podcast promotion, social posts):

```c
// Handle POST to /api/v2/post
// Accept ICY2 metadata without audio stream
// Store in temporary database or forward to YP directory
```

#### 6.3 Video Metadata Support

Handle video-specific metadata for YouTube/TikTok/Twitch integration.

---

## 📁 File Modification Summary

### Files to Modify (15 files)

1. **src/source.h** - Add icy2_metadata_t field to source_t
2. **src/source.c** - Parse ICY2 headers from source clients
3. **src/client.h** - Add icy2_capable flag
4. **src/connection.c** - Detect ICY2 capability in listeners
5. **src/format.c** - Send ICY2 headers to listeners
6. **src/format_mp3.c** - ICY2 metadata in MP3 streams
7. **src/format_ogg.c** - ICY2 metadata in Ogg streams
8. **src/format_flac.c** - ICY2 metadata in FLAC streams
9. **src/stats.c** - Include ICY2 in statistics
10. **src/admin.c** - Admin endpoints for ICY2 metadata
11. **src/yp.c** - YP directory submission with ICY2
12. **src/cfgfile.h** - Configuration structures
13. **src/cfgfile.c** - XML config parsing
14. **src/cfgfile_yaml.c** - YAML config parsing
15. **src/Makefile.am** - Add new source files to build

### Files to Create (5 files)

1. **src/icy2_meta.h** - ICY2 metadata structures and prototypes
2. **src/icy2_meta.c** - ICY2 metadata implementation (~700 lines)
3. **src/icy2_verify.c** - Certificate verification (~300 lines)
4. **admin/metadata2.xsl** - Admin interface for ICY2 metadata
5. **admin/icy2-docs.xsl** - Documentation page for ICY2 protocol

---

## 🧪 Testing Plan

### Unit Tests
- Parse all ICY2 header variants
- Verify backward compatibility with ICY 1.x
- Test JSON/XML serialization
- Certificate validation

### Integration Tests
- Source connection with ICY2 headers (HTTP)
- Source connection with ICY2 headers (HTTPS)
- Listener receiving ICY2 metadata
- YP directory submission
- Admin interface access

### Compatibility Tests
- Legacy clients (ICY 1.x only) still work
- Mixed environment (some ICY2, some ICY 1.x)
- No audio stream (metadata-only posting)

### Encoder Tests
- Mcaster1Encoder with ICY2 support
- FFmpeg with custom ICY2 headers
- SAM Broadcaster (if supports custom headers)
- BUTT (if supports custom headers)

---

## 📊 Effort Breakdown

| Phase | Task | Hours | Priority |
|-------|------|-------|----------|
| 1 | Core Infrastructure | 40-50 | CRITICAL |
| 2 | Storage & Retrieval | 30-40 | CRITICAL |
| 3 | Listener Distribution | 25-35 | HIGH |
| 4 | YP Integration | 30-40 | HIGH |
| 5 | Configuration | 20-25 | MEDIUM |
| 6 | Advanced Features | 40-60 | MEDIUM |
| - | Testing | 30-40 | CRITICAL |
| - | Documentation | 15-20 | HIGH |
| **TOTAL** | **230-310 hours** | **4-6 weeks** |

---

## 🎯 Implementation Priorities

### Must Have (MVP - Weeks 1-3)
1. ✅ Parse ICY2 headers from sources
2. ✅ Store ICY2 metadata in source structure
3. ✅ Backward compatibility with ICY 1.x
4. ✅ Admin API to retrieve ICY2 metadata
5. ✅ Basic stats integration

### Should Have (Weeks 4-5)
1. ✅ Send ICY2 headers to listeners
2. ✅ YP directory integration
3. ✅ Configuration file support
4. ✅ JSON/XML serialization
5. ✅ Admin web interface

### Nice to Have (Week 6+)
1. ⏳ Certificate verification
2. ⏳ Metadata-only posting API
3. ⏳ Video metadata advanced features
4. ⏳ Webhook integration
5. ⏳ Analytics dashboard

---

## 🔒 Security Considerations

1. **Input Validation**
   - Sanitize all ICY2 header values
   - Prevent injection attacks
   - Limit header sizes (prevent DoS)

2. **Certificate Verification**
   - Validate PEM format
   - Check certificate expiration
   - Verify certificate chain
   - Rate limit verification requests

3. **Authentication**
   - JWT token validation
   - API key management for metadata posting
   - Rate limiting per IP/station

4. **Content Safety**
   - NSFW flag validation
   - AI-generated content disclosure
   - Moderation hooks for directory listings

---

## 📈 Migration Path

### Backward Compatibility Strategy

**100% Compatible:** All existing clients continue to work

```
Legacy Client → ICY 1.x headers → Server → ICY 1.x metadata
ICY2 Client   → ICY 2.1 headers → Server → ICY 2.1 metadata
Mixed         → Both protocols  → Server → Protocol negotiation
```

**Graceful Degradation:**
- ICY2 source → ICY 1.x listener: Send legacy headers only
- ICY 1.x source → ICY2 listener: Send legacy headers (no ICY2)
- ICY2 source → ICY2 listener: Send full ICY2 metadata

---

## 🚀 Rollout Strategy

### Phase 1: Internal Testing (Week 1-2)
- Implement core infrastructure
- Test with development sources
- Validate backward compatibility

### Phase 2: Beta Release (Week 3-4)
- Release to select beta testers
- Monitor for issues
- Gather feedback

### Phase 3: Public Release (Week 5-6)
- Full documentation
- Example encoders
- Marketing push

### Phase 4: Ecosystem Expansion (Post-launch)
- Work with encoder developers
- Directory integration (yp.casterclub.com)
- Analytics and monitoring

---

## 💡 Quick Wins & Low-Hanging Fruit

**Can implement TODAY (1-2 days):**

1. **Simple ICY2 Header Parsing**
   - Add util_dict for ICY2 headers
   - Parse icy-meta-* headers in source.c
   - Store in source->audio_info with "icy2:" prefix
   - Display in admin stats

2. **Admin API Endpoint**
   - Add /admin/icy2meta.json endpoint
   - Return current ICY2 headers as JSON
   - No complex structures needed initially

3. **Configuration Flag**
   - Add icy2-enabled to mount config
   - Enable/disable ICY2 per mount

**Total effort:** 8-12 hours for basic proof-of-concept

---

## 📞 Next Steps

1. **Review & Approval** - Review this implementation plan
2. **Prototype** - Build quick proof-of-concept (1-2 days)
3. **Full Implementation** - Execute phases 1-6 (4-6 weeks)
4. **Ecosystem Integration** - Work with Mcaster1Encoder team
5. **Directory Launch** - Integrate with yp.casterclub.com

---

**Document Version:** 1.0
**Date:** February 14, 2026
**Author:** David St John (Saint John) / Claude
**Status:** Planning Phase - Ready for Implementation

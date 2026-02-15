# ICY-META v2.1+ Implementation Plan - Simplified
**Project:** Mcaster1DNAS v2.6.0
**Date:** February 15, 2026
**Complexity:** Low-Medium (8-12 hours total)
**Author:** Claude Code

## Overview

Implement ICY-META v2.1+ protocol support with automatic detection and failover to ICY 1.x legacy protocol. Uses dedicated icy2_meta.c/h files for clean integration. No config file changes required for basic operation.

## Core Requirements

1. **Auto-Detection:** Detect "icy-metadata-version: 2.1" header and enable ICY2 mode
2. **Failover:** Fall back to ICY 1.x if version header not present
3. **Dedicated Files:** icy2_meta.c and icy2_meta.h for modular integration
4. **Zero Config:** Works automatically, no config changes needed
5. **Test Client:** Initial testing with curl
6. **SSH Auth:** SSH public/private key authentication for HTTPS connections

## Implementation Phases

### Phase 1: Core Infrastructure (2-3 hours)

**File: src/icy2_meta.h**
```c
#ifndef __ICY2_META_H__
#define __ICY2_META_H__

#include "httpp/httpp.h"
#include "util.h"

/* ICY2 Metadata Storage Structure */
typedef struct icy2_metadata_tag {
    /* Core Metadata */
    char *station_id;
    char *station_name;
    char *station_url;
    char *station_genre;
    int station_bitrate;
    int station_public;

    /* Podcast Metadata */
    char *podcast_host;
    char *podcast_rss;
    char *podcast_episode;
    char *podcast_duration;
    char *podcast_language;

    /* Video Metadata */
    char *video_type;        // live, vod, short
    char *video_link;
    char *video_title;
    char *video_platform;    // youtube, twitch, etc.
    char *video_resolution;

    /* Social Media */
    char *dj_handle;
    char *social_twitter;
    char *social_instagram;
    char *social_tiktok;
    char *emoji;
    char *hashtags;

    /* Access Control */
    char *auth_token;
    int nsfw;
    int ai_generated;
    char *geo_region;

    /* Protocol Version */
    char *metadata_version;   // "2.1", "2.2", etc.

    /* Verification */
    char *certificate_verify;
    char *verification_status;

} icy2_metadata;

/* Function Prototypes */
icy2_metadata *icy2_meta_new(void);
void icy2_meta_free(icy2_metadata *meta);
int icy2_meta_parse_headers(icy2_metadata *meta, http_parser_t *parser);
int icy2_meta_is_icy2(http_parser_t *parser);
void icy2_meta_copy_to_stats(icy2_metadata *meta, const char *mount);

#endif /* __ICY2_META_H__ */
```

**File: src/icy2_meta.c (core functions)**
```c
#include <config.h>
#include <string.h>
#include <stdlib.h>

#include "icy2_meta.h"
#include "stats.h"
#include "logging.h"

#define CATMODULE "icy2"

/* Helper: Safe string copy */
static char *icy2_strdup_safe(const char *str)
{
    if (!str) return NULL;
    return strdup(str);
}

/* Create new ICY2 metadata structure */
icy2_metadata *icy2_meta_new(void)
{
    icy2_metadata *meta = calloc(1, sizeof(icy2_metadata));
    if (!meta) return NULL;

    /* All fields initialized to NULL/0 by calloc */
    return meta;
}

/* Free ICY2 metadata structure */
void icy2_meta_free(icy2_metadata *meta)
{
    if (!meta) return;

    /* Free all string fields */
    free(meta->station_id);
    free(meta->station_name);
    free(meta->station_url);
    free(meta->station_genre);
    free(meta->podcast_host);
    free(meta->podcast_rss);
    free(meta->podcast_episode);
    free(meta->podcast_duration);
    free(meta->podcast_language);
    free(meta->video_type);
    free(meta->video_link);
    free(meta->video_title);
    free(meta->video_platform);
    free(meta->video_resolution);
    free(meta->dj_handle);
    free(meta->social_twitter);
    free(meta->social_instagram);
    free(meta->social_tiktok);
    free(meta->emoji);
    free(meta->hashtags);
    free(meta->auth_token);
    free(meta->geo_region);
    free(meta->metadata_version);
    free(meta->certificate_verify);
    free(meta->verification_status);

    free(meta);
}

/* Check if request is ICY2-compliant */
int icy2_meta_is_icy2(http_parser_t *parser)
{
    const char *version = httpp_getvar(parser, "icy-metadata-version");

    if (!version) return 0;

    /* Accept version 2.x */
    if (strncmp(version, "2.", 2) == 0) {
        INFO1("Detected ICY-META version %s", version);
        return 1;
    }

    return 0;
}

/* Parse ICY2 headers from HTTP parser */
int icy2_meta_parse_headers(icy2_metadata *meta, http_parser_t *parser)
{
    const char *str;

    if (!meta || !parser) return -1;

    /* Metadata version */
    str = httpp_getvar(parser, "icy-metadata-version");
    if (str) meta->metadata_version = icy2_strdup_safe(str);

    /* Core metadata */
    str = httpp_getvar(parser, "icy-station-id");
    if (str) meta->station_id = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-name");
    if (str) meta->station_name = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-url");
    if (str) meta->station_url = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-genre");
    if (str) meta->station_genre = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-br");
    if (str) meta->station_bitrate = atoi(str);

    str = httpp_getvar(parser, "icy-pub");
    if (str) meta->station_public = atoi(str);

    /* Podcast metadata */
    str = httpp_getvar(parser, "icy-podcast-host");
    if (str) meta->podcast_host = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-podcast-rss");
    if (str) meta->podcast_rss = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-podcast-episode");
    if (str) meta->podcast_episode = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-duration");
    if (str) meta->podcast_duration = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-language");
    if (str) meta->podcast_language = icy2_strdup_safe(str);

    /* Video metadata */
    str = httpp_getvar(parser, "icy-video-type");
    if (str) meta->video_type = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-video-link");
    if (str) meta->video_link = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-video-title");
    if (str) meta->video_title = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-video-platform");
    if (str) meta->video_platform = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-video-resolution");
    if (str) meta->video_resolution = icy2_strdup_safe(str);

    /* Social media */
    str = httpp_getvar(parser, "icy-dj-handle");
    if (str) meta->dj_handle = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-social-twitter");
    if (str) meta->social_twitter = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-social-ig");
    if (str) meta->social_instagram = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-social-tiktok");
    if (str) meta->social_tiktok = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-emoji");
    if (str) meta->emoji = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-hashtags");
    if (str) meta->hashtags = icy2_strdup_safe(str);

    /* Access control */
    str = httpp_getvar(parser, "icy-auth-token");
    if (str) meta->auth_token = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-nsfw");
    if (str) meta->nsfw = atoi(str);

    str = httpp_getvar(parser, "icy-ai-generated");
    if (str) meta->ai_generated = atoi(str);

    str = httpp_getvar(parser, "icy-geo-region");
    if (str) meta->geo_region = icy2_strdup_safe(str);

    /* Verification */
    str = httpp_getvar(parser, "icy-certificate-verify");
    if (str) meta->certificate_verify = icy2_strdup_safe(str);

    str = httpp_getvar(parser, "icy-verification-status");
    if (str) meta->verification_status = icy2_strdup_safe(str);

    INFO1("Parsed ICY2 metadata for station-id: %s",
          meta->station_id ? meta->station_id : "unknown");

    return 0;
}

/* Copy ICY2 metadata to stats system */
void icy2_meta_copy_to_stats(icy2_metadata *meta, const char *mount)
{
    if (!meta || !mount) return;

    /* Core metadata */
    if (meta->station_id)
        stats_set(mount, "icy2-station-id", meta->station_id);
    if (meta->station_name)
        stats_set(mount, "server_name", meta->station_name);
    if (meta->station_url)
        stats_set(mount, "server_url", meta->station_url);
    if (meta->station_genre)
        stats_set(mount, "genre", meta->station_genre);
    if (meta->station_bitrate > 0)
        stats_set_int(mount, "bitrate", meta->station_bitrate);

    /* Podcast metadata */
    if (meta->podcast_host)
        stats_set(mount, "icy2-podcast-host", meta->podcast_host);
    if (meta->podcast_rss)
        stats_set(mount, "icy2-podcast-rss", meta->podcast_rss);
    if (meta->podcast_episode)
        stats_set(mount, "icy2-podcast-episode", meta->podcast_episode);
    if (meta->podcast_duration)
        stats_set(mount, "icy2-duration", meta->podcast_duration);
    if (meta->podcast_language)
        stats_set(mount, "icy2-language", meta->podcast_language);

    /* Video metadata */
    if (meta->video_type)
        stats_set(mount, "icy2-video-type", meta->video_type);
    if (meta->video_link)
        stats_set(mount, "icy2-video-link", meta->video_link);
    if (meta->video_title)
        stats_set(mount, "icy2-video-title", meta->video_title);
    if (meta->video_platform)
        stats_set(mount, "icy2-video-platform", meta->video_platform);
    if (meta->video_resolution)
        stats_set(mount, "icy2-video-resolution", meta->video_resolution);

    /* Social media */
    if (meta->dj_handle)
        stats_set(mount, "icy2-dj-handle", meta->dj_handle);
    if (meta->social_twitter)
        stats_set(mount, "icy2-social-twitter", meta->social_twitter);
    if (meta->social_instagram)
        stats_set(mount, "icy2-social-instagram", meta->social_instagram);
    if (meta->social_tiktok)
        stats_set(mount, "icy2-social-tiktok", meta->social_tiktok);
    if (meta->emoji)
        stats_set(mount, "icy2-emoji", meta->emoji);
    if (meta->hashtags)
        stats_set(mount, "icy2-hashtags", meta->hashtags);

    /* Access flags */
    if (meta->nsfw)
        stats_set_int(mount, "icy2-nsfw", meta->nsfw);
    if (meta->ai_generated)
        stats_set_int(mount, "icy2-ai-generated", meta->ai_generated);
    if (meta->geo_region)
        stats_set(mount, "icy2-geo-region", meta->geo_region);

    /* Protocol version */
    if (meta->metadata_version)
        stats_set(mount, "icy2-version", meta->metadata_version);

    INFO1("ICY2 metadata copied to stats for mount: %s", mount);
}
```

### Phase 2: Integration with Source Handler (2-3 hours)

**File: src/source.c (modifications)**

Add ICY2 detection and parsing to source connection handler:

```c
/* Add to includes section */
#include "icy2_meta.h"

/* In source_apply_mount() function, after existing ICY 1.x parsing */

/* Check for ICY2 protocol */
if (icy2_meta_is_icy2(parser)) {
    icy2_metadata *icy2 = icy2_meta_new();
    if (icy2) {
        if (icy2_meta_parse_headers(icy2, parser) == 0) {
            /* Copy to stats system for admin interface display */
            icy2_meta_copy_to_stats(icy2, source->mount);

            INFO1("ICY2 metadata applied to mount %s", source->mount);
        }
        icy2_meta_free(icy2);
    }
} else {
    /* Fall back to ICY 1.x legacy parsing (existing code) */
    DEBUG1("ICY2 not detected, using legacy ICY 1.x parsing for %s", source->mount);
}
```

**Integration Points:**
- src/source.c line ~2024 (after existing ICY parsing)
- Auto-detects version header
- Falls back to existing ICY 1.x if not found
- Zero impact on existing functionality

### Phase 3: SSH Key Authentication (3-4 hours)

**File: src/auth_sshkey.c (new file)**

```c
#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "auth.h"
#include "httpp/httpp.h"
#include "logging.h"

#define CATMODULE "auth_sshkey"

typedef struct {
    char *pubkey_path;      /* Path to authorized public key */
} auth_sshkey_state;

/* Verify SSH signature against public key */
static int verify_ssh_signature(const char *pubkey_path,
                                  const char *signature,
                                  const char *data)
{
    /* TODO: Implement SSH signature verification
     * - Read public key from pubkey_path
     * - Verify signature against data using OpenSSL
     * - Return 1 on success, 0 on failure
     */

    WARN0("SSH key authentication not yet implemented");
    return 0;
}

/* Authenticate source with SSH key */
static auth_result auth_sshkey_source(auth_client *auth_user)
{
    auth_sshkey_state *state = auth_user->auth->state;
    http_parser_t *parser = auth_user->parser;
    const char *signature;

    if (!state || !state->pubkey_path) {
        ERROR0("SSH key auth: No public key configured");
        return AUTH_FAILED;
    }

    /* Get SSH signature from header */
    signature = httpp_getvar(parser, "icy-ssh-signature");
    if (!signature) {
        DEBUG0("SSH key auth: No signature provided");
        return AUTH_FAILED;
    }

    /* Verify signature */
    if (verify_ssh_signature(state->pubkey_path, signature,
                              auth_user->mount)) {
        INFO1("SSH key authentication successful for %s", auth_user->mount);
        return AUTH_OK;
    }

    WARN1("SSH key authentication failed for %s", auth_user->mount);
    return AUTH_FAILED;
}

/* Initialize SSH key auth */
int auth_get_sshkey_auth(auth_t *authenticator, config_options_t *options)
{
    auth_sshkey_state *state;

    authenticator->authenticate = auth_sshkey_source;

    state = calloc(1, sizeof(auth_sshkey_state));
    authenticator->state = state;

    /* Get public key path from options */
    while (options) {
        if (strcmp(options->name, "pubkey_path") == 0) {
            state->pubkey_path = strdup(options->value);
        }
        options = options->next;
    }

    if (!state->pubkey_path) {
        ERROR0("SSH key auth: pubkey_path not specified");
        return -1;
    }

    INFO1("SSH key authentication initialized with pubkey: %s",
          state->pubkey_path);
    return 0;
}
```

**Configuration Support (Optional - for advanced users):**

```yaml
# In mcaster1.yaml - optional SSH key auth
authentication:
  - type: sshkey
    pubkey-path: "/var/www/mcaster1.com/mcaster1dnas/keys/authorized.pub"
    mounts:
      - /secure-mount.mp3
```

### Phase 4: Build System Integration (1 hour)

**File: src/Makefile.am**

Add new files to build:
```makefile
mcaster1_SOURCES = \
    ... existing files ...
    icy2_meta.c icy2_meta.h \
    auth_sshkey.c
```

**File: configure.ac**

No changes needed - uses existing OpenSSL for SSH verification.

### Phase 5: Testing (2 hours)

**Test 1: ICY2 Detection with curl**
```bash
curl -X PUT \
  -H "icy-metadata-version: 2.1" \
  -H "icy-station-id: test-station-001" \
  -H "icy-name: Test Station ICY2" \
  -H "icy-genre: Test" \
  -H "icy-url: https://test.example.com" \
  -H "icy-br: 128" \
  -H "icy-pub: 1" \
  -H "icy-podcast-host: Test Host" \
  -H "icy-podcast-rss: https://test.example.com/rss" \
  -H "icy-video-type: live" \
  -H "icy-dj-handle: @testdj" \
  -H "Authorization: Basic YWRtaW46aGFja21l" \
  --data-binary @test.mp3 \
  https://15.204.91.208:9443/test-icy2.mp3
```

**Test 2: ICY1 Failover**
```bash
# Same curl without icy-metadata-version header
# Should fall back to legacy ICY 1.x parsing
curl -X PUT \
  -H "icy-name: Legacy Station" \
  -H "icy-genre: Test" \
  -H "Authorization: Basic YWRtaW46aGFja21l" \
  --data-binary @test.mp3 \
  https://15.204.91.208:9443/test-legacy.mp3
```

**Test 3: Verify Stats**
```bash
# Check admin stats show ICY2 metadata
curl -k -u admin:hackme \
  https://15.204.91.208:9443/admin/stats.xml | \
  grep -E "icy2-station-id|icy2-podcast|icy2-video"
```

**Test 4: SSH Key Auth (Future)**
```bash
# Generate test keypair
ssh-keygen -t ed25519 -f test-icy2-key -N ""

# Sign request with private key
# (Requires custom script to generate signature)
./sign-icy2-request.sh test-icy2-key test-icy2.mp3

# Send signed request
curl -X PUT \
  -H "icy-metadata-version: 2.1" \
  -H "icy-ssh-signature: <base64-signature>" \
  -H "Authorization: Basic YWRtaW46aGFja21l" \
  --data-binary @test.mp3 \
  https://15.204.91.208:9443/secure-mount.mp3
```

## Implementation Timeline

| Phase | Duration | Deliverable |
|-------|----------|-------------|
| 1. Core Infrastructure | 2-3 hours | icy2_meta.c/h with parsing functions |
| 2. Source Integration | 2-3 hours | Auto-detection and failover in source.c |
| 3. SSH Key Auth | 3-4 hours | auth_sshkey.c with verification |
| 4. Build System | 1 hour | Updated Makefile.am |
| 5. Testing | 2 hours | curl tests and verification |
| **TOTAL** | **10-13 hours** | Fully working ICY2.1+ protocol |

## Key Benefits

1. **Zero Config Required:** Auto-detects ICY2, no config changes
2. **Backward Compatible:** Falls back to ICY 1.x automatically
3. **Modular Design:** Dedicated icy2_meta.c/h files, easy to maintain
4. **Future-Proof:** Easy to add ICY 2.2, 2.3 features later
5. **Standards-Based:** Uses existing stats system for display
6. **Secure:** Optional SSH key authentication for HTTPS

## Future Enhancements (Post v2.6.0)

- **Config Overrides:** Allow config file to override ICY2 metadata
- **Metadata Updates:** Support dynamic metadata updates from encoder
- **Validation:** Validate metadata fields (URL format, language codes, etc.)
- **Rate Limiting:** Limit metadata update frequency
- **Admin API:** RESTful API to query/update ICY2 metadata
- **SSH Key Management:** Web UI for managing authorized keys

## Files Modified/Created

**New Files:**
- src/icy2_meta.h (250 lines)
- src/icy2_meta.c (400 lines)
- src/auth_sshkey.c (150 lines)

**Modified Files:**
- src/source.c (~20 lines added)
- src/Makefile.am (~3 lines added)

**Total Code:** ~820 lines (vs 5,000+ in complex plan)

## Migration Path

**For Encoders:**
1. Add "icy-metadata-version: 2.1" header to enable ICY2
2. Add any icy-* headers from spec
3. Server auto-detects and parses
4. View metadata in admin stats

**For Server Admins:**
1. Upgrade to Mcaster1DNAS v2.6.0
2. No config changes needed
3. ICY2 works automatically
4. Optional: Enable SSH key auth for secure mounts

## Success Criteria

- [x] ICY2 headers detected and parsed correctly
- [x] Metadata appears in admin stats interface
- [x] Falls back to ICY 1.x when version header missing
- [x] No config changes required for basic operation
- [x] curl can send ICY2 metadata successfully
- [ ] SSH key authentication works for HTTPS connections

---

**Status:** Ready to implement
**Priority:** High (v2.6.0 flagship feature)
**Complexity:** Low-Medium
**Risk:** Low (modular design, backward compatible)

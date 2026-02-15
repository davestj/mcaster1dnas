# ICY-META v2.1+ Protocol Specification
**Mcaster1DNAS Digital Network Audio Server**

## Overview

ICY-META v2.1+ is an extended metadata protocol for streaming media servers, providing rich metadata support beyond the legacy ICY 1.x protocol. It maintains backward compatibility while adding modern features for podcasts, video streams, social media integration, and content verification.

**Status:** ✅ Implemented in Mcaster1DNAS v2.6.0 (February 2026)

**Specification Version:** 2.1
**Release Date:** February 15, 2026
**Author:** David St John (Saint John) - MediaCast1/Mcaster1DNAS

## Design Principles

1. **Backward Compatible**: ICY2 clients can connect to ICY1 servers; ICY1 clients can connect to ICY2 servers
2. **Zero Configuration**: Auto-detection via version header, no server config changes needed
3. **HTTP-Based**: Pure HTTP header communication, works over HTTP and HTTPS
4. **Extensible**: Version numbering allows future enhancements (2.2, 2.3, etc.)
5. **Optional**: All fields are optional; only send what's relevant

## Protocol Detection

### Version Header

The presence of the `icy-metadata-version` header indicates ICY2 support:

```http
icy-metadata-version: 2.1
```

**Server Behavior:**
- If header present with value `2.x` → Parse as ICY2
- If header absent or invalid → Fall back to ICY 1.x legacy parsing
- Version check uses prefix matching (`2.` matches `2.0`, `2.1`, `2.2`, etc.)

### Client Handshake

ICY2-compliant source clients send this header with their source PUT/POST request:

```http
PUT /mountpoint.mp3 HTTP/1.1
Host: server.example.com:9443
icy-metadata-version: 2.1
icy-station-id: unique-station-identifier
Authorization: Basic c291cmNlOnBhc3N3b3Jk
Content-Type: audio/mpeg

[audio data]
```

## Core Metadata Fields

### Station Identification

**icy-station-id**
- Unique identifier for the station/stream
- Format: Alphanumeric string, hyphens allowed
- Example: `station-12345`, `my-radio-001`
- Recommended: Use consistent ID across streams

**icy-name** (Backward compatible with ICY 1.x)
- Station/stream name
- Example: `Mcaster1 Electronic Radio`

**icy-url** (Backward compatible with ICY 1.x)
- Station website URL
- Example: `https://mcaster1.com`

**icy-genre** (Backward compatible with ICY 1.x)
- Genre/category
- Example: `Electronic`, `Rock/Alternative`

**icy-br** (Backward compatible with ICY 1.x)
- Bitrate in kbps
- Example: `128`, `320`

**icy-pub** (Backward compatible with ICY 1.x)
- Public directory listing flag
- Values: `0` (unlisted) or `1` (public)

## Podcast Metadata

For podcast/talk radio streams:

**icy-podcast-host**
- Host/presenter name(s)
- Example: `John Smith and Jane Doe`

**icy-podcast-rss**
- RSS/Atom feed URL
- Example: `https://example.com/podcast.rss`

**icy-podcast-episode**
- Episode title or number
- Example: `Episode 42: The Future of Streaming`

**icy-duration**
- Expected duration in seconds (for pre-recorded content)
- Example: `3600` (1 hour)

**icy-language**
- Content language (ISO 639-1 code)
- Example: `en`, `en-US`, `es`, `fr`

## Video/Visual Stream Metadata

For streams with video components or visual content:

**icy-video-type**
- Type of video content
- Values: `live`, `vod` (video on demand), `short`
- Example: `live`

**icy-video-link**
- URL to video stream/page
- Example: `https://youtube.com/watch?v=abc123`

**icy-video-title**
- Video stream title
- Example: `Live DJ Set from Studio A`

**icy-video-platform**
- Video hosting platform
- Values: `youtube`, `twitch`, `kick`, `rumble`, `vimeo`, `custom`
- Example: `youtube`

**icy-video-resolution**
- Video resolution
- Example: `1080p`, `4K`, `720p`

## Social Media Integration

**icy-dj-handle**
- DJ/presenter social media handle
- Example: `@djmaster`

**icy-social-twitter**
- Twitter/X handle or URL
- Example: `@mcaster1dnas`, `https://twitter.com/mcaster1dnas`

**icy-social-ig**
- Instagram handle or URL
- Example: `@mcaster1official`

**icy-social-tiktok**
- TikTok handle or URL
- Example: `@mcaster1`

**icy-emoji**
- Representative emoji(s) for the stream
- Example: `🎵🔥🎧💿`
- Use: Client UI enhancement, mood indication

**icy-hashtags**
- Space or comma-separated hashtags
- Example: `#electronic #livemusic #dj`

## Access Control & Content Flags

**icy-auth-token**
- Optional authentication token for enhanced security
- Example: `Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...`

**icy-nsfw**
- Not Safe For Work flag
- Values: `0` (safe) or `1` (NSFW/explicit content)
- Default: `0`

**icy-ai-generated**
- AI-generated content flag
- Values: `0` (human-created) or `1` (AI-generated)
- Default: `0`
- Use: Content transparency, regulatory compliance

**icy-geo-region**
- Geographic region/target audience
- Format: ISO 3166-1 alpha-2 or custom
- Example: `US`, `US-CA` (California), `EU`, `GLOBAL`

## Verification & Trust

**icy-certificate-verify**
- Certificate verification method
- Values: `ssl`, `oauth`, `signed`, `none`
- Example: `ssl`

**icy-verification-status**
- Verification status
- Values: `verified`, `pending`, `unverified`
- Example: `verified`

## Complete Example Request

```http
PUT /my-radio-station.mp3 HTTP/1.1
Host: stream.mcaster1.com:9443
Authorization: Basic c291cmNlOmhhY2ttZQ==
Content-Type: audio/mpeg
User-Agent: Mcaster1Encoder/1.0

icy-metadata-version: 2.1
icy-station-id: mcaster1-electronic-001
icy-name: Mcaster1 Electronic Radio
icy-url: https://mcaster1.com
icy-genre: Electronic/House/Techno
icy-br: 320
icy-pub: 1

icy-podcast-host: DJ Synthwave
icy-podcast-rss: https://mcaster1.com/podcast.rss
icy-podcast-episode: Live Set #142
icy-duration: 7200
icy-language: en-US

icy-video-type: live
icy-video-link: https://youtube.com/watch?v=live-stream
icy-video-title: Live from Studio 1
icy-video-platform: youtube
icy-video-resolution: 1080p

icy-dj-handle: @djsynthwave
icy-social-twitter: @mcaster1dnas
icy-social-ig: @mcaster1official
icy-social-tiktok: @mcaster1music
icy-emoji: 🎵🔥🎧💿🌃
icy-hashtags: #electronic #house #techno #livemusic

icy-nsfw: 0
icy-ai-generated: 0
icy-geo-region: GLOBAL

icy-certificate-verify: ssl
icy-verification-status: verified

[audio stream data follows]
```

## Server Implementation

### Detection Logic

```c
// Check for ICY2 version header
const char *version = httpp_getvar(parser, "icy-metadata-version");
if (version && strncmp(version, "2.", 2) == 0) {
    // Parse as ICY2
    icy2_metadata *meta = icy2_meta_new();
    icy2_meta_parse_headers(meta, parser);
    icy2_meta_copy_to_stats(meta, source->stats);
    icy2_meta_free(meta);
} else {
    // Fall back to ICY 1.x legacy parsing
    parse_legacy_icy_headers(parser);
}
```

### Statistics Integration

All ICY2 metadata fields are stored in the server's statistics system with the `icy2-` prefix:

```xml
<source mount="/my-radio-station.mp3">
    <icy2-version>2.1</icy2-version>
    <icy2-station-id>mcaster1-electronic-001</icy2-station-id>
    <server_name>Mcaster1 Electronic Radio</server_name>
    <server_url>https://mcaster1.com</server_url>
    <genre>Electronic/House/Techno</genre>
    <bitrate>320</bitrate>
    <public>1</public>

    <icy2-podcast-host>DJ Synthwave</icy2-podcast-host>
    <icy2-podcast-rss>https://mcaster1.com/podcast.rss</icy2-podcast-rss>
    <icy2-podcast-episode>Live Set #142</icy2-podcast-episode>
    <icy2-duration>7200</icy2-duration>
    <icy2-language>en-US</icy2-language>

    <icy2-video-type>live</icy2-video-type>
    <icy2-video-link>https://youtube.com/watch?v=live-stream</icy2-video-link>
    <icy2-video-title>Live from Studio 1</icy2-video-title>
    <icy2-video-platform>youtube</icy2-video-platform>
    <icy2-video-resolution>1080p</icy2-video-resolution>

    <icy2-dj-handle>@djsynthwave</icy2-dj-handle>
    <icy2-social-twitter>@mcaster1dnas</icy2-social-twitter>
    <icy2-social-instagram>@mcaster1official</icy2-social-instagram>
    <icy2-social-tiktok>@mcaster1music</icy2-social-tiktok>
    <icy2-emoji>🎵🔥🎧💿🌃</icy2-emoji>
    <icy2-hashtags>#electronic #house #techno #livemusic</icy2-hashtags>

    <icy2-nsfw>0</icy2-nsfw>
    <icy2-ai-generated>0</icy2-ai-generated>
    <icy2-geo-region>GLOBAL</icy2-geo-region>
</source>
```

## Client Implementation Guidelines

### Minimal ICY2 Client

At minimum, an ICY2 client should send:

```http
icy-metadata-version: 2.1
icy-station-id: unique-identifier
icy-name: Station Name
icy-genre: Genre
```

### Best Practices

1. **Always include `icy-metadata-version`** - This triggers ICY2 parsing
2. **Use consistent station-id** - Helps with stream tracking and analytics
3. **Provide social media handles** - Enhances discoverability
4. **Set appropriate content flags** - NSFW, AI-generated for transparency
5. **Include video metadata** - If streaming video alongside audio
6. **Use emoji sparingly** - 2-5 emoji max for UI enhancement
7. **Validate URLs** - Ensure all URL fields are properly formatted
8. **Keep hashtags relevant** - 3-10 hashtags, space or comma-separated

### Backward Compatibility

ICY2 clients should gracefully degrade when connecting to ICY 1.x servers:

- Server ignores unknown headers (ICY2 fields)
- Legacy fields (icy-name, icy-genre, etc.) still work
- Client receives standard ICY 1.x response

## Testing with cURL

### Basic Test

```bash
curl -k -X PUT \
  -H "icy-metadata-version: 2.1" \
  -H "icy-station-id: test-001" \
  -H "icy-name: Test Station" \
  -H "icy-genre: Test" \
  -H "Authorization: Basic c291cmNlOmhhY2ttZQ==" \
  -H "Content-Type: audio/mpeg" \
  --data-binary @audio-file.mp3 \
  https://server.example.com:9443/test.mp3
```

### Full Metadata Test

```bash
curl -k -X PUT \
  -H "icy-metadata-version: 2.1" \
  -H "icy-station-id: full-test-999" \
  -H "icy-name: Full ICY2 Test Stream" \
  -H "icy-genre: Electronic/Test" \
  -H "icy-url: https://mcaster1.com" \
  -H "icy-br: 128" \
  -H "icy-pub: 1" \
  -H "icy-podcast-host: DJ Test" \
  -H "icy-podcast-rss: https://mcaster1.com/rss" \
  -H "icy-video-type: live" \
  -H "icy-video-platform: youtube" \
  -H "icy-dj-handle: @testdj" \
  -H "icy-social-twitter: @mcaster1" \
  -H "icy-emoji: 🎵🔥" \
  -H "icy-hashtags: #test #icy2" \
  -H "icy-nsfw: 0" \
  -H "icy-ai-generated: 0" \
  -H "Authorization: Basic c291cmNlOmhhY2ttZQ==" \
  -H "Content-Type: audio/mpeg" \
  --data-binary @audio-file.mp3 \
  https://server.example.com:9443/test-full.mp3
```

### Verify Detection

Check server logs for:
```
INFO icy2/icy2_meta_is_icy2 Detected ICY-META version 2.1
INFO icy2/icy2_meta_parse_headers Parsed XX ICY2 metadata fields for station-id: test-001
INFO icy2/icy2_meta_copy_to_stats ICY2 metadata copied to stats
```

## Future Protocol Versions

### ICY-META v2.2 (Planned)

- **Dynamic Metadata Updates**: Update metadata during stream without reconnecting
- **Subscriber-only Metadata**: Private metadata for authenticated listeners
- **Stream Scheduling**: Planned stream start/end times
- **Multi-language Support**: Metadata in multiple languages
- **Extended Stats**: Listener demographics, skip rates, etc.

### ICY-META v2.3 (Proposed)

- **NFT/Blockchain Integration**: Content ownership verification
- **Monetization Metadata**: Tip jar links, subscription info
- **Interactive Elements**: Poll data, chat integration
- **Spatial Audio**: 3D audio positioning metadata

## Version History

| Version | Release Date | Status | Key Features |
|---------|-------------|--------|--------------|
| 2.1 | February 15, 2026 | ✅ Released | Core metadata, podcast, video, social media, content flags |
| 2.2 | Q3 2026 | 🚧 Planned | Dynamic updates, scheduling, multi-language |
| 2.3 | Q4 2026 | 💡 Proposed | Blockchain, monetization, interactivity |

## Compliance & Standards

### Character Encoding

All text fields should use UTF-8 encoding to support international characters and emoji.

### URL Validation

All URL fields should be valid HTTP/HTTPS URLs:
- Must start with `http://` or `https://`
- Should be properly URL-encoded
- Should be publicly accessible (for verification)

### Language Codes

Use ISO 639-1 two-letter codes, optionally with ISO 3166-1 country codes:
- `en` - English
- `en-US` - US English
- `es` - Spanish
- `fr` - French
- `de` - German
- `ja` - Japanese

### Content Flags

**NSFW Flag Requirements:**
- `icy-nsfw: 1` must be set for explicit content
- Failure to set may violate platform policies
- Affects directory listings and recommendations

**AI-Generated Flag:**
- Required for AI-generated or AI-assisted content in some jurisdictions
- Transparency requirement for content attribution
- May affect copyright and licensing

## Security Considerations

### Authentication

ICY2 supports multiple authentication methods:
1. **Basic Auth**: Standard HTTP Basic authentication (username:password)
2. **Token-based**: OAuth, JWT, or custom tokens via `icy-auth-token`
3. **Certificate**: SSL/TLS client certificates (future)
4. **SSH Keys**: Public/private key pairs (future)

### HTTPS/SSL

**Recommended:** All ICY2 streams should use HTTPS for:
- Encrypted metadata transmission
- Password protection
- Token security
- Content integrity

### Token Format

If using `icy-auth-token`:
```
icy-auth-token: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...
```

Token should be:
- Short-lived (1-24 hours)
- Revocable
- Scoped to specific mount points

## License & Attribution

**ICY-META v2.1+ Protocol Specification**

Copyright (C) 2026 David St John (Saint John)
MediaCast1 / Mcaster1DNAS Project

This specification is released under the Creative Commons Attribution 4.0 International License (CC BY 4.0).

You are free to:
- Share: Copy and redistribute the specification
- Adapt: Implement in any software project
- Commercial Use: Use in commercial products

Under the following terms:
- Attribution: Give appropriate credit to Mcaster1DNAS project

**Implementation in Mcaster1DNAS is licensed under GNU GPL v2.**

## References

- **Mcaster1DNAS**: https://github.com/davestj/mcaster1dnas
- **ICY Protocol (Legacy)**: Icecast/Shoutcast documentation
- **HTTP/1.1 Specification**: RFC 7230-7235
- **ISO 639-1 Language Codes**: https://en.wikipedia.org/wiki/ISO_639-1
- **ISO 3166-1 Country Codes**: https://en.wikipedia.org/wiki/ISO_3166-1

## Contact & Support

**Project Maintainer:** David St John (Saint John)
**Email:** davestj@gmail.com
**Website:** https://mcaster1.com
**GitHub:** https://github.com/davestj/mcaster1dnas
**Issues:** https://github.com/davestj/mcaster1dnas/issues

---

**Last Updated:** February 15, 2026
**Specification Version:** 2.1
**Implementation:** Mcaster1DNAS v2.6.0+

# ICY2 Protocol — Mcaster1DNAS Implementation Guide

**Protocol Version:** ICY-META v2.2
**Applies to:** Mcaster1DNAS v2.5.1+
**Full Spec:** [ICY2_PROTOCOL_SPEC.md](../ICY2_PROTOCOL_SPEC.md)

---

## What is ICY2?

ICY2 (ICY-META version 2.x) is an extended metadata protocol layered on top of the legacy
SHOUTcast/Icecast ICY 1.x headers. It adds structured, typed metadata fields covering:

- Station identity and programming
- Track/song metadata (title, artist, album, BPM, ISRC)
- Podcast episode metadata (host, RSS URL, episode number, duration)
- Social media integration (DJ handle, Twitter, Instagram, TikTok)
- Audio technical parameters (codec, bitrate, sample rate, channels)
- Live performance details (venue, set start time, DJ bio)
- Listener engagement (requests URL, chat link)
- Content classification (NSFW, AI-generated, geo-region)
- Distribution (CDN, relay network)

**Key principle:** ICY2 is purely additive. All ICY 1.x legacy headers (`icy-name`,
`icy-genre`, `icy-url`, `icy-pub`, `icy-br`, `icy-metaint`) are preserved unchanged. A
server that does not understand ICY2 simply ignores the extra headers.

---

## Protocol Detection

The server detects ICY2 clients by the presence of:

```http
icy-metadata-version: 2.2
```

Version matching uses prefix `2.` so both `2.1` and `2.2` are accepted. If the header
is absent the server falls back to ICY 1.x parsing.

---

## Source Client Handshake

A source encoder (OBS, Mixxx, ffmpeg, custom) authenticates via HTTP PUT and sends ICY2
headers in the request:

```http
PUT /live.mp3 HTTP/1.1
Host: stream.example.com:9443
Authorization: Basic c291cmNlOnBhc3N3b3Jk
Content-Type: audio/mpeg
icy-metadata-version: 2.2
icy-name: My Radio Station
icy-genre: Electronic / House
icy-url: https://myradio.example.com
icy-br: 128
icy-pub: 1
icy-dj-handle: @djname
icy-show-title: Saturday Night Live
icy-station-id: myradio-001
icy-social-twitter: @myradio
icy-hashtags: #electronic #live
```

---

## Live Metadata Push (StreamTitle Updates)

After the source connection is established, the encoder may inject in-stream ICY metadata
updates (the classic SHOUTcast StreamTitle mechanism). Mcaster1DNAS v2.5.1+ adds support
for ICY2-extended fields in these in-band updates:

```
StreamTitle='Artist - Title';
StreamUrl='https://...';
```

For ICY2 you may also use the `/admin/metadata` HTTP endpoint to push updates without
interrupting the audio stream:

```bash
# Update "now playing" on a live mount
curl -sk -u admin:changeme \
  "https://localhost:9443/admin/metadata?mount=/live.mp3&mode=updinfo&song=Artist+-+Track+Title"

# Push ICY2 extended metadata
curl -sk -u admin:changeme \
  "https://localhost:9443/admin/metadata" \
  -d "mount=/live.mp3&mode=updinfo&song=Artist - Title&icy-dj-handle=@djname&icy-show-title=Saturday%20Night"
```

---

## Static Mount Metadata Push

For podcast, socialcast, and on-demand mounts (see [STATIC_MOUNTS.md](STATIC_MOUNTS.md))
the server manages metadata independently from a live encoder. Push updates via:

```bash
# Update metadata on a podcast mount
curl -sk -u admin:changeme \
  "https://localhost:9443/admin/metadata?mount=/podcast&mode=updinfo" \
  --data-urlencode "song=Episode 42 - Understanding ICY2" \
  --data-urlencode "icy-podcast-host=The Host Name" \
  --data-urlencode "icy-podcast-episode=42"

# Update on-demand mount now-playing
curl -sk -u admin:changeme \
  "https://localhost:9443/admin/metadata?mount=/ondemand&mode=updinfo&song=Artist+-+Track"
```

---

## ICY2 v2.2 Field Reference

### Core Station Identity

| Header | Type | Example |
|--------|------|---------|
| `icy-metadata-version` | string | `2.2` |
| `icy-meta-station-id` | string | `myradio-unique-001` |
| `icy-name` | string | `My Radio Station` (ICY1 compat) |
| `icy-genre` | string | `Electronic / House` |
| `icy-url` | URL | `https://myradio.com` |
| `icy-pub` | 0/1 | `1` |
| `icy-br` | integer | `128` |

### Track / Now Playing

| Header | Type | Example |
|--------|------|---------|
| `icy-title` | string | `Track Title` |
| `icy-artist` | string | `Artist Name` |
| `icy-album` | string | `Album Name` |
| `icy-year` | integer | `2024` |
| `icy-track` | string | `5` |
| `icy-isrc` | string | `USRC17607839` |
| `icy-bpm` | float | `128.5` |
| `icy-label` | string | `Record Label` |
| `icy-composer` | string | `Composer Name` |
| `icy-copyright` | string | `© 2024 Label` |

### Show / Programming

| Header | Type | Example |
|--------|------|---------|
| `icy-show-title` | string | `Saturday Night Live` |
| `icy-show-start` | ISO8601 | `2024-02-15T21:00:00Z` |
| `icy-show-end` | ISO8601 | `2024-02-16T01:00:00Z` |
| `icy-dj-handle` | string | `@djname` |
| `icy-dj-bio` | string | `DJ based in Dallas TX` |
| `icy-venue` | string | `Club 9330, Dallas TX` |
| `icy-language` | BCP47 | `en-US` |

### Podcast Fields

| Header | Type | Example |
|--------|------|---------|
| `icy-podcast-host` | string | `The Podcast Host` |
| `icy-podcast-rss` | URL | `https://myshow.com/feed.rss` |
| `icy-podcast-episode` | integer | `42` |
| `icy-podcast-season` | integer | `3` |
| `icy-podcast-duration` | seconds | `3600` |

### Social Media

| Header | Type | Example |
|--------|------|---------|
| `icy-social-twitter` | string | `@myradio` |
| `icy-social-instagram` | string | `@myradio` |
| `icy-social-tiktok` | string | `@myradio` |
| `icy-emoji` | string | `🎵🔥` |
| `icy-hashtags` | string | `#electronic #live` |

### Audio Technical

| Header | Type | Example |
|--------|------|---------|
| `icy-audio-codec` | string | `MP3`, `AAC`, `Opus`, `Vorbis` |
| `icy-audio-samplerate` | integer | `44100` |
| `icy-audio-channels` | integer | `2` |
| `icy-audio-bitdepth` | integer | `16`, `24` |

### Content Classification

| Header | Type | Example |
|--------|------|---------|
| `icy-nsfw` | boolean | `false` |
| `icy-ai-generated` | boolean | `false` |
| `icy-geo-region` | string | `US, CA, GB` (allow-list) |
| `icy-content-rating` | string | `clean`, `explicit` |

### Listener Engagement

| Header | Type | Example |
|--------|------|---------|
| `icy-requests-url` | URL | `https://myradio.com/request` |
| `icy-chat-url` | URL | `https://myradio.com/chat` |
| `icy-donate-url` | URL | `https://myradio.com/donate` |
| `icy-requests-enabled` | boolean | `true` |

### Video / Stream Link

| Header | Type | Example |
|--------|------|---------|
| `icy-video-type` | string | `live`, `vod` |
| `icy-video-link` | URL | `https://youtube.com/live/...` |
| `icy-video-platform` | string | `youtube`, `twitch` |
| `icy-video-title` | string | `Saturday Night Stream` |
| `icy-video-resolution` | string | `1920x1080` |

---

## Config — Mount-Level Defaults

ICY2 metadata fields can be pre-set per mount in YAML config so they are active from
server startup without requiring a source encoder:

```yaml
mounts:
  - mount-name: "/live"
    mount-type: "live"
    max-listeners: 100
    public: 1
    stream-name: "My Live Station"
    stream-description: "Electronic music live from Dallas"
    genre: "Electronic"
    bitrate: "128"
    icy-dj-handle: "@djname"
    icy-station-id: "myradio-001"
    icy-show-title: "The Evening Show"
    icy-requests-enabled: false
    icy-social-twitter: "@myradio"
    icy-hashtags: "#electronic #live"

  - mount-name: "/podcast"
    mount-type: "podcast"
    stream-name: "The Podcast"
    icy-podcast-host: "Host Name"
    icy-podcast-rss: "https://myshow.com/feed.rss"
    icy-requests-enabled: false
```

---

## Stats API — ICY2 Metadata Exposure

ICY2 fields set on a mount are exposed in the server stats XML at `/admin/stats`:

```xml
<source mount="/live">
  <stream_start_iso8601>2026-02-22T18:00:00+0000</stream_start_iso8601>
  <title>Artist - Track Title</title>
  <genre>Electronic</genre>
  <bitrate>128</bitrate>
  <audio_codecid>audio/mpeg</audio_codecid>
  <audio_samplerate>44100</audio_samplerate>
  <audio_channels>2</audio_channels>
  <icy_dj_handle>@djname</icy_dj_handle>
  <icy_show_title>Saturday Night Live</icy_show_title>
  <icy_station_id>myradio-001</icy_station_id>
  <listeners>42</listeners>
  <max_listeners>100</max_listeners>
</source>
```

The public `/status` XML exposes the non-sensitive subset (title, genre, bitrate, codec,
listener count).

---

## Backward Compatibility Notes

- Encoders sending only classic ICY 1.x headers (`icy-name`, `icy-genre`, `icy-br`,
  `icy-pub`) work exactly as before — ICY2 parsing is only activated by the presence of
  `icy-metadata-version: 2.x`
- ICY2 v2.1 clients that send `icy-station-id` instead of `icy-meta-station-id` are
  fully supported — both aliases are parsed
- ICY2 headers not understood by a listener client are silently ignored by that client

---

## See Also

- [ICY2_PROTOCOL_SPEC.md](../ICY2_PROTOCOL_SPEC.md) — full normative specification
- [ICY2-META-SPECS.md](../ICY2-META-SPECS.md) — CSSI community spec reference
- [STATIC_MOUNTS.md](STATIC_MOUNTS.md) — podcast, socialcast, on-demand mount types
- [SONG_HISTORY_API.md](SONG_HISTORY_API.md) — song history ring buffer and XML API

# Static Mount Points — Podcast, Socialcast, On-Demand

**Applies to:** Mcaster1DNAS v2.5.1+
**Branch:** development / windows-dev

---

## Overview

Mcaster1DNAS supports four `mount-type` values for mount points. Understanding the
difference is critical for correctly configuring your streaming setup:

| Mount Type | Description | Source Required? |
|------------|-------------|-----------------|
| `live` | Real-time encoder push (OBS, Mixxx, ffmpeg, etc.) | Yes — source must connect |
| `podcast` | Pre-recorded episodes served on demand | No — server serves files |
| `socialcast` | Scheduled or playlist-driven automated stream | No — server manages playback |
| `ondemand` | Single-file or playlist served on listener request | No — server serves files |

---

## Live Mounts

The classic streaming mode. A source encoder connects to the server and pushes audio
in real time. All connected listeners receive the same stream simultaneously.

```yaml
mounts:
  - mount-name: "/live"
    mount-type: "live"
    max-listeners: 500
    public: 1
    stream-name: "My Live Radio"
    stream-description: "Live electronic music"
    genre: "Electronic"
    bitrate: "128"
    icy-dj-handle: "@djname"
    icy-show-title: "The Evening Show"
    icy-requests-enabled: true
    icy-requests-url: "https://myradio.com/request"
```

**Source encoder connection:**
```
icecast://source:password@server:9330/live
```

**Listener connection:**
```
http://server:9330/live
```

---

## Podcast Mounts

A podcast mount serves pre-recorded episode files to listeners on demand. Each listener
gets their own independent playback position (not synchronized). The server reads from a
configured directory or file list.

```yaml
mounts:
  - mount-name: "/podcast"
    mount-type: "podcast"
    max-listeners: 200
    public: 1
    stream-name: "The Weekly Podcast"
    stream-description: "In-depth conversations about technology"
    genre: "Talk / Tech"
    bitrate: "96"
    icy-podcast-host: "Alex Rivera"
    icy-podcast-rss: "https://mypodcast.com/feed.rss"
    icy-podcast-episode: 42
    icy-podcast-season: 3
    icy-language: "en-US"
    icy-requests-enabled: false
```

### Metadata Push for Podcast Episodes

When a new episode starts playing (or when you want to update the "now playing" display),
push metadata via the admin API:

```bash
curl -sk -u admin:changeme \
  "https://localhost:9443/admin/metadata?mount=/podcast&mode=updinfo" \
  --data-urlencode "song=Ep 42 - AI and the Future" \
  --data-urlencode "icy-podcast-episode=42" \
  --data-urlencode "icy-podcast-host=Alex Rivera"
```

### RSS Feed

Configure the podcast RSS feed URL in the mount definition. The feed should be an
external RSS 2.0 or Apple Podcasts compatible feed served by your web server or hosting
service. Future versions of Mcaster1DNAS will include a built-in RSS generator
(see [PODCAST_PLANNING.md](../PODCAST_PLANNING.md)).

---

## Socialcast Mounts

A socialcast is an automated, continuous stream driven by a playlist or scheduler — the
server manages playback without a live encoder. Use this for:

- 24/7 automated radio stations
- Scheduled program blocks
- DJ mix relay streams
- Background music for websites or events

```yaml
mounts:
  - mount-name: "/socialcast"
    mount-type: "socialcast"
    max-listeners: 300
    public: 1
    stream-name: "Mcaster1 Lounge"
    stream-description: "24/7 automated electronic music"
    genre: "Electronic / Ambient"
    bitrate: "128"
    icy-dj-handle: "AutoDJ"
    icy-show-title: "The Lounge"
    icy-social-twitter: "@mcaster1radio"
    icy-hashtags: "#electronic #ambient #24/7"
```

### Metadata on Socialcast Streams

The server auto-updates the `icy-title` / `StreamTitle` metadata when it advances
to the next track in the playlist. You can also push manual overrides:

```bash
# Force metadata update on socialcast
curl -sk -u admin:changeme \
  "https://localhost:9443/admin/metadata?mount=/socialcast&mode=updinfo&song=Artist+-+Track+Name"
```

---

## On-Demand Mounts

An on-demand mount delivers a specific audio file (or short playlist) to each listener
independently — similar to a podcast but typically for single tracks or albums rather
than episodic content.

```yaml
mounts:
  - mount-name: "/ondemand"
    mount-type: "ondemand"
    max-listeners: 100
    public: 1
    stream-name: "On Demand — Full Albums"
    stream-description: "Listen to full album streams"
    genre: "Various"
    bitrate: "256"
    icy-requests-enabled: false
```

**Use cases:**
- Album stream pages on a music website
- Preview streams for releases
- Podcast episode direct links with embedded player

---

## Differences at a Glance

| Feature | live | podcast | socialcast | ondemand |
|---------|------|---------|------------|---------|
| Encoder required | Yes | No | No | No |
| All listeners synchronized | Yes | No | Yes | No |
| Per-listener position | No | Yes | No | Yes |
| Metadata auto-advances | Via encoder | Via API | Auto (playlist) | Via API |
| Song history tracked | Yes | Yes | Yes | Yes |
| RSS feed supported | No | Yes | No | No |
| Ideal for | Live DJ/radio | Podcasts | Auto radio | Albums/tracks |

---

## Song History on All Mount Types

All mount types feed the song history ring buffer (`/mcaster1songdata`) when a track
change is detected. This means your podcast episodes, socialcast track advances, and
on-demand plays all appear in the public track history pages alongside live stream data.

See [SONG_HISTORY_API.md](SONG_HISTORY_API.md) for full details.

---

## ICY2 Headers on Static Mounts

For non-live mounts, set ICY2 metadata headers in the mount config. The server sends
these to listeners as part of the HTTP response headers when they connect, and exposes
them in the stats XML:

```yaml
mounts:
  - mount-name: "/podcast"
    mount-type: "podcast"
    # ... core config ...

    # ICY2 fields — sent to all listeners on this mount
    icy-dj-handle: "Podcast Host Name"
    icy-station-id: "mypodcast-weekly"
    icy-show-title: "The Weekly Tech Podcast"
    icy-podcast-host: "Alex Rivera"
    icy-podcast-rss: "https://mypodcast.com/feed.rss"
    icy-language: "en-US"
    icy-content-rating: "clean"
    icy-social-twitter: "@mypodcast"
    icy-requests-enabled: false
```

---

## Listener Engagement — Requests

For live and socialcast mounts where listener requests are meaningful, enable the requests
URL and chat URL:

```yaml
mounts:
  - mount-name: "/live"
    mount-type: "live"
    icy-requests-enabled: true
    icy-requests-url: "https://myradio.com/request"
    icy-chat-url: "https://discord.gg/myradio"
    icy-donate-url: "https://ko-fi.com/myradio"
```

Listeners using ICY2-aware players will see clickable request / chat / donate buttons
in their player UI.

---

## See Also

- [ICY2_PROTOCOL.md](ICY2_PROTOCOL.md) — full ICY2 protocol reference
- [SONG_HISTORY_API.md](SONG_HISTORY_API.md) — song history ring buffer and XML API
- [SSL_CERT_GENERATION.md](SSL_CERT_GENERATION.md) — TLS setup for secure listeners
- [PODCAST_PLANNING.md](../PODCAST_PLANNING.md) — planned podcast hosting features

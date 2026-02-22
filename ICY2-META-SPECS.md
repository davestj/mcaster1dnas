
ICY-META v2.2 Protocol Specification (FULL + Legacy Compatibility & Auth)

Project: mcaster1 DNAS / CasterClub Streaming Protocol Initiative
Version: 2.2
Spec Date: February 2026
Maintained By: CasterClub / mcaster1 Core Team
License: Open Spec (CSSI)

---

## Overview

ICY-META v2.2 is a comprehensive streaming metadata and control protocol preserving legacy SHOUTcast
compatibility while adding full support for podcasts, social media integration, video broadcast metadata,
track-level detail, station programming, listener engagement, and stream identity verification.

It powers next-generation platforms such as mcaster1 DNAS and yp.casterclub.com, enabling content creators
to stream, post, promote, or schedule metadata-rich content with or without live audio/video feeds.

All fields use the `icy-meta-` prefix for ICY2-specific headers, making them cleanly distinguishable from
legacy ICY 1.x fields. All ICY2 fields are optional — send only what is relevant to your content type.

---

## Protocol Detection

The presence of `icy-metadata-version` with a `2.x` value triggers ICY2 parsing on the server.

```
icy-metadata-version: 2.2
```

- Servers accept any `2.x` value (2.0, 2.1, 2.2, etc.) via prefix matching
- Absence of this header falls back to ICY 1.x legacy parsing
- ICY2 clients gracefully degrade when connecting to ICY 1.x servers

---

## Legacy ICY 1.x Headers (Preserved for Compatibility)

| Header       | Type    | Description                                    |
|--------------|---------|------------------------------------------------|
| icy-name     | String  | Station or stream display name                 |
| icy-genre    | String  | Genre or content type                          |
| icy-url      | URL     | Station homepage or site                       |
| icy-pub      | Boolean | Public listing flag (1 = yes)                  |
| icy-br       | Integer | Bitrate in kbps                                |
| icy-metaint  | Integer | Interval in bytes between metadata blocks      |

---

## Legacy Authentication (Backwards-Compatible)

| Field         | Type   | Description                                               |
|---------------|--------|-----------------------------------------------------------|
| adminpassword | String | Admin interface password (for HTML/admin.cgi)             |
| password      | String | Source encoder password for streaming                     |
| user          | String | Optional username (used in SHOUTcast v2 or Icecast2)      |

```
SOURCE password@hostname:port/stream
```

---

## Station Identity

Core station identification, verification, and PKI/authentication fields. These are static per-station
values and should be consistent across all stream sessions from the same broadcaster.

| Header                        | Type    | Description                                                        |
|-------------------------------|---------|--------------------------------------------------------------------|
| icy-meta-station-id           | String  | Unique global station ID (alphanumeric, hyphens allowed)           |
| icy-meta-station-logo         | URL     | Station logo or branding image URL                                 |
| icy-meta-certissuer-id        | String  | Certificate authority ID                                           |
| icy-meta-cert-rootca          | String  | Root CA hash or fingerprint                                        |
| icy-meta-certificate          | String  | Base64 PEM certificate                                             |
| icy-meta-ssh-pubkey           | String  | SSH public key for source authentication                           |
| icy-meta-verification-status  | Enum    | `unverified`, `pending`, `verified`, `gold`                        |

---

## Programming / Show Scheduling

Show and programming metadata for the current and next scheduled broadcast slot. These fields allow
directories, players, and notice boards to display what is on now and what is coming up. Also supports
automation systems such as mcaster1 DNAS AutoDJ and mcaster1TagStack.

| Header                    | Type     | Description                                                         |
|---------------------------|----------|---------------------------------------------------------------------|
| icy-meta-show-title       | String   | Current show or program title                                       |
| icy-meta-show-start       | ISO8601  | Datetime the current show started                                   |
| icy-meta-show-end         | ISO8601  | Datetime the current show ends                                      |
| icy-meta-next-show        | String   | Title of the next scheduled program                                 |
| icy-meta-next-show-time   | ISO8601  | Scheduled start time of the next program                            |
| icy-meta-schedule-url     | URL      | Link to the full station program schedule                           |
| icy-meta-autodj           | Boolean  | `1` = AutoDJ/automation active, `0` = live human DJ                 |
| icy-meta-playlist-name    | String   | Current playlist or automation source name (e.g., Top 40 Rotation) |

---

## DJ / Host Metadata

Per-show DJ and host identification. Distinct from podcast host fields — these apply to live DJ sets,
talk shows, and hosted radio programming. The `dj-genre` field supports up to 5 comma-separated values.

| Header                | Type   | Description                                                              |
|-----------------------|--------|--------------------------------------------------------------------------|
| icy-meta-dj-handle    | String | Current DJ or host social handle (e.g., @djsynthwave)                    |
| icy-meta-dj-bio       | String | Short DJ or host biography or tagline — max 280 characters               |
| icy-meta-dj-genre     | String | DJ's genre set. Comma-separated, max 5 values. e.g., `Electronic, House` |
| icy-meta-dj-showrating| Enum   | DJ/host show content rating: `all-ages`, `teen`, `mature`, `explicit`    |

---

## Track Metadata

Per-track metadata pushed by source encoders and metadata injection systems such as mcaster1TagStack.
These fields update on every track change. `icy-meta-track-mbid` enables direct MusicBrainz linking
rather than search queries. `icy-meta-track-isrc` supports royalty and licensing tracking.
`icy-meta-track-artwork` is the primary field for player album art display.

| Header                 | Type    | Description                                                                      |
|------------------------|---------|----------------------------------------------------------------------------------|
| icy-meta-track-artwork | URL     | Album or track artwork image URL — used by players for visual display            |
| icy-meta-track-album   | String  | Album or release name                                                            |
| icy-meta-track-year    | Integer | Track or album release year                                                      |
| icy-meta-track-label   | String  | Record label                                                                     |
| icy-meta-track-bpm     | Integer | Beats per minute                                                                 |
| icy-meta-track-key     | String  | Musical key — Camelot notation or standard (e.g., `8B`, `Am`)                   |
| icy-meta-track-genre   | String  | Per-track genre — may differ from station genre                                  |
| icy-meta-track-mbid    | UUID    | MusicBrainz Recording ID — enables direct record lookup                          |
| icy-meta-track-isrc    | String  | International Standard Recording Code — for royalty and licensing tracking       |

---

## Audio + Podcast Metadata

Fields for podcast and talk-format programming. `icy-meta-duration` covers content runtime for all
content types — audio, podcast episode, and video.

| Header                    | Type    | Description                                                      |
|---------------------------|---------|------------------------------------------------------------------|
| icy-meta-podcast-host     | String  | Podcast creator or host name                                     |
| icy-meta-podcast-rating   | Enum    | Podcast content rating: `all-ages`, `teen`, `mature`, `explicit` |
| icy-meta-podcast-rss      | URL     | Podcast RSS feed URL                                             |
| icy-meta-podcast-episode  | String  | Episode title or ID (e.g., S4E1 – Decentralized Rights)          |
| icy-meta-duration         | Integer | Total content runtime in seconds — applies to audio, podcast, or video |
| icy-meta-language         | String  | Content language tag per ISO 639-1 (e.g., `en`, `en-US`, `es`)  |

---

## Video Streaming & Metadata

Full video metadata support for simulcast, short-form, VOD, and scheduled video content. Supports
platforms including YouTube, TikTok, Twitch, Kick, Rumble, and others.

| Header                    | Type     | Description                                                                   |
|---------------------------|----------|-------------------------------------------------------------------------------|
| icy-meta-videotype        | Enum     | `live`, `short`, `clip`, `trailer`, `ad`                                      |
| icy-meta-videorating      | Enum     | Video content rating: `all-ages`, `teen`, `mature`, `explicit`                |
| icy-meta-videolink        | URL      | Link to the video content or stream page                                      |
| icy-meta-videotitle       | String   | Title of the video                                                            |
| icy-meta-videoposter      | URL      | Thumbnail or preview image URL                                                |
| icy-meta-videochannel     | String   | Creator, uploader, or channel handle                                          |
| icy-meta-videoplatform    | Enum     | `youtube`, `tiktok`, `twitch`, `kick`, `rumble`, `vimeo`, `custom`            |
| icy-meta-videostart       | ISO8601  | Scheduled start datetime for the video                                        |
| icy-meta-videolive        | Boolean  | `1` = currently live, `0` = pre-recorded                                      |
| icy-meta-videocodec       | String   | Video codec in use (e.g., `h264`, `vp9`, `av1`)                               |
| icy-meta-videofps         | Integer  | Frames per second                                                             |
| icy-meta-videoresolution  | String   | Video resolution (e.g., `1080p`, `4K`, `720x1280`)                           |
| icy-meta-videonsfw        | Boolean  | Video-specific NSFW indicator                                                 |

---

## Audio Technical

Technical audio stream parameters reported by source encoders such as mcaster1DSPEncoder. These allow
directories, players, and monitoring systems to display and verify stream quality details.
`icy-meta-loudness` uses EBU R128 integrated loudness measurement in LUFS.

| Header               | Type    | Description                                                                  |
|----------------------|---------|------------------------------------------------------------------------------|
| icy-meta-audio-codec | Enum    | `mp3`, `aac`, `aac-he`, `ogg`, `opus`, `flac`                               |
| icy-meta-samplerate  | Integer | Sample rate in Hz (e.g., `44100`, `48000`)                                   |
| icy-meta-channels    | Integer | `1` = mono, `2` = stereo, `6` = 5.1 surround                                |
| icy-meta-loudness    | Float   | Integrated loudness in LUFS per EBU R128 (e.g., `-14.0`)                    |
| icy-meta-encoder     | String  | Encoder software name and version (e.g., `Mcaster1DSP/1.2.0`, `BUTT/1.40`) |

---

## Social, Discovery & Branding

Social media handles and discoverability fields. `icy-meta-creator-handle` is a platform-agnostic
public identity handle for the broadcaster — distinct from platform-specific social handles below.
`icy-meta-hashtag-array` uses JSON array format. `icy-meta-emoji` supports multiple emoji characters.

| Header                        | Type     | Description                                                          |
|-------------------------------|----------|----------------------------------------------------------------------|
| icy-meta-creator-handle       | String   | Public creator or brand handle (platform-agnostic identity)          |
| icy-meta-social-twitter       | String   | Twitter/X handle (e.g., @mcaster1dnas)                              |
| icy-meta-social-twitch        | String   | Twitch handle                                                        |
| icy-meta-social-ig            | String   | Instagram username                                                   |
| icy-meta-social-tiktok        | String   | TikTok profile name                                                  |
| icy-meta-social-youtube       | URL      | YouTube channel URL — static social presence link                    |
| icy-meta-social-facebook-page | URL      | Facebook page URL                                                    |
| icy-meta-social-linkedin      | URL      | LinkedIn profile URL                                                 |
| icy-meta-social-linktree      | URL      | Unified profile link (Linktree, Beacons, etc.)                       |
| icy-meta-emoji                | String   | Mood or emotion indicators (e.g., `🎵🔥🎧`)                         |
| icy-meta-hashtag-array        | String[] | Searchable tags in JSON array format (e.g., `["#electronic","#dj"]`) |

---

## Listener Engagement

Fields enabling listeners to interact with the station directly from the player. These are pushed by
the source encoder or injection system and displayed by ICY2-aware players and directories.

| Header                    | Type    | Description                                                            |
|---------------------------|---------|------------------------------------------------------------------------|
| icy-meta-request-enabled  | Boolean | `1` = listener song requests are currently open                        |
| icy-meta-request-url      | URL     | URL for song requests or listener dedications                          |
| icy-meta-chat-url         | URL     | Live listener chat room URL                                            |
| icy-meta-tip-url          | URL     | Listener donation or tip URL (Ko-fi, Patreon, PayPal, etc.)            |
| icy-meta-events-url       | URL     | Link to upcoming station events or gigs page                           |

---

## Broadcast Distribution

Fields describing where and how the content is being distributed across platforms and infrastructure.
`icy-meta-crosspost-platforms` uses comma-separated platform identifiers for simultaneous live
distribution — distinct from `icy-meta-social-youtube` which is a static channel profile link.
Supported by mcaster1CastIt and compatible multi-platform broadcast clients.

| Header                        | Type   | Description                                                                              |
|-------------------------------|--------|------------------------------------------------------------------------------------------|
| icy-meta-crosspost-platforms  | String | Comma-separated active live broadcast platforms (e.g., `youtube,twitch,tiktok,kick`)     |
| icy-meta-stream-session-id    | String | Unique ID for this broadcast session — different from the permanent station-id            |
| icy-meta-cdn-region           | String | CDN or distribution region (e.g., `us-east`, `eu-west`, `ap-southeast`)                  |
| icy-meta-relay-origin         | URL    | Origin server URL if this mount is a relay (e.g., `https://source.example.com:9443`)     |

---

## Station Notices

Real-time listener notices and announcements pushed via the stream for display in ICY2-aware players
and notice boards. `icy-meta-notice-expires` is an ISO8601 datetime — players should hide the notice
after this time. Supported by mcaster1DNAS webplayer notice panel.

| Header                   | Type    | Description                                                              |
|--------------------------|---------|--------------------------------------------------------------------------|
| icy-meta-notice          | String  | General listener notice or announcement                                  |
| icy-meta-notice-url      | URL     | Click-through URL for more information about the notice                  |
| icy-meta-notice-expires  | ISO8601 | Datetime after which the notice should no longer be displayed            |

---

## Access & Authentication

| Header               | Type   | Description                                                                   |
|----------------------|--------|-------------------------------------------------------------------------------|
| icy-meta-auth-token  | JWT    | Optional access token (Bearer JWT or custom token) for enhanced security      |

---

## Content Flags & Compliance

Machine-readable flags for content filtering, AI transparency, geographic licensing, and royalty
tracking. `icy-meta-nsfw` is the binary filter flag; the per-content-type rating fields provide
human-readable classification for DJ sets, podcasts, and video independently.

| Header                     | Type    | Description                                                                            |
|----------------------------|---------|----------------------------------------------------------------------------------------|
| icy-meta-nsfw              | Boolean | `1` = NSFW/explicit content — affects directory listings and recommendations           |
| icy-meta-ai-generator      | Boolean | `1` = AI-generated or AI-assisted content — required for transparency in some regions  |
| icy-meta-geo-region        | String  | Target geographic region (ISO 3166-1, e.g., `US`, `EU`, `GLOBAL`)                     |
| icy-meta-license-type      | Enum    | `cc-by`, `cc-by-sa`, `cc0`, `pro-licensed`, `all-rights-reserved`                     |
| icy-meta-royalty-free      | Boolean | `1` = royalty-free content                                                             |
| icy-meta-license-territory | String  | Comma-separated ISO country codes where content is licensed (e.g., `US,CA,EU,GLOBAL`) |

---

## Sample Scenarios

### Live DJ Set with Full Metadata

```
icy-metadata-version: 2.2
icy-name: ChillZone FM
icy-genre: Electronic/House/Techno
icy-br: 320
icy-pub: 1

icy-meta-station-id: chillzone-fm-001
icy-meta-station-logo: https://chillzone.fm/logo.png
icy-meta-verification-status: verified

icy-meta-show-title: Late Night House Sessions
icy-meta-show-start: 2026-02-21T22:00:00Z
icy-meta-show-end: 2026-02-22T02:00:00Z
icy-meta-next-show: Morning Chill
icy-meta-next-show-time: 2026-02-22T07:00:00Z
icy-meta-autodj: 0

icy-meta-dj-handle: @djsynthwave
icy-meta-dj-bio: Electronic music producer and DJ based in Berlin. Known for deep house and techno.
icy-meta-dj-genre: Electronic, House, Techno, Deep House
icy-meta-dj-showrating: all-ages

icy-meta-track-artwork: https://cdn.example.com/art/track123.jpg
icy-meta-track-album: Midnight Sessions Vol. 3
icy-meta-track-year: 2025
icy-meta-track-label: Hypnotic Records
icy-meta-track-bpm: 124
icy-meta-track-key: 8B
icy-meta-track-mbid: 3a8e7c21-1234-5678-abcd-ef0123456789
icy-meta-track-isrc: USRC12345678

icy-meta-creator-handle: @djsynthwave
icy-meta-social-twitter: @djsynthwave
icy-meta-social-ig: @djsynthwave
icy-meta-social-tiktok: @djsynthwave
icy-meta-social-youtube: https://youtube.com/@djsynthwave
icy-meta-social-linktree: https://linktr.ee/djsynthwave
icy-meta-emoji: 🎵🔥🎧
icy-meta-hashtag-array: ["#house","#techno","#livedjset","#electronic"]

icy-meta-request-enabled: 1
icy-meta-request-url: https://chillzone.fm/requests
icy-meta-chat-url: https://chillzone.fm/chat
icy-meta-tip-url: https://ko-fi.com/djsynthwave

icy-meta-audio-codec: mp3
icy-meta-samplerate: 44100
icy-meta-channels: 2
icy-meta-loudness: -14.0
icy-meta-encoder: Mcaster1DSP/1.2.0

icy-meta-nsfw: 0
icy-meta-ai-generator: 0
icy-meta-geo-region: GLOBAL
icy-meta-license-type: pro-licensed
```

### Podcast Episode Push

```
icy-metadata-version: 2.2
icy-name: FutureTalks
icy-meta-station-id: futuretalks-podcast-001
icy-meta-verification-status: verified

icy-meta-show-title: FutureTalks Podcast
icy-meta-autodj: 0

icy-meta-podcast-host: Sasha Tran
icy-meta-podcast-rating: all-ages
icy-meta-podcast-episode: S4E1 – Decentralized Rights
icy-meta-podcast-rss: https://futuretalks.fm/feed.xml
icy-meta-duration: 3600
icy-meta-language: en

icy-meta-track-artwork: https://futuretalks.fm/episodes/s4e1-art.jpg

icy-meta-request-url: https://futuretalks.fm/contact
icy-meta-license-type: cc-by
icy-meta-royalty-free: 1
```

### TikTok Short-Form Post (No Audio Required)

```
icy-metadata-version: 2.2
icy-name: @DropMaster

icy-meta-videotype: short
icy-meta-videorating: all-ages
icy-meta-videolink: https://tiktok.com/@dropmaster/video/7739201
icy-meta-videochannel: @dropmaster
icy-meta-videoplatform: tiktok
icy-meta-videoposter: https://cdn.tiktok.com/posters/7739201.jpg
icy-meta-videolive: 0
icy-meta-videoresolution: 720x1280
icy-meta-videocodec: h264
icy-meta-videofps: 30
icy-meta-duration: 45

icy-meta-social-tiktok: @dropmaster
icy-meta-emoji: 🎵🔥🎥
icy-meta-hashtag-array: ["#beatdrop","#shorts","#music"]
```

### YouTube Livestream Simulcast

```
icy-metadata-version: 2.2
icy-name: ChillZone FM
icy-meta-verification-status: verified

icy-meta-show-title: Synthwave All Night
icy-meta-autodj: 0

icy-meta-videotype: live
icy-meta-videorating: all-ages
icy-meta-videolink: https://youtube.com/watch?v=live543
icy-meta-videotitle: Synthwave All Night — Live Stream
icy-meta-videoplatform: youtube
icy-meta-videoresolution: 1080p
icy-meta-videocodec: h264
icy-meta-videofps: 60
icy-meta-videolive: 1
icy-meta-duration: 7200

icy-meta-crosspost-platforms: youtube,twitch
icy-meta-stream-session-id: session-20260221-chillzone-001
icy-meta-cdn-region: us-east

icy-meta-notice: We are live on YouTube and Twitch tonight! Join the chat.
icy-meta-notice-url: https://youtube.com/watch?v=live543
icy-meta-notice-expires: 2026-02-22T02:00:00Z
```

---

## Integration Targets

- **DSPs**: SAM Broadcaster, Edcast, BUTT, Rocket, Mixxx
- **Encoders**: mcaster1DSPEncoder, Liquidsoap, Icecast source clients
- **Metadata**: mcaster1TagStack, metadata-pusher agents
- **Multi-Platform**: mcaster1CastIt
- **Directory**: yp.casterclub.com via `/8.json`, `/status.html`, or `/push`
- **Players**: mcaster1DNAS webplayer, ICY2-aware clients
- **Webhook/event-forwarding**: Real-time content relay and notification

---

## Maintainer Contact

CasterClub Streaming Standards Initiative (CSSI)
Email: specs@casterclub.com
Web: https://casterclub.com/specs/icy-2
GitHub: https://github.com/casterclub/specs
License: Open Specification / Attribution Preferred

---

*ICY-META v2.2 Specification — February 2026*
*Supersedes ICY-META v2.1 (February 15, 2026)*

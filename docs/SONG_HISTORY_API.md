# Song History API — Mcaster1DNAS

**Applies to:** Mcaster1DNAS v2.5.1+
**Endpoint:** `/mcaster1songdata` (raw XML) + `/songdata.xsl` (public HTML) + `/admin/songdata.xsl` (admin HTML)

---

## Overview

Mcaster1DNAS maintains an in-memory ring buffer of recently played tracks across all mount
points. When the "now playing" metadata changes on any source the new entry is appended;
when the limit is reached the oldest entry is discarded.

**Key properties:**
- Pure in-memory — no disk I/O, zero database overhead
- Configurable limit (`song-history-limit`) per config file; `0` = unlimited
- Deduplication — consecutive identical titles on the same mount are silently ignored
- `ended_at` back-fill — when track N+1 starts, `ended_at` for track N is set automatically
- Exposed as a raw XML API, two XSL HTML pages (admin + public), and JSON (planned)

---

## Configuration

### YAML
```yaml
limits:
  song-history-limit: 25   # 0 = keep all tracks (memory-backed, no disk I/O)
```

### XML
```xml
<limits>
    <song-history-limit>25</song-history-limit>
</limits>
```

Default is `25`. Set to `0` to keep unlimited history (memory grows with uptime).

---

## Raw XML Endpoint

**URL:** `GET /mcaster1songdata`
**Auth:** None (public)
**Format:** `application/xml`

```bash
curl http://localhost:9330/mcaster1songdata
```

### Response Structure

```xml
<?xml version="1.0" encoding="UTF-8"?>
<mcaster1songdata>
  <server-name>My Radio Station</server-name>
  <generated>2026-02-22T18:45:00Z</generated>
  <song-count>5</song-count>
  <songs>
    <song>
      <mount>/live</mount>
      <title>Artist - Track Title</title>
      <artist>Artist Name</artist>
      <song-title>Track Title</song-title>
      <started_at>2026-02-22T18:40:00Z</started_at>
      <ended_at>2026-02-22T18:43:30Z</ended_at>
      <duration>210</duration>
      <listeners>42</listeners>
      <codec>audio/mpeg</codec>
      <bitrate>128</bitrate>
      <samplerate>44100</samplerate>
      <channels>2</channels>
    </song>
    <song>
      <mount>/live</mount>
      <title>DJ Set — Live On Air</title>
      <artist></artist>
      <song-title>DJ Set — Live On Air</song-title>
      <started_at>2026-02-22T18:43:30Z</started_at>
      <ended_at></ended_at>
      <duration>-1</duration>
      <listeners>45</listeners>
      <codec>audio/mpeg</codec>
      <bitrate>128</bitrate>
      <samplerate>44100</samplerate>
      <channels>2</channels>
    </song>
  </songs>
</mcaster1songdata>
```

**Fields:**

| Field | Type | Description |
|-------|------|-------------|
| `mount` | string | Mount point path (e.g. `/live`, `/podcast`) |
| `title` | string | Full ICY StreamTitle (artist + title combined) |
| `artist` | string | Artist extracted from `Artist - Title` format |
| `song-title` | string | Track title extracted from `Artist - Title` format |
| `started_at` | ISO8601 | UTC timestamp when track started |
| `ended_at` | ISO8601 | UTC timestamp when track ended; empty = currently playing |
| `duration` | integer | Seconds the track played; `-1` = still playing |
| `listeners` | integer | Listener count at the moment the track started |
| `codec` | string | Audio codec content-type string |
| `bitrate` | integer | Stream bitrate in kbps |
| `samplerate` | integer | Sample rate in Hz |
| `channels` | integer | Channel count (1=mono, 2=stereo) |

---

## HTML Pages

### Public Track History

**URL:** `GET /songdata.xsl`
**Auth:** None (public)
**Format:** HTML page (via XSL transform)

Displays a rich table of recently played tracks with:
- Artist, title, mount point
- Codec quality badge: `MP3 / 128kbps / 44.1kHz / Stereo`
- Played-at timestamp and live "on air" duration counter (Xm Ys)
- Listener count at the moment the track played
- Animated pulsing green dot on the most recently added entry
- Music service lookup icons for each track:
  - MusicBrainz — open music encyclopedia
  - Last.fm — scrobbling and artist radio
  - Discogs — release/label database
  - AllMusic — editorial reviews and discographies

### Admin Track History

**URL:** `GET /admin/songdata.xsl`
**Auth:** HTTP Basic (admin credentials)
**Format:** HTML page (via XSL transform)

Same as public view with additional admin controls and full codec details.

---

## Filtering by Mount

The XML API can be filtered to a single mount:

```bash
# History for /live mount only
curl "http://localhost:9330/mcaster1songdata?mount=/live"

# History for /podcast mount only
curl "http://localhost:9330/mcaster1songdata?mount=/podcast"
```

---

## Integration Examples

### JavaScript (web widget)

```javascript
async function fetchSongHistory(mount = null) {
    const url = mount
        ? `/mcaster1songdata?mount=${encodeURIComponent(mount)}`
        : '/mcaster1songdata';
    const resp = await fetch(url);
    const xml = await resp.text();
    const parser = new DOMParser();
    return parser.parseFromString(xml, 'application/xml');
}

// Get currently playing track
async function getNowPlaying(mount = '/live') {
    const doc = await fetchSongHistory(mount);
    const songs = doc.querySelectorAll('song');
    for (const song of songs) {
        const endedAt = song.querySelector('ended_at')?.textContent;
        if (!endedAt || endedAt.trim() === '') {
            return {
                title: song.querySelector('title')?.textContent,
                artist: song.querySelector('artist')?.textContent,
                startedAt: song.querySelector('started_at')?.textContent,
                listeners: song.querySelector('listeners')?.textContent,
            };
        }
    }
    return null;
}
```

### Python

```python
import requests
import xml.etree.ElementTree as ET

def get_song_history(base_url, mount=None):
    url = f"{base_url}/mcaster1songdata"
    if mount:
        url += f"?mount={mount}"
    resp = requests.get(url, timeout=5)
    root = ET.fromstring(resp.text)
    songs = []
    for song in root.findall('.//song'):
        songs.append({
            'mount':       song.findtext('mount'),
            'title':       song.findtext('title'),
            'artist':      song.findtext('artist'),
            'started_at':  song.findtext('started_at'),
            'ended_at':    song.findtext('ended_at'),
            'listeners':   int(song.findtext('listeners') or 0),
        })
    return songs

# Example usage
history = get_song_history('http://localhost:9330', mount='/live')
for track in history:
    status = 'NOW PLAYING' if not track['ended_at'] else track['ended_at']
    print(f"{status}  {track['title']}  ({track['listeners']} listeners)")
```

### curl + jq (via future JSON endpoint)

```bash
# JSON API planned for v2.6 — use XML for now
curl -s http://localhost:9330/mcaster1songdata \
  | python3 -c "import sys,xml.etree.ElementTree as ET; \
    root=ET.fromstring(sys.stdin.read()); \
    [print(s.findtext('title')) for s in root.findall('.//song')]"
```

---

## Deduplication Behavior

Consecutive duplicate titles on the same mount are silently dropped:

```
# These are deduplicated (same mount, same title back-to-back)
18:00  /live  "DJ Live Mix"     ← stored
18:00  /live  "DJ Live Mix"     ← DROPPED (duplicate)
18:05  /live  "DJ Live Mix"     ← DROPPED (still same title)
18:10  /live  "Artist - Track"  ← stored (new title)
```

Titles on different mounts are never deduplicated against each other:

```
18:00  /live      "Artist - Track"  ← stored
18:00  /podcast   "Artist - Track"  ← stored (different mount)
```

---

## `ended_at` Back-Fill

When track N+1 is added to a mount, the `ended_at` timestamp of track N is set to the
`started_at` of track N+1. This gives accurate time-on-air calculations in the UI.

The most recent entry on each mount always has an empty `ended_at` (currently playing).

---

## Memory Footprint

Each song entry occupies approximately 200–400 bytes. With the default limit of 25 entries
the total per-server memory overhead is under 10 KB regardless of listener count or stream
uptime.

With `song-history-limit: 0` (unlimited), the buffer grows by one entry per track change
per mount. For a busy station with frequent track changes, budget ~1 MB per 5,000 total
track changes.

---

## See Also

- [STATIC_MOUNTS.md](STATIC_MOUNTS.md) — mount types and how song history integrates
- [ICY2_PROTOCOL.md](ICY2_PROTOCOL.md) — how ICY2 metadata drives track change events
- [WEBPLAYER_FEATURE.md](../WEBPLAYER_FEATURE.md) — browser player that shows now-playing from this API

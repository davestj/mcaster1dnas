# Mcaster1DNAS -- Codec Support Roadmap

## Current Status (2026-03-31)
VP9+WebM video streaming confirmed working end-to-end with Mcaster1 DSP Encoder.
- Audio: Opus correctly declared in WebM Tracks element (A_OPUS)
- Video: VP9 1552x1552 @ 30fps, keyframes every 2s
- EBML header: 171 bytes from encoder, starts with `1a 45 df a3` (correct)
- ffplay confirms: VP9 video + Opus audio decoding, stream stable

## Audio Codecs in WebM/Matroska -- VERIFIED (2026-03-31)

The DNAS EBML handler (`format_ebml.c`) is a **pure container pass-through**. It has zero
codec-specific code -- it never inspects TrackEntry codec IDs, CodecPrivate data, or
SimpleBlock contents. Any audio codec the encoder puts in a WebM/Matroska container will
pass through the DNAS correctly to listeners.

### Supported Audio Codecs in EBML Container (no DNAS changes needed)

| Audio Codec | Matroska CodecID | WebM Spec | Desktop Players | Browsers |
|-------------|-----------------|-----------|-----------------|----------|
| Opus        | `A_OPUS`        | Official  | ffplay, VLC, mpv | All modern |
| Vorbis      | `A_VORBIS`      | Official  | ffplay, VLC, mpv | All modern |
| AAC-LC      | `A_AAC`         | MKV only  | ffplay, VLC, mpv | May reject in WebM |
| HE-AAC v1/v2 | `A_AAC`       | MKV only  | ffplay, VLC, mpv | May reject in WebM |
| MP3         | `A_MPEG/L3`     | MKV only  | ffplay, VLC, mpv | May reject in WebM |
| FLAC        | `A_FLAC`        | MKV only  | ffplay, VLC, mpv | Chrome only |

### Content-Type Guidance for Encoder

- **`video/webm`** -- Use when audio is Opus or Vorbis (official WebM codecs).
  Strict players/browsers will reject non-standard audio codecs in WebM.
- **`video/x-matroska`** -- Use when audio is AAC, MP3, or FLAC.
  Matroska allows all codecs. Desktop players handle this fine.
  Browsers may not play MKV natively but ffplay/VLC/mpv will.

### Encoder Requirements for AAC in Matroska

The encoder must set these correctly in the EBML TrackEntry:
- **CodecID:** `A_AAC` (string, exactly 5 bytes)
- **CodecPrivate:** AudioSpecificConfig (2-5 bytes depending on profile)
  - AAC-LC 44100Hz stereo: `0x12 0x10` (2 bytes)
  - HE-AAC v1 44100Hz stereo: `0x2B 0x92 0x08 0x00` (4 bytes)
- **SamplingFrequency:** Must match actual sample rate
- **Channels:** Must be set (1=mono, 2=stereo, 6=5.1, etc.)
- **Content-Type header:** `video/x-matroska` (not `video/webm`)

### Encoder Requirements for MP3 in Matroska

- **CodecID:** `A_MPEG/L3`
- **CodecPrivate:** Not needed (MP3 frames are self-describing)
- **Content-Type header:** `video/x-matroska`

## Priority 1: H.264/AVC in FLV Container
**Why:** Most common live video codec. OBS, Wirecast, and most hardware encoders output H.264+FLV.
The Mcaster1 DSP Encoder already supports H.264 via VideoToolbox on macOS and Media Foundation on Windows.

**What's needed in DNAS:**
- \`src/flv.c\` — Add FLV video tag (type 0x09) handler alongside existing audio tag (0x08)
- Parse AVC sequence header (tag type 0x09, codec ID 7, AVC packet type 0)
- Store SPS/PPS for relay to new listeners (like EBML header caching)
- Parse AVC NALU packets (AVC packet type 1) — just pass through, no decode needed
- Handle composition time offset for B-frames
- Content-Type: \`video/x-flv\` (already mapped in format.c)

**Encoder sends:**
\`\`\`
PUT /stream.flv HTTP/1.1
Content-Type: video/x-flv
\`\`\`
FLV header (9 bytes) → FLV tag 0x09 (AVC seq header) → FLV tag 0x08 (AAC config) → interleaved video/audio tags

**Key files to modify:**
- \`src/flv.c\` / \`src/flv.h\` — Add video tag parsing + sync point detection on keyframes
- \`src/format.c\` — Ensure \`video/x-flv\` and \`application/x-flv\` map to FLV handler
- \`src/source.c\` — FLV video streams need larger queue/burst sizes (like EBML VIDEO flag)

## Priority 2: AV1 in WebM/Matroska
**Why:** Royalty-free VP9 successor. 30% better compression. YouTube/Netflix default codec.
**What's needed:** Minimal — EBML handler already parses WebM containers.
- Just verify the existing EBML handler works with AV1 codec ID (\`V_AV1\`)
- Add content-type mapping if needed: \`video/webm\` already works
- The encoder would use libsvtav1 or libaom for encoding
- Test with: \`ffmpeg -f lavfi -i testsrc=1280x720:r=30 -c:v libsvtav1 -f webm - | curl -T - ...\`

## Priority 3: AAC Audio in WebM/Matroska -- NO DNAS CHANGES NEEDED
**Status:** Already works. EBML handler is codec-agnostic pass-through.
The encoder just needs to use `Content-Type: video/x-matroska` and set CodecID `A_AAC`
with correct AudioSpecificConfig in CodecPrivate. See "Audio Codecs in WebM/Matroska"
section above for full details.

## Priority 4: MPEG-TS Container (H.264 + AAC)
**Why:** Used by HLS, IPTV, and satellite broadcast. Universal player support.
**What's needed:**
- New format handler: \`format_mpegts.c\`
- Parse TS packets (188 bytes each), extract PES streams
- Sync on 0x47 byte, detect PAT/PMT for stream discovery
- Content-Type: \`video/MP2T\` (already in codec table above)

## Priority 5: Raw FLAC (without Ogg wrapper)
**Why:** Many encoders output raw FLAC (audio/flac) without Ogg encapsulation.
**What's needed:**
- Add FLAC frame sync detection (\`0xFFF8\` / \`0xFFF9\`) in \`src/mpeg.c\` or new \`format_flac_raw.c\`
- Content-Type: \`audio/flac\`
- Frame sizes vary — detect frame boundaries by parsing the FLAC frame header

## Priority 6: SRT Subtitles in WebM
**Why:** Subtitle support for video streams (accessibility, multi-language).
**What's needed:**
- EBML handler already supports multiple tracks — add subtitle TrackEntry parsing
- Codec ID: \`S_TEXT/UTF8\` (SRT) or \`S_TEXT/ASS\` (ASS/SSA)
- Pass through subtitle blocks in clusters

---

## Architecture Notes

### Video Queue Sizing
Video streams need larger buffers than audio:
- Audio: 64KB queue typical
- Video: 512KB-2MB queue needed (keyframes can be 50-200KB)
- The \`FORMAT_FLAG_VIDEO\` flag (added in the EBML patch) auto-scales queue/burst sizes

### Keyframe Sync Points
For video, listeners MUST start playback from a keyframe:
- EBML: Cluster boundaries with keyframe SimpleBlocks → \`SOURCE_BLOCK_SYNC\` flag
- FLV: Video tags with keyframe flag (0x10 in first byte) → sync point
- MPEG-TS: IDR NALU (NAL type 5) or I-frame → sync point
- New listeners should be sent the most recent sync point, not arbitrary stream data

### Header Caching for Video
Each video format needs cached initialization data for new listeners:
- **WebM/MKV:** EBML Header + Segment + Info + Tracks (already implemented in format_ebml.c)
- **FLV:** FLV file header (9 bytes) + first video tag (AVC seq header) + first audio tag (AAC config)
- **MPEG-TS:** PAT + PMT tables

### Content-Type -> Format Handler Mapping
Current `format.c` mappings (verified 2026-03-31):
```c
"audio/webm"             -> format_ebml  VERIFIED (audio-only WebM)
"video/webm"             -> format_ebml  VERIFIED (VP9+Opus working end-to-end)
"audio/x-matroska"       -> format_ebml  VERIFIED (audio-only MKV)
"video/x-matroska"       -> format_ebml  VERIFIED (MKV with any codec combo)
"video/x-matroska-3d"    -> format_ebml  VERIFIED (3D MKV)
"video/x-flv"            -> format_flv   NEEDS WORK (audio-only, no video tag 0x09)
"application/x-flv"      -> NOT MAPPED   NEEDS ADDING
"video/MP2T"             -> format_mpeg  BASIC (packet pass-through, no PES parsing)
"video/ogg"              -> format_ogg   VERIFIED (Theora)
```
Note: All EBML/WebM/Matroska paths are codec-agnostic. VP9, VP8, AV1, H.264 video
and Opus, Vorbis, AAC, MP3, FLAC audio all pass through identically.

---

## Encoder-Side Status
The Mcaster1 DSP Encoder (macOS + Windows) supports:
- **Video codecs:** H.264 (VideoToolbox/MF), VP8, VP9 (libvpx)
- **Audio codecs:** MP3, Vorbis, Opus, FLAC, AAC-LC, HE-AAC v1/v2, AAC-ELD (fdk-aac)
- **Containers:** FLV, WebM, MKV, HLS segments
- **Transports:** Icecast2 PUT (HTTP/HTTPS), RTMP, HLS (local files)
- **SSL/TLS:** macOS SecureTransport, Windows OpenSSL

The encoder is ready to send H.264+FLV and AV1+WebM once the DNAS supports them.

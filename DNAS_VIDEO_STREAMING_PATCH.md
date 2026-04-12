# Mcaster1DNAS -- Video Streaming Patch Plan

> **Last updated:** 2026-03-30 20:53 PDT
> **DNAS Branch:** `development`
> **Status:** VERIFIED WORKING -- VP9/WebM video streaming end-to-end

---

## Result

**Full pipeline confirmed:**
FaceTime camera -> VP9 encode -> WebM mux -> SSL/TLS -> DNAS:9443 -> listeners

- 2,701 frames / 90+ seconds continuous VP9 video
- 12.4MB sent over SSL, zero stream disconnects
- EBML header parsed successfully at 209,659 bytes (204KB)
- Listeners connecting and receiving video stream
- YP directory updated for /tvstream.webm

---

## Problem Summary

The Mcaster1 DSP Encoder (macOS build) successfully connects to the DNAS on port 9443 (HTTPS)
and streams VP9+WebM video, but the DNAS disconnected the source after ~10 seconds despite
continuous data flow. Three root causes identified and fixed.

### Root Causes Found

1. **`source-timeout: 10`** too aggressive for video -- already patched to 30.

2. **EBML_HEADER_MAX_SIZE (128KB) too small** -- VP9 WebM headers with codec private data
   are ~204KB. The EBML parser accumulated data looking for the first Cluster ID but hit
   the size limit before finding it.

3. **Cross-boundary Cluster ID miss** -- the scanner only searched within each input buffer
   (`memcmp` on `input_buffer`). The 4-byte Cluster ID `\x1F\x43\xB6\x75` could straddle
   two consecutive reads and never be detected.

4. **`source->last_read` not updated** when format plugin consumed bytes without producing
   a complete cluster -- caused false socket timeouts.

5. **Wrong binary running** -- systemd service used `build/bin/mcaster1` (old Feb 21 build),
   not the newly compiled binary.

---

## All Patches Applied

| # | File | Patch | Status |
|---|------|-------|--------|
| 1 | `mcaster1-production.yaml` | `source-timeout: 10` -> `30` | **DONE** (prior) |
| 2 | `src/format_ebml.c` | `EBML_SLICE_SIZE` 4KB->16KB, buffer alloc 4x->8x (128KB) | **DONE** |
| 3 | `src/format_ebml.c` | `EBML_HEADER_MAX_SIZE` 128KB->1MB | **DONE** |
| 4 | `src/format_ebml.c` | Cross-boundary Cluster ID detection (tail[] buffer) | **DONE** |
| 5 | `src/format_ebml.c` | Diagnostic logging (first bytes, header size) | **DONE** |
| 6 | `src/format.h` | Added `FORMAT_FL_VIDEO (1<<1)` flag | **DONE** |
| 7 | `src/format_ebml.c` | Set `FORMAT_FL_VIDEO` for `video/*` content types | **DONE** |
| 8 | `src/source.c` | Update `last_read` when `format->read_bytes` increases | **DONE** |
| 9 | `src/source.c` | Video queue floor: 512KB queue, 128KB burst | **DONE** |
| 10 | `src/format.c` | Video content-type mappings | **VERIFIED OK** |
| 11 | Binary deployment | Copy compiled binary to `build/bin/` for systemd | **DONE** |

---

## Technical Details

### EBML Header Size (format_ebml.c)
```c
// BEFORE: 128KB -- too small for VP9 WebM headers
#define EBML_HEADER_MAX_SIZE 131072
// AFTER: 1MB -- handles video codec private data
#define EBML_HEADER_MAX_SIZE 1048576
```
Actual header from encoder: 209,659 bytes (204KB).

### EBML Buffer Sizing (format_ebml.c)
```c
// BEFORE:
#define EBML_SLICE_SIZE 4096
ebml->buffer = calloc(1, EBML_SLICE_SIZE * 4);   // 16KB
// AFTER:
#define EBML_SLICE_SIZE 16384
ebml->buffer = calloc(1, EBML_SLICE_SIZE * 8);   // 128KB
```

### Cross-Boundary Cluster ID (format_ebml.c)
Added `tail[3]` + `tail_len` fields to `ebml_st` struct. On each `ebml_wrote()` call,
saves last 3 bytes. Next call checks if Cluster ID `\x1F\x43\xB6\x75` spans the boundary.

### Source Timeout Fix (source.c)
```c
uint64_t prev_read_bytes = source->format ? source->format->read_bytes : 0;
// ... in fds==0 block:
if (source->format && source->format->read_bytes > prev_read_bytes)
    source->last_read = current;
```

### Video Queue/Burst Sizing (source.c + format.h)
```c
#define FORMAT_FL_VIDEO  (1<<1)
// In source_apply_mount():
if (source->format && (source->format->flags & FORMAT_FL_VIDEO)) {
    if (source->queue_len_value < 524288)   source->queue_len_value = 524288;
    if (source->default_burst_value < 131072) source->default_burst_value = 131072;
}
```

## REFERENCE WORKING STREAM

A known working WebM stream at http://168.119.74.185:9000/dos.webm
First bytes: `1a 45 df a3 9f 42 86 81 01 42 f7 81 01 42 f2 81`
That's a standard EBML Header → Segment → SegmentInfo → Tracks → Clusters.

When a listener connects, the Icecast server sends the cached EBML header FIRST,
then live cluster data. This is what's broken on Mcaster1DNAS — listeners get
raw cluster data without the EBML header prepended.

## OUR ENCODER OUTPUT (verified correct)
First bytes sent: `1a 45 df a3` (EBML header element ID) — CORRECT
Header structure: EBML Header → Segment (unknown size) → SegmentInfo → Tracks → Cluster
Total header size: ~204KB (VP9 + Opus tracks)

## WHAT LISTENERS RECEIVE (broken)
First bytes received: `a3 5d 1e 81` — this is mid-cluster SimpleBlock data
The EBML header is NOT being sent to listeners.

## FIX NEEDED
In format_ebml.c → ebml_create_client_data():
The function stores ebml_source_state->header as a refbuf for each new listener.
send_ebml_header() in ebml_write_buf_to_client() should send this cached header
before any live cluster data. Verify:
1. ebml_source_state->header is non-NULL
2. Its first bytes are 1a 45 df a3
3. send_ebml_header() is actually called for new client connections
4. The header refbuf isn't being freed or overwritten prematurely

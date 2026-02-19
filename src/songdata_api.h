/* Mcaster1DNAS - Song Data History API
 *
 * Tracks the last N song title changes across all mount points and exposes
 * them via the /songdata and /admin/songdata endpoints as XML.
 *
 * Copyright 2026, Mcaster1 Project Contributors
 * Distributed under the GNU General Public License, version 2.
 */

#ifndef __SONGDATA_API_H__
#define __SONGDATA_API_H__

#include <time.h>
#include <libxml/tree.h>

/* One entry in the song history ring buffer */
typedef struct songdata_entry_tag
{
    char   *mount;          /* mount point e.g. "/grr-mp3-hd" */
    char   *title;          /* song title string from ICY/vorbis metadata */
    char   *artist;         /* artist (may be NULL if not split by encoder) */
    char   *codec;          /* "MP3", "AAC", "Vorbis", "Opus", etc. */
    int     samplerate;     /* Hz — 44100, 48000, etc. (0 if unknown) */
    int     channels;       /* 1=mono, 2=stereo (0 if unknown) */
    int     bitrate;        /* kbps (0 if unknown) */
    long    listeners;      /* listener count at moment title changed */
    time_t  played_at;      /* Unix timestamp when this title started */
    time_t  ended_at;       /* Unix timestamp when next title started; 0 = still playing */
} songdata_entry_t;

/* Initialise the song history buffer.
 * limit = max entries to keep (0 = unlimited).
 * Call once at server startup after config is loaded. */
void songdata_init (int limit);

/* Push a new song entry.  Called from logging_playlist() so it fires whenever
 * a real title change has already been confirmed by the format handler.
 * Duplicate detection: if title+mount match the most recent entry for that
 * mount, the call is silently ignored.
 * Also fills ended_at on the previous entry for the same mount. */
void songdata_push (const char *mount, const char *title, long listeners);

/* Build and return an XML document rooted at <mcaster1songdata>.
 * If filter_mount is non-NULL, only entries for that mount are included.
 * Caller must xmlFreeDoc() the returned document. */
xmlDocPtr songdata_get_xml (const char *filter_mount);

/* Free all resources.  Call at server shutdown. */
void songdata_destroy (void);

#endif /* __SONGDATA_API_H__ */

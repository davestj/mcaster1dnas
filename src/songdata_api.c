/* Mcaster1DNAS - Song Data History API
 *
 * In-memory ring buffer of recent song title changes, exposed as XML via the
 * /songdata and /admin/songdata endpoints.
 *
 * Thread safety: all access to the global history list is protected by
 * songdata_lock (a mutex shared with stats.c conventions).
 *
 * Copyright 2026, Mcaster1 Project Contributors
 * Distributed under the GNU General Public License, version 2.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "compat.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#include <libxml/tree.h>

#include "cfgfile.h"
#include "thread/thread.h"
#include "util.h"
#include "stats.h"

#define CATMODULE "songdata"
#include "logging.h"

#include "songdata_api.h"

/* ------------------------------------------------------------------ */
/* Internal state                                                       */
/* ------------------------------------------------------------------ */

static mutex_t          songdata_lock;
static songdata_entry_t *history      = NULL;  /* dynamic array */
static int               history_len  = 0;     /* allocated slots */
static int               history_used = 0;     /* entries actually filled */
static int               history_head = 0;     /* index of oldest entry (ring) */
static int               history_limit = 25;   /* 0 = unlimited */

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

/* Return index of the Nth-newest entry (0 = newest).
 * Returns -1 if n >= history_used. */
static int newest_idx (int n)
{
    if (n >= history_used)
        return -1;
    if (history_limit > 0)
    {
        /* ring buffer mode */
        int idx = history_head - 1 - n;
        while (idx < 0)
            idx += history_len;
        return idx;
    }
    else
    {
        /* unlimited — history is a simple growing array; newest at end */
        return history_used - 1 - n;
    }
}

static void free_entry_fields (songdata_entry_t *e)
{
    free (e->mount);  e->mount  = NULL;
    free (e->title);  e->title  = NULL;
    free (e->artist); e->artist = NULL;
    free (e->codec);  e->codec  = NULL;
}

/* Pull codec / samplerate / channels / bitrate from stats for a mount */
static void fill_tech_fields (songdata_entry_t *e, const char *mount)
{
    char *v;

    /* codec */
    v = stats_get_value (mount, "audio_codecid");
    if (!v) v = stats_get_value (mount, "subtype");
    if (v)
    {
        /* audio_codecid is a number: 2=MP3, 10=AAC */
        if (strcmp(v,"2") == 0)
            e->codec = strdup("MP3");
        else if (strcmp(v,"10") == 0)
            e->codec = strdup("AAC");
        else
            e->codec = strdup(v);  /* vorbis/opus: "subtype" is already a name */
        free(v);
    }
    else
        e->codec = strdup("Unknown");

    /* samplerate — MP3 uses mpeg_samplerate, Ogg uses audio_samplerate */
    v = stats_get_value (mount, "mpeg_samplerate");
    if (!v) v = stats_get_value (mount, "audio_samplerate");
    if (v) { e->samplerate = atoi(v); free(v); }

    /* channels — mpeg_channels or audio_channels */
    v = stats_get_value (mount, "mpeg_channels");
    if (!v) v = stats_get_value (mount, "audio_channels");
    if (v) { e->channels = atoi(v); free(v); }

    /* bitrate — ice-bitrate (kbps) preferred; audio_bitrate is bps */
    v = stats_get_value (mount, "ice-bitrate");
    if (v) { e->bitrate = atoi(v); free(v); }
    else
    {
        v = stats_get_value (mount, "audio_bitrate");
        if (v) { e->bitrate = atoi(v) / 1000; free(v); }
    }
}

/* Parse "Artist - Title" into separate fields if no separate artist stat */
static void split_title_artist (const char *raw, char **out_artist, char **out_title)
{
    const char *sep = strstr(raw, " - ");
    if (sep && sep != raw)
    {
        size_t alen = (size_t)(sep - raw);
        *out_artist = malloc(alen + 1);
        memcpy(*out_artist, raw, alen);
        (*out_artist)[alen] = '\0';
        *out_title = strdup(sep + 3);
    }
    else
    {
        *out_artist = NULL;
        *out_title  = strdup(raw);
    }
}

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */

void songdata_init (int limit)
{
    thread_mutex_create (&songdata_lock);
    history_limit = limit;

    if (limit > 0)
    {
        history_len  = limit;
        history      = calloc (limit, sizeof(songdata_entry_t));
    }
    /* unlimited: start with a small block and realloc as needed */
    else
    {
        history_len  = 64;
        history      = calloc (64, sizeof(songdata_entry_t));
    }
    history_used = 0;
    history_head = 0;
}


void songdata_push (const char *mount, const char *title, long listeners)
{
    if (!mount || !title || title[0] == '\0')
        return;

    thread_mutex_lock (&songdata_lock);

    /* --- duplicate guard: skip if same title+mount as most recent entry for this mount --- */
    for (int n = 0; n < history_used; n++)
    {
        int idx = newest_idx(n);
        if (idx < 0) break;
        songdata_entry_t *e = &history[idx];
        if (e->mount && strcmp(e->mount, mount) == 0)
        {
            /* found most recent entry for this mount */
            if (e->title && strcmp(e->title, title) == 0)
            {
                /* identical title — skip */
                thread_mutex_unlock (&songdata_lock);
                return;
            }
            /* different title — fill ended_at on the outgoing entry */
            if (e->ended_at == 0)
                e->ended_at = time(NULL);
            break;
        }
    }

    /* --- select slot to write into --- */
    songdata_entry_t *slot = NULL;

    if (history_limit > 0)
    {
        /* ring buffer: overwrite oldest when full */
        slot = &history[history_head];
        free_entry_fields(slot);
        history_head = (history_head + 1) % history_len;
        if (history_used < history_len)
            history_used++;
    }
    else
    {
        /* unlimited: grow array if needed */
        if (history_used >= history_len)
        {
            int newlen = history_len * 2;
            songdata_entry_t *tmp = realloc(history, newlen * sizeof(songdata_entry_t));
            if (!tmp)
            {
                thread_mutex_unlock (&songdata_lock);
                return;
            }
            memset(tmp + history_len, 0, (newlen - history_len) * sizeof(songdata_entry_t));
            history     = tmp;
            history_len = newlen;
        }
        slot = &history[history_used];
        history_used++;
    }

    /* --- populate the new slot --- */
    memset(slot, 0, sizeof(*slot));
    slot->mount     = strdup(mount);
    slot->played_at = time(NULL);
    slot->ended_at  = 0;
    slot->listeners = listeners;

    /* Split raw ICY "Artist - Title" string — safe: no stats lock needed here.
     * We intentionally do NOT call stats_get_value() here because this function
     * is invoked from logging_playlist() which is called while the stats AVL
     * write lock may already be held (e.g. from metadata_setup in format_mp3.c).
     * Calling stats_get_value() would try to acquire a read lock on the same
     * tree and trigger an abort.  Tech fields (codec, bitrate, etc.) are filled
     * lazily in songdata_get_xml() which runs in a safe HTTP request context. */
    split_title_artist(title, &slot->artist, &slot->title);

    thread_mutex_unlock (&songdata_lock);
}


xmlDocPtr songdata_get_xml (const char *filter_mount)
{
    char timebuf[128];
    char numbuf[32];

    /* Take a snapshot of the history under the lock (shallow copies of strings),
     * then release the lock before calling fill_tech_fields() which calls
     * stats_get_value() and would deadlock if songdata_lock were still held
     * while the source thread holds the stats avl write lock. */
    int snap_count = 0;
    songdata_entry_t *snap = NULL;

    thread_mutex_lock (&songdata_lock);
    if (history_used > 0)
    {
        snap = calloc(history_used, sizeof(songdata_entry_t));
        for (int n = 0; n < history_used; n++)
        {
            int idx = newest_idx(n);
            if (idx < 0) break;
            songdata_entry_t *src = &history[idx];
            if (filter_mount && (!src->mount || strcmp(src->mount, filter_mount) != 0))
                continue;
            songdata_entry_t *dst = &snap[snap_count++];
            dst->mount      = src->mount  ? strdup(src->mount)  : NULL;
            dst->title      = src->title  ? strdup(src->title)  : NULL;
            dst->artist     = src->artist ? strdup(src->artist) : NULL;
            dst->codec      = src->codec  ? strdup(src->codec)  : NULL;
            dst->samplerate = src->samplerate;
            dst->channels   = src->channels;
            dst->bitrate    = src->bitrate;
            dst->played_at  = src->played_at;
            dst->ended_at   = src->ended_at;
            dst->listeners  = src->listeners;
        }
    }
    thread_mutex_unlock (&songdata_lock);

    /* Build XML from snapshot — stats_get_value() is safe to call here */
    xmlDocPtr  doc  = xmlNewDoc (XMLSTR("1.0"));
    xmlNodePtr root = xmlNewDocNode (doc, NULL, XMLSTR("mcaster1songdata"), NULL);
    xmlDocSetRootElement (doc, root);

    util_get_clf_time (timebuf, sizeof(timebuf), time(NULL));
    xmlNewTextChild (root, NULL, XMLSTR("generated"), XMLSTR(timebuf));

    snprintf (numbuf, sizeof(numbuf), "%d", snap_count);
    xmlNewTextChild (root, NULL, XMLSTR("total_entries"), XMLSTR(numbuf));

    for (int n = 0; n < snap_count; n++)
    {
        songdata_entry_t *e = &snap[n];

        /* Fill tech fields from live stats (codec may still be unknown for
         * active mounts; for disconnected sources returns "Unknown") */
        if (!e->codec && e->mount)
            fill_tech_fields(e, e->mount);

        xmlNodePtr entry = xmlNewTextChild (root, NULL, XMLSTR("entry"), NULL);

        xmlNewTextChild (entry, NULL, XMLSTR("mount"),
            XMLSTR(e->mount ? e->mount : ""));
        xmlNewTextChild (entry, NULL, XMLSTR("title"),
            XMLSTR(e->title ? e->title : ""));
        xmlNewTextChild (entry, NULL, XMLSTR("artist"),
            XMLSTR(e->artist ? e->artist : ""));
        xmlNewTextChild (entry, NULL, XMLSTR("codec"),
            XMLSTR(e->codec ? e->codec : ""));

        snprintf (numbuf, sizeof(numbuf), "%d", e->samplerate);
        xmlNewTextChild (entry, NULL, XMLSTR("samplerate"), XMLSTR(numbuf));

        snprintf (numbuf, sizeof(numbuf), "%d", e->channels);
        xmlNewTextChild (entry, NULL, XMLSTR("channels"), XMLSTR(numbuf));

        snprintf (numbuf, sizeof(numbuf), "%d", e->bitrate);
        xmlNewTextChild (entry, NULL, XMLSTR("bitrate"), XMLSTR(numbuf));

        /* played_at as CLF string and Unix timestamp */
        util_get_clf_time (timebuf, sizeof(timebuf), e->played_at);
        xmlNewTextChild (entry, NULL, XMLSTR("played_at"), XMLSTR(timebuf));
        snprintf (numbuf, sizeof(numbuf), "%ld", (long)e->played_at);
        xmlNewTextChild (entry, NULL, XMLSTR("played_at_unix"), XMLSTR(numbuf));

        /* time on air: for current track use now; for historical use ended_at */
        time_t ref = (e->ended_at > 0) ? e->ended_at : time(NULL);
        long on_air = (long)(ref - e->played_at);
        if (on_air < 0) on_air = 0;
        snprintf (numbuf, sizeof(numbuf), "%ld", on_air);
        xmlNewTextChild (entry, NULL, XMLSTR("on_air_seconds"), XMLSTR(numbuf));

        snprintf (numbuf, sizeof(numbuf), "%ld", e->listeners);
        xmlNewTextChild (entry, NULL, XMLSTR("listeners"), XMLSTR(numbuf));

        /* free snapshot entry strings */
        free_entry_fields(e);
    }
    free(snap);

    return doc;
}


void songdata_destroy (void)
{
    thread_mutex_lock (&songdata_lock);
    for (int i = 0; i < history_used; i++)
    {
        int idx = (history_limit > 0) ? i : i;
        free_entry_fields (&history[idx]);
    }
    free (history);
    history      = NULL;
    history_used = 0;
    history_len  = 0;
    thread_mutex_unlock (&songdata_lock);
    thread_mutex_destroy (&songdata_lock);
}

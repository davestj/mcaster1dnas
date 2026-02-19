/* Mcaster1
 *
 *  This program is distributed under the GNU General Public License,
 *  version 2. A copy of this license is included with this source.
 *  At your option, this specific source file can also be distributed
 *  under the GNU GPL version 3.
 *
 * Copyright 2012,      David Richards, Mozilla Foundation,
 *                      and others (see AUTHORS for details).
 */


/* Ogg codec handler for opus streams */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <ogg/ogg.h>

typedef struct source_tag source_t;

#include "format_opus.h"
#include "refbuf.h"
#include "client.h"
#include "stats.h"
#include "format_ogg.h"
#include "logging.h"

#define CATMODULE "format-opus"
#include "logging.h"


/* Handle ICY metadata tag updates (title/artist) for Opus streams */
static void opus_set_tag (format_plugin_t *plugin, const char *tag, const char *value, const char *charset)
{
    ogg_state_t *ogg_info = plugin->_state;

    if (ogg_info->use_url_metadata == 0)
        return;

    if (tag == NULL)
    {
        /* Commit signal — log title change to playlist log and songdata API */
        if (ogg_info->title && ogg_info->title[0])
        {
            char *metadata;
            if (ogg_info->artist && ogg_info->artist[0])
            {
                size_t len = strlen(ogg_info->artist) + strlen(ogg_info->title) + 4;
                metadata = malloc(len);
                snprintf(metadata, len, "%s - %s", ogg_info->artist, ogg_info->title);
            }
            else
                metadata = strdup(ogg_info->title);

            char *ls = stats_get_value(ogg_info->mount, "listeners");
            long listeners = ls ? atol(ls) : 0;
            free(ls);

            logging_playlist(ogg_info->mount, metadata, listeners);
            stats_event_time(ogg_info->mount, "metadata_updated", STATS_GENERAL);
            free(metadata);
        }
        return;
    }

    if (strcmp (tag, "title") == 0 || strcmp (tag, "song") == 0)
    {
        free (ogg_info->title);
        ogg_info->title = value ? strdup (value) : NULL;
        stats_event (ogg_info->mount, "title", value);
        stats_event (ogg_info->mount, "yp_currently_playing", value);
    }
    else if (strcmp (tag, "artist") == 0)
    {
        free (ogg_info->artist);
        ogg_info->artist = value ? strdup (value) : NULL;
        stats_event (ogg_info->mount, "artist", value);
    }
}

static void opus_codec_free (ogg_state_t *ogg_info, ogg_codec_t *codec)
{
    ogg_stream_clear (&codec->os);
    free (codec);
}


static refbuf_t *process_opus_page (ogg_state_t *ogg_info,
        ogg_codec_t *codec, ogg_page *page)
{
    refbuf_t *refbuf;

    if (codec->headers < 2)
    {
        ogg_packet packet;

        ogg_stream_pagein (&codec->os, page);
        while (ogg_stream_packetout (&codec->os, &packet) > 0)
        {
           /* first time around (normal case) yields comments */
           codec->headers++;
        }
        /* add header page to associated list */
        format_ogg_attach_header (codec, page);
        return NULL;
    }
    refbuf = make_refbuf_with_page (codec, page);
    return refbuf;
}


ogg_codec_t *initial_opus_page (format_plugin_t *plugin, ogg_page *page)
{
    ogg_state_t *ogg_info = plugin->_state;
    ogg_codec_t *codec = calloc (1, sizeof (ogg_codec_t));
    ogg_packet packet;

    ogg_stream_init (&codec->os, ogg_page_serialno (page));
    ogg_stream_pagein (&codec->os, page);

    ogg_stream_packetout (&codec->os, &packet);

    DEBUG0("checking for opus codec");
    if (strncmp((char *)packet.packet, "OpusHead", 8) != 0)
    {
        ogg_stream_clear (&codec->os);
        free (codec);
        return NULL;
    }
    INFO0 ("seen initial opus header");
    codec->process_page = process_opus_page;
    codec->codec_free = opus_codec_free;
    codec->headers = 1;
    codec->parent = ogg_info;
    codec->name = "Opus";
    format_ogg_attach_header (codec, page);

    /* Parse OpusHead to extract channel count and input sample rate.
     * OpusHead layout (little-endian):
     *   bytes  0- 7: "OpusHead"
     *   byte   8   : version
     *   byte   9   : channel count
     *   bytes 10-11: pre-skip
     *   bytes 12-15: input sample rate (Hz)
     *   bytes 16-17: output gain
     *   byte  18   : channel mapping family
     */
    if (packet.bytes >= 19)
    {
        unsigned char channels = packet.packet[9];
        uint32_t samplerate =
            ((uint32_t)packet.packet[12])        |
            ((uint32_t)packet.packet[13] << 8)   |
            ((uint32_t)packet.packet[14] << 16)  |
            ((uint32_t)packet.packet[15] << 24);

        char tmp[32];
        snprintf (tmp, sizeof (tmp), "%u", (unsigned)channels);
        stats_event (ogg_info->mount, "audio_channels", tmp);

        if (samplerate > 0)
        {
            snprintf (tmp, sizeof (tmp), "%u", (unsigned)samplerate);
            stats_event (ogg_info->mount, "audio_samplerate", tmp);
        }
        INFO2 ("Opus stream: %u channel(s), %u Hz input rate", (unsigned)channels, (unsigned)samplerate);
    }

    /* Register tag handler so ICY metadata updates work for Opus */
    plugin->set_tag = opus_set_tag;

    return codec;
}


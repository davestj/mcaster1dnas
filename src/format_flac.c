/* Mcaster1
 *
 * This program is distributed under the GNU General Public License, version 2.
 * A copy of this license is included with this source.
 *
 * Copyright 2010-2022, Karl Heyes <karl@kheyes.plus.com>,
 * Copyright 2000-2004, Jack Moffitt <jack@xiph.org>,
 *                      Michael Smith <msmith@xiph.org>,
 *                      oddsock <oddsock@xiph.org>,
 *                      Karl Heyes <karl@xiph.org>
 *                      and others (see AUTHORS for details).
 */


/* Ogg codec handler for FLAC logical streams */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdint.h>
#include <ogg/ogg.h>
#include <string.h>

typedef struct source_tag source_t;

#include "refbuf.h"
#include "format_ogg.h"
#include "client.h"
#include "stats.h"
#include "global.h"

#define CATMODULE "format-flac"
#include "logging.h"


static void flac_codec_free (ogg_state_t *ogg_info, ogg_codec_t *codec)
{
    DEBUG0 ("freeing FLAC codec");
    stats_event (ogg_info->mount, "FLAC_version", NULL);
    stats_event (ogg_info->mount, "FLAC_bits_per_sample", NULL);
    stats_event (ogg_info->mount, "FLAC_total_samples", NULL);
    stats_event (ogg_info->mount, "audio_samplerate", NULL);
    stats_event (ogg_info->mount, "audio_channels", NULL);
    stats_event (ogg_info->mount, "audio_bitrate", NULL);
    ogg_stream_clear (&codec->os);
    format_ogg_free_cached (codec);
    free (codec);
}


/* Parse a Vorbis-comment block body (same format as Ogg Vorbis comments) and
 * emit title/artist stats events. Body starts at the payload after the 4-byte
 * FLAC METADATA_BLOCK_HEADER. All integer fields are little-endian per the
 * Vorbis comment spec. */
static void flac_parse_vorbis_comment (ogg_state_t *ogg_info,
                                       const unsigned char *body, long bytes)
{
    if (body == NULL || bytes < 8) return;
    long pos = 0;
    uint32_t vendor_len =  body[pos]
                        | ((uint32_t)body[pos+1] << 8)
                        | ((uint32_t)body[pos+2] << 16)
                        | ((uint32_t)body[pos+3] << 24);
    pos += 4;
    if (vendor_len > (uint32_t)(bytes - pos)) return;
    pos += (long)vendor_len;
    if (pos + 4 > bytes) return;
    uint32_t num =  body[pos]
                 | ((uint32_t)body[pos+1] << 8)
                 | ((uint32_t)body[pos+2] << 16)
                 | ((uint32_t)body[pos+3] << 24);
    pos += 4;
    if (num > 1024) return;   /* sanity */

    char *title = NULL, *artist = NULL, *album = NULL;
    for (uint32_t i = 0; i < num; i++)
    {
        if (pos + 4 > bytes) break;
        uint32_t clen =  body[pos]
                      | ((uint32_t)body[pos+1] << 8)
                      | ((uint32_t)body[pos+2] << 16)
                      | ((uint32_t)body[pos+3] << 24);
        pos += 4;
        if (clen > (uint32_t)(bytes - pos)) break;
        /* Comment is "KEY=VALUE" UTF-8. Split on first '='. */
        const unsigned char *entry = body + pos;
        uint32_t eq = 0;
        while (eq < clen && entry[eq] != '=') eq++;
        if (eq < clen && eq > 0) {
            char key[64];
            uint32_t klen = eq < sizeof(key) - 1 ? eq : sizeof(key) - 1;
            for (uint32_t k = 0; k < klen; k++) {
                key[k] = (char)((entry[k] >= 'A' && entry[k] <= 'Z') ? entry[k] + 32 : entry[k]);
            }
            key[klen] = '\0';
            uint32_t vlen = clen - eq - 1;
            char *val = malloc (vlen + 1);
            if (val) {
                memcpy (val, entry + eq + 1, vlen);
                val[vlen] = '\0';
                if (strcmp (key, "title") == 0 && title == NULL)         title  = val;
                else if (strcmp (key, "artist") == 0 && artist == NULL)  artist = val;
                else if (strcmp (key, "album") == 0 && album == NULL)    album  = val;
                else free (val);
            }
        }
        pos += (long)clen;
    }

    if (title)
    {
        stats_event (ogg_info->mount, "title", title);
        stats_event (ogg_info->mount, "yp_currently_playing", title);
        free (ogg_info->title);
        ogg_info->title = strdup (title);
    }
    if (artist)
    {
        stats_event (ogg_info->mount, "artist", artist);
        free (ogg_info->artist);
        ogg_info->artist = strdup (artist);
    }
    if (album)
    {
        stats_event (ogg_info->mount, "album", album);
    }

    if (title || artist)
    {
        char *metadata = NULL;
        if (title && artist) {
            size_t len = strlen (title) + strlen (artist) + 4;
            metadata = malloc (len);
            if (metadata) snprintf (metadata, len, "%s - %s", artist, title);
        } else if (title) {
            metadata = strdup (title);
        }
        if (metadata) {
            char *ls = stats_get_value (ogg_info->mount, "listeners");
            long listeners = ls ? atol (ls) : 0;
            free (ls);
            logging_playlist (ogg_info->mount, metadata, listeners);
            stats_event_time (ogg_info->mount, "metadata_updated", STATS_GENERAL);
            free (metadata);
        }
    }

    free (title);
    free (artist);
    free (album);
}


/* Called when the admin interface pushes new metadata (ICY metadata update).
 * For FLAC we just update the stats directly; the codec stream itself will
 * continue to carry its own in-band Vorbis comments on the next block. */
static void flac_set_tag (format_plugin_t *plugin, const char *tag,
                          const char *value, const char *charset)
{
    (void)charset;
    ogg_state_t *ogg_info = plugin->_state;
    if (ogg_info == NULL) return;

    if (tag == NULL)
    {
        /* commit: push the composite "artist - title" string */
        if (ogg_info->title && ogg_info->title[0])
        {
            char *metadata;
            if (ogg_info->artist && ogg_info->artist[0]) {
                size_t len = strlen (ogg_info->artist) + strlen (ogg_info->title) + 4;
                metadata = malloc (len);
                if (metadata) snprintf (metadata, len, "%s - %s", ogg_info->artist, ogg_info->title);
            } else {
                metadata = strdup (ogg_info->title);
            }
            if (metadata) {
                char *ls = stats_get_value (ogg_info->mount, "listeners");
                long listeners = ls ? atol (ls) : 0;
                free (ls);
                logging_playlist (ogg_info->mount, metadata, listeners);
                stats_event_time (ogg_info->mount, "metadata_updated", STATS_GENERAL);
                free (metadata);
            }
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


/* Here, we just verify the page is ok and then add it to the queue */
static refbuf_t *process_flac_page (ogg_state_t *ogg_info, ogg_codec_t *codec, ogg_page *page)
{
    refbuf_t * refbuf;

    if (codec->headers > 0)
    {
        int loop = 20, found_header = 0, packets = 0;
        ogg_packet packet;
        if (ogg_stream_pagein (&codec->os, page) < 0)   // only put in pages during the header detection phase
        {
            ogg_info->error = 1;
            return NULL;
        }
        do
        {
            int pkt = ogg_stream_packetout (&codec->os, &packet);

            if (pkt > 0)
            {
                packets++;
                if (codec->headers)
                {
                    int type = packet.packet[0];
                    if ((type >= 0x01 && type <= 0x7E) || (type >= 0x81 && type <= 0xFE))
                    {   // valid metadata code type
                        /* VORBIS_COMMENT block (type 4, with or without last-block flag).
                         * Body starts after the 4-byte METADATA_BLOCK_HEADER. Length is
                         * BE 24-bit in packet.packet[1..3]. */
                        if ((type & 0x7F) == 4 && packet.bytes > 4)
                        {
                            long blen = ((long)packet.packet[1] << 16)
                                      | ((long)packet.packet[2] << 8)
                                      |  (long)packet.packet[3];
                            if (blen > packet.bytes - 4) blen = packet.bytes - 4;
                            flac_parse_vorbis_comment (ogg_info,
                                    packet.packet + 4, blen);
                        }
                        codec->headers++;
                        found_header++;
                        continue;
                    }
                    if (type == 0x7F)
                    {   // should never be encountered
                        WARN0 ("Found another initial header packet");
                        ogg_info->error = 1;
                        return NULL;
                    }
                    // non-metadata packet
                    if (found_header)
                        WARN0 ("Found an unexpected packet with headers");
                    else
                        format_ogg_free_cached (codec); // found non-header packet, no headers outstanding
                    DEBUG1 ("header packet count %d", codec->headers);
                    codec->headers = 0;         // stop the header processing phase
                }
                continue;
            }
            if (pkt == 0) break;     // usual loop exit path
            loop--;
        } while (loop);
        if (loop == 0)
        {   // packetout can occur but abort in such repeated cases
            WARN0 ("Looping too many times, abort packetout loop");
            ogg_info->error = 1;
            return NULL;
        }
        if (packets == 0)
        {   // unlikely but possible, page added but incomplete packet
            DEBUG0 ("page cached in case of header packet");
            refbuf = make_refbuf_with_page (codec, page);
            *codec->cached_p = refbuf;
            codec->cached_p = (void*)&refbuf->associated;
            return NULL;
        }
        if (found_header)
        {
            // add any previously undetermined header pages
            format_ogg_attach_cached (codec);
            DEBUG0("Adding header page");
            format_ogg_attach_header (codec, page);
            return NULL;
        }
    }
    refbuf = make_refbuf_with_page (codec, page);
    return refbuf;
}


/* Check for flac header in logical stream */

ogg_codec_t *initial_flac_page (format_plugin_t *plugin, ogg_page *page)
{
    ogg_state_t *ogg_info = plugin->_state;
    ogg_codec_t *codec = calloc (1, sizeof (ogg_codec_t));
    ogg_packet packet;

    ogg_stream_init (&codec->os, ogg_page_serialno (page));
    ogg_stream_pagein (&codec->os, page);

    ogg_stream_packetout (&codec->os, &packet);

    DEBUG0("checking for FLAC codec");
    do
    {
        unsigned char *parse = packet.packet;
        // format 0x7F F L A C, '1' x  y y f L a C zzzzz
        // x   1 byte minor number
        // y   2 BE byte count of header packets
        // z   StreamINFO structure

        if (page->header_len + page->body_len != 79)
            break;
        if (parse[0] != 0x7F)
            break;

        if (memcmp (parse+1, "FLAC", 4) != 0)
            break;

        if (parse[5] != 1)
        {
            WARN1 ("Unknown Ogg FLAC version %d, skipping", parse[5]);
            break;
        }
        int headers = ((parse[7]<<8) + parse[8]) + 1; // include this one

        if (memcmp (parse+9, "fLaC", 4) != 0)
            break;
        INFO0 ("seen initial FLAC header");
        stats_event_args (ogg_info->mount, "FLAC_version", "%d.%d",  parse[5], parse[6]);
        if (headers)
            INFO1 ("FLAC stream reports to have %d headers", headers);
        else
            INFO0 ("FLAC stream has undefined number of headers, will check as they come in");

        /* Parse STREAMINFO metadata block (34 bytes of data after the 4-byte
         * METADATA_BLOCK_HEADER). The Ogg FLAC mapping packet layout is:
         *   0x7F 'F' 'L' 'A' 'C' maj min hdr_count_hi hdr_count_lo 'f' 'L' 'a' 'C'
         *   [METADATA_BLOCK_HEADER 4B] [STREAMINFO 34B]
         * So STREAMINFO data starts at parse+17 (13 + 4). */
        if (packet.bytes >= 17 + 18)
        {
            const unsigned char *si = parse + 17;
            /* si[10..12] (first 20 bits) = sample_rate */
            uint32_t sr = ((uint32_t)si[10] << 12)
                        | ((uint32_t)si[11] << 4)
                        | (((uint32_t)si[12] >> 4) & 0x0f);
            /* si[12] bits 1-3 = channels-1, si[12] bit 0 | si[13] hi4 = bps-1 */
            int channels = (int)(((si[12] >> 1) & 0x07) + 1);
            int bps      = (int)((((si[12] & 0x01) << 4) | (si[13] >> 4)) + 1);

            if (sr > 0 && sr < 655350)
                stats_event_args (ogg_info->mount, "audio_samplerate", "%u", (unsigned)sr);
            stats_event_args (ogg_info->mount, "audio_channels", "%d", channels);
            stats_event_args (ogg_info->mount, "FLAC_bits_per_sample", "%d", bps);

            /* Theoretical uncompressed PCM bitrate for display purposes. FLAC
             * is lossless and variable; admins can use outgoing_kbitrate for
             * the actual on-wire rate. */
            if (sr > 0 && channels > 0 && bps > 0)
            {
                long pcm_bps = (long)sr * channels * bps;
                stats_event_args (ogg_info->mount, "audio_bitrate", "%ld", pcm_bps);
            }
            INFO3 ("FLAC stream: %d channel(s), %u Hz, %d-bit",
                    channels, (unsigned)sr, bps);
        }

        codec->process_page = process_flac_page;
        codec->codec_free = flac_codec_free;
        codec->headers = 1;
        codec->parent = ogg_info;
        codec->name = "FLAC";

        /* Register ICY metadata update hook so admin title/artist pushes work. */
        plugin->set_tag = flac_set_tag;

        format_ogg_attach_header (codec, page);
        return codec;
    } while (0);

    ogg_stream_clear (&codec->os);
    free (codec);
    return NULL;
}


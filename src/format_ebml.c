/* Mcaster1
 *
 * This program is distributed under the GNU General Public License,
 * version 2. A copy of this license is included with this source.
 * At your option, this specific source file can also be distributed
 * under the GNU GPL version 3.
 *
 * Copyright 2012,      David Richards, Mozilla Foundation,
 *                      and others (see AUTHORS for details).
 */

/* format_ebml.c
 *
 * format plugin for EBML
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "refbuf.h"
#include "source.h"
#include "client.h"

#include "stats.h"
#include "format.h"
#include "format_ebml.h"

#define CATMODULE "format-ebml"

#include "logging.h"
#include "global.h"

#define EBML_DEBUG 0
#define EBML_HEADER_MAX_SIZE 1048576
#define EBML_SLICE_SIZE 16384


typedef struct ebml_client_data_st ebml_client_data_t;

struct ebml_client_data_st {

    refbuf_t *header;
    int header_pos;

};

struct ebml_st {

    char *cluster_id;
    int cluster_start;

    int position;
    unsigned char *input_buffer;
    unsigned char *buffer;

    int header_read;
    int header_size;
    int header_position;
    int header_read_position;
    unsigned char *header;

    /* Cross-boundary cluster ID detection: last 3 bytes of previous buffer */
    unsigned char tail[3];
    int tail_len;
    int header_logged;

};

static void ebml_free_plugin (format_plugin_t *plugin, client_t *client);
static refbuf_t *ebml_get_buffer (source_t *source);
static int  ebml_write_buf_to_client (client_t *client);
static void  ebml_write_buf_to_file (source_t *source, refbuf_t *refbuf);
static int  ebml_create_client_data (format_plugin_t *format, mc_http_t *http, client_t *client);
static void ebml_free_client_data (client_t *client);
static void ebml_apply_client (format_plugin_t *plugin, client_t *client);

static ebml_t *ebml_create();
static void ebml_destroy(ebml_t *ebml);
static int  ebml_read_space(ebml_t *ebml);
static int ebml_read(ebml_t *ebml, char *buffer, int len);
static int ebml_last_was_sync(ebml_t *ebml);
static char *ebml_write_buffer(ebml_t *ebml, int len);
static int ebml_wrote(ebml_t *ebml, int len);

int format_ebml_get_plugin (format_plugin_t *plugin)
{

    ebml_source_state_t *ebml_source_state = calloc(1, sizeof(ebml_source_state_t));

    plugin->get_buffer = ebml_get_buffer;
    plugin->write_buf_to_client = ebml_write_buf_to_client;
    plugin->create_client_data = ebml_create_client_data;
    plugin->free_plugin = ebml_free_plugin;
    plugin->write_buf_to_file = ebml_write_buf_to_file;
    plugin->set_tag = NULL;
    plugin->apply_settings = NULL;
    plugin->apply_client = ebml_apply_client;
    plugin->_state = ebml_source_state;

    /* Mark as video-capable so source.c can apply larger queue/burst defaults */
    if (plugin->contenttype && (strncmp(plugin->contenttype, "video/", 6) == 0))
        plugin->flags |= FORMAT_FL_VIDEO;

    return 0;
}

static void ebml_free_plugin (format_plugin_t *plugin, client_t *client)
{

    ebml_source_state_t *ebml_source_state = plugin->_state;

    refbuf_release (ebml_source_state->header);
    ebml_destroy (ebml_source_state->ebml);
    free (ebml_source_state);

}


static void ebml_apply_client (format_plugin_t *plugin, client_t *client)
{
    ebml_source_state_t *ebml_source_state = plugin->_state;
    const char *s;

    refbuf_release (ebml_source_state->header);
    ebml_source_state->header = NULL;
    ebml_destroy (ebml_source_state->ebml);
    ebml_source_state->ebml = NULL;
    free (plugin->contenttype);

    if (client == NULL)
        return;

    s = plugin->parser ? httpp_getvar (plugin->parser, "content-type") : NULL;
    plugin->contenttype = strdup (s ? s : "video/x-matroska");
    ebml_source_state->ebml = ebml_create();
}


static int send_ebml_header (client_t *client)
{

    ebml_client_data_t *ebml_client_data = client->format_data;
    int len = EBML_SLICE_SIZE;
    int ret;

    if (ebml_client_data->header->len - ebml_client_data->header_pos < len) 
    {
        len = ebml_client_data->header->len - ebml_client_data->header_pos;
    }
    ret = client_send_bytes (client, 
                             ebml_client_data->header->data + ebml_client_data->header_pos,
                             len);

    if (ret > 0)
    {
        ebml_client_data->header_pos += ret;
    }

    return ret;

}

static int ebml_write_buf_to_client (client_t *client)
{

    ebml_client_data_t *ebml_client_data = client->format_data;

    if (ebml_client_data->header && ebml_client_data->header_pos != ebml_client_data->header->len)
    {
        return send_ebml_header (client);
    }
    else
    {
        return format_generic_write_to_client(client);
    }

}

static refbuf_t *ebml_get_buffer (source_t *source)
{

    ebml_source_state_t *ebml_source_state = source->format->_state;
    format_plugin_t *format = source->format;
    char *data = NULL;
    int bytes = 0;
    refbuf_t *refbuf;
    int ret;

    while (1)
    {

        if ((bytes = ebml_read_space(ebml_source_state->ebml)) > 0)
        {
            refbuf = refbuf_new(bytes);
            ebml_read(ebml_source_state->ebml, refbuf->data, bytes);

            if (ebml_source_state->header == NULL)
            {
                ebml_source_state->header = refbuf;
                if (refbuf->len >= 4)
                {
                    INFO1("EBML source header captured: %d bytes", refbuf->len);
                    INFO4("EBML source header bytes: %02x %02x %02x %02x",
                          (unsigned char)refbuf->data[0], (unsigned char)refbuf->data[1],
                          (unsigned char)refbuf->data[2], (unsigned char)refbuf->data[3]);
                }
                continue;
            }

            if (ebml_last_was_sync(ebml_source_state->ebml))
            {
                refbuf->flags |= SOURCE_BLOCK_SYNC;
            }
            if (refbuf->len > 0)
            {
                source->client->queue_pos += refbuf->len;
            }
            return refbuf;

        }
        else
        {

            data = ebml_write_buffer(ebml_source_state->ebml, EBML_SLICE_SIZE);
            bytes = client_read_bytes (source->client, data, EBML_SLICE_SIZE);
            if (bytes <= 0)
            {
                ebml_wrote (ebml_source_state->ebml, 0);
                return NULL;
            }
            format->read_bytes += bytes;
            ret = ebml_wrote (ebml_source_state->ebml, bytes);
            if (ret != bytes) {
                ERROR0 ("Problem processing stream");
                source->flags &= ~SOURCE_RUNNING;
                return NULL;
            }
        }
    }
}

static int ebml_create_client_data (format_plugin_t *format, mc_http_t *http, client_t *client)
{

    ebml_client_data_t *ebml_client_data = calloc(1, sizeof(ebml_client_data_t));
    ebml_source_state_t *ebml_source_state = format->_state;

    if ((ebml_client_data) && (ebml_source_state->header))
    {
        ebml_client_data->header = ebml_source_state->header;
        refbuf_addref (ebml_client_data->header);
        INFO2("EBML client gets header: %d bytes, first_byte=0x%02x",
              ebml_client_data->header->len,
              (unsigned char)ebml_client_data->header->data[0]);
    }
    else
    {
        WARN1("EBML client has NO header: source_header=%p",
              (void *)ebml_source_state->header);
    }
    client->format_data = ebml_client_data;
    client->free_client_data = ebml_free_client_data;

    if (client->refbuf == NULL)
        client->refbuf = refbuf_new (4096);
    return format_client_headers (format, http, client);
}


static void ebml_free_client_data (client_t *client)
{

    ebml_client_data_t *ebml_client_data = client->format_data;

    refbuf_release (ebml_client_data->header);
    free (client->format_data);
    client->format_data = NULL;
}


static void ebml_write_buf_to_file_fail (source_t *source)
{
    WARN0 ("Write to dump file failed, disabling");
    fclose (source->dumpfile);
    source->dumpfile = NULL;
}


static void ebml_write_buf_to_file (source_t *source, refbuf_t *refbuf)
{

    ebml_source_state_t *ebml_source_state = source->format->_state;

    if (ebml_source_state->file_headers_written == 0)
    {
        if (fwrite (ebml_source_state->header->data, 1,
                    ebml_source_state->header->len, 
                    source->dumpfile) != ebml_source_state->header->len)
            ebml_write_buf_to_file_fail(source);
        else
            ebml_source_state->file_headers_written = 1;
    }

    if (fwrite (refbuf->data, 1, refbuf->len, source->dumpfile) != refbuf->len)
    {
        ebml_write_buf_to_file_fail(source);
    }

}


/* internal ebml parsing */

static void ebml_destroy(ebml_t *ebml)
{

    if (ebml == NULL) return;
    free(ebml->header);
    free(ebml->input_buffer);
    free(ebml->buffer);
    free(ebml);

}

static ebml_t *ebml_create()
{

    ebml_t *ebml = calloc(1, sizeof(ebml_t));
    if (ebml == NULL) return NULL;

    ebml->header = calloc(1, EBML_HEADER_MAX_SIZE);
    ebml->buffer = calloc(1, EBML_SLICE_SIZE * 8);
    ebml->input_buffer = calloc(1, EBML_SLICE_SIZE);

    if (ebml->header == NULL || ebml->buffer == NULL || ebml->input_buffer == NULL) {
        free(ebml->header);
        free(ebml->buffer);
        free(ebml->input_buffer);
        free(ebml);
        return NULL;
    }

    ebml->cluster_id = "\x1F\x43\xB6\x75";

    ebml->cluster_start = -2;

    return ebml;

}

static int ebml_read_space(ebml_t *ebml)
{

    int read_space;

    if (ebml->header_read == 1)
    {
        if (ebml->cluster_start > 0)
            read_space = ebml->cluster_start;
        else
            read_space = ebml->position - 4;

        return read_space;
    }
    else
    {
        if (ebml->header_size != 0)
            return ebml->header_size;
        else
            return 0;
    }

}

static int ebml_read(ebml_t *ebml, char *buffer, int len)
{

    int read_space;
    int to_read;

    if (len < 1)
        return 0;

    if (ebml->header_read == 1)
    {
        if (ebml->cluster_start > 0)
            read_space = ebml->cluster_start;
        else
            read_space = ebml->position - 4;
 
        if (read_space < 1)
            return 0;

        if (read_space >= len )
            to_read = len;
        else
            to_read = read_space;

        memcpy(buffer, ebml->buffer, to_read);
        memmove(ebml->buffer, ebml->buffer + to_read, ebml->position - to_read);
        ebml->position -= to_read;
        
        if (ebml->cluster_start > 0)
            ebml->cluster_start -= to_read;
    }
    else
    {
        if (ebml->header_size != 0)
        {
            read_space = ebml->header_size - ebml->header_read_position;

            if (read_space >= len)
                to_read = len;
            else
                to_read = read_space;

            memcpy(buffer, ebml->header + ebml->header_read_position, to_read);
            if (ebml->header_read_position == 0 && to_read >= 4)
                INFO4("EBML raw header read: %02x %02x %02x %02x",
                      (unsigned char)ebml->header[0], (unsigned char)ebml->header[1],
                      (unsigned char)ebml->header[2], (unsigned char)ebml->header[3]);
            ebml->header_read_position += to_read;

            if (ebml->header_read_position == ebml->header_size)
                ebml->header_read = 1;
        }
        else
        {
            return 0;
        }
    }

    return to_read;

}

static int ebml_last_was_sync(ebml_t *ebml)
{

    if (ebml->cluster_start == 0)
    {
        ebml->cluster_start -= 1;
        return 0;
    }

    if (ebml->cluster_start == -1)
    {
        ebml->cluster_start -= 1;
        return 1;
    }

    return 0;

}

static char *ebml_write_buffer(ebml_t *ebml, int len)
{

    return (char *)ebml->input_buffer;

}


static int ebml_wrote(ebml_t *ebml, int len)
{

    int b;

    if (ebml->header_size == 0)
    {
        if ((ebml->header_position + len) > EBML_HEADER_MAX_SIZE)
        {
            ERROR2("EBML Header too large (%d + %d bytes), failing",
                   ebml->header_position, len);
            return -1;
        }

        /* Log first bytes of stream for diagnostics */
        if (ebml->header_position == 0 && !ebml->header_logged)
        {
            ebml->header_logged = 1;
            if (len >= 4)
            {
                INFO1("EBML first input: %d bytes", len);
                INFO4("EBML first bytes: %02x %02x %02x %02x",
                      ebml->input_buffer[0], ebml->input_buffer[1],
                      ebml->input_buffer[2], ebml->input_buffer[3]);
            }
        }

        memcpy(ebml->header + ebml->header_position, ebml->input_buffer, len);
        ebml->header_position += len;
    }
    else
    {
        memcpy(ebml->buffer + ebml->position, ebml->input_buffer, len);
    }

    /* Check for Cluster ID straddling previous and current buffer boundary */
    if (ebml->tail_len > 0 && len >= 4)
    {
        unsigned char cross[7]; /* max 3 tail + 4 new = 7 */
        int cross_len = ebml->tail_len + (4 - ebml->tail_len);
        if (cross_len > ebml->tail_len + len)
            cross_len = ebml->tail_len + len;
        memcpy(cross, ebml->tail, ebml->tail_len);
        memcpy(cross + ebml->tail_len, ebml->input_buffer,
               cross_len - ebml->tail_len);
        int i;
        for (i = 0; i <= cross_len - 4; i++)
        {
            if (!memcmp(cross + i, ebml->cluster_id, 4))
            {
                /* Cluster ID found spanning buffer boundary */
                int offset_in_new = 4 - (ebml->tail_len - i);
                if (ebml->header_size == 0)
                {
                    ebml->header_size = ebml->header_position - len + offset_in_new - 4;
                    INFO1("EBML header parsed (cross-boundary), size %d bytes",
                          ebml->header_size);
                    memcpy(ebml->buffer, ebml->input_buffer + offset_in_new - 4,
                           len - offset_in_new + 4);
                    ebml->position = len - offset_in_new + 4;
                    ebml->cluster_start = -1;
                    ebml->tail_len = 0;
                    return len;
                }
                else
                {
                    ebml->cluster_start = ebml->position + offset_in_new - 4;
                }
                break;
            }
        }
    }

    for (b = 0; b <= len - 4; b++)
    {
        if (!memcmp(ebml->input_buffer + b, ebml->cluster_id, 4))
        {
            if (ebml->header_size == 0)
            {
                ebml->header_size = ebml->header_position - len + b;
                INFO1("EBML header parsed, size %d bytes", ebml->header_size);
                memcpy(ebml->buffer, ebml->input_buffer + b, len - b);
                ebml->position = len - b;
                ebml->cluster_start = -1;
                ebml->tail_len = 0;
                return len;
            }
            else
            {
                ebml->cluster_start = ebml->position + b;
            }
        }
    }

    /* Save last 3 bytes for cross-boundary detection on next call */
    if (len >= 3)
    {
        memcpy(ebml->tail, ebml->input_buffer + len - 3, 3);
        ebml->tail_len = 3;
    }
    else
    {
        memcpy(ebml->tail + ebml->tail_len, ebml->input_buffer, len);
        ebml->tail_len += len;
        if (ebml->tail_len > 3) ebml->tail_len = 3;
    }

    ebml->position += len;

    return len;

}

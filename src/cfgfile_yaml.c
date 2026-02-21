/* Mcaster1DNAS - Digital Network Audio Server
 *
 * YAML Configuration File Parser
 *
 * This program is distributed under the GNU General Public License, version 2.
 * A copy of this license is included with this source.
 *
 * Copyright 2025-2026, Saint John (David St John) <davestj@gmail.com>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_YAML

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <ctype.h>

#include <yaml.h>
#include <libxml/xmlmemory.h>

#include "cfgfile.h"
#include "cfgfile_yaml.h"
#include "logging.h"
#include "util.h"
#include "auth.h"
#include "compat.h"
#include "global.h"

#define CATMODULE "config-yaml"
#ifdef WIN32
#define YAML_TRACE(msg) do { FILE *_yt=fopen("mcaster1win_start.log","a"); if(_yt){fprintf(_yt,"[yaml] " msg "\n");fclose(_yt);} } while(0)
#else
#define YAML_TRACE(msg) do {} while(0)
#endif

extern mc_config_http_header_t default_headers[];

/* Forward declarations */
static int yaml_parse_chown(yaml_parse_ctx *ctx, yaml_node_t *node);
static config_options_t *yaml_parse_mount_auth_options(yaml_parse_ctx *ctx, yaml_node_t *node);
static int _add_http_header(mc_config_http_header_t **top, const mc_config_http_header_t *src, int log);

/* ============================================================================
 * YAML Error Reporting
 * ============================================================================ */

void yaml_parse_error(yaml_parse_ctx *ctx, yaml_node_t *node, const char *fmt, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (node)
        ERROR2("%s (line %lu)", buffer, (unsigned long)node->start_mark.line + 1);
    else
        ERROR1("%s", buffer);
}

void yaml_parse_warning(yaml_parse_ctx *ctx, yaml_node_t *node, const char *fmt, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (node)
        WARN2("%s (line %lu)", buffer, (unsigned long)node->start_mark.line + 1);
    else
        WARN1("%s", buffer);
}

/* ============================================================================
 * YAML Helper Functions
 * ============================================================================ */

const char *yaml_get_scalar_value(yaml_document_t *doc, yaml_node_t *node)
{
    if (!node || node->type != YAML_SCALAR_NODE)
        return NULL;
    return (const char *)node->data.scalar.value;
}

yaml_node_t *yaml_get_mapping_value(yaml_document_t *doc, yaml_node_t *mapping, const char *key)
{
    if (!mapping || mapping->type != YAML_MAPPING_NODE || !key)
        return NULL;

    yaml_node_pair_t *pair;
    for (pair = mapping->data.mapping.pairs.start;
         pair < mapping->data.mapping.pairs.top; pair++)
    {
        yaml_node_t *key_node = yaml_document_get_node(doc, pair->key);
        const char *key_str = yaml_get_scalar_value(doc, key_node);

        if (key_str && strcasecmp(key_str, key) == 0)
            return yaml_document_get_node(doc, pair->value);
    }
    return NULL;
}

int yaml_get_bool_value(yaml_document_t *doc, yaml_node_t *node, int *result)
{
    const char *str = yaml_get_scalar_value(doc, node);
    if (!str) return -1;

    *result = 0;
    if (strcasecmp(str, "true") == 0 || strcasecmp(str, "yes") == 0 ||
        strcasecmp(str, "on") == 0 || strcmp(str, "1") == 0)
        *result = 1;
    else if (strcasecmp(str, "false") == 0 || strcasecmp(str, "no") == 0 ||
             strcasecmp(str, "off") == 0 || strcmp(str, "0") == 0)
        *result = 0;
    else
        return -1;

    return 0;
}

int yaml_get_int_value(yaml_document_t *doc, yaml_node_t *node, int *result)
{
    const char *str = yaml_get_scalar_value(doc, node);
    if (!str) return -1;

    char *endptr;
    errno = 0;
    long val = strtol(str, &endptr, 10);
    if (errno != 0 || *endptr != '\0' || endptr == str)
        return -1;

    *result = (int)val;
    return 0;
}

int yaml_get_long_value(yaml_document_t *doc, yaml_node_t *node, long *result)
{
    const char *str = yaml_get_scalar_value(doc, node);
    if (!str) return -1;

    char *endptr;
    errno = 0;
    long val = strtol(str, &endptr, 10);
    if (errno != 0 || *endptr != '\0' || endptr == str)
        return -1;

    *result = val;
    return 0;
}

int yaml_get_port_value(yaml_document_t *doc, yaml_node_t *node, int *result)
{
    int val;
    if (yaml_get_int_value(doc, node, &val) < 0)
        return -1;

    if (val <= 0 || val >= 65536) {
        WARN1("port out of range: %d", val);
        return -1;
    }

    *result = val;
    return 0;
}

int yaml_get_bitrate_value(yaml_document_t *doc, yaml_node_t *node, int64_t *result)
{
    const char *str = yaml_get_scalar_value(doc, node);
    if (!str) return -1;

    char *endptr;
    errno = 0;
    int64_t val = strtoll(str, &endptr, 10);

    if (errno == 0 && endptr != str) {
        char metric = tolower(*endptr);
        if (metric == 'k')
            val *= 1000;
        else if (metric == 'm')
            val *= 1000000;
        *result = val;
        return 0;
    }
    return -1;
}

int yaml_get_qsizing_value(yaml_document_t *doc, yaml_node_t *node, uint32_t *result)
{
    const char *str = yaml_get_scalar_value(doc, node);
    if (!str) return -1;

    return config_qsizing_conv_a2n(str, result);
}

int yaml_get_loglevel_value(yaml_document_t *doc, yaml_node_t *node, log_levels_t *result)
{
    const char *str = yaml_get_scalar_value(doc, node);
    if (!str) return -1;

    result->mark = 3; /* default */

    if (strcasecmp(str, "debug") == 0)      result->mark = 4;
    else if (strcasecmp(str, "info") == 0)  result->mark = 3;
    else if (strcasecmp(str, "warn") == 0)  result->mark = 2;
    else if (strcasecmp(str, "error") == 0) result->mark = 1;
    else {
        int v;
        if (sscanf(str, "%d", &v) == 1)
            result->mark = v;
    }

    return 0;
}

/* ============================================================================
 * HTTP Headers Parsing
 * ============================================================================ */

static int _add_http_header(mc_config_http_header_t **top, const mc_config_http_header_t *src, int log)
{
    mc_config_http_header_t **trail = top, *cur = *top;

    while (cur) {
        if (strcasecmp(cur->hdr.name, src->hdr.name) == 0) {
            if (cur->flags & PARAM_CONST)
                return -1;
            if (cur->flags & PARAM_MULTI)
                cur = NULL;
            else {
                xmlFree(cur->hdr.name);
                xmlFree(cur->hdr.value);
                xmlFree(cur->hdr.status);
                cur->hdr = src->hdr;
                break;
            }
        }
        trail = &cur->next;
        cur = *trail;
    }

    if (cur == NULL) {
        cur = malloc(sizeof(*cur));
        *cur = *src;
        cur->next = *trail;
        *trail = cur;
    }

    if ((src->flags & PARAM_NOCOPY) == 0) {
        cur->hdr.name = (char *)xmlCharStrdup(src->hdr.name);
        cur->hdr.value = (char *)xmlCharStrdup(src->hdr.value);
        cur->hdr.status = (char *)xmlCharStrdup(src->hdr.status);
    }

    if (log)
        DEBUG4("Adding HTTP header %s as %s, status %s (%d)",
               cur->hdr.name, cur->hdr.value ? cur->hdr.value : "set later",
               cur->hdr.status, (cur->hdr.callback) ? 1 : 0);

    return 0;
}

int yaml_parse_http_headers(yaml_parse_ctx *ctx, yaml_node_t *node, mc_config_http_header_t **headers)
{
    if (!node || node->type != YAML_SEQUENCE_NODE)
        return -1;

    yaml_node_item_t *item;
    for (item = node->data.sequence.items.start;
         item < node->data.sequence.items.top; item++)
    {
        yaml_node_t *header_node = yaml_document_get_node(ctx->document, *item);
        if (!header_node || header_node->type != YAML_MAPPING_NODE)
            continue;

        yaml_node_t *name_node = yaml_get_mapping_value(ctx->document, header_node, "name");
        yaml_node_t *value_node = yaml_get_mapping_value(ctx->document, header_node, "value");
        yaml_node_t *status_node = yaml_get_mapping_value(ctx->document, header_node, "status");

        const char *name = yaml_get_scalar_value(ctx->document, name_node);
        const char *value = yaml_get_scalar_value(ctx->document, value_node);
        const char *status = yaml_get_scalar_value(ctx->document, status_node);

        if (!name || !name[0]) {
            yaml_parse_warning(ctx, header_node, "HTTP header missing name");
            continue;
        }

        char *name_copy = (char *)xmlCharStrdup(name);
        char *value_copy = value ? (char *)xmlCharStrdup(value) : (char *)xmlCharStrdup("");
        char *status_copy = status ? (char *)xmlCharStrdup(status) : (char *)xmlCharStrdup("*");

        mc_config_http_header_t hdr = {
            .flags = PARAM_NOCOPY,
            .hdr = { .name = name_copy, .value = value_copy, .status = status_copy }
        };

        if (_add_http_header(headers, &hdr, 1) < 0) {
            xmlFree(name_copy);
            xmlFree(value_copy);
            xmlFree(status_copy);
        }
    }

    return 0;
}

/* ============================================================================
 * Logging Section Parsers
 * ============================================================================ */

int yaml_parse_accesslog(yaml_parse_ctx *ctx, yaml_node_t *node, struct access_log *log)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    log->logid = -1;
    log->type = LOG_ACCESS_CLF;
    log->qstr = 1;
    log->archive = -1;
    log->log_ip = 1;

    yaml_node_t *name = yaml_get_mapping_value(ctx->document, node, "name");
    yaml_node_t *ip = yaml_get_mapping_value(ctx->document, node, "ip");
    yaml_node_t *type = yaml_get_mapping_value(ctx->document, node, "type");
    yaml_node_t *archive = yaml_get_mapping_value(ctx->document, node, "archive");
    yaml_node_t *exclude_ext = yaml_get_mapping_value(ctx->document, node, "exclude-ext");
    yaml_node_t *display = yaml_get_mapping_value(ctx->document, node, "display");
    yaml_node_t *querystr = yaml_get_mapping_value(ctx->document, node, "querystr");
    yaml_node_t *size = yaml_get_mapping_value(ctx->document, node, "size");
    yaml_node_t *duration = yaml_get_mapping_value(ctx->document, node, "duration");

    if (name) {
        const char *val = yaml_get_scalar_value(ctx->document, name);
        if (val) log->name = (char *)xmlCharStrdup(val);
    }
    if (ip) yaml_get_bool_value(ctx->document, ip, &log->log_ip);
    if (archive) yaml_get_bool_value(ctx->document, archive, &log->archive);
    if (display) yaml_get_int_value(ctx->document, display, &log->display);
    if (querystr) yaml_get_bool_value(ctx->document, querystr, &log->qstr);
    if (size) yaml_get_long_value(ctx->document, size, &log->size);
    if (duration) yaml_get_int_value(ctx->document, duration, (int *)&log->duration);

    if (type) {
        const char *type_str = yaml_get_scalar_value(ctx->document, type);
        if (type_str && strcmp(type_str, "CLF-ESC") == 0)
            log->type = LOG_ACCESS_CLF_ESC;
    }

    if (exclude_ext) {
        const char *val = yaml_get_scalar_value(ctx->document, exclude_ext);
        if (val) log->exclude_ext = (char *)xmlCharStrdup(val);
    }

    return 0;
}

int yaml_parse_errorlog(yaml_parse_ctx *ctx, yaml_node_t *node, error_log *log)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    log->logid = -1;
    log->archive = -1;
    logging_init_levels(&log->level, 0);

    yaml_node_t *name = yaml_get_mapping_value(ctx->document, node, "name");
    yaml_node_t *archive = yaml_get_mapping_value(ctx->document, node, "archive");
    yaml_node_t *display = yaml_get_mapping_value(ctx->document, node, "display");
    yaml_node_t *level = yaml_get_mapping_value(ctx->document, node, "level");
    yaml_node_t *size = yaml_get_mapping_value(ctx->document, node, "size");
    yaml_node_t *duration = yaml_get_mapping_value(ctx->document, node, "duration");

    if (name) {
        const char *val = yaml_get_scalar_value(ctx->document, name);
        if (val) log->name = (char *)xmlCharStrdup(val);
    }
    if (archive) yaml_get_bool_value(ctx->document, archive, &log->archive);
    if (display) yaml_get_int_value(ctx->document, display, &log->display);
    if (level) yaml_get_loglevel_value(ctx->document, level, &log->level);
    if (size) yaml_get_long_value(ctx->document, size, &log->size);
    if (duration) yaml_get_int_value(ctx->document, duration, (int *)&log->duration);

    return 0;
}

int yaml_parse_playlistlog(yaml_parse_ctx *ctx, yaml_node_t *node, playlist_log *log)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    log->logid = -1;
    log->archive = -1;

    yaml_node_t *name = yaml_get_mapping_value(ctx->document, node, "name");
    yaml_node_t *archive = yaml_get_mapping_value(ctx->document, node, "archive");
    yaml_node_t *display = yaml_get_mapping_value(ctx->document, node, "display");
    yaml_node_t *size = yaml_get_mapping_value(ctx->document, node, "size");
    yaml_node_t *duration = yaml_get_mapping_value(ctx->document, node, "duration");

    if (name) {
        const char *val = yaml_get_scalar_value(ctx->document, name);
        if (val) log->name = (char *)xmlCharStrdup(val);
    }
    if (archive) yaml_get_bool_value(ctx->document, archive, &log->archive);
    if (display) yaml_get_int_value(ctx->document, display, &log->display);
    if (size) yaml_get_long_value(ctx->document, size, &log->size);
    if (duration) yaml_get_int_value(ctx->document, duration, (int *)&log->duration);

    return 0;
}

int yaml_parse_logging(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    mc_config_t *config = ctx->config;
    long old_trigger_size = -1;
    int old_archive = 1;

    /* Initialize log defaults */
    config->preroll_log.logid = -1;
    config->preroll_log.display = 50;
    config->preroll_log.archive = -1;
    config->access_log.type = LOG_ACCESS_CLF;
    config->access_log.logid = -1;
    config->access_log.display = 100;
    config->access_log.archive = -1;
    config->error_log.logid = -1;
    config->error_log.archive = -1;
    config->playlist_log.logid = -1;
    config->playlist_log.display = 10;
    config->playlist_log.archive = -1;

    yaml_node_t *preroll = yaml_get_mapping_value(ctx->document, node, "preroll-log");
    yaml_node_t *accesslog = yaml_get_mapping_value(ctx->document, node, "accesslog");
    yaml_node_t *errorlog = yaml_get_mapping_value(ctx->document, node, "errorlog");
    yaml_node_t *playlistlog = yaml_get_mapping_value(ctx->document, node, "playlistlog");
    yaml_node_t *loglevel = yaml_get_mapping_value(ctx->document, node, "loglevel");
    yaml_node_t *logsize = yaml_get_mapping_value(ctx->document, node, "logsize");
    yaml_node_t *logarchive = yaml_get_mapping_value(ctx->document, node, "logarchive");

    if (preroll) yaml_parse_errorlog(ctx, preroll, &config->preroll_log);
    if (accesslog) yaml_parse_accesslog(ctx, accesslog, &config->access_log);
    if (errorlog) yaml_parse_errorlog(ctx, errorlog, &config->error_log);
    if (playlistlog) yaml_parse_playlistlog(ctx, playlistlog, &config->playlist_log);
    if (loglevel) yaml_get_loglevel_value(ctx->document, loglevel, &config->error_log.level);
    if (logsize) yaml_get_long_value(ctx->document, logsize, &old_trigger_size);
    if (logarchive) yaml_get_bool_value(ctx->document, logarchive, &old_archive);

    /* Apply old-style settings if present */
    if (old_trigger_size < 0)
        old_trigger_size = 20000;
    if (old_trigger_size > 2000000)
        old_trigger_size = 2000000;
    old_trigger_size <<= 10;

    if (config->preroll_log.size == 0) config->preroll_log.size = old_trigger_size;
    if (config->error_log.size == 0) config->error_log.size = old_trigger_size;
    if (config->access_log.size == 0) config->access_log.size = old_trigger_size;
    if (config->playlist_log.size == 0) config->playlist_log.size = old_trigger_size;

    if (config->preroll_log.archive == -1) config->preroll_log.archive = old_archive;
    if (config->error_log.archive == -1) config->error_log.archive = old_archive;
    if (config->access_log.archive == -1) config->access_log.archive = old_archive;
    if (config->playlist_log.archive == -1) config->playlist_log.archive = old_archive;

    return 0;
}

/* ============================================================================
 * Authentication Parsers
 * ============================================================================ */

int yaml_parse_authentication(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    mc_config_t *config = ctx->config;

    yaml_node_t *source_pw = yaml_get_mapping_value(ctx->document, node, "source-password");
    yaml_node_t *admin_user = yaml_get_mapping_value(ctx->document, node, "admin-user");
    yaml_node_t *admin_pw = yaml_get_mapping_value(ctx->document, node, "admin-password");
    yaml_node_t *relay_user = yaml_get_mapping_value(ctx->document, node, "relay-user");
    yaml_node_t *relay_pw = yaml_get_mapping_value(ctx->document, node, "relay-password");

    if (source_pw) {
        const char *val = yaml_get_scalar_value(ctx->document, source_pw);
        if (val) {
            if (config->source_password) xmlFree(config->source_password);
            config->source_password = (char *)xmlCharStrdup(val);
        }
    }

    if (admin_user) {
        const char *val = yaml_get_scalar_value(ctx->document, admin_user);
        if (val) {
            if (config->admin_username) xmlFree(config->admin_username);
            config->admin_username = (char *)xmlCharStrdup(val);
        }
    }

    if (admin_pw) {
        const char *val = yaml_get_scalar_value(ctx->document, admin_pw);
        if (val) {
            if (config->admin_password) xmlFree(config->admin_password);
            config->admin_password = (char *)xmlCharStrdup(val);
        }
    }

    if (relay_user) {
        const char *val = yaml_get_scalar_value(ctx->document, relay_user);
        if (val) {
            if (config->relay_username) xmlFree(config->relay_username);
            config->relay_username = (char *)xmlCharStrdup(val);
        }
    }

    if (relay_pw) {
        const char *val = yaml_get_scalar_value(ctx->document, relay_pw);
        if (val) {
            if (config->relay_password) xmlFree(config->relay_password);
            config->relay_password = (char *)xmlCharStrdup(val);
        }
    }

    return 0;
}

static config_options_t *yaml_parse_mount_auth_options(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_SEQUENCE_NODE)
        return NULL;

    config_options_t *head = NULL;
    yaml_node_item_t *item;

    for (item = node->data.sequence.items.start;
         item < node->data.sequence.items.top; item++)
    {
        yaml_node_t *opt_node = yaml_document_get_node(ctx->document, *item);
        if (!opt_node || opt_node->type != YAML_MAPPING_NODE)
            continue;

        yaml_node_t *name = yaml_get_mapping_value(ctx->document, opt_node, "name");
        yaml_node_t *value = yaml_get_mapping_value(ctx->document, opt_node, "value");

        const char *name_str = yaml_get_scalar_value(ctx->document, name);
        const char *value_str = yaml_get_scalar_value(ctx->document, value);

        if (!name_str || !value_str)
            continue;

        config_options_t *opt = calloc(1, sizeof(*opt));
        opt->name = (char *)xmlCharStrdup(name_str);
        opt->value = (char *)xmlCharStrdup(value_str);
        opt->next = head;
        head = opt;
    }

    return head;
}

int yaml_parse_mount_auth(yaml_parse_ctx *ctx, yaml_node_t *node, auth_t **auth_out)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    auth_t *auth = calloc(1, sizeof(auth_t));
    config_options_t *opt = NULL;
    int ret = -1;

    yaml_node_t *type = yaml_get_mapping_value(ctx->document, node, "type");
    yaml_node_t *options = yaml_get_mapping_value(ctx->document, node, "options");

    if (type) {
        const char *type_str = yaml_get_scalar_value(ctx->document, type);
        if (type_str)
            auth->type = (char *)xmlCharStrdup(type_str);
    }

    if (options)
        opt = yaml_parse_mount_auth_options(ctx, options);

    auth->handlers = 5;
    if (auth_get_authenticator(auth, opt) >= 0) {
        *auth_out = auth;
        ret = 0;
    } else {
        thread_mutex_lock(&auth->lock);
        auth_release(auth);
    }

    while (opt)
        opt = config_clear_option(opt);

    return ret;
}

/* ============================================================================
 * Limits, Paths, Security, etc.
 * ============================================================================ */

int yaml_parse_limits(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    mc_config_t *config = ctx->config;

    yaml_node_t *max_bandwidth = yaml_get_mapping_value(ctx->document, node, "max-bandwidth");
    yaml_node_t *max_listeners = yaml_get_mapping_value(ctx->document, node, "max-listeners");
    yaml_node_t *clients = yaml_get_mapping_value(ctx->document, node, "clients");
    yaml_node_t *sources = yaml_get_mapping_value(ctx->document, node, "sources");
    yaml_node_t *queue_size = yaml_get_mapping_value(ctx->document, node, "queue-size");
    yaml_node_t *min_queue = yaml_get_mapping_value(ctx->document, node, "min-queue-size");
    yaml_node_t *burst_size = yaml_get_mapping_value(ctx->document, node, "burst-size");
    yaml_node_t *workers = yaml_get_mapping_value(ctx->document, node, "workers");
    yaml_node_t *client_timeout = yaml_get_mapping_value(ctx->document, node, "client-timeout");
    yaml_node_t *header_timeout = yaml_get_mapping_value(ctx->document, node, "header-timeout");
    yaml_node_t *source_timeout = yaml_get_mapping_value(ctx->document, node, "source-timeout");
    yaml_node_t *inactivity = yaml_get_mapping_value(ctx->document, node, "inactivity-timeout");
    yaml_node_t *song_hist  = yaml_get_mapping_value(ctx->document, node, "song-history-limit");

    if (max_bandwidth) yaml_get_bitrate_value(ctx->document, max_bandwidth, &config->max_bandwidth);
    if (max_listeners) yaml_get_int_value(ctx->document, max_listeners, &config->max_listeners);
    if (clients) yaml_get_int_value(ctx->document, clients, &config->client_limit);
    if (sources) yaml_get_int_value(ctx->document, sources, &config->source_limit);
    if (queue_size) yaml_get_qsizing_value(ctx->document, queue_size, &config->queue_size_limit);
    if (min_queue) yaml_get_qsizing_value(ctx->document, min_queue, (uint32_t *)&config->min_queue_size);
    if (burst_size) yaml_get_qsizing_value(ctx->document, burst_size, &config->burst_size);
    if (workers) yaml_get_int_value(ctx->document, workers, &config->workers_count);
    if (client_timeout) yaml_get_int_value(ctx->document, client_timeout, &config->client_timeout);
    if (header_timeout) yaml_get_int_value(ctx->document, header_timeout, &config->header_timeout);
    if (source_timeout) yaml_get_int_value(ctx->document, source_timeout, &config->source_timeout);
    if (inactivity) yaml_get_int_value(ctx->document, inactivity, &config->inactivity_timeout);
    if (song_hist)  yaml_get_int_value(ctx->document, song_hist,  &config->song_history_limit);

    if (config->workers_count < 1) config->workers_count = 1;
    if (config->workers_count > 400) config->workers_count = 400;

    return 0;
}

int yaml_parse_alias(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    aliases *alias = calloc(1, sizeof(aliases));
    alias->port = -1;

    yaml_node_t *source = yaml_get_mapping_value(ctx->document, node, "source");
    yaml_node_t *dest = yaml_get_mapping_value(ctx->document, node, "destination");
    yaml_node_t *bind = yaml_get_mapping_value(ctx->document, node, "bind-address");
    yaml_node_t *port = yaml_get_mapping_value(ctx->document, node, "port");

    if (source) {
        const char *val = yaml_get_scalar_value(ctx->document, source);
        if (val) alias->source = (char *)xmlCharStrdup(val);
    }

    if (dest) {
        const char *val = yaml_get_scalar_value(ctx->document, dest);
        if (val) alias->destination = (char *)xmlCharStrdup(val);
    }

    if (bind) {
        const char *val = yaml_get_scalar_value(ctx->document, bind);
        if (val) alias->bind_address = (char *)xmlCharStrdup(val);
    }

    if (port) yaml_get_port_value(ctx->document, port, &alias->port);

    if (!alias->source || !alias->destination) {
        yaml_parse_warning(ctx, node, "Incomplete alias definition");
        config_clear_alias(alias);
        return -1;
    }

    /* Add to config list */
    aliases **cur = &ctx->config->aliases;
    while (*cur) cur = &((*cur)->next);
    *cur = alias;

    return 0;
}

int yaml_parse_paths(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    mc_config_t *config = ctx->config;

    yaml_node_t *basedir = yaml_get_mapping_value(ctx->document, node, "basedir");
    yaml_node_t *logdir = yaml_get_mapping_value(ctx->document, node, "logdir");
    yaml_node_t *webroot = yaml_get_mapping_value(ctx->document, node, "webroot");
    yaml_node_t *adminroot = yaml_get_mapping_value(ctx->document, node, "adminroot");
    yaml_node_t *pidfile = yaml_get_mapping_value(ctx->document, node, "pidfile");
    yaml_node_t *banfile = yaml_get_mapping_value(ctx->document, node, "banfile");
    yaml_node_t *ban_file = yaml_get_mapping_value(ctx->document, node, "ban-file");
    yaml_node_t *allow_ip = yaml_get_mapping_value(ctx->document, node, "allow-ip");
    yaml_node_t *deny_agents = yaml_get_mapping_value(ctx->document, node, "deny-agents");
    yaml_node_t *ssl_cert = yaml_get_mapping_value(ctx->document, node, "ssl-certificate");
    yaml_node_t *ssl_key = yaml_get_mapping_value(ctx->document, node, "ssl-private-key");
    yaml_node_t *ssl_cafile = yaml_get_mapping_value(ctx->document, node, "ssl-cafile");
    yaml_node_t *ssl_ciphers = yaml_get_mapping_value(ctx->document, node, "ssl-allowed-ciphers");
    yaml_node_t *mime_types = yaml_get_mapping_value(ctx->document, node, "mime-types");
    yaml_node_t *aliases = yaml_get_mapping_value(ctx->document, node, "aliases");

    #define SET_STR_CONFIG(yaml_node, config_field) \
        if (yaml_node) { \
            const char *val = yaml_get_scalar_value(ctx->document, yaml_node); \
            if (val) { \
                if (config->config_field) xmlFree(config->config_field); \
                config->config_field = (char *)xmlCharStrdup(val); \
            } \
        }

    SET_STR_CONFIG(basedir, base_dir);
    SET_STR_CONFIG(logdir, log_dir);
    SET_STR_CONFIG(webroot, webroot_dir);
    SET_STR_CONFIG(adminroot, adminroot_dir);
    SET_STR_CONFIG(pidfile, pidfile);
    SET_STR_CONFIG(banfile, banfile);
    SET_STR_CONFIG(ban_file, banfile);
    SET_STR_CONFIG(allow_ip, allowfile);
    SET_STR_CONFIG(deny_agents, agentfile);
    SET_STR_CONFIG(ssl_cert, cert_file);
    SET_STR_CONFIG(ssl_key, key_file);
    SET_STR_CONFIG(ssl_cafile, ca_file);
    SET_STR_CONFIG(ssl_ciphers, cipher_list);
    SET_STR_CONFIG(mime_types, mimetypes_fn);

    #undef SET_STR_CONFIG

    if (config->cert_file && !config->key_file)
        config->key_file = strdup(config->cert_file);

    if (aliases && aliases->type == YAML_SEQUENCE_NODE) {
        yaml_node_item_t *item;
        for (item = aliases->data.sequence.items.start;
             item < aliases->data.sequence.items.top; item++)
        {
            yaml_node_t *alias_node = yaml_document_get_node(ctx->document, *item);
            yaml_parse_alias(ctx, alias_node);
        }
    }

    return 0;
}

static int yaml_parse_chown(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    mc_config_t *config = ctx->config;

    yaml_node_t *user = yaml_get_mapping_value(ctx->document, node, "user");
    yaml_node_t *group = yaml_get_mapping_value(ctx->document, node, "group");

    if (user) {
        const char *val = yaml_get_scalar_value(ctx->document, user);
        if (val) {
            if (config->user) xmlFree(config->user);
            config->user = (char *)xmlCharStrdup(val);
        }
    }

    if (group) {
        const char *val = yaml_get_scalar_value(ctx->document, group);
        if (val) {
            if (config->group) xmlFree(config->group);
            config->group = (char *)xmlCharStrdup(val);
        }
    }

    config->chuid = 1;
    return 0;
}

int yaml_parse_security(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    mc_config_t *config = ctx->config;

    yaml_node_t *chroot = yaml_get_mapping_value(ctx->document, node, "chroot");
    yaml_node_t *changeowner = yaml_get_mapping_value(ctx->document, node, "changeowner");

    if (chroot) yaml_get_bool_value(ctx->document, chroot, &config->chroot);
    if (changeowner) yaml_parse_chown(ctx, changeowner);

    return 0;
}

int yaml_parse_directory(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    mc_config_t *config = ctx->config;

    if (config->num_yp_directories >= MAX_YP_DIRECTORIES) {
        yaml_parse_error(ctx, node, "Maximum number of YP directories exceeded");
        return -1;
    }

    int idx = config->num_yp_directories;
    config->yp_url_timeout[idx] = 10;
    config->yp_touch_interval[idx] = 600;

    yaml_node_t *yp_url = yaml_get_mapping_value(ctx->document, node, "yp-url");
    yaml_node_t *timeout = yaml_get_mapping_value(ctx->document, node, "yp-url-timeout");
    yaml_node_t *touch = yaml_get_mapping_value(ctx->document, node, "touch-interval");
    yaml_node_t *logfile = yaml_get_mapping_value(ctx->document, node, "yp-logfile");

    if (yp_url) {
        const char *val = yaml_get_scalar_value(ctx->document, yp_url);
        if (val) config->yp_url[idx] = (char *)xmlCharStrdup(val);
    }

    if (timeout) yaml_get_int_value(ctx->document, timeout, &config->yp_url_timeout[idx]);
    if (touch) yaml_get_int_value(ctx->document, touch, &config->yp_touch_interval[idx]);

    if (logfile) {
        const char *val = yaml_get_scalar_value(ctx->document, logfile);
        if (val) config->yp_logfile[idx] = (char *)xmlCharStrdup(val);
    }

    if (!config->yp_url[idx])
        return -1;

    config->num_yp_directories++;
    return 0;
}

int yaml_parse_master(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    mc_config_t *config = ctx->config;

    yaml_node_t *server = yaml_get_mapping_value(ctx->document, node, "server");
    yaml_node_t *port = yaml_get_mapping_value(ctx->document, node, "port");
    yaml_node_t *ssl_port = yaml_get_mapping_value(ctx->document, node, "ssl-port");
    yaml_node_t *username = yaml_get_mapping_value(ctx->document, node, "username");
    yaml_node_t *password = yaml_get_mapping_value(ctx->document, node, "password");
    yaml_node_t *bind = yaml_get_mapping_value(ctx->document, node, "bind");
    yaml_node_t *interval = yaml_get_mapping_value(ctx->document, node, "interval");
    yaml_node_t *relay_auth = yaml_get_mapping_value(ctx->document, node, "relay-auth");
    yaml_node_t *retry_delay = yaml_get_mapping_value(ctx->document, node, "retry-delay");
    yaml_node_t *redirect = yaml_get_mapping_value(ctx->document, node, "redirect");
    yaml_node_t *run_on = yaml_get_mapping_value(ctx->document, node, "run-on");
    yaml_node_t *on_demand = yaml_get_mapping_value(ctx->document, node, "on-demand");

    #define SET_STR_CONFIG(yaml_node, config_field) \
        if (yaml_node) { \
            const char *val = yaml_get_scalar_value(ctx->document, yaml_node); \
            if (val) { \
                if (config->config_field) xmlFree(config->config_field); \
                config->config_field = (char *)xmlCharStrdup(val); \
            } \
        }

    SET_STR_CONFIG(server, master_server);
    SET_STR_CONFIG(username, master_username);
    SET_STR_CONFIG(password, master_password);
    SET_STR_CONFIG(bind, master_bind);

    #undef SET_STR_CONFIG

    if (port) yaml_get_port_value(ctx->document, port, &config->master_server_port);
    if (ssl_port) yaml_get_int_value(ctx->document, ssl_port, &config->master_ssl_port);
    if (interval) yaml_get_int_value(ctx->document, interval, &config->master_update_interval);
    if (relay_auth) yaml_get_bool_value(ctx->document, relay_auth, &config->master_relay_auth);
    if (retry_delay) yaml_get_int_value(ctx->document, retry_delay, &config->master_relay_retry);
    if (redirect) yaml_get_bool_value(ctx->document, redirect, &config->master_redirect);
    if (run_on) yaml_get_int_value(ctx->document, run_on, &config->master_run_on);
    if (on_demand) yaml_get_bool_value(ctx->document, on_demand, &config->on_demand);

    if (config->master_update_interval < 2)
        config->master_update_interval = 60;
    if (config->master_relay_retry < 1)
        config->master_relay_retry = 60;

    return 0;
}

int yaml_parse_redirect(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    redirect_host *redir = calloc(1, sizeof(*redir));
    redir->port = 8000;

    yaml_node_t *host = yaml_get_mapping_value(ctx->document, node, "host");
    yaml_node_t *port = yaml_get_mapping_value(ctx->document, node, "port");

    if (host) {
        const char *val = yaml_get_scalar_value(ctx->document, host);
        if (val) redir->server = (char *)xmlCharStrdup(val);
    }

    if (port) yaml_get_port_value(ctx->document, port, &redir->port);

    if (!redir->server) {
        free(redir);
        return -1;
    }

    redir->next = ctx->config->redirect_hosts;
    ctx->config->redirect_hosts = redir;

    return 0;
}

/* ============================================================================
 * Listen Socket Parsers
 * ============================================================================ */

int yaml_parse_listen_socket(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    mc_config_t *config = ctx->config;
    listener_t *listener = calloc(1, sizeof(listener_t));
    listener->refcount = 1;
    listener->qlen = MC_LISTEN_QUEUE;

    yaml_node_t *port = yaml_get_mapping_value(ctx->document, node, "port");
    yaml_node_t *bind = yaml_get_mapping_value(ctx->document, node, "bind-address");
    yaml_node_t *shoutcast_compat = yaml_get_mapping_value(ctx->document, node, "shoutcast-compat");
    yaml_node_t *shoutcast_mount = yaml_get_mapping_value(ctx->document, node, "shoutcast-mount");
    yaml_node_t *queue_len = yaml_get_mapping_value(ctx->document, node, "queue-len");
    yaml_node_t *so_sndbuf = yaml_get_mapping_value(ctx->document, node, "so-sndbuf");
#ifndef _WIN32
    yaml_node_t *so_mss = yaml_get_mapping_value(ctx->document, node, "so-mss");
#endif

    if (port) {
        if (yaml_get_port_value(ctx->document, port, &listener->port) < 0) {
            config_clear_listener(listener);
            return -1;
        }
    } else {
        config_clear_listener(listener);
        return -1;
    }

    if (bind) {
        const char *val = yaml_get_scalar_value(ctx->document, bind);
        if (val) listener->bind_address = (char *)xmlCharStrdup(val);
    }

    if (shoutcast_compat) yaml_get_bool_value(ctx->document, shoutcast_compat, &listener->shoutcast_compat);

    if (shoutcast_mount) {
        const char *val = yaml_get_scalar_value(ctx->document, shoutcast_mount);
        if (val) listener->shoutcast_mount = (char *)xmlCharStrdup(val);
    }

    if (queue_len) yaml_get_int_value(ctx->document, queue_len, &listener->qlen);
    if (so_sndbuf) yaml_get_int_value(ctx->document, so_sndbuf, &listener->so_sndbuf);
#ifndef _WIN32
    if (so_mss) yaml_get_int_value(ctx->document, so_mss, &listener->so_mss);
#endif

    if (listener->qlen < 1)
        listener->qlen = MC_LISTEN_QUEUE;

    /* Handle shoutcast-mount special case */
    if (listener->shoutcast_mount) {
        if (listener->shoutcast_compat) {
            WARN1("shoutcast-compat and shoutcast-mount both set on port %d, ignoring", listener->port);
            config_clear_listener(listener);
            return -1;
        }

        if (config->shoutcast_mount == NULL)
            config->shoutcast_mount = (char *)xmlStrdup(XMLSTR(listener->shoutcast_mount));

        listener_t *sc_port = calloc(1, sizeof(listener_t));
        sc_port->refcount = 1;
        sc_port->port = listener->port + 1;
        sc_port->qlen = listener->qlen;
        sc_port->shoutcast_compat = 1;
        sc_port->shoutcast_mount = (char *)xmlStrdup(XMLSTR(listener->shoutcast_mount));
        if (listener->bind_address)
            sc_port->bind_address = (char *)xmlStrdup(XMLSTR(listener->bind_address));

        sc_port->next = config->listen_sock;
        config->listen_sock = sc_port;
        config->listen_sock_count++;
    }

    listener->next = config->listen_sock;
    config->listen_sock = listener;
    config->listen_sock_count++;

    if (config->port == 0)
        config->port = listener->port;

    return 0;
}

int yaml_parse_listen_sockets(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_SEQUENCE_NODE)
        return -1;

    yaml_node_item_t *item;
    for (item = node->data.sequence.items.start;
         item < node->data.sequence.items.top; item++)
    {
        yaml_node_t *sock_node = yaml_document_get_node(ctx->document, *item);
        yaml_parse_listen_socket(ctx, sock_node);
    }

    return 0;
}

/* ============================================================================
 * Fallback Parser
 * ============================================================================ */

int yaml_parse_fallback(yaml_parse_ctx *ctx, yaml_node_t *node, fbinfo *fb)
{
    if (!node)
        return -1;

    /* Handle simple string fallback */
    if (node->type == YAML_SCALAR_NODE) {
        const char *mount = yaml_get_scalar_value(ctx->document, node);
        if (mount) {
            fb->mount = (char *)xmlCharStrdup(mount);
            fb->limit = 0;
        }
        return 0;
    }

    /* Handle mapping with mount and rate */
    if (node->type == YAML_MAPPING_NODE) {
        yaml_node_t *mount = yaml_get_mapping_value(ctx->document, node, "mount");
        yaml_node_t *rate = yaml_get_mapping_value(ctx->document, node, "rate");

        if (mount) {
            const char *val = yaml_get_scalar_value(ctx->document, mount);
            if (val) fb->mount = (char *)xmlCharStrdup(val);
        }

        if (rate) {
            int64_t limit = 0;
            yaml_get_bitrate_value(ctx->document, rate, &limit);
            fb->limit = limit / 8;  /* Convert bits to bytes */
        }

        if (!fb->mount) {
            yaml_parse_warning(ctx, node, "Incomplete fallback specification");
            return -1;
        }

        return 0;
    }

    return -1;
}

/* ============================================================================
 * Mount Point Parser
 * ============================================================================ */

int yaml_parse_mount(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    mc_config_t *config = ctx->config;
    mount_proxy *mount = calloc(1, sizeof(mount_proxy));

    /* Set defaults from global config */
    mount->_refcount = 1;
    mount->priority = INT_MAX;
    mount->max_listeners = -1;
    mount->max_bandwidth = -1;
    mount->burst_size = config->burst_size;
    mount->queue_size_limit = config->queue_size_limit;
    mount->min_queue_size = config->min_queue_size;
    mount->mp3_meta_interval = -1;
    mount->yp_public = -1;
    mount->url_ogg_meta = 1;
    mount->source_timeout = config->source_timeout;
    mount->file_seekable = 1;
    mount->access_log.type = LOG_ACCESS_CLF;
    mount->access_log.logid = -1;
    mount->access_log.log_ip = 1;
    mount->fallback_override = 1;
    mount->max_send_size = 0;
    mount->preroll_log.logid = -1;
    mount->preroll_log.display = 50;
    mount->preroll_log.archive = -1;
    config_http_copy(config->http_headers, &mount->http_headers);

    /* Parse all mount fields */
    yaml_node_t *mount_name = yaml_get_mapping_value(ctx->document, node, "mount-name");
    yaml_node_t *priority = yaml_get_mapping_value(ctx->document, node, "priority");
    yaml_node_t *source_timeout = yaml_get_mapping_value(ctx->document, node, "source-timeout");
    yaml_node_t *queue_size = yaml_get_mapping_value(ctx->document, node, "queue-size");
    yaml_node_t *burst_size = yaml_get_mapping_value(ctx->document, node, "burst-size");
    yaml_node_t *min_queue = yaml_get_mapping_value(ctx->document, node, "min-queue-size");
    yaml_node_t *username = yaml_get_mapping_value(ctx->document, node, "username");
    yaml_node_t *password = yaml_get_mapping_value(ctx->document, node, "password");
    yaml_node_t *dump_file = yaml_get_mapping_value(ctx->document, node, "dump-file");
    yaml_node_t *intro = yaml_get_mapping_value(ctx->document, node, "intro");
    yaml_node_t *file_seekable = yaml_get_mapping_value(ctx->document, node, "file-seekable");
    yaml_node_t *fallback = yaml_get_mapping_value(ctx->document, node, "fallback");
    yaml_node_t *fallback_override = yaml_get_mapping_value(ctx->document, node, "fallback-override");
    yaml_node_t *fallback_when_full = yaml_get_mapping_value(ctx->document, node, "fallback-when-full");
    yaml_node_t *hijack = yaml_get_mapping_value(ctx->document, node, "hijack");
    yaml_node_t *allow_chunked = yaml_get_mapping_value(ctx->document, node, "allow-chunked");
    yaml_node_t *max_listeners = yaml_get_mapping_value(ctx->document, node, "max-listeners");
    yaml_node_t *max_bandwidth = yaml_get_mapping_value(ctx->document, node, "max-bandwidth");
    yaml_node_t *wait_time = yaml_get_mapping_value(ctx->document, node, "wait-time");
    yaml_node_t *filter_theora = yaml_get_mapping_value(ctx->document, node, "filter-theora");
    yaml_node_t *limit_rate = yaml_get_mapping_value(ctx->document, node, "limit-rate");
    yaml_node_t *skip_accesslog = yaml_get_mapping_value(ctx->document, node, "skip-accesslog");
    yaml_node_t *charset = yaml_get_mapping_value(ctx->document, node, "charset");
    yaml_node_t *max_send_size = yaml_get_mapping_value(ctx->document, node, "max-send-size");
    yaml_node_t *linger_for = yaml_get_mapping_value(ctx->document, node, "linger-for");
    yaml_node_t *redirect = yaml_get_mapping_value(ctx->document, node, "redirect");
    yaml_node_t *redirect_to = yaml_get_mapping_value(ctx->document, node, "redirect-to");
    yaml_node_t *http_headers = yaml_get_mapping_value(ctx->document, node, "http-headers");
    yaml_node_t *metadata_interval = yaml_get_mapping_value(ctx->document, node, "metadata-interval");
    yaml_node_t *ogg_passthrough = yaml_get_mapping_value(ctx->document, node, "ogg-passthrough");
    yaml_node_t *admin_comments_only = yaml_get_mapping_value(ctx->document, node, "admin-comments-only");
    yaml_node_t *url_ogg_meta = yaml_get_mapping_value(ctx->document, node, "allow-url-ogg-metadata");
    yaml_node_t *no_mount = yaml_get_mapping_value(ctx->document, node, "no-mount");
    yaml_node_t *ban_client = yaml_get_mapping_value(ctx->document, node, "ban-client");
    yaml_node_t *intro_skip_replay = yaml_get_mapping_value(ctx->document, node, "intro-skip-replay");
    yaml_node_t *so_sndbuf = yaml_get_mapping_value(ctx->document, node, "so-sndbuf");
    yaml_node_t *hidden = yaml_get_mapping_value(ctx->document, node, "hidden");
    yaml_node_t *authentication = yaml_get_mapping_value(ctx->document, node, "authentication");
    yaml_node_t *on_connect = yaml_get_mapping_value(ctx->document, node, "on-connect");
    yaml_node_t *on_disconnect = yaml_get_mapping_value(ctx->document, node, "on-disconnect");
    yaml_node_t *max_stream_duration = yaml_get_mapping_value(ctx->document, node, "max-stream-duration");
    yaml_node_t *max_listener_duration = yaml_get_mapping_value(ctx->document, node, "max-listener-duration");
    yaml_node_t *preroll_log = yaml_get_mapping_value(ctx->document, node, "preroll-log");
    yaml_node_t *accesslog = yaml_get_mapping_value(ctx->document, node, "accesslog");
    yaml_node_t *listenurl = yaml_get_mapping_value(ctx->document, node, "listenurl");
    yaml_node_t *cluster_password = yaml_get_mapping_value(ctx->document, node, "cluster-password");
    yaml_node_t *stream_name = yaml_get_mapping_value(ctx->document, node, "stream-name");
    yaml_node_t *stream_description = yaml_get_mapping_value(ctx->document, node, "stream-description");
    yaml_node_t *stream_url = yaml_get_mapping_value(ctx->document, node, "stream-url");
    yaml_node_t *genre = yaml_get_mapping_value(ctx->document, node, "genre");
    yaml_node_t *bitrate = yaml_get_mapping_value(ctx->document, node, "bitrate");
    yaml_node_t *public_yp = yaml_get_mapping_value(ctx->document, node, "public");
    yaml_node_t *type = yaml_get_mapping_value(ctx->document, node, "type");
    yaml_node_t *subtype = yaml_get_mapping_value(ctx->document, node, "subtype");

    /* Set mount name (required) */
    if (mount_name) {
        const char *val = yaml_get_scalar_value(ctx->document, mount_name);
        if (val) mount->mountname = (char *)xmlCharStrdup(val);
    }

    if (!mount->mountname) {
        config_clear_mount(mount, 0);
        return -1;
    }

    /* Set all other fields */
    #define SET_STR(node, field) \
        if (node) { \
            const char *val = yaml_get_scalar_value(ctx->document, node); \
            if (val) mount->field = (char *)xmlCharStrdup(val); \
        }

    SET_STR(username, username);
    SET_STR(password, password);
    SET_STR(dump_file, dumpfile);
    SET_STR(intro, intro_filename);
    SET_STR(charset, charset);
    SET_STR(redirect_to, redirect);
    SET_STR(on_connect, on_connect);
    SET_STR(on_disconnect, on_disconnect);
    SET_STR(listenurl, listenurl);
    SET_STR(cluster_password, cluster_password);
    SET_STR(stream_name, stream_name);
    SET_STR(stream_description, stream_description);
    SET_STR(stream_url, stream_url);
    SET_STR(genre, stream_genre);
    SET_STR(bitrate, bitrate);
    SET_STR(type, type);
    SET_STR(subtype, subtype);

    #undef SET_STR

    if (priority) yaml_get_int_value(ctx->document, priority, &mount->priority);
    if (source_timeout) yaml_get_int_value(ctx->document, source_timeout, (int *)&mount->source_timeout);
    if (queue_size) yaml_get_qsizing_value(ctx->document, queue_size, &mount->queue_size_limit);
    if (burst_size) yaml_get_qsizing_value(ctx->document, burst_size, &mount->burst_size);
    if (min_queue) yaml_get_qsizing_value(ctx->document, min_queue, &mount->min_queue_size);
    if (file_seekable) yaml_get_bool_value(ctx->document, file_seekable, &mount->file_seekable);
    if (fallback) yaml_parse_fallback(ctx, fallback, &mount->fallback);
    if (fallback_override) yaml_get_bool_value(ctx->document, fallback_override, &mount->fallback_override);
    if (fallback_when_full) yaml_get_bool_value(ctx->document, fallback_when_full, &mount->fallback_when_full);
    if (hijack) yaml_get_bool_value(ctx->document, hijack, &mount->hijack);
    if (allow_chunked) yaml_get_bool_value(ctx->document, allow_chunked, &mount->allow_chunked);
    if (max_listeners) yaml_get_int_value(ctx->document, max_listeners, &mount->max_listeners);
    if (max_bandwidth) yaml_get_bitrate_value(ctx->document, max_bandwidth, &mount->max_bandwidth);
    if (wait_time) yaml_get_int_value(ctx->document, wait_time, &mount->wait_time);
    if (filter_theora) yaml_get_bool_value(ctx->document, filter_theora, &mount->filter_theora);
    if (limit_rate) yaml_get_bitrate_value(ctx->document, limit_rate, &mount->limit_rate);
    if (skip_accesslog) yaml_get_bool_value(ctx->document, skip_accesslog, &mount->skip_accesslog);
    if (max_send_size) yaml_get_int_value(ctx->document, max_send_size, &mount->max_send_size);
    if (linger_for) yaml_get_int_value(ctx->document, linger_for, &mount->linger_duration);
    if (metadata_interval) yaml_get_int_value(ctx->document, metadata_interval, &mount->mp3_meta_interval);
    if (ogg_passthrough) yaml_get_bool_value(ctx->document, ogg_passthrough, &mount->ogg_passthrough);
    if (admin_comments_only) yaml_get_bool_value(ctx->document, admin_comments_only, &mount->admin_comments_only);
    if (url_ogg_meta) yaml_get_bool_value(ctx->document, url_ogg_meta, &mount->url_ogg_meta);
    if (no_mount) yaml_get_bool_value(ctx->document, no_mount, &mount->no_mount);
    if (ban_client) yaml_get_int_value(ctx->document, ban_client, &mount->ban_client);
    if (intro_skip_replay) yaml_get_int_value(ctx->document, intro_skip_replay, &mount->intro_skip_replay);
    if (so_sndbuf) yaml_get_int_value(ctx->document, so_sndbuf, &mount->so_sndbuf);
    if (hidden) yaml_get_bool_value(ctx->document, hidden, &mount->hidden);
    if (max_stream_duration) yaml_get_int_value(ctx->document, max_stream_duration, (int *)&mount->max_stream_duration);
    if (max_listener_duration) yaml_get_int_value(ctx->document, max_listener_duration, (int *)&mount->max_listener_duration);
    if (public_yp) yaml_get_bool_value(ctx->document, public_yp, &mount->yp_public);

    if (authentication) yaml_parse_mount_auth(ctx, authentication, &mount->auth);
    if (http_headers) yaml_parse_http_headers(ctx, http_headers, &mount->http_headers);
    if (preroll_log) yaml_parse_errorlog(ctx, preroll_log, &mount->preroll_log);
    if (accesslog) yaml_parse_accesslog(ctx, accesslog, &mount->access_log);

    /* Handle redirect special case */
    if (redirect) {
        const char *redirect_str = yaml_get_scalar_value(ctx->document, redirect);
        if (redirect_str) {
            char patt[] = "/${mount}";
            int len = strlen(redirect_str) + strlen(patt) + 1;
            xmlFree(mount->redirect);
            mount->redirect = xmlMalloc(len);
            snprintf(mount->redirect, len, "%s%s", redirect_str, patt);
        }
    }

    /* Validate and adjust settings */
    if (mount->priority < 0) mount->priority = INT_MAX;
    if (mount->auth) auth_finish_setup(mount->auth, mount->mountname);
    if (mount->admin_comments_only) mount->url_ogg_meta = 1;
    if (mount->url_ogg_meta) mount->ogg_passthrough = 0;
    if (mount->ban_client < 0) mount->no_mount = 0;

    if (mount->fallback.mount && mount->fallback.mount[0] != '/') {
        WARN1("fallback does not start with / on %s", mount->mountname);
        xmlFree(mount->fallback.mount);
        mount->fallback.mount = NULL;
    }

    /* Add mount to config */
    if (config_mount_template(mount->mountname)) {
        mount_proxy *m = config->mounts, **trail = &config->mounts;
        while (m && m->priority < mount->priority) {
            trail = &m->next;
            m = *trail;
        }
        mount->next = m;
        *trail = mount;
    } else {
        avl_insert(config->mounts_tree, mount);
    }

    return 0;
}

int yaml_parse_mounts(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_SEQUENCE_NODE)
        return -1;

    yaml_node_item_t *item;
    for (item = node->data.sequence.items.start;
         item < node->data.sequence.items.top; item++)
    {
        yaml_node_t *mount_node = yaml_document_get_node(ctx->document, *item);
        yaml_parse_mount(ctx, mount_node);
    }

    return 0;
}

/* ============================================================================
 * Relay Parsers
 * ============================================================================ */

int yaml_parse_relay_host(yaml_parse_ctx *ctx, yaml_node_t *node, relay_server *relay)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    relay_server_host *host = calloc(1, sizeof(relay_server_host));
    int secure = 0;

    /* Set defaults from relay */
    host->ip = (char *)xmlCharStrdup(relay->hosts->ip);
    host->mount = (char *)xmlCharStrdup(relay->hosts->mount);
    if (relay->hosts->bind)
        host->bind = (char *)xmlCharStrdup(relay->hosts->bind);
    host->port = relay->hosts->port;
    host->timeout = relay->hosts->timeout;

    yaml_node_t *ip = yaml_get_mapping_value(ctx->document, node, "ip");
    yaml_node_t *server = yaml_get_mapping_value(ctx->document, node, "server");
    yaml_node_t *port = yaml_get_mapping_value(ctx->document, node, "port");
    yaml_node_t *mount = yaml_get_mapping_value(ctx->document, node, "mount");
    yaml_node_t *ssl = yaml_get_mapping_value(ctx->document, node, "ssl");
    yaml_node_t *tls = yaml_get_mapping_value(ctx->document, node, "tls");
    yaml_node_t *bind = yaml_get_mapping_value(ctx->document, node, "bind");
    yaml_node_t *timeout = yaml_get_mapping_value(ctx->document, node, "timeout");
    yaml_node_t *headers = yaml_get_mapping_value(ctx->document, node, "http-headers");
    yaml_node_t *priority = yaml_get_mapping_value(ctx->document, node, "priority");

    if (ip || server) {
        const char *val = yaml_get_scalar_value(ctx->document, ip ? ip : server);
        if (val) {
            xmlFree(host->ip);
            host->ip = (char *)xmlCharStrdup(val);
        }
    }

    if (port) yaml_get_port_value(ctx->document, port, &host->port);

    if (mount) {
        const char *val = yaml_get_scalar_value(ctx->document, mount);
        if (val) {
            xmlFree(host->mount);
            host->mount = (char *)xmlCharStrdup(val);
        }
    }

    if (ssl || tls) yaml_get_bool_value(ctx->document, ssl ? ssl : tls, &secure);

    if (bind) {
        const char *val = yaml_get_scalar_value(ctx->document, bind);
        if (val) {
            if (host->bind) xmlFree(host->bind);
            host->bind = (char *)xmlCharStrdup(val);
        }
    }

    if (timeout) yaml_get_int_value(ctx->document, timeout, &host->timeout);
    if (headers) yaml_parse_http_headers(ctx, headers, &host->http_hdrs);
    if (priority) yaml_get_int_value(ctx->document, priority, &host->priority);

    if (host->timeout < 1 || host->timeout > 60)
        host->timeout = 4;

    /* Add to relay's host list */
    relay_server_host *pre = relay->hosts, *chk;
    while ((chk = pre->next)) {
        if (host->priority && (chk->priority > host->priority))
            break;
        pre = pre->next;
    }
    pre->next = host;
    host->next = chk;
    if (secure) host->secure = 1;
    if (host->priority == 0)
        host->priority = pre->priority + 1;

    return 0;
}

int yaml_parse_relay(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_MAPPING_NODE)
        return -1;

    mc_config_t *config = ctx->config;
    relay_server *relay = calloc(1, sizeof(relay_server));
    relay_server_host *host = calloc(1, sizeof(relay_server_host));
    int on_demand = config->on_demand;
    int icy_metadata = 1;
    int running = 1;

    relay->interval = config->master_update_interval;
    relay->run_on = config->master_run_on;
    relay->hosts = host;

    host->port = config->port;
    host->ip = (char *)xmlCharStrdup("127.0.0.1");
    host->mount = (char *)xmlCharStrdup("/");
    host->timeout = 4;

    yaml_node_t *masters = yaml_get_mapping_value(ctx->document, node, "masters");
    yaml_node_t *hosts = yaml_get_mapping_value(ctx->document, node, "hosts");
    yaml_node_t *server = yaml_get_mapping_value(ctx->document, node, "server");
    yaml_node_t *ip = yaml_get_mapping_value(ctx->document, node, "ip");
    yaml_node_t *bind = yaml_get_mapping_value(ctx->document, node, "bind");
    yaml_node_t *port = yaml_get_mapping_value(ctx->document, node, "port");
    yaml_node_t *mount = yaml_get_mapping_value(ctx->document, node, "mount");
    yaml_node_t *timeout = yaml_get_mapping_value(ctx->document, node, "timeout");
    yaml_node_t *local_mount = yaml_get_mapping_value(ctx->document, node, "local-mount");
    yaml_node_t *on_demand_node = yaml_get_mapping_value(ctx->document, node, "on-demand");
    yaml_node_t *run_on = yaml_get_mapping_value(ctx->document, node, "run-on");
    yaml_node_t *http_headers = yaml_get_mapping_value(ctx->document, node, "http-headers");
    yaml_node_t *retry_delay = yaml_get_mapping_value(ctx->document, node, "retry-delay");
    yaml_node_t *relay_icy = yaml_get_mapping_value(ctx->document, node, "relay-icy-metadata");
    yaml_node_t *username = yaml_get_mapping_value(ctx->document, node, "username");
    yaml_node_t *password = yaml_get_mapping_value(ctx->document, node, "password");
    yaml_node_t *enable = yaml_get_mapping_value(ctx->document, node, "enable");

    if (server || ip) {
        const char *val = yaml_get_scalar_value(ctx->document, server ? server : ip);
        if (val) {
            xmlFree(host->ip);
            host->ip = (char *)xmlCharStrdup(val);
        }
    }

    if (bind) {
        const char *val = yaml_get_scalar_value(ctx->document, bind);
        if (val) host->bind = (char *)xmlCharStrdup(val);
    }

    if (port) yaml_get_port_value(ctx->document, port, &host->port);

    if (mount) {
        const char *val = yaml_get_scalar_value(ctx->document, mount);
        if (val) {
            xmlFree(host->mount);
            host->mount = (char *)xmlCharStrdup(val);
        }
    }

    if (timeout) yaml_get_int_value(ctx->document, timeout, &host->timeout);

    if (local_mount) {
        const char *val = yaml_get_scalar_value(ctx->document, local_mount);
        if (val) relay->localmount = (char *)xmlCharStrdup(val);
    }

    if (on_demand_node) yaml_get_bool_value(ctx->document, on_demand_node, &on_demand);
    if (run_on) yaml_get_int_value(ctx->document, run_on, &relay->run_on);
    if (http_headers) yaml_parse_http_headers(ctx, http_headers, &relay->http_hdrs);
    if (retry_delay) yaml_get_int_value(ctx->document, retry_delay, &relay->interval);
    if (relay_icy) yaml_get_bool_value(ctx->document, relay_icy, &icy_metadata);

    if (username) {
        const char *val = yaml_get_scalar_value(ctx->document, username);
        if (val) relay->username = (char *)xmlCharStrdup(val);
    }

    if (password) {
        const char *val = yaml_get_scalar_value(ctx->document, password);
        if (val) relay->password = (char *)xmlCharStrdup(val);
    }

    if (enable) yaml_get_bool_value(ctx->document, enable, &running);

    /* Parse multiple masters/hosts */
    if (masters && masters->type == YAML_SEQUENCE_NODE) {
        yaml_node_item_t *item;
        for (item = masters->data.sequence.items.start;
             item < masters->data.sequence.items.top; item++)
        {
            yaml_node_t *host_node = yaml_document_get_node(ctx->document, *item);
            yaml_parse_relay_host(ctx, host_node, relay);
        }
    }

    if (hosts && hosts->type == YAML_SEQUENCE_NODE) {
        yaml_node_item_t *item;
        for (item = hosts->data.sequence.items.start;
             item < hosts->data.sequence.items.top; item++)
        {
            yaml_node_t *host_node = yaml_document_get_node(ctx->document, *item);
            yaml_parse_relay_host(ctx, host_node, relay);
        }
    }

    if (on_demand) relay->flags |= RELAY_ON_DEMAND;
    if (icy_metadata) relay->flags |= RELAY_ICY_META;
    if (running) relay->flags |= RELAY_RUNNING;

    if (!relay->localmount)
        relay->localmount = (char *)xmlCharStrdup(host->mount);

    if (relay->localmount[0] != '/') {
        WARN1("relay \"%s\" must begin with /, skipping", relay->localmount);
        config_clear_relay(relay);
        return -1;
    }

    /* Remove default host if masters were specified */
    if (relay->hosts->next) {
        relay->hosts = relay->hosts->next;
        if (host->mount) xmlFree(host->mount);
        if (host->ip) xmlFree(host->ip);
        if (host->bind) xmlFree(host->bind);
        free(host);
    }

    relay->new_details = config->relays;
    config->relays = relay;

    return 0;
}

int yaml_parse_relays(yaml_parse_ctx *ctx, yaml_node_t *node)
{
    if (!node || node->type != YAML_SEQUENCE_NODE)
        return -1;

    yaml_node_item_t *item;
    for (item = node->data.sequence.items.start;
         item < node->data.sequence.items.top; item++)
    {
        yaml_node_t *relay_node = yaml_document_get_node(ctx->document, *item);
        yaml_parse_relay(ctx, relay_node);
    }

    return 0;
}

/* ============================================================================
 * Root Parser
 * ============================================================================ */

int yaml_parse_root(yaml_parse_ctx *ctx, yaml_node_t *root)
{
    if (!root || root->type != YAML_MAPPING_NODE)
        return -1;

    mc_config_t *config = ctx->config;

    /* Add default HTTP headers */
    for (int i = 0; default_headers[i].hdr.name; i++) {
        if (_add_http_header(&config->http_headers, &default_headers[i], 1) < 0)
            WARN1("Problem with default header %s", default_headers[i].hdr.name);
    }

    config->master_relay_auth = 1;

    /* Parse all root-level settings */
    yaml_node_t *location = yaml_get_mapping_value(ctx->document, root, "location");
    yaml_node_t *admin = yaml_get_mapping_value(ctx->document, root, "admin");
    yaml_node_t *server_id = yaml_get_mapping_value(ctx->document, root, "server-id");
    yaml_node_t *hostname = yaml_get_mapping_value(ctx->document, root, "hostname");
    yaml_node_t *port = yaml_get_mapping_value(ctx->document, root, "port");
    yaml_node_t *bind_address = yaml_get_mapping_value(ctx->document, root, "bind-address");
    yaml_node_t *fileserve = yaml_get_mapping_value(ctx->document, root, "fileserve");
    yaml_node_t *relays_on_demand = yaml_get_mapping_value(ctx->document, root, "relays-on-demand");
    yaml_node_t *shoutcast_mount = yaml_get_mapping_value(ctx->document, root, "shoutcast-mount");

    yaml_node_t *authentication = yaml_get_mapping_value(ctx->document, root, "authentication");
    yaml_node_t *limits = yaml_get_mapping_value(ctx->document, root, "limits");
    yaml_node_t *paths = yaml_get_mapping_value(ctx->document, root, "paths");
    yaml_node_t *logging = yaml_get_mapping_value(ctx->document, root, "logging");
    yaml_node_t *security = yaml_get_mapping_value(ctx->document, root, "security");
    yaml_node_t *master = yaml_get_mapping_value(ctx->document, root, "master");
    yaml_node_t *directory = yaml_get_mapping_value(ctx->document, root, "directory");
    yaml_node_t *directories = yaml_get_mapping_value(ctx->document, root, "directories");
    yaml_node_t *listen_socket = yaml_get_mapping_value(ctx->document, root, "listen-socket");
    yaml_node_t *listen_sockets = yaml_get_mapping_value(ctx->document, root, "listen-sockets");
    yaml_node_t *http_headers = yaml_get_mapping_value(ctx->document, root, "http-headers");
    yaml_node_t *mounts = yaml_get_mapping_value(ctx->document, root, "mounts");
    yaml_node_t *relays = yaml_get_mapping_value(ctx->document, root, "relays");
    yaml_node_t *redirect = yaml_get_mapping_value(ctx->document, root, "redirect");
    yaml_node_t *redirects = yaml_get_mapping_value(ctx->document, root, "redirects");

    #define SET_STR(node, field) \
        if (node) { \
            const char *val = yaml_get_scalar_value(ctx->document, node); \
            if (val) { \
                if (config->field) xmlFree(config->field); \
                config->field = (char *)xmlCharStrdup(val); \
            } \
        }

    SET_STR(location, location);
    SET_STR(admin, admin);
    SET_STR(server_id, server_id);
    SET_STR(hostname, hostname);
    SET_STR(shoutcast_mount, shoutcast_mount);

    #undef SET_STR

    if (port) yaml_get_port_value(ctx->document, port, &config->port);
    if (fileserve) yaml_get_bool_value(ctx->document, fileserve, &config->fileserve);
    if (relays_on_demand) yaml_get_bool_value(ctx->document, relays_on_demand, &config->on_demand);

    /* Parse complex sections */
    if (authentication) yaml_parse_authentication(ctx, authentication);
    if (limits) yaml_parse_limits(ctx, limits);
    if (paths) yaml_parse_paths(ctx, paths);
    if (logging) yaml_parse_logging(ctx, logging);
    if (security) yaml_parse_security(ctx, security);
    if (master) yaml_parse_master(ctx, master);
    if (http_headers) yaml_parse_http_headers(ctx, http_headers, &config->http_headers);
    if (mounts) yaml_parse_mounts(ctx, mounts);
    if (relays) yaml_parse_relays(ctx, relays);

    /* Parse directory/directories */
    if (directory) yaml_parse_directory(ctx, directory);
    if (directories && directories->type == YAML_SEQUENCE_NODE) {
        yaml_node_item_t *item;
        for (item = directories->data.sequence.items.start;
             item < directories->data.sequence.items.top; item++)
        {
            yaml_node_t *dir_node = yaml_document_get_node(ctx->document, *item);
            yaml_parse_directory(ctx, dir_node);
        }
    }

    /* Parse listen sockets */
    if (listen_socket) yaml_parse_listen_socket(ctx, listen_socket);
    if (listen_sockets) yaml_parse_listen_sockets(ctx, listen_sockets);

    /* Parse redirects */
    if (redirect) yaml_parse_redirect(ctx, redirect);
    if (redirects && redirects->type == YAML_SEQUENCE_NODE) {
        yaml_node_item_t *item;
        for (item = redirects->data.sequence.items.start;
             item < redirects->data.sequence.items.top; item++)
        {
            yaml_node_t *redir_node = yaml_document_get_node(ctx->document, *item);
            yaml_parse_redirect(ctx, redir_node);
        }
    }

    /* Handle bind-address fallback for listen socket */
    if (config->listen_sock_count == 0) {
        if (config->port) {
            listener_t *listener = calloc(1, sizeof(listener_t));
            listener->refcount = 1;
            listener->port = config->port;
            listener->qlen = MC_LISTEN_QUEUE;

            if (bind_address) {
                const char *val = yaml_get_scalar_value(ctx->document, bind_address);
                if (val) listener->bind_address = (char *)xmlStrdup(XMLSTR(val));
            }

            listener->next = config->listen_sock;
            config->listen_sock = listener;
            config->listen_sock_count++;
        } else {
            WARN0("No listen-socket definitions");
            return -1;
        }
    }

    if (config->max_redirects == 0 && config->master_redirect)
        config->max_redirects = 1;

    if (config->master_update_interval < 2)
        config->master_update_interval = 60;

    return 0;
}

/* ============================================================================
 * Main Entry Point
 * ============================================================================ */

int config_parse_yaml_file(const char *filename, mc_config_t *configuration)
{
    FILE *fp;
    yaml_parser_t parser;
    yaml_document_t document;
    yaml_node_t *root;
    unsigned char *file_buf = NULL;
    size_t file_size = 0;
    int ret = CONFIG_EPARSE;

    YAML_TRACE("entry");
    if (!filename || filename[0] == '\0')
        return CONFIG_EINSANE;

    /* Read the file into memory using the exe's own CRT FILE* so we never
     * pass a FILE* across the DLL boundary into yaml.dll (different CRTs on
     * Windows have incompatible FILE struct layouts, causing a crash inside
     * yaml_parser_load when it tries to read through the alien FILE*). */
    YAML_TRACE("fopen");
    fp = fopen(filename, "rb");
    if (!fp) {
        ERROR2("Cannot open YAML config file %s: %s", filename, strerror(errno));
        return CONFIG_EPARSE;
    }
    YAML_TRACE("fopen ok");

    fseek(fp, 0, SEEK_END);
    long flen = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (flen <= 0) {
        ERROR1("YAML config file is empty or unreadable: %s", filename);
        fclose(fp);
        return CONFIG_EPARSE;
    }

    file_size = (size_t)flen;
    file_buf = malloc(file_size + 1);
    if (!file_buf) {
        ERROR0("Out of memory reading YAML config file");
        fclose(fp);
        return CONFIG_EPARSE;
    }

    size_t bytes_read = fread(file_buf, 1, file_size, fp);
    fclose(fp);
    fp = NULL;

    if (bytes_read != file_size) {
        ERROR2("Short read on YAML config file %s: read %lu bytes", filename, (unsigned long)bytes_read);
        free(file_buf);
        return CONFIG_EPARSE;
    }
    YAML_TRACE("file read into memory");

    YAML_TRACE("yaml_parser_initialize");
    if (!yaml_parser_initialize(&parser)) {
        ERROR0("Failed to initialize YAML parser");
        free(file_buf);
        return CONFIG_EPARSE;
    }
    YAML_TRACE("yaml_parser_initialize ok");

    /* Use in-memory string input - avoids passing FILE* to yaml.dll */
    yaml_parser_set_input_string(&parser, file_buf, bytes_read);
    YAML_TRACE("yaml_parser_load");

    if (!yaml_parser_load(&parser, &document)) {
        ERROR3("YAML parse error in %s at line %lu: %s",
               filename,
               (unsigned long)parser.problem_mark.line + 1,
               parser.problem);
        yaml_parser_delete(&parser);
        free(file_buf);
        return CONFIG_EPARSE;
    }
    YAML_TRACE("yaml_parser_load ok");

    free(file_buf);
    file_buf = NULL;

    root = yaml_document_get_root_node(&document);
    if (!root) {
        ERROR1("No root node in YAML config: %s", filename);
        yaml_document_delete(&document);
        yaml_parser_delete(&parser);
        return CONFIG_ENOROOT;
    }
    YAML_TRACE("root node ok");

    if (root->type != YAML_MAPPING_NODE) {
        ERROR1("YAML root must be a mapping in %s", filename);
        yaml_document_delete(&document);
        yaml_parser_delete(&parser);
        return CONFIG_EBADROOT;
    }
    YAML_TRACE("root is mapping");

    YAML_TRACE("config_init_configuration");
    config_init_configuration(configuration);
    YAML_TRACE("config_init_configuration ok");
    configuration->config_filename = strdup(filename);

    yaml_parse_ctx ctx = {
        .document = &document,
        .config = configuration,
        .filename = filename
    };

    YAML_TRACE("yaml_parse_root");
    if (yaml_parse_root(&ctx, root) == 0)
        ret = 0;
    YAML_TRACE("yaml_parse_root done");

    yaml_document_delete(&document);
    yaml_parser_delete(&parser);

    return ret;
}

#endif /* HAVE_YAML */

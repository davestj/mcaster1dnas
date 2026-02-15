/* Mcaster1DNAS - Digital Network Audio Server
 *
 * YAML Configuration File Parser
 *
 * This program is distributed under the GNU General Public License, version 2.
 * A copy of this license is included with this source.
 *
 * Copyright 2025-2026, Saint John (David St John) <davestj@gmail.com>
 */

#ifndef __CFGFILE_YAML_H__
#define __CFGFILE_YAML_H__

#ifdef HAVE_YAML

#include <yaml.h>
#include "cfgfile.h"

/* Main YAML config parsing entry point */
int config_parse_yaml_file(const char *filename, mc_config_t *configuration);

/* YAML parsing context structure */
typedef struct {
    yaml_document_t *document;
    mc_config_t *config;
    const char *filename;
} yaml_parse_ctx;

/* YAML node helper functions */
const char *yaml_get_scalar_value(yaml_document_t *doc, yaml_node_t *node);
yaml_node_t *yaml_get_mapping_value(yaml_document_t *doc, yaml_node_t *mapping, const char *key);
int yaml_get_bool_value(yaml_document_t *doc, yaml_node_t *node, int *result);
int yaml_get_int_value(yaml_document_t *doc, yaml_node_t *node, int *result);
int yaml_get_long_value(yaml_document_t *doc, yaml_node_t *node, long *result);
int yaml_get_port_value(yaml_document_t *doc, yaml_node_t *node, int *result);
int yaml_get_bitrate_value(yaml_document_t *doc, yaml_node_t *node, int64_t *result);
int yaml_get_qsizing_value(yaml_document_t *doc, yaml_node_t *node, uint32_t *result);
int yaml_get_loglevel_value(yaml_document_t *doc, yaml_node_t *node, log_levels_t *result);

/* YAML section parsers (mirrors XML parser structure) */
int yaml_parse_root(yaml_parse_ctx *ctx, yaml_node_t *root);
int yaml_parse_authentication(yaml_parse_ctx *ctx, yaml_node_t *node);
int yaml_parse_limits(yaml_parse_ctx *ctx, yaml_node_t *node);
int yaml_parse_paths(yaml_parse_ctx *ctx, yaml_node_t *node);
int yaml_parse_logging(yaml_parse_ctx *ctx, yaml_node_t *node);
int yaml_parse_security(yaml_parse_ctx *ctx, yaml_node_t *node);
int yaml_parse_master(yaml_parse_ctx *ctx, yaml_node_t *node);
int yaml_parse_directory(yaml_parse_ctx *ctx, yaml_node_t *node);
int yaml_parse_http_headers(yaml_parse_ctx *ctx, yaml_node_t *node, mc_config_http_header_t **headers);
int yaml_parse_listen_socket(yaml_parse_ctx *ctx, yaml_node_t *node);
int yaml_parse_listen_sockets(yaml_parse_ctx *ctx, yaml_node_t *node);
int yaml_parse_mount(yaml_parse_ctx *ctx, yaml_node_t *node);
int yaml_parse_mounts(yaml_parse_ctx *ctx, yaml_node_t *node);
int yaml_parse_relay(yaml_parse_ctx *ctx, yaml_node_t *node);
int yaml_parse_relays(yaml_parse_ctx *ctx, yaml_node_t *node);
int yaml_parse_redirect(yaml_parse_ctx *ctx, yaml_node_t *node);
int yaml_parse_alias(yaml_parse_ctx *ctx, yaml_node_t *node);
int yaml_parse_accesslog(yaml_parse_ctx *ctx, yaml_node_t *node, struct access_log *log);
int yaml_parse_errorlog(yaml_parse_ctx *ctx, yaml_node_t *node, error_log *log);
int yaml_parse_playlistlog(yaml_parse_ctx *ctx, yaml_node_t *node, playlist_log *log);
int yaml_parse_mount_auth(yaml_parse_ctx *ctx, yaml_node_t *node, auth_t **auth_out);
int yaml_parse_fallback(yaml_parse_ctx *ctx, yaml_node_t *node, fbinfo *fb);
int yaml_parse_relay_host(yaml_parse_ctx *ctx, yaml_node_t *node, relay_server *relay);

/* YAML error reporting */
void yaml_parse_error(yaml_parse_ctx *ctx, yaml_node_t *node, const char *fmt, ...);
void yaml_parse_warning(yaml_parse_ctx *ctx, yaml_node_t *node, const char *fmt, ...);

#endif /* HAVE_YAML */

#endif /* __CFGFILE_YAML_H__ */

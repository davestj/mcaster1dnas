/* Mcaster1DNAS - Digital Network Audio Server
 * ICY-META v2.1+ Protocol Support
 *
 * Copyright (C) 2026 David St John (Saint John) <davestj@gmail.com>
 *
 * This program is distributed under the GNU General Public License, version 2.
 * A copy of this license is included with this source.
 */

#ifndef __ICY2_META_H__
#define __ICY2_META_H__

#include "httpp/httpp.h"
#include "stats.h"

/* ICY2 Metadata Storage Structure */
typedef struct icy2_metadata_tag {
	/* Core Metadata */
	char *station_id;
	char *station_name;
	char *station_url;
	char *station_genre;
	int station_bitrate;
	int station_public;

	/* Podcast Metadata */
	char *podcast_host;
	char *podcast_rss;
	char *podcast_episode;
	char *podcast_duration;
	char *podcast_language;

	/* Video Metadata */
	char *video_type;        /* live, vod, short */
	char *video_link;
	char *video_title;
	char *video_platform;    /* youtube, twitch, kick, rumble, etc. */
	char *video_resolution;  /* 1080p, 4K, etc. */

	/* Social Media */
	char *dj_handle;
	char *social_twitter;
	char *social_instagram;
	char *social_tiktok;
	char *emoji;
	char *hashtags;

	/* Access Control */
	char *auth_token;
	int nsfw;
	int ai_generated;
	char *geo_region;

	/* Protocol Version */
	char *metadata_version;   /* "2.1", "2.2", etc. */

	/* Verification */
	char *certificate_verify;
	char *verification_status;

} icy2_metadata;

/* Function Prototypes */

/**
 * icy2_meta_new - Create new ICY2 metadata structure
 *
 * Returns: Pointer to allocated icy2_metadata structure, or NULL on error
 */
icy2_metadata *icy2_meta_new(void);

/**
 * icy2_meta_free - Free ICY2 metadata structure
 *
 * @meta: ICY2 metadata structure to free
 */
void icy2_meta_free(icy2_metadata *meta);

/**
 * icy2_meta_is_icy2 - Check if HTTP request is ICY2-compliant
 *
 * @parser: HTTP parser containing request headers
 *
 * Returns: 1 if ICY2 detected (icy-metadata-version: 2.x), 0 otherwise
 */
int icy2_meta_is_icy2(http_parser_t *parser);

/**
 * icy2_meta_parse_headers - Parse ICY2 headers from HTTP request
 *
 * @meta: ICY2 metadata structure to populate
 * @parser: HTTP parser containing request headers
 *
 * Returns: 0 on success, -1 on error
 */
int icy2_meta_parse_headers(icy2_metadata *meta, http_parser_t *parser);

/**
 * icy2_meta_copy_to_stats - Copy ICY2 metadata to stats system
 *
 * @meta: ICY2 metadata structure
 * @stats_handle: Stats handle from source->stats
 *
 * Copies all ICY2 metadata fields to the stats system for display
 * in the admin interface. Uses "icy2-" prefix for all fields.
 * Caller must lock/unlock stats handle.
 */
void icy2_meta_copy_to_stats(icy2_metadata *meta, stats_handle_t stats_handle);

#endif /* __ICY2_META_H__ */

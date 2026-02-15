/* Mcaster1DNAS - Digital Network Audio Server
 * ICY-META v2.1+ Protocol Support
 *
 * Copyright (C) 2026 David St John (Saint John) <davestj@gmail.com>
 *
 * This program is distributed under the GNU General Public License, version 2.
 * A copy of this license is included with this source.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string.h>
#include <stdlib.h>

#include "icy2_meta.h"
#include "stats.h"
#include "logging.h"

#define CATMODULE "icy2"

/* Helper: Safe string duplicate */
static char *icy2_strdup_safe(const char *str)
{
	if (!str) return NULL;
	return strdup(str);
}

/* Create new ICY2 metadata structure */
icy2_metadata *icy2_meta_new(void)
{
	icy2_metadata *meta = calloc(1, sizeof(icy2_metadata));
	if (!meta) {
		ERROR0("Failed to allocate ICY2 metadata structure");
		return NULL;
	}

	/* All fields initialized to NULL/0 by calloc */
	DEBUG0("Created new ICY2 metadata structure");
	return meta;
}

/* Free ICY2 metadata structure */
void icy2_meta_free(icy2_metadata *meta)
{
	if (!meta) return;

	/* Free all string fields */
	free(meta->station_id);
	free(meta->station_name);
	free(meta->station_url);
	free(meta->station_genre);
	free(meta->podcast_host);
	free(meta->podcast_rss);
	free(meta->podcast_episode);
	free(meta->podcast_duration);
	free(meta->podcast_language);
	free(meta->video_type);
	free(meta->video_link);
	free(meta->video_title);
	free(meta->video_platform);
	free(meta->video_resolution);
	free(meta->dj_handle);
	free(meta->social_twitter);
	free(meta->social_instagram);
	free(meta->social_tiktok);
	free(meta->emoji);
	free(meta->hashtags);
	free(meta->auth_token);
	free(meta->geo_region);
	free(meta->metadata_version);
	free(meta->certificate_verify);
	free(meta->verification_status);

	free(meta);
	DEBUG0("Freed ICY2 metadata structure");
}

/* Check if request is ICY2-compliant */
int icy2_meta_is_icy2(http_parser_t *parser)
{
	const char *version;

	if (!parser) return 0;

	version = httpp_getvar(parser, "icy-metadata-version");
	if (!version) {
		DEBUG0("No icy-metadata-version header found, not ICY2");
		return 0;
	}

	/* Accept version 2.x (2.0, 2.1, 2.2, etc.) */
	if (strncmp(version, "2.", 2) == 0) {
		INFO1("Detected ICY-META version %s", version);
		return 1;
	}

	WARN1("Unknown icy-metadata-version: %s (expected 2.x)", version);
	return 0;
}

/* Parse ICY2 headers from HTTP parser */
int icy2_meta_parse_headers(icy2_metadata *meta, http_parser_t *parser)
{
	const char *str;
	int field_count = 0;

	if (!meta || !parser) {
		ERROR0("Invalid arguments to icy2_meta_parse_headers");
		return -1;
	}

	/* Metadata version */
	str = httpp_getvar(parser, "icy-metadata-version");
	if (str) {
		meta->metadata_version = icy2_strdup_safe(str);
		field_count++;
	}

	/* Core metadata */
	str = httpp_getvar(parser, "icy-station-id");
	if (str) {
		meta->station_id = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-name");
	if (str) {
		meta->station_name = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-url");
	if (str) {
		meta->station_url = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-genre");
	if (str) {
		meta->station_genre = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-br");
	if (str) {
		meta->station_bitrate = atoi(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-pub");
	if (str) {
		meta->station_public = atoi(str);
		field_count++;
	}

	/* Podcast metadata */
	str = httpp_getvar(parser, "icy-podcast-host");
	if (str) {
		meta->podcast_host = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-podcast-rss");
	if (str) {
		meta->podcast_rss = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-podcast-episode");
	if (str) {
		meta->podcast_episode = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-duration");
	if (str) {
		meta->podcast_duration = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-language");
	if (str) {
		meta->podcast_language = icy2_strdup_safe(str);
		field_count++;
	}

	/* Video metadata */
	str = httpp_getvar(parser, "icy-video-type");
	if (str) {
		meta->video_type = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-video-link");
	if (str) {
		meta->video_link = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-video-title");
	if (str) {
		meta->video_title = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-video-platform");
	if (str) {
		meta->video_platform = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-video-resolution");
	if (str) {
		meta->video_resolution = icy2_strdup_safe(str);
		field_count++;
	}

	/* Social media */
	str = httpp_getvar(parser, "icy-dj-handle");
	if (str) {
		meta->dj_handle = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-social-twitter");
	if (str) {
		meta->social_twitter = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-social-ig");
	if (str) {
		meta->social_instagram = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-social-tiktok");
	if (str) {
		meta->social_tiktok = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-emoji");
	if (str) {
		meta->emoji = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-hashtags");
	if (str) {
		meta->hashtags = icy2_strdup_safe(str);
		field_count++;
	}

	/* Access control */
	str = httpp_getvar(parser, "icy-auth-token");
	if (str) {
		meta->auth_token = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-nsfw");
	if (str) {
		meta->nsfw = atoi(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-ai-generated");
	if (str) {
		meta->ai_generated = atoi(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-geo-region");
	if (str) {
		meta->geo_region = icy2_strdup_safe(str);
		field_count++;
	}

	/* Verification */
	str = httpp_getvar(parser, "icy-certificate-verify");
	if (str) {
		meta->certificate_verify = icy2_strdup_safe(str);
		field_count++;
	}

	str = httpp_getvar(parser, "icy-verification-status");
	if (str) {
		meta->verification_status = icy2_strdup_safe(str);
		field_count++;
	}

	INFO2("Parsed %d ICY2 metadata fields for station-id: %s",
	      field_count,
	      meta->station_id ? meta->station_id : "unknown");

	return 0;
}

/* Copy ICY2 metadata to stats system */
void icy2_meta_copy_to_stats(icy2_metadata *meta, stats_handle_t handle)
{
	if (!meta || !handle) {
		ERROR0("Invalid arguments to icy2_meta_copy_to_stats");
		return;
	}

	/* Protocol version - always set this first */
	if (meta->metadata_version)
		stats_set(handle, "icy2-version", meta->metadata_version);

	/* Core metadata */
	if (meta->station_id)
		stats_set(handle, "icy2-station-id", meta->station_id);
	if (meta->station_name)
		stats_set(handle, "server_name", meta->station_name);
	if (meta->station_url)
		stats_set(handle, "server_url", meta->station_url);
	if (meta->station_genre)
		stats_set(handle, "genre", meta->station_genre);
	if (meta->station_bitrate > 0)
		stats_set_args(handle, "bitrate", "%d", meta->station_bitrate);
	if (meta->station_public >= 0)
		stats_set_args(handle, "public", "%d", meta->station_public);

	/* Podcast metadata */
	if (meta->podcast_host)
		stats_set(handle, "icy2-podcast-host", meta->podcast_host);
	if (meta->podcast_rss)
		stats_set(handle, "icy2-podcast-rss", meta->podcast_rss);
	if (meta->podcast_episode)
		stats_set(handle, "icy2-podcast-episode", meta->podcast_episode);
	if (meta->podcast_duration)
		stats_set(handle, "icy2-duration", meta->podcast_duration);
	if (meta->podcast_language)
		stats_set(handle, "icy2-language", meta->podcast_language);

	/* Video metadata */
	if (meta->video_type)
		stats_set(handle, "icy2-video-type", meta->video_type);
	if (meta->video_link)
		stats_set(handle, "icy2-video-link", meta->video_link);
	if (meta->video_title)
		stats_set(handle, "icy2-video-title", meta->video_title);
	if (meta->video_platform)
		stats_set(handle, "icy2-video-platform", meta->video_platform);
	if (meta->video_resolution)
		stats_set(handle, "icy2-video-resolution", meta->video_resolution);

	/* Social media */
	if (meta->dj_handle)
		stats_set(handle, "icy2-dj-handle", meta->dj_handle);
	if (meta->social_twitter)
		stats_set(handle, "icy2-social-twitter", meta->social_twitter);
	if (meta->social_instagram)
		stats_set(handle, "icy2-social-instagram", meta->social_instagram);
	if (meta->social_tiktok)
		stats_set(handle, "icy2-social-tiktok", meta->social_tiktok);
	if (meta->emoji)
		stats_set(handle, "icy2-emoji", meta->emoji);
	if (meta->hashtags)
		stats_set(handle, "icy2-hashtags", meta->hashtags);

	/* Access control */
	if (meta->auth_token)
		stats_set(handle, "icy2-auth-token", meta->auth_token);
	if (meta->nsfw)
		stats_set_args(handle, "icy2-nsfw", "%d", meta->nsfw);
	if (meta->ai_generated)
		stats_set_args(handle, "icy2-ai-generated", "%d", meta->ai_generated);
	if (meta->geo_region)
		stats_set(handle, "icy2-geo-region", meta->geo_region);

	/* Verification */
	if (meta->certificate_verify)
		stats_set(handle, "icy2-certificate-verify", meta->certificate_verify);
	if (meta->verification_status)
		stats_set(handle, "icy2-verification-status", meta->verification_status);

	INFO0("ICY2 metadata copied to stats");
}

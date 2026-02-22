/* Mcaster1DNAS - Digital Network Audio Server
 * ICY-META v2.2 Protocol Support
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

/*
 * Helper: Get ICY2 header value with dual-prefix fallback.
 * Tries icy-meta-<field> first (v2.2 canonical), then icy-<field> (v2.1 compat).
 * For new v2.2-only fields, pass NULL as the legacy name.
 */
static const char *icy2_getvar(http_parser_t *parser, const char *new_name, const char *legacy_name)
{
	const char *val = httpp_getvar(parser, new_name);
	if (!val && legacy_name)
		val = httpp_getvar(parser, legacy_name);
	return val;
}

/* Create new ICY2 metadata structure */
icy2_metadata *icy2_meta_new(void)
{
	icy2_metadata *meta = calloc(1, sizeof(icy2_metadata));
	if (!meta) {
		ERROR0("Failed to allocate ICY2 metadata structure");
		return NULL;
	}

	/* Integer fields default: -1 means "not set" for flags, 0 for counts */
	meta->station_bitrate = 0;
	meta->station_public  = -1;
	meta->nsfw            = -1;
	meta->ai_generated    = -1;
	meta->autodj          = -1;
	meta->audio_lossless  = -1;

	DEBUG0("Created new ICY2 metadata structure");
	return meta;
}

/* Free ICY2 metadata structure */
void icy2_meta_free(icy2_metadata *meta)
{
	if (!meta) return;

	/* Protocol version */
	free(meta->metadata_version);

	/* Core */
	free(meta->station_id);
	free(meta->station_name);
	free(meta->station_url);
	free(meta->station_genre);

	/* Station Identity (v2.2) */
	free(meta->station_slogan);
	free(meta->station_logo);
	free(meta->station_founded);
	free(meta->station_type);
	free(meta->station_country);

	/* Programming / Show (v2.2) */
	free(meta->show_title);
	free(meta->show_episode);
	free(meta->show_season);
	free(meta->show_start_time);
	free(meta->show_end_time);
	free(meta->playlist_name);
	free(meta->stream_session_id);

	/* DJ / Host */
	free(meta->dj_handle);
	free(meta->dj_bio);
	free(meta->dj_showrating);
	free(meta->dj_photo);

	/* Track Metadata (v2.2) */
	free(meta->track_artist);
	free(meta->track_album);
	free(meta->track_title);
	free(meta->track_year);
	free(meta->track_isrc);
	free(meta->track_artwork);
	free(meta->track_buy_url);
	free(meta->track_label);
	free(meta->track_bpm);

	/* Podcast */
	free(meta->podcast_host);
	free(meta->podcast_rss);
	free(meta->podcast_episode);
	free(meta->podcast_duration);
	free(meta->podcast_language);
	free(meta->podcast_rating);

	/* Audio Technical (v2.2) */
	free(meta->audio_codec);
	free(meta->audio_samplerate);
	free(meta->audio_channels);
	free(meta->audio_quality);

	/* Video */
	free(meta->video_type);
	free(meta->video_link);
	free(meta->video_title);
	free(meta->video_platform);
	free(meta->video_resolution);
	free(meta->video_rating);
	free(meta->video_codec);
	free(meta->video_framerate);
	free(meta->video_bitrate);
	free(meta->video_aspect);
	free(meta->video_hdr);
	free(meta->video_thumbnail);
	free(meta->video_duration);

	/* Social Media */
	free(meta->social_twitter);
	free(meta->social_instagram);
	free(meta->social_tiktok);
	free(meta->social_facebook);
	free(meta->social_youtube);
	free(meta->social_bluesky);
	free(meta->social_website);
	free(meta->tip_url);
	free(meta->chat_url);
	free(meta->emoji);
	free(meta->hashtags);

	/* Listener Engagement (v2.2) */
	free(meta->listener_peak);
	free(meta->listener_count);
	free(meta->like_count);
	free(meta->share_url);
	free(meta->request_url);

	/* Broadcast Distribution (v2.2) */
	free(meta->cdn_region);
	free(meta->relay_origin);
	free(meta->stream_failover);
	free(meta->stream_quality_tier);

	/* Station Notices (v2.2) */
	free(meta->notice_board);
	free(meta->upcoming_show);
	free(meta->emergency_alert);

	/* Access Control */
	free(meta->auth_token);
	free(meta->geo_region);

	/* Compliance (v2.2) */
	free(meta->content_rating);
	free(meta->parental_advisory);
	free(meta->dmca_compliant);

	/* Verification */
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

	/* Check v2.2 canonical header first, then v2.1 legacy header */
	version = httpp_getvar(parser, "icy-meta-version");
	if (!version)
		version = httpp_getvar(parser, "icy-metadata-version");

	if (!version) {
		DEBUG0("No icy-meta-version header found, not ICY2");
		return 0;
	}

	/* Accept version 2.x (2.0, 2.1, 2.2, etc.) */
	if (strncmp(version, "2.", 2) == 0) {
		INFO1("Detected ICY-META version %s", version);
		return 1;
	}

	WARN1("Unknown icy-meta-version: %s (expected 2.x)", version);
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

	/* --- Protocol Version --- */
	str = icy2_getvar(parser, "icy-meta-version", "icy-metadata-version");
	if (str) { meta->metadata_version = icy2_strdup_safe(str); field_count++; }

	/* --- Core Metadata (dual-prefix: v2.2 canonical / v2.1 legacy) --- */
	str = icy2_getvar(parser, "icy-meta-station-id", "icy-station-id");
	if (str) { meta->station_id = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-name", "icy-name");
	if (str) { meta->station_name = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-url", "icy-url");
	if (str) { meta->station_url = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-genre", "icy-genre");
	if (str) { meta->station_genre = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-br", "icy-br");
	if (str) { meta->station_bitrate = atoi(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-pub", "icy-pub");
	if (str) { meta->station_public = atoi(str); field_count++; }

	/* --- Station Identity (v2.2 only) --- */
	str = httpp_getvar(parser, "icy-meta-station-slogan");
	if (str) { meta->station_slogan = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-station-logo");
	if (str) { meta->station_logo = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-station-founded");
	if (str) { meta->station_founded = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-station-type");
	if (str) { meta->station_type = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-station-country");
	if (str) { meta->station_country = icy2_strdup_safe(str); field_count++; }

	/* --- Programming / Show (v2.2) --- */
	str = httpp_getvar(parser, "icy-meta-show-title");
	if (str) { meta->show_title = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-show-episode");
	if (str) { meta->show_episode = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-show-season");
	if (str) { meta->show_season = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-show-start-time");
	if (str) { meta->show_start_time = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-show-end-time");
	if (str) { meta->show_end_time = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-playlist-name");
	if (str) { meta->playlist_name = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-autodj");
	if (str) { meta->autodj = atoi(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-stream-session-id");
	if (str) { meta->stream_session_id = icy2_strdup_safe(str); field_count++; }

	/* --- DJ / Host (dual-prefix for handle, v2.2-only for bio/rating/photo) --- */
	str = icy2_getvar(parser, "icy-meta-dj-handle", "icy-dj-handle");
	if (str) { meta->dj_handle = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-dj-bio");
	if (str) { meta->dj_bio = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-dj-showrating");
	if (str) { meta->dj_showrating = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-dj-photo");
	if (str) { meta->dj_photo = icy2_strdup_safe(str); field_count++; }

	/* --- Track Metadata (v2.2) --- */
	str = httpp_getvar(parser, "icy-meta-track-artist");
	if (str) { meta->track_artist = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-track-album");
	if (str) { meta->track_album = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-track-title");
	if (str) { meta->track_title = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-track-year");
	if (str) { meta->track_year = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-track-isrc");
	if (str) { meta->track_isrc = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-track-artwork");
	if (str) { meta->track_artwork = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-track-buy-url");
	if (str) { meta->track_buy_url = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-track-label");
	if (str) { meta->track_label = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-track-bpm");
	if (str) { meta->track_bpm = icy2_strdup_safe(str); field_count++; }

	/* --- Podcast Metadata (dual-prefix for legacy fields) --- */
	str = icy2_getvar(parser, "icy-meta-podcast-host", "icy-podcast-host");
	if (str) { meta->podcast_host = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-podcast-rss", "icy-podcast-rss");
	if (str) { meta->podcast_rss = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-podcast-episode", "icy-podcast-episode");
	if (str) { meta->podcast_episode = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-duration", "icy-duration");
	if (str) { meta->podcast_duration = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-language", "icy-language");
	if (str) { meta->podcast_language = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-podcast-rating");
	if (str) { meta->podcast_rating = icy2_strdup_safe(str); field_count++; }

	/* --- Audio Technical (v2.2) --- */
	str = httpp_getvar(parser, "icy-meta-audio-codec");
	if (str) { meta->audio_codec = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-audio-samplerate");
	if (str) { meta->audio_samplerate = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-audio-channels");
	if (str) { meta->audio_channels = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-audio-quality");
	if (str) { meta->audio_quality = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-audio-lossless");
	if (str) { meta->audio_lossless = atoi(str); field_count++; }

	/* --- Video Metadata (dual-prefix for legacy fields) --- */
	str = icy2_getvar(parser, "icy-meta-video-type", "icy-video-type");
	if (str) { meta->video_type = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-video-link", "icy-video-link");
	if (str) { meta->video_link = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-video-title", "icy-video-title");
	if (str) { meta->video_title = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-video-platform", "icy-video-platform");
	if (str) { meta->video_platform = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-video-resolution", "icy-video-resolution");
	if (str) { meta->video_resolution = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-video-rating");
	if (str) { meta->video_rating = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-video-codec");
	if (str) { meta->video_codec = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-video-framerate");
	if (str) { meta->video_framerate = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-video-bitrate");
	if (str) { meta->video_bitrate = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-video-aspect");
	if (str) { meta->video_aspect = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-video-hdr");
	if (str) { meta->video_hdr = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-video-thumbnail");
	if (str) { meta->video_thumbnail = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-video-duration");
	if (str) { meta->video_duration = icy2_strdup_safe(str); field_count++; }

	/* --- Social Media (dual-prefix for legacy fields) --- */
	str = icy2_getvar(parser, "icy-meta-social-twitter", "icy-social-twitter");
	if (str) { meta->social_twitter = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-social-ig", "icy-social-ig");
	if (str) { meta->social_instagram = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-social-tiktok", "icy-social-tiktok");
	if (str) { meta->social_tiktok = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-social-facebook");
	if (str) { meta->social_facebook = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-social-youtube");
	if (str) { meta->social_youtube = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-social-bluesky");
	if (str) { meta->social_bluesky = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-social-website");
	if (str) { meta->social_website = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-tip-url");
	if (str) { meta->tip_url = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-chat-url");
	if (str) { meta->chat_url = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-emoji", "icy-emoji");
	if (str) { meta->emoji = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-hashtags", "icy-hashtags");
	if (str) { meta->hashtags = icy2_strdup_safe(str); field_count++; }

	/* --- Listener Engagement (v2.2) --- */
	str = httpp_getvar(parser, "icy-meta-listener-peak");
	if (str) { meta->listener_peak = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-listener-count");
	if (str) { meta->listener_count = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-like-count");
	if (str) { meta->like_count = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-share-url");
	if (str) { meta->share_url = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-request-url");
	if (str) { meta->request_url = icy2_strdup_safe(str); field_count++; }

	/* --- Broadcast Distribution (v2.2) --- */
	str = httpp_getvar(parser, "icy-meta-cdn-region");
	if (str) { meta->cdn_region = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-relay-origin");
	if (str) { meta->relay_origin = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-stream-failover");
	if (str) { meta->stream_failover = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-stream-quality-tier");
	if (str) { meta->stream_quality_tier = icy2_strdup_safe(str); field_count++; }

	/* --- Station Notices (v2.2) --- */
	str = httpp_getvar(parser, "icy-meta-notice-board");
	if (str) { meta->notice_board = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-upcoming-show");
	if (str) { meta->upcoming_show = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-emergency-alert");
	if (str) { meta->emergency_alert = icy2_strdup_safe(str); field_count++; }

	/* --- Access Control (dual-prefix for legacy fields) --- */
	str = icy2_getvar(parser, "icy-meta-auth-token", "icy-auth-token");
	if (str) { meta->auth_token = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-nsfw", "icy-nsfw");
	if (str) { meta->nsfw = atoi(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-ai-generated", "icy-ai-generated");
	if (str) { meta->ai_generated = atoi(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-geo-region", "icy-geo-region");
	if (str) { meta->geo_region = icy2_strdup_safe(str); field_count++; }

	/* --- Compliance (v2.2) --- */
	str = httpp_getvar(parser, "icy-meta-content-rating");
	if (str) { meta->content_rating = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-parental-advisory");
	if (str) { meta->parental_advisory = icy2_strdup_safe(str); field_count++; }

	str = httpp_getvar(parser, "icy-meta-dmca-compliant");
	if (str) { meta->dmca_compliant = icy2_strdup_safe(str); field_count++; }

	/* --- Verification (dual-prefix) --- */
	str = icy2_getvar(parser, "icy-meta-certificate-verify", "icy-certificate-verify");
	if (str) { meta->certificate_verify = icy2_strdup_safe(str); field_count++; }

	str = icy2_getvar(parser, "icy-meta-verification-status", "icy-verification-status");
	if (str) { meta->verification_status = icy2_strdup_safe(str); field_count++; }

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

	/* Protocol version */
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

	/* Station Identity (v2.2) */
	if (meta->station_slogan)
		stats_set(handle, "icy2-station-slogan", meta->station_slogan);
	if (meta->station_logo)
		stats_set(handle, "icy2-station-logo", meta->station_logo);
	if (meta->station_founded)
		stats_set(handle, "icy2-station-founded", meta->station_founded);
	if (meta->station_type)
		stats_set(handle, "icy2-station-type", meta->station_type);
	if (meta->station_country)
		stats_set(handle, "icy2-station-country", meta->station_country);

	/* Programming / Show (v2.2) */
	if (meta->show_title)
		stats_set(handle, "icy2-show-title", meta->show_title);
	if (meta->show_episode)
		stats_set(handle, "icy2-show-episode", meta->show_episode);
	if (meta->show_season)
		stats_set(handle, "icy2-show-season", meta->show_season);
	if (meta->show_start_time)
		stats_set(handle, "icy2-show-start-time", meta->show_start_time);
	if (meta->show_end_time)
		stats_set(handle, "icy2-show-end-time", meta->show_end_time);
	if (meta->playlist_name)
		stats_set(handle, "icy2-playlist-name", meta->playlist_name);
	if (meta->autodj >= 0)
		stats_set_args(handle, "icy2-autodj", "%d", meta->autodj);
	if (meta->stream_session_id)
		stats_set(handle, "icy2-stream-session-id", meta->stream_session_id);

	/* DJ / Host */
	if (meta->dj_handle)
		stats_set(handle, "icy2-dj-handle", meta->dj_handle);
	if (meta->dj_bio)
		stats_set(handle, "icy2-dj-bio", meta->dj_bio);
	if (meta->dj_showrating)
		stats_set(handle, "icy2-dj-showrating", meta->dj_showrating);
	if (meta->dj_photo)
		stats_set(handle, "icy2-dj-photo", meta->dj_photo);

	/* Track Metadata (v2.2) */
	if (meta->track_artist)
		stats_set(handle, "icy2-track-artist", meta->track_artist);
	if (meta->track_album)
		stats_set(handle, "icy2-track-album", meta->track_album);
	if (meta->track_title)
		stats_set(handle, "icy2-track-title", meta->track_title);
	if (meta->track_year)
		stats_set(handle, "icy2-track-year", meta->track_year);
	if (meta->track_isrc)
		stats_set(handle, "icy2-track-isrc", meta->track_isrc);
	if (meta->track_artwork)
		stats_set(handle, "icy2-track-artwork", meta->track_artwork);
	if (meta->track_buy_url)
		stats_set(handle, "icy2-track-buy-url", meta->track_buy_url);
	if (meta->track_label)
		stats_set(handle, "icy2-track-label", meta->track_label);
	if (meta->track_bpm)
		stats_set(handle, "icy2-track-bpm", meta->track_bpm);

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
	if (meta->podcast_rating)
		stats_set(handle, "icy2-podcast-rating", meta->podcast_rating);

	/* Audio Technical (v2.2) */
	if (meta->audio_codec)
		stats_set(handle, "icy2-audio-codec", meta->audio_codec);
	if (meta->audio_samplerate)
		stats_set(handle, "icy2-audio-samplerate", meta->audio_samplerate);
	if (meta->audio_channels)
		stats_set(handle, "icy2-audio-channels", meta->audio_channels);
	if (meta->audio_quality)
		stats_set(handle, "icy2-audio-quality", meta->audio_quality);
	if (meta->audio_lossless >= 0)
		stats_set_args(handle, "icy2-audio-lossless", "%d", meta->audio_lossless);

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
	if (meta->video_rating)
		stats_set(handle, "icy2-video-rating", meta->video_rating);
	if (meta->video_codec)
		stats_set(handle, "icy2-video-codec", meta->video_codec);
	if (meta->video_framerate)
		stats_set(handle, "icy2-video-framerate", meta->video_framerate);
	if (meta->video_bitrate)
		stats_set(handle, "icy2-video-bitrate", meta->video_bitrate);
	if (meta->video_aspect)
		stats_set(handle, "icy2-video-aspect", meta->video_aspect);
	if (meta->video_hdr)
		stats_set(handle, "icy2-video-hdr", meta->video_hdr);
	if (meta->video_thumbnail)
		stats_set(handle, "icy2-video-thumbnail", meta->video_thumbnail);
	if (meta->video_duration)
		stats_set(handle, "icy2-video-duration", meta->video_duration);

	/* Social media */
	if (meta->social_twitter)
		stats_set(handle, "icy2-social-twitter", meta->social_twitter);
	if (meta->social_instagram)
		stats_set(handle, "icy2-social-instagram", meta->social_instagram);
	if (meta->social_tiktok)
		stats_set(handle, "icy2-social-tiktok", meta->social_tiktok);
	if (meta->social_facebook)
		stats_set(handle, "icy2-social-facebook", meta->social_facebook);
	if (meta->social_youtube)
		stats_set(handle, "icy2-social-youtube", meta->social_youtube);
	if (meta->social_bluesky)
		stats_set(handle, "icy2-social-bluesky", meta->social_bluesky);
	if (meta->social_website)
		stats_set(handle, "icy2-social-website", meta->social_website);
	if (meta->tip_url)
		stats_set(handle, "icy2-tip-url", meta->tip_url);
	if (meta->chat_url)
		stats_set(handle, "icy2-chat-url", meta->chat_url);
	if (meta->emoji)
		stats_set(handle, "icy2-emoji", meta->emoji);
	if (meta->hashtags)
		stats_set(handle, "icy2-hashtags", meta->hashtags);

	/* Listener Engagement (v2.2) */
	if (meta->listener_peak)
		stats_set(handle, "icy2-listener-peak", meta->listener_peak);
	if (meta->listener_count)
		stats_set(handle, "icy2-listener-count", meta->listener_count);
	if (meta->like_count)
		stats_set(handle, "icy2-like-count", meta->like_count);
	if (meta->share_url)
		stats_set(handle, "icy2-share-url", meta->share_url);
	if (meta->request_url)
		stats_set(handle, "icy2-request-url", meta->request_url);

	/* Broadcast Distribution (v2.2) */
	if (meta->cdn_region)
		stats_set(handle, "icy2-cdn-region", meta->cdn_region);
	if (meta->relay_origin)
		stats_set(handle, "icy2-relay-origin", meta->relay_origin);
	if (meta->stream_failover)
		stats_set(handle, "icy2-stream-failover", meta->stream_failover);
	if (meta->stream_quality_tier)
		stats_set(handle, "icy2-stream-quality-tier", meta->stream_quality_tier);

	/* Station Notices (v2.2) */
	if (meta->notice_board)
		stats_set(handle, "icy2-notice-board", meta->notice_board);
	if (meta->upcoming_show)
		stats_set(handle, "icy2-upcoming-show", meta->upcoming_show);
	if (meta->emergency_alert)
		stats_set(handle, "icy2-emergency-alert", meta->emergency_alert);

	/* Access control */
	if (meta->auth_token)
		stats_set(handle, "icy2-auth-token", meta->auth_token);
	if (meta->nsfw >= 0)
		stats_set_args(handle, "icy2-nsfw", "%d", meta->nsfw);
	if (meta->ai_generated >= 0)
		stats_set_args(handle, "icy2-ai-generated", "%d", meta->ai_generated);
	if (meta->geo_region)
		stats_set(handle, "icy2-geo-region", meta->geo_region);

	/* Compliance (v2.2) */
	if (meta->content_rating)
		stats_set(handle, "icy2-content-rating", meta->content_rating);
	if (meta->parental_advisory)
		stats_set(handle, "icy2-parental-advisory", meta->parental_advisory);
	if (meta->dmca_compliant)
		stats_set(handle, "icy2-dmca-compliant", meta->dmca_compliant);

	/* Verification */
	if (meta->certificate_verify)
		stats_set(handle, "icy2-certificate-verify", meta->certificate_verify);
	if (meta->verification_status)
		stats_set(handle, "icy2-verification-status", meta->verification_status);

	INFO0("ICY2 v2.2 metadata copied to stats");
}

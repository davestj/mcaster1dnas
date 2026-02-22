/* Mcaster1DNAS - Digital Network Audio Server
 * ICY-META v2.2 Protocol Support
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

/* ICY2 Metadata Storage Structure (v2.2) */
typedef struct icy2_metadata_tag {

	/* Protocol Version */
	char *metadata_version;   /* icy-meta-version / icy-metadata-version */

	/* Core Metadata (ICY 1.x compatible) */
	char *station_id;         /* icy-meta-station-id */
	char *station_name;       /* icy-meta-name / icy-name */
	char *station_url;        /* icy-meta-url / icy-url */
	char *station_genre;      /* icy-meta-genre / icy-genre */
	int   station_bitrate;    /* icy-meta-br / icy-br */
	int   station_public;     /* icy-meta-pub / icy-pub */

	/* Station Identity (v2.2) */
	char *station_slogan;     /* icy-meta-station-slogan */
	char *station_logo;       /* icy-meta-station-logo (URL) */
	char *station_founded;    /* icy-meta-station-founded */
	char *station_type;       /* icy-meta-station-type (internet, terrestrial, etc.) */
	char *station_country;    /* icy-meta-station-country */

	/* Programming / Show (v2.2) */
	char *show_title;         /* icy-meta-show-title */
	char *show_episode;       /* icy-meta-show-episode */
	char *show_season;        /* icy-meta-show-season */
	char *show_start_time;    /* icy-meta-show-start-time (ISO 8601) */
	char *show_end_time;      /* icy-meta-show-end-time (ISO 8601) */
	char *playlist_name;      /* icy-meta-playlist-name */
	int   autodj;             /* icy-meta-autodj (0/1) */
	char *stream_session_id;  /* icy-meta-stream-session-id */

	/* DJ / Host */
	char *dj_handle;          /* icy-meta-dj-handle / icy-dj-handle */
	char *dj_bio;             /* icy-meta-dj-bio (v2.2) */
	char *dj_showrating;      /* icy-meta-dj-showrating (v2.2) */
	char *dj_photo;           /* icy-meta-dj-photo URL (v2.2) */

	/* Track Metadata (v2.2) */
	char *track_artist;       /* icy-meta-track-artist */
	char *track_album;        /* icy-meta-track-album */
	char *track_title;        /* icy-meta-track-title */
	char *track_year;         /* icy-meta-track-year */
	char *track_isrc;         /* icy-meta-track-isrc */
	char *track_artwork;      /* icy-meta-track-artwork (URL) */
	char *track_buy_url;      /* icy-meta-track-buy-url */
	char *track_label;        /* icy-meta-track-label */
	char *track_bpm;          /* icy-meta-track-bpm */

	/* Podcast Metadata */
	char *podcast_host;       /* icy-meta-podcast-host / icy-podcast-host */
	char *podcast_rss;        /* icy-meta-podcast-rss / icy-podcast-rss */
	char *podcast_episode;    /* icy-meta-podcast-episode / icy-podcast-episode */
	char *podcast_duration;   /* icy-meta-duration / icy-duration */
	char *podcast_language;   /* icy-meta-language / icy-language */
	char *podcast_rating;     /* icy-meta-podcast-rating (v2.2) */

	/* Audio Technical (v2.2) */
	char *audio_codec;        /* icy-meta-audio-codec (AAC, MP3, FLAC, Opus) */
	char *audio_samplerate;   /* icy-meta-audio-samplerate */
	char *audio_channels;     /* icy-meta-audio-channels */
	char *audio_quality;      /* icy-meta-audio-quality */
	int   audio_lossless;     /* icy-meta-audio-lossless (0/1) */

	/* Video Metadata */
	char *video_type;         /* icy-meta-video-type / icy-video-type */
	char *video_link;         /* icy-meta-video-link / icy-video-link */
	char *video_title;        /* icy-meta-video-title / icy-video-title */
	char *video_platform;     /* icy-meta-video-platform / icy-video-platform */
	char *video_resolution;   /* icy-meta-video-resolution / icy-video-resolution */
	char *video_rating;       /* icy-meta-video-rating (v2.2) */
	char *video_codec;        /* icy-meta-video-codec (v2.2) */
	char *video_framerate;    /* icy-meta-video-framerate (v2.2) */
	char *video_bitrate;      /* icy-meta-video-bitrate (v2.2) */
	char *video_aspect;       /* icy-meta-video-aspect (v2.2) */
	char *video_hdr;          /* icy-meta-video-hdr (v2.2) */
	char *video_thumbnail;    /* icy-meta-video-thumbnail URL (v2.2) */
	char *video_duration;     /* icy-meta-video-duration (v2.2) */

	/* Social Media */
	char *social_twitter;     /* icy-meta-social-twitter / icy-social-twitter */
	char *social_instagram;   /* icy-meta-social-ig / icy-social-ig */
	char *social_tiktok;      /* icy-meta-social-tiktok / icy-social-tiktok */
	char *social_facebook;    /* icy-meta-social-facebook (v2.2) */
	char *social_youtube;     /* icy-meta-social-youtube (v2.2) */
	char *social_bluesky;     /* icy-meta-social-bluesky (v2.2) */
	char *social_website;     /* icy-meta-social-website (v2.2) */
	char *tip_url;            /* icy-meta-tip-url (v2.2) */
	char *chat_url;           /* icy-meta-chat-url (v2.2) */
	char *emoji;              /* icy-meta-emoji / icy-emoji */
	char *hashtags;           /* icy-meta-hashtags / icy-hashtags */

	/* Listener Engagement (v2.2) */
	char *listener_peak;      /* icy-meta-listener-peak */
	char *listener_count;     /* icy-meta-listener-count */
	char *like_count;         /* icy-meta-like-count */
	char *share_url;          /* icy-meta-share-url */
	char *request_url;        /* icy-meta-request-url */

	/* Broadcast Distribution (v2.2) */
	char *cdn_region;         /* icy-meta-cdn-region */
	char *relay_origin;       /* icy-meta-relay-origin */
	char *stream_failover;    /* icy-meta-stream-failover */
	char *stream_quality_tier;/* icy-meta-stream-quality-tier */

	/* Station Notices (v2.2) */
	char *notice_board;       /* icy-meta-notice-board */
	char *upcoming_show;      /* icy-meta-upcoming-show */
	char *emergency_alert;    /* icy-meta-emergency-alert */

	/* Access Control */
	char *auth_token;         /* icy-meta-auth-token / icy-auth-token */
	int   nsfw;               /* icy-meta-nsfw / icy-nsfw (0/1) */
	int   ai_generated;       /* icy-meta-ai-generated / icy-ai-generated (0/1) */
	char *geo_region;         /* icy-meta-geo-region / icy-geo-region */

	/* Compliance (v2.2) */
	char *content_rating;     /* icy-meta-content-rating */
	char *parental_advisory;  /* icy-meta-parental-advisory */
	char *dmca_compliant;     /* icy-meta-dmca-compliant */

	/* Verification */
	char *certificate_verify; /* icy-meta-certificate-verify / icy-certificate-verify */
	char *verification_status;/* icy-meta-verification-status / icy-verification-status */

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

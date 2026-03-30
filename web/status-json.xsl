<?xml version="1.0" encoding="UTF-8"?>
<!--
  status-json.xsl - Mcaster1DNAS JSON Status API
  Transforms /mcaster1stats XML into a complete JSON response.
  Covers all codec types: MP3, AACP, Ogg Vorbis, Opus
  Fields: all available from mcaster1stats XML API
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="text" encoding="UTF-8" media-type="application/json"/>

<!-- Recursive find-and-replace for string escaping -->
<xsl:template name="str-replace">
  <xsl:param name="str"/>
  <xsl:param name="find"/>
  <xsl:param name="replace"/>
  <xsl:choose>
    <xsl:when test="contains($str, $find)">
      <xsl:value-of select="substring-before($str, $find)"/>
      <xsl:value-of select="$replace"/>
      <xsl:call-template name="str-replace">
        <xsl:with-param name="str" select="substring-after($str, $find)"/>
        <xsl:with-param name="find" select="$find"/>
        <xsl:with-param name="replace" select="$replace"/>
      </xsl:call-template>
    </xsl:when>
    <xsl:otherwise><xsl:value-of select="$str"/></xsl:otherwise>
  </xsl:choose>
</xsl:template>

<!-- Escape a string value for safe JSON output -->
<!-- Handles: backslash, double-quote, tab, newline, carriage-return -->
<xsl:template name="json-string">
  <xsl:param name="str"/>
  <!-- Outermost: escape tab (&#9;) → \t -->
  <xsl:call-template name="str-replace">
    <xsl:with-param name="str">
      <!-- escape newline (&#10;) → \n -->
      <xsl:call-template name="str-replace">
        <xsl:with-param name="str">
          <!-- escape carriage-return (&#13;) → \r -->
          <xsl:call-template name="str-replace">
            <xsl:with-param name="str">
              <!-- escape double-quote -->
              <xsl:call-template name="str-replace">
                <xsl:with-param name="str">
                  <!-- escape backslash first -->
                  <xsl:call-template name="str-replace">
                    <xsl:with-param name="str" select="$str"/>
                    <xsl:with-param name="find" select="'\'"/>
                    <xsl:with-param name="replace" select="'\\'"/>
                  </xsl:call-template>
                </xsl:with-param>
                <xsl:with-param name="find" select="'&quot;'"/>
                <xsl:with-param name="replace" select="'\&quot;'"/>
              </xsl:call-template>
            </xsl:with-param>
            <xsl:with-param name="find" select="'&#13;'"/>
            <xsl:with-param name="replace" select="'\r'"/>
          </xsl:call-template>
        </xsl:with-param>
        <xsl:with-param name="find" select="'&#10;'"/>
        <xsl:with-param name="replace" select="'\n'"/>
      </xsl:call-template>
    </xsl:with-param>
    <xsl:with-param name="find" select="'&#9;'"/>
    <xsl:with-param name="replace" select="'\t'"/>
  </xsl:call-template>
</xsl:template>

<!-- Output numeric value or 0 if absent/empty -->
<xsl:template name="num">
  <xsl:param name="v"/>
  <xsl:choose>
    <xsl:when test="string($v) != ''"><xsl:value-of select="$v"/></xsl:when>
    <xsl:otherwise>0</xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="/mcaster1stats">
{
  "mcaster1stats": {
    "admin": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="admin"/></xsl:call-template>",
    "host": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="host"/></xsl:call-template>",
    "location": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="location"/></xsl:call-template>",
    "server_id": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="server_id"/></xsl:call-template>",
    "server_start": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="server_start"/></xsl:call-template>",
    "build": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="build"/></xsl:call-template>",
    "sources": <xsl:value-of select="sources"/>,
    "clients": <xsl:value-of select="clients"/>,
    "listeners": <xsl:value-of select="listeners"/>,
    "connections": <xsl:value-of select="connections"/>,
    "client_connections": <xsl:value-of select="client_connections"/>,
    "source_client_connections": <xsl:value-of select="source_client_connections"/>,
    "source_relay_connections": <xsl:value-of select="source_relay_connections"/>,
    "source_total_connections": <xsl:value-of select="source_total_connections"/>,
    "listener_connections": <xsl:value-of select="listener_connections"/>,
    "file_connections": <xsl:value-of select="file_connections"/>,
    "stats_connections": <xsl:value-of select="stats_connections"/>,
    "stats": <xsl:value-of select="stats"/>,
    "banned_IPs": <xsl:value-of select="banned_IPs"/>,
    "outgoing_kbitrate": <xsl:value-of select="outgoing_kbitrate"/>,
    "stream_kbytes_read": <xsl:value-of select="stream_kbytes_read"/>,
    "stream_kbytes_sent": <xsl:value-of select="stream_kbytes_sent"/><xsl:if test="source">,
    "source": [<xsl:for-each select="source">
      {
        "mount": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="@mount"/></xsl:call-template>",
        "server_name": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="server_name"/></xsl:call-template>",
        "server_description": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="server_description"/></xsl:call-template>",
        "server_type": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="server_type"/></xsl:call-template>",
        "server_url": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="server_url"/></xsl:call-template>",
        "listenurl": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="listenurl"/></xsl:call-template>",
        "genre": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="genre"/></xsl:call-template>",
        "title": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="title"/></xsl:call-template>",
        "artist": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="artist"/></xsl:call-template>",
        "yp_currently_playing": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="yp_currently_playing"/></xsl:call-template>",
        "source_ip": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="source_ip"/></xsl:call-template>",
        "stream_start": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="stream_start"/></xsl:call-template>",
        "metadata_updated": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="metadata_updated"/></xsl:call-template>",
        "max_listeners": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="max_listeners"/></xsl:call-template>",
        "subtype": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="subtype"/></xsl:call-template>",
        "public": <xsl:choose><xsl:when test="public = '1'">true</xsl:when><xsl:otherwise>false</xsl:otherwise></xsl:choose>,
        "bitrate": <xsl:choose><xsl:when test="bitrate"><xsl:value-of select="bitrate"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "audio_codecid": <xsl:choose><xsl:when test="audio_codecid"><xsl:value-of select="audio_codecid"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "mpeg_samplerate": <xsl:choose><xsl:when test="mpeg_samplerate"><xsl:value-of select="mpeg_samplerate"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "mpeg_channels": <xsl:choose><xsl:when test="mpeg_channels"><xsl:value-of select="mpeg_channels"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "audio_bitrate": <xsl:choose><xsl:when test="audio_bitrate"><xsl:value-of select="audio_bitrate"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "audio_samplerate": <xsl:choose><xsl:when test="audio_samplerate"><xsl:value-of select="audio_samplerate"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "audio_channels": <xsl:choose><xsl:when test="audio_channels"><xsl:value-of select="audio_channels"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "ice_bitrate": <xsl:choose><xsl:when test="*[local-name()='ice-bitrate']"><xsl:value-of select="*[local-name()='ice-bitrate']"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "listeners": <xsl:choose><xsl:when test="listeners"><xsl:value-of select="listeners"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "listener_peak": <xsl:choose><xsl:when test="listener_peak"><xsl:value-of select="listener_peak"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "listener_connections": <xsl:choose><xsl:when test="listener_connections"><xsl:value-of select="listener_connections"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "slow_listeners": <xsl:choose><xsl:when test="slow_listeners"><xsl:value-of select="slow_listeners"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "connected": <xsl:choose><xsl:when test="connected"><xsl:value-of select="connected"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "incoming_bitrate": <xsl:choose><xsl:when test="incoming_bitrate"><xsl:value-of select="incoming_bitrate"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "outgoing_kbitrate": <xsl:choose><xsl:when test="outgoing_kbitrate"><xsl:value-of select="outgoing_kbitrate"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "queue_size": <xsl:choose><xsl:when test="queue_size"><xsl:value-of select="queue_size"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "total_bytes_read": <xsl:choose><xsl:when test="total_bytes_read"><xsl:value-of select="total_bytes_read"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "total_bytes_sent": <xsl:choose><xsl:when test="total_bytes_sent"><xsl:value-of select="total_bytes_sent"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>,
        "total_mbytes_sent": <xsl:choose><xsl:when test="total_mbytes_sent"><xsl:value-of select="total_mbytes_sent"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose><xsl:if test="string(mount_type) != ''">,
        "mount_type": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="mount_type"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-version']) != ''">,
        "icy2-version": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-version']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-station-id']) != ''">,
        "icy2-station-id": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-station-id']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-station-slogan']) != ''">,
        "icy2-station-slogan": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-station-slogan']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-station-logo']) != ''">,
        "icy2-station-logo": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-station-logo']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-station-country']) != ''">,
        "icy2-station-country": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-station-country']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-station-type']) != ''">,
        "icy2-station-type": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-station-type']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-show-title']) != ''">,
        "icy2-show-title": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-show-title']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-show-episode']) != ''">,
        "icy2-show-episode": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-show-episode']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-show-season']) != ''">,
        "icy2-show-season": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-show-season']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-show-start-time']) != ''">,
        "icy2-show-start-time": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-show-start-time']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-show-end-time']) != ''">,
        "icy2-show-end-time": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-show-end-time']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-playlist-name']) != ''">,
        "icy2-playlist-name": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-playlist-name']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-autodj']) != ''">,
        "icy2-autodj": <xsl:value-of select="*[local-name()='icy2-autodj']"/></xsl:if><xsl:if test="string(*[local-name()='icy2-stream-session-id']) != ''">,
        "icy2-stream-session-id": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-stream-session-id']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-dj-handle']) != ''">,
        "icy2-dj-handle": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-dj-handle']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-dj-bio']) != ''">,
        "icy2-dj-bio": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-dj-bio']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-dj-showrating']) != ''">,
        "icy2-dj-showrating": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-dj-showrating']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-dj-photo']) != ''">,
        "icy2-dj-photo": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-dj-photo']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-track-artist']) != ''">,
        "icy2-track-artist": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-track-artist']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-track-album']) != ''">,
        "icy2-track-album": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-track-album']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-track-title']) != ''">,
        "icy2-track-title": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-track-title']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-track-year']) != ''">,
        "icy2-track-year": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-track-year']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-track-isrc']) != ''">,
        "icy2-track-isrc": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-track-isrc']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-track-artwork']) != ''">,
        "icy2-track-artwork": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-track-artwork']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-track-buy-url']) != ''">,
        "icy2-track-buy-url": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-track-buy-url']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-track-label']) != ''">,
        "icy2-track-label": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-track-label']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-track-bpm']) != ''">,
        "icy2-track-bpm": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-track-bpm']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-podcast-host']) != ''">,
        "icy2-podcast-host": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-podcast-host']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-podcast-rss']) != ''">,
        "icy2-podcast-rss": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-podcast-rss']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-podcast-episode']) != ''">,
        "icy2-podcast-episode": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-podcast-episode']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-duration']) != ''">,
        "icy2-duration": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-duration']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-language']) != ''">,
        "icy2-language": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-language']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-podcast-rating']) != ''">,
        "icy2-podcast-rating": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-podcast-rating']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-audio-codec']) != ''">,
        "icy2-audio-codec": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-audio-codec']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-audio-samplerate']) != ''">,
        "icy2-audio-samplerate": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-audio-samplerate']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-audio-channels']) != ''">,
        "icy2-audio-channels": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-audio-channels']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-audio-quality']) != ''">,
        "icy2-audio-quality": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-audio-quality']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-audio-lossless']) != ''">,
        "icy2-audio-lossless": <xsl:value-of select="*[local-name()='icy2-audio-lossless']"/></xsl:if><xsl:if test="string(*[local-name()='icy2-video-type']) != ''">,
        "icy2-video-type": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-video-type']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-video-link']) != ''">,
        "icy2-video-link": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-video-link']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-video-title']) != ''">,
        "icy2-video-title": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-video-title']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-video-platform']) != ''">,
        "icy2-video-platform": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-video-platform']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-video-resolution']) != ''">,
        "icy2-video-resolution": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-video-resolution']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-video-thumbnail']) != ''">,
        "icy2-video-thumbnail": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-video-thumbnail']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-video-duration']) != ''">,
        "icy2-video-duration": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-video-duration']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-social-twitter']) != ''">,
        "icy2-social-twitter": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-social-twitter']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-social-instagram']) != ''">,
        "icy2-social-instagram": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-social-instagram']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-social-tiktok']) != ''">,
        "icy2-social-tiktok": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-social-tiktok']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-social-facebook']) != ''">,
        "icy2-social-facebook": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-social-facebook']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-social-youtube']) != ''">,
        "icy2-social-youtube": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-social-youtube']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-social-bluesky']) != ''">,
        "icy2-social-bluesky": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-social-bluesky']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-social-website']) != ''">,
        "icy2-social-website": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-social-website']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-tip-url']) != ''">,
        "icy2-tip-url": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-tip-url']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-chat-url']) != ''">,
        "icy2-chat-url": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-chat-url']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-share-url']) != ''">,
        "icy2-share-url": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-share-url']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-request-url']) != ''">,
        "icy2-request-url": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-request-url']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-emoji']) != ''">,
        "icy2-emoji": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-emoji']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-hashtags']) != ''">,
        "icy2-hashtags": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-hashtags']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-notice-board']) != ''">,
        "icy2-notice-board": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-notice-board']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-upcoming-show']) != ''">,
        "icy2-upcoming-show": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-upcoming-show']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-emergency-alert']) != ''">,
        "icy2-emergency-alert": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-emergency-alert']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-cdn-region']) != ''">,
        "icy2-cdn-region": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-cdn-region']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-relay-origin']) != ''">,
        "icy2-relay-origin": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-relay-origin']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-stream-quality-tier']) != ''">,
        "icy2-stream-quality-tier": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-stream-quality-tier']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-content-rating']) != ''">,
        "icy2-content-rating": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-content-rating']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-parental-advisory']) != ''">,
        "icy2-parental-advisory": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-parental-advisory']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-dmca-compliant']) != ''">,
        "icy2-dmca-compliant": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-dmca-compliant']"/></xsl:call-template>"</xsl:if><xsl:if test="string(*[local-name()='icy2-verification-status']) != ''">,
        "icy2-verification-status": "<xsl:call-template name="json-string"><xsl:with-param name="str" select="*[local-name()='icy2-verification-status']"/></xsl:call-template>"</xsl:if>
      }<xsl:if test="position() != last()">,</xsl:if>
    </xsl:for-each>]</xsl:if>
  }
}
</xsl:template>
</xsl:stylesheet>

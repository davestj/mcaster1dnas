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
<xsl:template name="json-string">
  <xsl:param name="str"/>
  <xsl:call-template name="str-replace">
    <xsl:with-param name="str">
      <xsl:call-template name="str-replace">
        <xsl:with-param name="str" select="$str"/>
        <xsl:with-param name="find" select="'\'"/>
        <xsl:with-param name="replace" select="'\\'"/>
      </xsl:call-template>
    </xsl:with-param>
    <xsl:with-param name="find" select="'&quot;'"/>
    <xsl:with-param name="replace" select="'\&quot;'"/>
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
        "total_mbytes_sent": <xsl:choose><xsl:when test="total_mbytes_sent"><xsl:value-of select="total_mbytes_sent"/></xsl:when><xsl:otherwise>0</xsl:otherwise></xsl:choose>
      }<xsl:if test="position() != last()">,</xsl:if>
    </xsl:for-each>]</xsl:if>
  }
}
</xsl:template>
</xsl:stylesheet>

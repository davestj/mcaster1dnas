<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>

<xsl:template match="/mcaster1stats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Stream Status - Mcaster1DNAS</title>
    <link rel="stylesheet" type="text/css" href="mcaster1-modern.css"/>
</head>
<body>
    <div class="mcaster-main">
        <div class="mcaster-container">

            <!-- Server Stats Overview -->
            <div class="mcaster-card">
                <h2>Server Statistics</h2>
                <div class="stat-grid">
                    <div class="stat-box">
                        <span class="stat-value"><xsl:value-of select="listeners"/></span>
                        <span class="stat-label">Total Listeners</span>
                    </div>
                    <div class="stat-box">
                        <span class="stat-value"><xsl:value-of select="sources"/></span>
                        <span class="stat-label">Active Sources</span>
                    </div>
                    <div class="stat-box">
                        <span class="stat-value"><xsl:value-of select="clients"/></span>
                        <span class="stat-label">Total Clients</span>
                    </div>
                    <div class="stat-box">
                        <span class="stat-value"><xsl:value-of select="connections"/></span>
                        <span class="stat-label">Connections</span>
                    </div>
                </div>
            </div>

            <!-- Active Mount Points -->
            <xsl:for-each select="source">
                <div class="mcaster-card">
                    <h2>
                        <xsl:value-of select="@mount"/>
                        <xsl:if test="listeners">
                            <span class="status-badge status-active">LIVE - <xsl:value-of select="listeners"/> listeners</span>
                        </xsl:if>
                    </h2>

                    <table>
                        <xsl:if test="server_name">
                            <tr>
                                <th style="width: 200px">Stream Title</th>
                                <td><xsl:value-of select="server_name"/></td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="server_description">
                            <tr>
                                <th>Description</th>
                                <td><xsl:value-of select="server_description"/></td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="title">
                            <tr>
                                <th>Now Playing</th>
                                <td>
                                    <strong><xsl:if test="artist"><xsl:value-of select="artist"/> - </xsl:if><xsl:value-of select="title"/></strong>
                                </td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="server_type">
                            <tr>
                                <th>Content Type</th>
                                <td><xsl:value-of select="server_type"/></td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="bitrate">
                            <tr>
                                <th>Bitrate</th>
                                <td><xsl:value-of select="bitrate"/> kbps</td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="quality">
                            <tr>
                                <th>Quality</th>
                                <td><xsl:value-of select="quality"/></td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="genre">
                            <tr>
                                <th>Genre</th>
                                <td><xsl:value-of select="genre"/></td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="listener_peak">
                            <tr>
                                <th>Peak Listeners</th>
                                <td><xsl:value-of select="listener_peak"/></td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="stream_start">
                            <tr>
                                <th>Stream Started</th>
                                <td><xsl:value-of select="stream_start"/></td>
                            </tr>
                        </xsl:if>

                        <xsl:if test="server_url">
                            <tr>
                                <th>Stream URL</th>
                                <td><a href="{server_url}" target="_blank"><xsl:value-of select="server_url"/></a></td>
                            </tr>
                        </xsl:if>
                    </table>

                    <div style="margin-top: 1rem;">
                        <xsl:choose>
                            <xsl:when test="authenticator">
                                <a href="./auth.xsl" class="btn btn-primary">Login to Listen</a>
                            </xsl:when>
                            <xsl:otherwise>
                                <a href=".{@mount}.m3u" class="btn btn-primary">Listen (M3U)</a>
                                <a href=".{@mount}.xspf" class="btn btn-secondary">Listen (XSPF)</a>
                            </xsl:otherwise>
                        </xsl:choose>
                    </div>
                </div>
            </xsl:for-each>

            <!-- No Active Streams Message -->
            <xsl:if test="not(source)">
                <div class="mcaster-card text-center">
                    <h2>No Active Streams</h2>
                    <p>There are currently no active broadcasts. Check back later!</p>
                </div>
            </xsl:if>

        </div>
    </div>

    <div class="mcaster-footer">
        <div class="mcaster-container">
            <p>Powered by <a href="https://mcaster1.com">Mcaster1DNAS</a> - Digital Network Audio Server</p>
        </div>
    </div>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

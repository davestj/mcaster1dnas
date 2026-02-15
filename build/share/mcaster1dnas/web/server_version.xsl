<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>

<xsl:template match="/mcaster1stats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Server Information - Mcaster1DNAS</title>
    <link rel="stylesheet" type="text/css" href="mcaster1-modern.css"/>
</head>
<body>
    <div class="mcaster-main">
        <div class="mcaster-container">

            <div class="mcaster-card">
                <h2>Mcaster1DNAS Server Information</h2>

                <table>
                    <tr>
                        <th style="width: 250px">Server ID</th>
                        <td><strong><xsl:value-of select="server_id"/></strong></td>
                    </tr>
                    <tr>
                        <th>Build Number</th>
                        <td class="font-mono"><xsl:value-of select="build"/></td>
                    </tr>
                    <tr>
                        <th>Server Started</th>
                        <td><xsl:value-of select="server_start"/></td>
                    </tr>
                    <tr>
                        <th>Hostname</th>
                        <td><xsl:value-of select="host"/></td>
                    </tr>
                    <tr>
                        <th>Location</th>
                        <td><xsl:value-of select="location"/></td>
                    </tr>
                    <tr>
                        <th>Administrator</th>
                        <td><a href="mailto:{admin}"><xsl:value-of select="admin"/></a></td>
                    </tr>
                </table>
            </div>

            <div class="mcaster-card">
                <h2>Server Statistics</h2>

                <div class="stat-grid">
                    <div class="stat-box">
                        <span class="stat-value"><xsl:value-of select="listeners"/></span>
                        <span class="stat-label">Current Listeners</span>
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
                        <span class="stat-label">Total Connections</span>
                    </div>
                </div>

                <table style="margin-top: 1.5rem">
                    <tr>
                        <th style="width: 250px">Client Connections</th>
                        <td><xsl:value-of select="client_connections"/></td>
                    </tr>
                    <tr>
                        <th>Source Connections</th>
                        <td><xsl:value-of select="source_client_connections"/></td>
                    </tr>
                    <tr>
                        <th>Listener Connections</th>
                        <td><xsl:value-of select="listener_connections"/></td>
                    </tr>
                    <tr>
                        <th>File Connections</th>
                        <td><xsl:value-of select="file_connections"/></td>
                    </tr>
                    <tr>
                        <th>Stats Connections</th>
                        <td><xsl:value-of select="stats_connections"/></td>
                    </tr>
                </table>
            </div>

            <div class="mcaster-card">
                <h2>Bandwidth Statistics</h2>

                <table>
                    <tr>
                        <th style="width: 250px">Outgoing Bitrate</th>
                        <td><xsl:value-of select="outgoing_kbitrate"/> kbps</td>
                    </tr>
                    <tr>
                        <th>Stream Data Read</th>
                        <td><xsl:value-of select="stream_kbytes_read"/> KB</td>
                    </tr>
                    <tr>
                        <th>Stream Data Sent</th>
                        <td><xsl:value-of select="stream_kbytes_sent"/> KB</td>
                    </tr>
                    <xsl:if test="banned_IPs">
                        <tr>
                            <th>Banned IPs</th>
                            <td><xsl:value-of select="banned_IPs"/></td>
                        </tr>
                    </xsl:if>
                </table>
            </div>

        </div>
    </div>

    <div class="mcaster-footer">
        <div class="mcaster-container">
            <p><i class="fas fa-server"></i> Powered by <a href="https://mcaster1.com">Mcaster1DNAS</a> - Digital Network Audio Server</p>
        </div>
    </div>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

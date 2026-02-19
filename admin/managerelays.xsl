<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>
<xsl:include href="header.xsl"/>
<xsl:include href="footer.xsl"/>

<xsl:template match="/icerelaystats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Manage Relays - Mcaster1DNAS Admin</title>

    <!-- FontAwesome 6.x for professional icons -->
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css"
          integrity="sha512-iecdLmaskl7CVkqkXNQ/ZH/XLlvWZOJyj7Yy7tcenmpD1ypASozpmT/E0iPtmFIB46ZmdtAc9eNBvH0H/ZpiBw=="
          crossorigin="anonymous" referrerpolicy="no-referrer"/>

    <link rel="stylesheet" type="text/css" href="/style.css"/>
    <link rel="icon" type="image/x-icon" href="/favicon.ico"/>
    <link rel="icon" type="image/png" sizes="32x32" href="/favicon-32x32.png"/>
    <link rel="icon" type="image/png" sizes="16x16" href="/favicon-16x16.png"/>
    <link rel="apple-touch-icon" sizes="180x180" href="/apple-touch-icon.png"/>
    <script src="/mcaster-utils.js"></script>
</head>
<body>
    <xsl:call-template name="admin-header">
        <xsl:with-param name="active-page" select="'relays'"/>
    </xsl:call-template>

    <div class="mcaster-main">
        <div class="mcaster-container">

            <xsl:for-each select="relay">
                <div class="mcaster-card">
                    <h2>
                        <i class="fas fa-project-diagram"></i> Relay: <xsl:value-of select="localmount"/>
                        <xsl:choose>
                            <xsl:when test="enable!='0'">
                                <a href="managerelays.xsl?relay={localmount}&amp;enable=0" class="btn btn-secondary" style="font-size: 0.875rem; margin-left: 1rem;">
                                    <i class="fas fa-pause"></i> Disable
                                </a>
                            </xsl:when>
                            <xsl:otherwise>
                                <a href="managerelays.xsl?relay={localmount}&amp;enable=1" class="btn btn-primary" style="font-size: 0.875rem; margin-left: 1rem;">
                                    <i class="fas fa-play"></i> Enable
                                </a>
                            </xsl:otherwise>
                        </xsl:choose>
                    </h2>

                    <!-- Relay Status -->
                    <div style="margin-bottom: 1.5rem;">
                        <xsl:choose>
                            <xsl:when test="enable">
                                <span class="status-badge status-active">
                                    <i class="fas fa-check-circle"></i> Enabled
                                </span>
                            </xsl:when>
                            <xsl:otherwise>
                                <span class="status-badge status-inactive">
                                    <i class="fas fa-times-circle"></i> Disabled
                                </span>
                            </xsl:otherwise>
                        </xsl:choose>
                        <xsl:if test="on_demand=1">
                            <span class="status-badge status-warning">
                                <i class="fas fa-clock"></i> On Demand
                            </span>
                        </xsl:if>
                        <xsl:if test="from_master=1">
                            <span class="status-badge status-active">
                                <i class="fas fa-server"></i> Slave Relay
                            </span>
                        </xsl:if>
                        <xsl:if test="run_on &gt; 0">
                            <span class="status-badge" style="background: #dbeafe; color: #1e40af;">
                                <i class="fas fa-hourglass-half"></i> Run on for <xsl:value-of select="run_on"/>s
                            </span>
                        </xsl:if>
                    </div>

                    <!-- Master Servers -->
                    <h3><i class="fas fa-server"></i> Master Servers</h3>
                    <table>
                        <thead>
                            <tr>
                                <th style="width: 100px;">Priority</th>
                                <th>Server</th>
                                <th>Port</th>
                                <th>Mount</th>
                            </tr>
                        </thead>
                        <tbody>
                            <xsl:for-each select="master">
                                <tr>
                                    <xsl:if test="active">
                                        <xsl:attribute name="style">background: #dcfce7;</xsl:attribute>
                                    </xsl:if>
                                    <td style="text-align: center;">
                                        <xsl:if test="active">
                                            <i class="fas fa-check-circle" style="color: var(--dnas-green); margin-right: 0.5rem;"></i>
                                        </xsl:if>
                                        <xsl:value-of select="priority"/>
                                    </td>
                                    <td><xsl:value-of select="server"/></td>
                                    <td><xsl:value-of select="port"/></td>
                                    <td><xsl:value-of select="mount"/></td>
                                </tr>
                            </xsl:for-each>
                        </tbody>
                    </table>
                </div>
            </xsl:for-each>

            <xsl:if test="not(relay)">
                <div class="mcaster-card text-center">
                    <h2><i class="fas fa-info-circle"></i> No Relays Configured</h2>
                    <p>There are currently no relays configured on this server.</p>
                </div>
            </xsl:if>

        </div>
    </div>

    <xsl:call-template name="admin-footer"/>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

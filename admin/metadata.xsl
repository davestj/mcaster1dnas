<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>
<xsl:include href="header.xsl"/>
<xsl:include href="footer.xsl"/>

<xsl:template match="/iceresponse">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Metadata Update Result - Mcaster1DNAS Admin</title>

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
        <xsl:with-param name="active-page" select="''"/>
    </xsl:call-template>

    <div class="mcaster-main">
        <div class="mcaster-container">

            <div class="mcaster-card">

                <!-- Title and status banner -->
                <xsl:choose>
                    <xsl:when test="return = '1'">
                        <h2 style="color: #16a34a;">
                            <i class="fas fa-check-circle"></i> Metadata Update Successful
                        </h2>
                        <div style="background: rgba(22,163,74,0.08); border: 1px solid rgba(22,163,74,0.3);
                                    border-radius: var(--radius-md); padding: 1rem 1.25rem; margin: 1rem 0;
                                    color: #166534; font-size: 0.95rem;">
                            <i class="fas fa-info-circle"></i>
                            <xsl:text> </xsl:text>
                            <xsl:value-of select="message"/>
                            <xsl:if test="mount">
                                <xsl:text> — mount: </xsl:text>
                                <strong><xsl:value-of select="mount"/></strong>
                            </xsl:if>
                        </div>
                    </xsl:when>
                    <xsl:otherwise>
                        <h2 style="color: #dc2626;">
                            <i class="fas fa-times-circle"></i> Metadata Update Failed
                        </h2>
                        <div style="background: rgba(220,38,38,0.08); border: 1px solid rgba(220,38,38,0.3);
                                    border-radius: var(--radius-md); padding: 1rem 1.25rem; margin: 1rem 0;
                                    color: #7f1d1d; font-size: 0.95rem;">
                            <i class="fas fa-exclamation-triangle"></i>
                            <xsl:text> </xsl:text>
                            <xsl:value-of select="message"/>
                        </div>
                    </xsl:otherwise>
                </xsl:choose>

                <!-- ICY2 updated fields table (only shown on success with fields present) -->
                <xsl:if test="return = '1' and updated_fields/field">
                    <div style="margin-top: 1.5rem;">
                        <h3 style="color: #0891b2; font-size: 0.85rem; text-transform: uppercase;
                                   letter-spacing: 0.08em; margin-bottom: 0.75rem;">
                            <i class="fas fa-satellite-dish"></i> ICY2 v2.2 — Fields Updated
                        </h3>
                        <div style="overflow-x: auto;">
                            <table class="mcaster-table" style="width: 100%; font-size: 0.88rem;">
                                <thead>
                                    <tr>
                                        <th style="text-align: left; padding: 0.5rem 0.75rem;">HTTP Parameter</th>
                                        <th style="text-align: left; padding: 0.5rem 0.75rem;">Stats Key</th>
                                        <th style="text-align: left; padding: 0.5rem 0.75rem;">Value</th>
                                    </tr>
                                </thead>
                                <tbody>
                                    <xsl:for-each select="updated_fields/field">
                                        <tr>
                                            <td style="padding: 0.45rem 0.75rem; font-family: var(--font-mono); color: #0891b2;">
                                                <xsl:value-of select="@param"/>
                                            </td>
                                            <td style="padding: 0.45rem 0.75rem; font-family: var(--font-mono); color: var(--text-secondary);">
                                                <xsl:value-of select="@stat"/>
                                            </td>
                                            <td style="padding: 0.45rem 0.75rem; color: var(--text-primary); word-break: break-all;">
                                                <xsl:value-of select="value"/>
                                            </td>
                                        </tr>
                                    </xsl:for-each>
                                </tbody>
                            </table>
                        </div>
                    </div>
                </xsl:if>

                <!-- Action buttons -->
                <div style="margin-top: 1.75rem; display: flex; gap: 0.75rem; flex-wrap: wrap;">
                    <xsl:choose>
                        <xsl:when test="mount">
                            <a class="btn btn-primary"
                               href="updatemetadata.xsl?mount={mount}">
                                <i class="fas fa-edit"></i> Update More Metadata
                            </a>
                            <a class="btn btn-secondary"
                               href="stats.xsl?mount={mount}">
                                <i class="fas fa-chart-bar"></i> View Mount Stats
                            </a>
                        </xsl:when>
                        <xsl:otherwise>
                            <a href="stats.xsl" class="btn btn-primary">
                                <i class="fas fa-chart-bar"></i> Back to Statistics
                            </a>
                        </xsl:otherwise>
                    </xsl:choose>
                </div>

            </div>

        </div>
    </div>

    <xsl:call-template name="admin-footer"/>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

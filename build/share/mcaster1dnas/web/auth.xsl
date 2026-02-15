<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>

<xsl:template match="/mcaster1stats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Login - Mcaster1DNAS</title>

    <!-- FontAwesome 6.x for professional icons -->
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css"
          integrity="sha512-iecdLmaskl7CVkqkXNQ/ZH/XLlvWZOJyj7Yy7tcenmpD1ypASozpmT/E0iPtmFIB46ZmdtAc9eNBvH0H/ZpiBw=="
          crossorigin="anonymous" referrerpolicy="no-referrer"/>

    <link rel="stylesheet" type="text/css" href="mcaster1-modern.css"/>
</head>
<body>
    <div class="mcaster-header">
        <div class="mcaster-container">
            <div class="mcaster-brand">
                <div class="brand-icon">M1</div>
                <div class="brand-text">
                    <h1 style="margin: 0; font-size: 1.75rem;">
                        <span class="brand-mcaster">Mcaster1</span>
                        <span class="brand-dnas">DNAS</span>
                    </h1>
                </div>
            </div>
            <div class="mcaster-nav">
                <a href="status.xsl"><i class="fas fa-home"></i> Status</a>
                <a href="server_version.xsl"><i class="fas fa-info-circle"></i> Server Info</a>
            </div>
        </div>
    </div>

    <div class="mcaster-main">
        <div class="mcaster-container">

            <xsl:for-each select="source">
                <xsl:if test="authenticator">
                    <div class="mcaster-card" style="max-width: 500px; margin: 2rem auto;">
                        <h2 style="text-align: center;">
                            <i class="fas fa-lock"></i> Authentication Required
                        </h2>

                        <xsl:if test="server_name">
                            <p style="text-align: center; color: var(--text-secondary); margin-bottom: 2rem;">
                                <strong><xsl:value-of select="server_name"/></strong><br/>
                                (<xsl:value-of select="@mount"/>)
                            </p>
                        </xsl:if>

                        <form method="GET" action="/admin/buildm3u">
                            <div style="margin-bottom: 1.5rem;">
                                <label style="display: block; margin-bottom: 0.5rem; font-weight: 600;">
                                    <i class="fas fa-user"></i> Username
                                </label>
                                <input type="text" name="username" required="required" style="width: 100%;"/>
                            </div>

                            <div style="margin-bottom: 2rem;">
                                <label style="display: block; margin-bottom: 0.5rem; font-weight: 600;">
                                    <i class="fas fa-key"></i> Password
                                </label>
                                <input type="password" name="password" required="required" style="width: 100%;"/>
                            </div>

                            <input type="hidden" name="mount" value="{@mount}"/>

                            <button type="submit" class="btn btn-primary" style="width: 100%; padding: 0.75rem; font-size: 1rem;">
                                <i class="fas fa-sign-in-alt"></i> Login
                            </button>
                        </form>

                        <p style="text-align: center; margin-top: 1.5rem; color: var(--text-secondary); font-size: 0.875rem;">
                            <i class="fas fa-info-circle"></i> This stream requires authentication to access.
                        </p>
                    </div>
                </xsl:if>
            </xsl:for-each>

            <xsl:if test="not(source/authenticator)">
                <div class="mcaster-card text-center" style="max-width: 500px; margin: 2rem auto;">
                    <h2><i class="fas fa-check-circle"></i> No Authentication Required</h2>
                    <p>No streams require authentication at this time.</p>
                    <a href="status.xsl" class="btn btn-primary">
                        <i class="fas fa-arrow-left"></i> Back to Status
                    </a>
                </div>
            </xsl:if>

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

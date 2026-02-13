<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>
<xsl:include href="header.xsl"/>
<xsl:include href="footer.xsl"/>

<xsl:template match="/mcaster1stats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Manage Authentication - Mcaster1DNAS Admin</title>

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
        <xsl:with-param name="active-page" select="''"/>
    </xsl:call-template>

    <div class="mcaster-main">
        <div class="mcaster-container">

            <!-- Response Message -->
            <xsl:for-each select="iceresponse">
                <div class="mcaster-card" style="background: #dcfce7; border-left: 4px solid var(--dnas-green);">
                    <p style="margin: 0; color: var(--text-primary);">
                        <i class="fas fa-check-circle" style="color: var(--dnas-green);"></i>
                        <strong>Response:</strong> <xsl:value-of select="message"/>
                    </p>
                </div>
            </xsl:for-each>

            <xsl:for-each select="source">
                <div class="mcaster-card">
                    <h2>
                        <i class="fas fa-key"></i> Authentication Management
                        <xsl:if test="server_name">
                            - <xsl:value-of select="server_name"/>
                        </xsl:if>
                    </h2>
                    <p style="color: var(--text-secondary); margin-bottom: 1.5rem;">
                        Mount Point: <strong><xsl:value-of select="@mount"/></strong>
                    </p>

                    <!-- Admin Actions -->
                    <div style="margin: 1rem 0; padding: 1rem; background: var(--bg-light); border-radius: var(--radius-md);">
                        <strong style="display: block; margin-bottom: 0.5rem;">Admin Actions:</strong>
                        <a href="listclients.xsl?mount={@mount}" class="admin-action-btn">
                            <i class="fas fa-users"></i> List Clients
                        </a>
                        <a href="moveclients.xsl?mount={@mount}" class="admin-action-btn">
                            <i class="fas fa-exchange-alt"></i> Move Listeners
                        </a>
                        <a href="updatemetadata.xsl?mount={@mount}" class="admin-action-btn">
                            <i class="fas fa-edit"></i> Update Metadata
                        </a>
                        <a href="killsource.xsl?mount={@mount}" class="admin-action-btn danger">
                            <i class="fas fa-stop-circle"></i> Kill Source
                        </a>
                    </div>

                    <!-- Existing Users -->
                    <h3><i class="fas fa-users-cog"></i> Authorized Users</h3>
                    <xsl:choose>
                        <xsl:when test="User">
                            <table>
                                <thead>
                                    <tr>
                                        <th>Username</th>
                                        <th style="text-align: center; width: 150px;">Action</th>
                                    </tr>
                                </thead>
                                <tbody>
                                    <xsl:variable name="themount"><xsl:value-of select="@mount"/></xsl:variable>
                                    <xsl:for-each select="User">
                                        <tr>
                                            <td><i class="fas fa-user"></i> <xsl:value-of select="username"/></td>
                                            <td style="text-align: center;">
                                                <a href="manageauth.xsl?mount={$themount}&amp;username={username}&amp;action=delete" class="btn btn-secondary" style="font-size: 0.75rem; padding: 0.375rem 0.75rem;">
                                                    <i class="fas fa-trash"></i> Delete
                                                </a>
                                            </td>
                                        </tr>
                                    </xsl:for-each>
                                </tbody>
                            </table>
                        </xsl:when>
                        <xsl:otherwise>
                            <p style="text-align: center; padding: 2rem; color: var(--text-secondary); background: var(--bg-light); border-radius: var(--radius-md);">
                                <i class="fas fa-info-circle"></i> No users configured yet.
                            </p>
                        </xsl:otherwise>
                    </xsl:choose>

                    <!-- Add New User Form -->
                    <h3 style="margin-top: 2rem;"><i class="fas fa-user-plus"></i> Add New User</h3>
                    <form method="GET" action="manageauth.xsl">
                        <div style="background: var(--bg-light); padding: 1.5rem; border-radius: var(--radius-md);">
                            <div style="margin-bottom: 1rem;">
                                <label style="display: block; margin-bottom: 0.5rem; font-weight: 600;">
                                    <i class="fas fa-user"></i> Username
                                </label>
                                <input type="text" name="username" required="required" style="width: 100%; max-width: 400px;"/>
                            </div>
                            <div style="margin-bottom: 1.5rem;">
                                <label style="display: block; margin-bottom: 0.5rem; font-weight: 600;">
                                    <i class="fas fa-lock"></i> Password
                                </label>
                                <input type="password" name="password" required="required" style="width: 100%; max-width: 400px;"/>
                            </div>
                            <input type="hidden" name="mount" value="{@mount}"/>
                            <input type="hidden" name="action" value="add"/>
                            <button type="submit" class="btn btn-primary">
                                <i class="fas fa-plus-circle"></i> Add User
                            </button>
                        </div>
                    </form>
                </div>
            </xsl:for-each>

        </div>
    </div>

    <xsl:call-template name="admin-footer"/>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

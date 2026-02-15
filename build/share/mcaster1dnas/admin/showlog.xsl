<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>

<xsl:template match="/mcaster1stats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Mcaster1DNAS Log Viewer</title>
    <style>
        body {
            margin: 0;
            padding: 1rem;
            background: #f8fafc;
            font-family: 'SF Mono', 'Monaco', 'Menlo', 'Ubuntu Mono', monospace;
            font-size: 0.875rem;
            line-height: 1.5;
        }
        pre {
            margin: 0;
            padding: 1rem;
            background: #1e293b;
            color: #e2e8f0;
            border-radius: 0.5rem;
            overflow-x: auto;
            white-space: pre-wrap;
            word-wrap: break-word;
        }
        .log-entry {
            margin-bottom: 0.25rem;
        }
    </style>
</head>
<body>
    <pre><xsl:for-each select="/mcaster1stats"><xsl:for-each select="log"><xsl:value-of select="."/></xsl:for-each></xsl:for-each></pre>
</body>
</html>
</xsl:template>
</xsl:stylesheet>

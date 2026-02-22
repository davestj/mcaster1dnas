<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:output method="html" indent="yes" encoding="UTF-8" doctype-system="about:legacy-compat"/>
<xsl:include href="header.xsl"/>
<xsl:include href="footer.xsl"/>

<xsl:template match="/mcaster1stats">
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Update Metadata - Mcaster1DNAS Admin</title>

    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.4.0/css/all.min.css"
          integrity="sha512-iecdLmaskl7CVkqkXNQ/ZH/XLlvWZOJyj7Yy7tcenmpD1ypASozpmT/E0iPtmFIB46ZmdtAc9eNBvH0H/ZpiBw=="
          crossorigin="anonymous" referrerpolicy="no-referrer"/>

    <link rel="stylesheet" type="text/css" href="/style.css"/>
    <link rel="icon" type="image/x-icon" href="/favicon.ico"/>
    <link rel="icon" type="image/png" sizes="32x32" href="/favicon-32x32.png"/>
    <link rel="icon" type="image/png" sizes="16x16" href="/favicon-16x16.png"/>
    <link rel="apple-touch-icon" sizes="180x180" href="/apple-touch-icon.png"/>
    <script src="/mcaster-utils.js"></script>

    <style>
        .icy2-row {
            display: flex;
            align-items: center;
            gap: 0.75rem;
            padding: 0.65rem 0.85rem;
            background: var(--bg-light, #f8fafc);
            border: 1px solid var(--border-light, #e2e8f0);
            border-radius: 6px;
            margin-bottom: 0.5rem;
        }
        .icy2-row-label {
            min-width: 190px;
            max-width: 190px;
        }
        .icy2-row-label strong {
            display: block;
            font-size: 0.82rem;
            font-weight: 600;
            color: var(--text-primary, #1e293b);
        }
        .icy2-row-label code {
            display: block;
            font-size: 0.69rem;
            color: #0891b2;
            margin-top: 1px;
            word-break: break-all;
        }
        .icy2-row input[type=text] {
            flex: 1;
            min-width: 0;
        }
        .icy2-remove-btn {
            background: none;
            border: none;
            color: #dc2626;
            cursor: pointer;
            font-size: 1.15rem;
            line-height: 1;
            padding: 0 0.2rem;
            opacity: 0.7;
            transition: opacity 0.15s;
        }
        .icy2-remove-btn:hover { opacity: 1; }
        .icy2-builder-controls {
            display: flex;
            align-items: center;
            gap: 0.6rem;
            margin-bottom: 0.85rem;
            flex-wrap: wrap;
        }
        .icy2-builder-controls select {
            flex: 1;
            min-width: 200px;
            max-width: 480px;
        }
        #icy2-empty-hint {
            color: var(--text-secondary, #64748b);
            font-size: 0.85rem;
            font-style: italic;
            padding: 0.75rem;
            text-align: center;
            border: 1px dashed var(--border-light, #e2e8f0);
            border-radius: 6px;
        }
    </style>
</head>
<body>
    <xsl:call-template name="admin-header">
        <xsl:with-param name="active-page" select="''"/>
    </xsl:call-template>

    <div class="mcaster-main">
        <div class="mcaster-container">

            <div class="info-box">
                <div class="info-box-title">
                    <i class="fas fa-question-circle"></i> What is Stream Metadata?
                </div>
                <div class="info-box-content">
                    Metadata is the "Now Playing" information displayed in listeners' media players.
                    Use the <strong>ICY2 v2.2 Field Builder</strong> below to add only the fields you
                    want to update — select a field from the dropdown, fill in the value, and add as
                    many as needed before submitting.
                </div>
            </div>

            <xsl:for-each select="source">
                <div class="mcaster-card">
                    <h2>
                        <i class="fas fa-edit"></i> Update Metadata
                        <xsl:if test="server_name"> &#8212; <xsl:value-of select="server_name"/></xsl:if>
                    </h2>
                    <p style="color: var(--text-secondary); margin-bottom: 1.5rem;">
                        Mount Point: <strong><xsl:value-of select="@mount"/></strong>
                    </p>

                    <form id="meta-form" method="GET" action="/admin/metadata.xsl"
                          onsubmit="return metaFormSubmit(this)">
                        <input type="hidden" name="mount"   value="{@mount}"/>
                        <input type="hidden" name="mode"    value="updinfo"/>
                        <input type="hidden" name="charset" value="UTF-8"/>

                        <!-- ── Basic stream fields ── -->
                        <div style="margin-bottom: 1.5rem;">
                            <h3 style="font-size: 0.8rem; text-transform: uppercase; letter-spacing: 0.08em;
                                       color: var(--text-secondary); margin-bottom: 0.85rem;">
                                <i class="fas fa-music"></i> Standard Stream Info
                            </h3>
                            <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(240px,1fr)); gap: 0.85rem;">
                                <div>
                                    <label style="display:block;margin-bottom:0.35rem;font-size:0.8rem;font-weight:600;">
                                        Song (Artist &#8212; Title)
                                    </label>
                                    <input type="text" name="song" placeholder="e.g. Daft Punk - Get Lucky" style="width:100%;"/>
                                </div>
                                <div>
                                    <label style="display:block;margin-bottom:0.35rem;font-size:0.8rem;font-weight:600;">
                                        Title (standalone)
                                    </label>
                                    <input type="text" name="title" placeholder="e.g. Get Lucky" style="width:100%;"/>
                                </div>
                                <div>
                                    <label style="display:block;margin-bottom:0.35rem;font-size:0.8rem;font-weight:600;">
                                        Artist
                                    </label>
                                    <input type="text" name="artist" placeholder="e.g. Daft Punk" style="width:100%;"/>
                                </div>
                                <div>
                                    <label style="display:block;margin-bottom:0.35rem;font-size:0.8rem;font-weight:600;">
                                        Artwork URL
                                    </label>
                                    <input type="text" name="artwork" placeholder="https://example.com/cover.jpg" style="width:100%;"/>
                                </div>
                            </div>
                        </div>

                        <!-- ── ICY2 v2.2 Dynamic Field Builder ── -->
                        <div style="padding-top: 1.25rem; border-top: 1px solid var(--border-light);">
                            <h3 style="color: #0891b2; font-size: 0.8rem; text-transform: uppercase;
                                       letter-spacing: 0.08em; margin-bottom: 0.85rem;">
                                <i class="fas fa-satellite-dish"></i> ICY2 v2.2 Field Builder
                            </h3>

                            <div class="icy2-builder-controls">
                                <select id="icy2-select" onchange="">
                                    <option value="">&#8212; Select a field to add &#8212;</option>
                                </select>
                                <button type="button" class="btn btn-secondary"
                                        onclick="icy2AddField()"
                                        style="white-space:nowrap;">
                                    <i class="fas fa-plus"></i> Add Field
                                </button>
                            </div>

                            <div id="icy2-fields">
                                <div id="icy2-empty-hint">
                                    No ICY2 fields added yet. Use the selector above to add fields.
                                </div>
                            </div>
                        </div>

                        <div style="margin-top: 1.5rem; display: flex; gap: 0.75rem; flex-wrap: wrap; align-items: center;">
                            <button type="submit" class="btn btn-primary" style="padding: 0.75rem 2rem;">
                                <i class="fas fa-satellite-dish"></i> Push Metadata Update
                            </button>
                            <button type="button" class="btn btn-secondary"
                                    onclick="icy2ClearAll()" style="padding: 0.75rem 1.25rem;">
                                <i class="fas fa-times"></i> Clear All ICY2 Fields
                            </button>
                        </div>
                    </form>
                </div>
            </xsl:for-each>

        </div>
    </div>

    <xsl:call-template name="admin-footer"/>

<script>//<![CDATA[
/* ---------------------------------------------------------------
 * ICY2 v2.2 Field Builder — all fields grouped by category
 * ---------------------------------------------------------------*/
var ICY2_FIELDS = [
    /* Track */
    { g:'Track',    p:'icy-meta-track-artist',      l:'Track Artist',          ph:'e.g. Daft Punk' },
    { g:'Track',    p:'icy-meta-track-title',       l:'Track Title',           ph:'e.g. Get Lucky' },
    { g:'Track',    p:'icy-meta-track-album',       l:'Track Album',           ph:'e.g. Random Access Memories' },
    { g:'Track',    p:'icy-meta-track-artwork',     l:'Track Artwork URL',     ph:'https://cdn.example.com/art.jpg' },
    { g:'Track',    p:'icy-meta-track-bpm',         l:'Track BPM',             ph:'e.g. 128' },
    { g:'Track',    p:'icy-meta-track-year',        l:'Track Year',            ph:'e.g. 2024' },
    { g:'Track',    p:'icy-meta-track-isrc',        l:'Track ISRC',            ph:'e.g. USRC17607839' },
    { g:'Track',    p:'icy-meta-track-label',       l:'Record Label',          ph:'e.g. Atlantic Records' },
    { g:'Track',    p:'icy-meta-track-buy-url',     l:'Track Buy URL',         ph:'https://store.example.com/track' },
    /* Show / Programming */
    { g:'Show',     p:'icy-meta-show-title',        l:'Show Title',            ph:'e.g. The Morning Show' },
    { g:'Show',     p:'icy-meta-show-episode',      l:'Show Episode',          ph:'e.g. Episode 42' },
    { g:'Show',     p:'icy-meta-show-season',       l:'Show Season',           ph:'e.g. Season 3' },
    { g:'Show',     p:'icy-meta-show-start-time',   l:'Show Start Time (ISO)', ph:'2026-02-21T20:00:00Z' },
    { g:'Show',     p:'icy-meta-show-end-time',     l:'Show End Time (ISO)',   ph:'2026-02-21T22:00:00Z' },
    { g:'Show',     p:'icy-meta-playlist-name',     l:'Playlist Name',         ph:'e.g. Evening Chill Mix' },
    /* DJ / Host */
    { g:'DJ',       p:'icy-meta-dj-handle',         l:'DJ Handle',             ph:'@djmcaster' },
    { g:'DJ',       p:'icy-meta-dj-bio',            l:'DJ Bio',                ph:'Short bio text...' },
    { g:'DJ',       p:'icy-meta-dj-photo',          l:'DJ Photo URL',          ph:'https://cdn.example.com/dj.jpg' },
    { g:'DJ',       p:'icy-meta-dj-showrating',     l:'DJ Show Rating',        ph:'e.g. 4.8/5' },
    /* Station Notices */
    { g:'Notices',  p:'icy-meta-notice-board',      l:'Notice Board',          ph:'Station announcement text...' },
    { g:'Notices',  p:'icy-meta-upcoming-show',     l:'Coming Up Next',        ph:'e.g. Jazz Lounge at 8pm' },
    { g:'Notices',  p:'icy-meta-emergency-alert',   l:'Emergency Alert',       ph:'Critical alert message' },
    { g:'Notices',  p:'icy-meta-hashtags',          l:'Hashtags',              ph:'#radio #live #streaming' },
    /* Video */
    { g:'Video',    p:'icy-meta-video-platform',    l:'Video Platform',        ph:'YouTube, Twitch, Kick...' },
    { g:'Video',    p:'icy-meta-video-link',        l:'Video Link URL',        ph:'https://youtube.com/live/...' },
    { g:'Video',    p:'icy-meta-video-title',       l:'Video Title',           ph:'Live stream title' },
    { g:'Video',    p:'icy-meta-video-resolution',  l:'Video Resolution',      ph:'e.g. 1080p' },
    { g:'Video',    p:'icy-meta-video-codec',       l:'Video Codec',           ph:'e.g. H.264' },
    { g:'Video',    p:'icy-meta-video-framerate',   l:'Video Framerate',       ph:'e.g. 30fps' },
    { g:'Video',    p:'icy-meta-video-bitrate',     l:'Video Bitrate',         ph:'e.g. 3500k' },
    { g:'Video',    p:'icy-meta-video-thumbnail',   l:'Video Thumbnail URL',   ph:'https://cdn.example.com/thumb.jpg' },
    /* Listener Engagement */
    { g:'Engage',   p:'icy-meta-tip-url',           l:'Tip Jar URL',           ph:'https://ko-fi.com/...' },
    { g:'Engage',   p:'icy-meta-chat-url',          l:'Chat URL',              ph:'https://discord.gg/...' },
    { g:'Engage',   p:'icy-meta-request-url',       l:'Request URL',           ph:'https://example.com/request' },
    { g:'Engage',   p:'icy-meta-share-url',         l:'Share URL',             ph:'https://example.com/listen' },
    /* Social Media */
    { g:'Social',   p:'icy-meta-social-twitter',    l:'Twitter / X Handle',    ph:'@mycallsign' },
    { g:'Social',   p:'icy-meta-social-ig',         l:'Instagram Handle',      ph:'@mycallsign' },
    { g:'Social',   p:'icy-meta-social-tiktok',     l:'TikTok Handle',         ph:'@mycallsign' },
    { g:'Social',   p:'icy-meta-social-facebook',   l:'Facebook Page URL',     ph:'https://facebook.com/...' },
    { g:'Social',   p:'icy-meta-social-youtube',    l:'YouTube Channel URL',   ph:'https://youtube.com/@...' },
    { g:'Social',   p:'icy-meta-social-bluesky',    l:'Bluesky Handle',        ph:'@mycallsign.bsky.social' },
    { g:'Social',   p:'icy-meta-social-website',    l:'Website URL',           ph:'https://mystation.com' },
    /* Station Identity */
    { g:'Station',  p:'icy-meta-station-id',        l:'Station ID',            ph:'Unique station identifier' },
    { g:'Station',  p:'icy-meta-station-slogan',    l:'Station Slogan',        ph:'Your station tagline' },
    { g:'Station',  p:'icy-meta-station-logo',      l:'Station Logo URL',      ph:'https://cdn.example.com/logo.png' },
    { g:'Station',  p:'icy-meta-station-country',   l:'Station Country',       ph:'e.g. US' },
    { g:'Station',  p:'icy-meta-station-founded',   l:'Station Founded',       ph:'e.g. 2010' },
    { g:'Station',  p:'icy-meta-station-type',      l:'Station Type',          ph:'internet, terrestrial, podcast...' },
    /* Audio Technical */
    { g:'Audio',    p:'icy-meta-audio-codec',       l:'Audio Codec',           ph:'AAC, MP3, FLAC, Opus...' },
    { g:'Audio',    p:'icy-meta-audio-samplerate',  l:'Sample Rate',           ph:'e.g. 44100' },
    { g:'Audio',    p:'icy-meta-audio-channels',    l:'Channels',              ph:'e.g. 2 (stereo)' },
    { g:'Audio',    p:'icy-meta-audio-quality',     l:'Audio Quality',         ph:'e.g. High' },
    /* Broadcast */
    { g:'Broadcast',p:'icy-meta-cdn-region',        l:'CDN Region',            ph:'e.g. us-east-1' },
    { g:'Broadcast',p:'icy-meta-relay-origin',      l:'Relay Origin URL',      ph:'https://origin.example.com/stream' },
    { g:'Broadcast',p:'icy-meta-stream-failover',   l:'Failover Stream URL',   ph:'https://backup.example.com/stream' },
    { g:'Broadcast',p:'icy-meta-stream-quality-tier',l:'Quality Tier',         ph:'e.g. premium, standard, low' }
];

var icy2Added = {};   /* param -> row element */

function icy2BuildDropdown() {
    var sel = document.getElementById('icy2-select');
    var curVal = sel.value;
    sel.innerHTML = '';
    var opt0 = document.createElement('option');
    opt0.value = '';
    opt0.textContent = '\u2014 Select a field to add \u2014';
    sel.appendChild(opt0);

    var groups = {};
    for (var i = 0; i < ICY2_FIELDS.length; i++) {
        var f = ICY2_FIELDS[i];
        if (!icy2Added[f.p]) {
            if (!groups[f.g]) groups[f.g] = [];
            groups[f.g].push(f);
        }
    }
    var gnames = Object.keys(groups);
    for (var gi = 0; gi < gnames.length; gi++) {
        var og = document.createElement('optgroup');
        og.label = gnames[gi];
        var gfields = groups[gnames[gi]];
        for (var fi = 0; fi < gfields.length; fi++) {
            var opt = document.createElement('option');
            opt.value = gfields[fi].p;
            opt.textContent = gfields[fi].l;
            og.appendChild(opt);
        }
        sel.appendChild(og);
    }
    /* restore selection if still available */
    if (curVal && !icy2Added[curVal]) sel.value = curVal;
}

function icy2FindField(param) {
    for (var i = 0; i < ICY2_FIELDS.length; i++) {
        if (ICY2_FIELDS[i].p === param) return ICY2_FIELDS[i];
    }
    return null;
}

function icy2SafeId(param) {
    return 'icy2row-' + param.replace(/[^a-z0-9]/g, '-');
}

function icy2AddField() {
    var sel = document.getElementById('icy2-select');
    var param = sel.value;
    if (!param) return;
    var field = icy2FindField(param);
    if (!field || icy2Added[param]) return;

    var container = document.getElementById('icy2-fields');
    var hint = document.getElementById('icy2-empty-hint');
    if (hint) hint.style.display = 'none';

    var row = document.createElement('div');
    row.className = 'icy2-row';
    row.id = icy2SafeId(param);

    var labelDiv = document.createElement('div');
    labelDiv.className = 'icy2-row-label';
    var strong = document.createElement('strong');
    strong.textContent = field.l;
    var code = document.createElement('code');
    code.textContent = field.p;
    labelDiv.appendChild(strong);
    labelDiv.appendChild(code);

    var inp = document.createElement('input');
    inp.type = 'text';
    inp.name = field.p;
    inp.placeholder = field.ph;
    inp.autocomplete = 'off';

    var btn = document.createElement('button');
    btn.type = 'button';
    btn.className = 'icy2-remove-btn';
    btn.title = 'Remove this field';
    btn.textContent = '\u2715';
    (function(p) {
        btn.onclick = function() { icy2RemoveField(p); };
    }(param));

    row.appendChild(labelDiv);
    row.appendChild(inp);
    row.appendChild(btn);
    container.appendChild(row);

    icy2Added[param] = row;
    sel.value = '';
    icy2BuildDropdown();
    inp.focus();
}

function icy2RemoveField(param) {
    var row = document.getElementById(icy2SafeId(param));
    if (row) row.parentNode.removeChild(row);
    delete icy2Added[param];
    icy2BuildDropdown();

    /* show hint if no fields left */
    var container = document.getElementById('icy2-fields');
    var hint = document.getElementById('icy2-empty-hint');
    if (hint && container.children.length === 1) hint.style.display = '';
}

function icy2ClearAll() {
    var keys = Object.keys(icy2Added);
    for (var i = 0; i < keys.length; i++) {
        var row = document.getElementById(icy2SafeId(keys[i]));
        if (row) row.parentNode.removeChild(row);
    }
    icy2Added = {};
    icy2BuildDropdown();
    var hint = document.getElementById('icy2-empty-hint');
    if (hint) hint.style.display = '';
}

function metaFormSubmit(form) {
    /* Disable empty ICY2 inputs so they are not included in the GET query string */
    var inputs = document.querySelectorAll('#icy2-fields input[type=text]');
    for (var i = 0; i < inputs.length; i++) {
        if (!inputs[i].value.trim()) inputs[i].disabled = true;
    }
    return true;
}

/* Also allow pressing Enter in the dropdown to add the field */
document.addEventListener('DOMContentLoaded', function() {
    icy2BuildDropdown();
    var sel = document.getElementById('icy2-select');
    if (sel) {
        sel.addEventListener('keydown', function(e) {
            if (e.key === 'Enter') { e.preventDefault(); icy2AddField(); }
        });
    }
});
//]]></script>

</body>
</html>
</xsl:template>
</xsl:stylesheet>

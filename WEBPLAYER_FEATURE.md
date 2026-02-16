# Mcaster1DNAS Web Player Feature

## Overview
A modern, browser-based web player has been added to Mcaster1DNAS that allows listeners and administrators to play streams directly in their browser without downloading M3U or XSPF files.

## Features

### Player Capabilities
- **Real-time Metadata Updates**: Song title, artist, and stream info update every 5 seconds
- **Professional VU Meters**: Visual audio level indicators with Web Audio API
- **Responsive Design**: Works on desktop, tablet, and mobile devices
- **Volume Control**: Adjustable volume with localStorage persistence
- **Keyboard Shortcuts**:
  - `Space`: Play/Pause toggle
  - `Escape`: Stop playback
- **Bookmarkable**: Users can bookmark the player window for quick access

### Player Interface
- **Now Playing Display**: Shows current song title, artist, and stream description
- **Audio Controls**: Play, Pause, Stop buttons with visual feedback
- **Stream Statistics**: Real-time display of:
  - Format/Codec (MP3, Vorbis, AAC, Opus)
  - Bitrate (kbps)
  - Sample Rate (Hz)
  - Channels (Stereo/Mono)
  - Current Listeners
  - Peak Listeners
- **Professional Styling**: Mcaster1DNAS branded theme with gradients and modern UI
- **Buffering Indicator**: Visual feedback while stream loads

## Integration Points

### Admin Interface
Player buttons have been added to:
- **`/admin/stats.xsl`**: Admin statistics page
- **`/admin/listmounts.xsl`**: Mount points listing page

Button location: Between "Manage Auth" and "Kill Source" buttons
Tooltip: "Open web player in new window - bookmark for quick access"

### Public Interface
Player button added to:
- **`/status.xsl`**: Public status page

Button location: Primary button alongside M3U and XSPF download options

## How It Works

### For Administrators
1. Navigate to Stats or List Mounts page
2. Click the "Player" button next to any active mount
3. A new window opens with the web player
4. Click Play to start listening
5. Bookmark the player window for future use

### For Public Listeners
1. Visit the status page (e.g., `https://dnas.mcaster1.com:9443/status.xsl`)
2. Click the "Web Player" button
3. New player window opens
4. Click Play to start listening
5. Bookmark for instant access

## Technical Implementation

### Files Created
1. **`/web/webplayer.xsl`** - Public player XSLT template
2. **`/admin/webplayer.xsl`** - Admin player XSLT template (identical functionality)

### How It's Called
- **URL Format**: `webplayer.xsl?mount=/stream.mp3`
- **Window Properties**: 650x800 pixels, resizable, scrollable
- **Window Name**: `mcaster1player` (reuses same window if already open)

### Data Flow
1. Player XSL receives XML from Mcaster1DNAS with mount-specific statistics
2. XSLT transforms XML into HTML player interface
3. JavaScript extracts stream metadata and constructs audio element
4. Metadata polling updates every 5 seconds via `status-json.xsl`
5. Web Audio API provides VU meter visualization

### Metadata Polling
```javascript
// Polls status-json.xsl every 5 seconds
fetch('../status-json.xsl')
    .then(response => response.json())
    .then(data => {
        // Update current title if changed
        if (currentSource && currentSource.title) {
            titleElement.textContent = currentSource.title;
        }
    });
```

### Volume Persistence
```javascript
// Save to localStorage
localStorage.setItem('mcaster1_volume', volume);

// Restore on load
const savedVolume = localStorage.getItem('mcaster1_volume');
```

## Browser Compatibility

### Supported Browsers
- **Chrome/Edge**: Full support including Web Audio API VU meters
- **Firefox**: Full support including Web Audio API VU meters
- **Safari**: Full support (may require user interaction before autoplay)
- **Mobile Chrome/Safari**: Full support with responsive layout

### Required Browser Features
- HTML5 Audio Element
- JavaScript ES6
- CSS3 Flexbox/Grid
- Web Audio API (optional, for VU meters)
- localStorage (optional, for volume persistence)

## Mobile Support

### Responsive Features
- Adaptive grid layout for stream info cards
- Touch-friendly button sizes (50px minimum)
- Optimized font sizes for small screens
- Full-width player on screens < 640px

### Mobile Instructions
Bookmark instructions are automatically tailored:
- **Desktop**: Press Ctrl+D (Windows/Linux) or Cmd+D (Mac)
- **Mobile**: Tap browser menu → "Add to Home Screen"

## Security Considerations

### HTTPS Support
- Player works over both HTTP and HTTPS
- Stream URLs inherit protocol from parent page
- No mixed-content warnings

### CORS Headers
- Server must send appropriate CORS headers for cross-origin requests
- Metadata API (`status-json.xsl`) must be accessible to player

## Customization

### Branding Colors
Defined in CSS variables:
```css
:root {
    --mcaster-blue: #0891b2;
    --mcaster-teal: #14b8a6;
    --dnas-green: #10b981;
    --accent-yellow: #fbbf24;
    /* ... */
}
```

### Player Dimensions
Default: 650x800 pixels
Modify in button onclick:
```javascript
window.open('webplayer.xsl?mount={@mount}', 'mcaster1player',
    'width=650,height=800,resizable=yes,scrollbars=yes')
```

### Metadata Update Interval
Default: 5000ms (5 seconds)
Modify in JavaScript:
```javascript
setInterval(updateMetadata, 5000); // Change to desired interval
```

## Installation

### Files Modified
1. `/web/Makefile.am` - Added webplayer.xsl to dist_web_DATA
2. `/admin/Makefile.am` - Added webplayer.xsl to dist_admin_DATA
3. `/admin/stats.xsl` - Added Player button
4. `/admin/listmounts.xsl` - Added Player button
5. `/web/status.xsl` - Added Web Player button

### Build Commands
```bash
autoreconf -fi
./configure
make
make install
```

### Installation Locations
- **Public**: `/build/share/mcaster1dnas/web/webplayer.xsl`
- **Admin**: `/build/share/mcaster1dnas/admin/webplayer.xsl`

## Usage Examples

### Direct Link
```html
<a href="/webplayer.xsl?mount=/stream.mp3">Listen Now</a>
```

### Pop-up Window
```html
<a href="javascript:void(0);"
   onclick="window.open('/webplayer.xsl?mount=/stream.mp3',
   'mcaster1player', 'width=650,height=800,resizable=yes,scrollbars=yes')">
   Web Player
</a>
```

### Iframe Embed (Not Recommended)
```html
<iframe src="/webplayer.xsl?mount=/stream.mp3"
        width="650" height="800"
        frameborder="0">
</iframe>
```

## Future Enhancements

### Planned Features
- [ ] Playlist support (multiple mounts)
- [ ] Equalizer controls
- [ ] Stream recording capability
- [ ] Share buttons (Facebook, Twitter, etc.)
- [ ] Chromecast/AirPlay support
- [ ] Visualizer options (spectrum analyzer, waveform)
- [ ] Dark/Light theme toggle
- [ ] Mini player mode (compact view)
- [ ] History tracking (recently played songs)
- [ ] Favorites/Bookmarks for specific streams

### Potential Integrations
- [ ] Discord Rich Presence
- [ ] Twitch integration (rebroadcast)
- [ ] Social media metadata cards (OpenGraph)
- [ ] Analytics tracking (Google Analytics, Matomo)
- [ ] Push notifications for song changes

## Troubleshooting

### Player Won't Load
- Check browser console for JavaScript errors
- Verify webplayer.xsl is installed correctly
- Ensure mount point exists and is streaming
- Check CORS headers are configured

### No Audio Playback
- Verify stream URL is accessible
- Check browser autoplay policies (may require user interaction)
- Test stream with M3U/XSPF to confirm it works
- Check browser codec support (MP3, Vorbis, AAC)

### Metadata Not Updating
- Verify `status-json.xsl` endpoint is accessible
- Check browser network tab for failed requests
- Ensure stream is sending ICY metadata
- Verify 5-second polling interval isn't blocked

### VU Meters Not Working
- Check browser supports Web Audio API
- Look for CORS errors preventing audio analysis
- Verify AudioContext is created successfully
- Check browser console for Web Audio API errors

## Credits

**Developed for**: Mcaster1DNAS v2.5.1-rc1
**Based on**: webplayer.php from CasterClub YP
**Styling**: Mcaster1DNAS official theme
**Icons**: FontAwesome 6.4.0

---

**Last Updated**: February 15, 2026
**Version**: 1.0.0

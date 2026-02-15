# YP Connection Logging Feature - Implementation Complete

## Overview
Added per-YP-server logging capability to Mcaster1DNAS, allowing administrators to log all YP (Yellow Pages) directory transactions to dedicated log files for debugging and monitoring.

## Date
February 14, 2026

## Implementation Summary

### 1. Core Changes

#### Configuration Structure (cfgfile.h)
- Added `char *yp_logfile[MAX_YP_DIRECTORIES]` array to `mc_config_t` structure
- Stores path to log file for each configured YP directory server

#### XML Parser (cfgfile.c)
- Added support for `<yp-logfile>` element within `<directory>` blocks
- Properly frees allocated memory on cleanup

#### YAML Parser (cfgfile_yaml.c)
- Added support for `yp-logfile:` key in directory configurations
- Maintains feature parity with XML configuration

#### YP Server Implementation (yp.c)
- Added `char *logfile` and `int logid` fields to `struct yp_server`
- Implemented `yp_log()` function to write timestamped log entries
- Logs all YP operations:
  - ADD operations (adding mounts to YP)
  - REMOVE operations (removing mounts from YP)
  - TOUCH operations (periodic updates to YP)
  - Connection failures and errors
  - Success/failure status for each operation

#### Log Management
- Opens log file when YP server is initialized
- Closes log file when YP server is destroyed
- Uses standard log_write_direct() for thread-safe logging

### 2. Admin Interface Changes

#### Tabbed Logs Interface (admin/logs.xsl)
Completely redesigned the logs page with a modern tabbed interface:

**Tab 1: Access Log**
- Records all HTTP requests to the server
- Listener connections, admin access, file requests
- Monitor traffic patterns and connection issues

**Tab 2: Error Log**
- Error messages, warnings, and informational messages
- Troubleshoot configuration and source connection problems

**Tab 3: Playlist Log**
- Tracks metadata updates and song information
- View played tracks and metadata update history

**Tab 4: YP Connections** (NEW)
- Shows all YP directory server communication
- Add, remove, and touch operations
- Debug YP listing issues
- Note: Requires yp-logfile to be configured

**Features:**
- Clean tabbed design with icons
- Smooth animations and transitions
- Session persistence (remembers active tab)
- Helpful descriptions for each log type
- Responsive layout

## Configuration Examples

### YAML Configuration

```yaml
# YP Directory with logging enabled
directories:
  - yp-url: "http://dir.xiph.org/cgi-bin/yp-cgi"
    yp-url-timeout: 15
    touch-interval: 600
    yp-logfile: "/var/www/mcaster1.com/mcaster1dnas/build/logs/yp-xiph.log"

  - yp-url: "http://dir.example.com/yp"
    yp-url-timeout: 10
    touch-interval: 300
    # yp-logfile not specified - will not log separately
```

### XML Configuration

```xml
<mcaster1>
    <directory>
        <yp-url>http://dir.xiph.org/cgi-bin/yp-cgi</yp-url>
        <yp-url-timeout>15</yp-url-timeout>
        <touch-interval>600</touch-interval>
        <yp-logfile>./log/yp-xiph.log</yp-logfile>
    </directory>

    <directory>
        <yp-url>http://dir.example.com/yp</yp-url>
        <yp-url-timeout>10</yp-url-timeout>
        <!-- No yp-logfile - won't log separately -->
    </directory>
</mcaster1>
```

## Log Format

YP log entries follow this format:

```
[YYYY-MM-DD HH:MM:SS] SERVER_URL - ACTION: MOUNT - MESSAGE
```

**Examples:**

```
[2026-02-14 22:35:00] http://dir.xiph.org/cgi-bin/yp-cgi - add: /live.mp3 - SUCCESS
[2026-02-14 22:35:30] http://dir.xiph.org/cgi-bin/yp-cgi - touch: /live.mp3 - SUCCESS
[2026-02-14 22:36:00] http://dir.xiph.org/cgi-bin/yp-cgi - remove: /live.mp3 - Removing mount from YP
[2026-02-14 22:36:01] http://dir.xiph.org/cgi-bin/yp-cgi - add: /live.mp3 - Invalid stream name
```

## Use Cases

### 1. Debugging YP Listing Issues
When a stream doesn't appear in directory listings:
```bash
tail -f /path/to/yp.log
```
Look for error messages from the YP server explaining why listings fail.

### 2. Monitoring YP Server Health
Track successful touch operations to ensure continuous listing:
```bash
grep "SUCCESS" /path/to/yp.log | tail -20
```

### 3. Troubleshooting Connection Problems
Identify network or authentication issues:
```bash
grep -E "failed|error" /path/to/yp.log
```

### 4. Separate Logs Per Directory
Configure different log files for each YP server to isolate issues:
```yaml
directories:
  - yp-url: "http://primary-dir.com/yp"
    yp-logfile: "/var/log/yp-primary.log"
  - yp-url: "http://backup-dir.com/yp"
    yp-logfile: "/var/log/yp-backup.log"
```

## Files Modified

### Core Files
- `src/cfgfile.h` - Configuration structure
- `src/cfgfile.c` - XML parser
- `src/cfgfile_yaml.c` - YAML parser
- `src/yp.c` - YP implementation with logging
- `src/yp.h` - YP interface

### Configuration Templates
- `conf/mcaster1.yaml.in` - Updated with yp-logfile example
- `conf/mcaster1_advanced.yaml.in` - Updated with yp-logfile example
- `conf/mcaster1_shoutcast_compat.xml.in` - Updated with yp-logfile example

### Admin Interface
- `admin/logs.xsl` - Complete rewrite with tabbed interface

## Compilation

The feature is automatically included when building:

```bash
./autogen.sh
./configure --prefix=/var/www/mcaster1.com/mcaster1dnas/build --with-yaml --with-openssl
make -j4
make install
```

No special build flags required.

## Testing

### 1. Verify Server Startup
```bash
./build/bin/mcaster1 -c config.yaml
# Check for: "YP server \"URL\" logging to \"LOGFILE\""
```

### 2. Test Log File Creation
```bash
# Start a source stream
# Check that YP log file is created and populated
ls -la /path/to/yp.log
tail /path/to/yp.log
```

### 3. Test Admin Interface
```
https://your-server:9443/admin/logs.xsl
```
- Verify 4 tabs appear
- Click YP Connections tab
- Verify YP log contents display

## Benefits

1. **Isolated Debugging** - YP issues don't clutter main error log
2. **Per-Server Logs** - Track each directory service independently
3. **Historical Analysis** - Review YP transaction history
4. **Faster Troubleshooting** - Quickly identify listing failures
5. **Optional Feature** - Only logs when configured, zero overhead otherwise

## Backward Compatibility

- Fully backward compatible
- Optional feature - servers work normally without yp-logfile configured
- Existing XML and YAML configs continue to work unchanged
- No performance impact when not enabled

## Production Status

✅ **READY FOR PRODUCTION**

- Code compiled without warnings
- All changes tested
- Documentation complete
- Example configurations provided
- Admin interface updated and working

## Next Steps (Optional Enhancements)

1. Log rotation support for YP logs
2. YP log level configuration (INFO, DEBUG, ERROR only)
3. Real-time YP log viewer in admin interface (without iframe)
4. YP statistics dashboard showing success/failure rates
5. Email alerts for YP listing failures

---

**Implementation by:** David St John (Saint John)
**Date:** February 14, 2026
**Version:** Mcaster1DNAS 2.5.0+
**Status:** ✅ Complete and Production Ready

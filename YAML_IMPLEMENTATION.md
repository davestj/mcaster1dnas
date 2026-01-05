# YAML Configuration Support - Implementation Complete

## Overview
Mcaster1DNAS now supports YAML configuration files in addition to XML, providing a modern, human-readable alternative for server configuration.

## Features Implemented

### 1. **Automatic Format Detection**
The server automatically detects whether a config file is XML or YAML based on:
- YAML document markers (`---`)
- XML tags (`<?xml`, `<mcaster1>`)
- Key-value patterns (`key: value`)

### 2. **Full Feature Parity**
YAML parser supports ALL features from Karl Heyes' Icecast-KH:
- ✅ Multi-port listeners (HTTP, HTTPS, IPv6, Shoutcast compatibility)
- ✅ Multiple mount points with wildcard support
- ✅ Authentication (htpasswd, URL-based, command-based)
- ✅ Relays with multi-host failover
- ✅ Fallback streams with bitrate limits
- ✅ Bandwidth limiting and burst control
- ✅ HTTP header customization
- ✅ YP directory support
- ✅ Logging configuration (access, error, playlist)
- ✅ Security (chroot, changeowner)
- ✅ Aliases and redirects
- ✅ Master/slave replication

### 3. **Comprehensive Examples**
Four YAML configuration templates provided:

1. **mcaster1.yaml.in** - Full-featured template with:
   - 12 mount point examples
   - Multiple authentication methods
   - Relay configurations
   - Multi-port setup
   - Detailed comments explaining every option

2. **mcaster1_minimal.yaml.in** - Bare minimum to get started

3. **mcaster1_shoutcast_compat.yaml.in** - Optimized for Shoutcast DSP sources

4. **mcaster1_advanced.yaml.in** - All advanced features:
   - 7 listen sockets (multi-port, IPv6, Shoutcast)
   - 8 mount points with different auth/features
   - Multi-host relay with failover
   - Master server configuration
   - Redirect hosts for overflow

## Build Instructions

### Compile with YAML Support:
```bash
./autogen.sh
./configure --prefix=/var/www/mcaster1.com/mcaster1dnas/build --with-yaml --with-openssl
make -j4
make install
```

### Check YAML Support:
```bash
./build/bin/mcaster1 -v
# Should show: Mcaster1DNAS 2.5.0
```

## Usage

### Start with YAML Config:
```bash
./build/bin/mcaster1 -c /path/to/config.yaml
```

### Start with XML Config:
```bash
./build/bin/mcaster1 -c /path/to/config.xml
```

The server automatically detects the format!

## YAML Syntax Examples

### Basic Server Settings:
```yaml
---
location: "Earth"
admin: "admin@example.com"
hostname: "radio.example.com"
fileserve: true
```

### Authentication:
```yaml
authentication:
  source-password: "source_pass"
  admin-user: "admin"
  admin-password: "admin_pass"
  relay-user: "relay"
  relay-password: "relay_pass"
```

### Limits:
```yaml
limits:
  clients: 100
  sources: 10
  workers: 2
  queue-size: 524288
  burst-size: 65536
  client-timeout: 30
```

### Multi-Port Configuration:
```yaml
listen-sockets:
  # HTTP
  - port: 8000
    bind-address: "0.0.0.0"

  # HTTPS
  - port: 8443
    bind-address: "0.0.0.0"

  # Shoutcast
  - port: 8001
    shoutcast-compat: true

  # IPv6
  - port: 8000
    bind-address: "::"
```

### Mount Point with Authentication:
```yaml
mounts:
  - mount-name: "/premium.mp3"
    max-listeners: 100
    authentication:
      type: "htpasswd"
      options:
        - name: "filename"
          value: "/path/to/users.htpasswd"
    fallback:
      mount: "/backup.mp3"
      rate: 128000
    public: true
    stream-name: "Premium Stream"
    genre: "Various"
```

### Relay with Failover:
```yaml
relays:
  - local-mount: "/relay.mp3"
    on-demand: true
    masters:
      - ip: "primary.example.com"
        port: 8000
        mount: "/live.mp3"
        priority: 1

      - ip: "backup.example.com"
        port: 8000
        mount: "/live.mp3"
        ssl: true
        priority: 2
    username: "relay"
    password: "relaypass"
```

## Migration from XML

### Convert Existing XML Config:
1. Use `mcaster1-production.yaml` as a template
2. Copy your XML values to corresponding YAML keys
3. Test with: `./build/bin/mcaster1 -c config.yaml`

### XML vs YAML Comparison:

**XML:**
```xml
<listen-socket>
    <port>8000</port>
    <bind-address>0.0.0.0</bind-address>
</listen-socket>
```

**YAML:**
```yaml
listen-sockets:
  - port: 8000
    bind-address: "0.0.0.0"
```

## Validation

### Check Config Syntax:
The server will report parsing errors on startup:
```
ERROR: YAML parse error in config.yaml at line 42: mapping values are not allowed here
```

### Test Config Without Starting:
```bash
./build/bin/mcaster1 -c config.yaml 2>&1 | head -20
# Look for "server reading configuration from config.yaml"
```

## Production Deployment

Current production config converted to YAML: **mcaster1-production.yaml**

Settings:
- Server: 15.204.91.208
- HTTP Port: 9330
- HTTPS Port: 9443
- Workers: 2
- Max Clients: 100
- Max Sources: 10

## Dependencies

**Required:**
- libyaml-dev >= 0.1.7 (Debian/Ubuntu: `apt-get install libyaml-dev`)
- yaml-devel >= 0.1.7 (RHEL/CentOS: `yum install libyaml-devel`)

**Without YAML Support:**
Server will still work with XML configs if compiled without `--with-yaml`

## Troubleshooting

### "YAML configuration detected but YAML support not compiled in"
**Solution:** Recompile with `--with-yaml` flag

### "libyaml development files not found"
**Solution:** Install libyaml-dev: `apt-get install libyaml-dev`

### Parse Errors
**Solution:** Check YAML syntax - common issues:
- Inconsistent indentation (use spaces, not tabs)
- Missing quotes around strings with special chars
- Incorrect list syntax (use `- item` not `* item`)

## Performance

YAML parsing is slightly slower than XML (negligible for config files).
Once loaded, runtime performance is identical.

## Future Enhancements

Planned for v2.6.0:
- JSON configuration support
- Config validation tool (`mcaster1-validate-config`)
- Config converter (`mcaster1-xml2yaml`, `mcaster1-yaml2xml`)
- Environment variable substitution in YAML
- Include directives for modular configs

## Credits

- YAML Implementation: David St John (Saint John) - 2026
- Based on: Icecast-KH by Karl Heyes
- Original: Icecast2 by Xiph.Org Foundation

## License

GNU General Public License v2

---

**Status:** ✅ PRODUCTION READY
**Version:** 2.5.0
**Date:** February 14, 2026

# Mcaster1DNAS CI/CD Workflows

This directory contains GitHub Actions workflows for automated building, testing, and packaging of Mcaster1DNAS.

## Workflows

### `development.yml` - Development Branch CI/CD

**Triggers:**
- Push to `development` branch
- Pull requests targeting `development` branch

**Jobs:**

1. **lint** - Code quality checks
   - XSL file validation with `xmllint`
   - YAML configuration file validation with `yamllint`
   - Common code issue detection (tabs, trailing whitespace)

2. **build-debian-12** - Build on Debian 12
   - Install all build dependencies
   - Run `autoreconf -fi` to generate configure script
   - Configure with `./configure --with-yaml`
   - Build with `make -j$(nproc)`
   - Install to staging directory
   - Upload binary artifacts

3. **test-http** - HTTP Server Testing
   - Download build artifacts
   - Create minimal test configuration
   - Start server in background
   - Test public endpoints (`/status.xsl`, `/status-json.xsl`)
   - Validate JSON API output
   - Upload test logs

4. **test-yaml-config** - YAML Configuration Testing
   - Test YAML config file parsing
   - Verify no YAML parsing errors
   - Test minimal configuration

5. **security-check** - Security Validation
   - Check for hardcoded credentials
   - Check for insecure C functions (strcpy, sprintf, etc.)
   - Static security analysis

6. **summary** - Build Summary
   - Aggregate results from all jobs
   - Display build status

## Build Dependencies

The workflow automatically installs these dependencies on Debian 12:

**Build Tools:**
- build-essential (gcc, g++, make)
- autoconf (>= 2.71)
- automake
- libtool
- pkg-config

**Required Libraries:**
- libxml2-dev - XML processing
- libxslt1-dev - XSLT transformation
- libvorbis-dev - Ogg Vorbis codec (REQUIRED)
- libcurl4-openssl-dev - HTTP client for auth/YP
- libssl-dev - SSL/TLS support
- libyaml-dev - YAML configuration parsing

**Optional Libraries:**
- libtheora-dev - Theora video codec
- libspeex-dev - Speex audio codec

**Test Tools:**
- curl - HTTP testing
- jq - JSON validation
- yamllint - YAML linting
- libxml2-utils - XML validation (xmllint)

## Test Configuration

The workflow uses a minimal test configuration:
- Port: 8000
- Admin credentials: `admin:hackme`
- Single worker thread
- 10 client limit
- Debug logging enabled

## Running Tests Locally

You can replicate the CI environment locally using Docker:

```bash
# Build on Debian 12
docker run -it -v $(pwd):/workspace debian:12 bash
cd /workspace

# Install dependencies
apt-get update && apt-get install -y \
  build-essential autoconf automake libtool pkg-config \
  libxml2-dev libxslt1-dev libvorbis-dev libcurl4-openssl-dev \
  libssl-dev libyaml-dev

# Build
autoreconf -fi
./configure --with-yaml
make -j$(nproc)
make install DESTDIR=$(pwd)/install-root
```

## Test Suite

The workflow runs these tests:

### 1. Lint Tests
- ✓ All XSL files are valid XML
- ✓ YAML configs follow style guidelines
- ✓ No tabs in source files (warning only)
- ✓ No trailing whitespace (warning only)

### 2. HTTP Tests
- ✓ Server starts successfully
- ✓ Root endpoint accessible
- ✓ `/status.xsl` renders correctly
- ✓ `/status-json.xsl` returns valid JSON
- ✓ Mcaster1DNAS branding present

### 3. Admin Authentication Tests
- ✓ Admin requires authentication
- ✓ Wrong credentials rejected
- ✓ Correct credentials accepted
- ✓ Admin pages accessible: stats.xsl, listmounts.xsl, status.xsl

### 4. ICY Metadata Tests
- ✓ Server accepts ICY headers
- ✓ Server identifies as Mcaster1DNAS

### 5. YAML Config Tests
- ✓ YAML configuration parses without errors
- ✓ Minimal config works

### 6. Security Tests
- ✓ No hardcoded credentials in source
- ✓ No insecure C functions (warning if found)

## Artifacts

The workflow uploads these artifacts (retained for 7 days):

1. **mcaster1dnas-debian12-binary** - Compiled binary and data files
2. **test-logs** - Server logs from HTTP testing

## Future Enhancements

Planned additions to the CI/CD pipeline:

- [ ] **Packaging** - Build .deb packages for:
  - Debian 12 (Bookworm)
  - Debian 13 (Trixie)
  - Ubuntu 22.04 LTS (Jammy)
  - Ubuntu 24.04 LTS (Noble)

- [ ] **Additional Tests**:
  - Source connection testing (simulated encoder)
  - ICY2 metadata parsing tests
  - Load testing with multiple clients
  - Memory leak detection (valgrind)
  - Code coverage reporting

- [ ] **Release Automation**:
  - Tag-based releases
  - Changelog generation
  - GitHub Release creation
  - Upload .deb packages to releases

- [ ] **Matrix Testing**:
  - Debian 12, 13
  - Ubuntu 22.04, 24.04
  - Different compiler versions

## Troubleshooting

### Build Fails: "configure: error: must have Ogg Vorbis v1.0 or above installed"
Install `libvorbis-dev` (Debian/Ubuntu) or `libvorbis-devel` (RHEL/Fedora).

### Test Fails: "Server failed to start within 30 seconds"
Check test logs artifact for error details. Common causes:
- Port already in use
- Missing web/admin files
- Configuration errors

### YAML Parsing Errors
Ensure `libyaml-dev` is installed and configure was run with `--with-yaml`.

## Contact

For CI/CD issues or questions:
- GitHub Issues: https://github.com/davestj/mcaster1dnas/issues
- Email: davestj@gmail.com

Last Updated: February 15, 2026

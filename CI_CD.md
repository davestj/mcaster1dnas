# Mcaster1DNAS - CI/CD Documentation

**Last Updated:** February 15, 2026
**Status:** Active
**Branch:** development

---

## Overview

Mcaster1DNAS uses GitHub Actions for continuous integration and deployment. Every push to the `development` branch triggers automated building, testing, and validation.

## Workflow Status

Check the status of the latest build:
- **Badge:** Will be available at: `https://github.com/davestj/mcaster1dnas/actions/workflows/development.yml/badge.svg`
- **Workflow runs:** `https://github.com/davestj/mcaster1dnas/actions`

---

## Build Process

### 1. **Lint Stage** (3-5 minutes)
Validates code quality before building:
- XSL files validated with `xmllint`
- YAML configs checked with `yamllint`
- Source code scanned for common issues

**Passes when:** All XSL files are valid XML, YAML follows style guidelines

### 2. **Build Stage** (5-10 minutes)
Compiles Mcaster1DNAS on Debian 12:
1. Install build dependencies (autotools, libraries)
2. Run `autoreconf -fi` to generate configure script
3. Configure with `./configure --with-yaml`
4. Compile with `make -j$(nproc)`
5. Install to staging directory with `make install`
6. Upload binary artifacts

**Passes when:** Binary compiles successfully and all files install

### 3. **Test Stage** (5-10 minutes)
Tests the compiled server:

**HTTP Tests:**
- Server starts successfully
- Public endpoints accessible (`/status.xsl`, `/status-json.xsl`)
- JSON API returns valid output
- Server branding correct

**Admin Tests:**
- Authentication required for `/admin/`
- Wrong credentials rejected
- Correct credentials accepted (`admin:hackme`)
- Admin pages render correctly

**Config Tests:**
- YAML configuration parses without errors
- Minimal config works

**ICY2 Tests:**
- Server accepts ICY headers
- Server identifies as Mcaster1DNAS

**Passes when:** All endpoints respond correctly, authentication works

### 4. **Security Stage** (1-2 minutes)
Scans for security issues:
- No hardcoded credentials in source
- No insecure C functions (strcpy, sprintf, gets, etc.)

**Passes when:** No critical security issues found

---

## What Gets Tested

### ✅ Tested
- [x] XSL file validation
- [x] YAML config parsing
- [x] HTTP server functionality
- [x] Admin authentication
- [x] JSON API output
- [x] Basic security checks
- [x] Server branding
- [x] ICY header support

### ⏳ Planned (Future)
- [ ] Source connection simulation (encoder test)
- [ ] ICY2 metadata parsing
- [ ] Multi-client load testing
- [ ] Memory leak detection (valgrind)
- [ ] Code coverage reporting
- [ ] Package building (.deb files)

---

## Testing Locally

### Option 1: Docker (Recommended)

Replicate the exact CI environment:

```bash
# Start Debian 12 container
docker run -it --rm -v $(pwd):/workspace debian:12 bash
cd /workspace

# Install dependencies
apt-get update && apt-get install -y \
  build-essential autoconf automake libtool pkg-config \
  libxml2-dev libxslt1-dev libvorbis-dev libcurl4-openssl-dev \
  libssl-dev libyaml-dev curl jq

# Build
autoreconf -fi
./configure --with-yaml
make -j$(nproc)

# Run lint checks
find admin web -name "*.xsl" -exec xmllint --noout {} \;

# Test
make install DESTDIR=$(pwd)/install-root
./install-root/opt/mcaster1dnas/bin/mcaster1 --version
```

### Option 2: Native Build

If you're on Debian/Ubuntu:

```bash
# Install dependencies
sudo apt-get install -y \
  build-essential autoconf automake libtool pkg-config \
  libxml2-dev libxslt1-dev libvorbis-dev libcurl4-openssl-dev \
  libssl-dev libyaml-dev curl jq yamllint libxml2-utils

# Build
autoreconf -fi
./configure --with-yaml --prefix=/opt/mcaster1dnas
make -j$(nproc)
sudo make install

# Create test config
cat > test-ci.yaml <<EOF
---
location: "Local Test"
admin: "test@test.local"
hostname: "localhost"

authentication:
  source-password: "testpass"
  admin-user: "admin"
  admin-password: "hackme"

limits:
  clients: 10
  sources: 2

listen-sockets:
  - port: 8000

paths:
  basedir: "/tmp/mcaster1-test"
  logdir: "/tmp/mcaster1-test/logs"
  webroot: "$(pwd)/web"
  adminroot: "$(pwd)/admin"

logging:
  errorlog:
    name: "error.log"
    level: "debug"
EOF

# Create directories
mkdir -p /tmp/mcaster1-test/logs

# Run server
/opt/mcaster1dnas/bin/mcaster1 -c test-ci.yaml &

# Wait for startup
sleep 3

# Test endpoints
curl -f http://localhost:8000/status.xsl
curl -f -u admin:hackme http://localhost:8000/admin/stats.xsl
curl -f http://localhost:8000/status-json.xsl | jq .

# Stop server
pkill mcaster1
```

---

## Dependencies

### Build-Time Dependencies (Debian/Ubuntu)

```bash
sudo apt-get install -y \
  build-essential \      # GCC, G++, make
  autoconf \             # >= 2.71
  automake \             # Build automation
  libtool \              # Shared library support
  pkg-config \           # Library detection
  libxml2-dev \          # XML processing (REQUIRED)
  libxslt1-dev \         # XSLT transformation (REQUIRED)
  libvorbis-dev \        # Ogg Vorbis codec (REQUIRED)
  libcurl4-openssl-dev \ # HTTP client for auth/YP
  libssl-dev \           # SSL/TLS support
  libyaml-dev \          # YAML config parsing
  libtheora-dev \        # Theora video codec (optional)
  libspeex-dev           # Speex audio codec (optional)
```

### Runtime Dependencies (Debian/Ubuntu)

```bash
sudo apt-get install -y \
  libxml2 \
  libxslt1.1 \
  libvorbis0a \
  libcurl4 \
  libssl3 \
  libyaml-0-2 \
  libtheora0 \      # Optional
  libspeex1         # Optional
```

### Test Dependencies

```bash
sudo apt-get install -y \
  curl \           # HTTP testing
  jq \             # JSON validation
  yamllint \       # YAML linting
  libxml2-utils    # xmllint for XSL validation
```

---

## Artifacts

The CI workflow uploads these artifacts (retained for 7 days):

### 1. **mcaster1dnas-debian12-binary**
- Compiled `mcaster1` binary
- Installed data files (web, admin XSL)
- Location: `install-root/opt/mcaster1dnas/`

**Download:** Available from workflow run page

### 2. **test-logs**
- Server error logs
- Server access logs
- Location: `/tmp/mcaster1-test/logs/`

**Download:** Available from workflow run page (if tests run)

---

## Troubleshooting CI Failures

### Build Fails: "configure: error: must have Ogg Vorbis"

**Cause:** Missing libvorbis-dev
**Fix:** Install dependency in workflow (already done)

```yaml
apt-get install -y libvorbis-dev
```

### Build Fails: "autoreconf: command not found"

**Cause:** Missing autoconf/automake
**Fix:** Install build tools

```yaml
apt-get install -y autoconf automake libtool
```

### Test Fails: "Server failed to start within 30 seconds"

**Causes:**
1. Port 8000 already in use
2. Missing web/admin files
3. Configuration errors
4. Missing runtime libraries

**Fix:** Check test-logs artifact for error details

### Test Fails: Admin authentication

**Cause:** Admin credentials mismatch
**Fix:** Ensure test config uses `admin:hackme`

### Lint Fails: "Opening and ending tag mismatch"

**Cause:** Invalid XML in XSL file
**Fix:** Run `xmllint --noout <file.xsl>` locally to find error

---

## Adding New Tests

To add tests to the CI pipeline:

1. **Edit `.github/workflows/development.yml`**
2. **Add a new job or step:**

```yaml
- name: Test my new feature
  run: |
    echo "=== Testing new feature ==="

    # Your test commands here
    curl -f http://localhost:8000/my-endpoint || exit 1

    echo "✓ Test passed"
```

3. **Commit and push to development branch**
4. **Check workflow results on GitHub Actions**

---

## Future: Package Building

Planned for Q2 2026:

### Debian/Ubuntu Packages

```yaml
- name: Build .deb package
  run: |
    # Create debian/ packaging directory
    # Build with dpkg-buildpackage
    # Upload .deb artifact
```

**Target distributions:**
- Debian 12 (Bookworm)
- Debian 13 (Trixie)
- Ubuntu 22.04 LTS (Jammy)
- Ubuntu 24.04 LTS (Noble)

### RPM Packages (Future)

**Target distributions:**
- Fedora 39, 40
- RHEL 9
- Rocky Linux 9

---

## Manual Release Process (Current)

Until automated releases are ready:

1. **Tag the release:**
   ```bash
   git tag -a v2.6.0 -m "Release 2.6.0 - Podcast Features"
   git push origin v2.6.0
   ```

2. **Build packages manually:**
   ```bash
   # On Debian 12
   dpkg-buildpackage -us -uc
   ```

3. **Create GitHub Release:**
   - Go to: https://github.com/davestj/mcaster1dnas/releases/new
   - Select tag: `v2.6.0`
   - Title: "Mcaster1DNAS v2.6.0 - Podcast Features"
   - Description: Copy from `ChangeLog`
   - Attach: `.deb` files, source tarball

---

## Environment Variables

The workflow uses these environment variables:

| Variable | Value | Purpose |
|----------|-------|---------|
| `BUILD_DIR` | `build` | Build artifacts directory |
| `INSTALL_PREFIX` | `/opt/mcaster1dnas` | Installation prefix |
| `TEST_PORT` | `8000` | HTTP test server port |
| `TEST_ADMIN_USER` | `admin` | Test admin username |
| `TEST_ADMIN_PASS` | `hackme` | Test admin password |

---

## Security Considerations

### Secrets Management

**Currently:** No secrets required (open-source, public repo)

**Future (if needed):**
- Package signing keys → GitHub Secrets
- Docker Hub credentials → GitHub Secrets
- Deploy SSH keys → GitHub Secrets

### Test Credentials

Test credentials (`admin:hackme`) are:
- Only used in CI environment
- Never exposed to production
- Clearly marked as test-only in code

---

## Contributing

When contributing code:

1. **Fork the repository**
2. **Create a feature branch from `development`:**
   ```bash
   git checkout development
   git pull
   git checkout -b feature/my-feature
   ```

3. **Make your changes**
4. **Test locally** (see "Testing Locally" section)
5. **Commit with clear messages:**
   ```bash
   git commit -m "feat: Add new feature X

   - Implements feature X
   - Adds tests for X
   - Updates documentation"
   ```

6. **Push to your fork:**
   ```bash
   git push origin feature/my-feature
   ```

7. **Open a Pull Request** targeting `development` branch
8. **CI will automatically run** on your PR
9. **Address any CI failures** before review

---

## Monitoring CI

### View Workflow Runs
https://github.com/davestj/mcaster1dnas/actions

### Download Artifacts
1. Go to workflow run
2. Scroll to "Artifacts" section
3. Click to download

### View Logs
1. Go to workflow run
2. Click on job name (e.g., "Build on Debian 12")
3. Expand step to see output

---

## Contact

CI/CD questions or issues:

- **GitHub Issues:** https://github.com/davestj/mcaster1dnas/issues
- **Email:** davestj@gmail.com
- **Subject:** `[CI/CD] Your question here`

---

## References

- **GitHub Actions Docs:** https://docs.github.com/en/actions
- **Workflow File:** `.github/workflows/development.yml`
- **Workflow README:** `.github/workflows/README.md`
- **Build Dependencies:** See `configure.ac`
- **Test Configuration:** See workflow YAML

---

**Happy Building! 🚀**

# Mcaster1DNAS - Version Management System

**Last Updated:** February 15, 2026
**Current System:** Automated branch-based versioning

---

## Overview

Mcaster1DNAS uses an automated version management system that generates version strings based on the Git branch and build context. This ensures consistent, meaningful version numbers across all builds and releases.

## Version Format

### Semantic Versioning Base

Base version follows [Semantic Versioning 2.0.0](https://semver.org/):

```
MAJOR.MINOR.PATCH
```

- **MAJOR** - Incompatible API changes, major features
- **MINOR** - Backward-compatible functionality additions
- **PATCH** - Backward-compatible bug fixes

**Current base version:** `2.5.1` (stored in `VERSION` file)

### Branch-Based Release Types

Version strings are automatically generated based on the Git branch:

| Branch Pattern | Release Type | Version Format | Example |
|----------------|--------------|----------------|---------|
| `main` or `master` | **Production** | `MAJOR.MINOR.PATCH` | `2.5.1` |
| `rc/*` | **Release Candidate** | `MAJOR.MINOR.PATCH-rcN` | `2.5.1-rc1` |
| `development` | **Beta** | `MAJOR.MINOR.PATCH-beta.YYYYMMDD.HHMMSS` | `2.5.1-beta.20260215.143022` |
| `feature/*` | **Alpha** | `MAJOR.MINOR.PATCH-alpha.YYYYMMDD.HHMMSS` | `2.5.1-alpha.20260215.120000` |
| Other | **Dev** | `MAJOR.MINOR.PATCH-dev.HASH` | `2.5.1-dev.a3f2c1d` |

### Modified/Dirty Builds

If the Git working directory has uncommitted changes, `-modified` is appended:

```
2.5.1-beta.20260215.143022-modified
```

---

## File Structure

### VERSION File

```
/var/www/mcaster1.com/mcaster1dnas/VERSION
```

**Contents:** Single line with semantic version (no pre-release suffixes)

```
2.5.1
```

This is the **source of truth** for the base version.

### Generated Files

1. **git_hash.h** - Auto-generated C header with version string
   ```c
   #define GIT_VERSION    "2.5.1-beta.20260215.143022"
   ```

2. **build-version.txt** - Build metadata (CI/CD only)
   ```
   Base Version: 2.5.1
   Full Version: 2.5.1-beta.20260215.143022
   Branch: development
   Commit: a3f2c1d4b5e6f7890abcdef1234567890abcdef
   Build Date: 2026-02-15 14:30:22 UTC
   Build Number: 42
   ```

---

## Scripts

### 1. GIT-VERSION-GEN

**Location:** `/var/www/mcaster1.com/mcaster1dnas/GIT-VERSION-GEN`

**Purpose:** Automatically generates version string based on branch

**Called by:** Makefile during build (automatic)

**How it works:**
1. Reads base version from `VERSION` file
2. Detects current Git branch
3. Generates appropriate version string
4. Writes to `git_hash.h`

**Manual usage:**
```bash
./GIT-VERSION-GEN
# Output: GIT_VERSION=2.5.1-beta.20260215.143022
```

### 2. version-bump.sh

**Location:** `/var/www/mcaster1.com/mcaster1dnas/scripts/version-bump.sh`

**Purpose:** Manually increment version numbers

**Usage:**
```bash
# Auto-bump (branch-aware)
./scripts/version-bump.sh auto

# Bump specific component
./scripts/version-bump.sh major    # 2.5.1 → 3.0.0
./scripts/version-bump.sh minor    # 2.5.1 → 2.6.0
./scripts/version-bump.sh patch    # 2.5.1 → 2.5.2
```

**What it does:**
1. Reads current version from `VERSION` file
2. Increments specified component
3. Updates `VERSION` file
4. Updates `configure.ac` with new version
5. Optionally creates Git tag

**Interactive prompts:**
- Asks to create Git tag (for release branches)
- Shows version change summary

---

## Make Targets

### Display Version

```bash
make version
# or
make version-info
```

**Output:**
```
=== Mcaster1DNAS Version Information ===
Base Version:  2.5.1
Full Version:  2.5.1-beta.20260215.143022
Git Branch:    development
Git Commit:    a3f2c1d
========================================
```

### Bump Version

```bash
# Auto-bump based on branch
make version-bump-auto

# Bump MAJOR version (2.5.1 → 3.0.0)
make version-bump-major

# Bump MINOR version (2.5.1 → 2.6.0)
make version-bump-minor

# Bump PATCH version (2.5.1 → 2.5.2)
make version-bump-patch
```

---

## Workflow

### Development Workflow

```bash
# 1. Create feature branch
git checkout development
git pull
git checkout -b feature/my-feature

# 2. Make changes
# ... code changes ...

# 3. Check version (automatic)
make version
# Output: 2.5.1-alpha.20260215.120000

# 4. Build (version auto-generated)
make clean
make -j$(nproc)

# 5. Commit and push
git add .
git commit -m "feat: Add my feature"
git push origin feature/my-feature
```

**Version on feature branch:** `2.5.1-alpha.YYYYMMDD.HHMMSS`

### Release Candidate Workflow

```bash
# 1. Create RC branch from development
git checkout development
git pull
git checkout -b rc/2.6.0

# 2. Bump version to next minor
./scripts/version-bump.sh minor
# VERSION file: 2.6.0
# Full version: 2.6.0-rc1

# 3. Test and fix bugs
# ... testing ...

# 4. If fixes needed, RC auto-increments
# Full version: 2.6.0-rc2 (after new commits)

# 5. When ready for release, merge to main
git checkout main
git merge rc/2.6.0
git tag v2.6.0
git push origin main --tags
```

**Version on rc/* branch:** `2.6.0-rc1`, `2.6.0-rc2`, etc.

### Production Release Workflow

```bash
# 1. Merge RC to main
git checkout main
git merge rc/2.6.0

# 2. Version is clean (no suffixes)
make version
# Output: 2.6.0

# 3. Tag the release
git tag -a v2.6.0 -m "Release 2.6.0 - Podcast Features"
git push origin v2.6.0

# 4. Build production binaries
make clean
make -j$(nproc)
make install

# 5. Create distribution tarball
make dist
# Output: mcaster1dnas-2.6.0.tar.gz
```

**Version on main/master:** `2.6.0` (clean, no suffixes)

---

## CI/CD Integration

### GitHub Actions Workflow

The `.github/workflows/development.yml` workflow automatically:

1. **Detects branch and generates version**
   ```yaml
   - name: Generate version
     run: |
       ./GIT-VERSION-GEN
       echo "version=$(cat VERSION)" >> $GITHUB_OUTPUT
   ```

2. **Builds with correct version**
   - Version embedded in binary
   - Version shown in `mcaster1 -v`

3. **Creates versioned artifacts**
   - Tarball: `mcaster1dnas-2.5.1-beta.20260215.143022.tar.gz`
   - Artifact: `mcaster1dnas-2.5.1-beta.20260215.143022-debian12`

4. **Generates build metadata**
   ```
   build-version.txt:
     Base Version: 2.5.1
     Full Version: 2.5.1-beta.20260215.143022
     Branch: development
     Commit: a3f2c1d
     Build Date: 2026-02-15 14:30:22 UTC
     Build Number: 42
   ```

### Build Artifacts

**Development builds:**
```
mcaster1dnas-2.5.1-beta.20260215.143022-debian12.tar.gz
```

**RC builds:**
```
mcaster1dnas-2.6.0-rc1-debian12.tar.gz
mcaster1dnas-2.6.0-rc2-debian12.tar.gz
```

**Production builds:**
```
mcaster1dnas-2.6.0-debian12.tar.gz
mcaster1dnas-2.6.0-debian12.deb
```

---

## Version Display

### Command Line

```bash
$ mcaster1 -v
Mcaster1DNAS 2.5.1-beta.20260215.143022
```

### Server Response Headers

```http
Server: Mcaster1DNAS/2.5.1-beta.20260215.143022
```

### Admin Interface

Version displayed in:
- Stats page header
- Server information section
- Footer

### JSON API

```json
{
  "icestats": {
    "admin": "admin@mcaster1.com",
    "host": "dnas.mcaster1.com",
    "location": "Earth",
    "server_id": "Mcaster1DNAS 2.5.1-beta.20260215.143022",
    ...
  }
}
```

---

## Manual Version Updates

### When to Bump Manually

- **MAJOR bump:** Breaking changes, incompatible API changes
- **MINOR bump:** New features, significant enhancements
- **PATCH bump:** Bug fixes, minor improvements

### How to Bump

1. **Determine bump type:**
   ```bash
   # Major: 2.5.1 → 3.0.0 (breaking changes)
   # Minor: 2.5.1 → 2.6.0 (new features)
   # Patch: 2.5.1 → 2.5.2 (bug fixes)
   ```

2. **Run version bump script:**
   ```bash
   ./scripts/version-bump.sh minor
   ```

3. **Review changes:**
   ```bash
   git diff VERSION configure.ac
   ```

4. **Commit:**
   ```bash
   git add VERSION configure.ac
   git commit -m "chore: bump version to 2.6.0"
   ```

5. **Rebuild:**
   ```bash
   autoreconf -fi
   ./configure
   make clean
   make -j$(nproc)
   ```

---

## Troubleshooting

### Problem: Wrong version displayed

**Cause:** Stale `git_hash.h` file

**Fix:**
```bash
rm -f git_hash.h
make clean
make -j$(nproc)
```

### Problem: Version shows "unknown"

**Cause:** Not in a Git repository or VERSION file missing

**Fix:**
```bash
# Ensure VERSION file exists
echo "2.5.1" > VERSION

# Rebuild
./GIT-VERSION-GEN
make clean
make
```

### Problem: RC number not incrementing

**Cause:** Git tags not fetched

**Fix:**
```bash
git fetch --tags
./GIT-VERSION-GEN
```

### Problem: Version has -modified suffix

**Cause:** Uncommitted changes in working directory

**Fix:**
```bash
# Check status
git status

# Commit or stash changes
git add .
git commit -m "fix: uncommitted changes"

# Or stash
git stash

# Rebuild
make clean
make
```

---

## Version History

| Version | Date | Type | Description |
|---------|------|------|-------------|
| 2.5.1 | 2026-02-15 | Current | Version management system implemented |
| 2.5.0 | 2026-02-14 | Release | First Mcaster1DNAS release (forked from Icecast-KH) |
| 2.4.0.kh22 | Pre-fork | Legacy | Last Icecast-KH version |

---

## Best Practices

### ✅ DO

- Use `make version` to check current version
- Use `./scripts/version-bump.sh auto` for branch-aware bumps
- Create Git tags for releases: `git tag v2.6.0`
- Let CI/CD generate versions automatically
- Commit VERSION file changes with clear messages

### ❌ DON'T

- Manually edit `git_hash.h` (auto-generated)
- Edit `configure.ac` version directly (use VERSION file)
- Create releases from feature branches
- Skip version bumps for releases

---

## Future Enhancements

- [ ] Automated version bump in CI/CD on release
- [ ] Changelog generation from git commits
- [ ] Version validation in CI/CD
- [ ] Debian package versioning alignment
- [ ] RPM package versioning support

---

## References

- **Semantic Versioning:** https://semver.org/
- **VERSION file:** `/var/www/mcaster1.com/mcaster1dnas/VERSION`
- **Version bump script:** `/var/www/mcaster1.com/mcaster1dnas/scripts/version-bump.sh`
- **Git version generator:** `/var/www/mcaster1.com/mcaster1dnas/GIT-VERSION-GEN`
- **Makefile:** `/var/www/mcaster1.com/mcaster1dnas/Makefile.am`

---

**Questions?** Contact: davestj@gmail.com

#!/bin/bash
# Mcaster1DNAS Version Bump Script (Linux/macOS)
# Automatically increments version based on branch and release type
#
# Usage:
#   ./scripts/version-bump.sh [major|minor|patch|auto]
#   ./scripts/version-bump.sh set 2.5.3-dev
#
# Branch-based release detection:
#   - main/master → Production release (2.5.2)
#   - rc/* → Release candidate (2.5.2-rc1, rc2, rc3...)
#   - development → Beta release (2.5.2-beta.YYYYMMDD.HHMMSS)
#   - feature/* → Alpha release (2.5.2-alpha.YYYYMMDD.HHMMSS)
#
# Files updated:
#   VERSION                   (canonical source)
#   windows/config.h          (PACKAGE_VERSION, PACKAGE_STRING, VERSION)
#   windows/git-version.ps1   ($baseVer fallback)
#   GIT-VERSION-GEN           (BASE_VER fallback)
#   windows/mcaster1dnas.iss  (AppVerName, OutputBaseFilename, VersionInfoVersion)
#   build.sh                  (display line)
#   README.md                 (version badge)

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
VERSION_FILE="$PROJECT_ROOT/VERSION"
CONFIGURE_AC="$PROJECT_ROOT/configure.ac"

# Color output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
info() {
    echo -e "${BLUE}ℹ${NC} $1"
}

success() {
    echo -e "${GREEN}✓${NC} $1"
}

warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

error() {
    echo -e "${RED}✗${NC} $1"
    exit 1
}

# Get current version from VERSION file
get_current_version() {
    if [ ! -f "$VERSION_FILE" ]; then
        error "VERSION file not found at $VERSION_FILE"
    fi
    cat "$VERSION_FILE" | tr -d '\n'
}

# Parse semantic version into components
parse_version() {
    local version=$1
    # Strip any pre-release or metadata
    version=$(echo "$version" | sed 's/-.*$//')

    MAJOR=$(echo "$version" | cut -d. -f1)
    MINOR=$(echo "$version" | cut -d. -f2)
    PATCH=$(echo "$version" | cut -d. -f3)
}

# Increment version component
bump_major() {
    local version=$1
    parse_version "$version"
    echo "$((MAJOR + 1)).0.0"
}

bump_minor() {
    local version=$1
    parse_version "$version"
    echo "$MAJOR.$((MINOR + 1)).0"
}

bump_patch() {
    local version=$1
    parse_version "$version"
    echo "$MAJOR.$MINOR.$((PATCH + 1))"
}

# Get current git branch
get_branch() {
    git rev-parse --abbrev-ref HEAD 2>/dev/null || echo "unknown"
}

# Get current RC number from git tags
get_latest_rc_number() {
    local base_version=$1
    # Find all tags matching the pattern base_version-rcN
    local latest_rc=$(git tag -l "${base_version}-rc*" 2>/dev/null | \
                      sed 's/.*-rc//' | \
                      sort -n | \
                      tail -1)

    if [ -z "$latest_rc" ]; then
        echo "0"
    else
        echo "$latest_rc"
    fi
}

# Generate version string based on branch and type
generate_version_string() {
    local base_version=$1
    local branch=$(get_branch)
    local timestamp=$(date -u "+%Y%m%d.%H%M%S")
    local short_hash=$(git rev-parse --short HEAD 2>/dev/null || echo "nogit")

    info "Branch: $branch"
    info "Base version: $base_version"

    case "$branch" in
        main|master)
            # Production release - use base version only
            echo "$base_version"
            ;;
        rc/*)
            # Release candidate - increment RC number
            local rc_num=$(get_latest_rc_number "$base_version")
            local next_rc=$((rc_num + 1))
            echo "${base_version}-rc${next_rc}"
            ;;
        development)
            # Beta release - append beta + timestamp
            echo "${base_version}-beta.${timestamp}"
            ;;
        feature/*|hotfix/*|bugfix/*)
            # Alpha release - append alpha + timestamp
            echo "${base_version}-alpha.${timestamp}"
            ;;
        *)
            # Unknown branch - append dev + timestamp
            warning "Unknown branch type, using dev suffix"
            echo "${base_version}-dev.${timestamp}"
            ;;
    esac
}

# Update VERSION file
update_version_file() {
    local new_version=$1
    printf '%s' "$new_version" > "$VERSION_FILE"
    success "Updated VERSION file to: $new_version"
}

# Update configure.ac
update_configure_ac() {
    local new_version=$1

    # Update AC_INIT line
    sed -i.bak "s/AC_INIT(\[Mcaster1DNAS\], \[.*\], \[support@mcaster1.com\])/AC_INIT([Mcaster1DNAS], [$new_version], [support@mcaster1.com])/" "$CONFIGURE_AC"

    if [ $? -eq 0 ]; then
        rm -f "$CONFIGURE_AC.bak"
        success "Updated configure.ac to version: $new_version"
    else
        error "Failed to update configure.ac"
    fi
}

# Update windows/config.h
update_windows_config_h() {
    local new_version=$1
    local f="$PROJECT_ROOT/windows/config.h"
    if [ ! -f "$f" ]; then warning "windows/config.h not found — skipped"; return; fi
    sed -i.bak \
        -e "s/#define PACKAGE_VERSION \"[^\"]*\"/#define PACKAGE_VERSION \"$new_version\"/" \
        -e "s/#define PACKAGE_STRING  \"[^\"]*\"/#define PACKAGE_STRING  \"Mcaster1DNAS $new_version\"/" \
        -e "s/#define VERSION         \"[^\"]*\"/#define VERSION         \"$new_version\"/" \
        "$f"
    rm -f "$f.bak"
    success "windows/config.h updated"
}

# Update windows/git-version.ps1 fallback
update_git_version_ps1() {
    local new_version=$1
    local f="$PROJECT_ROOT/windows/git-version.ps1"
    if [ ! -f "$f" ]; then warning "windows/git-version.ps1 not found — skipped"; return; fi
    sed -i.bak "s/\\\$baseVer = \"[^\"]*\"/\$baseVer = \"$new_version\"/" "$f"
    rm -f "$f.bak"
    success "windows/git-version.ps1 fallback updated"
}

# Update GIT-VERSION-GEN fallback
update_git_version_gen() {
    local new_version=$1
    local f="$PROJECT_ROOT/GIT-VERSION-GEN"
    if [ ! -f "$f" ]; then warning "GIT-VERSION-GEN not found — skipped"; return; fi
    sed -i.bak "s/BASE_VER=\"[^\"]*\"/BASE_VER=\"$new_version\"/" "$f"
    rm -f "$f.bak"
    success "GIT-VERSION-GEN fallback updated"
}

# Update windows/mcaster1dnas.iss
update_iss() {
    local new_version=$1
    local f="$PROJECT_ROOT/windows/mcaster1dnas.iss"
    if [ ! -f "$f" ]; then warning "mcaster1dnas.iss not found — skipped"; return; fi
    # Numeric only for VersionInfoVersion (strip pre-release suffix)
    local num_ver
    num_ver=$(echo "$new_version" | sed 's/-.*$//')
    sed -i.bak \
        -e "s/AppVerName=Mcaster1DNAS v.*/AppVerName=Mcaster1DNAS v$new_version/" \
        -e "s/OutputBaseFilename=mcaster1dnas_win32_v[^ ]*/OutputBaseFilename=mcaster1dnas_win32_v${new_version}_setup/" \
        -e "s/VersionInfoVersion=.*/VersionInfoVersion=$num_ver.0/" \
        "$f"
    rm -f "$f.bak"
    success "mcaster1dnas.iss updated"
}

# Update build.sh display line
update_build_sh() {
    local new_version=$1
    local f="$PROJECT_ROOT/build.sh"
    if [ ! -f "$f" ]; then warning "build.sh not found — skipped"; return; fi
    sed -i.bak "s/Mcaster1DNAS Build Script v[^ \"]*/Mcaster1DNAS Build Script v$new_version/" "$f"
    rm -f "$f.bak"
    success "build.sh updated"
}

# Update README.md version badge
update_readme_badge() {
    local new_version=$1
    local f="$PROJECT_ROOT/README.md"
    if [ ! -f "$f" ]; then warning "README.md not found — skipped"; return; fi
    # shields.io uses -- for a literal dash in the badge label
    local badge_ver
    badge_ver=$(echo "$new_version" | sed 's/-/--/g')
    sed -i.bak \
        "s|badge/version-[^-]*-[^-]*-[^)]*-brightgreen|badge/version-${badge_ver}-brightgreen|g" \
        "$f"
    rm -f "$f.bak"
    success "README.md badge updated"
}

# Update Makefile version target
update_makefile() {
    local new_version=$1
    # This will be called by `make version`
    info "Makefile will use version from VERSION file: $new_version"
}

# Create git tag for release
create_git_tag() {
    local version=$1
    local branch=$(get_branch)

    # Only tag on main/master or rc branches
    case "$branch" in
        main|master|rc/*)
            if git tag -l "v$version" | grep -q "v$version"; then
                warning "Tag v$version already exists"
            else
                git tag -a "v$version" -m "Release $version"
                success "Created git tag: v$version"
                info "Push tag with: git push origin v$version"
            fi
            ;;
        *)
            info "Skipping git tag creation for branch: $branch"
            ;;
    esac
}

# Display version information
show_version_info() {
    local version=$1
    local branch=$(get_branch)
    local full_version=$(generate_version_string "$version")

    echo ""
    echo "═══════════════════════════════════════════════════"
    echo "  Mcaster1DNAS Version Information"
    echo "═══════════════════════════════════════════════════"
    echo "  Base Version:  $version"
    echo "  Full Version:  $full_version"
    echo "  Branch:        $branch"
    echo "  Release Type:  $(get_release_type "$branch")"
    echo "═══════════════════════════════════════════════════"
    echo ""
}

# Get release type from branch
get_release_type() {
    local branch=$1
    case "$branch" in
        main|master) echo "Production" ;;
        rc/*) echo "Release Candidate" ;;
        development) echo "Beta" ;;
        feature/*|hotfix/*|bugfix/*) echo "Alpha" ;;
        *) echo "Development" ;;
    esac
}

# Main script
main() {
    local bump_type="${1:-auto}"
    local current_version=$(get_current_version)
    local new_base_version=""

    info "Current version: $current_version"
    info "Bump type: $bump_type"

    # Determine new base version
    case "$bump_type" in
        set)
            # Explicit set: ./version-bump.sh set 2.5.3-dev
            if [ -z "$2" ]; then
                error "Usage: $0 set <version>  (e.g. set 2.5.3-dev)"
            fi
            new_base_version="$2"
            update_version_file "$new_base_version"
            update_windows_config_h "$new_base_version"
            update_git_version_ps1 "$new_base_version"
            update_git_version_gen "$new_base_version"
            update_iss "$new_base_version"
            update_build_sh "$new_base_version"
            update_readme_badge "$new_base_version"
            success "All files set to $new_base_version"
            exit 0
            ;;
        major)
            new_base_version=$(bump_major "$current_version")
            ;;
        minor)
            new_base_version=$(bump_minor "$current_version")
            ;;
        patch)
            new_base_version=$(bump_patch "$current_version")
            ;;
        auto)
            # Auto-detect based on branch
            local branch=$(get_branch)
            case "$branch" in
                main|master|rc/*)
                    # For release branches, bump patch by default
                    new_base_version=$(bump_patch "$current_version")
                    ;;
                *)
                    # For dev branches, keep current base version
                    new_base_version="$current_version"
                    ;;
            esac
            ;;
        *)
            error "Invalid bump type: $bump_type (use: major, minor, patch, or auto)"
            ;;
    esac

    # Generate full version string with suffixes
    local full_version=$(generate_version_string "$new_base_version")

    # Update all files
    update_version_file "$new_base_version"
    update_configure_ac "$full_version"
    update_windows_config_h "$new_base_version"
    update_git_version_ps1 "$new_base_version"
    update_git_version_gen "$new_base_version"
    update_iss "$new_base_version"
    update_build_sh "$new_base_version"
    update_readme_badge "$new_base_version"

    # Show version info
    show_version_info "$new_base_version"

    # Create git tag if appropriate
    if [ "$bump_type" != "auto" ] || [ "$(get_branch)" = "main" ] || [ "$(get_branch)" = "master" ]; then
        read -p "Create git tag v$full_version? [y/N] " -n 1 -r
        echo
        if [[ $REPLY =~ ^[Yy]$ ]]; then
            create_git_tag "$full_version"
        fi
    fi

    success "Version bump complete!"
    info "Next steps:"
    echo "  1. Review:  git diff VERSION configure.ac windows/config.h GIT-VERSION-GEN README.md"
    echo "  2. Commit:  git add VERSION configure.ac windows/config.h windows/git-version.ps1"
    echo "              git add GIT-VERSION-GEN windows/mcaster1dnas.iss build.sh README.md"
    echo "              git commit -m 'chore: bump version to $full_version'"
    echo "  3. Linux rebuild:   autoreconf -fi && ./configure && make"
    echo "  4. Windows rebuild: msbuild windows/Mcaster1DNAS.sln /p:Configuration=Debug /p:Platform=x64"
}

# Run main function
main "$@"

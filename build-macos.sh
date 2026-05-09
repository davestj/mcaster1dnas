#!/bin/bash
#
# build-macos.sh — Cross-platform (macOS + Linux) build script for Mcaster1DNAS
#
# We detect the OS at runtime and wire the correct include/library paths into
# configure so all platform-specific library locations are resolved without
# hand-editing.
#
# macOS notes:
#   - Homebrew on Apple Silicon uses /opt/homebrew (Intel uses /usr/local)
#   - openssl@3, libxml2, curl are keg-only; pkg-config won't find them unless
#     we explicitly add their pkgconfig dirs to PKG_CONFIG_PATH
#   - The legacy Xiph m4 macros (ogg, vorbis, theora, speex) hard-code /usr/local
#     as fallback and do NOT call pkg-config, so we must pass --with-XXX=PREFIX
#   - autogen.sh calls glibtoolize on macOS (Homebrew libtool)
#
# Linux notes:
#   - Libraries are typically in /usr or /usr/local; pkg-config covers everything
#   - nproc is used for CPU count (not sysctl)
#   - No keg-only package special casing needed
#
# Usage:
#   ./build-macos.sh                      # installs to ./build/
#   ./build-macos.sh --prefix /usr/local  # install to a different prefix
#   ./build-macos.sh --clean              # clean before building
#   ./build-macos.sh --no-install         # compile only, skip install
#   ./build-macos.sh --regenerate         # force autogen.sh before configure
#   ./build-macos.sh --no-yaml            # build without libyaml support
#   ./build-macos.sh --no-ssl             # build without OpenSSL
#
# Author : David St John <davestj@gmail.com>
# Project: Mcaster1DNAS v2.5.2-dev
# Branch : macos-dev
# Date   : 2026-02-28
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# ─── Defaults ─────────────────────────────────────────────────────────────────
BUILD_PREFIX="${SCRIPT_DIR}/build"
DO_CLEAN=0
DO_INSTALL=1
DO_REGENERATE=0
WITH_YAML="--with-yaml"
WITH_SSL=1      # handled per-platform below

# ─── Parse arguments ──────────────────────────────────────────────────────────
while [[ $# -gt 0 ]]; do
    case "$1" in
        --prefix)      BUILD_PREFIX="$2"; shift 2 ;;
        --prefix=*)    BUILD_PREFIX="${1#*=}"; shift ;;
        --clean)       DO_CLEAN=1; shift ;;
        --no-install)  DO_INSTALL=0; shift ;;
        --regenerate)  DO_REGENERATE=1; shift ;;
        --no-yaml)     WITH_YAML=""; shift ;;
        --no-ssl)      WITH_SSL=0; shift ;;
        -h|--help)
            sed -n '3,28p' "$0" | sed 's/^# //'
            exit 0
            ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

# ─── OS detection ─────────────────────────────────────────────────────────────
OS_TYPE="$(uname -s)"
ARCH="$(uname -m)"
IS_MACOS=0
IS_LINUX=0

case "$OS_TYPE" in
    Darwin)
        IS_MACOS=1
        OS_LABEL="macOS (Darwin)"
        CPU_COUNT=$(sysctl -n hw.logicalcpu 2>/dev/null || echo 4)
        ;;
    Linux)
        IS_LINUX=1
        OS_LABEL="Linux"
        CPU_COUNT=$(nproc 2>/dev/null || grep -c ^processor /proc/cpuinfo 2>/dev/null || echo 4)
        ;;
    *)
        echo "WARNING: Unknown OS '$OS_TYPE'. Proceeding with Linux defaults."
        IS_LINUX=1
        OS_LABEL="Unknown ($OS_TYPE)"
        CPU_COUNT=4
        ;;
esac

echo "=========================================="
echo "  Mcaster1DNAS Cross-Platform Build"
echo "  Version : 2.5.2-dev"
echo "  OS      : ${OS_LABEL}"
echo "  Arch    : ${ARCH}"
echo "  Prefix  : ${BUILD_PREFIX}"
echo "  Jobs    : ${CPU_COUNT}"
echo "=========================================="
echo ""

# ─── macOS: locate Homebrew and verify packages ───────────────────────────────
if [ "$IS_MACOS" -eq 1 ]; then
    if ! command -v brew &>/dev/null; then
        echo "ERROR: Homebrew is required on macOS. Install from https://brew.sh"
        exit 1
    fi

    HOMEBREW_PREFIX="$(brew --prefix)"
    echo "  Homebrew prefix: ${HOMEBREW_PREFIX}"
    echo ""

    echo "[1/6] Checking Homebrew dependencies..."
    MISSING=()
    REQUIRED_BREW=(
        "autoconf"
        "automake"
        "libtool"
        "pkg-config"
        "libogg"
        "libvorbis"
        "theora"
        "speex"
        "openssl@3"
        "libxml2"
        "libxslt"
        "libyaml"
        "curl"
    )
    for pkg in "${REQUIRED_BREW[@]}"; do
        if ! brew list --formula "$pkg" &>/dev/null; then
            MISSING+=("$pkg")
        fi
    done
    if [ ${#MISSING[@]} -gt 0 ]; then
        echo ""
        echo "  ERROR: Missing required Homebrew packages:"
        echo "    ${MISSING[*]}"
        echo ""
        echo "  Install them with:"
        echo "    brew install ${MISSING[*]}"
        echo ""
        exit 1
    fi
    echo "  All required Homebrew packages found."

    # We build PKG_CONFIG_PATH to include keg-only packages that Homebrew does not
    # link into its main prefix because macOS ships system versions.
    EXTRA_PKG_CONFIG="${HOMEBREW_PREFIX}/lib/pkgconfig"
    EXTRA_PKG_CONFIG="${EXTRA_PKG_CONFIG}:${HOMEBREW_PREFIX}/opt/openssl@3/lib/pkgconfig"
    EXTRA_PKG_CONFIG="${EXTRA_PKG_CONFIG}:${HOMEBREW_PREFIX}/opt/curl/lib/pkgconfig"
    EXTRA_PKG_CONFIG="${EXTRA_PKG_CONFIG}:${HOMEBREW_PREFIX}/opt/libxml2/lib/pkgconfig"
    EXTRA_PKG_CONFIG="${EXTRA_PKG_CONFIG}:${HOMEBREW_PREFIX}/opt/libxslt/lib/pkgconfig"
    EXTRA_PKG_CONFIG="${EXTRA_PKG_CONFIG}:${HOMEBREW_PREFIX}/opt/libyaml/lib/pkgconfig"

    # CPPFLAGS and LDFLAGS cover both the main tree and keg-only formulae.
    EXTRA_CPPFLAGS="-I${HOMEBREW_PREFIX}/include"
    EXTRA_CPPFLAGS="${EXTRA_CPPFLAGS} -I${HOMEBREW_PREFIX}/opt/openssl@3/include"
    EXTRA_CPPFLAGS="${EXTRA_CPPFLAGS} -I${HOMEBREW_PREFIX}/opt/libxml2/include/libxml2"
    EXTRA_CPPFLAGS="${EXTRA_CPPFLAGS} -I${HOMEBREW_PREFIX}/opt/curl/include"
    EXTRA_CPPFLAGS="${EXTRA_CPPFLAGS} -I${HOMEBREW_PREFIX}/opt/libyaml/include"

    EXTRA_LDFLAGS="-L${HOMEBREW_PREFIX}/lib"
    EXTRA_LDFLAGS="${EXTRA_LDFLAGS} -L${HOMEBREW_PREFIX}/opt/openssl@3/lib"
    EXTRA_LDFLAGS="${EXTRA_LDFLAGS} -L${HOMEBREW_PREFIX}/opt/libxml2/lib"
    EXTRA_LDFLAGS="${EXTRA_LDFLAGS} -L${HOMEBREW_PREFIX}/opt/curl/lib"
    EXTRA_LDFLAGS="${EXTRA_LDFLAGS} -L${HOMEBREW_PREFIX}/opt/libyaml/lib"

    # The legacy Xiph m4 macros for ogg/vorbis/theora/speex do not use pkg-config.
    # We must pass --with-XXX= so they probe the correct Homebrew prefix.
    XIPH_PREFIX_FLAGS="--with-ogg=${HOMEBREW_PREFIX} --with-vorbis=${HOMEBREW_PREFIX} --with-theora=${HOMEBREW_PREFIX} --with-speex=${HOMEBREW_PREFIX}"

    # xiph_xml2.m4 uses AC_PATH_PROGS to find xslt-config and xml2-config by
    # scanning $PATH.  /usr/bin/xslt-config (system SDK) comes before Homebrew's
    # version, so configure picks up the system library, which is compiled against
    # the system libxml2.  Our binary links against Homebrew's libxml2 2.15+, so
    # mixing the two causes an ABI mismatch (different xmlBuf layouts) → SIGBUS.
    # Passing --with-xslt-config / --with-xml-config bypasses the PATH search.
    HOMEBREW_XSLT_CONFIG="${HOMEBREW_PREFIX}/opt/libxslt/bin/xslt-config"
    HOMEBREW_XML_CONFIG="${HOMEBREW_PREFIX}/opt/libxml2/bin/xml2-config"
    XML_CONFIG_FLAGS=""
    if [ -x "$HOMEBREW_XSLT_CONFIG" ]; then
        XML_CONFIG_FLAGS="${XML_CONFIG_FLAGS} --with-xslt-config=${HOMEBREW_XSLT_CONFIG}"
        echo "  Using Homebrew xslt-config: ${HOMEBREW_XSLT_CONFIG}"
    else
        echo "  WARNING: Homebrew xslt-config not found; system libxslt may be used (ABI risk)"
    fi
    if [ -x "$HOMEBREW_XML_CONFIG" ]; then
        XML_CONFIG_FLAGS="${XML_CONFIG_FLAGS} --with-xml-config=${HOMEBREW_XML_CONFIG}"
        echo "  Using Homebrew xml2-config: ${HOMEBREW_XML_CONFIG}"
    else
        echo "  WARNING: Homebrew xml2-config not found; system libxml2 may be used"
    fi

    # macOS glibtoolize (Homebrew libtool) must be used by autogen.sh.
    LIBTOOLIZE_CMD="${HOMEBREW_PREFIX}/opt/libtool/bin/glibtoolize"
    if [ ! -x "$LIBTOOLIZE_CMD" ]; then
        # Fall back to whatever is in PATH
        LIBTOOLIZE_CMD="$(command -v glibtoolize 2>/dev/null || command -v libtoolize)"
    fi
    export LIBTOOLIZE="$LIBTOOLIZE_CMD"

# ─── Linux: use system packages via pkg-config ────────────────────────────────
else
    echo "[1/6] Checking system dependencies..."
    MISSING=()
    REQUIRED_PKGS=(
        "libogg"
        "libvorbis"
        "theora"
        "speex"
        "openssl"
        "libxml-2.0"
        "libxslt"
        "yaml-0.1"
        "libcurl"
    )
    for pc in "${REQUIRED_PKGS[@]}"; do
        pkg-config --exists "$pc" 2>/dev/null || MISSING+=("$pc")
    done
    if [ ${#MISSING[@]} -gt 0 ]; then
        echo ""
        echo "  ERROR: Missing pkg-config packages:"
        echo "    ${MISSING[*]}"
        echo ""
        echo "  Install them on Debian/Ubuntu with:"
        echo "    sudo apt-get install libogg-dev libvorbis-dev libtheora-dev libspeex-dev \\"
        echo "      libssl-dev libxml2-dev libxslt1-dev libyaml-dev libcurl4-openssl-dev"
        echo ""
        echo "  Install them on RHEL/Fedora with:"
        echo "    sudo dnf install libogg-devel libvorbis-devel libtheora-devel speex-devel \\"
        echo "      openssl-devel libxml2-devel libxslt-devel libyaml-devel libcurl-devel"
        echo ""
        exit 1
    fi
    echo "  All required system packages found."

    # On Linux, pkg-config handles discovery. We still pass --with-XXX for compatibility
    # with the legacy Xiph m4 macros, but let them default to standard prefix.
    EXTRA_PKG_CONFIG="${PKG_CONFIG_PATH:-}"
    EXTRA_CPPFLAGS="${CPPFLAGS:-}"
    EXTRA_LDFLAGS="${LDFLAGS:-}"
    XIPH_PREFIX_FLAGS=""
    XML_CONFIG_FLAGS=""   # Linux: pkg-config / AC_PATH_PROGS find the right version
    LIBTOOLIZE_CMD="$(command -v libtoolize 2>/dev/null || echo libtoolize)"
    export LIBTOOLIZE="$LIBTOOLIZE_CMD"
fi

echo ""

# ─── Step 2: Optional clean ───────────────────────────────────────────────────
if [ "$DO_CLEAN" -eq 1 ]; then
    echo "[2/6] Cleaning previous build artifacts..."
    make clean 2>/dev/null || true
    rm -rf autom4te.cache
    rm -f config.status config.log config.h stamp-h1
    echo "  Clean complete."
else
    echo "[2/6] Skipping clean (use --clean to enable)."
fi
echo ""

# ─── Step 3: Regenerate build system ──────────────────────────────────────────
if [ "$DO_REGENERATE" -eq 1 ] || [ ! -f configure ]; then
    echo "[3/6] Regenerating build system (autogen.sh)..."
    echo "  Using libtoolize: ${LIBTOOLIZE}"
    ./autogen.sh
    echo "  autogen.sh complete."
else
    echo "[3/6] configure already exists (use --regenerate to force autogen.sh)."
fi
echo ""

# ─── Step 4: Configure ────────────────────────────────────────────────────────
echo "[4/6] Running configure..."
echo ""

SSL_FLAG=""
if [ "$WITH_SSL" -eq 0 ]; then
    SSL_FLAG="--without-openssl"
fi

PKG_CONFIG_PATH="${EXTRA_PKG_CONFIG}" \
CPPFLAGS="${EXTRA_CPPFLAGS}" \
LDFLAGS="${EXTRA_LDFLAGS}" \
./configure \
    --prefix="${BUILD_PREFIX}" \
    ${XIPH_PREFIX_FLAGS} \
    ${XML_CONFIG_FLAGS} \
    ${WITH_YAML} \
    ${SSL_FLAG}

echo ""
echo "  Configure complete."
echo ""

# ─── Step 5: Compile ──────────────────────────────────────────────────────────
echo "[5/6] Compiling with ${CPU_COUNT} parallel jobs..."
make -j"${CPU_COUNT}"
echo "  Compilation complete."
echo ""

# ─── Step 6: Install ──────────────────────────────────────────────────────────
if [ "$DO_INSTALL" -eq 1 ]; then
    echo "[6/6] Installing to ${BUILD_PREFIX}..."
    mkdir -p "${BUILD_PREFIX}/bin" "${BUILD_PREFIX}/logs"
    make install

    # We always copy the freshly-compiled binary directly over any stale binary
    # that may exist from a previous Linux build or a different architecture.
    # make install can leave behind old binaries if libtool wrapper resolution
    # has any ambiguity, so we do an explicit copy as the authoritative step.
    if [ -f "src/mcaster1" ]; then
        cp src/mcaster1 "${BUILD_PREFIX}/bin/mcaster1"
        chmod 755 "${BUILD_PREFIX}/bin/mcaster1"
        echo "  Copied src/mcaster1 → ${BUILD_PREFIX}/bin/mcaster1"
        echo "  Arch: $(file ${BUILD_PREFIX}/bin/mcaster1 | awk -F: '{print $2}' | xargs)"
        # On macOS (Apple Silicon), replacing a binary that the kernel has already
        # evaluated can leave a stale code-directory cache entry.  Force an ad-hoc
        # re-sign so macOS recognises the new content and allows the process to start.
        if [ "$IS_MACOS" -eq 1 ] && command -v codesign &>/dev/null; then
            codesign --force --sign - "${BUILD_PREFIX}/bin/mcaster1" 2>/dev/null \
                && echo "  Ad-hoc signed (macOS ARM64 code-directory refresh)." \
                || echo "  WARNING: codesign failed — binary may not start on Apple Silicon."
        fi
    fi
    echo "  Installation complete."
else
    echo "[6/6] Skipping install (--no-install was set)."
    # Even without a full install, copy the binary so it is immediately runnable.
    mkdir -p "${BUILD_PREFIX}/bin"
    if [ -f "src/mcaster1" ]; then
        cp src/mcaster1 "${BUILD_PREFIX}/bin/mcaster1"
        chmod 755 "${BUILD_PREFIX}/bin/mcaster1"
        echo "  Binary copied to ${BUILD_PREFIX}/bin/mcaster1 (no full install)."
        if [ "$IS_MACOS" -eq 1 ] && command -v codesign &>/dev/null; then
            codesign --force --sign - "${BUILD_PREFIX}/bin/mcaster1" 2>/dev/null \
                && echo "  Ad-hoc signed (macOS ARM64 code-directory refresh)." \
                || echo "  WARNING: codesign failed — binary may not start on Apple Silicon."
        fi
    fi
fi
echo ""

# ─── Summary ──────────────────────────────────────────────────────────────────
BINARY="${BUILD_PREFIX}/bin/mcaster1"
echo "=========================================="
echo "  Build Complete!"
echo "=========================================="
echo ""
if [ -f "${BINARY}" ]; then
    echo "  Binary : ${BINARY}"
    echo "  Version: $("${BINARY}" --version 2>&1 | head -1)"
fi
echo ""
echo "  Next steps:"
echo "    1. Copy and edit a config:  cp mcaster1dnas-console.yaml my.yaml"
echo "    2. Start server:            ${BINARY} -c my.yaml"
echo "    3. Open web UI:             open http://127.0.0.1:9330/status.xsl"
echo ""
echo "  See README-MACOS-BUILD.html for full instructions."
echo ""

#!/bin/bash
#
# build-macos-gui.sh — Build the Qt6 macOS/Linux GUI for Mcaster1DNAS
#
# Prerequisites:
#   macOS:  brew install qt (already done)
#           Server must have been configured first (config.h must exist in ../)
#           Run ../build-macos.sh at least once before this script.
#   Linux:  Install qt6-base-dev qt6-tools-dev (Ubuntu/Debian)
#           or qt6-qtbase-devel (Fedora/RHEL)
#
# Usage:
#   ./build-macos-gui.sh                 # build in ./build-qt/
#   ./build-macos-gui.sh --clean         # clean before building
#   ./build-macos-gui.sh --no-deploy     # skip macdeployqt (faster iteration)
#   ./build-macos-gui.sh --release       # Release build (default: Debug)
#
# Output:
#   macOS: ./build-qt/Mcaster1DNAS.app (signed ad-hoc)
#   Linux: ./build-qt/Mcaster1DNAS (executable)
#
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

BUILD_DIR="${SCRIPT_DIR}/build-qt"
BUILD_TYPE="Debug"
DO_CLEAN=0
DO_DEPLOY=1

while [[ $# -gt 0 ]]; do
    case "$1" in
        --clean)    DO_CLEAN=1; shift ;;
        --release)  BUILD_TYPE=Release; shift ;;
        --no-deploy) DO_DEPLOY=0; shift ;;
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

OS_TYPE="$(uname -s)"

echo "========================================"
echo "  Mcaster1DNAS Qt6 GUI Build"
echo "  OS        : ${OS_TYPE}"
echo "  Build type: ${BUILD_TYPE}"
echo "  Build dir : ${BUILD_DIR}"
echo "========================================"
echo ""

# ── Verify config.h exists (server must be configured first) ──────────────────
if [ ! -f "${SCRIPT_DIR}/../config.h" ]; then
    echo "ERROR: ${SCRIPT_DIR}/../config.h not found."
    echo "Please run  cd .. && ./build-macos.sh  (or ./configure) first."
    exit 1
fi
echo "[1/4] config.h found — server build system is configured."

# ── Locate Qt ─────────────────────────────────────────────────────────────────
if [ "$OS_TYPE" = "Darwin" ]; then
    QT_PREFIX="$(brew --prefix qt 2>/dev/null)"
    if [ -z "$QT_PREFIX" ]; then
        echo "ERROR: Qt not found via Homebrew.  brew install qt"
        exit 1
    fi
    CMAKE_PREFIX_PATH="$QT_PREFIX"
    MACDEPLOYQT="${QT_PREFIX}/bin/macdeployqt"
    CPU_COUNT=$(sysctl -n hw.logicalcpu 2>/dev/null || echo 4)
else
    CMAKE_PREFIX_PATH="/usr"
    CPU_COUNT=$(nproc 2>/dev/null || echo 4)
fi

# ── Optional clean ─────────────────────────────────────────────────────────────
if [ "$DO_CLEAN" -eq 1 ]; then
    echo "[2/4] Cleaning ${BUILD_DIR}..."
    rm -rf "$BUILD_DIR"
fi
mkdir -p "$BUILD_DIR"

# ── CMake configure ───────────────────────────────────────────────────────────
echo "[2/4] Running CMake configure..."
cmake -B "$BUILD_DIR" -S "$SCRIPT_DIR" \
    -DCMAKE_PREFIX_PATH="$CMAKE_PREFIX_PATH" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
echo "  Configure complete."
echo ""

# ── Build ─────────────────────────────────────────────────────────────────────
echo "[3/4] Building with ${CPU_COUNT} parallel jobs..."
cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j"$CPU_COUNT"
echo "  Build complete."
echo ""

# ── macOS: deploy Qt frameworks + codesign ────────────────────────────────────
if [ "$OS_TYPE" = "Darwin" ]; then
    APP="${BUILD_DIR}/Mcaster1DNAS.app"
    if [ ! -d "$APP" ]; then
        echo "ERROR: ${APP} not found after build!"
        exit 1
    fi

    if [ "$DO_DEPLOY" -eq 1 ] && [ -x "$MACDEPLOYQT" ]; then
        echo "[4/4] Running macdeployqt..."
        "$MACDEPLOYQT" "$APP" -verbose=1
        echo "  macdeployqt complete."
    fi

    echo "  Ad-hoc signing (Apple Silicon code-directory refresh)..."
    codesign --force --sign - "$APP" 2>/dev/null \
        && echo "  Signed OK." \
        || echo "  WARNING: codesign failed."

    echo ""
    echo "========================================"
    echo "  macOS GUI Build Complete!"
    echo "  App: ${APP}"
    echo "========================================"
    echo ""
    echo "  Run:   open ${APP}"
    echo "  Or:    ${APP}/Contents/MacOS/Mcaster1DNAS -c ../mcaster1dnas-macos.yaml"
else
    echo "[4/4] Linux build — no deployment step needed."
    BIN="${BUILD_DIR}/Mcaster1DNAS"
    echo ""
    echo "========================================"
    echo "  Linux GUI Build Complete!"
    echo "  Binary: ${BIN}"
    echo "========================================"
    echo ""
    echo "  Run:   ${BIN} -c ../mcaster1dnas-console.yaml"
fi
echo ""

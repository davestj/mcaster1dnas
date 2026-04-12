#!/bin/bash
# =============================================================================
# Mcaster1DNAS — Debian/Ubuntu .deb Package Builder
#
# Builds mcaster1dnas_<version>_amd64.deb from source tree.
#
# Usage:
#   cd install/deb
#   ./build-deb.sh                  # Auto-detect version from ../../VERSION
#   ./build-deb.sh 2.5.3-sec        # Override version
#
# Supported targets:
#   Debian 12 (Bookworm), Debian 11 (Bullseye)
#   Ubuntu 22.04 (Jammy), Ubuntu 24.04 (Noble)
#
# Requirements:
#   dpkg-deb, fakeroot, build-essential, autoconf, automake, libtool
#   + all Mcaster1DNAS build dependencies (see debian/control)
#
# Copyright 2026 MediaCast1 LLC — David St John <davestj@mcaster1.com>
# Licensed under GNU GPL v2
# =============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"

# Version
if [ -n "${1:-}" ]; then
    VERSION="$1"
else
    VERSION="$(cat "${REPO_ROOT}/VERSION" 2>/dev/null || echo "0.0.0")"
fi

# Strip any leading 'v'
VERSION="${VERSION#v}"

PKG_NAME="mcaster1dnas"
PKG_ARCH="amd64"
PKG_DIR="${SCRIPT_DIR}/${PKG_NAME}_${VERSION}_${PKG_ARCH}"
INSTALL_PREFIX="/usr"
CONF_DIR="/etc/mcaster1dnas"
DATA_DIR="/usr/share/mcaster1dnas"
LOG_DIR="/var/log/mcaster1dnas"
RUN_DIR="/run/mcaster1dnas"

echo "=========================================="
echo "  Mcaster1DNAS .deb Package Builder"
echo "  Version:  ${VERSION}"
echo "  Arch:     ${PKG_ARCH}"
echo "  Output:   ${PKG_DIR}.deb"
echo "=========================================="

# ---- Step 1: Build from source ----
echo ""
echo "[1/5] Building from source..."
cd "${REPO_ROOT}"

if [ ! -f configure ]; then
    autoreconf -fi
fi

CFLAGS="-g -O2 -fstack-protector-strong -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security" \
LDFLAGS="-Wl,-z,relro,-z,now" \
./configure --prefix="${INSTALL_PREFIX}" \
            --sysconfdir="/etc" \
            --localstatedir="/var" \
            --with-openssl \
            --with-curl \
            --with-ogg \
            --with-vorbis \
            --with-theora \
            --with-speex

make clean
make -j"$(nproc)"

# ---- Step 2: Create package directory structure ----
echo ""
echo "[2/5] Creating package structure..."
rm -rf "${PKG_DIR}"

# Binary
install -Dm755 src/mcaster1 "${PKG_DIR}${INSTALL_PREFIX}/bin/mcaster1"

# Binary GPG signature (if present)
if [ -f src/mcaster1.sig ]; then
    install -Dm644 src/mcaster1.sig "${PKG_DIR}${INSTALL_PREFIX}/bin/mcaster1.sig"
    echo "  Binary signature included"
fi

# Configuration (marked as conffiles so dpkg preserves user edits)
install -Dm640 "${SCRIPT_DIR}/debian/mcaster1dnas.yaml" "${PKG_DIR}${CONF_DIR}/mcaster1dnas.yaml"

# Systemd service
install -Dm644 "${SCRIPT_DIR}/debian/mcaster1-dnas.service" "${PKG_DIR}/lib/systemd/system/mcaster1-dnas.service"

# Web interface
mkdir -p "${PKG_DIR}${DATA_DIR}"
cp -r web "${PKG_DIR}${DATA_DIR}/web"
cp -r admin "${PKG_DIR}${DATA_DIR}/admin"

# Documentation
mkdir -p "${PKG_DIR}${INSTALL_PREFIX}/share/doc/${PKG_NAME}"
cp README.md "${PKG_DIR}${INSTALL_PREFIX}/share/doc/${PKG_NAME}/" 2>/dev/null || true
cp CHANGELOG.md "${PKG_DIR}${INSTALL_PREFIX}/share/doc/${PKG_NAME}/" 2>/dev/null || true
cp LICENSE.md "${PKG_DIR}${INSTALL_PREFIX}/share/doc/${PKG_NAME}/copyright" 2>/dev/null || true
if [ -d docs ]; then
    cp -r docs "${PKG_DIR}${DATA_DIR}/docs"
fi

# Log directory (created by postinst, but define structure)
mkdir -p "${PKG_DIR}${LOG_DIR}"

# PID directory (tmpfiles.d)
install -Dm644 "${SCRIPT_DIR}/debian/mcaster1dnas.tmpfiles" \
    "${PKG_DIR}${INSTALL_PREFIX}/lib/tmpfiles.d/mcaster1dnas.conf"

# Logrotate
install -Dm644 "${SCRIPT_DIR}/debian/mcaster1dnas.logrotate" \
    "${PKG_DIR}/etc/logrotate.d/mcaster1dnas"

# ---- Step 3: Create DEBIAN control files ----
echo ""
echo "[3/5] Creating DEBIAN control files..."
mkdir -p "${PKG_DIR}/DEBIAN"

# Calculate installed size in KB
INSTALLED_SIZE=$(du -sk "${PKG_DIR}" | cut -f1)

# Generate control file
sed -e "s/@VERSION@/${VERSION}/g" \
    -e "s/@ARCH@/${PKG_ARCH}/g" \
    -e "s/@INSTALLED_SIZE@/${INSTALLED_SIZE}/g" \
    "${SCRIPT_DIR}/debian/control.in" > "${PKG_DIR}/DEBIAN/control"

# conffiles — tell dpkg which files are user-editable config
cat > "${PKG_DIR}/DEBIAN/conffiles" << 'EOF'
/etc/mcaster1dnas/mcaster1dnas.yaml
EOF

# postinst script
install -m755 "${SCRIPT_DIR}/debian/postinst" "${PKG_DIR}/DEBIAN/postinst"

# prerm script
install -m755 "${SCRIPT_DIR}/debian/prerm" "${PKG_DIR}/DEBIAN/prerm"

# postrm script
install -m755 "${SCRIPT_DIR}/debian/postrm" "${PKG_DIR}/DEBIAN/postrm"

# ---- Step 4: Set permissions ----
echo ""
echo "[4/5] Setting permissions..."
# Binary
chmod 755 "${PKG_DIR}${INSTALL_PREFIX}/bin/mcaster1"
# Config must be readable by mcaster1 user
chmod 640 "${PKG_DIR}${CONF_DIR}/mcaster1dnas.yaml"
# Web files readable
find "${PKG_DIR}${DATA_DIR}" -type f -exec chmod 644 {} \;
find "${PKG_DIR}${DATA_DIR}" -type d -exec chmod 755 {} \;

# ---- Step 5: Build the .deb ----
echo ""
echo "[5/5] Building .deb package..."
cd "${SCRIPT_DIR}"
fakeroot dpkg-deb --build "${PKG_DIR}"

# Show result
DEB_FILE="${PKG_DIR}.deb"
if [ -f "${DEB_FILE}" ]; then
    echo ""
    echo "=========================================="
    echo "  SUCCESS"
    echo "  Package: ${DEB_FILE}"
    echo "  Size:    $(du -h "${DEB_FILE}" | cut -f1)"
    echo ""
    echo "  Install:"
    echo "    sudo dpkg -i ${DEB_FILE}"
    echo "    sudo apt-get -f install    # resolve deps"
    echo ""
    echo "  Or use apt directly:"
    echo "    sudo apt install ./${PKG_NAME}_${VERSION}_${PKG_ARCH}.deb"
    echo "=========================================="
    dpkg-deb --info "${DEB_FILE}"
else
    echo "ERROR: .deb file not created"
    exit 1
fi

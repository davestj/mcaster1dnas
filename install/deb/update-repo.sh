#!/bin/bash
# =============================================================================
# Mcaster1DNAS — Update Debian APT Repository
#
# Regenerates Packages, Release, InRelease, and Release.gpg from the pool.
# Run after adding/removing .deb files from the pool directory.
#
# Usage:
#   ./update-repo.sh                    # Uses default paths
#   REPO=/path/to/repo ./update-repo.sh # Override repo path
#   GPG_KEY=KEYID ./update-repo.sh      # Override signing key
#
# Copyright 2026 MediaCast1 LLC
# =============================================================================

set -euo pipefail

REPO="${REPO:-/var/www/mcaster1.com/html/repos/debian}"
GPG_KEY="${GPG_KEY:-939D89481C7B121E}"

echo "Mcaster1DNAS Repo Update"
echo "  Repo: ${REPO}"
echo "  Key:  ${GPG_KEY}"
echo ""

cd "${REPO}"

# Regenerate Packages index
echo "[1/4] Generating Packages..."
apt-ftparchive packages pool/main/ > dists/stable/main/binary-amd64/Packages
gzip -9c dists/stable/main/binary-amd64/Packages > dists/stable/main/binary-amd64/Packages.gz
xz -9c dists/stable/main/binary-amd64/Packages > dists/stable/main/binary-amd64/Packages.xz 2>/dev/null || true

# Count packages
PKG_COUNT=$(grep -c "^Package:" dists/stable/main/binary-amd64/Packages || echo 0)
echo "  ${PKG_COUNT} package(s) indexed"

# Regenerate Release
echo "[2/4] Generating Release..."
apt-ftparchive -c conf/apt-release.conf release dists/stable > dists/stable/Release

# Sign: InRelease (inline)
echo "[3/4] Signing InRelease..."
rm -f dists/stable/InRelease
gpg --default-key "${GPG_KEY}" --clearsign --output dists/stable/InRelease dists/stable/Release

# Sign: Release.gpg (detached)
echo "[4/4] Signing Release.gpg..."
rm -f dists/stable/Release.gpg
gpg --default-key "${GPG_KEY}" --armor --detach-sign --output dists/stable/Release.gpg dists/stable/Release

echo ""
echo "Done. Repository updated with ${PKG_COUNT} package(s)."
echo ""
echo "Users can install with:"
echo "  curl -fsSL https://mcaster1.com/repos/debian/mcaster1-keysmith.gpg | sudo tee /usr/share/keyrings/mcaster1-keysmith.gpg > /dev/null"
echo "  echo 'deb [signed-by=/usr/share/keyrings/mcaster1-keysmith.gpg] https://mcaster1.com/repos/debian stable main' | sudo tee /etc/apt/sources.list.d/mcaster1.list"
echo "  sudo apt update && sudo apt install mcaster1dnas"

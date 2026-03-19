#!/bin/bash
#
# Mcaster1DNAS Build Script
# Automates the complete build process
#

set -e  # Exit on any error

echo "========================================="
echo "  Mcaster1DNAS Build Script v2.5.0"
echo "========================================="
echo ""

# Set build directory
BUILD_PREFIX="/var/www/mcaster1.com/mcaster1dnas/build"
SOURCE_DIR="/var/www/mcaster1.com/mcaster1dnas"

cd "$SOURCE_DIR"

# Step 1: Clean old build artifacts
echo "[1/6] Cleaning previous build artifacts..."
rm -rf autom4te.cache config.h config.h.in~ config.status
make clean 2>/dev/null || true
echo "✓ Clean complete"
echo ""

# Step 2: Regenerate build system
echo "[2/6] Regenerating build system (autogen.sh)..."
./autogen.sh
echo "✓ Build system regenerated"
echo ""

# Step 3: Configure
echo "[3/6] Configuring with all features..."
./configure \
  --prefix="$BUILD_PREFIX" \
  --with-openssl \
  --with-ogg \
  --with-vorbis \
  --with-theora \
  --with-speex \
  --with-curl
echo "✓ Configuration complete"
echo ""

# Step 4: Compile
echo "[4/6] Compiling (using $(nproc) CPU cores)..."
make -j$(nproc)
echo "✓ Compilation complete"
echo ""

# Step 5: Install
echo "[5/6] Installing to $BUILD_PREFIX..."
make install
echo "✓ Installation complete"
echo ""

# Step 6: Post-install setup
echo "[6/6] Setting up directories and permissions..."

# Create log directory
mkdir -p "$BUILD_PREFIX/logs"
chmod 755 "$BUILD_PREFIX/logs"

# Verify SSL certificate
if [ -f "$SOURCE_DIR/ssl/mcaster1dnas.pem" ]; then
    echo "✓ SSL certificate found at $SOURCE_DIR/ssl/mcaster1dnas.pem"
    chmod 600 "$SOURCE_DIR/ssl/mcaster1dnas.pem"
else
    echo "⚠ WARNING: SSL certificate not found at $SOURCE_DIR/ssl/mcaster1dnas.pem"
fi

# Verify configuration file
if [ -f "$SOURCE_DIR/mcaster1-production.xml" ]; then
    echo "✓ Production config found at $SOURCE_DIR/mcaster1-production.xml"
else
    echo "⚠ WARNING: Production config not found"
fi

echo ""
echo "========================================="
echo "  Build Complete! ✓"
echo "========================================="
echo ""
echo "Binary location: $BUILD_PREFIX/bin/mcaster1"
echo "Version: $($BUILD_PREFIX/bin/mcaster1 --version 2>&1 | head -1)"
echo ""
echo "Next steps:"
echo "  1. Edit passwords in: $SOURCE_DIR/mcaster1-production.xml"
echo "  2. Start server: $BUILD_PREFIX/bin/mcaster1 -c $SOURCE_DIR/mcaster1-production.xml -b"
echo "  3. Check logs: tail -f $BUILD_PREFIX/logs/error.log"
echo "  4. Access web interface: https://15.204.91.208:9443/"
echo ""
echo "For detailed instructions, see: BUILD_AND_RUN.md"
echo ""

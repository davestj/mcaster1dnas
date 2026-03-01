#!/bin/sh
# Run this to set up the build system: configure, makefiles, etc.
# (based on the version in enlightenment's cvs)
#
# We detect macOS (Darwin) and Linux and select the correct libtoolize
# command for each platform. On macOS with Homebrew, GNU libtool ships as
# glibtoolize. On Linux, it is libtoolize. The $LIBTOOLIZE environment
# variable overrides autodetection when set by the caller (e.g. build-macos.sh).

package="mcaster1"

olddir=`pwd`
srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

cd "$srcdir"
DIE=0

# ── Detect OS and pick the correct libtoolize ─────────────────────────────────
OS_TYPE="$(uname -s 2>/dev/null)"
if test -z "$LIBTOOLIZE"; then
    case "$OS_TYPE" in
        Darwin)
            # We prefer the Homebrew-installed glibtoolize over the macOS system stub.
            if command -v glibtoolize > /dev/null 2>&1; then
                LIBTOOLIZE=glibtoolize
            elif command -v libtoolize > /dev/null 2>&1; then
                LIBTOOLIZE=libtoolize
            else
                LIBTOOLIZE=""
            fi
            ;;
        *)
            # Linux and other POSIX systems use plain libtoolize.
            if command -v libtoolize > /dev/null 2>&1; then
                LIBTOOLIZE=libtoolize
            else
                LIBTOOLIZE=""
            fi
            ;;
    esac
fi

(autoconf --version) < /dev/null > /dev/null 2>&1 || {
        echo
        echo "You must have autoconf installed to compile $package."
        echo "Download the appropriate package for your distribution,"
        echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
        DIE=1
}
VERSIONGREP="sed -e s/.*[^0-9\.]\([0-9]\.[0-9]\).*/\1/"

# do we need automake?
if test -r Makefile.am; then
    echo Checking for automake version
    options=`fgrep AUTOMAKE_OPTIONS Makefile.am`
    AM_NEEDED=`echo "$options" | $VERSIONGREP`
    AM_PROGS=automake
    AC_PROGS=aclocal
    if test -n "$AM_NEEDED" && test "x$AM_NEEDED" != "x$options"
    then
        AM_PROGS="automake-$AM_NEEDED automake$AM_NEEDED $AM_PROGS"
        AC_PROGS="aclocal-$AM_NEEDED aclocal$AM_NEEDED $AC_PROGS"
    else
        AM_NEEDED=""
    fi
    AM_PROGS="$AUTOMAKE $AM_PROGS"
    AC_PROGS="$ACLOCAL $AC_PROGS"
    for am in $AM_PROGS; do
      ($am --version > /dev/null 2>&1) 2>/dev/null || continue
      ver=`$am --version | head -1 | $VERSIONGREP`
      AWK_RES=`echo $ver $AM_NEEDED | awk '{ if ( $1 >= $2 ) print "yes"; else print "no" }'`
      if test "$AWK_RES" = "yes"; then
        AUTOMAKE=$am
        echo "  found $AUTOMAKE"
        break
      fi
    done
    for ac in $AC_PROGS; do
      ($ac --version > /dev/null 2>&1) 2>/dev/null || continue
      ver=`$ac --version < /dev/null | head -1 | $VERSIONGREP`
      AWK_RES=`echo $ver $AM_NEEDED | awk '{ if ( $1 >= $2 ) print "yes"; else print "no" }'`
      if test "$AWK_RES" = "yes"; then
        ACLOCAL=$ac
        echo "  found $ACLOCAL"
        break
      fi
    done
    test -z $AUTOMAKE || test -z $ACLOCAL && {
        echo
        if test -n "$AM_NEEDED"; then
            echo "You must have automake version $AM_NEEDED installed"
            echo "to compile $package."
        else
            echo "You must have automake installed to compile $package."
        fi
        echo "Download the appropriate package for your distribution,"
        echo "or get the source tarball at ftp://ftp.gnu.org/pub/gnu/"
        DIE=1
      }
fi

if test -z "$LIBTOOLIZE"; then
    echo
    echo "You must have libtool installed to compile $package."
    echo "  Linux:  sudo apt-get install libtool  (or equivalent)"
    echo "  macOS:  brew install libtool"
    echo ""
    echo "Note: on macOS with Homebrew, glibtoolize is used instead of libtoolize."
    DIE=1
else
    ($LIBTOOLIZE --version) > /dev/null 2>&1 || {
        echo
        echo "libtoolize command '$LIBTOOLIZE' not functional."
        DIE=1
    }
fi

if test "$DIE" -eq 1; then
        exit 1
fi

echo "Generating configuration files for $package, please wait...."
echo "  OS         : $OS_TYPE"
echo "  libtoolize : $LIBTOOLIZE"

ACLOCAL_FLAGS="$ACLOCAL_FLAGS -I m4"
if test -n "$ACLOCAL"; then
  echo "  $ACLOCAL $ACLOCAL_FLAGS"
  $ACLOCAL $ACLOCAL_FLAGS
fi

echo "  autoheader"
autoheader

echo "  $LIBTOOLIZE"
$LIBTOOLIZE --automake --copy

if test -n "$AUTOMAKE"; then
  echo "  $AUTOMAKE"
  $AUTOMAKE --add-missing --copy --foreign
fi

echo "  autoconf"
autoconf

if test -z "$*"; then
        echo ""
        echo "Build system generated. Next step:"
        echo "  macOS:  ./build-macos.sh"
        echo "  Linux:  ./build-macos.sh    (also handles Linux)"
        echo "  Manual: ./configure [OPTIONS]"
fi
cd $olddir

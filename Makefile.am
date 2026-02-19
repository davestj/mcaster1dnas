## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = conf doc web admin src
if WINDOWS
SUBDIRS += windows
endif

BUILT_SOURCES = git_hash.h
dist_check_SCRIPTS = GIT-VERSION-GEN
CLEANFILES = git_hash.h

EXTRA_DIST = HACKING config.h.vc6 m4/acx_pthread.m4 m4/ogg.m4 \
    m4/theora.m4 m4/vorbis.m4 m4/speex.m4\
    m4/xiph_compiler.m4 m4/xiph_curl.m4 m4/xiph_net.m4 \
    m4/xiph_types.m4 m4/xiph_xml2.m4 m4/xiph_openssl.m4 mcaster1.spec \
    VERSION scripts/version-bump.sh

docdir = $(datadir)/doc/$(PACKAGE)
doc_DATA = README.md ChangeLog TODO.md \
	ICY2_PROTOCOL_SPEC.md YAML_IMPLEMENTATION.md YP_LOGGING_FEATURE.md \
	FORK.md BUILD_AND_RUN.md

git_hash.h: FORCE
	@$(SHELL_PATH) $(srcdir)/GIT-VERSION-GEN
FORCE:

# example build line
# make CFLAGS="-O2 -s -pipe -Wall -Wp,-D_FORTIFY_SOURCE=2 -fexceptions --param=ssp-buffer-size=4 -ffile-prefix-map=$(pwd)=" CXXFLAGS='$(CFLAGS)' V=1
# V=1 to show lines
# file-prefix-map to filter out absolute path lines in binary
#
debug:
	$(MAKE) all CFLAGS="-Wno-unused-parameter @DEBUG@"

win64:
	$(MAKE) all CFLAGS="-O2 -s" CXXFLAGS="-O2 -s"
win32:
	$(MAKE) all CFLAGS="-O2 -s" CXXFLAGS="-O2 -s"
profile:
	$(MAKE) all CFLAGS="@PROFILE@"

staticdebug:
	$(MAKE) all CFLAGS="-g -DLIBXML_STATIC -DCURL_STATICLIB" LDFLAGS="${LDFLAGS} -all-static"
static:
	$(MAKE) all CFLAGS="-O2 -DLIBXML_STATIC -DCURL_STATICLIB" LDFLAGS="${LDFLAGS} -static-libstdc++" CXXFLAGS=

# Version management targets
.PHONY: version version-info version-bump-major version-bump-minor version-bump-patch

version-info:
	@echo "=== Mcaster1DNAS Version Information ==="
	@echo "Base Version:  $$(cat VERSION 2>/dev/null || echo 'unknown')"
	@echo "Full Version:  $$(./GIT-VERSION-GEN 2>&1 | grep GIT_VERSION= | cut -d= -f2)"
	@echo "Git Branch:    $$(git rev-parse --abbrev-ref HEAD 2>/dev/null || echo 'unknown')"
	@echo "Git Commit:    $$(git rev-parse --short HEAD 2>/dev/null || echo 'unknown')"
	@echo "========================================"

version: version-info

version-bump-major:
	@echo "Bumping MAJOR version..."
	@bash scripts/version-bump.sh major

version-bump-minor:
	@echo "Bumping MINOR version..."
	@bash scripts/version-bump.sh minor

version-bump-patch:
	@echo "Bumping PATCH version..."
	@bash scripts/version-bump.sh patch

version-bump-auto:
	@echo "Auto-bumping version based on branch..."
	@bash scripts/version-bump.sh auto

## Process this with automake to create Makefile.in

SUBDIRS = avl thread httpp net log timing

noinst_HEADERS = admin.h cfgfile.h cfgfile_yaml.h logging.h sighandler.h connection.h \
    global.h util.h slave.h source.h stats.h refbuf.h client.h \
    compat.h fserve.h xslt.h yp.h event.h md5.h params.h icy2_meta.h \
    auth.h auth_htpasswd.h auth_cmd.h auth_url.h \
    fnmatch_loop.c fnmatch.h \
    format.h format_ogg.h format_mp3.h format_ebml.h \
    format_vorbis.h format_theora.h format_flac.h format_speex.h format_midi.h format_opus.h \
    format_kate.h format_skeleton.h mpeg.h flv.h
mcaster1_SOURCES = cfgfile.c main.c logging.c sighandler.c connection.c global.c \
    util.c slave.c source.c stats.c refbuf.c client.c params.c icy2_meta.c \
    xslt.c fserve.c event.c admin.c md5.c \
    format.c format_ogg.c format_mp3.c format_midi.c format_flac.c format_ebml.c format_opus.c \
    auth.c auth_htpasswd.c auth_url.c format_kate.c format_skeleton.c mpeg.c flv.c \
    songdata_api.c songdata_api.h

mcaster1_DEPENDENCIES = \
    net/libmcnet.la \
    thread/libmcthread.la \
    httpp/libmchttpp.la \
    log/libmclog.la \
    avl/libmcavl.la \
    timing/libmctiming.la

if BUILD_FNMATCH
mcaster1_SOURCES += fnmatch.c
endif
if BUILD_YP
mcaster1_SOURCES += yp.c
endif
if BUILD_VORBIS
mcaster1_SOURCES += format_vorbis.c
endif
if BUILD_SPEEX
mcaster1_SOURCES += format_speex.c
endif
if BUILD_THEORA
mcaster1_SOURCES += format_theora.c
endif
if BUILD_AUTHCMD
mcaster1_SOURCES += auth_cmd.c
endif
if HAVE_YAML
mcaster1_SOURCES += cfgfile_yaml.c
endif

if WINDOWS
noinst_LTLIBRARIES = libmcaster1.la
libmcaster1_la_SOURCES = $(mcaster1_SOURCES)
libmcaster1_la_CFLAGS = -static
libmcaster1_la_LIBADD = $(mcaster1_DEPENDENCIES) @XIPH_LIBS@ @KATE_LIBS@
libmcaster1_la_DEPENDENCIES = $(mcaster1_DEPENDENCIES)
else
bin_PROGRAMS = mcaster1
mcaster1_LDADD = $(mcaster1_DEPENDENCIES) @XIPH_LIBS@ @KATE_LIBS@
endif

AM_CFLAGS = @XIPH_CFLAGS@
AM_CPPFLAGS = @XIPH_CPPFLAGS@
AM_LDFLAGS = @XIPH_LDFLAGS@ @KATE_LIBS@


debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"


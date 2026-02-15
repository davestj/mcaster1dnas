## Process this with automake to create Makefile.in

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = BUILDING COPYING README TODO avl.dsp test.c

noinst_LTLIBRARIES = libmcavl.la
noinst_HEADERS = avl.h

libmcavl_la_SOURCES = avl.c
libmcavl_la_CFLAGS = @XIPH_CFLAGS@ -static

AM_CPPFLAGS = -I$(top_srcdir)/src

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"


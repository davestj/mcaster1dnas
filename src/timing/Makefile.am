## Process this with automake to create Makefile.in

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = BUILDING COPYING README TODO

noinst_LTLIBRARIES = libmctiming.la
noinst_HEADERS = timing.h

libmctiming_la_SOURCES = timing.c
libmctiming_la_CFLAGS = @XIPH_CFLAGS@ -static

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"


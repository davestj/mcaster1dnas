## Process this with automake to create Makefile.in

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = BUILDING COPYING README TODO

noinst_LTLIBRARIES = libmcthread.la
noinst_HEADERS = thread.h

libmcthread_la_SOURCES = thread.c
libmcthread_la_CFLAGS = @XIPH_CFLAGS@ -static

AM_CPPFLAGS = -I$(top_srcdir)/src

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"


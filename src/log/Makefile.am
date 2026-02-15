## Process this with automake to create Makefile.in

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = test.c

noinst_LTLIBRARIES = libmclog.la
noinst_HEADERS = log.h

libmclog_la_SOURCES = log.c
AM_CFLAGS = $(XIPH_CFLAGS) -static

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"


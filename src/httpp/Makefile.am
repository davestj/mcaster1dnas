## Process this with automake to create Makefile.in

AUTOMAKE_OPTIONS = foreign

noinst_LTLIBRARIES = libmchttpp.la
noinst_HEADERS = httpp.h

libmchttpp_la_SOURCES = httpp.c
libmchttpp_la_CFLAGS = @XIPH_CFLAGS@ -static
AM_CPPFLAGS = @XIPH_CPPFLAGS@ -I$(top_srcdir)/src

# SCCS stuff (for BitKeeper)
GET = true

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"


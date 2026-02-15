## Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = ConfigTab.cpp ConfigTab.h Mcaster1Win.clw Mcaster1Win.cpp \
    Mcaster1Win.dsp Mcaster1Win.dsw Mcaster1Win.h Mcaster1Win.rc \
    Mcaster1WinDlg.cpp Mcaster1WinDlg.h Makefile.am \
    StatsTab.cpp StatsTab.h Status.cpp Status.h StdAfx.cpp \
    StdAfx.h TabCtrlSSL.cpp TabCtrlSSL.h TabPageSSL.cpp TabPageSSL.h black.bmp \
    colors.h mcaster1.dsp mcaster1.ico mcaster1dnas.iss mcaster1dnaslogo2.bmp\
    resource.h running.bmp stopped.bmp TRAYNOT.h Traynot.cpp \
    mcaster1dnas_console.dsw mcaster1dnas_console.dsp credits.bmp mcaster1dnastitle.bmp \
    mcaster1Service.cpp mcaster1Service.dsp

bin_PROGRAMS = mcaster1
mcaster1_DEPENDENCIES = \
   $(top_builddir)/src/libmcaster1.la \
   $(top_builddir)/src/net/libmcnet.la \
   $(top_builddir)/src/thread/libmcthread.la \
   $(top_builddir)/src/httpp/libmchttpp.la \
   $(top_builddir)/src/log/libmclog.la \
   $(top_builddir)/src/avl/libmcavl.la \
   $(top_builddir)/src/timing/libmctiming.la

mcaster1_SOURCES = mcaster1Service.cpp
mcaster1_CPPFLAGS = -I$(top_srcdir)/src @XIPH_CFLAGS@ @XIPH_CPPFLAGS@
mcaster1_LDFLAGS = -static-libgcc
mcaster1_LDADD = $(mcaster1_DEPENDENCIES) @XIPH_LDFLAGS@ @XIPH_LIBS@ @KATE_LIBS@


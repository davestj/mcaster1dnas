## Process this with automake to create Makefile.in

AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = mcaster1.xml.in mcaster1_minimal.xml.in mcaster1_shoutcast_compat.xml.in mcaster1_auth.php
DISTCLEANFILES = mcaster1.xml.dist mcaster1_minimal.xml.dist mcaster1_shoutcast_compat.xml.dist

docdir = $(datadir)/$(PACKAGE)/doc
doc_DATA = mcaster1.xml.dist mcaster1_minimal.xml.dist mcaster1_shoutcast_compat.xml.dist

install-data-hook:
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)
	test -f $(DESTDIR)$(sysconfdir)/mcaster1.xml || \
	$(INSTALL_DATA) mcaster1.xml.dist $(DESTDIR)$(sysconfdir)/mcaster1.xml

uninstall-hook:
	-rm -f $(DESTDIR)$(sysconfdir)/mcaster1.xml

edit = sed -e 's,@pkgdatadir\@,$(pkgdatadir),g' \
	-e 's,@localstatedir\@,$(localstatedir),g' \
	-e 's,@PACKAGE\@,$(PACKAGE),g'

mcaster1.xml.dist: $(srcdir)/mcaster1.xml.in
	$(edit) $(srcdir)/mcaster1.xml.in > mcaster1.xml.dist

mcaster1_minimal.xml.dist: $(srcdir)/mcaster1_minimal.xml.in
	$(edit) $(srcdir)/mcaster1_minimal.xml.in > mcaster1_minimal.xml.dist

mcaster1_shoutcast_compat.xml.dist: $(srcdir)/mcaster1_shoutcast_compat.xml.in
	$(edit) $(srcdir)/mcaster1_shoutcast_compat.xml.in > mcaster1_shoutcast_compat.xml.dist

debug:
	$(MAKE) all CFLAGS="@DEBUG@"

profile:
	$(MAKE) all CFLAGS="@PROFILE@"


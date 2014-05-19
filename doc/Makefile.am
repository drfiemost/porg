.PHONY: \
	logme

EXTRA_DIST = \
	grop.desktop \
	faq.txt \
	porg.png \
	download.png \
	index.html \
	grop.png

man_MANS = \
	porg.8 \
	porgball.8 \
	superporg.8 \
	porgrc.5

pkgdata_DATA = \
	porgrc \
	index.html \
	porg.png \
	download.png \
	faq.txt

logme_files = \
	$(DESTDIR)$(man5dir)/porgrc.5 \
	$(DESTDIR)$(man8dir)/porg.8 \
	$(DESTDIR)$(man8dir)/superporg.8 \
	$(DESTDIR)$(man8dir)/porgball.8 \
	$(DESTDIR)$(sysconfdir)/porgrc \
	$(DESTDIR)$(pkgdatadir)/porgrc \
	$(DESTDIR)$(pkgdatadir)/index.html \
	$(DESTDIR)$(pkgdatadir)/porg.png \
	$(DESTDIR)$(pkgdatadir)/download.png \
	$(DESTDIR)$(pkgdatadir)/faq.txt

if ENABLE_GROP

grop_apps_DATA = \
	grop.desktop

grop_appsdir = \
	$(datadir)/applications
	
grop_icons_DATA = \
	grop.png

grop_iconsdir = \
	$(datadir)/pixmaps

logme_files += \
	$(DESTDIR)$(grop_appsdir)/grop.desktop \
	$(DESTDIR)$(grop_iconsdir)/grop.png

endif

install-data-local: porgrc
	rc_dir="$(DESTDIR)$(sysconfdir)"; \
	( test -d $$rc_dir || $(mkinstalldirs) $$rc_dir ); \
	( test -f $$rc_dir/porgrc || $(INSTALL_DATA) porgrc $$rc_dir/porgrc )

logme:
	ls $(logme_files) 2>/dev/null | porg -lp+ porg-$(PACKAGE_VERSION)

web: index.html
	scp $< davidrr,porg@web.sourceforge.net:htdocs/


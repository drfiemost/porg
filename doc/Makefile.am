.PHONY: \
	logme

EXTRA_DIST = \
	grop.desktop \
	faq.txt \
	grop.png

man_MANS = \
	porg.8 \
	porgrc.5

pkgdata_DATA = \
	porgrc \
	faq.txt

logme_files = \
	$(DESTDIR)$(man5dir)/porgrc.5 \
	$(DESTDIR)$(man8dir)/porg.8 \
	$(DESTDIR)$(sysconfdir)/porgrc \
	$(DESTDIR)$(pkgdatadir)/porgrc \
	$(DESTDIR)$(pkgdatadir)/faq.txt

if ENABLE_SCRIPTS

man_MANS += \
	porgball.8 \
	superporg.8
	
logme_files += \
	$(DESTDIR)$(man8dir)/superporg.8 \
	$(DESTDIR)$(man8dir)/porgball.8

endif

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
	echo $(logme_files) | porg -lp+ porg-$(PACKAGE_VERSION)


.PHONY: \
	logme

ACLOCAL_AMFLAGS = -I build

SUBDIRS = \
	lib \
	porg \
	grop \
	doc \
	scripts

pkgdata_DATA = \
	README

EXTRA_DIST = \
	ChangeLog.paco

logdir = \
	$(DESTDIR)@LOGDIR@

## Create the porg log directory on install, if it does not exist
install-exec-local:
	test -d $(logdir) || $(mkinstalldirs) $(logdir)

logme_files = \
	$(DESTDIR)$(pkgdatadir)/README

## A rule to log the own porg's installation
logme:
	echo $(logme_files) | porg -lp porg-$(PACKAGE_VERSION); \
	for dir in $(SUBDIRS); do \
	  ( cd $$dir && $(MAKE) $@ ) 2>/dev/null; \
	done && \
	echo && porg -vvfsty porg-$(PACKAGE_VERSION)


.PHONY: \
	logme

EXTRA_DIST = \
	porg_bash_completion \
	rpm2porg \
	superporg \
	porgball

if ENABLE_SCRIPTS

bin_SCRIPTS = \
	rpm2porg \
	superporg \
	porgball

logme_files = \
	$(DESTDIR)$(sysconfdir)/bash_completion.d/porg_bash_completion \
	$(DESTDIR)$(bindir)/superporg \
	$(DESTDIR)$(bindir)/porgball \
	$(DESTDIR)$(bindir)/rpm2porg

logme:
	echo $(logme_files) | porg -lp+ porg-$(PACKAGE_VERSION)

else

logme:

endif


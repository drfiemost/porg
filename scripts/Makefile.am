.PHONY: \
	logme

bin_SCRIPTS = \
	paco2porg \
	porgball

completiondir = \
	$(sysconfdir)/bash_completion.d

dist_completion_DATA = \
	porg_bash_completion

logme_files = \
	$(DESTDIR)$(sysconfdir)/bash_completion.d/porg_bash_completion \
	$(DESTDIR)$(bindir)/paco2porg \
	$(DESTDIR)$(bindir)/porgball

logme:
	ls $(logme_files) | porg -lp+ porg-$(PACKAGE_VERSION)


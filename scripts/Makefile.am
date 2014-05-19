.PHONY: \
	logme

EXTRA_DIST = \
	porg_bash_completion \
	superporg \
	paco2porg \
	porgball

bin_SCRIPTS = \
	superporg \
	paco2porg \
	porgball

install-exec-local:
	if test -f /etc/bash_completion ; then \
		$(mkinstalldirs) /etc/bash_completion.d && \
		$(INSTALL_DATA) porg_bash_completion /etc/bash_completion.d; \
	fi

logme_files = \
	$(DESTDIR)$(sysconfdir)/bash_completion.d/porg_bash_completion \
	$(DESTDIR)$(bindir)/superporg \
	$(DESTDIR)$(bindir)/paco2porg \
	$(DESTDIR)$(bindir)/porgball

logme:
	ls $(logme_files) 2>/dev/null | porg -lp+ porg-$(PACKAGE_VERSION)


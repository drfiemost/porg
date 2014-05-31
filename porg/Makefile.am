.PHONY: \
	logme

bin_PROGRAMS = \
	porg

porg_CXXFLAGS = \
	-I$(top_srcdir)/lib \
	-DLIBDIR='"$(DESTDIR)$(libdir)"' \
	$(MY_CXXFLAGS)

porg_SOURCES = \
	main.cc \
	pkg.cc \
	info.cc \
	out.cc \
	pkgset.cc \
	log.cc \
	opt.cc \
	util.cc

noinst_HEADERS = \
	util.h \
	out.h \
	info.h \
	pkgset.h \
	pkg.h \
	log.h \
	opt.h

porg_LDADD = \
	$(top_builddir)/lib/porg/libporg.a

logme_files = \
	$(DESTDIR)$(bindir)/porg

logme:
	ls $(logme_files) | porg -lp+ porg-$(PACKAGE_VERSION)


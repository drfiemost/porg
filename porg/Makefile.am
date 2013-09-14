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
	dir.cc \
	opt.cc \
	global.cc

if !HAVE_GETOPT_LONG
porg_SOURCES += \
	getopt.cc
endif

if HAVE_REGEX_H
porg_SOURCES += \
	regexp.cc
endif

noinst_HEADERS = \
	global.h \
	out.h \
	info.h \
	getopt.h \
	pkgset.h \
	regexp.h \
	pkg.h \
	log.h \
	dir.h \
	opt.h

porg_LDADD = \
	$(top_builddir)/lib/porg/libporg.a

logme_files = \
	$(DESTDIR)$(bindir)/porg

logme:
	echo $(logme_files) | porg -lp+ porg-$(PACKAGE_VERSION)


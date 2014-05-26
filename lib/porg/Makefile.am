noinst_LIBRARIES = \
	libporg.a

libporg_a_SOURCES = \
	basepkg.cc \
	common.cc \
	file.cc \
	regexp.cc \
	porgrc.cc

noinst_HEADERS = \
	common.h \
	basepkg.h \
	porgrc.h \
	regexp.h \
	file.h

libporg_a_CXXFLAGS = \
	$(MY_CXXFLAGS) \
	-DPORGRC='"$(DESTDIR)$(sysconfdir)/porgrc"'


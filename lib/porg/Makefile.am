noinst_LIBRARIES = \
	libporg.a

libporg_a_SOURCES = \
	basepkg.cc \
	common.cc \
	file.cc \
	porgrc.cc

noinst_HEADERS = \
	common.h \
	basepkg.h \
	porgrc.h \
	file.h

libporg_a_CXXFLAGS = \
	$(MY_CXXFLAGS) \
	-DPORGRC='"$(DESTDIR)$(sysconfdir)/porgrc"'


noinst_LIBRARIES = \
	libporg.a

libporg_a_SOURCES = \
	basepkg.cc \
	common.cc \
	file.cc \
	rexp.cc \
	porgrc.cc

noinst_HEADERS = \
	common.h \
	basepkg.h \
	porgrc.h \
	rexp.h \
	file.h

libporg_a_CXXFLAGS = \
	$(MY_CXXFLAGS) \
	-DPORGRC='"$(DESTDIR)$(sysconfdir)/porgrc"'


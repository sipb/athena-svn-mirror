NULL=

SUBDIRS = eel test po

EXTRA_DIST=					\
	COPYING.LIB				\
	HACKING					\
	MAINTAINERS				\
	eel.spec.in				\
	eel-2.0-uninstalled.pc.in		\
	$(NULL)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = eel-2.0.pc

dist-hook: eel.spec
	cp eel.spec $(distdir)

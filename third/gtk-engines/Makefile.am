SUBDIRS = pixbuf redmond95 metal

EXTRA_DIST = gtk-engines.spec.in makefile.cygwin makefile.msc README.win32 gtk-engines-2.pc.in

dist-hook: gtk-engines.spec
	cp gtk-engines.spec $(distdir)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gtk-engines-2.pc

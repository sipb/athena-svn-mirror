SUBDIRS = bonobo glade samples tests tools po doc

libincludedir = $(includedir)/libbonoboui-2.0
libinclude_HEADERS =    \
	bonobo.h	\
	libbonoboui.h

EXTRA_DIST = \
	changes.txt          \
	intltool-extract.in  \
	intltool-merge.in    \
	intltool-update.in   \
	gtk-doc.make

DISTCLEANFILES = \
	intltool-extract \
	intltool-merge   \
	intltool-update

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc


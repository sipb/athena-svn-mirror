SUBDIRS = \
	idl bonobo-activation bonobo \
	activation-server monikers \
	utils samples tests doc po

libincludedir = $(includedir)/libbonobo-2.0
libinclude_HEADERS = \
	libbonobo.h

EXTRA_DIST = \
	bonobo-activation-config.xml \
	intltool-extract.in  \
	intltool-merge.in    \
	intltool-update.in   \
	changes.txt          \
	gtk-doc.make

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

install-data-local:
	-mkdir -p $(DESTDIR)$(sysconfdir)/bonobo-activation
	if test -f $(DESTDIR)$(sysconfdir)/bonobo-activation/bonobo-activation-config.xml; then	\
		echo "bonobo-activation-config.xml configuration file not changed";		\
	else											\
		$(INSTALL_DATA) $(top_srcdir)/bonobo-activation-config.xml			\
			$(DESTDIR)$(sysconfdir)/bonobo-activation;				\
	fi

DISTCLEANFILES=intltool-extract intltool-merge intltool-update

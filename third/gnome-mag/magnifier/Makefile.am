NULL =

Cursorsdir = $(datadir)/gnome-mag

lib_LTLIBRARIES = libgnome-mag.la

bin_PROGRAMS = magnifier

libgnome_mag_la_SOURCES = \
	GNOME_Magnifier-common.c \
	GNOME_Magnifier-stubs.c

libgnome_mag_la_DEPENDENCIES = \
	GNOME_Magnifier.h

libgnomemagincludedir=$(includedir)/gnome-mag-1.0/magnifier
libgnomemaginclude_HEADERS=GNOME_Magnifier.h

magnifier_SOURCES = magnifier-main.c \
	magnifier.c \
	magnifier-private.h \
	zoom-region.c \
	zoom-region.h \
	zoom-region-private.h \
	magnifier.h \
	magnifier-private.h \
	damage-client.c \
	damage-client.h	\
	GNOME_Magnifier.h \
	GNOME_Magnifier-skels.c \
	GNOME_Magnifier-common.c

INCLUDES = \
	$(DEPS_CFLAGS)		\
	-I$(top_srcdir)		\
	-I$(top_builddir)	\
	-I$(top_srcdir)/idl	\
	$(GNOME_MAG_DEBUG_CFLAGS)	\
	-DCURSORSDIR=\""$(Cursorsdir)"\"

CFLAGS += $(DEPS_CFLAGS)

IDL_OUT = GNOME_Magnifier.h GNOME_Magnifier-skels.c GNOME_Magnifier-stubs.c GNOME_Magnifier-common.c

IDL = $(top_srcdir)/idl/GNOME_Magnifier.idl

BUILT_SOURCES = $(IDL_OUT)
CLEANFILES+=$(IDL_OUT)

magnifier.h : GNOME_Magnifier.h

IDLFLAGS = -I$(BONOBO_ACTIVATION_IDL_DIR) 	\
	-I$(LIBBONOBO_IDL_DIR)			\
	-I$(top_srcdir)/idl		

orbittypelibdir = $(libdir)/orbit

LDADD = $(DEPS_LIBS) libgnome-mag.la $(X_LIBS) $(POPT_LIBS)

serverinfodir= $(libdir)/bonobo/servers
$(server_in_files): $(server_in_files).in
	sed -e "s|\@BINDIR\@|$(bindir)|" $< > $@
server_in_files = GNOME_Magnifier.server.in
serverinfo_DATA = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

libgnome_mag_la_LDFLAGS = \
	$(LT_VERSION_INFO)

EXTRA_DIST = $(server_in_files).in 

$(IDL_OUT) : $(IDL) $(ORBIT_IDL)
	$(ORBIT_IDL) $(IDLFLAGS) $(IDL)

DONT_DIST_SOURCE = $(IDL_OUT)

CLEANFILES= $(server_in_files) $(serverinfo_DATA)

dist-hook:
	for file in $(DONT_DIST_SOURCE) ; do \
	    rm -f $(distdir)/$$file ; \
	done

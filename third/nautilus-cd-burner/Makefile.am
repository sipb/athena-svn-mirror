module_flags = -avoid-version -module

SUBDIRS = po

lib_LTLIBRARIES = libnautilus-burn.la
libexec_PROGRAMS = mapping-daemon
bin_PROGRAMS = nautilus-cd-burner
noinst_PROGRAMS = list_cddrives bcs_test

if MKDTEMP_MISSING
MKDTEMP_FILES=mkdtemp.c mkdtemp.h
else
MKDTEMP_FILES=
endif

NULL =

INCLUDES =					\
	$(MAPPING_CFLAGS)                       \
	$(GUI_CFLAGS)				\
	$(EXTENSION_CFLAGS)			\
	$(WERROR)				\
	-D_FILE_OFFSET_BITS=64			\
	-D_GNU_SOURCE				\
	-D_LARGEFILE64_SOURCE			\
	-D_REENTRANT				\
	-D_POSIX_PTHREAD_SEMANTICS		\
	-DPREFIX=\"$(prefix)\"			\
	-DBINDIR=\""$(bindir)"\"	\
	-DLIBEXECDIR=\""$(libexecdir)"\"	\
	-DSHAREDIR=\""$(datadir)"\"		\
	-DDATADIR=\""$(datadir)/nautilus-cd-burner"\"

EXTRA_DIST =			\
	mapping-modules.conf 	\
	cdburn.glade		\
	cdspin1.png cdspin2.png cdspin3.png \
	cdspin4.png cdspin5.png cdspin6.png \
	cdspin7.png cdspin8.png \
	mkdtemp.c mkdtemp.h \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	libnautilus-burn.pc.in \
	cdrecorder-marshal.list \
	$(schema_in_files)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnautilus-burn.pc

CLEANFILES = $(desktop_in_files:.desktop.in=.desktop) \
	     $(schema_DATA) $(CDRECMARSHALFILES)

DISTCLEANFILES = intltool-extract intltool-merge intltool-update

modulesconfdir=$(sysconfdir)/gnome-vfs-2.0/modules
modulesconf_DATA = mapping-modules.conf
modulesdir = $(libdir)/gnome-vfs-2.0/modules

schema_in_files = ncb.schemas.in
schemadir = $(sysconfdir)/gconf/schemas
schema_DATA = $(schema_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

modules_LTLIBRARIES = libmapping.la

### GUI burner app

cdburndir = $(datadir)/nautilus-cd-burner
cdburn_DATA = cdburn.glade \
	cdspin1.png cdspin2.png cdspin3.png \
	cdspin4.png cdspin5.png cdspin6.png \
	cdspin7.png cdspin8.png 

nautilus_cd_burner_SOURCES = \
	nautilus-cd-burner.c \
	nautilus-cd-burner.h \
	make-iso.c \
	make-iso.h \
	$(NULL)

nautilus_cd_burner_LDADD = \
	libnautilus-burn.la \
	$(GUI_LIBS) \
	$(NULL)

### Test programs
list_cddrives_SOURCES = list_cddrives.c
list_cddrives_LDADD = libnautilus-burn.la
bcs_test_SOURCES = bcs-test.c
bcs_test_LDADD = libnautilus-burn.la

### Mapping daemon

mapping_daemon_SOURCES = \
	mapping-daemon.c \
	mapping-daemon.h \
	mapping-protocol.h \
	mapping-protocol.c \
	$(NULL)

mapping_daemon_LDADD = $(DAEMON_LIBS)

### `mapping' method

libmapping_la_SOURCES = 				\
	mapping-method.c 				\
	mapping-method.h				\
	mapping-protocol.h \
	mapping-protocol.c \
	$(NULL)

# we set this so that Automake will use different file names when
# compiling, so that the copy of mapping-method.o compiled for
# libmapping.la doesn't conflict with the copy compiled for
# mapping-daemon (the library one should be PIC code).
libmapping_la_CFLAGS =

libmapping_la_LDFLAGS = $(module_flags)
libmapping_la_LIBADD = $(MAPPING_LIBS)

### Nautilus Extension
nautilus_extensiondir=$(libdir)/nautilus/extensions-1.0
nautilus_extension_LTLIBRARIES=libnautilus-burn-extension.la
libnautilus_burn_extension_la_SOURCES = burn-extension.c
libnautilus_burn_extension_la_LIBADD = $(EXTENSION_LIBS)
libnautilus_burn_extension_la_LDFLAGS = $(module_flags)


### Our burning library
GLIB_GENMARSHAL=`pkg-config --variable=glib_genmarshal glib-2.0`
CDRECMARSHALFILES = cd-recorder-marshal.h cd-recorder-marshal.c
cd-recorder-marshal.h: cdrecorder-marshal.list
	( $(GLIB_GENMARSHAL) --prefix=cdrecorder_marshal $(srcdir)/cdrecorder-marshal.list --header > cd-recorder-marshal.h )
cd-recorder-marshal.c: cd-recorder-marshal.h
	( $(GLIB_GENMARSHAL) --prefix=cdrecorder_marshal $(srcdir)/cdrecorder-marshal.list --body --header > cd-recorder-marshal.c )

libnautilus_burn_la_SOURCES = 	\
	bacon-cd-selection.c	\
	bacon-cd-selection.h	\
	$(CDRECMARSHALFILES)	\
	cd-recorder.c		\
	cd-recorder.h		\
	cd-drive.c		\
	cd-drive.h		\
	dvd_plus_rw_utils.cpp	\
	transport.hxx		\
	$(NULL)

if LIBSTDCXX_HACK
AM_CXXFLAGS = -fno-exceptions -fno-rtti
CXX_LINK_TAG = --tag=CC
endif 
CXXLINK = $(LIBTOOL) --mode=link $(CXX_LINK_TAG) $(CXXLD) $(AM_CXXFLAGS) \
  $(CXXFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@


libnautilus_burn_la_LIBADD = $(BURN_LIBS)
libnautilus_burnincludedir = $(includedir)/libnautilus-burn/
libnautilus_burninclude_HEADERS = \
	cd-drive.h		\
	cd-recorder.h		\
	bacon-cd-selection.h	\
	$(NULL)

install-data-local:
	-GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA)


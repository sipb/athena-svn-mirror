INCLUDES = \
        -I$(top_srcdir) \
	$(MEDIA_CFLAGS)	\
	-I$(top_srcdir)/cddb-slave2	\
	$(GAILUTIL_CFLAGS)

noinst_LTLIBRARIES = libgnomecdaccess.la

libgnomecdaccess_la_SOURCES = \
	cddisplayaccessible.c	\
	cddisplayaccessible.h	\
	cddisplayaccessiblefactory.c	\
	cddisplayaccessiblefactory.h	\
	pangoaccessible.c	\
	pangoaccessible.h	\
	pangoaccessiblefactory.c	\
	pangoaccessiblefactory.h	\
	factory.c	\
	factory.h

libgnomecdaccess_la_LIBADD = $(INTLLIBS)

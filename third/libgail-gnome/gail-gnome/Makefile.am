
module_LTLIBRARIES = libgail-gnome.la

gailgnome_c_sources =				\
	bonobo-control-accessible.c		\
	bonobo-control-frame-accessible.c 	\
	bonobo_plug_atk_object.c		\
	bonobo_plug_atk_object_factory.c 	\
	bonobo_socket_atk_object.c		\
	bonobo_socket_atk_object_factory.c	\
	bonobo-accessibility-init.c		

gailgnome_private_h_sources =			\
	bonobo-control-accessible.h		\
	bonobo-control-frame-accessible.h	\
	bonobo_plug_atk_object.h		\
	bonobo_plug_atk_object_factory.h 	\
	bonobo_socket_atk_object.h		\
	bonobo_socket_atk_object_factory.h	\
        gail-gnome-debug.h

gailgnome_public_h_sources =

libgail_gnome_la_SOURCES = 			\
	$(gailgnome_c_sources)		\
	$(gailgnome_private_h_sources)

INCLUDES =				\
	-I$(top_srcdir)		 	\
	-DVERSION=\"$(VERSION)\"	\
	$(GAIL_GNOME_DEBUG_CFLAGS)	\
	$(DEPS_CFLAGS)

libgail_gnome_la_LIBADD =     \
        $(DEPS_LIBS)

moduledir = $(libdir)/gtk-2.0/modules

libgail_gnome_la_LDFLAGS =    \
        -no-undefined   \
	-rpath $(moduledir) -module -avoid-version \
        @LDFLAGS@	


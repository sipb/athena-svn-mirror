SUBDIRS = 
NULL =

lib_LIBRARIES = libammonite.a

libammoniteincludedir = $(includedir)/libtrilobite

libammoniteinclude_HEADERS = \
	libammonite.h	\
	eazelproxy.h	\
	$(NULL)

libammonite_a_idl_sources = \
	eazelproxy-common.c	\
	eazelproxy-stubs.c	\
	eazelproxy-skels.c	\
	eazelproxy.h		\
	$(NULL)

libammonite_a_SOURCES = \
	authn-callback-wrapper.c \
	authn-listener-wrapper.c \
	user-prompter-wrapper.c \
	ammonite-general.c \
	$(libammonite_a_idl_sources) \
	$(NULL)

BUILT_SOURCES = \
	$(libammonite_a_idl_sources) \
	$(NULL)

INCLUDES = \
	$(GLIB_CFLAGS)	\
	$(OAF_CFLAGS)	\
	$(WERROR)	\
	$(NULL)

#
# for CORBA 
#

$(libammonite_a_idl_sources): libammonite_a_idl_stamp

libammonite_a_idl_stamp: $(top_srcdir)/idl/eazelproxy.idl
	$(ORBIT_IDL) $(IDL_CFLAGS) $(top_srcdir)/idl/eazelproxy.idl
	touch libammonite_a_idl_stamp

CLEANFILES+=$(CORBA_GENERATED) libammonite_a_idl_stamp

# Files that depend on the corba-generated files

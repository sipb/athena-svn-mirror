SUBDIRS = util 
NULL = 


bin_PROGRAMS = eazel-proxy

eazel_proxy_SOURCES = config.h 	\
	compat.c compat.h 	\
	gnuregex.c gnuregex.h 	\
	log.c log.h 		\
	proxy.c proxy.h 	\
	request.c 		\
	session.c session.h	\
	sock.c sock.h 		\
	sock-ssl.c sock-ssl.h	\
	dnscache.c dnscache.h	\
	utils.c 		\
	impl-eazelproxy.c	\
	impl-eazelproxy.h	\
	eazelproxy-common.c	\
	eazelproxy-skels.c	\
	eazelproxy-stubs.c	\
	eazelproxy.h		\
	http-connection.h	\
	http-connection.c	\
	digest.c		\
	digest.h		\
	util-gconf.c		\
	util-gconf.h		\
	authenticate.h		\
	authenticate.c		\
	eazelproxy-change-password.c \
	eazelproxy-change-password.h \
	request.h \
	utils.h \
	$(NULL)

eazel_proxy_LDADD = \
	$(top_builddir)/libammonite/libammonite.a \
	$(GLIB_LIBS) 	\
	$(GCONF_LIBS)	\
	$(OAF_LIBS) 	\
	$(ORBIT_LIBS)	\
	$(OPENSSL_LIBS) \
	$(XML_LIBS)	\
	$(NULL)

INCLUDES = \
	$(GLIB_CFLAGS)	\
	$(GCONF_CFLAGS)	\
	$(OAF_CFLAGS)   \
	$(OPENSSL_CFLAGS) \
	$(XML_CFLAGS)	\
	$(WERROR)	\
	-DINSTALL_PATH_BIN=\""$(bindir)"\" 	\
	-I$(top_srcdir)/libammonite		\
	-DDATADIR=\""$(datadir)"\" 		\
	$(NULL)


#
# for CORBA 
#

IDL = $(top_srcdir)/idl/eazelproxy.idl

CORBA_GENERATED = \
	eazelproxy-common.c	\
	eazelproxy-skels.c	\
	eazelproxy-stubs.c	\
	eazelproxy.h		\
	$(NULL)

# make the auto-generated files depend on a timestamp file.
# this is to ensure the individual files wait for each other before
# proceeding which is needed in parallel makes

$(CORBA_GENERATED): eazel_proxy_idl_stamp

eazel_proxy_idl_stamp: $(IDL)
	$(ORBIT_IDL) $(IDL_CFLAGS) $(IDL)
	touch eazel_proxy_idl_stamp

CLEANFILES+=$(CORBA_GENERATED) eazel_proxy_idl_stamp

# Files that depend on the corba-generated files
proxy.c impl-eazelproxy.h impl-eazelproxy.c: $(CORBA_GENERATED)

oafdir = $(datadir)/oaf
oaf_DATA = \
	eazel-proxy.oafinfo \
	$(NULL)

EXTRA_DIST = regexp.h gnuregex.c gnuregex.h $(oaf_DATA)

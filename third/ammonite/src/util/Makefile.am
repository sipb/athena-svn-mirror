SUBDIRS = 
NULL =

bin_PROGRAMS = eazel-proxy-util

# idl_sources are generated files
eazel_proxy_util_idl_sources = \
	eazelproxy-common.c	\
	eazelproxy-stubs.c	\
	eazelproxy.h		\
	$(NULL)

eazel_proxy_util_SOURCES = \
	eazel-proxy-util.c		\
	$(eazel_proxy_util_idl_sources)  \
	$(NULL)

eazel_proxy_util_LDADD = \
	$(top_builddir)/libammonite-gtk/libammonite-gtk.a \
	$(top_builddir)/libammonite/libammonite.a	\
	$(GLIB_LIBS)					\
	$(GCONF_LIBS)					\
	$(ORBIT_LIBS)					\
	$(OAF_LIBS)					\
	$(GNOMEUI_LIBS)					\
	$(NULL)

INCLUDES = \
	-I$(top_srcdir)/libammonite \
	-I$(top_srcdir)/libammonite-gtk \
	$(GLIB_CFLAGS)	\
	$(GNOMEUI_CFLAGS) \
	$(OAF_CFLAGS)   \
	$(WERROR)	\
	$(NULL)


#
# for CORBA 
#

IDL = $(top_srcdir)/idl/eazelproxy.idl

$(eazel_proxy_util_idl_sources): eazel_proxy_util_idl_stamp

# make the auto-generated files depend on a timestamp file.
# this is to ensure the individual files wait for each other before
# proceeding which is needed in parallel makes

eazel_proxy_util_idl_stamp: $(IDL)
	$(ORBIT_IDL) $(IDL_CFLAGS) $(IDL)
	touch eazel_proxy_util_idl_stamp

BUILT_SOURCES = \
	$(eazel_proxy_util_idl_sources) \
	$(NULL)

CLEANFILES+=$(BUILT_SOURCES) eazel_proxy_util_idl_stamp


# Files that depend on the corba-generated files
# $(eazel_proxy_util_SOURCES): $(eazel_proxy_util_idl_sources)

#
# For gnome-vfs
#

ammonite-methods.conf: ammonite-methods.conf.sub
	sed -e "s|%bindir%|$(bindir)|" <$< >$@

vfs_confdir = $(sysconfdir)/vfs/modules
vfs_conf_DATA = ammonite-methods.conf


EXTRA_DIST=\
	$(vfs_conf_DATA) \
	ammonite-methods.conf.sub \
	$(NULL)

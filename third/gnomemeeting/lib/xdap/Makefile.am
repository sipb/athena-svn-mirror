noinst_LTLIBRARIES = libxdap.la

libxdap_la_SOURCES =					\
	xdap.c						\
	xdap.h					

INCLUDES =						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-DGNOMEMEETING_IMAGES=\""$(datadir)/pixmaps"\"	\
	-DG_LOG_DOMAIN=\"GnomeMeeting\"		 	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-I$(top_srcdir) 				\
	$(GNOMEMEETING_LIB_CFLAGS)				

libxdap_la_LDFLAGS = -export-dynamic
libxdap_la_LIDADD = 

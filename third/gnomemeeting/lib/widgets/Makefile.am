noinst_LTLIBRARIES = libgmwidgets.la

libgmwidgets_la_SOURCES =				\
	gmentrydialog.c					\
	gmentrydialog.h					\
	gtklevelmeter.c					\
	gtklevelmeter.h					\
	history-combo.c					\
	history-combo.h					

INCLUDES =						\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-DGNOMEMEETING_IMAGES=\""$(datadir)/pixmaps"\"	\
	-DG_LOG_DOMAIN=\"GnomeMeeting\"		 	\
	-DSHARE_DIR=\"$(pkgdatadir)\"			\
	-I$(top_srcdir) 				\
	$(GNOMEMEETING_LIB_CFLAGS)

libgmwidgets_la_LDFLAGS = -export-dynamic
libgmwidgets_la_LIDADD = 

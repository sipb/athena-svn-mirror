NULL=

libbrl_LTLIBRARIES = libbrl.la

INCLUDES = -I$(top_builddir)			\
	   -I$(top_srcdir)/srutil		\
	   -I$(top_srcdir)/srconf/libsrconf 	\
	   -DBRAILLE_TRANS_TABLES_DIR=\"$(sysconfdir)/gnopernicus-1.0/translation_tables/\" \
	   -DTTYNAME0=\"$(TTYNAME_0)\" -DTTYNAME1=\"$(TTYNAME_1)\" \
	   $(BRLTTY_ACTIVE)			\
	   $(LIBBRL_CFLAGS) 			\
	   -DG_LOG_DOMAIN=\"gnopernicus\"

LDFLAGS = $(LIBBRL_LIBS) @LT_VERSION_INFO@

LDADD = ../../srconf/libsrconf/libsrconf.la	\
	../../srutil/srutil.la 		\
	$(LIBBRL_LIBS)

libbrlincludedir =$(includedir)/gnopernicus-1.0/libbrl
libbrldir=$(libdir)/gnopernicus-1.0

if BUILD_BRLTTY_SUPPORT
libbrl_la_LIBADD = -lbrlapi
endif

libbrlinclude_HEADERS = brlxmlapi.h  

libbrl_la_SOURCES = baumbrl.h	 \
		    alvabrl.h	 \
		    braille.h    \
		    brlxmlapi.h  \
		    brlxml.h     \
		    sercomm.h    \
		    handybrl.h	 \
		    sercomm.c    \
		    baumbrl.c    \
		    alvabrl.c	 \
		    braille.c    \
		    brlxml.c     \
		    handybrl.c	 \
		    $(EXTRA_libbrl_la_SOURCES)

if BUILD_BRLTTY_SUPPORT
EXTRA_libbrl_la_SOURCES = ttybrl.c 	 \
			 ttybrl.h
endif

EXTRA_DIST = ttybrl.c 	 \
	    ttybrl.h

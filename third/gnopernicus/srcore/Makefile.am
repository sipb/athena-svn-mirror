
INCLUDES = -I$(srcdir)                     	\
	   -I$(top_srcdir)/braille/libbrl     	\
	   -I$(top_srcdir)/speech/libsrs      	\
	   -I$(top_srcdir)/srlow/libsrlow     	\
	   -I$(top_srcdir)/srconf/libsrconf   	\
	   -I$(top_srcdir)/kbd_mouse/libke    	\
	   -I$(top_srcdir)/magnifier/libmag   	\
	   -I$(top_srcdir)/srutil             	\
	   -DGNOPERNICUSLOCALEDIR=\""$(gnopernicuslocaledir)"\" \
	   -DBRAILLE_TRANS_TABLES_DIR=\"$(sysconfdir)/gnopernicus-1.0/translation_tables/\" \
	   -DPRESENTATION_DIR=\"$(datadir)/gnopernicus/presentation/\" \
	   -DBRLMON_DIR=\"$(libexecdir)/\" \
	   $(SRCORE_CFLAGS)  			\
	   -DG_LOG_DOMAIN=\"gnopernicus\"	\
	   -DSRS_NO_MARKERS_SUPPORTED


bin_PROGRAMS = srcore

srcore_SOURCES = src-speech-in.h \
		 src-speech-in.c \
		 srmain.c \
		 srbrl.c  \
		 srspc.c  \
		 srmag.c  \
		 brlinp.c \
		 srpres.c \
		 srmain.h \
		 srbrl.h  \
		 srspc.h  \
		 srmag.h  \
		 brlinp.h \
		 srpres.h \
		 srctrl.h \
		 srctrl.c

LDADD = ../speech/libsrs/libsrs.la       \
	../srlow/libsrlow/libsrlow.la    \
	../srutil/libsrutil.la           \
	../braille/libbrl/libbrl.la      \
	../srconf/libsrconf/libsrconf.la \
	../kbd_mouse/libke/libke.la      \
	../magnifier/libmag/libmag.la    \
	$(SRCORE_LIBS)			 \
	$(X_LIBS)

xml_in_files = default.xml.in \
	       verbose.xml

xml_files = $(xml_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@
     
xmldir=$(datadir)/gnopernicus/presentation
xml_DATA=$(xml_files) 

EXTRA_DIST = brl_out_test.xml \
	     srsml.xml        \
	     de.a2b           \
	     es.a2b	      \
	     sv.a2b	      \
	     en_US.a2b	      \
	     $(xml_in_files)

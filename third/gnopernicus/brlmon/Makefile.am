INCLUDES = -I$(srcdir)						\
	   -I$(top_srcdir)/srutil				\
	   -DBRLMON_GLADEDIR=\""$(datadir)/gnopernicus/glade/"\"\
	   -DDATADIR=\""$(datadir)"\"				\
	   $(INET6_ACTIVE) -DGNOPERNICUSLOCALEDIR=\""$(gnopernicuslocaledir)"\" \
	   $(BRLMON_CFLAGS)					\
	   -DG_LOG_DOMAIN=\"gnopernicus\"


#schemasdir	 = $(sysconfdir)/gconf/schemas
#schemas_in_files = brlmonitor.schemas.in
#schemas_DATA 	 = $(schemas_in_files:.schemas.in=.schemas)


libexec_PROGRAMS = brlmonitor 
noinst_PROGRAMS = tester

brlmonitor_SOURCES = 			\
		     brlmon.c		\
	    	     brlmonxml.c	\
		     brlmonui.c		\
		     brlmon.h		\
	    	     brlmonxml.h	\
		     brlmonui.h		\
	    	     brlmonxmlapi.h  

tester_SOURCES = tester.c \
		 brlmon.h 

LDADD = ../srutil/libsrutil.la \
	$(BRLMON_LIBS)\
	$(X_LIBS)

#brlmon_gladedir = $(datadir)/gnopernicus/glade/brlmon_files
#brlmon_glade_DATA = monitor.glade2

#brlmon_fontdir = $(sysconfdir)/gnopernicus-1.0/braille_fonts
#brlmon_font_DATA = braille14.bdf

EXTRA_DIST = brl.xml		

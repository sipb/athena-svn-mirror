SUBDIRS = gnopi_files 

INCLUDES = -I$(srcdir)\
	   -I$(top_srcdir)/srutil \
	   -I$(top_srcdir)/srlow/libsrlow \
	   -I$(top_srcdir)/srconf/libsrconf \
	   -DGNOPI_GLADEDIR=\""$(datadir)/gnopernicus/glade/"\"			\
	   -DDATADIR=\""$(datadir)"\"				\
	   -DGNOPERNICUSLOCALEDIR=\""$(gnopernicuslocaledir)"\" \
	   -DPRESENTATION_DIR=\"$(datadir)/gnopernicus/presentation/\" \
	   $(GNOPI_CFLAGS)					\
	   -DG_LOG_DOMAIN=\"gnopernicus\"

gnopernicusdir = $(prefix)/bin

bin_PROGRAMS = gnopernicus

gnopernicus_SOURCES = gnopi.c   \
		gnopiui.c 	\
		gnopiui.h 	\
		brlconf.c       \
		brlconf.h       \
		brlui.c         \
		brlui.h         \
		kbconf.c	\
		kbconf.h	\
		kbui.c	  	\
		kbui.h	  	\
		spconf.c	\
		spconf.h	\
		spui.c	  	\
		spui.h	  	\
		spdictui.c	\
		spdictui.h	\
		spvoiceui.c	\
		spvoiceui.h	\
		magconf.c	\
		magconf.h 	\
		magui.c	  	\
		magui.h 	\
		coreconf.c	\
		coreconf.h	\
		defui.c		\
		defui.h		\
		genui.c		\
		genui.h		\
		cmdmapui.c	\
		cmdmapui.h	\
		cmdmapconf.c	\
		cmdmapconf.h	\
		gnopiconf.c	\
		gnopiconf.h	\
		findui.c	\
		findui.h	\
		presui.c	\
		presui.h	\
		presconf.c	\
		presconf.h	\
		langui.c	\
		langui.h	\
		scrui.c		\
		scrui.h		\
		bmconf.c	\
		bmconf.h	\
		bmui.c		\
		bmui.h


LDADD = ../srutil/libsrutil.la  \
	../srconf/libsrconf/libsrconf.la\
	$(GNOPI_LIBS)

translationdir = $(sysconfdir)/gnopernicus-1.0/translation_tables
translation_DATA = de.a2b	\
	           es.a2b	\
	           sv.a2b	\
	           en_US.a2b	    

pixmapsdir   = $(datadir)/pixmaps
pixmaps_DATA = gnopernicus.png

desktopdir 	 = $(datadir)/applications
desktop_in_files = gnopernicus.desktop.in
desktop_DATA 	 = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = ./gnopi_files	\
	     $(translation_DATA)\
	     $(desktop_DATA)	\
	     $(pixmaps_DATA)

dist-hook:
	    rm -rf `find $(distdir)/gnopi_files -name CVS`
	    rm -rf `find $(distdir)/gnopi_files -name .cvsignore`
	    rm -rf `find $(distdir)/gnopi_files -name Makefile`




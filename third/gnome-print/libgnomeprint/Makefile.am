
lib_LTLIBRARIES = libgnomeprint.la

libgnomeprint_la_LDFLAGS = -version-info $(GNOMEPRINT_CURRENT):$(GNOMEPRINT_REVISION):$(GNOMEPRINT_AGE)

CPPFLAGS = \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" 	\
	-DBINDIR=\""$(bindir)"\"	\
	-DDATADIR=\""$(datadir)"\" 

INCLUDES = \
	-I$(top_srcdir)		\
	$(GTK_CFLAGS)			\
	-I$(top_srcdir)/intl 	\
	-I$(top_builddir)/intl	\
	$(GNOME_INCLUDEDIR)		\
	@PRINT_INCLUDEDIR@		\
	-DPROFILEDIR=\""$(datadir)/gnome-print/$(VERSION)/profiles/"\"

temp_sources =					\
	gnome-canvas-hacktext.c		\
	gnome-canvas-hacktext.h		\
	gnome-canvas-bpath.c		\
	gnome-canvas-bpath.h		\
	gnome-canvas-bpath-private.h	\
	gnome-canvas-clipgroup.c		\
	gnome-canvas-clipgroup.h		\
	gt1-parset1.c				\
	gt1-parset1.h				\
	gt1-dict.c				\
	gt1-dict.h				\
	gt1-misc.h				\
	gt1-namecontext.c			\
	gt1-namecontext.h			\
	gt1-region.c				\
	gt1-region.h				\
	gt1-value.h				\
	art_rgb_affine_private.h		\
	art_rgba_rgba_affine.c			\
	art_rgba_rgba_affine.h			\
	art_rgba_svp.c				\
	art_rgba_svp.h				\
	text-utils.c               		\
	text-utils.h   	            		\
	gp-ps-unicode.h				\
	gp-ps-unicode.c				\
	gp-unicode.c				\
	gp-unicode.h				\
	gp-character-block.c			\
	gp-character-block.h

libgnomeprint_la_SOURCES = 		\
	gnome-print-master.c		\
	gnome-print-master-private.h	\
	gnome-print-master-preview.c	\
	gnome-print-dialog.c		\
	gnome-print-copies.c		\
	gnome-printer.c 			\
	gnome-printer-private.h		\
	gnome-print.c 				\
	gnome-print-private.h		\
	gnome-print-meta.c			\
	gnome-print-multipage.c		\
	gnome-print-multipage.h		\
	gnome-print-pixbuf.c		\
	gnome-print-pdf.c			\
	gnome-print-pdf-type1.c		\
	gnome-print-pdf-type1.h		\
	gnome-print-pclr.c			\
	gnome-print-pclr.h			\
	gnome-print-pclv.c			\
	gnome-print-pclv.h			\
	gnome-print-pclc.c			\
	gnome-print-pclc.h			\
	gnome-print-ps.c 			\
	gnome-print-ps.h 			\
	gnome-print-ps2.c 			\
	gnome-print-ps2.h 			\
	gnome-print-preview.c		\
	gnome-print-preview-private.h	\
	gnome-print-preview-icons.h	\
	gnome-print-rbuf.c			\
	gnome-print-rbuf-private.h	\
	gnome-print-frgba.c			\
	gnome-print-rgbp.c		  	\
	gnome-print-rgbp.h		  	\
	gnome-font.c 			  	\
	gnome-font-private.h	  	\
	gnome-font-face.c		  	\
	gnome-rfont.c			  	\
	gnome-font-family.c		  	\
	gnome-font-family.h		  	\
	gnome-font-dialog.c        	\
	gnome-printer-dialog.c     	\
	gnome-printer-profile.c    	\
	gnome-printer-profile.h    	\
	gnome-text.c 			  	\
	gnome-print-i18n.h		  	\
	parseAFM.c 			  	\
	parseAFM.h			  	\
	gp-path.c			       	\
	gp-path.h			       	\
	gp-gc.c				  	\
	gp-gc.h				  	\
	gnome-glyphlist.c		  	\
	gnome-glyphlist-private.h  	\
	gnome-pgl.c			  	\
	gnome-pgl-private.h		  	\
	gnome-print-encode.c       	\
	$(temp_sources)

libgnomeprintincdir = $(includedir)/libgnomeprint
libgnomeprintinc_HEADERS = 		\
	gnome-print-master.h		\
	gnome-print-master-preview.h	\
	gnome-print-dialog.h		\
	gnome-print-copies.h		\
	gnome-print.h 				\
	gnome-printer.h 			\
	gnome-font.h 				\
	gnome-font-face.h			\
	gnome-rfont.h				\
	gnome-printer-dialog.h   	\
	gnome-print-meta.h			\
	gnome-print-multipage.h  	\
	gnome-print-pixbuf.h     	\
	gnome-print-pdf.h			\
	gnome-print-preview.h    	\
	gnome-print-rbuf.h			\
	gnome-print-frgba.h			\
	gnome-font-dialog.h			\
	gnome-print-encode.h     	\
	gnome-text.h				\
	gnome-glyphlist.h			\
	gnome-pgl.h



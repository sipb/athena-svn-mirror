## Process this file with automake to produce Makefile.in

bin_PROGRAMS = xalf xalfoff

xalf_SOURCES = xalf.c getopt.c getopt1.c

xalfoff_SOURCES = xalfoff.c

xalf_LDADD =  -lXmu -lXt -lSM -lICE -lXext -lX11 -lXext -L/usr/X11R6/lib @GTK_LIBS@
xalfoff_LDADD =  -lXmu -lXt -lSM -lICE -lXext -lX11 -lXext -L/usr/X11R6/lib @GTK_LIBS@

INCLUDES = @GTK_CFLAGS@ 

CFLAGS = -DLIBDIR=\""$(libdir)"\"


EXTRA_DIST = hourglass.xpm splash.xpm hgcursor.h hgcursor_mask.h getopt.h \
	sp1.xpm sp2.xpm sp3.xpm sp4.xpm sp5.xpm sp6.xpm sp7.xpm sp8.xpm sp9.xpm 


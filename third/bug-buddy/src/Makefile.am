buddydir = $(datadir)/bug-buddy

INCLUDES = \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	$(BUG_BUDDY_CFLAGS) -I$(includedir)		\
	-DBUDDY_DATADIR=\""$(buddydir)"\"		\
        -DBUDDY_ICONDIR=\""$(datadir)/pixmaps"\"	\
	-DREAL_DATADIR=\""$(datadir)"\"			\
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED	\
	-DGNOME_VFS_DISABLE_DEPRECATED	\
	-DLIBGLADE_DISABLE_DEPRECATED	\
	-D_GNU_SOURCE

bb_built_sources = \
	bb-marshal.c	\
	bb-marshal.h

bb-marshal.h: $(srcdir)/bb-marshal.list
	$(GLIB_GENMARSHAL) --prefix=_bb_marshal $(srcdir)/bb-marshal.list --header >> xgen-bmh \
	&& (cmp -s xgen-bmh bb-marshal.h || cp xgen-bmh bb-marshal.h) \
	&& rm -f xgen-bmh

bb-marshal.c: $(srcdir)/bb-marshal.list
	$(GLIB_GENMARSHAL) --prefix=_bb_marshal $(srcdir)/bb-marshal.list --body >> xgen-bmc \
	&& (cmp -s xgen-bmc bb-marshal.c || cp xgen-bmc bb-marshal.c) \
	&& rm -f xgen-bmc


bin_PROGRAMS = bug-buddy

$(bug_buddy_OBJECTS): $(bb_built_sources)

bug_buddy_SOURCES = \
	$(bb_built_sources)	\
	bug-buddy.c		\
	bug-buddy.h		\
	bugzilla.c		\
	bugzilla.h		\
	cell-renderer-uri.c	\
	cell-renderer-uri.h	\
	config.c		\
	gdb-buddy.c		\
	libglade-buddy.h	\
	united-states-of-bug-buddy.c

bug_buddy_LDADD = \
	$(BUG_BUDDY_LIBS)

noinst_PROGRAMS = gnome-crash

gnome_crash_SOURCES = gnome-crash.c

gnome_crash_LDADD = \
	$(GNOME_CRASH_LIBS)

# taken from gnumeric/Makefile.am
bug-buddy.keys: bug-buddy.keys.in Makefile
	sed -e 's^\@icondir\@^$(datadir)/pixmaps^g' < $(srcdir)/bug-buddy.keys.in > bug-buddy.keys.tmp \
	&& mv bug-buddy.keys.tmp bug-buddy.keys 

mimedir = $(datadir)/mime-info
mime_DATA = \
	bug-buddy.keys	\
	bug-buddy.mime

applicationsdir = $(datadir)/application-registry
applications_DATA = bug-buddy.applications

buddy_DATA = \
	bug-buddy.glade	\
	gdb-cmd

utilsdir = $(datadir)/applications
utils_in_files = bug-buddy.desktop.in
utils_DATA = $(utils_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	$(utils_DATA)		\
	$(utils_in_files)	\
	$(buddy_DATA)		\
	$(applications_DATA)	\
	bug-buddy.mime		\
	bug-buddy.keys.in	\
	bb-marshal.list

DONT_DIST_SOURCE = $(bb_built_sources)

dist-hook:
	for file in $(DONT_DIST_SOURCE) ; do \
		rm -f $(distdir)/$$file ; \
	done
if USING_OAF
oaf_programs=gnome-file-selection-test_corba
else
oaf_programs=
endif

noinst_PROGRAMS =				\
	gnome-file-selection-test_pthread	\
	$(oaf_programs)

INCLUDES =					\
	-I$(top_srcdir)/intl			\
	-I$(top_builddir)/intl			\
	-I$(top_srcdir)/libgnomevfs		\
	$(GNOME_INCLUDEDIR)			\
	$(GTK_CFLAGS)			\
	-I$(top_srcdir)				\
	-DICONDIR=\"$(datadir)/pixmaps/mc\"	\
	$(VFS_CFLAGS) $(WERROR)

common_sources =				\
	gicon.c					\
	gnome-file-selection.c			\
	gnome-file-selection-history.c

noinst_HEADERS =				\
	gicon.h					\
	gnome-file-selection.h			\
	gnome-file-selection-history.h

EXTRA_DIST =					\
	dir-close.xpm				\
	dir-open.xpm				\
	listing-iconic.xpm			\
	main.c

main_corba.o: main.c
	$(COMPILE) -DWITH_CORBA -o $@ -c $<
gnome_file_selection_test_corba_SOURCES = $(common_sources)
gnome_file_selection_test_corba_LDADD =				\
	main_corba.o						\
	$(top_builddir)/libgnomevfs/libgnomevfs.la		\
	$(top_builddir)/libgnomevfs-corba/libgnomevfs-corba.la	\
	$(GNOME_LIBDIR)						\
	$(GNOMEUI_LIBS)						\
	$(GNOMEGNORBA_LIBS)                                     \
        $(GCONF_LIBS)

main_pthread.o: main.c
	$(COMPILE) -DWITH_PTHREAD -o $@ -c $<
gnome_file_selection_test_pthread_SOURCES = $(common_sources)
gnome_file_selection_test_pthread_LDADD =				\
	main_pthread.o							\
	$(top_builddir)/libgnomevfs/libgnomevfs.la			\
	$(top_builddir)/libgnomevfs-pthread/libgnomevfs-pthread.la	\
	$(GNOME_LIBDIR)							\
	$(GNOMEUI_LIBS)

noinst_HEADERS =				\
	gicon.h					\
	gnome-file-selection-history.h		\
	gnome-file-selection.h


if USING_OAF
oaf_programs=test-async-directory_corba test-async_corba test-channel_corba
else
oaf_programs=
endif

SUBDIRS = gnome-file-selection

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/intl			\
	-I$(top_builddir)/intl			\
	-I$(top_srcdir)/libgnomevfs		\
	$(OAF_CFLAGS)				\
	$(GNOME_INCLUDEDIR)			\
	$(GTK_CFLAGS)				\
	$(VFS_CFLAGS) $(WERROR)

noinst_PROGRAMS =				\
	test-async-cancel                       \
	test-async-directory_pthread		\
	test-async_pthread			\
	test-channel_pthread			\
	test-directory				\
	test-directory-visit			\
	test-escape				\
	test-find-directory			\
	test-info				\
	test-mime				\
	test-mime-handlers			\
	test-mime-handlers-set			\
	test-seek				\
	test-symlinks				\
	test-sync				\
	test-sync-write				\
	test-sync-create			\
	test-xfer				\
	test-shell				\
	test-uri				\
	test-unlink				\
	test-dirop				\
	test-mime-info				\
	$(oaf_programs)

test_files=		\
	test.input	\
	test.cmds	\
	test.output

if HAVE_LIBEFS
auto_tests=auto-test
else
auto_tests=
endif

# Set up the environment so the tests can find the back end.
TESTS_ENVIRONMENT = LD_LIBRARY_PATH=$(top_builddir)/libgnomevfs-pthread/.libs/
TESTS = test-async-cancel test-escape test-uri $(auto_tests)

libraries =						\
	$(top_builddir)/libgnomevfs/libgnomevfs.la	\
	$(GNOME_LIBDIR)					\
	$(GNOMEUI_LIBS)					\
	$(GNOMEGNORBA_LIBS)                             \
	$(GCONF_LIBS)					\
	$(LIBEFS_LIBS)

test-async_pthread.o: test-async.c
	$(COMPILE) -DWITH_PTHREAD -o $@ -c $<
test_async_pthread_SOURCES = dummy.c
test_async_pthread_LDADD =						\
	test-async_pthread.o						\
	$(libraries)							\
	$(top_builddir)/libgnomevfs-pthread/libgnomevfs-pthread.la
test_async_pthread_LDFLAGS =

test-async_corba.o: test-async.c
	$(COMPILE) -DWITH_CORBA -o $@ -c $<
test_async_corba_SOURCES = dummy.c
test_async_corba_LDADD =					\
	test-async_corba.o					\
	$(libraries)						\
	$(top_builddir)/libgnomevfs-corba/libgnomevfs-corba.la
test_async_corba_LDFLAGS =

test-async-directory_pthread.o: test-async-directory.c
	$(COMPILE) -DWITH_PTHREAD -o $@ -c $<
test_async_directory_pthread_SOURCES = dummy.c
test_async_directory_pthread_LDADD =					\
	test-async-directory_pthread.o					\
	$(libraries)							\
	$(top_builddir)/libgnomevfs-pthread/libgnomevfs-pthread.la
test_async_directory_pthread_LDFLAGS =

test-async-directory_corba.o: test-async-directory.c
	$(COMPILE) -DWITH_CORBA -o $@ -c $<
test_async_directory_corba_SOURCES = dummy.c
test_async_directory_corba_LDADD =				\
	test-async-directory_corba.o				\
	$(libraries)						\
	$(top_builddir)/libgnomevfs-corba/libgnomevfs-corba.la
test_async_directory_corba_LDFLAGS =

test-channel_corba.o: test-channel.c
	$(COMPILE) -DWITH_CORBA -o $@ -c $<
test_channel_corba_SOURCES = dummy.c
test_channel_corba_LDADD =					\
	test-channel_corba.o					\
	$(libraries)						\
	$(top_builddir)/libgnomevfs-corba/libgnomevfs-corba.la
test_channel_corba_LDFLAGS =

test-channel_pthread.o: test-channel.c
	$(COMPILE) -DWITH_PTHREAD -o $@ -c $<
test_channel_pthread_SOURCES = dummy.c
test_channel_pthread_LDADD =					\
	test-channel_pthread.o					\
	$(libraries)						\
	$(top_builddir)/libgnomevfs-pthread/libgnomevfs-pthread.la
test_channel_pthread_LDFLAGS =

test_seek_SOURCES = test-seek.c
test_seek_LDADD = $(libraries)
test_seek_LDFLAGS =

test_shell_SOURCES = test-shell.c
test_shell_LDADD = $(libraries)
test_shell_LDFLAGS =

test_info_SOURCES = test-info.c
test_info_LDADD = $(libraries)
test_info_LDFLAGS =

test_mime_SOURCES = test-mime.c
test_mime_LDADD = $(libraries)
test_mime_LDFLAGS =

test_mime_handlers_SOURCES = test-mime-handlers.c
test_mime_handlers_LDADD = $(libraries)
test_mime_handlers_LDFLAGS =

test_mime_handlers_set_SOURCES = test-mime-handlers-set.c
test_mime_handlers_set_LDADD = $(libraries)
test_mime_handlers_set_LDFLAGS =

test_xfer_SOURCES = test-xfer.c
test_xfer_LDADD = $(libraries)
test_xfer_LDFLAGS =

test_directory_SOURCES = test-directory.c
test_directory_LDADD = $(libraries)
test_directory_LDFLAGS = 

test_directory_visit_SOURCES = test-directory-visit.c
test_directory_visit_LDADD = $(libraries)
test_directory_visit_LDFLAGS = 

test_symlinks_SOURCES = test-symlinks.c
test_symlinks_LDADD = $(libraries)
test_symlinks_LDFLAGS =

test_sync_SOURCES = test-sync.c
test_sync_LDADD = $(libraries)
test_sync_LDFLAGS =

test_sync_write_SOURCES = test-sync-write.c
test_sync_write_LDADD = $(libraries)
test_sync_write_LDFLAGS =

test_sync_create_SOURCES = test-sync-create.c
test_sync_create_LDADD = $(libraries)
test_sync_create_LDFLAGS =

test_async_cancel_SOURCES = test-async-cancel.c
test_async_cancel_LDADD = $(libraries)
test_async_cancel_LDFLAGS = 

test_escape_SOURCES = test-escape.c
test_escape_LDADD = $(libraries)
test_escape_LDFLAGS = 

test_uri_SOURCES = test-uri.c
test_uri_LDADD = $(libraries)
test_uri_LDFLAGS = 

test_unlink_SOURCES = test-unlink.c
test_unlink_LDADD = $(libraries)
test_unlink_LDFLAGS = 

test_dirop_SOURCES = test-dirop.c
test_dirop_LDADD = $(libraries)
test_dirop_LDFLAGS = 

test_find_directory_SOURCES = test-find-directory.c
test_find_directory_LDADD = $(libraries)
test_find_directory_LDFLAGS = 

test_mime_info_SOURCES = test-mime-info.c
test_mime_info_LDADD = $(libraries)
test_mime_info_LDFLAGS = 


EXTRA_DIST =					\
	$(test_files)				\
	test-async-directory.c			\
	test-async.c				\
	test-channel.c				\
	test-directory-visit.c			\
	test-directory.c			\
	test-symlinks.c				\
	test-seek.c				\
	test-sync.c				\
	test-sync-write.c			\
	test-sync-create.c			\
	test-xfer.c				\
	test-mime-info.c			\
	vfs-run.in

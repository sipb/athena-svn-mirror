NULL =

noinst_PROGRAMS = test-gail-gnome

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/gail-gnome		\
	$(GAIL_GNOME_DEBUG_CFLAGS)		\
	$(TEST_CFLAGS)				\
	$(NULL)

LDADD =						\
	../gail-gnome/libgail-gnome.la		\
	$(TEST_LIBS)				\
	$(NULL)

TESTS = test-gail-gnome

## noinst_PROGRAMS = testprint

INCLUDES = \
	-I$(top_srcdir) \
	$(LIBGNOMEPRINTUI_CFLAGS) \
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DBINDIR=\""$(bindir)"\" \
	-DDATADIR=\""$(datadir)"\"

printui_libs = \
	$(top_builddir)/libgnomeprintui/libgnomeprintui-2.la \
	$(LIBGNOMEPRINTUI_LIBS) \
	-lpopt

## testprint_SOURCES = testprint.c
## testprint_LDFLAGS =
## testprint_DEPENDENCIES = $(DEPS)
## testprint_LDADD = $(printui_libs)


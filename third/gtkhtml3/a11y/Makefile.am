END=

INCLUDES = -I$(top_srcdir)/src $(GTKHTML_CFLAGS)

CFLAGS += $(GAIL_CFLAGS) -DGTK_DISABLE_DEPRECATED=1 -DGDK_DISABLE_DEPRECATED=1 -DG_DISABLE_DEPRECATED=1 -DGNOME_DISABLE_DEPRECATED=1

lib_LTLIBRARIES = libgtkhtml-a11y-3.0.la

libgtkhtml_a11y_3_0_la_LDFLAGS = -version-info $(GTKHTML_CURRENT):$(GTKHTML_REVISION):$(GTKHTML_AGE)
libgtkhtml_a11y_3_0_la_LIBADD = $(GTKHTML_LIBS) $(GAIL_LIBS)

libgtkhtml_a11y_3_0_la_SOURCES =			\
	cell.h						\
	cell.c						\
	factory.h					\
	factory.c					\
	html.h						\
	html.c						\
	hyperlink.h					\
	hyperlink.c					\
	image.h						\
	image.c						\
	link.h						\
	link.c						\
	object.h					\
	object.c					\
	paragraph.h					\
	paragraph.c					\
	table.h						\
	table.c						\
	text.h						\
	text.c						\
	utils.h						\
	utils.c						\
	$(END)

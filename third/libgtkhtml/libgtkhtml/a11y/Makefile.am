INCLUDES = \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libgtkhtml \
	$(GTKHTML2_CFLAGS) \
	$(GAILUTIL_CFLAGS) \
	-DG_LOG_DOMAIN=\"HtmlA11y\"

if ENABLE_ACCESSIBILITY
noinst_LTLIBRARIES = libhtmla11y.la
endif

libhtmla11y_la_SOURCES = \
	htmlaccessiblefactory.h \
	htmlviewaccessible.c \
	htmlviewaccessible.h \
	htmlviewaccessiblefactory.c \
	htmlviewaccessiblefactory.h \
	htmlboxaccessible.c	\
	htmlboxaccessible.h	\
	htmlboxblockaccessible.c	\
	htmlboxblockaccessible.h	\
	htmlboxembeddedaccessible.c	\
	htmlboxembeddedaccessible.h	\
	htmlboxtableaccessible.c	\
	htmlboxtableaccessible.h	\
	htmlboxtextaccessible.c	\
	htmlboxtextaccessible.h	\
	htmllinkaccessible.c	\
	htmllinkaccessible.h

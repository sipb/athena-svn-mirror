noinst_LTLIBRARIES = libunicode.la

libunicode_la_SOURCES = 		\
	gunibreak.c			\
	gunicollate.c			\
	gunidecomp.c			\
	guniprop.c			\
	gutf8.c

libunicodeincludedir = $(includedir)/gal/unicode

libunicodeinclude_HEADERS = 		\
	gunicode.h

noinst_HEADERS =			\
	gunibreak.h			\
	gunichartables.h		\
	gunicomp.h			\
	gunidecomp.h

INCLUDES = 				\
	-I$(top_srcdir)			\
	$(GNOME_INCLUDEDIR)		\
	$(ICONV_CFLAGS)

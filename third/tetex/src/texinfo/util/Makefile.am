## Makefile.am for texinfo/util.
## $Id: Makefile.am,v 1.1.1.1 2000-03-10 17:53:16 ghudson Exp $
## Run automake in .. to produce Makefile.in from this.

bin_PROGRAMS = install-info texindex
bin_SCRIPTS = texi2dvi

ZLIBDIR=../../libs/zlib
ZLIBSRCDIR=$(srcdir)/$(ZLIBDIR)
ZLIBDEP = @ZLIBDEP@
install_info_DEPENDENCIES =  ../lib/libtxi.a @ZLIBDEP@
install_info_LDADD = $(LDADD) @LDZLIB@
localedir = $(datadir)/locale
INCLUDES = -I$(top_srcdir)/lib -I../intl -DLOCALEDIR=\"$(localedir)\" @ZLIBCPPFLAGS@
LDADD = ../lib/libtxi.a @INTLLIBS@

EXTRA_DIST = README deref.c fixfonts gen-dir-node tex3patch texi2dvi \
             update-info 

$(ZLIBDIR)/zlib.a: $(ZLIBSRCDIR)/*.c
	$(MAKE) $(MFLAGS) CC='$(CC)' CFLAGS='$(CFLAGS)' zlib.a

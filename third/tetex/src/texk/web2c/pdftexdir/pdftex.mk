# Makefile fragment for pdfTeX and web2c. --infovore@xs4all.nl. Public domain.
# This fragment contains the parts of the makefile that are most likely to
# differ between releases of pdfTeX.

# The libraries are not mentioned.  As the matter stands, a change in their
# number or how they are configured requires changes to the main distribution
# anyway.

Makefile: $(srcdir)/pdftexdir/pdftex.mk

# The C sources.
pdftex_c = pdftexini.c pdftex0.c pdftex1.c pdftex2.c
pdftex_o = pdftexini.o pdftex0.o pdftex1.o pdftex2.o pdftexextra.o

# Generation of the web and ch files.
pdftex.web: tie tex.web pdftexdir/pdftex.ch
	./tie -m pdftex.web $(srcdir)/tex.web $(srcdir)/pdftexdir/pdftex.ch
pdftex.ch: tie pdftex.web tex.ch pdftexdir/tex.pch
	./tie -c pdftex.ch pdftex.web $(srcdir)/tex.ch $(srcdir)/pdftexdir/tex.pch

linux_build_dir = $(HOME)/pdftex/build/linux/texk/web2c

pdftex_bin = pdftex pdfetex ttf2afm
pdftex_exe = pdftex.exe pdfetex.exe ttf2afm.exe
pdftex_pool = pdftex.pool pdfetex.pool

pdftex-djgpp: $(web2c_programs)
	@if test ! -x $(linux_build_dir)/tangle; then echo Error: linux_build_dir not ready; exit -1; fi
	rm -f web2c/fixwrites web2c/splitup web2c/web2c
	ln -s $(linux_build_dir)/web2c/fixwrites web2c
	ln -s $(linux_build_dir)/web2c/splitup web2c
	ln -s $(linux_build_dir)/web2c/web2c web2c
	touch web2c/fixwrites web2c/splitup web2c/web2c
	make tie
	rm -f tie
	ln -s $(linux_build_dir)/tie .
	touch tie
	make tangleboot
	rm -f tangleboot
	ln -s $(linux_build_dir)/tangleboot .
	touch tangleboot
	make tangle
	rm -f tangle
	ln -s $(linux_build_dir)/tangle .
	touch tangle
	make pdftex pdfetex

pdftex.zip: $(pdftex_bin)
	rm -f $@ $(pdftex_bin)
	XLDFLAGS=-static make $(pdftex_bin)
	strip $(pdftex_bin)
	zip $@ $(pdftex_bin) $(pdftex_pool)

pdftex-djgpp.zip: $(pdftex_bin)
	rm -f $@ $(pdftex_bin)
	XLDFLAGS=-static make $(pdftex_bin)
	dos-strip $(pdftex_bin)
	dos-stubify $(pdftex_bin)
	zip $@ $(pdftex_exe) $(pdftex_pool)

pdftex_sources = \
    Makefile.in \
    configure \
    configure.in \
    multiplatform.ac \
    reautoconf \
    tetex.ac \
    withenable.ac \
    config/ \
    libs/configure \
    libs/configure.in \
    libs/libpng/ \
    libs/zlib/ \
    texk/web2c/configure \
    texk/web2c/configure.in \
    texk/web2c/stamp-auto.in \
    texk/web2c/fmtutil.in \
    texk/web2c/pdftexdir/ \
    texk/web2c/pdfetexdir/

pdftexsrc.tgz:
	cd $(kpathsea_srcdir_parent)/..; tar cvfz $(HOME)/pdftex/arch/$@ $(pdftex_sources)

pdftexlib.zip:
	cd $(HOME)/pdftex; rm -f $(HOME)/pdftex/arch/$@; zip -r $(HOME)/pdftex/arch/$@ texmf

ttf2afm: ttf2afm.o
	$(kpathsea_link) ttf2afm.o
ttf2afm.o: ttf2afm.c macnames.c
	$(compile) $<
ttf2afm.c:
	$(LN) $(srcdir)/pdftexdir/ttf2afm.c .
macnames.c:
	$(LN) $(srcdir)/pdftexdir/macnames.c .

# end of pdftex.mk

# Makefile fragment for pdfTeX and web2c. --infovore@xs4all.nl. Public domain.
# This fragment contains the parts of the makefile that are most likely to
# differ between releases of pdfTeX.

# The libraries are not mentioned.  As the matter stands, a change in their
# number or how they are configured requires changes to the main distribution
# anyway.

Makefile: $(srcdir)/pdftexdir/pdftex.mk

# The C sources.
pdftex_c = pdftexini.c pdftex0.c pdftex1.c pdftex2.c pdftex3.c
pdftex_o = pdftexini.o pdftex0.o pdftex1.o pdftex2.o pdftex3.o pdftexextra.o

pdftex_bin = pdftex pdfetex ttf2afm pdftosrc
pdftex_exe = pdftex.exe pdfetex.exe ttf2afm.exe pdftosrc.exe
pdftex_pool = pdftex.pool pdfetex.pool
linux_build_dir = $(HOME)/pdftex/build/linux/texk/web2c

# Generation of the web and ch files.
pdftex.web: tie tex.web pdftexdir/pdftex.ch
	./tie -m pdftex.web $(srcdir)/tex.web $(srcdir)/pdftexdir/pdftex.ch
pdftex.ch: tie pdftex.web pdftexdir/tex.ch0 tex.ch pdftexdir/tex.ch1 \
           pdftexdir/tex.pch pdftexdir/tex.ch2
	./tie -c pdftex.ch pdftex.web \
	$(srcdir)/pdftexdir/tex.ch0 \
	$(srcdir)/tex.ch \
	$(srcdir)/pdftexdir/tex.ch1 \
	$(srcdir)/pdftexdir/tex.pch \
	$(srcdir)/pdftexdir/tex.ch2

# ttf2afm
ttf2afm: ttf2afm.o
	$(kpathsea_link) ttf2afm.o $(kpathsea)
ttf2afm.o: ttf2afm.c macnames.c
	$(compile) $<
ttf2afm.c:
	$(LN) $(srcdir)/pdftexdir/ttf2afm.c .
macnames.c:
	$(LN) $(srcdir)/pdftexdir/macnames.c .

# pdftosrc
pdftosrc: pdftexdir/pdftosrc.o
	$(kpathsea_cxx_link) pdftexdir/pdftosrc.o $(LDLIBXPDF) -lm
pdftexdir/pdftosrc.o:$(srcdir)/pdftexdir/pdftosrc.cc
	cd pdftexdir && $(MAKE) pdftosrc.o

# pdftex binaries archive
pdftexbin:
	rm -f pdtex*.zip $(pdftex_bin)
	XLDFLAGS=-static $(MAKE) $(pdftex_bin)
	if test "x$(CC)" = "xdos-gcc"; then \
	    $(MAKE) pdftexbin-djgpp; \
	elif test "x$(CC)" = "xi386-mingw32-gcc"; then \
	    $(MAKE) pdftexbin-mingw32; \
	elif test "x$(CC)" = "xgnuwin32gcc"; then \
	    $(MAKE) pdftexbin-gnuwin32; \
	else \
	    $(MAKE) pdftexbin-native; \
	fi

pdftex-cross:
	$(MAKE) web2c-cross
	$(MAKE) pdftexbin

web2c-cross: $(web2c_programs)
	@if test ! -x $(linux_build_dir)/tangle; then echo Error: linux_build_dir not ready; exit -1; fi
	rm -f web2c/fixwrites web2c/splitup web2c/web2c
	cp -f $(linux_build_dir)/web2c/fixwrites web2c
	cp -f $(linux_build_dir)/web2c/splitup web2c
	cp -f $(linux_build_dir)/web2c/web2c web2c
	touch web2c/fixwrites web2c/splitup web2c/web2c
	$(MAKE) tie
	rm -f tie
	cp -f $(linux_build_dir)/tie .
	touch tie
	$(MAKE) tangleboot
	rm -f tangleboot
	cp -f $(linux_build_dir)/tangleboot .
	touch tangleboot
	$(MAKE) tangle
	rm -f tangle
	cp -f $(linux_build_dir)/tangle .
	touch tangle

pdftexbin-native:
	strip $(pdftex_bin)
	zip pdftex-native-`datestr`.zip $(pdftex_bin) $(pdftex_pool)

pdftexbin-djgpp:
	dos-strip $(pdftex_bin)
	dos-stubify $(pdftex_bin)
	zip pdftex-djgpp-`datestr`.zip $(pdftex_exe) $(pdftex_pool)

pdftexbin-mingw32:
	i386-mingw32-strip $(pdftex_bin)
	for f in $(pdftex_bin); do mv $$f $$f.exe; done
	zip pdftex-mingw32-`datestr`.zip $(pdftex_exe) $(pdftex_pool)

pdftexbin-gnuwin32:
	gnuwin32strip $(pdftex_bin)
	for f in $(pdftex_bin); do mv $$f $$f.exe; done
	zip pdftex-gnuwin32-`datestr`.zip $(pdftex_exe) $(pdftex_pool)

# end of pdftex.mk

# Makefile fragment for pdfeTeX and web2c. --infovore@xs4all.nl. Public domain.
# This fragment contains the parts of the makefile that are most likely to
# differ between releases of pdfeTeX.

Makefile: $(srcdir)/pdfetexdir/pdfetex.mk

# The C sources.
pdfetex_c = pdfetexini.c pdfetex0.c pdfetex1.c pdfetex2.c pdfetex3.c
pdfetex_o = pdfetexini.o pdfetex0.o pdfetex1.o pdfetex2.o pdfetex3.o pdfetexextra.o

# Generation of the web and ch file.
pdfetex.web: tie tex.web etexdir/etex.ch etexdir/etex.fix \
             pdfetexdir/pdfetex.ch1 pdftexdir/pdftex.ch pdfetexdir/pdfetex.ch2 \
             $(srcdir)/pdfetexdir/pdfetex.h $(srcdir)/pdfetexdir/pdfetex.defines
	./tie -m pdfetex.web $(srcdir)/tex.web \
		$(srcdir)/etexdir/etex.ch \
		$(srcdir)/etexdir/etex.fix \
		$(srcdir)/pdfetexdir/pdfetex.ch1 \
		$(srcdir)/pdftexdir/pdftex.ch \
		$(srcdir)/pdfetexdir/pdfetex.ch2
pdfetex.ch: tie pdfetex.web pdfetexdir/tex.ch0 tex.ch etexdir/tex.ch1 \
            etexdir/tex.ech etexdir/tex.ch2 pdfetexdir/tex.ch1 \
	    pdftexdir/tex.pch pdfetexdir/tex.ch2
	./tie -c pdfetex.ch pdfetex.web \
		$(srcdir)/pdfetexdir/tex.ch0 \
		$(srcdir)/tex.ch \
		$(srcdir)/etexdir/tex.ch1 \
		$(srcdir)/etexdir/tex.ech \
		$(srcdir)/etexdir/tex.ch2 \
		$(srcdir)/pdfetexdir/tex.ch1 \
		$(srcdir)/pdftexdir/tex.pch \
		$(srcdir)/pdfetexdir/tex.ch2

$(srcdir)/pdfetexdir/pdfetex.h: $(srcdir)/pdftexdir/pdftex.h
	cp -f $(srcdir)/pdftexdir/pdftex.h $@

$(srcdir)/pdfetexdir/pdfetex.defines: $(srcdir)/pdftexdir/pdftex.defines
	cp -f $(srcdir)/pdftexdir/pdftex.defines $@

# end of pdfetex.mk

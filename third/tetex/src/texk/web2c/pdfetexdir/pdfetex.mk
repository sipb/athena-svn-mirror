# Makefile fragment for pdfeTeX and web2c. --infovore@xs4all.nl. Public domain.
# This fragment contains the parts of the makefile that are most likely to
# differ between releases of pdfeTeX.

Makefile: $(srcdir)/pdfetexdir/pdfetex.mk

# The C sources.
pdfetex_c = pdfetexini.c pdfetex0.c pdfetex1.c pdfetex2.c pdfetex3.c
pdfetex_o = pdfetexini.o pdfetex0.o pdfetex1.o pdfetex2.o pdfetex3.o pdfetexextra.o

# Generation of the web and ch file.
pdfetex.web: tie tex.web etexdir/etex.ch \
             pdfetexdir/pdfetex.ch1 pdftexdir/pdftex.ch pdfetexdir/pdfetex.ch2 \
             $(srcdir)/pdfetexdir/pdfetex.h $(srcdir)/pdfetexdir/pdfetex.defines
	./tie -m pdfetex.web $(srcdir)/tex.web $(srcdir)/etexdir/etex.ch \
		 $(srcdir)/pdfetexdir/pdfetex.ch1 \
		 $(srcdir)/pdftexdir/pdftex.ch \
		 $(srcdir)/pdfetexdir/pdfetex.ch2
pdfetex.ch: tie pdfetex.web tex.ch etexdir/tex.ech \
            pdfetexdir/tex.ch1 pdftexdir/tex.pch pdfetexdir/tex.ch2
	./tie -c pdfetex.ch pdfetex.web \
		 $(srcdir)/tex.ch $(srcdir)/etexdir/tex.ech \
		 \
		 $(srcdir)/pdfetexdir/tex.ch1 \
		 $(srcdir)/pdftexdir/tex.pch \
		 $(srcdir)/pdfetexdir/tex.ch2

$(srcdir)/pdfetexdir/pdfetex.h: $(srcdir)/pdftexdir/pdftex.h
	rm -f $@; cp $(srcdir)/pdftexdir/pdftex.h $@

$(srcdir)/pdfetexdir/pdfetex.defines: $(srcdir)/pdftexdir/pdftex.defines
	rm -f $@; cp $(srcdir)/pdftexdir/pdftex.defines $@

# end of pdfetex.mk

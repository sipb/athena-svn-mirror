test_TEXS = \
	simple-links.tex

if HAVE_PDFTEX
test_PDFS = $(test_TEXS:.tex=.pdf)
else
test_PDFS =
endif

noinst_DATA = \
	$(test_PDFS)

%.pdf: $(srcdir)/%.tex
	-$(PDFTEX) --file-line-error-style --interaction nonstopmode  $< 

EXTRA_DIST = $(test_TEXS) encrypted.pdf
CLEANFILES = $(test_PDFS) $(test_PDFS:.pdf=.log)

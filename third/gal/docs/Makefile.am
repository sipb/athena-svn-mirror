DOC_MODULE=gal

DOC_MAIN_SGML_FILE=gal-api.sgml

DOC_SOURCE_DIR=$(top_srcdir)/gal

CFLAGS="-I../../ `gnome-config --cflags gal`"
LDFLAGS="-I../../ `gnome-config --libs gal`"

TARGET_DIR=$(HTML_DIR)/$(DOC_MODULE)

tmpl_sources =

gal_docdir = $(HTML_DIR)

gal_doc_DATA =			\
	gal-api.sgml		\
	gal-api.html		\
	gal.hierarchy		\
	gal.signals		\
	gal.types		\
	gal-decl.txt		\
	gal-sections.txt

if ENABLE_GTK_DOC
gal-api.html: html/book1.html
	-cp $(srcdir)/html/book1.html $(srcdir)/gal-api.html
else
gal-api.html:
endif

html/book1.html: sgml/gal-api-doc.bottom
	$(MAKE) html

sgml/gal-api-doc.bottom: $(tmpl_sources)
	$(MAKE) sgml

scan:
	-(cd $(srcdir) \
	&& env CFLAGS=$(CFLAGS) LDFLAGS=$(LDFLAGS) \
		 gtkdoc-scanobj --module=$(DOC_MODULE) 		\
	&& gtkdoc-scan --module=$(DOC_MODULE) --source-dir=$(top_srcdir)/gal \
	--ignore-headers='*.glade.h' )


templates: scan
	cd $(srcdir) && gtkdoc-mktmpl --module=$(DOC_MODULE)

sgml:
	cd $(srcdir) \
	&& gtkdoc-mkdb --module=$(DOC_MODULE) --source-dir=../gal

html:
	test -d $(srcdir)/html || mkdir $(srcdir)/html
	-cd $(srcdir)/html && gtkdoc-mkhtml $(DOC_MODULE) ../$(DOC_MAIN_SGML_FILE)

clean-local:
	rm -f *~ *.bak *-unused.txt

maintainer-clean-local: clean
	cd $(srcdir) && rm -rf sgml html $(DOC_MODULE)-decl-list.txt $(DOC_MODULE)-decl.txt

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(TARGET_DIR)
	(installfiles=`echo $(srcdir)/html/*.html`; \
	if test "$$installfiles" = '$(srcdir)/html/*.html'; \
	then echo '-- Nothing to install' ; \
	else \
	  for i in $$installfiles; do \
	    echo '-- Installing '$$i ; \
	    $(INSTALL_DATA) $$i $(DESTDIR)$(TARGET_DIR); \
	  done; \
	  echo '-- Installing $(srcdir)/html/index.sgml' ; \
	  $(INSTALL_DATA) $(srcdir)/html/index.sgml $(DESTDIR)$(TARGET_DIR); \
	  echo '-- Fixing Crossreferences' ; \
	  gtkdoc-fixxref --module=$(DOC_MODULE) --html-dir=$(HTML_DIR)|| true; \
	fi)

dist-hook:
	mkdir $(distdir)/html
	mkdir $(distdir)/sgml
	mkdir $(distdir)/tmpl
	-cp $(srcdir)/html/*.html $(srcdir)/html/*.css $(distdir)/html
	-cp $(srcdir)/tmpl/*.sgml $(distdir)/tmpl
	-cp $(srcdir)/sgml/*.sgml $(distdir)/sgml
	-cp $(srcdir)/sgml/*.bottom $(srcdir)/sgml/*.top $(distdir)/sgml

.PHONY : html sgml templates scan


EXTRA_DIST = $(gal_doc_DATA)

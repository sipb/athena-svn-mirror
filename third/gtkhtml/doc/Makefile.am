## Process this file with automake to produce Makefile.in

# The name of the module.
DOC_MODULE=gtkhtml

# The top-level SGML file.
DOC_MAIN_SGML_FILE=gtkhtml.sgml

# The directory containing the source code (if it contains documentation).
DOC_SOURCE_DIR=$(top_srcdir)/src

CFLAGS += $(EXTRA_GNOME_CFLAGS)
LDFLAGS += $(EXTRA_GNOME_LIBS)

HTML_DIR=$(datadir)/gnome/html

TARGET_DIR=$(HTML_DIR)/$(DOC_MODULE)

tmpl_sources =				\
	tmpl/gtkhtml.sgml		\
	tmpl/gtkhtml-types.sgml		\
	tmpl/gtkhtml-enums.sgml		\
	tmpl/gtkhtml-embedded.sgml	\
	tmpl/gtkhtml-stream.sgml


gtkhtml_docdir = $(HTML_DIR)
gtkhtml_doc_DATA =			\
	$(DOC_MAIN_SGML_FILE)		\
	gtkhtml.hierarchy		\
	gtkhtml.types			\
	gtkhtml-decl.txt		\
	gtkhtml-sections.txt

content_files =				\
	gtkhtml.sgml

EXTRA_DIST = 				\
	$(gtkhtml_doc_DATA)		\
	$(content_files)

if ENABLE_GTK_DOC
all:	html/index.html
else
all:
endif

html/index.html: sgml/gtkhtml-doc.bottom $(content_files)
	$(MAKE) html

sgml/gtkhtml-doc.bottom: $(tmpl_sources)
	$(MAKE) sgml

scan:
	-(cd $(srcdir) \
	&& env CFLAGS="$(CFLAGS) -I$(top_srcdir)/src" LDFLAGS="$(LDFLAGS) -L$(top_srcdir) -lgtkhtml" \
		 gtkdoc-scanobj --module=$(DOC_MODULE) \
	&& gtkdoc-scan --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR) --ignore-headers="")

templates: scan
	cd $(srcdir) && gtkdoc-mktmpl --module=$(DOC_MODULE)

sgml:
	cd $(srcdir) \
	&& gtkdoc-mkdb --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR)

html:
	test -d $(srcdir)/html || mkdir $(srcdir)/html
	-cd $(srcdir)/html && gtkdoc-mkhtml $(DOC_MODULE) ../$(DOC_MAIN_SGML_FILE)

clean-local:
	rm -f *~ *.bak *.signals *-unused.txt

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

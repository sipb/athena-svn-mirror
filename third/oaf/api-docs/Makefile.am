NULL=

# The name of the module.
DOC_MODULE=liboaf

# The top-level SGML file.
DOC_MAIN_SGML_FILE=liboaf.sgml

# The directory containing the source code (if it contains documentation).
DOC_SOURCE_DIR=$(top_srcdir)/liboaf

HTML_DIR=$(datadir)/oaf/html

TARGET_DIR=$(HTML_DIR)/$(DOC_MODULE)

tmpl_sources =				\
	tmpl/utilities.sgml		\
	tmpl/initialization.sgml	\
	tmpl/query.sgml			\
	tmpl/registration.sgml		\
	$(NULL)


liboaf_docdir = $(HTML_DIR)
liboaf_doc_DATA =			\
	liboaf.html			\
	$(NULL)


my_sgml_files = 			\
	tutorial.sgml			\
	migrating.sgml			\
	oafinfo-ref.sgml		\
	configuring.sgml		\
	query-langage.sgml		\
	architecture.sgml		\
	liboaf.sgml			\
	fdl.sgml			\
	$(NULL)

EXTRA_DIST = 			\
	$(my_sgml_files)	\
	oaf.png			\
	liboaf-decl.txt		\
	liboaf-sections.txt	\
	$(NULL)

if ENABLE_GTK_DOC
liboaf.html:
	if test ! -d $(top_builddir)/api-docs/tmpl; then			\
		mkdir $(top_builddir)/api-docs/tmpl; 				\
		cp $(srcdir)/tmpl/*.sgml $(top_builddir)/api-docs/tmpl/; 	\
		cp $(srcdir)/*.sgml $(srcdir)/*.txt  $(top_builddir)/api-docs/; \
	fi
	cd $(top_builddir)/api-docs/ \
	&& gtkdoc-mkdb --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR)
	if test ! -d $(top_builddir)/api-docs/html; then	\
		mkdir $(top_builddir)/api-docs/html;		\
	fi
	-cd $(top_builddir)/api-docs/html && \
	gtkdoc-mkhtml $(DOC_MODULE) ../$(DOC_MAIN_SGML_FILE)
	-cd $(top_builddir)/api-docs/ && \
	cp $(srcdir)/oaf.png $(top_builddir)/api-docs/html

else

liboaf.html:

endif



templates:
	cd $(srcdir) && gtkdoc-mktmpl --module=$(DOC_MODULE)


clean-local:
	rm -f *~ *.bak *.signals *-unused.txt

maintainer-clean-local: clean
	cd $(top_builddir)/api-docs/ && rm -rf sgml html $(DOC_MODULE)-decl-list.txt $(DOC_MODULE)-decl.txt

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

dist-hook: sgml html
	mkdir $(distdir)/html
	mkdir $(distdir)/tmpl
	-cp $(srcdir)/html/* $(distdir)/html
	-cp $(srcdir)/tmpl/*.sgml $(distdir)/tmpl

.PHONY : html sgml templates scan



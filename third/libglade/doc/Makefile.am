## Process this file with automake to produce Makefile.in
# The name of the module.
DOC_MODULE=libglade

# The top-level SGML file.
DOC_MAIN_SGML_FILE=libglade-docs.sgml

# The directory containing the source code (if it contains documentation).
DOC_SOURCE_DIR=../glade

HTML_DIR=$(datadir)/gnome/html

TARGET_DIR=$(HTML_DIR)/$(DOC_MODULE)

if GNOME_SUPPORT
CFLAGS = $(GNOME_INCLUDEDIR) $(XML_CFLAGS) -I$(srcdir)/..
LDFLAGS = ../glade/libglade.la $(GNOME_LIBDIR) $(GNOMEUI_LIBS) $(XML_LIBS)
else
CFLAGS = $(GTK_CFLAGS) $(XML_CFLAGS) -I$(srcdir)/..
LDFLAGS = ../glade/libglade.la $(GTK_LIBS) $(XML_LIBS)
endif

SCANOBJ_FILES = $(DOC_MODULE).signals $(DOC_MODULE).hierachy $(DOC_MODULE).args

tmpl_sources =				\
	tmpl/glade.sgml			\
	tmpl/glade-xml.sgml		\
	tmpl/glade-build.sgml		\
	tmpl/libglade-unused.sgml

noinst_DATA = html/libglade.html

EXTRA_DIST = 				\
	libglade-decl.txt		\
	libglade-sections.txt		\
	libglade-docs.sgml		\
	libglade.types

if HAVE_GTK_DOC
html/libglade.html: sgml/glade.sgml
	$(MAKE) html
else
html/libglade.html:
endif

sgml/glade.sgml: $(tmpl_sources) libglade.hierarchy libglade-decl.txt
	$(MAKE) sgml	

libglade.hierarchy libglade-decl.txt:
	$(MAKE) templates

scan:
	env CC="$(LIBTOOL) --mode=compile $(CC)" \
		LD="$(LIBTOOL) --mode=link $(CC)" \
		CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" \
		gtkdoc-scanobj --module=$(DOC_MODULE) \
		--types=$(srcdir)/$(DOC_MODULE).types --nogtkinit
	-mv -f $(SCANOBJ_FILES) $(srcdir)
	cd $(srcdir) \
	&& gtkdoc-scan --module=$(DOC_MODULE) \
		       --source-dir=$(DOC_SOURCE_DIR) \
		       --ignore-headers=glade-private.h

templates: scan
	cd $(srcdir) && gtkdoc-mktmpl --module=$(DOC_MODULE)

sgml:
	test -d $(srcdir)/sgml || mkdir $(srcdir)/sgml
	cd $(srcdir) \
	&& gtkdoc-mkdb --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR)

html:
	test -d $(srcdir)/html || mkdir $(srcdir)/html
	-cd $(srcdir)/html && gtkdoc-mkhtml $(DOC_MODULE) ../$(DOC_MAIN_SGML_FILE)

clean-local:
	rm -f *~ *.bak *.hierarchy *.signals *-unused.txt
	rm -f libglade-scan.lo libglade-scan.o

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
	  $(INSTALL_DATA) $(srcdir)/html/index.sgml $(TARGET_DIR); \
	  echo '-- Fixing Crossreferences' ; \
	  gtkdoc-fixxref --module=$(DOC_MODULE) --html-dir=$(HTML_DIR) || true; \
	fi)

uninstall-local:
	rm -f $(DESTDIR)$(TARGET_DIR)/*
distclean-local:
	rm -f .libs/*

dist-hook:
	$(mkinstalldirs) $(DESTDIR)$(distdir)/html
	$(mkinstalldirs) $(DESTDIR)$(distdir)/sgml
	$(mkinstalldirs) $(DESTDIR)$(distdir)/tmpl
	-cp $(srcdir)/html/*.html $(srcdir)/html/*.css $(DESTDIR)$(distdir)/html
	-cp $(srcdir)/tmpl/*.sgml $(DESTDIR)$(distdir)/tmpl
	-cp $(srcdir)/sgml/*.sgml $(DESTDIR)$(distdir)/sgml

.PHONY : html sgml templates scan

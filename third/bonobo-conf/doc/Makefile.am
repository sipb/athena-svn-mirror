## Process this file with automake to produce Makefile.in

# The name of the module.
DOC_MODULE=bonobo-conf

# The top-level SGML file.
DOC_MAIN_SGML_FILE=$(DOC_MODULE).sgml

# The directory containing the source code (if it contains documentation).
DOC_SOURCE_DIR=../bonobo-conf

TARGET_DIR=$(HTML_DIR)/$(DOC_MODULE)

DOC_FILES= 						\
	$(DOC_SOURCE_DIR)/bonobo-config-database.h	\
	$(DOC_SOURCE_DIR)/bonobo-config-utils.h		\
	$(DOC_SOURCE_DIR)/bonobo-property-editor.h	

scan:
	gtkdoc-scan --module=$(DOC_MODULE) $(DOC_FILES)

templates: scan
	gtkdoc-mktmpl --module=$(DOC_MODULE)

sgml:
	gtkdoc-mkdb --module=$(DOC_MODULE) --source-dir=$(DOC_SOURCE_DIR)

html:
	if ! test -d html ; then mkdir html ; fi
	-cd html && gtkdoc-mkhtml $(DOC_MODULE) ../$(DOC_MAIN_SGML_FILE)
	-cp pictures/*gif html

print:
	-jade -t tex -d /usr/lib/sgml/stylesheet/dsssl/docbook/nwalsh/print/docbook.dsl $(DOC_MODULE).sgml
	-mkdir print
	cd print;pdfjadetex ../$(DOC_MODULE).tex;pdfjadetex ../$(DOC_MODULE).tex;
	cp print/$(DOC_MODULE).pdf $(DOC_MODULE).pdf 

clean-local:
	rm -f *~ *.bak */*.bak *.hierarchy *.signals *.args *-unused.txt *.log $(DOC_MODULE).tex

maintainer-clean-local: clean
	rm -rf sgml html print $(DOC_MODULE)-decl-list.txt $(DOC_MODULE)-decl.txt $(DOC_MODULE).tex $(DOC_MODULE).ps *.stamp

install-data-local:
	-install -d -m 0755 $(TARGET_DIR)
	-install -m 0644 html/*.html $(TARGET_DIR)
	-install -m 0644 html/index.sgml $(TARGET_DIR)
	-gtkdoc-fixxref --module=$(DOC_MODULE) --html-dir=$(HTML_DIR)

.PHONY : print html sgml templates scan

EXTRA_DIST = $(DOC_MODULE).pdf $(DOC_MODULE).sgml
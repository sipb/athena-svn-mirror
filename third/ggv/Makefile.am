SUBDIRS = po idl src bonobo pixmaps help omf-install

DESKTOP_IN_FILES=ggv.desktop.in
DESKTOP_FILES=$(DESKTOP_IN_FILES:.desktop.in=.desktop)

schemasdir = $(sysconfdir)/gconf/schemas
schemas_DATA = ggv.schemas

EXTRA_DIST = 			\
	$(DESKTOP_FILES)	\
	$(DESKTOP_IN_FILES)	\
	ggv.spec.in 		\
	ggv.spec		\
	ggv.schemas		\
	xmldocs.make		\
        intltool-extract.in     \
	intltool-update.in      \
        intltool-merge.in       \
	COPYING-DOCS		\
	TODO			\
	MAINTAINERS

Graphicsdir = $(datadir)/applications
Graphics_DATA = $(DESKTOP_FILES)

@INTLTOOL_DESKTOP_RULE@

install-schemas:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --shutdown
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_srcdir)/ggv.schemas

if INSTALL_SCHEMAS
install-data-local: install-schemas
endif


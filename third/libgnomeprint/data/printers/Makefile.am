
printersdir=$(GNOME_PRINT_DATA_DIR)/printers
printers_in_files = \
	GENERIC.xml.in \
	PDF-WRITER.xml.in
printers_DATA=$(printers_in_files:.xml.in=.xml)

if INSTALL_META_PRINTER
privateprintersdir=$(GNOME_PRINT_DATA_DIR)/printers
else
privateprintersdir=$(GNOME_PRINT_DATA_DIR)/printers/uninstalled
endif
privateprinters_DATA=META.xml

@INTLTOOL_XML_RULE@

EXTRA_DIST=$(printers_DATA) $(privateprinters_DATA) $(printers_in_files)

CLEANFILES = $(printers_DATA)

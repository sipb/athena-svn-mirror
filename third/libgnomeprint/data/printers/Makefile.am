
printersdir=$(GNOME_PRINT_DATA_DIR)/printers
printers_in_files = \
	GENERIC.xml.in \
	PDF-WRITER.xml.in
printers_DATA=$(printers_in_files:.xml.in=.xml)

@INTLTOOL_XML_RULE@

EXTRA_DIST=$(printers_DATA)
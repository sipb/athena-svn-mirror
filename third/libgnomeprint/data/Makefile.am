SUBDIRS = models printers

globalsdir=$(GNOME_PRINT_DATA_DIR)
globals_in_files=globals.xml.in
globals_DATA=$(globals_in_files:.xml.in=.xml)

@INTLTOOL_XML_RULE@

EXTRA_DIST = $(globals_in_files) $(globals_DATA)

CLEANFILES = $(globals_DATA)

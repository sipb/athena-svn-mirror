
modelsdir=$(GNOME_PRINT_DATA_DIR)/models
models_in_files = \
     GNOME-GENERIC-PS.xml.in \
	GNOME-PDF-WRITER.xml.in
models_DATA=$(models_in_files:.xml.in=.xml)

@INTLTOOL_XML_RULE@

EXTRA_DIST = $(models_DATA)
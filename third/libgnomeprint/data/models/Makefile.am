
modelsdir=$(GNOME_PRINT_DATA_DIR)/models
models_in_files = \
     GNOME-GENERIC-PS.xml.in \
	GNOME-PDF-WRITER.xml.in
models_DATA=$(models_in_files:.xml.in=.xml)

if INSTALL_META_PRINTER
modelsprivatedir=$(GNOME_PRINT_DATA_DIR)/models
else
modelsprivatedir=$(GNOME_PRINT_DATA_DIR)/models/uninstalled
endif
modelsprivate_DATA=GNOME-META.xml

@INTLTOOL_XML_RULE@

EXTRA_DIST = $(models_DATA) $(modelsprivate_DATA)
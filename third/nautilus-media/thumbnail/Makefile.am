bin_PROGRAMS = gst-thumbnail

INCLUDES = $(GST_CFLAGS)
LDFLAGS = $(GST_LIBS)

schemadir = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = gst-thumbnail.schemas

install-data-local:
	@GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) \
                --makefile-install-rule $(srcdir)/$(schema_DATA) || \
                (echo ;\
                echo "*****************************************************"; \
                echo "Installation of schemas failed, install them manually"; \
                echo "*****************************************************";)
	@true

EXTRA_DIST = gst-thumbnail.schemas

CATALOG=@XML_CATALOG@
dtdsdir=$(datadir)/xml/scrollkeeper/dtds
dtds_HEADERS=scrollkeeper-omf.dtd scrollkeeper-cl.dtd

#
# Register the DTD(s) in the system XML Catalog
#

# ScrollKeeper OMF Variation DTD V1.0
install-data-hook:
	-/usr/bin/xmlcatalog --noout --add "public" \
		"-//OMF//DTD Scrollkeeper OMF Variant V1.0//EN" \
		"$(dtdsdir)/scrollkeeper-omf.dtd" $(CATALOG)

#
# Unregister the DTD(s) in the system XML Catalog
#

# ScrollKeeper OMF Variation DTD V1.0
uninstall-local:
	-/usr/bin/xmlcatalog --noout --del \
		"$(dtdsdir)/scrollkeeper-omf.dtd" $(CATALOG)


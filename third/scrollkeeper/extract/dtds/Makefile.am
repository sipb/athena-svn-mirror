CATALOG=$(sysconfdir)/xml/catalog
dtdsdir=$(datadir)/xml/scrollkeeper/dtds
dtds_HEADERS=scrollkeeper-omf.dtd scrollkeeper-cl.dtd

install-data-hook:
	#
	# Register the DTD(s) in the system XML Catalog
	#

	# ScrollKeeper OMF Variation DTD V1.0
	-/usr/bin/xmlcatalog --noout --add "public" \
		"-//OMF//DTD Scrollkeeper OMF Variant V1.0//EN" \
		"$(dtdsdir)/scrollkeeper-omf.dtd" $(CATALOG)

uninstall-local:
	#
	# Unregister the DTD(s) in the system XML Catalog
	#

	# ScrollKeeper OMF Variation DTD V1.0
	-/usr/bin/xmlcatalog --noout --del \
		"$(dtdsdir)/scrollkeeper-omf.dtd" $(CATALOG)


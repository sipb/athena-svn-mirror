SUBDIRS=java native
if BUILD_FREETTS_DRIVER

freetts_driverdir = \
	$(prefix)/bin

freetts_driver_SCRIPTS = \
	freetts-synthesis-driver

endif
serverinfodir = \
	$(libdir)/bonobo/servers

EXTRA_DIST = GNOME_Speech_SynthesisDriver_FreeTTS.server.in GNOME_Speech_SynthesisDriver_FreeTTS.server 

if BUILD_FREETTS_DRIVER
serverinfo_DATA = \
	GNOME_Speech_SynthesisDriver_FreeTTS.server
endif

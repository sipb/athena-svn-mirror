INCLUDES = \
	$(GSTMIXER_CFLAGS)				\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
	-DDATA_DIR=\""$(datadir)"\"			\
	-DPIX_DIR=\""$(pkgdatadir)/pixmaps"\"

bin_PROGRAMS = gnome-volume-control

gnome_volume_control_SOURCES = \
	mixer.c

gnome_volume_control_LDFLAGS = \
	$(GSTMIXER_LIBS)

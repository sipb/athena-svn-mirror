Bonobo_Sample_Controls.server.in: Bonobo_Sample_Controls.server.in.in
	sed -e "s|\@prefix\@|$(prefix)|" $< > $@

samplesdir = $(libdir)/bonobo-2.0/samples
samples_PROGRAMS = bonobo-sample-controls-2

INCLUDES =	\
        -DGNOMELOCALEDIR=\""$(datadir)/locale"\"        \
	-I$(top_srcdir) 				\
	$(BONOBOUI_CFLAGS)

bonobo_sample_controls_2_SOURCES =			\
	bonobo-sample-controls.c

bonobo_sample_controls_2_LDADD =			\
	$(top_builddir)/bonobo/libbonoboui-2.la		\
	$(BONOBOUI_LIBS)				\
	$(INTLLIBS)

serverdir = $(libdir)/bonobo/servers
server_in_files = Bonobo_Sample_Controls.server.in
server_DATA = $(server_in_files:.server.in=.server)

@INTLTOOL_SERVER_RULE@

EXTRA_DIST = Bonobo_Sample_Controls.server.in.in $(server_in_files) $(server_DATA)

CLEANFILES = $(server_in_files) $(server_DATA)

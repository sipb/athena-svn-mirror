SUBDIRS = gok po help omf-install docs samples

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gok-1.0.pc

desktopdir = $(datadir)/applications
desktop_in_files = gok.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

xmldir = $(pkgdatadir)

kbd_in_files = \
	main.kbd.in			\
	gok-controls.kbd.in		\
	launcher.kbd.in			\
	text-operations.kbd.in		\
	quit.kbd.in			\
	hide.kbd.in			\
	quit.kbd.in			\
	move-resize.kbd.in		\
	numberpad.kbd.in		\
	valuator.kbd.in			\
	mouse.kbd.in

xml_in_files = direct-selection.xml.in	\
	automatic-scanning.xml.in	\
	inverse-scanning.xml.in		\
	dwell-selection.xml.in		\
	directed.xml.in			\
	singlekey-automatic-scanning.xml.in		\
	singlekey-inverse-scanning.xml.in 

xam_files = $(xml_in_files:.xml.in=.xam)
@INTLTOOL_XAM_RULE@

kbd_files = $(kbd_in_files:.kbd.in=.kbd)
@INTLTOOL_KBD_RULE@

xml_DATA = $(xam_files) \
	$(kbd_files)

gladedir = $(pkgdatadir)/glade

glade_DATA = gok.glade2

EXTRA_DIST = \
	$(xml_in_files)			\
	$(kbd_in_files)			\
	$(pkgdata_DATA)                \
	$(desktop_in_files)		\
	$(desktop_DATA)			\
	$(glade_DATA)			\
	intltool-extract.in            \
	intltool-merge.in              \
	intltool-update.in             \
	gok-1.0.pc.in                  \
	gok-with-references.schemas.m4	\
	gok-with-references.schemas.in	\
	xmldocs.make

pkgdata_DATA = \
	qwerty.kbd                     \
	manage.kbd                     \
	dictionary.txt                 \
	gok.rc                         \
	goksound1.wav                  \
	goksound2.wav		\
	latched.png		\
	locked.png		\
	empty.png		\
	value-min.png	\
	value-max.png	\
	value-less.png	\
	value-more.png	\
	value-fast-less.png	\
	value-fast-more.png	\
	NW.png		\
	NE.png		\
	SE.png		\
	SW.png		\
	North.png		\
	East.png		\
	South.png		\
	West.png		\
	small-empty.png		\
	gok.png			\
	goklogo.png			\
	Keyboard.kbd

pixmapsdir = $(datadir)/pixmaps
pixmaps_DATA = gok.png

schemadir = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = gok.schemas

gok-with-references.schemas.in: $(srcdir)/gok-with-references.schemas.m4
	m4 $(srcdir)/gok-with-references.schemas.m4 > gok-with-references.schemas.in

@INTLTOOL_SCHEMAS_RULE@

# If we use FORCE instead of depending on Makefile then there is 
# a problem building rpms because the rpm build process installs 
# to a temporary location
gok.schemas: Makefile gok-with-references.schemas
	sed s/\$$pkgdatadir/`echo $(pkgdatadir) | sed s/\\\\//\\\\\\\\\\\\//g`/g gok-with-references.schemas > gok.schemas

Keyboard.kbd: FORCE qwerty.kbd
	cat $(srcdir)/qwerty.kbd | sed 's/qwerty/Keyboard/' > Keyboard.kbd

FORCE:

install-data-local:
if GCONF_SCHEMAS_INSTALL
	$(mkinstalldirs) $(DESTDIR)$(schemadir)
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) gconftool-2 --makefile-install-rule $(schema_DATA)
endif
	@for l in *; do 						\
		if [ -f $$l/main.kbd ]; then 				\
			$(mkinstalldirs) $(DESTDIR)$(datadir)/gok/$$l;	\
			for f in $$l/*.kbd; do				\
				echo copying $$f to $(DESTDIR)$(datadir)/gok/$$l;	\
				cp $$f $(DESTDIR)$(datadir)/gok/$$l; 	\
			done;						\
		fi;							\
	done

uninstall-local:
	rm -rf $(DESTDIR)$(datadir)/gok
	@for l in *; do 						\
		if [ -f $$l/main.kbd ]; then 				\
			rm -rf $$l;					\
		fi;							\
	done

webdoc: gokwebdoc.tar.gz

gokwebdoc.tar.gz: README INSTALL help/gok/C/gok.xml
	rm -rf gokwebdoc
	mkdir gokwebdoc
	cp README gokwebdoc
	cp INSTALL gokwebdoc
	xsltproc -o gokwebdoc/gok-user-manual.html \
	         /usr/share/xml/docbook/xsl-stylesheets/html/docbook.xsl \
	         help/gok/C/gok.xml
	tar czf gokwebdoc.tar.gz gokwebdoc
	rm -rf gokwebdoc

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc
DISTCLEANFILES = 		\
	intltool-extract	\
	intltool-merge		\
	intltool-update		\
	$(desktop_DATA)		\
	Keyboard.kbd		\
	gok-with-references.schemas.in	\
	gok-with-references.schemas	\
	$(schema_DATA)		\
	$(xam_files)		\
	$(kbd_files)

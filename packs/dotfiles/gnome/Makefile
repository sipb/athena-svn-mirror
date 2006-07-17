# $Id: Makefile,v 1.5 2006-07-17 23:08:52 rbasch Exp $

PIXMAPDIR=/usr/athena/share/pixmaps
GNOMEATHDIR=/usr/athena/share/gnome/athena
SCHEMASDIR=/etc/athena/gconf/schemas
AFSMENUS=/afs/athena.mit.edu/system/config/panel/menus
APPLICATIONS=/usr/athena/share/applications

all: athena.schemas

athena.schemas: athena.schemas.in
	rm -f $@
	perl gen-schemas.pl athena.schemas.in > $@

check:

install:
	mkdir -p ${DESTDIR}${PIXMAPDIR}
	mkdir -p ${DESTDIR}${GNOMEATHDIR}
	mkdir -p ${DESTDIR}${SCHEMASDIR}
	mkdir -p ${DESTDIR}${APPLICATIONS}
	install -c -m 0644 browser.desktop ${DESTDIR}${GNOMEATHDIR}
	install -c -m 0644 editor.desktop ${DESTDIR}${GNOMEATHDIR}
	install -c -m 0644 logout.desktop ${DESTDIR}${GNOMEATHDIR}
	install -c -m 0644 mail.desktop ${DESTDIR}${GNOMEATHDIR}
	install -c -m 0644 terminal.desktop ${DESTDIR}${GNOMEATHDIR}
	install -c -m 0755 browser-script ${DESTDIR}${GNOMEATHDIR}
	install -c -m 0755 editor-script ${DESTDIR}${GNOMEATHDIR}
	install -c -m 0755 logout-script ${DESTDIR}${GNOMEATHDIR}
	install -c -m 0755 mail-script ${DESTDIR}${GNOMEATHDIR}
	install -c -m 0755 terminal-script ${DESTDIR}${GNOMEATHDIR}
	install -c -m 0644 browser-icon ${DESTDIR}${GNOMEATHDIR}
	install -c -m 0644 editor-icon ${DESTDIR}${GNOMEATHDIR}
	install -c -m 0644 logout-icon ${DESTDIR}${GNOMEATHDIR}
	install -c -m 0644 mail-icon ${DESTDIR}${GNOMEATHDIR}
	install -c -m 0644 menu-icon ${DESTDIR}${GNOMEATHDIR}
	install -c -m 0644 terminal-icon ${DESTDIR}${GNOMEATHDIR}
	install -c -m 0644 info.png ${DESTDIR}${PIXMAPDIR}
	install -c -m 0644 logout-icon-menu.png ${DESTDIR}${PIXMAPDIR}
	install -c -m 0644 athena.schemas ${DESTDIR}${SCHEMASDIR}
	rm -f $(DESTDIR)$(GNOMEATHDIR)/menus
	ln -s $(AFSMENUS)-$(ATHENA_MAJOR_VERSION).$(ATHENA_MINOR_VERSION) \
		$(DESTDIR)$(GNOMEATHDIR)/menus
	rm -f $(DESTDIR)$(GNOMEATHDIR)/menus.tar
	ln -s $(AFSMENUS)-$(ATHENA_MAJOR_VERSION).$(ATHENA_MINOR_VERSION).tar \
		$(DESTDIR)$(GNOMEATHDIR)/menus.tar
	install -c -m 0644 athena-office.desktop ${DESTDIR}${APPLICATIONS}
	install -c -m 0644 athena-pdf.desktop ${DESTDIR}${APPLICATIONS}
	install -c -m 0644 athena-web.desktop ${DESTDIR}${APPLICATIONS}
	install -c -m 0644 realplay.desktop ${DESTDIR}${APPLICATIONS}
	install -c -m 0644 defaults.list ${DESTDIR}${APPLICATIONS}

clean:
	rm -f athena-panel-config.schemas

distclean: clean

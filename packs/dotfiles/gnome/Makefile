# $Id: Makefile,v 1.1 2003-03-15 16:18:21 ghudson Exp $

PIXMAPDIR=/usr/athena/share/pixmaps
GNOMEATHDIR=/usr/athena/share/gnome/athena
SCHEMASDIR=/etc/athena/gconf/schemas

all: athena-panel-config.schemas

athena-panel-config.schemas: athena-panel-config.schemas.in
	rm -f $@
	perl gen-schemas.pl athena-panel-config.schemas.in > $@

check:

install:
	mkdir -p ${DESTDIR}${PIXMAPDIR}
	mkdir -p ${DESTDIR}${GNOMEATHDIR}
	mkdir -p ${DESTDIR}${SCHEMASDIR}
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
	install -c -m 0644 athena-panel-config.schemas ${DESTDIR}${SCHEMASDIR}

clean:
	rm -f athena-panel-config.schemas

distclean: clean

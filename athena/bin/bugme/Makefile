DEFAULT:
	@echo "Valid targets: install"

install:
	mkdir -p $(DESTDIR)/usr/bin
	mkdir -p $(DESTDIR)/usr/share/bugme
	mkdir -p $(DESTDIR)/etc/X11/Xsession.d
	install -m 755 bugme $(DESTDIR)/usr/bin/bugme
	install -m 644 bugme.glade $(DESTDIR)/usr/share/bugme/
	install -m 644 bugme.xsession $(DESTDIR)/etc/X11/Xsession.d/98debathena-bugme

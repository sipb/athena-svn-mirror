# $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/conf/Makefile,v 1.5 1991-01-24 06:42:39 probe Exp $
FONTS75=courR18 ncenB12 ncenB14 ncenB18 ncenB24 ncenR12
FONTSM=	6x13 cursor 9x15
BITMAPS=menu16

all::

clean::

install::
	-mkdir $(DESTDIR)/etc/athena/login
	-mkdir $(DESTDIR)/etc/athena/login/fonts
	-mkdir $(DESTDIR)/usr/athena/lib/login
	install -c reactivate	$(DESTDIR)/etc/athena/reactivate
	install -c Xsession	$(DESTDIR)/etc/athena/login/Xsession
	install -c config	$(DESTDIR)/etc/athena/login/config
#	install -c Console	$(DESTDIR)/etc/athena/login/Console
#	install -c console.bits	$(DESTDIR)/etc/athena/login/console.bits
#	install -c gray		$(DESTDIR)/etc/athena/login/gray
	install -c machine	$(DESTDIR)/etc/athena/login/machine
	install -c library	$(DESTDIR)/usr/athena/lib/login/library
	install -c lucy		$(DESTDIR)/usr/athena/lib/login/lucy
	install -c mitvma	$(DESTDIR)/usr/athena/lib/login/mitvma
	install -c moira	$(DESTDIR)/usr/athena/lib/login/moira
	install -c register	$(DESTDIR)/usr/athena/lib/login/register
	install -c techinfo	$(DESTDIR)/usr/athena/lib/login/techinfo
	install -c xcluster	$(DESTDIR)/usr/athena/lib/login/xcluster
	install -c xinfo	$(DESTDIR)/usr/athena/lib/login/xinfo

	-for i in txt dir pag; do \
		install -c $(DESTDIR)/usr/lib/X11/rgb.$$i \
			$(DESTDIR)/etc/athena/login/rgb.$$i; done
	-for i in $(FONTSM); do \
		install -c $(DESTDIR)/usr/lib/X11/fonts/misc/$$i.snf \
			$(DESTDIR)/etc/athena/login/fonts/$$i.snf; done
	-for i in $(FONTS75); do \
		install -c $(DESTDIR)/usr/lib/X11/fonts/75dpi/$$i.snf \
			$(DESTDIR)/etc/athena/login/fonts/$$i.snf; done
	-for i in $(BITMAPS); do \
		install -c $(DESTDIR)/usr/lib/X11/bitmaps/$$i \
			$(DESTDIR)/etc/athena/login/$$i; done
	-/usr/bin/X11/mkfontdir $(DESTDIR)/etc/athena/login/fonts/
	install -c fonts.alias	$(DESTDIR)/etc/athena/login/fonts

depend::

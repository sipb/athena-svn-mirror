# $Header: /afs/dev.mit.edu/source/repository/athena/etc/xdm/conf/Makefile,v 1.3 1990-11-18 22:07:35 probe Exp $

all::

clean::

install::
	-mkdir $(DESTDIR)/etc/athena/login
	-mkdir $(DESTDIR)/usr/athena/lib/login
	install -c Console	$(DESTDIR)/etc/athena/login/Console
	install -c Xsession	$(DESTDIR)/etc/athena/login/Xsession
	install -c config	$(DESTDIR)/etc/athena/login/config
	install -c console.bits	$(DESTDIR)/etc/athena/login/console.bits
	install -c gray		$(DESTDIR)/etc/athena/login/gray
	install -c library	$(DESTDIR)/usr/athena/lib/login/library
	install -c lucy		$(DESTDIR)/usr/athena/lib/login/lucy
	install -c machine	$(DESTDIR)/etc/athena/login/machine
	install -c mitvma	$(DESTDIR)/usr/athena/lib/login/mitvma
	install -c moira	$(DESTDIR)/usr/athena/lib/login/moira
	install -c reactivate	$(DESTDIR)/etc/athena/reactivate
	install -c register	$(DESTDIR)/usr/athena/lib/login/register
	install -c xcluster	$(DESTDIR)/usr/athena/lib/login/xcluster
	install -c xinfo	$(DESTDIR)/usr/athena/lib/login/xinfo

depend::

Subject: Kerberos file renaming for short DOS names
Date: Tue, 19 Apr 1994 13:34:28 -0700
From: John Gilmore <gnu@cygnus.com>

[edited since sending, to bring it up to date with what actually happened.]

I'd like to come up with some file naming and configuration
conventions that will work in DOS, Unix, and Mac environments.  At
Cygnus, we are creating a single freely available K4 source tree that
works on many Unixes, Windows, and Mac.  It currently works on Unixes.
(To get a copy, send mail to info@cygnus.com requesting our Kerberos
release.  It's in a hidden FTP location due to export control.)

My plan, which we have used successfully in the GNU software, is to
make sure that all filenames are unique if you take the first 8 chars
and the first 3 after the dot.  No files have more than a single dot
in them.  We don't restrict file names to just 8.3 characters, since
doing so would impact readability for the (99.9%) of the developers
who are on Unix or Mac, where long file names are fine.

There's an additional complication that names longer than 14
characters present problems to old System V Unix and to `ar' on Unix.
DJ Delorie's excellent `doschk' program points out all these problems.
(ftp:/prep.ai.mit.edu/pub/gnu/doschk-1.1.tar.gz).

Here's my proposal for the include directory.  The largest bunch are config
files only referenced from osconf.h.  These I renamed freely to be more
indicative of what they're actually for.  Four files were unused.  The
only truly painful rename would be <mit-copyright.h>, which truncates fine
to <mit-copy.h> for DOS, but is 15 characters and blows System V.  I
opted to leave it that way anyway, since it is a completely symbolic file
and contains no code or definitions anyway.

  MIT K4 patch10	MIT K4 PC	PROPOSED NAME	(trunc to 8.3)	old Cyg
$1     $2		$3		$4		$5		$6

@ addr_comp.h		addrcomp.h	(gone)		(gone)
@ -			-		AddressXlation.h	addressx.h 
@ admin_server.h	admnserv.h	(gone)		(gone)
@ -			-		c-386bsd.h	c-386bsd.h	conf-386bsd.h
@ -			-		ChangeLog	changelo
@ conf-aixps2.h		-		c-386aix.h	c-386aix.h
@ conf-aixrios.h	-		c-rs6000.h	c-rs6000.h
@ -			-		c-alpha.h	c-alpha.h	conf-alpha.h
@ conf-aux.h		-		c-aux.h		c-aux.h		
@ conf-bsd386i.h	-		c-sun386i.h	c-sun386.h	
@ conf-bsdapollo.h	-		c-apollo.h	c-apollo.h	
@ conf-bsdibm032.h	-		c-rtpc.h	c-rtpc.h	
@ conf-bsdm68k.h	-		c-sun3.h	c-sun3.h	
@ conf-bsdsparc.h	-		c-sunos4.h	c-sunos4.h	
@ conf-bsdtahoe.h	-		c-tahoe.h	c-tahoe.h	
@ conf-bsdvax.h		-		c-vax.h		c-vax.h		
@ -			-		c-hp68k.h	c-hp68k.h	conf-hp68k.h
@ -			-		c-hpsnake.h	c-hpsnak.h	conf-hpsnake.h
@ -			-		c-i960vx.h	c-i960vx.h	conf-i960vx.h
@ conf-ibm370.h		-		c-ibm370.h	c-ibm370.h	
@ -			-		c-pyramid.h	c-pyrami.h
@ -			-		c-386linux.h	c-386lin.h	conf-linux386.h
@ -			-		c-next.h	c-next.h	conf-next.h
@ conf-pc.h		conf-pc.h	c-pc.h		c-pc.h
@ conf-pyr.h		conf-pyr.h	c-pyr.h		c-pyr.h
@ -			-		c-386sco.h	c-386sco.h	conf-sco386.h
@ conf-sgiirix.h	-		c-sgi.h		c-sgi.h		conf-sgi.h
@ -			-		c-sol2.h	c-sol2.h	conf-sol20sparc.h
@ -			-		c-svr4.h	c-svr4.h	conf-svr4.h
@ conf-ultmips2.h	-		c-ultmips.h	c-ultmip.h	
@ -			-		c-windows.h	c-window.h	conf-windows.h
@ conf.h		conf.h		conf.h		conf.h
@ des.h			des.h		des.h		des.h
@ des_conf.h		des_conf.h	des_conf.h	des_conf.h
@ highc.h		highc.h		(gone)		(gone)
@ kadm.h		kadm.h		kadm.h		kadm.h
@ kdc.h			kdc.h		kdc.h		kdc.h
@ kdc.h.sed		-		kdc-sed.h	kdc-sed.h
@ klog.h		klog.h		klog.h		klog.h
@ klog.h.sed		-		klog-sed.h	klog-sed.h
@ kparse.h		kparse.h	kparse.h	kparse.h
@ krb.h.sed		-		krb-sed.h	krb-sed.h
@ krb_conf.h		krb_conf.h	krb_conf.h	krb_conf.h
@ krb_db.h		krb_db.h	krb_db.h	krb_db.h
@ krb_db.h.sed		-		krb_db-sed.h	krb_db-s.h	
@ -			-		krb_driver.h	krb_driv.h
@ -			krberr.h	krberr.h	krberr.h
@ -			krblayer.h	krblayer.h	krblayer.h
@ -			-		krbports.h	krbports.h
@ -			-		kstream.h	kstream.h
@ -			leasherr.h	leasherr.h	leasherr.h
@ lsb_addr_comp.h	lsbaddcm.h	lsb_addr_cmp.h	lsb_addr.h
@ -			-		MacTCPCommonTypes.h mactcpco.h
@ -			-		Makefile.in	makefile.in
@ mit-copyright.h	mit_copy.h	mit-copyright.h	mit-copy.h
@ osconf.h		osconf.h	osconf.h	osconf.h
@ passwd_server.h	paswserv.h	(gone)		(gone)
@ principal.h		principa.h	principal.h	principa.h
@ prot.h		prot.h		prot.h		prot.h
@ -			timeval.h	-		-
@ -			-		UDPPB.h		udppb.h
@ -			winkrbid.h	winkrbid.h	winkrbid.h
@ -			-		winsock.def	winsock.def
@ -			-		winsock.h	winsock.h

I've supplied Unix shell scripts in ../lib/krb, which use the table
from `ren.msg' in the current directory, for moving:
 ren-pl10.sh	V4 pl10 filenames to proposed names	for converting old trees
 ren-pc.sh	V4 MIT PC names to proposed names	for converting old trees
 ren2long.sh	truncated names to proposed names	for moving DOS->unix
 ren2dos.sh	proposed names to truncated names	for unix->DOS names

There's also shell scripts to produce sed scripts for converting Makefiles
and documentation.  You use them like:
	./sed-pl10.sh >/tmp/sed
	sed -f /tmp/sed <Makefile >newMakefile
sed-pl10.sh	V4 pl10 filenames to proposed names	for converting old trees
sed-pc.sh	V4 MIT PC names to proposed names	for converting old trees

I'll also supply a DOS script for moving:
ren-pc.bat	V4 MIT PC names to proposed names	for converting old trees

And an MPW script for moving
ren-pl10.mpw	V4 pl10 filenames to proposed names	for converting old trees

	John Gilmore
	Cygnus Support

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

@ -			-		ChangeLog	changelo
@ -			-		Imakefile	imakefil
@ -			-		Makefile.in	makefile.in

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

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

I diffed the current MIT release of Kerberos for PC and Windows
against the V4 patchlevel 10 release, and identified some 30 files in
lib/krb that have been renamed between Unix and PC.  Comparing source
trees becomes much more painful when files are renamed.  If we don't
come to sync on the file names, it will be very hard to collaborate,
which would make more work for all of us.

My plan, which we have used successfully in the GNU software, is to
make sure that all filenames are unique if you take the first 8 chars
and the first 3 after the dot.  No files have more than a single dot
in them.  We don't restrict file names to just 8.3 characters, since
doing so would impact readability for the (99.9%) of the developers
who are on Unix or Mac, where long file names are fine.

There's an additional complication that names longer than 14
characters present problems to old System V Unix and to `ar' on Unix.
DJ Delorie's excellent `doschk' program points out all these problems.
(prep.ai.mit.edu:/pub/gnu/doschk-1.1.tar.gz).

Here's my proposal for the lib/krb directory.  In general, I tried to
regularize the names, turning get_ into g_, removing krb_, turning
reply into repl, turning ticket into tkt, keeping all file names
unique across the various libraries, and making a file name more like
the function name contained in it when there were conflicts.  Some
resulting truncated names are more readable than in the current MIT K4
PC, some are less readable -- but the overall advantage is that the
new names should be acceptable to Unix/Mac developers, while the old
ones weren't.

  MIT K4 patch10	MIT K4 PC	PROPOSED NAME	(trunc to 8.3)	old Cyg
$1     $2		$3		$4		$5		$6

@ -			-		ChangeLog	changelo
@ -			-		Makefile.in	makefile.in
@ -			-		kadm_cli_wrap.c	kadm_cli.c
@ -			-		kadm_stream.c	kadm_str.c
@ -			-		kadm_supp.c	kadm_sup.c
# Cleanup for simplified sed scripts that use this table
@sed s/tf_ad_print\./tf_util\./g

I've supplied Unix shell scripts in the distribution for moving:
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

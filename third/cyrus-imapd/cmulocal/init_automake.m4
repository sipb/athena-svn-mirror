dnl init_automake.m4--cmulocal automake setup macro
dnl Rob Earhart
dnl $Id: init_automake.m4,v 1.1.1.2 2004-02-23 22:53:59 rbasch Exp $

AC_DEFUN([CMU_INIT_AUTOMAKE], [
	AC_REQUIRE([AM_INIT_AUTOMAKE])
	ACLOCAL="$ACLOCAL -I \$(top_srcdir)/cmulocal"
	])

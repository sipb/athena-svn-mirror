dnl aclocal.m4 generated automatically by aclocal 1.4-p6

dnl Copyright (C) 1994, 1995-8, 1999, 2001 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

dnl Configure paths for librep
dnl $Id: aclocal.m4,v 1.1.1.2 2003-01-05 00:30:20 ghudson Exp $
dnl
dnl AM_PATH_REP([MINIMUM_VERSION])
dnl Test for librep, define REP_VERSION, REP_CFLAGS, REP_LIBS and REP_EXECDIR
dnl
AC_DEFUN(AM_PATH_REP,
[dnl
  AC_ARG_WITH(rep_prefix,[  --with-rep-prefix=PFX   Prefix where rep is installed (optional)],
	      [rep_prefix="$withval"], [rep_prefix=""])
  if test "x$rep_prefix" = "x"; then
    rep_config="rep-config"
  else
    rep_config="${rep_prefix}/bin/rep-config"
  fi
  min_rep_version=ifelse([$1], ,0.1,$1)
  AC_MSG_CHECKING(for rep - version >= $min_rep_version)
  rep_version=`$rep_config --version`
  if test $? -eq 0; then
    rep_major=`echo $rep_version \
	| sed -e 's/\([[0-9]]*\)\..*/\1/'`
    rep_minor=`echo $rep_version \
	| sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\).*/\2/'`
    min_rep_major=`echo $min_rep_version \
	| sed -e 's/\([[0-9]]*\)\..*/\1/'`
    min_rep_minor=`echo $min_rep_version \
	| sed -e 's/\([[0-9]]*\)\.\([[0-9]]*\).*/\2/'`
    if test '(' $rep_major -gt $min_rep_major ')' \
	-o '(' $rep_major -eq $min_rep_major \
	       -a $rep_minor -ge $min_rep_minor ')';
    then
      REP_VERSION="${rep_version}"
      REP_CFLAGS="`$rep_config --cflags`"
      REP_LIBS="`$rep_config --libs`"
      REP_EXECDIR="`$rep_config --execdir`"
      AC_SUBST(REP_VERSION)
      AC_SUBST(REP_CFLAGS)
      AC_SUBST(REP_LIBS)
      AC_SUBST(REP_EXECDIR)
      AC_MSG_RESULT([version ${rep_version}])
    else
      AC_MSG_ERROR([version ${rep_version}; require $min_rep_version])
    fi
  else
    AC_MSG_ERROR([can't find librep; is it installed?])
  fi

  dnl scan for GNU msgfmt
  AC_MSG_CHECKING(for GNU msgfmt)
  REP_MSGFMT=
  for p in `echo "$PATH" | sed -e 's/:/ /g'`; do
    if test -x $p/msgfmt; then
      if $p/msgfmt --version 2>&1 | grep GNU >/dev/null; then
	REP_MSGFMT=$p/msgfmt
      fi
    fi
  done
  if test x$REP_MSGFMT != x; then
    AC_MSG_RESULT($REP_MSGFMT)
  else
    AC_MSG_RESULT(unavailable, disabling i18n)
    REP_MSGFMT=true
  fi
  AC_SUBST(REP_MSGFMT)
])


dnl PKG_CHECK_MODULES(GSTUFF, gtk+-2.0 >= 1.3 glib = 1.3.4, action-if, action-not)
dnl defines GSTUFF_LIBS, GSTUFF_CFLAGS, see pkg-config man page
dnl also defines GSTUFF_PKG_ERRORS on error
AC_DEFUN(PKG_CHECK_MODULES, [
  succeeded=no

  if test -z "$PKG_CONFIG"; then
    AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
  fi

  if test "$PKG_CONFIG" = "no" ; then
     echo "*** The pkg-config script could not be found. Make sure it is"
     echo "*** in your path, or set the PKG_CONFIG environment variable"
     echo "*** to the full path to pkg-config."
     echo "*** Or see http://www.freedesktop.org/software/pkgconfig to get pkg-config."
  else
     PKG_CONFIG_MIN_VERSION=0.9.0
     if $PKG_CONFIG --atleast-pkgconfig-version $PKG_CONFIG_MIN_VERSION; then
        AC_MSG_CHECKING(for $2)

        if $PKG_CONFIG --exists "$2" ; then
            AC_MSG_RESULT(yes)
            succeeded=yes

            AC_MSG_CHECKING($1_CFLAGS)
            $1_CFLAGS=`$PKG_CONFIG --cflags "$2"`
            AC_MSG_RESULT($$1_CFLAGS)

            AC_MSG_CHECKING($1_LIBS)
            $1_LIBS=`$PKG_CONFIG --libs "$2"`
            AC_MSG_RESULT($$1_LIBS)
        else
            $1_CFLAGS=""
            $1_LIBS=""
            ## If we have a custom action on failure, don't print errors, but 
            ## do set a variable so people can do so.
            $1_PKG_ERRORS=`$PKG_CONFIG --errors-to-stdout --print-errors "$2"`
            ifelse([$4], ,echo $$1_PKG_ERRORS,)
        fi

        AC_SUBST($1_CFLAGS)
        AC_SUBST($1_LIBS)
     else
        echo "*** Your version of pkg-config is too old. You need version $PKG_CONFIG_MIN_VERSION or newer."
        echo "*** See http://www.freedesktop.org/software/pkgconfig"
     fi
  fi

  if test $succeeded = yes; then
     ifelse([$3], , :, [$3])
  else
     ifelse([$4], , AC_MSG_ERROR([Library requirements ($2) not met; consider adjusting the PKG_CONFIG_PATH environment variable if your libraries are in a nonstandard prefix so pkg-config can find them.]), [$4])
  fi
])




dnl aclocal.m4 generated automatically by aclocal 1.4

dnl Copyright (C) 1994, 1995-8, 1999 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY, to the extent permitted by law; without
dnl even the implied warranty of MERCHANTABILITY or FITNESS FOR A
dnl PARTICULAR PURPOSE.

# Like AC_CONFIG_HEADER, but automatically create stamp file.

AC_DEFUN(AM_CONFIG_HEADER,
[AC_PREREQ([2.12])
AC_CONFIG_HEADER([$1])
dnl When config.status generates a header, we must update the stamp-h file.
dnl This file resides in the same directory as the config header
dnl that is generated.  We must strip everything past the first ":",
dnl and everything past the last "/".
AC_OUTPUT_COMMANDS(changequote(<<,>>)dnl
ifelse(patsubst(<<$1>>, <<[^ ]>>, <<>>), <<>>,
<<test -z "<<$>>CONFIG_HEADERS" || echo timestamp > patsubst(<<$1>>, <<^\([^:]*/\)?.*>>, <<\1>>)stamp-h<<>>dnl>>,
<<am_indx=1
for am_file in <<$1>>; do
  case " <<$>>CONFIG_HEADERS " in
  *" <<$>>am_file "*<<)>>
    echo timestamp > `echo <<$>>am_file | sed -e 's%:.*%%' -e 's%[^/]*$%%'`stamp-h$am_indx
    ;;
  esac
  am_indx=`expr "<<$>>am_indx" + 1`
done<<>>dnl>>)
changequote([,]))])

# Define a conditional.

AC_DEFUN(AM_CONDITIONAL,
[AC_SUBST($1_TRUE)
AC_SUBST($1_FALSE)
if $2; then
  $1_TRUE=
  $1_FALSE='#'
else
  $1_TRUE='#'
  $1_FALSE=
fi])

# Do all the work for Automake.  This macro actually does too much --
# some checks are only needed if your package does certain things.
# But this isn't really a big deal.

# serial 1

dnl Usage:
dnl AM_INIT_AUTOMAKE(package,version, [no-define])

AC_DEFUN(AM_INIT_AUTOMAKE,
[AC_REQUIRE([AC_PROG_INSTALL])
PACKAGE=[$1]
AC_SUBST(PACKAGE)
VERSION=[$2]
AC_SUBST(VERSION)
dnl test to see if srcdir already configured
if test "`cd $srcdir && pwd`" != "`pwd`" && test -f $srcdir/config.status; then
  AC_MSG_ERROR([source directory already configured; run "make distclean" there first])
fi
ifelse([$3],,
AC_DEFINE_UNQUOTED(PACKAGE, "$PACKAGE", [Name of package])
AC_DEFINE_UNQUOTED(VERSION, "$VERSION", [Version number of package]))
AC_REQUIRE([AM_SANITY_CHECK])
AC_REQUIRE([AC_ARG_PROGRAM])
dnl FIXME This is truly gross.
missing_dir=`cd $ac_aux_dir && pwd`
AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)
AM_MISSING_PROG(MAKEINFO, makeinfo, $missing_dir)
AC_REQUIRE([AC_PROG_MAKE_SET])])

#
# Check to make sure that the build environment is sane.
#

AC_DEFUN(AM_SANITY_CHECK,
[AC_MSG_CHECKING([whether build environment is sane])
# Just in case
sleep 1
echo timestamp > conftestfile
# Do `set' in a subshell so we don't clobber the current shell's
# arguments.  Must try -L first in case configure is actually a
# symlink; some systems play weird games with the mod time of symlinks
# (eg FreeBSD returns the mod time of the symlink's containing
# directory).
if (
   set X `ls -Lt $srcdir/configure conftestfile 2> /dev/null`
   if test "[$]*" = "X"; then
      # -L didn't work.
      set X `ls -t $srcdir/configure conftestfile`
   fi
   if test "[$]*" != "X $srcdir/configure conftestfile" \
      && test "[$]*" != "X conftestfile $srcdir/configure"; then

      # If neither matched, then we have a broken ls.  This can happen
      # if, for instance, CONFIG_SHELL is bash and it inherits a
      # broken ls alias from the environment.  This has actually
      # happened.  Such a system could not be considered "sane".
      AC_MSG_ERROR([ls -t appears to fail.  Make sure there is not a broken
alias in your environment])
   fi

   test "[$]2" = conftestfile
   )
then
   # Ok.
   :
else
   AC_MSG_ERROR([newly created file is older than distributed files!
Check your system clock])
fi
rm -f conftest*
AC_MSG_RESULT(yes)])

dnl AM_MISSING_PROG(NAME, PROGRAM, DIRECTORY)
dnl The program must properly implement --version.
AC_DEFUN(AM_MISSING_PROG,
[AC_MSG_CHECKING(for working $2)
# Run test in a subshell; some versions of sh will print an error if
# an executable is not found, even if stderr is redirected.
# Redirect stdin to placate older versions of autoconf.  Sigh.
if ($2 --version) < /dev/null > /dev/null 2>&1; then
   $1=$2
   AC_MSG_RESULT(found)
else
   $1="$3/missing $2"
   AC_MSG_RESULT(missing)
fi
AC_SUBST($1)])

dnl init_automake.m4--cmulocal automake setup macro
dnl Rob Earhart
dnl $Id: aclocal.m4,v 1.1.1.2 2003-02-12 22:34:32 ghudson Exp $

AC_DEFUN(CMU_INIT_AUTOMAKE, [
	AC_REQUIRE([AM_INIT_AUTOMAKE])
	ACLOCAL="$ACLOCAL -I \$(top_srcdir)/cmulocal"
	])

dnl
dnl $Id: aclocal.m4,v 1.1.1.2 2003-02-12 22:34:32 ghudson Exp $
dnl

dnl
dnl Test for __attribute__
dnl

AC_DEFUN(CMU_C___ATTRIBUTE__, [
AC_MSG_CHECKING(for __attribute__)
AC_CACHE_VAL(ac_cv___attribute__, [
AC_TRY_COMPILE([
#include <stdlib.h>
],
[
static void foo(void) __attribute__ ((noreturn));

static void
foo(void)
{
  exit(1);
}
],
ac_cv___attribute__=yes,
ac_cv___attribute__=no)])
if test "$ac_cv___attribute__" = "yes"; then
  AC_DEFINE(HAVE___ATTRIBUTE__, 1, [define if your compiler has __attribute__])
fi
AC_MSG_RESULT($ac_cv___attribute__)
])


dnl
dnl Additional macros for configure.in packaged up for easier theft.
dnl $Id: aclocal.m4,v 1.1.1.2 2003-02-12 22:34:32 ghudson Exp $
dnl tjs@andrew.cmu.edu 6-may-1998
dnl

dnl It would be good if ANDREW_ADD_LIBPATH could detect if something was
dnl already there and not redundantly add it if it is.

dnl add -L(arg), and possibly (runpath switch)(arg), to LDFLAGS
dnl (so the runpath for shared libraries is set).
AC_DEFUN(CMU_ADD_LIBPATH, [
  # this is CMU ADD LIBPATH
  if test "$andrew_runpath_switch" = "none" ; then
	LDFLAGS="-L$1 ${LDFLAGS}"
  else
	LDFLAGS="-L$1 $andrew_runpath_switch$1 ${LDFLAGS}"
  fi
])

dnl add -L(1st arg), and possibly (runpath switch)(1st arg), to (2nd arg)
dnl (so the runpath for shared libraries is set).
AC_DEFUN(CMU_ADD_LIBPATH_TO, [
  # this is CMU ADD LIBPATH TO
  if test "$andrew_runpath_switch" = "none" ; then
	$2="-L$1 ${$2}"
  else
	$2="-L$1 ${$2} $andrew_runpath_switch$1"
  fi
])

dnl runpath initialization
AC_DEFUN(CMU_GUESS_RUNPATH_SWITCH, [
   # CMU GUESS RUNPATH SWITCH
  AC_CACHE_CHECK(for runpath switch, andrew_runpath_switch, [
    # first, try -R
    SAVE_LDFLAGS="${LDFLAGS}"
    LDFLAGS="-R /usr/lib"
    AC_TRY_LINK([],[],[andrew_runpath_switch="-R"], [
  	LDFLAGS="-Wl,-rpath,/usr/lib"
    AC_TRY_LINK([],[],[andrew_runpath_switch="-Wl,-rpath,"],
    [andrew_runpath_switch="none"])
    ])
  LDFLAGS="${SAVE_LDFLAGS}"
  ])])

dnl bsd_sockets.m4--which socket libraries do we need? 
dnl Derrick Brashear
dnl from Zephyr
dnl $Id: aclocal.m4,v 1.1.1.2 2003-02-12 22:34:32 ghudson Exp $

dnl Hacked on by Rob Earhart to not just toss stuff in LIBS
dnl It now puts everything required for sockets into LIB_SOCKET

AC_DEFUN(CMU_SOCKETS, [
	save_LIBS="$LIBS"
	LIB_SOCKET=""
	AC_CHECK_FUNC(connect, :,
		AC_CHECK_LIB(nsl, gethostbyname,
			     LIB_SOCKET="-lnsl $LIB_SOCKET")
		AC_CHECK_LIB(socket, connect,
			     LIB_SOCKET="-lsocket $LIB_SOCKET")
	)
	LIBS="$LIB_SOCKET $save_LIBS"
	AC_CHECK_FUNC(res_search, :,
                AC_CHECK_LIB(resolv, res_search,
                              LIB_SOCKET="-lresolv $LIB_SOCKET") 
        )
	LIBS="$LIB_SOCKET $save_LIBS"
	AC_CHECK_FUNCS(dn_expand dns_lookup)
	LIBS="$save_LIBS"
	AC_SUBST(LIB_SOCKET)
	])

dnl
dnl macros for configure.in to detect openssl
dnl $Id: aclocal.m4,v 1.1.1.2 2003-02-12 22:34:32 ghudson Exp $
dnl

AC_DEFUN(CMU_HAVE_OPENSSL, [
AC_ARG_WITH(with-openssl,[  --with-openssl=PATH     use OpenSSL from PATH],
	with_openssl="${withval}", with_openssl="yes")

case "$with_openssl" in
	no) with_openssl="no";;
	""|yes) 
	  dnl if openssl has been compiled with the rsaref2 libraries,
	  dnl we need to include the rsaref libraries in the crypto check
                LIB_RSAREF=""
	        AC_CHECK_LIB(rsaref, RSAPublicEncrypt,
		       LIB_RSAREF="-lRSAglue -lrsaref"; cmu_have_rsaref=yes,
		       cmu_have_rsaref=no)

		AC_CHECK_HEADER(openssl/evp.h, [
			AC_CHECK_LIB(crypto, EVP_DigestInit,
					with_openssl="yes",
					with_openssl="no", $LIB_RSAREF)],
			with_openssl=no)
		;;
	*)
		if test -d $with_openssl; then
		  CPPFLAGS="${CPPFLAGS} -I${with_openssl}/include"
		  LDFLAGS="${LDFLAGS} -L${with_openssl}/lib"
		else
		  with_openssl="no"
		fi
		;;
esac
	if test "$with_openssl" != "no"; then
		AC_DEFINE(HAVE_OPENSSL)
	fi
])
dnl checking for kerberos 4 libraries (and DES)

AC_DEFUN(SASL_DES_CHK, [
AC_ARG_WITH(des, [  --with-des=DIR          with DES (look in DIR) [yes] ],
	with_des=$withval,
	with_des=yes)

LIB_DES=""
if test "$with_des" != no; then
  if test -d $with_des; then
    CPPFLAGS="$CPPFLAGS -I${with_des}/include"
    LDFLAGS="$LDFLAGS -L${with_des}/lib"
  fi

  dnl check for openssl installing -lcrypto, then make vanilla check
  AC_CHECK_LIB(crypto, des_cbc_encrypt, [
      AC_CHECK_HEADER(openssl/des.h, [AC_DEFINE(WITH_SSL_DES)
                                     LIB_DES="-lcrypto";
                                     with_des=yes],
                     with_des=no)],
      with_des=no, $LIB_RSAREF)

  dnl same test again, different symbol name
  if test "$with_des" = no; then
    AC_CHECK_LIB(crypto, DES_cbc_encrypt, [
      AC_CHECK_HEADER(openssl/des.h, [AC_DEFINE(WITH_SSL_DES)
                                     LIB_DES="-lcrypto";
                                     with_des=yes],
                     with_des=no)],
      with_des=no, $LIB_RSAREF)
  fi

  if test "$with_des" = no; then
    AC_CHECK_LIB(des, des_cbc_encrypt, [LIB_DES="-ldes";
                                        with_des=yes], with_des=no)
  fi

  if test "$with_des" = no; then
     AC_CHECK_LIB(des425, des_cbc_encrypt, [LIB_DES="-ldes425";
                                       with_des=yes], with_des=no)
  fi

  if test "$with_des" = no; then
     AC_CHECK_LIB(des524, des_cbc_encrypt, [LIB_DES="-ldes524";
                                       with_des=yes], with_des=no)
  fi

  if test "$with_des" = no; then
    dnl if openssl is around, we might be able to use that for des

    dnl if openssl has been compiled with the rsaref2 libraries,
    dnl we need to include the rsaref libraries in the crypto check
    LIB_RSAREF=""
    AC_CHECK_LIB(rsaref, RSAPublicEncrypt,
                 LIB_RSAREF="-lRSAglue -lrsaref"; cmu_have_rsaref=yes,
                 cmu_have_rsaref=no)

    AC_CHECK_LIB(crypto, des_cbc_encrypt, [
	AC_CHECK_HEADER(openssl/des.h, [AC_DEFINE(WITH_SSL_DES)
					LIB_DES="-lcrypto";
					with_des=yes],
			with_des=no)], 
        with_des=no, $LIB_RSAREF)
  fi
fi

if test "$with_des" != no; then
  AC_DEFINE(WITH_DES)
fi

AC_SUBST(LIB_DES)
])

AC_DEFUN(SASL_KERBEROS_V4_CHK, [
  AC_REQUIRE([SASL_DES_CHK])

  AC_ARG_ENABLE(krb4, [  --enable-krb4           enable KERBEROS_V4 authentication [yes] ],
    krb4=$enableval,
    krb4=yes)

  if test "$krb4" != no; then
    dnl In order to compile kerberos4, we need libkrb and libdes.
    dnl (We've already gotten libdes from SASL_DES_CHK)
    dnl we might need -lresolv for kerberos
    AC_CHECK_LIB(resolv,res_search)

    dnl if we were ambitious, we would look more aggressively for the
    dnl krb4 install
    if test -d ${krb4}; then
       AC_CACHE_CHECK(for Kerberos includes, cyrus_krbinclude, [
         for krbhloc in include/kerberosIV include/kerberos include
         do
           if test -f ${krb4}/${krbhloc}/krb.h ; then
             cyrus_krbinclude=${krb4}/${krbhloc}
             break
           fi
         done
         ])

       if test -n "${cyrus_krbinclude}"; then
         CPPFLAGS="$CPPFLAGS -I${cyrus_krbinclude}"
       fi
       LDFLAGS="$LDFLAGS -L$krb4/lib"
    fi

    if test "$with_des" != no; then
      AC_CHECK_HEADER(krb.h, [
        AC_CHECK_LIB(com_err, com_err, [
	  AC_CHECK_LIB(krb, krb_mk_priv,
                     [COM_ERR="-lcom_err"; SASL_KRB_LIB="-lkrb"; krb4lib="yes"],
                     krb4lib=no, $LIB_DES -lcom_err)], [
    	  AC_CHECK_LIB(krb, krb_mk_priv,
                     [COM_ERR=""; SASL_KRB_LIB="-lkrb"; krb4lib="yes"],
                     krb4lib=no, $LIB_DES)])], krb4="no")

    if test "$krb4" = "yes" -a "$krb4lib" = "no"; then
	AC_CHECK_LIB(krb4, krb_mk_priv,
                     [COM_ERR=""; SASL_KRB_LIB="-lkrb4"; krb4=yes],
                     krb4=no, $LIB_DES)
        if test "$krb4" = no; then
          AC_WARN(No Kerberos V4 found)
        fi
      fi
    else
      AC_WARN(No DES library found for Kerberos V4 support)
      krb4=no
    fi
  fi

  if test "$krb4" != no; then
    cmu_save_LIBS="$LIBS"
    LIBS="$LIBS $SASL_KRB_LIB"
    AC_CHECK_FUNCS(krb_get_err_text)
    LIBS="$cmu_save_LIBS"
  fi

  AC_MSG_CHECKING(KERBEROS_V4)
  if test "$krb4" != no; then
    AC_MSG_RESULT(enabled)
    SASL_MECHS="$SASL_MECHS libkerberos4.la"
    SASL_STATIC_OBJS="$SASL_STATIC_OBJS ../plugins/kerberos4.o"
    AC_DEFINE(STATIC_KERBEROS4)
    AC_DEFINE(HAVE_KRB)
    SASL_KRB_LIB="$SASL_KRB_LIB $LIB_DES $COM_ERR"
  else
    AC_MSG_RESULT(disabled)
  fi
  AC_SUBST(SASL_KRB_LIB)
])


dnl sasl2.m4--sasl2 libraries and includes
dnl Rob Siemborski
dnl $Id: aclocal.m4,v 1.1.1.2 2003-02-12 22:34:32 ghudson Exp $

AC_DEFUN(SASL_GSSAPI_CHK,[
 AC_ARG_ENABLE(gssapi, [  --enable-gssapi=<DIR>   enable GSSAPI authentication [yes] ],
    gssapi=$enableval,
    gssapi=yes)

 if test "$gssapi" != no; then
    if test -d ${gssapi}; then
       CPPFLAGS="$CPPFLAGS -I$gssapi/include"
       LDFLAGS="$LDFLAGS -L$gssapi/lib"
    fi
    AC_CHECK_HEADER(gssapi.h, AC_DEFINE(HAVE_GSSAPI_H,,[Define if you have the gssapi.h header file]), [
      AC_CHECK_HEADER(gssapi/gssapi.h,, AC_WARN(Disabling GSSAPI); gssapi=no)])
 fi

 if test "$gssapi" != no; then
  dnl We need to find out which gssapi implementation we are
  dnl using. Supported alternatives are: MIT Kerberos 5 and
  dnl Heimdal Kerberos 5 (http://www.pdc.kth.se/heimdal)
  dnl
  dnl The choice is reflected in GSSAPIBASE_LIBS
  dnl we might need libdb
  AC_CHECK_LIB(db, db_open)

  gss_impl="mit";
  AC_CHECK_LIB(resolv,res_search)
  if test -d ${gssapi}; then 
     CPPFLAGS="$CPPFLAGS -I$gssapi/include"
     LDFLAGS="$LDFLAGS -L$gssapi/lib"
  fi

  if test -d ${gssapi}; then
     gssapi_dir="${gssapi}/lib"
     GSSAPIBASE_LIBS="-L$gssapi_dir"
     GSSAPIBASE_STATIC_LIBS="-L$gssapi_dir"
  else
     dnl FIXME: This is only used for building cyrus, and then only as
     dnl a real hack.  it needs to be fixed.
     gssapi_dir="/usr/local/lib"
  fi

  # Check a full link against the heimdal libraries.  If this fails, assume
  # MIT.
  AC_CHECK_LIB(gssapi,gss_unwrap,gss_impl="heimdal",,$GSSAPIBASE_LIBS -lgssapi -lkrb5 -lasn1 -lroken ${LIB_CRYPT} -lcom_err)

  if test "$gss_impl" = "mit"; then
     GSSAPIBASE_LIBS="$GSSAPIBASE_LIBS -lgssapi_krb5 -lkrb5 -lk5crypto -lcom_err"
     GSSAPIBASE_STATIC_LIBS="$GSSAPIBASE_LIBS $gssapi_dir/libgssapi_krb5.a $gssapi_dir/libkrb5.a $gssapi_dir/libk5crypto.a $gssapi_dir/libcom_err.a"
  elif test "$gss_impl" = "heimdal"; then
     GSSAPIBASE_LIBS="$GSSAPIBASE_LIBS -lgssapi -lkrb5 -lasn1 -lroken ${LIB_CRYPT} -lcom_err"
     GSSAPIBASE_STATIC_LIBS="$GSSAPIBASE_STATIC_LIBS $gssapi_dir/libgssapi.a $gssapi_dir/libkrb5.a $gssapi_dir/libasn1.a $gssapi_dir/libroken.a $gssapi_dir/libcom_err.a ${LIB_CRYPT}"
  else
     gssapi="no"
     AC_WARN(Disabling GSSAPI)
  fi
 fi

 if test "$ac_cv_header_gssapi_h" = "yes"; then
  AC_EGREP_HEADER(GSS_C_NT_HOSTBASED_SERVICE, gssapi.h,
    AC_DEFINE(HAVE_GSS_C_NT_HOSTBASED_SERVICE,,[Define if your GSSAPI implimentation defines GSS_C_NT_HOSTBASED_SERVICE]))
 elif test "$ac_cv_header_gssapi_gssapi_h"; then
  AC_EGREP_HEADER(GSS_C_NT_HOSTBASED_SERVICE, gssapi/gssapi.h,
    AC_DEFINE(HAVE_GSS_C_NT_HOSTBASED_SERVICE,,[Define if your GSSAPI implimentation defines GSS_C_NT_HOSTBASED_SERVICE]))
 fi

 GSSAPI_LIBS=""
 AC_MSG_CHECKING(GSSAPI)
 if test "$gssapi" != no; then
  AC_MSG_RESULT(with implementation ${gss_impl})
  AC_CHECK_LIB(resolv,res_search,GSSAPIBASE_LIBS="$GSSAPIBASE_LIBS -lresolv")
  SASL_MECHS="$SASL_MECHS libgssapiv2.la"
  SASL_STATIC_OBJS="$SASL_STATIC_OBJS ../plugins/gssapi.o"

  cmu_save_LIBS="$LIBS"
  LIBS="$LIBS $GSSAPIBASE_LIBS"
  AC_CHECK_FUNCS(gsskrb5_register_acceptor_identity)
  LIBS="$cmu_save_LIBS"
else
  AC_MSG_RESULT(disabled)
fi
AC_SUBST(GSSAPI_LIBS)
AC_SUBST(GSSAPIBASE_LIBS)
])

dnl What we want to do here is setup LIB_SASL with what one would
dnl generally want to have (e.g. if static is requested, make it that,
dnl otherwise make it dynamic.

dnl We also want to creat LIB_DYN_SASL and DYNSASLFLAGS.

dnl Also sets using_static_sasl to "no" "static" or "staticonly"

AC_DEFUN(CMU_SASL2, [
AC_ARG_WITH(sasl,
            [  --with-sasl=DIR         Compile with libsasl2 in <DIR>],
	    with_sasl="$withval",
            with_sasl="yes")

AC_ARG_WITH(staticsasl,
	    [  --with-staticsasl=DIR   Compile with staticly linked libsasl2 in <DIR>],
	    with_staticsasl="$withval";
	    if test $with_staticsasl != "no"; then
		using_static_sasl="static"
	    fi,
	    with_staticsasl="no"; using_static_sasl="no")

	SASLFLAGS=""
	LIB_SASL=""

	cmu_saved_CPPFLAGS=$CPPFLAGS
	cmu_saved_LDFLAGS=$LDFLAGS
	cmu_saved_LIBS=$LIBS

	if test ${with_staticsasl} != "no"; then
	  if test -d ${with_staticsasl}; then
	    ac_cv_sasl_where_lib=${with_staticsasl}/lib
	    ac_cv_sasl_where_inc=${with_staticsasl}/include

	    SASLFLAGS="-I$ac_cv_sasl_where_inc"
	    LIB_SASL="-L$ac_cv_sasl_where_lib"
	    CPPFLAGS="${cmu_saved_CPPFLAGS} -I${ac_cv_sasl_where_inc}"
	    LDFLAGS="${cmu_saved_LDFLAGS} -L${ac_cv_sasl_where_lib}"
	  else
	    with_staticsasl="/usr"
	  fi

	  AC_CHECK_HEADER(sasl/sasl.h, [
	    AC_CHECK_HEADER(sasl/saslutil.h, [
	     if test -r ${with_staticsasl}/lib/libsasl2.a; then
		ac_cv_found_sasl=yes
		AC_MSG_CHECKING(for static libsasl)
		LIB_SASL="$LIB_SASL ${with_staticsasl}/lib/libsasl2.a"
	     else
	        AC_MSG_CHECKING(for static libsasl)
		AC_ERROR([Could not find ${with_staticsasl}/lib/libsasl2.a])
	     fi
	    ])])

	  AC_MSG_RESULT(found)

	  SASL_GSSAPI_CHK

	  LIB_SASL="$LIB_SASL $GSSAPIBASE_STATIC_LIBS"
	fi

	if test -d ${with_sasl}; then
            ac_cv_sasl_where_lib=${with_sasl}/lib
            ac_cv_sasl_where_inc=${with_sasl}/include

	    DYNSASLFLAGS="-I$ac_cv_sasl_where_inc"
	    if test "$ac_cv_sasl_where_lib" != ""; then
		CMU_ADD_LIBPATH_TO($ac_cv_sasl_where_lib, LIB_DYN_SASL)
	    fi
	    LIB_DYN_SASL="$LIB_DYN_SASL -lsasl2"
	    CPPFLAGS="${cmu_saved_CPPFLAGS} -I${ac_cv_sasl_where_inc}"
	    LDFLAGS="${cmu_saved_LDFLAGS} -L${ac_cv_sasl_where_lib}"
	fi

	dnl be sure to check for a SASLv2 specific function
	AC_CHECK_HEADER(sasl/sasl.h, [
	    AC_CHECK_HEADER(sasl/saslutil.h, [
	      AC_CHECK_LIB(sasl2, prop_get, 
                           ac_cv_found_sasl=yes,
		           ac_cv_found_sasl=no)],
	                   ac_cv_found_sasl=no)], ac_cv_found_sasl=no)

	if test "$ac_cv_found_sasl" = "yes"; then
	    if test "$ac_cv_sasl_where_lib" != ""; then
	        CMU_ADD_LIBPATH_TO($ac_cv_sasl_where_lib, DYNLIB_SASL)
	    fi
	    DYNLIB_SASL="$DYNLIB_SASL -lsasl2"
	    if test "$using_static_sasl" != "static"; then
		LIB_SASL=$DYNLIB_SASL
		SASLFLAGS=$DYNSASLFLAGS
	    fi
	else
	    DYNLIB_SASL=""
	    DYNSASLFLAGS=""
	    using_static_sasl="staticonly"
	fi

	LIBS="$cmu_saved_LIBS"
	LDFLAGS="$cmu_saved_LDFLAGS"
	CPPFLAGS="$cmu_saved_CPPFLAGS"

	AC_SUBST(LIB_DYN_SASL)
	AC_SUBST(DYNSASLFLAGS)
	AC_SUBST(LIB_SASL)
	AC_SUBST(SASLFLAGS)
	])

AC_DEFUN(CMU_SASL2_REQUIRED,
[AC_REQUIRE([CMU_SASL2])
if test "$ac_cv_found_sasl" != "yes"; then
        AC_ERROR([Cannot continue without libsasl2.
Get it from ftp://ftp.andrew.cmu.edu/pub/cyrus-mail/.])
fi])

AC_DEFUN(CMU_SASL2_CHECKAPOP_REQUIRED, [
	AC_REQUIRE([CMU_SASL2_REQUIRED])

	cmu_saved_LDFLAGS=$LDFLAGS

	LDFLAGS="$LDFLAGS $LIB_SASL"

	AC_CHECK_LIB(sasl2, sasl_checkapop, AC_DEFINE(HAVE_APOP),
		AC_MSG_ERROR([libsasl2 without working sasl_checkapop.  Cannot continue.]))

	LDFLAGS=$cmu_saved_LDFLAGS
])

dnl Check for PLAIN (and therefore crypt)

AC_DEFUN(SASL_CRYPT_CHK,[
 AC_CHECK_FUNC(crypt, cmu_have_crypt=yes, [
  AC_CHECK_LIB(crypt, crypt,
	       LIB_CRYPT="-lcrypt"; cmu_have_crypt=yes,
	       cmu_have_crypt=no)])
 AC_SUBST(LIB_CRYPT)
])

AC_DEFUN(SASL_PLAIN_CHK,[
AC_REQUIRE([SASL_CRYPT_CHK])

dnl PLAIN
 AC_ARG_ENABLE(plain, [  --enable-plain          enable PLAIN authentication [yes] ],
  plain=$enableval,
  plain=yes)

 PLAIN_LIBS=""
 if test "$plain" != no; then
  dnl In order to compile plain, we need crypt.
  if test "$cmu_have_crypt" = yes; then
    PLAIN_LIBS=$LIB_CRYPT
  fi
 fi
 AC_SUBST(PLAIN_LIBS)

 AC_MSG_CHECKING(PLAIN)
 if test "$plain" != no; then
  AC_MSG_RESULT(enabled)
  SASL_MECHS="$SASL_MECHS libplain.la"
  if test "$enable_static" = yes; then
    SASL_STATIC_OBJS="$SASL_STATIC_OBJS ../plugins/plain.o"
    AC_DEFINE(STATIC_PLAIN)
  fi
 else
  AC_MSG_RESULT(disabled)
 fi
])

dnl Functions to check what database to use for libsasldb

dnl Berkeley DB specific checks first..

dnl Figure out what database type we're using
AC_DEFUN(SASL_DB_CHECK, [
cmu_save_LIBS="$LIBS"
AC_ARG_WITH(dblib, [  --with-dblib=DBLIB      set the DB library to use [berkeley] ],
  dblib=$withval,
  dblib=auto_detect)

CYRUS_BERKELEY_DB_OPTS()

SASL_DB_LIB=""

case "$dblib" in
dnl this is unbelievably painful due to confusion over what db-3 should be
dnl named.  arg.
  berkeley)
	CYRUS_BERKELEY_DB_CHK()
	CPPFLAGS="${CPPFLAGS} ${BDB_INCADD}"
	SASL_DB_INC=$BDB_INCADD
	SASL_DB_LIB="${BDB_LIBADD}"
	;;
  gdbm)
	AC_ARG_WITH(with-gdbm,[  --with-gdbm=PATH        use gdbm from PATH],
                    with_gdbm="${withval}")

        case "$with_gdbm" in
           ""|yes)
               AC_CHECK_HEADER(gdbm.h, [
			AC_CHECK_LIB(gdbm, gdbm_open, SASL_DB_LIB="-lgdbm",
                                           dblib="no")],
			dblib="no")
               ;;
           *)
               if test -d $with_gdbm; then
                 CPPFLAGS="${CPPFLAGS} -I${with_gdbm}/include"
                 LDFLAGS="${LDFLAGS} -L${with_gdbm}/lib"
                 SASL_DB_LIB="-lgdbm" 
               else
                 with_gdbm="no"
               fi
       esac
	;;
  ndbm)
	dnl We want to attempt to use -lndbm if we can, just in case
	dnl there's some version of it installed and overriding libc
	AC_CHECK_HEADER(ndbm.h, [
			AC_CHECK_LIB(ndbm, dbm_open, SASL_DB_LIB="-lndbm", [
				AC_CHECK_FUNC(dbm_open,,dblib="no")])],
				dblib="no")
	;;
  auto_detect)
        dnl How about berkeley db?
	CYRUS_BERKELEY_DB_CHK()
	if test "$dblib" = no; then
	  dnl How about ndbm?
	  AC_CHECK_HEADER(ndbm.h, [
		AC_CHECK_LIB(ndbm, dbm_open,
			     dblib="ndbm"; SASL_DB_LIB="-lndbm",
		   	     dblib="weird")],
		   dblib="no")
	  if test "$dblib" = "weird"; then
	    dnl Is ndbm in the standard library?
            AC_CHECK_FUNC(dbm_open, dblib="ndbm", dblib="no")
	  fi

	  if test "$dblib" = no; then
            dnl Can we use gdbm?
   	    AC_CHECK_HEADER(gdbm.h, [
		AC_CHECK_LIB(gdbm, gdbm_open, dblib="gdbm";
					     SASL_DB_LIB="-lgdbm", dblib="no")],
  			     dblib="no")
	  fi
	else
	  dnl we took Berkeley
	  CPPFLAGS="${CPPFLAGS} ${BDB_INCADD}"
	  SASL_DB_INC=$BDB_INCADD
          SASL_DB_LIB="${BDB_LIBADD}"
	fi
	;;
  none)
	;;
  no)
	;;
  *)
	AC_MSG_WARN([Bad DB library implementation specified;])
	AC_ERROR([Use either \"berkeley\", \"gdbm\", \"ndbm\" or \"none\"])
	dblib=no
	;;
esac
LIBS="$cmu_save_LIBS"

AC_MSG_CHECKING(DB library to use)
AC_MSG_RESULT($dblib)

SASL_DB_BACKEND="db_${dblib}.lo"
SASL_DB_BACKEND_STATIC="../sasldb/db_${dblib}.o ../sasldb/allockey.o"
SASL_DB_UTILS="saslpasswd2 sasldblistusers2"
SASL_DB_MANS="saslpasswd2.8 sasldblistusers2.8"

case "$dblib" in
  gdbm) 
    SASL_MECHS="$SASL_MECHS libsasldb.la"
    AC_DEFINE(SASL_GDBM)
    ;;
  ndbm)
    SASL_MECHS="$SASL_MECHS libsasldb.la"
    AC_DEFINE(SASL_NDBM)
    ;;
  berkeley)
    SASL_MECHS="$SASL_MECHS libsasldb.la"
    AC_DEFINE(SASL_BERKELEYDB)
    ;;
  *)
    AC_MSG_WARN([Disabling SASL authentication database support])
    dnl note that we do not add libsasldb.la to SASL_MECHS, since it
    dnl will just fail to load anyway.
    SASL_DB_BACKEND="db_none.lo"
    SASL_DB_BACKEND_STATIC="../sasldb/db_none.o"
    SASL_DB_UTILS=""
    SASL_DB_MANS=""
    SASL_DB_LIB=""
    ;;
esac

if test "$enable_static" = yes; then
    if test "$dblib" != "none"; then
      SASL_STATIC_OBJS="$SASL_STATIC_OBJS ../plugins/sasldb.o $SASL_DB_BACKEND_STATIC"
      AC_DEFINE(STATIC_SASLDB)
    else
      SASL_STATIC_OBJS="$SASL_STATIC_OBJS $SASL_DB_BACKEND_STATIC"
    fi
fi
AC_SUBST(SASL_DB_UTILS)
AC_SUBST(SASL_DB_MANS)
AC_SUBST(SASL_DB_BACKEND)
AC_SUBST(SASL_DB_BACKEND_STATIC)
AC_SUBST(SASL_DB_INC)
AC_SUBST(SASL_DB_LIB)
])

dnl Figure out what database path we're using
AC_DEFUN(SASL_DB_PATH_CHECK, [
AC_ARG_WITH(dbpath, [  --with-dbpath=PATH      set the DB path to use [/etc/sasldb2] ],
  dbpath=$withval,
  dbpath=/etc/sasldb2)
AC_MSG_CHECKING(DB path to use)
AC_MSG_RESULT($dbpath)
AC_DEFINE_UNQUOTED(SASL_DB_PATH, "$dbpath")])

dnl $Id: aclocal.m4,v 1.1.1.2 2003-02-12 22:34:32 ghudson Exp $

AC_DEFUN(CMU_DB_INC_WHERE1, [
saved_CPPFLAGS=$CPPFLAGS
CPPFLAGS="$saved_CPPFLAGS -I$1"
AC_TRY_COMPILE([#include <db.h>],
[DB *db;
db_create(&db, NULL, 0);
db->open(db, "foo.db", NULL, DB_UNKNOWN, DB_RDONLY, 0644);],
ac_cv_found_db_inc=yes,
ac_cv_found_db_inc=no)
CPPFLAGS=$saved_CPPFLAGS
])

AC_DEFUN(CMU_DB_INC_WHERE, [
   for i in $1; do
      AC_MSG_CHECKING(for db headers in $i)
      CMU_DB_INC_WHERE1($i)
      CMU_TEST_INCPATH($i, db)
      if test "$ac_cv_found_db_inc" = "yes"; then
        ac_cv_db_where_inc=$i
        AC_MSG_RESULT(found)
        break
      else
        AC_MSG_RESULT(not found)
      fi
    done
])

#
# Test for lib files
#

AC_DEFUN(CMU_DB3_LIB_WHERE1, [
AC_REQUIRE([CMU_AFS])
AC_REQUIRE([CMU_KRB4])
saved_LIBS=$LIBS
  LIBS="$saved_LIBS -L$1 -ldb-3"
AC_TRY_LINK(,
[db_env_create();],
[ac_cv_found_db_3_lib=yes],
ac_cv_found_db_3_lib=no)
LIBS=$saved_LIBS
])
AC_DEFUN(CMU_DB4_LIB_WHERE1, [
AC_REQUIRE([CMU_AFS])
AC_REQUIRE([CMU_KRB4])
saved_LIBS=$LIBS
LIBS="$saved_LIBS -L$1 -ldb-4"
AC_TRY_LINK(,
[db_env_create();],
[ac_cv_found_db_4_lib=yes],
ac_cv_found_db_4_lib=no)
LIBS=$saved_LIBS
])

AC_DEFUN(CMU_DB_LIB_WHERE, [
   for i in $1; do
      AC_MSG_CHECKING(for db libraries in $i)
if test "$enable_db4" = "yes"; then
      CMU_DB4_LIB_WHERE1($i)
      CMU_TEST_LIBPATH($i, [db-4])
      ac_cv_found_db_lib=$ac_cv_found_db_4_lib
else
      CMU_DB3_LIB_WHERE1($i)
      CMU_TEST_LIBPATH($i, [db-3])
      ac_cv_found_db_lib=$ac_cv_found_db_3_lib
fi
      if test "$ac_cv_found_db_lib" = "yes" ; then
        ac_cv_db_where_lib=$i
        AC_MSG_RESULT(found)
        break
      else
        AC_MSG_RESULT(not found)
      fi
    done
])

AC_DEFUN(CMU_USE_DB, [
AC_ARG_WITH(db,
	[  --with-db=PREFIX      Compile with db support],
	[if test "X$with_db" = "X"; then
		with_db=yes
	fi])
AC_ARG_WITH(db-lib,
	[  --with-db-lib=dir     use db libraries in dir],
	[if test "$withval" = "yes" -o "$withval" = "no"; then
		AC_MSG_ERROR([No argument for --with-db-lib])
	fi])
AC_ARG_WITH(db-include,
	[  --with-db-include=dir use db headers in dir],
	[if test "$withval" = "yes" -o "$withval" = "no"; then
		AC_MSG_ERROR([No argument for --with-db-include])
	fi])
AC_ARG_ENABLE(db4,
	[  --enable-db4          use db 4.x libraries])
	
	if test "X$with_db" != "X"; then
	  if test "$with_db" != "yes"; then
	    ac_cv_db_where_lib=$with_db/lib
	    ac_cv_db_where_inc=$with_db/include
	  fi
	fi

	if test "X$with_db_lib" != "X"; then
	  ac_cv_db_where_lib=$with_db_lib
	fi
	if test "X$ac_cv_db_where_lib" = "X"; then
	  CMU_DB_LIB_WHERE(/usr/athena/lib /usr/lib /usr/local/lib)
	fi

	if test "X$with_db_include" != "X"; then
	  ac_cv_db_where_inc=$with_db_include
	fi
	if test "X$ac_cv_db_where_inc" = "X"; then
	  CMU_DB_INC_WHERE(/usr/athena/include /usr/local/include)
	fi

	AC_MSG_CHECKING(whether to include db)
	if test "X$ac_cv_db_where_lib" = "X" -o "X$ac_cv_db_where_inc" = "X"; then
	  ac_cv_found_db=no
	  AC_MSG_RESULT(no)
	else
	  ac_cv_found_db=yes
	  AC_MSG_RESULT(yes)
	  DB_INC_DIR=$ac_cv_db_where_inc
	  DB_LIB_DIR=$ac_cv_db_where_lib
	  DB_INC_FLAGS="-I${DB_INC_DIR}"
          if test "$enable_db4" = "yes"; then
	     DB_LIB_FLAGS="-L${DB_LIB_DIR} -ldb-4"
          else
	     DB_LIB_FLAGS="-L${DB_LIB_DIR} -ldb-3"
          fi
          dnl Do not force configure.in to put these in CFLAGS and LIBS unconditionally
          dnl Allow makefile substitutions....
          AC_SUBST(DB_INC_FLAGS)
          AC_SUBST(DB_LIB_FLAGS)
	  if test "X$RPATH" = "X"; then
		RPATH=""
	  fi
	  case "${host}" in
	    *-*-linux*)
	      if test "X$RPATH" = "X"; then
	        RPATH="-Wl,-rpath,${DB_LIB_DIR}"
	      else 
		RPATH="${RPATH}:${DB_LIB_DIR}"
	      fi
	      ;;
	    *-*-hpux*)
	      if test "X$RPATH" = "X"; then
	        RPATH="-Wl,+b${DB_LIB_DIR}"
	      else 
		RPATH="${RPATH}:${DB_LIB_DIR}"
	      fi
	      ;;
	    *-*-irix*)
	      if test "X$RPATH" = "X"; then
	        RPATH="-Wl,-rpath,${DB_LIB_DIR}"
	      else 
		RPATH="${RPATH}:${DB_LIB_DIR}"
	      fi
	      ;;
	    *-*-solaris2*)
	      if test "$ac_cv_prog_gcc" = yes; then
		if test "X$RPATH" = "X"; then
		  RPATH="-Wl,-R${DB_LIB_DIR}"
		else 
		  RPATH="${RPATH}:${DB_LIB_DIR}"
		fi
	      else
	        RPATH="${RPATH} -R${DB_LIB_DIR}"
	      fi
	      ;;
	  esac
	  AC_SUBST(RPATH)
	fi
	])



dnl ---- CUT HERE ---

dnl These are the Cyrus Berkeley DB macros.  In an ideal world these would be
dnl identical to the above.

dnl They are here so that they can be shared between Cyrus IMAPd
dnl and Cyrus SASL with relative ease.

dnl The big difference between this and the ones above is that we don't assume
dnl that we know the name of the library, and we try a lot of permutations
dnl instead.  We also assume that DB4 is acceptable.

dnl When we're done, there will be a BDB_LIBADD and a BDB_INCADD which should
dnl be used when necessary.  We should probably be smarter about our RPATH
dnl handling.

dnl Call these with BERKELEY_DB_CHK.

dnl We will also set $dblib to "berkeley" if we are successful, "no" otherwise.

dnl this is unbelievably painful due to confusion over what db-3 should be
dnl named and where the db-3 header file is located.  arg.
AC_DEFUN(CYRUS_BERKELEY_DB_CHK_LIB,
[
	BDB_SAVE_LIBS=$LIBS

	if test -d $with_bdb_lib; then
	    CMU_ADD_LIBPATH_TO($with_bdb_lib, LIBS)
	    CMU_ADD_LIBPATH_TO($with_bdb_lib, BDB_LIBADD)
	else
	    BDB_LIBADD=""
	fi

        for dbname in db-4.1 db4.1 db-4.0 db4.0 db-4 db4 db-3.3 db3.3 db-3.2 db3.2 db-3.1 db3.1 db-3 db3 db
          do
            AC_CHECK_LIB($dbname, db_create, BDB_LIBADD="$BDB_LIBADD -l$dbname";
              dblib="berkeley"; break, dblib="no")
          done
        if test "$dblib" = "no"; then
          AC_CHECK_LIB(db, db_open, BDB_LIBADD="$BDB_LIBADD -ldb";
            dblib="berkeley"; dbname=db,
            dblib="no")
        fi

	LIBS=$BDB_SAVE_LIBS
])

AC_DEFUN(CYRUS_BERKELEY_DB_OPTS,
[
AC_ARG_WITH(bdb-libdir,
	[  --with-bdb-libdir=DIR   Berkeley DB lib files are in DIR],
	with_bdb_lib=$withval,
	with_bdb_lib=none)
AC_ARG_WITH(bdb-incdir,
	[  --with-bdb-incdir=DIR   Berkeley DB include files are in DIR],
	with_bdb_inc=$withval,
	with_bdb_inc=none)
])

AC_DEFUN(CYRUS_BERKELEY_DB_CHK,
[
	AC_REQUIRE([CYRUS_BERKELEY_DB_OPTS])

	cmu_save_CPPFLAGS=$CPPFLAGS

	if test -d $with_bdb_inc; then
	    CPPFLAGS="$CPPFLAGS -I$with_bdb_inc"
	    BDB_INCADD="-I$with_bdb_inc"
	else
	    BDB_INCADD=""
	fi

	dnl Note that FreeBSD puts it in a wierd place
        dnl (but they should use with-bdb-incdir)
        AC_CHECK_HEADER(db.h,
                        CYRUS_BERKELEY_DB_CHK_LIB(),
                        dblib="no")

	CPPFLAGS=$cmu_save_CPPFLAGS
])

dnl $Id: aclocal.m4,v 1.1.1.2 2003-02-12 22:34:32 ghudson Exp $

AC_DEFUN(CMU_TEST_LIBPATH, [
changequote(<<, >>)
define(<<CMU_AC_CV_FOUND>>, translit(ac_cv_found_$2_lib, <<- *>>, <<__p>>))
changequote([, ])
if test "$CMU_AC_CV_FOUND" = "yes"; then
  if test \! -r "$1/lib$2.a" -a \! -r "$1/lib$2.so" -a \! -r "$1/lib$2.sl"; then
    CMU_AC_CV_FOUND=no
  fi
fi
])

AC_DEFUN(CMU_TEST_INCPATH, [
changequote(<<, >>)
define(<<CMU_AC_CV_FOUND>>, translit(ac_cv_found_$2_inc, [ *], [_p]))
changequote([, ])
if test "$CMU_AC_CV_FOUND" = "yes"; then
  if test \! -r "$1/$2.h"; then
    CMU_AC_CV_FOUND=no
  fi
fi
])

dnl CMU_CHECK_HEADER_NOCACHE(HEADER-FILE, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
AC_DEFUN(CMU_CHECK_HEADER_NOCACHE,
[dnl Do the transliteration at runtime so arg 1 can be a shell variable.
ac_safe=`echo "$1" | sed 'y%./+-%__p_%'`
AC_MSG_CHECKING([for $1])
AC_TRY_CPP([#include <$1>], eval "ac_cv_header_$ac_safe=yes",
  eval "ac_cv_header_$ac_safe=no")
if eval "test \"`echo '$ac_cv_header_'$ac_safe`\" = yes"; then
  AC_MSG_RESULT(yes)
  ifelse([$2], , :, [$2])
else
  AC_MSG_RESULT(no)
ifelse([$3], , , [$3
])dnl
fi
])

dnl afs.m4--AFS libraries, includes, and dependencies
dnl $Id: aclocal.m4,v 1.1.1.2 2003-02-12 22:34:32 ghudson Exp $
dnl Chaskiel Grundman
dnl based on kerberos_v4.m4
dnl Derrick Brashear
dnl from KTH krb and Arla

AC_DEFUN(CMU_AFS_INC_WHERE1, [
cmu_save_CPPFLAGS=$CPPFLAGS
CPPFLAGS="$cmu_save_CPPFLAGS -I$1"
AC_TRY_COMPILE([#include <afs/param.h>],
[#ifndef SYS_NAME
choke me
#endif
int foo;],
ac_cv_found_afs_inc=yes,
ac_cv_found_afs_inc=no)
CPPFLAGS=$cmu_save_CPPFLAGS
])

AC_DEFUN(CMU_AFS_LIB_WHERE1, [
save_LIBS="$LIBS"
save_LDFLAGS="$LDFLAGS"

LIBS="-lauth $1/afs/util.a $LIB_SOCKET $LIBS"
LDFLAGS="-L$1 -L$1/afs $LDFLAGS"
dnl suppress caching
AC_TRY_LINK([],[afsconf_Open();], ac_cv_found_afs_lib=yes, ac_cv_found_afs_lib=no)
LIBS="$save_LIBS"
LDFLAGS="$save_LDFLAGS"
])

AC_DEFUN(CMU_AFS_WHERE, [
   for i in $1; do
      AC_MSG_CHECKING(for AFS in $i)
      CMU_AFS_INC_WHERE1("$i/include")
      ac_cv_found_lwp_inc=$ac_cv_found_afs_inc
      CMU_TEST_INCPATH($i/include, lwp) 
      ac_cv_found_afs_inc=$ac_cv_found_lwp_inc
      if test "$ac_cv_found_afs_inc" = "yes"; then
        CMU_AFS_LIB_WHERE1("$i/lib")
        if test "$ac_cv_found_afs_lib" = "yes"; then
          ac_cv_afs_where=$i
          AC_MSG_RESULT(found)
          break
        else
          AC_MSG_RESULT(not found)
        fi
      else
        AC_MSG_RESULT(not found)
      fi
    done
])

AC_DEFUN(CMU_AFS, [
AC_REQUIRE([CMU_SOCKETS])
AC_REQUIRE([CMU_LIBSSL])
AC_ARG_WITH(AFS,
	[  --with-afs=PREFIX      Compile with AFS support],
	[if test "X$with_AFS" = "X"; then
		with_AFS=yes
	fi])

	if test "X$with_AFS" != "X"; then
	  ac_cv_afs_where=$with_AFS
	fi
	if test "X$ac_cv_afs_where" = "X"; then
	  CMU_AFS_WHERE(/usr/afsws /usr/local /usr/athena)
	fi

	AC_MSG_CHECKING(whether to include AFS)
	if test "X$ac_cv_afs_where" = "Xno" -o "X$ac_cv_afs_where" = "X"; then
	  ac_cv_found_afs=no
	  AC_MSG_RESULT(no)
	else
	  ac_cv_found_afs=yes
	  AC_MSG_RESULT(yes)
	  AFS_INC_DIR="$ac_cv_afs_where/include"
	  AFS_LIB_DIR="$ac_cv_afs_where/lib"
	  AFS_TOP_DIR="$ac_cv_afs_where"
	  AFS_INC_FLAGS="-I${AFS_INC_DIR}"
          AFS_LIB_FLAGS="-L${AFS_LIB_DIR} -L${AFS_LIB_DIR}/afs"
          cmu_save_LIBS="$LIBS"
          cmu_save_CPPFLAGS="$CPPFLAGS"
          CPPFLAGS="$CPPFLAGS ${AFS_INC_FLAGS}"
	  cmu_save_LDFLAGS="$LDFLAGS"
 	  LDFLAGS="$cmu_save_LDFLAGS ${AFS_LIB_FLAGS}"
                        
          AC_CHECK_HEADER(afs/stds.h)

          AC_MSG_CHECKING([if libdes is needed])
          AC_TRY_LINK([],[des_quad_cksum();],AFS_DES_LIB="",AFS_DES_LIB="maybe")
          if test "X$AFS_DES_LIB" != "X"; then
              LIBS="$cmu_save_LIBS -ldes"
              AC_TRY_LINK([], [des_quad_cksum();],AFS_DES_LIB="yes")
              if test "X$AFS_DES_LIB" = "Xyes"; then
                  AC_MSG_RESULT([yes])
    	          AFS_LIBDES="-ldes"
    	          AFS_LIBDESA="${AFS_LIB_DIR}/libdes.a"
    	      else
   	          LIBS="$cmu_save_LIBS $LIBSSL_LIB_FLAGS"
 	          AC_TRY_LINK([],
	          [des_quad_cksum();],AFS_DES_LIB="libcrypto")
	          if test "X$AFS_DES_LIB" = "Xlibcrypto"; then
	              AC_MSG_RESULT([libcrypto])
		      AFS_LIBDES="$LIBSSL_LIB_FLAGS"
	              AFS_LIBDESA="$LIBSSL_LIB_FLAGS"
	          else
         	      AC_MSG_RESULT([unknown])
	              AC_MSG_ERROR([Could not use -ldes])
	          fi 
	      fi 
	  else
             AC_MSG_RESULT([no])
          fi


	  AFS_CLIENT_LIBS_STATIC="${AFS_LIB_DIR}/afs/libvolser.a ${AFS_LIB_DIR}/afs/libvldb.a ${AFS_LIB_DIR}/afs/libkauth.a ${AFS_LIB_DIR}/afs/libprot.a ${AFS_LIB_DIR}/libubik.a ${AFS_LIB_DIR}/afs/libauth.a ${AFS_LIB_DIR}/librxkad.a ${AFS_LIB_DIR}/librx.a ${AFS_LIB_DIR}/afs/libsys.a ${AFS_LIB_DIR}/librx.a ${AFS_LIB_DIR}/liblwp.a ${AFS_LIBDESA} ${AFS_LIB_DIR}/afs/libcmd.a ${AFS_LIB_DIR}/afs/libcom_err.a ${AFS_LIB_DIR}/afs/util.a"
          AFS_KTC_LIBS_STATIC="${AFS_LIB_DIR}/afs/libauth.a ${AFS_LIB_DIR}/afs/libsys.a ${AFS_LIB_DIR}/librx.a ${AFS_LIB_DIR}/liblwp.a ${AFS_LIBDESA} ${AFS_LIB_DIR}/afs/libcom_err.a ${AFS_LIB_DIR}/afs/util.a"
	  AFS_CLIENT_LIBS="-lvolser -lvldb -lkauth -lprot -lubik -lauth -lrxkad -lrx ${AFS_LIB_DIR}/afs/libsys.a -lrx -llwp ${AFS_LIBDES} -lcmd -lcom_err ${AFS_LIB_DIR}/afs/util.a"
	  AFS_RX_LIBS="-lauth -lrxkad -lrx ${AFS_LIB_DIR}/afs/libsys.a -lrx -llwp ${AFS_LIBDES} -lcmd -lcom_err ${AFS_LIB_DIR}/afs/util.a"
          AFS_KTC_LIBS="-lauth ${AFS_LIB_DIR}/afs/libsys.a -lrx -llwp ${AFS_LIBDES} -lcom_err ${AFS_LIB_DIR}/afs/util.a"
          LIBS="$cmu_save_LIBS"
          AC_CHECK_FUNC(flock)
          LIBS="$cmu_save_LIBS ${AFS_CLIENT_LIBS} ${LIB_SOCKET}"
          if test "X$ac_cv_func_flock" != "Xyes"; then
             AC_MSG_CHECKING([if AFS needs flock])
             AC_TRY_LINK([#include <afs/param.h>
#ifdef HAVE_AFS_STDS_H
#include <afs/stds.h>
#endif
#include <ubik.h>
#include <afs/cellconfig.h>
#include <afs/auth.h>
#include <afs/volser.h>
struct ubik_client * cstruct;
int sigvec() {return 0;}
extern int UV_SetSecurity();],
             [vsu_ClientInit(1,"","",0,
                             &cstruct,UV_SetSecurity)],
             AFS_FLOCK=no,AFS_FLOCK=yes)
             if test $AFS_FLOCK = "no"; then
                AC_MSG_RESULT([no])
             else
               AC_MSG_RESULT([yes])
               LDFLAGS="$LDFLAGS -L/usr/ucblib"
               AC_CHECK_LIB(ucb, flock,:, [AC_CHECK_LIB(BSD, flock)])
             fi
          fi
          LIBS="$cmu_save_LIBS"
          AC_CHECK_FUNC(sigvec)
          LIBS="$cmu_save_LIBS ${AFS_CLIENT_LIBS} ${LIB_SOCKET}"
          if test "X$ac_cv_func_sigvec" != "Xyes"; then
             AC_MSG_CHECKING([if AFS needs sigvec])
             AC_TRY_LINK([#include <afs/param.h>
#ifdef HAVE_AFS_STDS_H
#include <afs/stds.h>
#endif
#include <ubik.h>
#include <afs/cellconfig.h>
#include <afs/auth.h>
#include <afs/volser.h>
struct ubik_client * cstruct;
int flock() {return 0;}
extern int UV_SetSecurity();],
             [vsu_ClientInit(1,"","",0,
                             &cstruct,UV_SetSecurity)],
             AFS_SIGVEC=no,AFS_SIGVEC=yes)
             if test $AFS_SIGVEC = "no"; then
                AC_MSG_RESULT([no])
             else
               AC_MSG_RESULT([yes])
               LDFLAGS="$LDFLAGS -L/usr/ucblib"
               AC_CHECK_LIB(ucb, sigvec,:,[AC_CHECK_LIB(BSD, sigvec)])
             fi
          fi
          if test "$ac_cv_lib_ucb_flock" = "yes" -o "$ac_cv_lib_ucb_sigvec" = "yes"; then
             AFS_LIB_FLAGS="${AFS_LIB_FLAGS} -L/usr/ucblib -R/usr/ucblib"
          fi
          if test "$ac_cv_lib_ucb_flock" = "yes" -o "$ac_cv_lib_ucb_sigvec" = "yes"; then
             AFS_BSD_LIB="-lucb"
          elif test "$ac_cv_lib_BSD_flock" = "yes" -o "$ac_cv_lib_BSD_sigvec" = "yes"; then
             AFS_BSD_LIB="-lBSD"
          fi
          if test "X$AFS_BSD_LIB" != "X" ; then
                AFS_CLIENT_LIBS_STATIC="$AFS_CLIENT_LIBS_STATIC $AFS_BSD_LIB"
                AFS_KTC_LIBS_STATIC="$AFS_KTC_LIBS_STATIC $AFS_BSD_LIB"
                AFS_CLIENT_LIBS="$AFS_CLIENT_LIBS $AFS_BSD_LIB"
                AFS_RX_LIBS="$AFS_CLIENT_LIBS $AFS_BSD_LIB"
                AFS_KTC_LIBS="$AFS_KTC_LIBS $AFS_BSD_LIB"
          fi
          LIBS="$cmu_save_LIBS $AFS_CLIENT_LIBS ${LIB_SOCKET}"
          AC_CHECK_FUNC(des_pcbc_init)
          if test "X$ac_cv_func_des_pcbc_init" != "Xyes"; then
           AC_CHECK_LIB(descompat, des_pcbc_init, AFS_DESCOMPAT_LIB="-ldescompat")
           if test "X$AFS_DESCOMPAT_LIB" != "X" ; then
                AFS_CLIENT_LIBS_STATIC="$AFS_CLIENT_LIBS_STATIC $AFS_DESCOMPAT_LIB"
                AFS_KTC_LIBS_STATIC="$AFS_KTC_LIBS_STATIC $AFS_DESCOMPAT_LIB"
                AFS_CLIENT_LIBS="$AFS_CLIENT_LIBS $AFS_DESCOMPAT_LIB"
                AFS_KTC_LIBS="$AFS_KTC_LIBS $AFS_DESCOMPAT_LIB"
           else

           AC_MSG_CHECKING([if rxkad needs des_pcbc_init])
           AC_TRY_LINK(,[tkt_DecodeTicket();],RXKAD_PROBLEM=no,RXKAD_PROBLEM=maybe)
            if test "$RXKAD_PROBLEM" = "maybe"; then
              AC_TRY_LINK([int des_pcbc_init() { return 0;}],
              [tkt_DecodeTicket();],RXKAD_PROBLEM=yes,RXKAD_PROBLEM=error)
              if test "$RXKAD_PROBLEM" = "yes"; then
                    AC_MSG_RESULT([yes])
                    AC_MSG_ERROR([cannot use rxkad])
              else
                    AC_MSG_RESULT([unknown])        
                    AC_MSG_ERROR([Unknown error testing rxkad])
              fi
            else
              AC_MSG_RESULT([no])
            fi
           fi
          fi

          AC_MSG_CHECKING([if libaudit is needed])
	  AFS_LIBAUDIT=""
          LIBS="$cmu_save_LIBS $AFS_CLIENT_LIBS ${LIB_SOCKET}"
          AC_TRY_LINK([#include <afs/param.h>
#ifdef HAVE_AFS_STDS_H
#include <afs/stds.h>
#endif
#include <afs/cellconfig.h>
#include <afs/auth.h>],
          [afsconf_SuperUser();],AFS_AUDIT_LIB="",AFS_AUDIT_LIB="maybe")
          if test "X$AFS_AUDIT_LIB" != "X"; then
          LIBS="$cmu_save_LIBS -lvolser -lvldb -lkauth -lprot -lubik -lauth -laudit -lrxkad -lrx ${AFS_LIB_DIR}/afs/libsys.a -lrx -llwp ${AFS_LIBDES} -lcmd -lcom_err ${AFS_LIB_DIR}/afs/util.a $AFS_BSD_LIB $AFS_DESCOMPAT_LIB $LIB_SOCKET"
             AC_TRY_LINK([#include <afs/param.h>
#ifdef HAVE_AFS_STDS_H
#include <afs/stds.h>
#endif
#include <afs/cellconfig.h>
#include <afs/auth.h>],
             [afsconf_SuperUser();],AFS_AUDIT_LIB="yes")
             if test "X$AFS_AUDIT_LIB" = "Xyes"; then
                 AC_MSG_RESULT([yes])
	         AFS_LIBAUDIT="-laudit"
	         AFS_CLIENT_LIBS_STATIC="${AFS_LIB_DIR}/afs/libvolser.a ${AFS_LIB_DIR}/afs/libvldb.a ${AFS_LIB_DIR}/afs/libkauth.a ${AFS_LIB_DIR}/afs/libprot.a ${AFS_LIB_DIR}/libubik.a ${AFS_LIB_DIR}/afs/libauth.a ${AFS_LIB_DIR}/afs/libaudit.a ${AFS_LIB_DIR}/librxkad.a ${AFS_LIB_DIR}/librx.a ${AFS_LIB_DIR}/afs/libsys.a ${AFS_LIB_DIR}/librx.a ${AFS_LIB_DIR}/liblwp.a ${AFS_LIBDESA} ${AFS_LIB_DIR}/afs/libcmd.a ${AFS_LIB_DIR}/afs/libcom_err.a ${AFS_LIB_DIR}/afs/util.a"
                 AFS_CLIENT_LIBS="-lvolser -lvldb -lkauth -lprot -lubik -lauth -laudit -lrxkad -lrx ${AFS_LIB_DIR}/afs/libsys.a -lrx -llwp ${AFS_LIBDES} -lcmd -lcom_err ${AFS_LIB_DIR}/afs/util.a $AFS_BSD_LIB $AFS_DESCOMPAT_LIB"
                 AFS_RX_LIBS="-lauth -laudit -lrxkad -lrx ${AFS_LIB_DIR}/afs/libsys.a -lrx -llwp ${AFS_LIBDES} -lcmd -lcom_err ${AFS_LIB_DIR}/afs/util.a $AFS_BSD_LIB $AFS_DESCOMPAT_LIB"
             else
                 AC_MSG_RESULT([unknown])
                 AC_MSG_ERROR([Could not use -lauth while testing for -laudit])
             fi 
          else
             AC_MSG_RESULT([no])
          fi

	  AC_CHECK_FUNCS(VL_ProbeServer)
          AC_MSG_CHECKING([if new-style afs_ integer types are defined])
          AC_CACHE_VAL(ac_cv_afs_int32,
dnl The next few lines contain a quoted argument to egrep
dnl It is critical that there be no leading or trailing whitespace
dnl or newlines
[AC_EGREP_CPP(dnl
changequote(<<,>>)dnl
<<(^|[^a-zA-Z_0-9])afs_int32[^a-zA-Z_0-9]>>dnl
changequote([,]), [#include <afs/param.h>
#ifdef HAVE_AFS_STDS_H
#include <afs/stds.h>
#endif],
ac_cv_afs_int32=yes, ac_cv_afs_int32=no)])
          AC_MSG_RESULT($ac_cv_afs_int32)
          if test $ac_cv_afs_int32 = yes ; then
            AC_DEFINE(HAVE_AFS_INT32,, [AFS provides new "unambiguous" type names])
          else
            AC_DEFINE(afs_int16, int16, [it's a type definition])
            AC_DEFINE(afs_int32, int32, [it's a type definition])
            AC_DEFINE(afs_uint16, u_int16, [it's a type definition])
            AC_DEFINE(afs_uint32, u_int32, [it's a type definition])
          fi

          CPPFLAGS="${cmu_save_CPPFLAGS}"
          LDFLAGS="${cmu_save_LDFLAGS}"
          LIBS="${cmu_save_LIBS}"
	  AC_DEFINE(AFS_ENV,, [Use AFS. (find what needs this and nuke it)])
          AC_DEFINE(AFS,, [Use AFS. (find what needs this and nuke it)])
          AC_SUBST(AFS_CLIENT_LIBS_STATIC)
          AC_SUBST(AFS_KTC_LIBS_STATIC)
          AC_SUBST(AFS_CLIENT_LIBS)
          AC_SUBST(AFS_RX_LIBS)
          AC_SUBST(AFS_KTC_LIBS)
          AC_SUBST(AFS_INC_FLAGS)
          AC_SUBST(AFS_LIB_FLAGS)
	  AC_SUBST(AFS_TOP_DIR)
	  AC_SUBST(AFS_LIBAUDIT)
	  AC_SUBST(AFS_LIBDES)
          AC_SUBST(AFS_LIBDESA)
       	fi
	])

AC_DEFUN(CMU_NEEDS_AFS,
[AC_REQUIRE([CMU_AFS])
if test "$ac_cv_found_afs" != "yes"; then
        AC_ERROR([Cannot continue without AFS])
fi])

dnl libssl.m4--Ssl libraries and includes
dnl Derrick Brashear
dnl from KTH kafs and Arla
dnl $Id: aclocal.m4,v 1.1.1.2 2003-02-12 22:34:32 ghudson Exp $

AC_DEFUN(CMU_LIBSSL_INC_WHERE1, [
saved_CPPFLAGS=$CPPFLAGS
CPPFLAGS="$saved_CPPFLAGS -I$1"
CMU_CHECK_HEADER_NOCACHE(openssl/ssl.h,
ac_cv_found_libssl_inc=yes,
ac_cv_found_libssl_inc=no)
CPPFLAGS=$saved_CPPFLAGS
])

AC_DEFUN(CMU_LIBSSL_INC_WHERE, [
   for i in $1; do
      AC_MSG_CHECKING(for libssl headers in $i)
      CMU_LIBSSL_INC_WHERE1($i)
      CMU_TEST_INCPATH($i, ssl)
      if test "$ac_cv_found_libssl_inc" = "yes"; then
        ac_cv_libssl_where_inc=$i
        AC_MSG_RESULT(found)
        break
      else
        AC_MSG_RESULT(not found)
      fi
    done
])

AC_DEFUN(CMU_LIBSSL_LIB_WHERE1, [
saved_LIBS=$LIBS
LIBS="$saved_LIBS -L$1 -lssl -lcrypto $LIB_SOCKET"
AC_TRY_LINK(,
[SSL_write();],
[ac_cv_found_ssl_lib=yes],
ac_cv_found_ssl_lib=no)
LIBS=$saved_LIBS
])

AC_DEFUN(CMU_LIBSSL_LIB_WHERE, [
   for i in $1; do
      AC_MSG_CHECKING(for libssl libraries in $i)
      CMU_LIBSSL_LIB_WHERE1($i)
      dnl deal with false positives from implicit link paths
      CMU_TEST_LIBPATH($i, ssl)
      if test "$ac_cv_found_ssl_lib" = "yes" ; then
        ac_cv_libssl_where_lib=$i
        AC_MSG_RESULT(found)
        break
      else
        AC_MSG_RESULT(not found)
      fi
    done
])

AC_DEFUN(CMU_LIBSSL, [
AC_REQUIRE([CMU_SOCKETS])
AC_ARG_WITH(libssl,
	[  --with-libssl=PREFIX      Compile with Libssl support],
	[if test "X$with_libssl" = "X"; then
		with_libssl=yes
	fi])
AC_ARG_WITH(libssl-lib,
	[  --with-libssl-lib=dir     use libssl libraries in dir],
	[if test "$withval" = "yes" -o "$withval" = "no"; then
		AC_MSG_ERROR([No argument for --with-libssl-lib])
	fi])
AC_ARG_WITH(libssl-include,
	[  --with-libssl-include=dir use libssl headers in dir],
	[if test "$withval" = "yes" -o "$withval" = "no"; then
		AC_MSG_ERROR([No argument for --with-libssl-include])
	fi])

	if test "X$with_libssl" != "X"; then
	  if test "$with_libssl" != "yes" -a "$with_libssl" != no; then
	    ac_cv_libssl_where_lib=$with_libssl/lib
	    ac_cv_libssl_where_inc=$with_libssl/include
	  fi
	fi

	if test "$with_libssl" != "no"; then 
	  if test "X$with_libssl_lib" != "X"; then
	    ac_cv_libssl_where_lib=$with_libssl_lib
	  fi
	  if test "X$ac_cv_libssl_where_lib" = "X"; then
	    CMU_LIBSSL_LIB_WHERE(/usr/local/lib/openssl /usr/lib/openssl /usr/local/lib /usr/lib)
	  fi

	  if test "X$with_libssl_include" != "X"; then
	    ac_cv_libssl_where_inc=$with_libssl_include
	  fi
	  if test "X$ac_cv_libssl_where_inc" = "X"; then
	    CMU_LIBSSL_INC_WHERE(/usr/local/include /usr/include)
	  fi
	fi

	AC_MSG_CHECKING(whether to include libssl)
	if test "X$ac_cv_libssl_where_lib" = "X" -a "X$ac_cv_libssl_where_inc" = "X"; then
	  ac_cv_found_libssl=no
	  AC_MSG_RESULT(no)
	else
	  ac_cv_found_libssl=yes
	  AC_MSG_RESULT(yes)
	  LIBSSL_INC_DIR=$ac_cv_libssl_where_inc
	  LIBSSL_LIB_DIR=$ac_cv_libssl_where_lib
	  LIBSSL_INC_FLAGS="-I${LIBSSL_INC_DIR}"
	  LIBSSL_LIB_FLAGS="-L${LIBSSL_LIB_DIR} -lssl -lcrypto"
	  if test "X$RPATH" = "X"; then
		RPATH=""
	  fi
	  case "${host}" in
	    *-*-linux*)
	      if test "X$RPATH" = "X"; then
	        RPATH="-Wl,-rpath,${LIBSSL_LIB_DIR}"
	      else 
 		RPATH="${RPATH}:${LIBSSL_LIB_DIR}"
	      fi
	      ;;
	    *-*-hpux*)
	      if test "X$RPATH" = "X"; then
	        RPATH="-Wl,+b${LIBSSL_LIB_DIR}"
	      else 
		RPATH="${RPATH}:${LIBSSL_LIB_DIR}"
	      fi
	      ;;
	    *-*-irix*)
	      if test "X$RPATH" = "X"; then
	        RPATH="-Wl,-rpath,${LIBSSL_LIB_DIR}"
	      else 
		RPATH="${RPATH}:${LIBSSL_LIB_DIR}"
	      fi
	      ;;
	    *-*-solaris2*)
	      if test "$ac_cv_prog_gcc" = yes; then
		if test "X$RPATH" = "X"; then
		  RPATH="-Wl,-R${LIBSSL_LIB_DIR}"
		else 
		  RPATH="${RPATH}:${LIBSSL_LIB_DIR}"
		fi
	      else
	        RPATH="${RPATH} -R${LIBSSL_LIB_DIR}"
	      fi
	      ;;
	  esac
	  AC_SUBST(RPATH)
	fi
	AC_SUBST(LIBSSL_INC_DIR)
	AC_SUBST(LIBSSL_LIB_DIR)
	AC_SUBST(LIBSSL_INC_FLAGS)
	AC_SUBST(LIBSSL_LIB_FLAGS)
	])


dnl kerberos_v4.m4--Kerberos 4 libraries and includes
dnl Derrick Brashear
dnl from KTH krb and Arla
dnl $Id: aclocal.m4,v 1.1.1.2 2003-02-12 22:34:32 ghudson Exp $

AC_DEFUN(CMU_KRB_SENDAUTH_PROTO, [
AC_MSG_CHECKING(for krb_sendauth prototype)
AC_TRY_COMPILE(
[#include <krb.h>
int krb_sendauth (long options, int fd, KTEXT ktext, char *service,
                  char *inst, char *realm, u_long checksum,
                  MSG_DAT *msg_data, CREDENTIALS *cred,
                  Key_schedule schedule, struct sockaddr_in *laddr,
                  struct sockaddr_in *faddr, char *version);],
[int foo = krb_sendauth(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0); ],
ac_cv_krb_sendauth_proto=no,
ac_cv_krb_sendauth_proto=yes)
AC_MSG_RESULT($ac_cv_krb_sendauth_proto)
if test "$ac_cv_krb_sendauth_proto" = yes; then
        AC_DEFINE(HAVE_KRB_SENDAUTH_PROTO)dnl
fi
AC_MSG_RESULT($ac_cv_krb_sendauth_proto)
])

AC_DEFUN(CMU_KRB_SET_KEY_PROTO, [
AC_MSG_CHECKING(for krb_set_key prototype)
AC_CACHE_VAL(ac_cv_krb_set_key_proto, [
cmu_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="${CPPFLAGS} ${KRB_INC_FLAGS}"
AC_TRY_COMPILE(
[#include <krb.h>
int krb_set_key(char *key, int cvt);],
[int foo = krb_set_key(0, 0);],
ac_cv_krb_set_key_proto=no,
ac_cv_krb_set_key_proto=yes)
])
CPPFLAGS="${cmu_save_CPPFLAGS}"
if test "$ac_cv_krb_set_key_proto" = yes; then
	AC_DEFINE(HAVE_KRB_SET_KEY_PROTO)dnl
fi
AC_MSG_RESULT($ac_cv_krb_set_key_proto)
])

AC_DEFUN(CMU_KRB4_32_DEFN, [
AC_MSG_CHECKING(for KRB4_32 definition)
AC_CACHE_VAL(ac_cv_krb4_32_defn, [
cmu_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="${CPPFLAGS} ${KRB_INC_FLAGS}"
AC_TRY_COMPILE(
[#include <krb.h>
],
[KRB4_32 foo = 1;],
ac_cv_krb4_32_defn=yes,
ac_cv_krb4_32_defn=no)
])
CPPFLAGS="${cmu_save_CPPFLAGS}"
if test "$ac_cv_krb4_32_defn" = yes; then
	AC_DEFINE(HAVE_KRB4_32_DEFINE)dnl
fi
AC_MSG_RESULT($ac_cv_krb4_32_defn)
])

AC_DEFUN(CMU_KRB_RD_REQ_PROTO, [
AC_MSG_CHECKING(for krb_rd_req prototype)
AC_CACHE_VAL(ac_cv_krb_rd_req_proto, [
cmu_save_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="${CPPFLAGS} ${KRB_INC_FLAGS}"
AC_TRY_COMPILE(
[#include <krb.h>
int krb_rd_req(KTEXT authent, char *service, char *instance,
unsigned KRB_INT32 from_addr, AUTH_DAT *ad, char *fn);],
[int foo = krb_rd_req(0,0,0,0,0,0);],
ac_cv_krb_rd_req_proto=no,
ac_cv_krb_rd_req_proto=yes)
])
CPPFLAGS="${cmu_save_CPPFLAGS}"
if test "$ac_cv_krb_rd_req_proto" = yes; then
	AC_DEFINE(HAVE_KRB_RD_REQ_PROTO)dnl
fi
AC_MSG_RESULT($ac_cv_krb_rd_req_proto)
])

AC_DEFUN(CMU_KRB_INC_WHERE1, [
saved_CPPFLAGS=$CPPFLAGS
CPPFLAGS="$saved_CPPFLAGS -I$1"
AC_TRY_COMPILE([#include <krb.h>],
[struct ktext foo;],
ac_cv_found_krb_inc=yes,
ac_cv_found_krb_inc=no)
if test "$ac_cv_found_krb_inc" = "no"; then
  CPPFLAGS="$saved_CPPFLAGS -I$1 -I$1/kerberosIV"
  AC_TRY_COMPILE([#include <krb.h>],
  [struct ktext foo;],
  [ac_cv_found_krb_inc=yes],
  ac_cv_found_krb_inc=no)
fi
CPPFLAGS=$saved_CPPFLAGS
])

AC_DEFUN(CMU_KRB_INC_WHERE, [
   for i in $1; do
      AC_MSG_CHECKING(for kerberos headers in $i)
      CMU_KRB_INC_WHERE1($i)
      CMU_TEST_INCPATH($i, krb)
      if test "$ac_cv_found_krb_inc" = "yes"; then
        ac_cv_krb_where_inc=$i
        AC_MSG_RESULT(found)
        break
      else
        AC_MSG_RESULT(not found)
      fi
    done
])

#
# Test for kerberos lib files
#

AC_DEFUN(CMU_KRB_LIB_WHERE1, [
saved_LIBS=$LIBS
LIBS="$saved_LIBS -L$1 -lkrb $KRB_LIBDES"
AC_TRY_LINK(,
[dest_tkt();],
[ac_cv_found_krb_lib=yes],
ac_cv_found_krb_lib=no)
LIBS=$saved_LIBS
])

AC_DEFUN(CMU_KRB_LIB_WHERE, [
   for i in $1; do
      AC_MSG_CHECKING(for kerberos libraries in $i)
      CMU_KRB_LIB_WHERE1($i)
      dnl deal with false positives from implicit link paths
      CMU_TEST_LIBPATH($i, krb)
      if test "$ac_cv_found_krb_lib" = "yes" ; then
        ac_cv_krb_where_lib=$i
        AC_MSG_RESULT(found)
        break
      else
        AC_MSG_RESULT(not found)
      fi
    done
])

AC_DEFUN(CMU_KRB4, [
AC_REQUIRE([CMU_SOCKETS])
AC_REQUIRE([CMU_LIBSSL])
AC_ARG_WITH(krb4,
	[  --with-krb4=PREFIX      Compile with Kerberos 4 support],
	[if test "X$with_krb4" = "X"; then
		with_krb4=yes
	fi])
AC_ARG_WITH(krb4-lib,
	[  --with-krb4-lib=dir     use kerberos 4 libraries in dir],
	[if test "$withval" = "yes" -o "$withval" = "no"; then
		AC_MSG_ERROR([No argument for --with-krb4-lib])
	fi])
AC_ARG_WITH(krb4-include,
	[  --with-krb4-include=dir use kerberos 4 headers in dir],
	[if test "$withval" = "yes" -o "$withval" = "no"; then
		AC_MSG_ERROR([No argument for --with-krb4-include])
	fi])

	if test "X$with_krb4" != "X"; then
	  if test "$with_krb4" != "yes" -a "$with_krb4" != "no"; then
	    ac_cv_krb_where_lib=$with_krb4/lib
	    ac_cv_krb_where_inc=$with_krb4/include
	  fi
	fi

	if test "$with_krb4" != "no"; then
	  if test "X$with_krb4_lib" != "X"; then
	    ac_cv_krb_where_lib=$with_krb4_lib
	  fi
	  if test "X$with_krb4_include" != "X"; then
	    ac_cv_krb_where_inc=$with_krb4_include
	  fi
	  if test "X$ac_cv_krb_where_inc" = "X"; then
	    CMU_KRB_INC_WHERE(/usr/athena/include /usr/include/kerberosIV /usr/local/include /usr/include/kerberos)
	  fi
	fi

          AC_MSG_CHECKING([if libdes is needed])
          AC_TRY_LINK([],[des_quad_cksum();],KRB_DES_LIB="",KRB_DES_LIB="maybe")
          if test "X$KRB_DES_LIB" != "X"; then
              LIBS="$cmu_save_LIBS -ldes"
              AC_TRY_LINK([], [des_quad_cksum();],KRB_DES_LIB="yes")
              if test "X$KRB_DES_LIB" = "Xyes"; then
                  AC_MSG_RESULT([yes])
                  KRB_LIBDES="-ldes"
                  KRB_LIBDESA="${KRB_LIB_DIR}/libdes.a"
              else
                  LIBS="$cmu_save_LIBS $LIBSSL_LIB_FLAGS"
                  AC_TRY_LINK([],
                  [des_quad_cksum();],KRB_DES_LIB="libcrypto")
                  if test "X$KRB_DES_LIB" = "Xlibcrypto"; then
                      AC_MSG_RESULT([libcrypto])
                      KRB_LIBDES="$LIBSSL_LIB_FLAGS"
                      KRB_LIBDESA="$LIBSSL_LIB_FLAGS"
                  else
                      AC_MSG_RESULT([unknown])
                      AC_MSG_ERROR([Could not use -ldes])
                  fi 
              fi 
          else
             AC_MSG_RESULT([no])
          fi
	  LIBS="${cmu_save_LIBS}"

	  if test "X$ac_cv_krb_where_lib" = "X"; then
	    CMU_KRB_LIB_WHERE(/usr/athena/lib /usr/local/lib /usr/lib)
	  fi

	AC_MSG_CHECKING(whether to include kerberos 4)
	if test "X$ac_cv_krb_where_lib" = "X" -o "X$ac_cv_krb_where_inc" = "X"; then
	  ac_cv_found_krb=no
	  AC_MSG_RESULT(no)
	else
	  ac_cv_found_krb=yes
	  AC_MSG_RESULT(yes)
	  KRB_INC_DIR=$ac_cv_krb_where_inc
	  KRB_LIB_DIR=$ac_cv_krb_where_lib
	  KRB_INC_FLAGS="-I${KRB_INC_DIR}"
	  KRB_LIB_FLAGS="-L${KRB_LIB_DIR} -lkrb ${KRB_LIBDES}"
	  LIBS="${cmu_save_LIBS} ${KRB_LIB_FLAGS}"
	  AC_CHECK_LIB(resolv, dns_lookup, KRB_LIB_FLAGS="${KRB_LIB_FLAGS} -lresolv",,"${KRB_LIB_FLAGS}")
	  AC_CHECK_LIB(crypt, crypt, KRB_LIB_FLAGS="${KRB_LIB_FLAGS} -lcrypt",,"${KRB_LIB_FLAGS}")
	  AC_CHECK_FUNCS(krb_get_int krb_life_to_time)
          AC_SUBST(KRB_INC_FLAGS)
          AC_SUBST(KRB_LIB_FLAGS)
	  LIBS="${cmu_save_LIBS}"
	  AC_DEFINE(KERBEROS,,[Use kerberos 4. find out what needs this symbol])
	  if test "X$RPATH" = "X"; then
		RPATH=""
	  fi
	  case "${host}" in
	    *-*-linux*)
	      if test "X$RPATH" = "X"; then
	        RPATH="-Wl,-rpath,${KRB_LIB_DIR}"
	      else 
		RPATH="${RPATH}:${KRB_LIB_DIR}"
	      fi
	      ;;
	    *-*-hpux*)
	      if test "X$RPATH" = "X"; then
	        RPATH="-Wl,+b${KRB_LIB_DIR}"
	      else 
		RPATH="${RPATH}:${KRB_LIB_DIR}"
	      fi
	      ;;
	    *-*-irix*)
	      if test "X$RPATH" = "X"; then
	        RPATH="-Wl,-rpath,${KRB_LIB_DIR}"
	      else 
		RPATH="${RPATH}:${KRB_LIB_DIR}"
	      fi
	      ;;
	    *-*-solaris2*)
	      if test "$ac_cv_prog_gcc" = yes; then
		if test "X$RPATH" = "X"; then
		  RPATH="-Wl,-R${KRB_LIB_DIR}"
		else 
		  RPATH="${RPATH}:${KRB_LIB_DIR}"
		fi
	      else
	        RPATH="${RPATH} -R${KRB_LIB_DIR}"
	      fi
	      ;;
	  esac
	  AC_SUBST(RPATH)
	fi
	])


dnl See whether we can use IPv6 related functions
dnl contributed by Hajimu UMEMOTO

AC_DEFUN(IPv6_CHECK_FUNC, [
AC_CHECK_FUNC($1, [dnl
  ac_cv_lib_socket_$1=no
  ac_cv_lib_inet6_$1=no
], [dnl
  AC_CHECK_LIB(socket, $1, [dnl
    LIBS="$LIBS -lsocket"
    ac_cv_lib_inet6_$1=no
  ], [dnl
    AC_MSG_CHECKING([whether your system has IPv6 directory])
    AC_CACHE_VAL(ipv6_cv_dir, [dnl
      for ipv6_cv_dir in /usr/local/v6 /usr/inet6 no; do
	if test $ipv6_cv_dir = no -o -d $ipv6_cv_dir; then
	  break
	fi
      done])dnl
    AC_MSG_RESULT($ipv6_cv_dir)
    if test $ipv6_cv_dir = no; then
      ac_cv_lib_inet6_$1=no
    else
      if test x$ipv6_libinet6 = x; then
	ipv6_libinet6=no
	SAVELDFLAGS="$LDFLAGS"
	LDFLAGS="$LDFLAGS -L$ipv6_cv_dir/lib"
      fi
      AC_CHECK_LIB(inet6, $1, [dnl
	if test $ipv6_libinet6 = no; then
	  ipv6_libinet6=yes
	  LIBS="$LIBS -linet6"
	fi],)dnl
      if test $ipv6_libinet6 = no; then
	LDFLAGS="$SAVELDFLAGS"
      fi
    fi])dnl
])dnl
if test $ac_cv_func_$1 = yes -o $ac_cv_lib_socket_$1 = yes \
     -o $ac_cv_lib_inet6_$1 = yes
then
  ipv6_cv_$1=yes
  ifelse([$2], , :, [$2])
else
  ipv6_cv_$1=no
  ifelse([$3], , :, [$3])
fi])


dnl See whether we have ss_family in sockaddr_storage
AC_DEFUN(IPv6_CHECK_SS_FAMILY, [
AC_MSG_CHECKING([whether you have ss_family in struct sockaddr_storage])
AC_CACHE_VAL(ipv6_cv_ss_family, [dnl
AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>],
	[struct sockaddr_storage ss; int i = ss.ss_family;],
	[ipv6_cv_ss_family=yes], [ipv6_cv_ss_family=no])])dnl
if test $ipv6_cv_ss_family = yes; then
  ifelse([$1], , AC_DEFINE(HAVE_SS_FAMILY), [$1])
else
  ifelse([$2], , :, [$2])
fi
AC_MSG_RESULT($ipv6_cv_ss_family)])


dnl whether you have sa_len in struct sockaddr
AC_DEFUN(IPv6_CHECK_SA_LEN, [
AC_MSG_CHECKING([whether you have sa_len in struct sockaddr])
AC_CACHE_VAL(ipv6_cv_sa_len, [dnl
AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/socket.h>],
	       [struct sockaddr sa; int i = sa.sa_len;],
	       [ipv6_cv_sa_len=yes], [ipv6_cv_sa_len=no])])dnl
if test $ipv6_cv_sa_len = yes; then
  ifelse([$1], , AC_DEFINE(HAVE_SOCKADDR_SA_LEN), [$1])
else
  ifelse([$2], , :, [$2])
fi
AC_MSG_RESULT($ipv6_cv_sa_len)])


dnl See whether sys/socket.h has socklen_t
AC_DEFUN(IPv6_CHECK_SOCKLEN_T, [
AC_MSG_CHECKING(for socklen_t)
AC_CACHE_VAL(ipv6_cv_socklen_t, [dnl
AC_TRY_LINK([#include <sys/types.h>
#include <sys/socket.h>],
	    [socklen_t len = 0;],
	    [ipv6_cv_socklen_t=yes], [ipv6_cv_socklen_t=no])])dnl
if test $ipv6_cv_socklen_t = yes; then
  ifelse([$1], , AC_DEFINE(HAVE_SOCKLEN_T), [$1])
else
  ifelse([$2], , :, [$2])
fi
AC_MSG_RESULT($ipv6_cv_socklen_t)])



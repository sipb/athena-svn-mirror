# a macro to get the libs/cflags for soup
# serial 1

dnl AM_PATH_SOUP([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND [, MODULES]]])
dnl Test to see if soup is installed, and define SOUP_CFLAGS, LIBS
dnl
AC_DEFUN(AM_PATH_SOUP,
[dnl
dnl Get the cflags and libraries from the soup-config script
dnl
AC_ARG_WITH(soup-config,
[  --with-soup-config=SOUP_CONFIG  Location of soup-config],
SOUP_CONFIG="$withval")

module_args=soup

AC_PATH_PROG(SOUP_CONFIG, soup-config, no)
AC_MSG_CHECKING(for soup)
if test "$SOUP_CONFIG" = "no"; then
  AC_MSG_RESULT(no)
  ifelse([$2], , :, [$2])
else
  if $SOUP_CONFIG --check $module_args; then
    SOUP_CFLAGS=`$SOUP_CONFIG --cflags $module_args`
    SOUP_LIBS=`$SOUP_CONFIG --libs $module_args`
    AC_MSG_RESULT(yes)
    ifelse([$1], , :, [$1])
  else
    echo "*** soup was not compiled with support for $module_args" 1>&2
    AC_MSG_RESULT(no)
    ifelse([$2], , :, [$2])
  fi
fi
AC_SUBST(SOUP_CFLAGS)
AC_SUBST(SOUP_LIBS)
])
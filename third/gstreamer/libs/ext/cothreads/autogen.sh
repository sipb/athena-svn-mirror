#!/bin/bash
# Run this to generate all the initial makefiles, etc.

DIE=0
package=cothreads
srcfile=cothreads/cothreads.c

# a quick cvs co to ease the transition
if test ! -d common; then
  echo "+ getting common from cvs"; cvs co common
fi

# source helper functions
if test ! -f common/gst-autogen.sh;
then
  echo There is something wrong with your source tree.
  echo You are missing common/gst-autogen.sh
  exit 1
fi
. common/gst-autogen.sh

autogen_options $@


echo -n "+ check for build tools"
if test ! -z $NOCHECK; then echo ": skipped version checks"; else  echo; fi
version_check "autoconf" "$AUTOCONF autoconf autoconf-2.54 autoconf-2.53 autoconf-2.52" \
              "ftp://ftp.gnu.org/pub/gnu/autoconf/" 2 52 || DIE=1
version_check "automake" "$AUTOMAKE automake automake-1.7 automake-1.6 automake-1.5" \
              "ftp://ftp.gnu.org/pub/gnu/automake/" 1 5 || DIE=1
version_check "libtool" "" \
              "ftp://ftp.gnu.org/pub/gnu/libtool/" 1 4 0 || DIE=1
version_check "pkg-config" "" \
              "http://www.freedesktop.org/software/pkgconfig" 0 8 0 || DIE=1

die_check $DIE

autoconf_2.52d_check || DIE=1
aclocal_check || DIE=1
autoheader_check || DIE=1

die_check $DIE

CONFIGURE_DEF_OPT='--enable-maintainer-mode'
# if no arguments specified then this will be printed
if test -z "$*"; then
  echo "+ checking for autogen.sh options"
  echo "  This autogen script will automatically run ./configure as:"
  echo "  ./configure $CONFIGURE_DEF_OPT"
  echo "  To pass any additional options, please specify them on the $0"
  echo "  command line."
fi

toplevel_check $srcfile

if test -e acinclude.m4; then rm acinclude.m4; fi
tool_run "$aclocal" "-I m4 -I common/m4 $ACLOCAL_FLAGS"

### delete me
### echo "+ creating acinclude.m4"
### cat m4/*.m4 > acinclude.m4

tool_run "libtoolize" "--copy --force"
tool_run "$autoheader"

# touch the stamp-h.in build stamp so we don't re-run autoheader in maintainer mode -- wingo
echo timestamp > stamp-h.in 2> /dev/null

tool_run "$autoconf"
debug "automake: $automake"
tool_run "$automake" "-a -c"

test -n "$NOCONFIGURE" && {
    echo "skipping configure stage for package $package, as requested."
    echo "autogen.sh done."
    exit 0
}

echo "+ running configure ... "
test ! -z "$CONFIGURE_DEF_OPT" && echo "  ./configure default flags: $CONFIGURE_DEF_OPT"
test ! -z "$CONFIGURE_EXT_OPT" && echo "  ./configure external flags: $CONFIGURE_EXT_OPT"
echo

./configure $CONFIGURE_DEF_OPT $CONFIGURE_EXT_OPT || {
        echo "  configure failed"
        exit 1
}

echo "Now type 'make' to compile $package."
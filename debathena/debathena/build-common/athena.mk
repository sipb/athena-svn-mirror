# -*- mode: makefile; coding: utf-8 -*-

# Copyright 2008 by the Massachusetts Institute of Technology.

# Permission to use, copy, modify, and distribute this software and
# its documentation for any purpose and without fee is hereby granted,
# provided that the above copyright notice appear in all copies and
# that both that copyright notice and this permission notice appear in
# supporting documentation, and that the name of M.I.T. not be used in
# advertising or publicity pertaining to distribution of the software
# without specific, written prior permission.  M.I.T. makes no
# representations about the suitability of this software for any
# purpose.  It is provided "as is" without express or implied
# warranty.

# athena.mk: CDBS class file for building Athena sources, which have
# the following characteristics:
#
#   - They use autoconf.
#   - They do not contain any generated or boilerplate autoconf files.
#   - They assume the standard Athena aclocal.m4.
#   - They may use libtool.
#
# buildcore.mk will re-copy config.guess and config.sub after one
# invocation of post-patches.  This is harmless.
#
# This package contains its own copies of install-sh and mkinstalldirs
# to avoid a dependency on any particular version of automake.
# Neither script is prone to frequent changes.

_cdbs_rules_path ?= /usr/share/cdbs/1/rules
_cdbs_class_path ?= /usr/share/cdbs/1/class

ifndef _cdbs_rules_athena
_cdbs_rules_athena = 1

include $(_cdbs_rules_path)/debhelper.mk$(_cdbs_makefile_suffix)
include $(_cdbs_class_path)/autotools.mk$(_cdbs_makefile_suffix)

DEB_AUTO_UPDATE_DEBIAN_CONTROL = 1

CDBS_BUILD_DEPENDS := $(CDBS_BUILD_DEPENDS), debathena-build-common
CDBS_BUILD_DEPENDS := $(CDBS_BUILD_DEPENDS), autotools-dev
CDBS_BUILD_DEPENDS := $(CDBS_BUILD_DEPENDS), libtool
CDBS_BUILD_DEPENDS := $(CDBS_BUILD_DEPENDS), autoconf

ATHENA_ACLOCAL = /usr/share/debathena-build-common/aclocal.m4
ATHENA_MKINSTALLDIRS = /usr/share/debathena-build-common/mkinstalldirs
ATHENA_INSTALL_SH = /usr/share/debathena-build-common/install-sh

post-patches:: athena-autoconf
athena-autoconf::
	cp /usr/share/debathena-build-common/aclocal.m4 $(DEB_SRCDIR)
	cp /usr/share/debathena-build-common/mkinstalldirs $(DEB_SRCDIR)
	cp /usr/share/debathena-build-common/install-sh $(DEB_SRCDIR)
	cp /usr/share/misc/config.guess $(DEB_SRCDIR)
	cp /usr/share/misc/config.sub $(DEB_SRCDIR)
	cat /usr/share/libtool/libtool.m4 >> $(DEB_SRCDIR)/aclocal.m4
	cp /usr/share/libtool/ltmain.sh $(DEB_SRCDIR)
	(cd $(DEB_SRCDIR) && autoconf)

clean:: athena-clean-autoconf
athena-clean-autoconf::
	rm -f $(DEB_SRCDIR)/aclocal.m4
	rm -f $(DEB_SRCDIR)/mkinstalldirs
	rm -f $(DEB_SRCDIR)/install-sh
	rm -f $(DEB_SRCDIR)/config.guess
	rm -f $(DEB_SRCDIR)/config.sub
	rm -f $(DEB_SRCDIR)/ltmain.sh
	rm -f $(DEB_SRCDIR)/configure
	rm -rf $(DEB_SRCDIR)/autom4te.cache

endif

## Process this file with automake to produce Makefile.in

NULL=

aclocal_macros = xml-i18n-tools.m4

CLEANFILES = xml-i18n-toolize xml-i18n-extract xml-i18n-merge xml-i18n-update

# These are required by xml-i18n-toolize.
pkgdata_SCRIPTS = xml-i18n-extract xml-i18n-merge xml-i18n-update
pkgdata_DATA = xml-i18n-tools.Makefile.patch

# This macro file should be visible to Automake's aclocal.
aclocal_DATA = $(aclocal_macros)

# The  xml-i18n-tools distributor.
bin_SCRIPTS = xml-i18n-toolize xml-i18n-update xml-i18n-extract

EXTRA_DIST = \
	$(aclocal_macros) \
	xml-i18n-toolize.in \
	xml-i18n-extract.in \
	xml-i18n-merge.in \
	xml-i18n-update.in \
	xml-i18n-unicodify \
	xml-i18n-tools.spec \
	$(pkgdata_DATA) \
	$(NULL)

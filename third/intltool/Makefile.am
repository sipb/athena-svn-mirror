## Process this file with automake to produce Makefile.in

SUBDIRS = tests doc

NULL = 

aclocal_macros = intltool.m4

CLEANFILES =					\
	intltoolize				\
	intltool-extract.in			\
	intltool-merge.in			\
	intltool-update.in			\
	intltool-extract			\
	intltool-merge				\
	intltool-prepare			\
	intltool-update				\
	$(NULL)

# These are required by intltoolize and
pkgdata_DATA =					\
	Makefile.in.in				\
	intltool-extract.in			\
	intltool-merge.in			\
	intltool-update.in			\
	$(NULL)


# This macro file should be visible to Automake's aclocal.
aclocal_DATA = $(aclocal_macros)

# The intltool distributor and installed versions of scripts

bin_SCRIPTS =					\
	intltoolize				\
	intltool-update				\
	intltool-merge				\
	intltool-extract			\
	intltool-prepare			\
	$(NULL)

EXTRA_DIST =					\
	$(aclocal_macros)			\
	intltoolize.in				\
	intltool-extract.in.in			\
	intltool-merge.in.in			\
	intltool-update.in.in			\
	intltool-prepare.in			\
	intltool.spec.in			\
	intltool.spec				\
	$(pkgdata_DATA)				\
	$(NULL)

%: %.in Makefile
	sed -e "s#@INTLTOOL_PERL@#$(PERL)#g" \
	    -e "s#@INTLTOOL_ICONV@#$(INTLTOOL_ICONV)#g" \
	    -e "s#@INTLTOOL_MSGFMT@#$(INTLTOOL_MSGFMT)#g" \
	    -e "s#@INTLTOOL_MSGMERGE@#$(INTLTOOL_MSGMERGE)#g" \
	    -e "s#@INTLTOOL_XGETTEXT@#$(INTLTOOL_XGETTEXT)#g" \
	    -e "s#@INTLTOOL_EXTRACT@#$(bindir)/intltool-extract#g" \
	    < $< > $@

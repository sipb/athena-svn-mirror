##
## This Makefile.am is used both in gnome-libs and gnome-docu.
## There is a `INSIDE_GNOME_DOCU' automake conditional which is true in
## gnome-docu and false in gnome-libs.
##
## April 10,
## Martin Baulig
##

EXTRA_DIST	= gnome-faq.sgml
SUFFIXES	= .tar.gz .sgml

if INSIDE_GNOME_DOCU

# Make rules for the `gnome-docu' module

htmldir		= $(datadir)/html

html_subdirs	= gnome-faq

# Well, yes - you are using GNU Make, aren't you ... ?
tgz_FILES	= $(addsuffix .tar.gz,$(html_subdirs))
sgml_FILES	= $(addsuffix .sgml,$(html_subdirs))

noinst_DATA	= $(tgz_FILES)

CLEANFILES	= $(tgz_FILES)

include $(TOPDIR)/scripts/Makefile.Common

distclean-local:
	$(MAKE) distclean-do-local

install-data-local:
	$(MAKE) install-data-do-local

else

# Make rules for the `gnome-libs' module

htmldir = $(prefix)/html
#html_DATA = gnome-faq.html

all-local: gnome-faq/book1.html

gnome-faq/book1.html: gnome-faq.sgml
	-(cd $(srcdir); db2html gnome-faq.sgml) > /dev/null

install-data-local: gnome-faq/book1.html
	$(mkinstalldirs) $(DESTDIR)$(htmldir)
	installfiles=`echo $(srcdir)/gnome-faq/*.html`;			\
	if test "$$installfiles" != '$(srcdir)/gnome-faq/*.html'; then	\
	  for file in $$installfiles; do				\
            basefile=`echo $$file | sed -e 's,^.*/,,'`;			\
            $(INSTALL_DATA) $$file $(DESTDIR)$(htmldir)/$$basefile;	\
          done;								\
	else :; fi
	installfiles=`echo $(srcdir)/gnome-faq/*.css`;			\
	if test "$$installfiles" != '$(srcdir)/gnome-faq/*.css'; then	\
	  for file in $$installfiles; do				\
            basefile=`echo $$file | sed -e 's,^.*/,,'`;			\
            $(INSTALL_DATA) $$file $(DESTDIR)$(htmldir)/$$basefile;	\
          done;								\
	else :; fi

dist-hook:
	tar cvf - gnome-faq | (cd $(distdir); tar xf -)

distclean-local:

endif

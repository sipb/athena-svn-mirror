lang = C
app = control-center

figs = \
	figs/bg-capplet.png		\
	figs/gccappdef.png		\
	figs/gccalf.png   		\
	figs/gccdialog.png		\
	figs/gccedit.png		\
	figs/gcchints.png		\
	figs/gccmdi.png			\
	figs/gccmime.png		\
	figs/gccsession-props.png	\
	figs/gccsessopt.png      	\
	figs/gccsound1.png		\
	figs/gccsound2.png		\
	figs/gccstartup.png		\
	figs/gccurl.png			\
	figs/glob_pref_anim.png		\
	figs/keyboard-capplet.png	\
	figs/mouse-capplet.png		\
	figs/screensave-capplet.png	\
	figs/theme-select.png		\
	figs/wm-main.png

EXTRA_DIST = \
	gnomecc.sgml

helpdir = $(datadir)/gnome/help/$(app)/$(lang)
help_DATA = \
	index.html	\
	topic.dat

EXTRA_DIST = gnomecc.sgml $(help_DATA) $(figs)


all: index.html

index.html: gnomecc/index.html
	-cp gnomecc/index.html .

# the wierd srcdir trick is because the db2html from the Cygnus RPMs
# cannot handle relative filenames
gnomecc/index.html: $(srcdir)/gnomecc.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/gnomecc.sgml

dist-hook: index.html
	-$(mkinstalldirs) $(distdir)/
	-$(mkinstalldirs) $(distdir)/gnomecc/stylesheet-images
	-cp $(srcdir)/gnomecc/*.html $(distdir)/gnomecc
	-cp $(srcdir)/gnomecc/*.css $(distdir)/gnomecc
	-cp $(srcdir)/gnomecc/stylesheet-images/*.gif \
		$(distdir)/gnomecc/stylesheet-images

install-data-am: index.html
	$(mkinstalldirs) $(DESTDIR)$(helpdir)
	$(mkinstalldirs) $(DESTDIR)$(helpdir)/stylesheet-images
	$(mkinstalldirs) $(DESTDIR)$(helpdir)/figs
	-cp $(srcdir)/topic.dat $(DESTDIR)$(helpdir)
	-for file in $(srcdir)/gnomecc/*.html $(srcdir)/gnomecc/*.css $(srcdir)/*.png; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(helpdir)/$$basefile; \
	done
	-for file in $(srcdir)/figs/*.png; do \
	  basefile=`echo $$file | sed -e  's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(helpdir)/figs/$$basefile; \
	done
	-for file in $(srcdir)/gnomecc/stylesheet-images/*.gif; do \
	  basefile=`echo $$file | sed -e  's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(helpdir)/stylesheet-images/$$basefile; \
	done

$(app).ps: $(srcdir)/gnomecc.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2ps $$srcdir/gnomecc.sgml

$(app).rtf: $(srcdir)/gnomecc.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2ps $$srcdir/gnomecc.sgml








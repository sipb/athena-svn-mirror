gnome_terminal_helpdir = $(datadir)/gnome/help/gnome-terminal/de

gnome_terminal_help_DATA = 	\
	index.html		\
	topic.dat

EXTRA_DIST = gnome-terminal.sgml topic.dat

all: index.html

index.html: gnome-terminal/index.html
	-cp gnome-terminal/index.html .

# the wierd srcdir trick is because the db2html from the Cygnus RPMs
# cannot handle relative filenames
gnome-terminal/index.html: $(srcdir)/gnome-terminal.sgml
	-srcdir=`cd $(srcdir) && pwd`; \
	db2html $$srcdir/gnome-terminal.sgml

dist-hook: index.html
	mkdir $(distdir)/gnome-terminal
	-cp gnome-terminal/*.html $(distdir)/gnome-terminal
	-cp gnome-terminal/*.css $(distdir)/gnome-terminal

install-data-local: index.html
	-for file in $(srcdir)/gnome-terminal/*.html $(srcdir)/gnome-terminal/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(gnome_terminal_helpdir)/$$basefile; \
	done

gnome-terminal.ps: gnome-terminal.sgml
	-db2ps $<

gnome-terminal.rtf: gnome-terminal.sgml
	-db2rtf $<

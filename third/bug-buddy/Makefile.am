SUBDIRS = po pixmaps src bugzilla docs

EXTRA_DIST = \
	bug-buddy.spec.in	\
	bug-buddy.spec		\
	intltool-extract.in	\
	intltool-merge.in	\
	intltool-update.in	\
	COPYING.ximian-logos	\
	omf.make		\
	xmldocs.make

# this should probably be run at dist time or something
update-bugzilla:
	cd $(top_srcdir)/bugzilla/ximian/ \
	&& for file in products config mostfreq; do \
		wget "http://bugzilla.ximian.com/bugzilla-$$file.xml" -O $$file.tmp \
		&& (cmp -s $$file.tmp $$file || cp -v $$file.tmp $$file) \
		&& rm -f $$file.tmp; \
	done
	cd $(top_srcdir)/bugzilla/gnome/ \
	&& for file in products config mostfreq; do \
		wget "http://bugzilla.gnome.org/bugzilla.gnome.org/bugzilla-$$file.xml" -O $$file.tmp \
		&& (cmp -s $$file.tmp $$file || cp -v $$file.tmp $$file) \
		&& rm -f $$file.tmp; \
	done

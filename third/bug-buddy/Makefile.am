SUBDIRS = po pixmaps src bugzilla docs

schemasdir       = $(GCONF_SCHEMA_FILE_DIR)
schemas_in_files = bug-buddy.schemas.in
schemas_DATA     = $(schemas_in_files:.schemas.in=.schemas)

EXTRA_DIST = \
	bug-buddy.spec.in				\
	bug-buddy.spec					\
	intltool-extract.in				\
	intltool-merge.in				\
	intltool-update.in				\
	COPYING.ximian-logos				\
	omf.make					\
	xmldocs.make					\
	$(schemas_in_files)				\
	$(schemas_DATA)

if GCONF_SCHEMAS_INSTALL
install-data-local:
	if test -z "$(DESTDIR)" ; then \
		for p in $(schemas_DATA) ; do \
			GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $$p ; \
		done \
	fi
else
install-data-local:
endif




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

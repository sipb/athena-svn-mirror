if BUILD_TEST_VIEW
TEST_DIR=test-view
else
TEST_DIR=
endif

SUBDIRS = \
	media-info \
	audio-view audio-properties-view \
	m4 $(TEST_DIR) thumbnail pixmaps po

EXTRA_DIST = \
	ChangeLog \
	intltool-extract.in intltool-update.in intltool-merge.in \
	$(PACKAGE).spec

DISTCLEANFILES = intltool-extract intltool-update intltool-merge \
	po/.intltool-merge-cache


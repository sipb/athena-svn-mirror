# If you want to build these components, please cd to their respective
# directories and type `make'.

always_built_SUBDIRS = cddb-slave2

SUBDIRS = po $(always_built_SUBDIRS) \
	  @PROGRAMS_VUMETER@ \
	  @PROGRAMS_GMIX@ \
	  @PROGRAMS_GNOME_CD@ \
	  @PROGRAMS_GRECORD@

DIST_SUBDIRS = po $(always_built_SUBDIRS) \
	       vu-meter gmix grecord gnome-cd

EXTRA_DIST = gnome-media.spec.in configure acinclude.m4 COPYING-DOCS \
		sgmldocs.make xmldocs.make omf.make \
		intltool-extract.in intltool-merge.in intltool-update.in

dist-hook: gnome-media.spec
	cp gnome-media.spec $(distdir)

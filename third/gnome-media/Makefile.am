always_built_SUBDIRS = cddb-slave2 profiles

SUBDIRS = po $(always_built_SUBDIRS) \
	  @PROGRAMS_VUMETER@ \
	  @PROGRAMS_GNOME_CD@ \
	  @PROGRAMS_GSTMIXER@ \
	  @PROGRAMS_GRECORD@ \
	  @PROGRAMS_GSTPROPS@

DIST_SUBDIRS = po $(always_built_SUBDIRS) \
	       vu-meter grecord gst-mixer gnome-cd \
               gstreamer-properties

INTLTOOL_BUILT = \
        intltool-extract \
        intltool-merge \
        intltool-update

EXTRA_DIST =	config.rpath gnome-media.spec.in configure \
		acinclude.m4 COPYING-DOCS \
		gnome-media.spec \
		sgmldocs.make xmldocs.make omf.make \
		m4/as-compiler-flag.m4 m4/as-version.m4 \
		$(INTLTOOL_BUILT:=.in)

DISTCLEANFILES = po/.intltool-merge-cache \
                 $(INTLTOOL_BUILT)


distuninstallcheck_listfiles = \
        find -path ./var/scrollkeeper/\* -prune -or -type f -print

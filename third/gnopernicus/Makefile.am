SUBDIRS= po srutil srconf braille speech srlow kbd_mouse magnifier srcore gnopi brlmon schemas help 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gnopernicus-1.0.pc

EXTRA_DIST= \
	    xmldocs.make		\
	    omf.make			\
	    intltool-extract.in     	\
	    intltool-merge.in       	\
	    intltool-update.in      	\
	    gnopernicus-1.0.pc.in   	\
	    gnopernicus-1.0.spec.in 	

install-exec-hook:
	(echo Installation of $(PACKAGE) complete)

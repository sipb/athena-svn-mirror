SUBDIRS = libxklavier doc tests

config_xml_DATA = xfree86.xml
config_xmldir = $(datadir)/$(PACKAGE)

EXTRA_DIST = libxklavier.spec libxklavier.spec.in \
             Doxyfile Doxyfile.in autogen.sh \
             CREDITS libxklavier.pc.in \
             $(config_xml_DATA)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libxklavier.pc

doxydocs: libxklavier/xklavier.h libxklavier/xklavier_config.h
	if test "x$(DO_DOXYGEN)" = "xyes" ; then \
	  doxygen Doxyfile; \
	fi;

all: doxydocs

.PHONY: doxydocs

INCLUDES = -I$(top_srcdir) \
           $(GNOME_CFLAGS) \
           $(GNOME_DESKTOP_CFLAGS) \
           $(LIBXKLAVIER_CFLAGS) \
           $(GNOME_LIBS2_CFLAGS) \
           -I../intl -I$(top_srcdir)/intl \
           -DSYS_PLUGIN_DIR=\"$(libdir)/gswitchit/\" \
           -DG_LOG_DOMAIN=\"GSwitchIt\" \
           -DGNOMELOCALEDIR=\""$(datadir)/locale"\"

noinst_LIBRARIES = libgswitchit.a

libgswitchit_a_SOURCES = gswitchit_config.c \
                          gswitchit_util.c \
                          gnome-startup.c \
                          gswitchit_plugin_manager.c

if INCLUDE_GSWITCHIT_DEVEL

gswitchitincdir = $(includedir)/libgswitchit
gswitchitinc_HEADERS = gswitchit_plugin.h gswitchit_xkb_config.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gswitchit.pc
else

extra_nih = gswitchit_plugin.h gswitchit_xkb_config.h
endif

noinst_HEADERS = gswitchit_applet_config.h \
                 gswitchit_xkb_config_private.h \
                 gswitchit_plugin_manager.h \
                 gswitchit_util.h gnome-startup.h \
                 $(extra_nih)

EXTRA_DIST = gswitchit.pc.in

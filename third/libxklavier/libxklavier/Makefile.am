if XKB_HEADERS_PRESENT
  XKB_HEADERS_PRESENT_CFLAG = -DXKB_HEADERS_PRESENT=1
endif

AM_CFLAGS = -DDATA_DIR=\"$(datadir)/$(PACKAGE)\" -I. -I$(includedir) $(XML_CFLAGS) -I$(x_includes) -I$(top_srcdir) $(XKB_HEADERS_PRESENT_CFLAG)

lib_LTLIBRARIES = libxklavier.la
noinst_HEADERS = xklavier_private.h xklavier_private_xkb.h

xklavierincdir = $(includedir)/libxklavier
xklavierinc_HEADERS = xklavier.h xklavier_config.h

libxklavier_la_SOURCES = xklavier.c xklavier_xkb.c xklavier_evt.c xklavier_util.c xklavier_config.c xklavier_config_xkb.c xklavier_config_i18n.c xklavier_props.c xklavier_dump.c \
                        $(noinst_HEADERS) $(xklavierinc_HEADERS)
libxklavier_la_LDFLAGS = -version-info @VERSION_INFO@ $(XML_LIBS) -lxkbfile -L$(x_libraries) $(LIBICONV)



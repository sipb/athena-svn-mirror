plugin_LTLIBRARIES = libgstkio.la

libgstkio_la_SOURCES = kiosrc.cpp kioreceiver.cpp
libgstkio_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstkio_la_LIBADD = $(KIO_LIBS)

libgstkio_la_CXXFLAGS = $(GST_CFLAGS) $(KIO_CFLAGS)

BUILT_SOURCES = kioreceiver.moc

kioreceiver.moc: kioreceiver.h
	$(KDE_MOC) $(srcdir)/kioreceiver.h > kioreceiver.moc

noinst_HEADERS = kioreceiver.h kiosrc.h

CLEANFILES = $(BUILT_SOURCES)


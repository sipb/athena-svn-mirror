# Disable deprecated kNFS lockers
AM_CFLAGS=-DMOUNT_CMD=\"/bin/false\" -DUMOUNT_CMD=\"/bin/false\"

include_HEADERS = locker.h

lib_LTLIBRARIES = liblocker.la
liblocker_la_SOURCES = \
	afs.c \
	attach.c \
	attachtab.c \
	conf.c \
	detach.c \
	locker.h \
	locker_private.h \
	misc.c \
	miscfs.c \
	mount.c \
	mountpoint.c \
	mul.c \
	nfs.c \
	util.c \
	zephyr.c

liblocker_la_LIBADD = @KRB5_LIBS@ @HESIOD_LIBS@ @ZEPHYR_LIBS@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = liblocker.pc

#AUTOMAKE_OPTIONS = no-dependencies
bin_PROGRAMS = xntpd
INCLUDES = -I$(top_srcdir)/include
# LDADD might need RESLIB and ADJLIB
LDADD =	version.o ../libntp/libntp.a @LIBPARSE@
DISTCLEANFILES = .version version.c
EXTRA_DIST = xntpd.mak
ETAGS_ARGS = Makefile.am

xntpd_SOURCES = map_vme.c ntp_config.c ntp_control.c ntp_io.c ntp_leap.c \
	ntp_loopfilter.c ntp_monitor.c ntp_peer.c ntp_proto.c \
	ntp_refclock.c ntp_request.c ntp_restrict.c ntp_timer.c \
	ntp_unixclock.c ntp_util.c ntp_intres.c ntp_filegen.c ntpd.c \
	refclock_conf.c refclock_chu.c refclock_local.c \
	refclock_pst.c refclock_wwvb.c refclock_mx4200.c \
	refclock_parse.c refclock_as2201.c refclock_bancomm.c \
	refclock_tpro.c refclock_leitch.c refclock_irig.c \
	refclock_msfees.c refclock_trak.c refclock_datum.c \
	refclock_acts.c refclock_heath.c refclock_nmea.c \
	refclock_atom.c refclock_ptbacts.c \
	refclock_usno.c refclock_true.c refclock_hpgps.c \
	refclock_shm.c refclock_gpsvme.c refclock_arbiter.c \
	refclock_arc.c

$(PROGRAMS): $(LDADD)

../libntp/libntp.a:
	cd ../libntp && $(MAKE)

../libparse/libparse.a:
	cd ../libparse && $(MAKE)

version.o: $(xntpd_OBJECTS) ../libntp/libntp.a Makefile
	$(top_srcdir)/scripts/mkversion xntpd @VERSION@
	$(COMPILE) -c version.c

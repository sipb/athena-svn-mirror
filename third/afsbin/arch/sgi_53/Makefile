# $Id: Makefile,v 1.3 1997-03-12 20:59:29 ghudson Exp $

SHELL=/bin/sh
ATHRBINDIR=/bin/athena
VICE=/var/usr/vice/etc
MODULES=${VICE}/sgiload
INITD=/etc/init.d
RC0=/etc/rc0.d
RC2=/etc/rc2.d
WS=/usr/afsws

all:

check:

install:
	-mkdir -p ${DESTDIR}${ATHRBINDIR}
	-mkdir -p ${DESTDIR}${VICE}
	-mkdir -p ${DESTDIR}${MODULES}
	-mkdir -p ${DESTDIR}${INITD}
	-mkdir -p ${DESTDIR}${RC0}
	-mkdir -p ${DESTDIR}${RC2}
	-mkdir -p ${DESTDIR}${WS}/include
	-mkdir -p ${DESTDIR}${WS}/include/afs
	-mkdir -p ${DESTDIR}${WS}/include/rx
	-mkdir -p ${DESTDIR}${WS}/lib
	-mkdir -p ${DESTDIR}${WS}/lib/afs
	install -c -m 555 fs ${DESTDIR}${ATHRBINDIR}
	install -c -m 555 tokens ${DESTDIR}${ATHRBINDIR}
	install -c -m 555 unlog ${DESTDIR}${ATHRBINDIR}
	install -c -m 555 afsd ${DESTDIR}${VICE}
	install -c -m 444 libafs.SP.R4000.nonfs.o ${DESTDIR}${MODULES}
	install -c -m 444 libafs.SP.R4000.o ${DESTDIR}${MODULES}
	install -c -m 555 afs.rc ${DESTDIR}${INITD}/afs
	rm -f ${DESTDIR}${RC0}/K35afs
	ln -s ../init.d/afs ${DESTDIR}${RC0}/K35afs
	rm -f ${DESTDIR}${RC2}/S35afs
	ln -s ../init.d/afs ${DESTDIR}${RC2}/S35afs 
	install -c -m 444 include/andrewOS.h ${DESTDIR}${WS}/include
	install -c -m 444 include/des.h ${DESTDIR}${WS}/include
	install -c -m 444 include/des_conf.h ${DESTDIR}${WS}/include
	install -c -m 444 include/des_odd.h ${DESTDIR}${WS}/include
	install -c -m 444 include/itc.h ${DESTDIR}${WS}/include
	install -c -m 444 include/lock.h ${DESTDIR}${WS}/include
	install -c -m 444 include/lwp.h ${DESTDIR}${WS}/include
	install -c -m 444 include/mit-cpyright.h ${DESTDIR}${WS}/include
	install -c -m 444 include/potpourri.h ${DESTDIR}${WS}/include
	install -c -m 444 include/preempt.h ${DESTDIR}${WS}/include
	install -c -m 444 include/pthread.h ${DESTDIR}${WS}/include
	install -c -m 444 include/timer.h ${DESTDIR}${WS}/include
	install -c -m 444 include/ubik.h ${DESTDIR}${WS}/include
	install -c -m 444 include/ubik_int.h ${DESTDIR}${WS}/include
	install -c -m 444 include/afs/acl.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/afs.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/afs_stats.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/afs_sysnames.h \
		${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/afscbint.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/afsint.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/assert.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/audit.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/auth.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/auxinode.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/bnode.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/bosint.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/bubasics.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/budb.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/budb_client.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/budb_errs.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/bumon.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/butc.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/butm.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/cellconfig.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/cmd.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/cnvldb.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/com_err.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/debug.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/dir.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/errors.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/exporter.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/fs_stats.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/fsprobe.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/fssync.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/gtxX11win.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/gtxcurseswin.h \
		${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/gtxdumbwin.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/gtxframe.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/gtxinput.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/gtxkeymap.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/gtxlightobj.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/gtxobjdict.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/gtxobjects.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/gtxtextcb.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/gtxtextobj.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/gtxwindows.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/icl.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/kaport.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/kauth.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/kautils.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/keys.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/ktime.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/mit-sipb-cr.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/nfs.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/nfsclient.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/packages.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/param.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/partition.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/permit_xprt.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/prclient.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/prerror.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/print.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/prs_fs.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/prserver.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/ptclient.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/pterror.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/ptint.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/ptserver.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/remote.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/rxgen_consts.h \
		${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/stds.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/tcdata.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/venus.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/vice.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/viceinode.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/vl_opcodes.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/vldb.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/vldbint.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/vlserver.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/vnode.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/voldefs.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/volint.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/volser.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/volume.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/xstat_cm.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/afs/xstat_fs.h ${DESTDIR}${WS}/include/afs
	install -c -m 444 include/rx/rx.h ${DESTDIR}${WS}/include/rx
	install -c -m 444 include/rx/rx_clock.h ${DESTDIR}${WS}/include/rx
	install -c -m 444 include/rx/rx_event.h ${DESTDIR}${WS}/include/rx
	install -c -m 444 include/rx/rx_globals.h ${DESTDIR}${WS}/include/rx
	install -c -m 444 include/rx/rx_machdep.h ${DESTDIR}${WS}/include/rx
	install -c -m 444 include/rx/rx_multi.h ${DESTDIR}${WS}/include/rx
	install -c -m 444 include/rx/rx_null.h ${DESTDIR}${WS}/include/rx
	install -c -m 444 include/rx/rx_packet.h ${DESTDIR}${WS}/include/rx
	install -c -m 444 include/rx/rx_queue.h ${DESTDIR}${WS}/include/rx
	install -c -m 444 include/rx/rx_user.h ${DESTDIR}${WS}/include/rx
	install -c -m 444 include/rx/rxkad.h ${DESTDIR}${WS}/include/rx
	install -c -m 444 include/rx/xdr.h ${DESTDIR}${WS}/include/rx
	install -c -m 444 lib/libdes.a ${DESTDIR}${WS}/lib
	install -c -m 444 lib/liblwp.a ${DESTDIR}${WS}/lib
	install -c -m 444 lib/librx.a ${DESTDIR}${WS}/lib
	install -c -m 444 lib/librxkad.a ${DESTDIR}${WS}/lib
	install -c -m 444 lib/libubik.a ${DESTDIR}${WS}/lib
	install -c -m 444 lib/afs/libacl.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libafsint.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libaudit.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libauth.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libauth.krb.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libbos.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libbubasics.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libbudb.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libbutm.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libbxdb.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libcmd.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libcom_err.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libdir.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libfsprobe.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libgtx.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libkauth.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libkauth.krb.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libnull.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libprot.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libsys.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libvldb.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libvolser.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libxstat_cm.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/libxstat_fs.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/util.a ${DESTDIR}${WS}/lib/afs
	install -c -m 444 lib/afs/vlib.a ${DESTDIR}${WS}/lib/afs
	rm -f ${DESTDIR}${WS}/lib/afs/libtermlib.a
	ln -s /usr/lib/libtermlib.a ${DESTDIR}${WS}/lib/afs/libtermlib.a

clean:

distclean:

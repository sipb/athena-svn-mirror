# $Id: Makefile,v 1.5 1999-12-24 16:01:16 ghudson Exp $

SHELL=/bin/sh

ATHRBINDIR=/bin/athena
VICE=/usr/vice/etc
AFSWS=/usr/afsws

all:

check:

install:
	-mkdir -p $(DESTDIR)$(ATHRBINDIR)
	-mkdir -p $(DESTDIR)$(VICE)/modload
	-mkdir -p $(DESTDIR)$(AFSWS)/lib/afs
	-mkdir -p $(DESTDIR)$(AFSWS)/include/afs
	-mkdir -p $(DESTDIR)$(AFSWS)/include/rx
	-mkdir -p $(DESTDIR)/etc/rc.d/init.d
	-mkdir -p $(DESTDIR)/etc/sysconfig
	-mkdir -p $(DESTDIR)/usr/vice/cache
	install -c -m 444 libafs-2.2.0.mp.o $(DESTDIR)$(VICE)/modload
	install -c -m 444 libafs-2.2.1.mp.o $(DESTDIR)$(VICE)/modload
	install -c -m 444 libafs-2.2.2.mp.o $(DESTDIR)$(VICE)/modload
	install -c -m 444 libafs-2.2.3.mp.o $(DESTDIR)$(VICE)/modload
	install -c -m 444 libafs-2.2.5-15.mp.o $(DESTDIR)$(VICE)/modload
	install -c -m 444 libafs-2.2.10.mp.o $(DESTDIR)$(VICE)/modload
	install -c -m 444 libafs-2.2.12.mp.o $(DESTDIR)$(VICE)/modload
	install -c -m 444 libafs-2.2.12-20smp.mp.o $(DESTDIR)$(VICE)/modload
	install -c -m 444 libafs-2.2.0.o $(DESTDIR)$(VICE)/modload
	install -c -m 444 libafs-2.2.1.o $(DESTDIR)$(VICE)/modload
	install -c -m 444 libafs-2.2.2.o $(DESTDIR)$(VICE)/modload
	install -c -m 444 libafs-2.2.3.o $(DESTDIR)$(VICE)/modload
	install -c -m 444 libafs-2.2.5-15.o $(DESTDIR)$(VICE)/modload
	install -c -m 444 libafs-2.2.10.o $(DESTDIR)$(VICE)/modload
	install -c -m 444 libafs-2.2.12.o $(DESTDIR)$(VICE)/modload
	install -c -m 444 libafs-2.2.12-20.o $(DESTDIR)$(VICE)/modload
	install -c -m 555 fs $(DESTDIR)$(ATHRBINDIR)
	install -c -m 555 tokens $(DESTDIR)$(ATHRBINDIR)
	install -c -m 555 unlog $(DESTDIR)$(ATHRBINDIR)
	install -c -m 555 afsd $(DESTDIR)$(VICE)
	install -c -m 555 ThisCell $(DESTDIR)$(VICE)
	install -c -m 555 CellServDB $(DESTDIR)$(VICE)
	install -c -m 555 cacheinfo $(DESTDIR)$(VICE)
	install -c -m 444 include/afs/acl.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_Admin.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_AdminBosErrors.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_AdminCfgErrors.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_AdminClientErrors.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_AdminCommonErrors.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_AdminErrors.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_AdminKasErrors.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_AdminMiscErrors.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_AdminPtsErrors.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_AdminUtilErrors.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_AdminVosErrors.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_args.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_atomlist.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_bosAdmin.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_cfgAdmin.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_clientAdmin.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_kasAdmin.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_lhash.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_ptsAdmin.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_stats.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_sysnames.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_utilAdmin.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afs_vosAdmin.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afscbint.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afsint.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afssyscalls.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/afsutil.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/assert.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/audit.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/auth.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/bnode.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/bosint.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/bubasics.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/budb.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/budb_client.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/budb_errs.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/bumon.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/butc.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/butm.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/cellconfig.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/cmd.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/cnvldb.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/com_err.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/debug.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/dir.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/dirpath.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/error_table.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/errors.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/exporter.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/fileutil.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/fs_stats.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/fsprobe.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/fssync.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/gtxX11win.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/gtxcurseswin.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/gtxdumbwin.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/gtxframe.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/gtxinput.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/gtxkeymap.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/gtxlightobj.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/gtxobjdict.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/gtxobjects.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/gtxtextcb.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/gtxtextobj.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/gtxwindows.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/icl.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/ihandle.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/kaport.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/kauth.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/kautils.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/keys.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/ktime.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/mit-sipb-cr.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/namei_ops.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/netutils.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/nfs.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/nfsclient.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/osi_inode.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/osi_vfs.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/packages.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/param.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/partition.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/permit_xprt.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/prclient.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/prerror.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/print.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/procmgmt.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/prs_fs.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/prserver.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/ptclient.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/pterror.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/pthread_glock.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/pthread_nosigs.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/ptint.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/ptserver.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/remote.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/rxgen_consts.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/stds.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/tcdata.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/usd.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/venus.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/vice.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/viceinode.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/vl_opcodes.h \
		$(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/vldbint.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/vlserver.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/vnode.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/voldefs.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/volint.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/volser.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/volume.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/xstat_cm.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/afs/xstat_fs.h $(DESTDIR)$(AFSWS)/include/afs
	install -c -m 444 include/des.h $(DESTDIR)$(AFSWS)/include
	install -c -m 444 include/des_conf.h $(DESTDIR)$(AFSWS)/include
	install -c -m 444 include/des_odd.h $(DESTDIR)$(AFSWS)/include
	install -c -m 444 include/itc.h $(DESTDIR)$(AFSWS)/include
	install -c -m 444 include/lock.h $(DESTDIR)$(AFSWS)/include
	install -c -m 444 include/lwp.h $(DESTDIR)$(AFSWS)/include
	install -c -m 444 include/mit-cpyright.h $(DESTDIR)$(AFSWS)/include
	install -c -m 444 include/potpourri.h $(DESTDIR)$(AFSWS)/include
	install -c -m 444 include/preempt.h $(DESTDIR)$(AFSWS)/include
	install -c -m 444 include/rx/rx.h $(DESTDIR)$(AFSWS)/include/rx
	install -c -m 444 include/rx/rx_clock.h $(DESTDIR)$(AFSWS)/include/rx
	install -c -m 444 include/rx/rx_event.h $(DESTDIR)$(AFSWS)/include/rx
	install -c -m 444 include/rx/rx_globals.h $(DESTDIR)$(AFSWS)/include/rx
	install -c -m 444 include/rx/rx_lwp.h $(DESTDIR)$(AFSWS)/include/rx
	install -c -m 444 include/rx/rx_misc.h $(DESTDIR)$(AFSWS)/include/rx
	install -c -m 444 include/rx/rx_multi.h $(DESTDIR)$(AFSWS)/include/rx
	install -c -m 444 include/rx/rx_null.h $(DESTDIR)$(AFSWS)/include/rx
	install -c -m 444 include/rx/rx_packet.h $(DESTDIR)$(AFSWS)/include/rx
	install -c -m 444 include/rx/rx_pthread.h $(DESTDIR)$(AFSWS)/include/rx
	install -c -m 444 include/rx/rx_queue.h $(DESTDIR)$(AFSWS)/include/rx
	install -c -m 444 include/rx/rx_user.h $(DESTDIR)$(AFSWS)/include/rx
	install -c -m 444 include/rx/rxkad.h $(DESTDIR)$(AFSWS)/include/rx
	install -c -m 444 include/rx/rxstat.h $(DESTDIR)$(AFSWS)/include/rx
	install -c -m 444 include/rx/xdr.h $(DESTDIR)$(AFSWS)/include/rx
	install -c -m 444 include/timer.h $(DESTDIR)$(AFSWS)/include
	install -c -m 444 include/ubik.h $(DESTDIR)$(AFSWS)/include
	install -c -m 444 include/ubik_int.h $(DESTDIR)$(AFSWS)/include
	install -c -m 444 lib/afs/libacl.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libafsadminutil.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libafsint.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libaudit.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libauth.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libauth.krb.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libbos.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libbosadmin.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libbubasics.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libbudb.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libbutm.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libbxdb.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libcfgadmin.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libclientadmin.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libcmd.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libcom_err.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libdir.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libfsprobe.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libgtx.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libkasadmin.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libkauth.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libkauth.krb.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libnull.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libprocmgmt.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libprot.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libptsadmin.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libsys.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libusd.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libvldb.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libvolser.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libvosadmin.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libxstat_cm.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/libxstat_fs.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/util.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/afs/vlib.a $(DESTDIR)$(AFSWS)/lib/afs
	install -c -m 444 lib/libafsauthent.a $(DESTDIR)$(AFSWS)/lib
	install -c -m 444 lib/libafsrpc.a $(DESTDIR)$(AFSWS)/lib
	install -c -m 444 lib/libdes.a $(DESTDIR)$(AFSWS)/lib
	install -c -m 444 lib/liblwp.a $(DESTDIR)$(AFSWS)/lib
	install -c -m 444 lib/librx.a $(DESTDIR)$(AFSWS)/lib
	install -c -m 444 lib/librxkad.a $(DESTDIR)$(AFSWS)/lib
	install -c -m 444 lib/librxstat.a $(DESTDIR)$(AFSWS)/lib
	install -c -m 444 lib/libubik.a $(DESTDIR)$(AFSWS)/lib
	install -c -m 444 lib/pam_afs.krb.so.1 $(DESTDIR)$(AFSWS)/lib
	install -c -m 444 lib/pam_afs.so.1 $(DESTDIR)$(AFSWS)/lib
	rm -f $(DESTDIR)$(AFSWS)/lib/afs/libtermlib.a
	ln -s libnull.a $(DESTDIR)$(AFSWS)/lib/afs/libtermlib.a
	install -c -m 555 afs.rc $(DESTDIR)/etc/rc.d/init.d/afs
	install -c -m 555 afs.conf $(DESTDIR)/etc/sysconfig/afs

clean:

distclean:


# Id: Makefile.in,v 11.175 2002/08/29 14:22:20 margo Exp 

srcdir=	../dist/..
builddir=.

##################################################
# Installation directories and permissions.
##################################################
prefix=	/usr
exec_prefix=${prefix}
bindir=	${exec_prefix}/bin
includedir=${prefix}/include
libdir=	${exec_prefix}/lib
docdir=	$(prefix)/docs

dmode=	755
emode=	555
fmode=	444

transform=s,x,x,

##################################################
# Paths for standard user-level commands.
##################################################
SHELL=	/bin/sh
ar=	ar
chmod=	chmod
cp=	cp
ln=	ln
mkdir=	mkdir
ranlib=	/usr/bin/ranlib
rm=	rm
rpm=	
strip=	/usr/bin/strip

##################################################
# General library information.
##################################################
DEF_LIB=	$(libso_target)
DEF_LIB_CXX=	
INSTALLER=	$(LIBTOOL) --mode=install cp -p
LIBTOOL=	$(SHELL) ./libtool

POSTLINK=	$(LIBTOOL) --mode=execute true
SOLINK=		$(LIBTOOL) --mode=link cc -avoid-version
SOFLAGS=	-rpath $(libdir)
SOMAJOR=	4
SOVERSION=	4.1

##################################################
# C API.
##################################################
CPPFLAGS=	-I$(builddir) -I$(srcdir) -I$(srcdir)/dbinc  -D_GNU_SOURCE -D_REENTRANT
CFLAGS=	 	-c $(CPPFLAGS) -O2
CC=		$(LIBTOOL) --mode=compile cc
CCLINK=		$(LIBTOOL) --mode=link cc

LDFLAGS=	
LIBS=		 -lpthread
LIBSO_LIBS=	

libdb=		libdb.a
libso_base=	libdb
libso=		$(libso_base)-$(SOVERSION).so
libso_static=	$(libso_base)-$(SOVERSION).a
libso_target=	$(libso_base)-$(SOVERSION).la
libso_default=	$(libso_base).so
libso_major=	$(libso_base)-$(SOMAJOR).so

##################################################
# C++ API.
#
# C++ support is optional, and can be built with static or shared libraries.
##################################################
CXXFLAGS=	-c $(CPPFLAGS) -O2
CXX=		$(LIBTOOL) --mode=compile nocxx
CXXLINK=	$(LIBTOOL) --mode=link nocxx
XSOLINK=	$(LIBTOOL) --mode=link nocxx -avoid-version
LIBXSO_LIBS=	

libcxx=		libdb_cxx.a
libxso_base=	libdb_cxx
libxso=		$(libxso_base)-$(SOVERSION).so
libxso_static=	$(libxso_base)-$(SOVERSION).a
libxso_target=	$(libxso_base)-$(SOVERSION).la
libxso_default=	$(libxso_base).so
libxso_major=	$(libxso_base)-$(SOMAJOR).so

##################################################
# Java API.
#
# Java support is optional and requires shared librarires.
##################################################
CLASSPATH=	$(JAVA_CLASSTOP)
LIBJSO_LIBS=	

JAR=		
JAVAC=		env CLASSPATH="$(CLASSPATH)" nojavac
JAVACFLAGS=	
JAVA_CLASSTOP=	./classes
JAVA_RPCCLASSES=./classes.rpc
JAVA_SRCDIR=	$(srcdir)/java/src
JAVA_DBREL=	com/sleepycat/db
JAVA_EXREL=	com/sleepycat/examples
JAVA_RPCREL=	com/sleepycat/db/rpcserver
JAVA_DBDIR=	$(JAVA_SRCDIR)/$(JAVA_DBREL)
JAVA_EXDIR=	$(JAVA_SRCDIR)/$(JAVA_EXREL)
JAVA_RPCDIR=	$(srcdir)/rpc_server/java

libj_jarfile=	db.jar
libj_exjarfile=	dbexamples.jar
rpc_jarfile=	dbsvc.jar
libjso_base=	libdb_java
libjso=		$(libjso_base)-$(SOVERSION).so
libjso_static=	$(libjso_base)-$(SOVERSION).a
libjso_target=	$(libjso_base)-$(SOVERSION).la
libjso_default=	$(libjso_base).so
libjso_major=	$(libjso_base)-$(SOMAJOR).so
libjso_g=	$(libjso_base)-$(SOVERSION)_g.so

##################################################
# TCL API.
#
# Tcl support is optional and requires shared libraries.
##################################################
TCFLAGS=	
LIBTSO_LIBS=	
libtso_base=	libdb_tcl
libtso=		$(libtso_base)-$(SOVERSION).so
libtso_static=	$(libtso_base)-$(SOVERSION).a
libtso_target=	$(libtso_base)-$(SOVERSION).la
libtso_default=	$(libtso_base).so
libtso_major=	$(libtso_base)-$(SOMAJOR).so

##################################################
# db_dump185 UTILITY
#
# The db_dump185 application should be compiled using the system's db.h file
# (which should be a DB 1.85/1.86 include file), and the system's 1.85/1.86
# object library.  To include the right db.h, don't include -I$(builddir) on
# the compile line.  You may also need to add a local include directory and
# local libraries, for example.  Do that by adding -I options to the DB185INC
# line, and -l options to the DB185LIB line.
##################################################
DB185INC=	-c -O2 -I$(srcdir)  -D_GNU_SOURCE -D_REENTRANT
DB185LIB=

##################################################
# NOTHING BELOW THIS LINE SHOULD EVER NEED TO BE MODIFIED.
##################################################

##################################################
# Object and utility lists.
##################################################
C_OBJS= mut_pthread.lo aes_method.lo crypto.lo mt19937db.lo rijndael-alg-fst.lo rijndael-api-fst.lo    \
	bt_compare.lo bt_conv.lo bt_curadj.lo bt_cursor.lo bt_delete.lo \
	bt_method.lo bt_open.lo bt_put.lo bt_rec.lo bt_reclaim.lo \
	bt_recno.lo bt_rsearch.lo bt_search.lo bt_split.lo bt_stat.lo \
	bt_upgrade.lo bt_verify.lo btree_auto.lo crdel_auto.lo \
	crdel_rec.lo db.lo db_am.lo db_auto.lo db_byteorder.lo db_cam.lo \
	db_conv.lo db_dispatch.lo db_dup.lo db_err.lo db_getlong.lo \
	db_idspace.lo db_iface.lo db_join.lo db_log2.lo db_meta.lo \
	db_method.lo db_open.lo db_overflow.lo db_pr.lo db_rec.lo \
	db_reclaim.lo db_rename.lo db_remove.lo db_ret.lo db_salloc.lo \
	db_shash.lo db_truncate.lo db_upg.lo db_upg_opd.lo db_vrfy.lo \
	db_vrfyutil.lo dbm.lo dbreg.lo dbreg_auto.lo dbreg_rec.lo \
	dbreg_util.lo env_file.lo env_method.lo env_open.lo env_recover.lo \
	env_region.lo fileops_auto.lo fop_basic.lo fop_rec.lo \
	fop_util.lo hash.lo hash_auto.lo hash_conv.lo hash_dup.lo \
	hash_func.lo hash_meta.lo hash_method.lo hash_open.lo \
	hash_page.lo hash_rec.lo hash_reclaim.lo hash_stat.lo \
	hash_upgrade.lo hash_verify.lo hmac.lo hsearch.lo lock.lo \
	lock_deadlock.lo lock_method.lo lock_region.lo lock_stat.lo \
	lock_util.lo log.lo log_archive.lo log_compare.lo log_get.lo \
	log_method.lo log_put.lo mp_alloc.lo mp_bh.lo mp_fget.lo \
	mp_fopen.lo mp_fput.lo mp_fset.lo mp_method.lo mp_region.lo \
	mp_register.lo mp_stat.lo mp_sync.lo mp_trickle.lo mutex.lo \
	os_abs.lo os_alloc.lo os_clock.lo os_config.lo os_dir.lo \
	os_errno.lo os_fid.lo os_fsync.lo os_handle.lo os_id.lo \
	os_map.lo os_method.lo os_oflags.lo os_open.lo os_region.lo \
	os_rename.lo os_root.lo os_rpath.lo os_rw.lo os_seek.lo \
	os_sleep.lo os_spin.lo os_stat.lo os_tmpdir.lo os_unlink.lo \
	qam.lo qam_auto.lo qam_conv.lo qam_files.lo qam_method.lo \
	qam_open.lo qam_rec.lo qam_stat.lo qam_upgrade.lo qam_verify.lo \
	rep_method.lo rep_record.lo rep_region.lo rep_util.lo sha1.lo \
	txn.lo txn_auto.lo txn_method.lo txn_rec.lo txn_recover.lo \
	txn_region.lo txn_stat.lo txn_util.lo xa.lo xa_db.lo xa_map.lo

CXX_OBJS=\
	cxx_db.lo cxx_dbc.lo cxx_dbt.lo cxx_env.lo cxx_except.lo \
	cxx_lock.lo cxx_logc.lo cxx_mpool.lo cxx_txn.lo

JAVA_OBJS=\
	java_Db.lo java_DbEnv.lo java_DbLock.lo java_DbLogc.lo \
	java_DbLsn.lo java_DbTxn.lo java_DbUtil.lo java_Dbc.lo \
	java_Dbt.lo \
	java_info.lo java_locked.lo java_util.lo java_stat_auto.lo

JAVA_DBSRCS=\
	$(JAVA_DBDIR)/Db.java $(JAVA_DBDIR)/DbAppendRecno.java \
	$(JAVA_DBDIR)/DbAppDispatch.java \
	$(JAVA_DBDIR)/DbBtreeCompare.java $(JAVA_DBDIR)/DbBtreePrefix.java \
	$(JAVA_DBDIR)/DbBtreeStat.java $(JAVA_DBDIR)/DbClient.java \
	$(JAVA_DBDIR)/DbConstants.java $(JAVA_DBDIR)/DbDeadlockException.java \
	$(JAVA_DBDIR)/DbDupCompare.java $(JAVA_DBDIR)/DbEnv.java \
	$(JAVA_DBDIR)/DbEnvFeedback.java $(JAVA_DBDIR)/DbErrcall.java \
	$(JAVA_DBDIR)/DbException.java $(JAVA_DBDIR)/DbFeedback.java \
	$(JAVA_DBDIR)/DbHash.java $(JAVA_DBDIR)/DbHashStat.java \
	$(JAVA_DBDIR)/DbKeyRange.java $(JAVA_DBDIR)/DbLock.java \
	$(JAVA_DBDIR)/DbLockNotGrantedException.java \
	$(JAVA_DBDIR)/DbLockRequest.java $(JAVA_DBDIR)/DbLockStat.java \
	$(JAVA_DBDIR)/DbLogc.java $(JAVA_DBDIR)/DbLogStat.java \
	$(JAVA_DBDIR)/DbLsn.java $(JAVA_DBDIR)/DbMemoryException.java \
	$(JAVA_DBDIR)/DbMpoolFStat.java $(JAVA_DBDIR)/DbMpoolStat.java \
	$(JAVA_DBDIR)/DbMultipleDataIterator.java \
	$(JAVA_DBDIR)/DbMultipleIterator.java \
	$(JAVA_DBDIR)/DbMultipleKeyDataIterator.java \
	$(JAVA_DBDIR)/DbMultipleRecnoDataIterator.java \
	$(JAVA_DBDIR)/DbOutputStreamErrcall.java \
	$(JAVA_DBDIR)/DbPreplist.java $(JAVA_DBDIR)/DbQueueStat.java \
	$(JAVA_DBDIR)/DbRepStat.java $(JAVA_DBDIR)/DbRepTransport.java \
	$(JAVA_DBDIR)/DbRunRecoveryException.java \
	$(JAVA_DBDIR)/DbSecondaryKeyCreate.java $(JAVA_DBDIR)/DbTxn.java \
	$(JAVA_DBDIR)/DbTxnStat.java \
	$(JAVA_DBDIR)/DbUtil.java $(JAVA_DBDIR)/Dbc.java $(JAVA_DBDIR)/Dbt.java

JAVA_EXSRCS=\
	$(JAVA_EXDIR)/AccessExample.java \
	$(JAVA_EXDIR)/BtRecExample.java \
	$(JAVA_EXDIR)/BulkAccessExample.java \
	$(JAVA_EXDIR)/EnvExample.java \
	$(JAVA_EXDIR)/LockExample.java \
	$(JAVA_EXDIR)/TpcbExample.java

TCL_OBJS=\
	tcl_compat.lo tcl_db.lo tcl_db_pkg.lo tcl_dbcursor.lo tcl_env.lo \
	tcl_internal.lo tcl_lock.lo tcl_log.lo tcl_mp.lo tcl_rep.lo \
	tcl_txn.lo tcl_util.lo

RPC_CLIENT_OBJS=\
	client.lo db_server_clnt.lo db_server_xdr.lo gen_client.lo \
	gen_client_ret.lo

RPC_SRV_OBJS=\
	db_server_proc.lo db_server_svc.lo db_server_util.lo \
	gen_db_server.lo

RPC_CXXSRV_OBJS=\
	db_server_cxxproc.lo db_server_cxxutil.lo db_server_svc.lo \
	gen_db_server.lo

RPC_JAVASRV_SRCS=\
	$(JAVA_RPCDIR)/DbDispatcher.java \
	$(JAVA_RPCDIR)/DbServer.java \
	$(JAVA_RPCDIR)/FreeList.java \
	$(JAVA_RPCDIR)/LocalIterator.java \
	$(JAVA_RPCDIR)/RpcDb.java \
	$(JAVA_RPCDIR)/RpcDbEnv.java \
	$(JAVA_RPCDIR)/RpcDbTxn.java \
	$(JAVA_RPCDIR)/RpcDbc.java \
	$(JAVA_RPCDIR)/Timer.java \
	$(JAVA_RPCDIR)/gen/DbServerStub.java \
	$(JAVA_RPCDIR)/gen/__db_associate_msg.java \
	$(JAVA_RPCDIR)/gen/__db_associate_reply.java \
	$(JAVA_RPCDIR)/gen/__db_bt_maxkey_msg.java \
	$(JAVA_RPCDIR)/gen/__db_bt_maxkey_reply.java \
	$(JAVA_RPCDIR)/gen/__db_bt_minkey_msg.java \
	$(JAVA_RPCDIR)/gen/__db_bt_minkey_reply.java \
	$(JAVA_RPCDIR)/gen/__db_close_msg.java \
	$(JAVA_RPCDIR)/gen/__db_close_reply.java \
	$(JAVA_RPCDIR)/gen/__db_create_msg.java \
	$(JAVA_RPCDIR)/gen/__db_create_reply.java \
	$(JAVA_RPCDIR)/gen/__db_cursor_msg.java \
	$(JAVA_RPCDIR)/gen/__db_cursor_reply.java \
	$(JAVA_RPCDIR)/gen/__db_del_msg.java \
	$(JAVA_RPCDIR)/gen/__db_del_reply.java \
	$(JAVA_RPCDIR)/gen/__db_encrypt_msg.java \
	$(JAVA_RPCDIR)/gen/__db_encrypt_reply.java \
	$(JAVA_RPCDIR)/gen/__db_extentsize_msg.java \
	$(JAVA_RPCDIR)/gen/__db_extentsize_reply.java \
	$(JAVA_RPCDIR)/gen/__db_flags_msg.java \
	$(JAVA_RPCDIR)/gen/__db_flags_reply.java \
	$(JAVA_RPCDIR)/gen/__db_get_msg.java \
	$(JAVA_RPCDIR)/gen/__db_get_reply.java \
	$(JAVA_RPCDIR)/gen/__db_h_ffactor_msg.java \
	$(JAVA_RPCDIR)/gen/__db_h_ffactor_reply.java \
	$(JAVA_RPCDIR)/gen/__db_h_nelem_msg.java \
	$(JAVA_RPCDIR)/gen/__db_h_nelem_reply.java \
	$(JAVA_RPCDIR)/gen/__db_join_msg.java \
	$(JAVA_RPCDIR)/gen/__db_join_reply.java \
	$(JAVA_RPCDIR)/gen/__db_key_range_msg.java \
	$(JAVA_RPCDIR)/gen/__db_key_range_reply.java \
	$(JAVA_RPCDIR)/gen/__db_lorder_msg.java \
	$(JAVA_RPCDIR)/gen/__db_lorder_reply.java \
	$(JAVA_RPCDIR)/gen/__db_open_msg.java \
	$(JAVA_RPCDIR)/gen/__db_open_reply.java \
	$(JAVA_RPCDIR)/gen/__db_pagesize_msg.java \
	$(JAVA_RPCDIR)/gen/__db_pagesize_reply.java \
	$(JAVA_RPCDIR)/gen/__db_pget_msg.java \
	$(JAVA_RPCDIR)/gen/__db_pget_reply.java \
	$(JAVA_RPCDIR)/gen/__db_put_msg.java \
	$(JAVA_RPCDIR)/gen/__db_put_reply.java \
	$(JAVA_RPCDIR)/gen/__db_re_delim_msg.java \
	$(JAVA_RPCDIR)/gen/__db_re_delim_reply.java \
	$(JAVA_RPCDIR)/gen/__db_re_len_msg.java \
	$(JAVA_RPCDIR)/gen/__db_re_len_reply.java \
	$(JAVA_RPCDIR)/gen/__db_re_pad_msg.java \
	$(JAVA_RPCDIR)/gen/__db_re_pad_reply.java \
	$(JAVA_RPCDIR)/gen/__db_remove_msg.java \
	$(JAVA_RPCDIR)/gen/__db_remove_reply.java \
	$(JAVA_RPCDIR)/gen/__db_rename_msg.java \
	$(JAVA_RPCDIR)/gen/__db_rename_reply.java \
	$(JAVA_RPCDIR)/gen/__db_stat_msg.java \
	$(JAVA_RPCDIR)/gen/__db_stat_reply.java \
	$(JAVA_RPCDIR)/gen/__db_sync_msg.java \
	$(JAVA_RPCDIR)/gen/__db_sync_reply.java \
	$(JAVA_RPCDIR)/gen/__db_truncate_msg.java \
	$(JAVA_RPCDIR)/gen/__db_truncate_reply.java \
	$(JAVA_RPCDIR)/gen/__dbc_close_msg.java \
	$(JAVA_RPCDIR)/gen/__dbc_close_reply.java \
	$(JAVA_RPCDIR)/gen/__dbc_count_msg.java \
	$(JAVA_RPCDIR)/gen/__dbc_count_reply.java \
	$(JAVA_RPCDIR)/gen/__dbc_del_msg.java \
	$(JAVA_RPCDIR)/gen/__dbc_del_reply.java \
	$(JAVA_RPCDIR)/gen/__dbc_dup_msg.java \
	$(JAVA_RPCDIR)/gen/__dbc_dup_reply.java \
	$(JAVA_RPCDIR)/gen/__dbc_get_msg.java \
	$(JAVA_RPCDIR)/gen/__dbc_get_reply.java \
	$(JAVA_RPCDIR)/gen/__dbc_pget_msg.java \
	$(JAVA_RPCDIR)/gen/__dbc_pget_reply.java \
	$(JAVA_RPCDIR)/gen/__dbc_put_msg.java \
	$(JAVA_RPCDIR)/gen/__dbc_put_reply.java \
	$(JAVA_RPCDIR)/gen/__env_cachesize_msg.java \
	$(JAVA_RPCDIR)/gen/__env_cachesize_reply.java \
	$(JAVA_RPCDIR)/gen/__env_close_msg.java \
	$(JAVA_RPCDIR)/gen/__env_close_reply.java \
	$(JAVA_RPCDIR)/gen/__env_create_msg.java \
	$(JAVA_RPCDIR)/gen/__env_create_reply.java \
	$(JAVA_RPCDIR)/gen/__env_dbremove_msg.java \
	$(JAVA_RPCDIR)/gen/__env_dbremove_reply.java \
	$(JAVA_RPCDIR)/gen/__env_dbrename_msg.java \
	$(JAVA_RPCDIR)/gen/__env_dbrename_reply.java \
	$(JAVA_RPCDIR)/gen/__env_encrypt_msg.java \
	$(JAVA_RPCDIR)/gen/__env_encrypt_reply.java \
	$(JAVA_RPCDIR)/gen/__env_flags_msg.java \
	$(JAVA_RPCDIR)/gen/__env_flags_reply.java \
	$(JAVA_RPCDIR)/gen/__env_open_msg.java \
	$(JAVA_RPCDIR)/gen/__env_open_reply.java \
	$(JAVA_RPCDIR)/gen/__env_remove_msg.java \
	$(JAVA_RPCDIR)/gen/__env_remove_reply.java \
	$(JAVA_RPCDIR)/gen/__txn_abort_msg.java \
	$(JAVA_RPCDIR)/gen/__txn_abort_reply.java \
	$(JAVA_RPCDIR)/gen/__txn_begin_msg.java \
	$(JAVA_RPCDIR)/gen/__txn_begin_reply.java \
	$(JAVA_RPCDIR)/gen/__txn_commit_msg.java \
	$(JAVA_RPCDIR)/gen/__txn_commit_reply.java \
	$(JAVA_RPCDIR)/gen/__txn_discard_msg.java \
	$(JAVA_RPCDIR)/gen/__txn_discard_reply.java \
	$(JAVA_RPCDIR)/gen/__txn_prepare_msg.java \
	$(JAVA_RPCDIR)/gen/__txn_prepare_reply.java \
	$(JAVA_RPCDIR)/gen/__txn_recover_msg.java \
	$(JAVA_RPCDIR)/gen/__txn_recover_reply.java \
	$(JAVA_RPCDIR)/gen/db_server.java

UTIL_PROGS=\
	 \
	db_archive db_checkpoint db_deadlock \
	db_dump db_load db_printlog db_recover db_stat db_upgrade db_verify

##################################################
# List of files installed into the library directory.
##################################################
LIB_INSTALL_FILE_LIST=\
	$(libdb) \
	$(libso) \
	$(libso_default) \
	$(libso_major) \
	$(libso_static) \
	$(libso_target) \
	$(libcxx) \
	$(libxso) \
	$(libxso_default) \
	$(libxso_major) \
	$(libxso_static) \
	$(libxso_target) \
	$(libtso) \
	$(libtso_default) \
	$(libtso_major) \
	$(libtso_static) \
	$(libtso_target) \
	$(libjso) \
	$(libjso_default) \
	$(libjso_g) \
	$(libjso_major) \
	$(libjso_static) \
	$(libjso_target) \
	$(libj_exjarfile) \
	$(libj_jarfile)

##################################################
# We're building a standard library or a RPM file hierarchy, potentially
# for Embedix.  Note: "all" must be the first target in the Makefile.
##################################################
all: library_build

install-strip install: all library_install

##################################################
# Library and standard utilities build.
##################################################
library_build: $(libso_target)  $(UTIL_PROGS)

$(libdb): $(C_OBJS)
	$(ar) cr $@ $(C_OBJS)
	test ! -f $(ranlib) || $(ranlib) $@

$(libcxx): $(CXX_OBJS) $(C_OBJS)
	$(ar) cr $@ $(CXX_OBJS) $(C_OBJS)
	test ! -f $(ranlib) || $(ranlib) $@

$(libso_target): $(C_OBJS)
	$(SOLINK) $(SOFLAGS) $(LDFLAGS) -o $@ $(C_OBJS) $(LIBSO_LIBS)

$(libjso_target): $(JAVA_OBJS) $(C_OBJS)
	$(SOLINK) -jnimodule $(SOFLAGS) $(LDFLAGS) \
	    -o $@ $(JAVA_OBJS) $(C_OBJS) $(LIBJSO_LIBS)

$(libtso_target): $(TCL_OBJS) $(C_OBJS)
	$(SOLINK) -module $(SOFLAGS) $(LDFLAGS) \
	    -o $@ $(TCL_OBJS) $(C_OBJS) $(LIBTSO_LIBS)

$(libxso_target): $(CXX_OBJS) $(C_OBJS)
	$(XSOLINK) $(SOFLAGS) $(LDFLAGS) \
	    -o $@ $(CXX_OBJS) $(C_OBJS) $(LIBXSO_LIBS)

##################################################
# Creating individual dependencies and actions for building class
# files is possible, but it is very messy and error prone.
##################################################
java:	$(libj_jarfile) $(libj_exjarfile)

$(libj_jarfile): $(JAVA_DBSRCS)
	@test -d $(JAVA_CLASSTOP) || \
	   ($(mkdir) -p $(JAVA_CLASSTOP) && $(chmod) $(dmode) $(JAVA_CLASSTOP))
	$(JAVAC) -d $(JAVA_CLASSTOP) $(JAVACFLAGS) $(JAVA_DBSRCS)
	cd $(JAVA_CLASSTOP) && $(JAR) cf ../$(libj_jarfile) $(JAVA_DBREL)

$(libj_exjarfile): $(libj_jarfile) $(JAVA_EXSRCS)
	@test -d $(JAVA_CLASSTOP) || \
	   ($(mkdir) -p $(JAVA_CLASSTOP) && $(chmod) $(dmode) $(JAVA_CLASSTOP))
	$(JAVAC) -d $(JAVA_CLASSTOP) $(JAVACFLAGS) $(JAVA_EXSRCS)
	cd $(JAVA_CLASSTOP) && $(JAR) cf ../$(libj_exjarfile) $(JAVA_EXREL)

$(rpc_jarfile): $(libj_jarfile) $(RPC_JAVASRV_SRCS)
	@test -d $(JAVA_RPCCLASSES) || \
	   ($(mkdir) -p $(JAVA_RPCCLASSES) && \
	       $(chmod) $(dmode) $(JAVA_RPCCLASSES))
	env CLASSPATH=$(CLASSPATH):$(JAVA_RPCDIR)/oncrpc.jar \
	    nojavac -d $(JAVA_RPCCLASSES) $(JAVACFLAGS) $(RPC_JAVASRV_SRCS)
	cd $(JAVA_RPCCLASSES) && $(JAR) cf ../$(rpc_jarfile) $(JAVA_RPCREL)


##################################################
# Utilities
##################################################
berkeley_db_svc: $(RPC_SRV_OBJS) util_log.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    $(RPC_SRV_OBJS) util_log.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

berkeley_db_cxxsvc: $(RPC_CXXSRV_OBJS) util_log.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) \
	    $(RPC_CXXSRV_OBJS) util_log.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

berkeley_db_javasvc: $(rpc_jarfile)
	echo > $@ "#!/bin/sh"
	echo >> $@ CLASSPATH="$(CLASSPATH):$(rpc_jarfile):$(JAVA_RPCDIR)/oncrpc.jar"
	echo >> $@ LD_LIBRARY_PATH=.libs
	echo >> $@ export CLASSPATH LD_LIBRARY_PATH
	echo >> $@ exec java com.sleepycat.db.rpcserver.DbServer \$$@
	chmod +x $@

db_archive: db_archive.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_archive.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_checkpoint: db_checkpoint.lo util_log.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_checkpoint.lo util_log.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_deadlock: db_deadlock.lo util_log.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_deadlock.lo util_log.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_dump: db_dump.lo util_cache.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) db_dump.lo util_cache.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_dump185: db_dump185.lo 
	$(CCLINK) -o $@ $(LDFLAGS) db_dump185.lo  $(DB185LIB)
	$(POSTLINK) $@

db_load: db_load.lo util_cache.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) db_load.lo util_cache.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_printlog: db_printlog.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_printlog.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_recover: db_recover.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_recover.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_stat: db_stat.lo util_cache.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) db_stat.lo util_cache.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_upgrade: db_upgrade.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_upgrade.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

db_verify: db_verify.lo util_cache.lo util_sig.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) \
	    db_verify.lo util_cache.lo util_sig.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

##################################################
# Library and standard utilities install.
##################################################
library_install: install_setup
library_install: install_include install_lib install_utilities install_docs

uninstall: uninstall_include uninstall_lib uninstall_utilities uninstall_docs

install_setup:
	@test -d $(prefix) || \
	    ($(mkdir) -p $(prefix) && $(chmod) $(dmode) $(prefix))

INCDOT=	db.h db_cxx.h 
INCINC=	cxx_common.h cxx_except.h
install_include:
	@echo "Installing DB include files: $(includedir) ..."
	@test -d $(includedir) || \
	    ($(mkdir) -p $(includedir) && $(chmod) $(dmode) $(includedir))
	@cd $(includedir) && $(rm) -f $(INCDOT) $(INCINC)
	@$(cp) -p $(INCDOT) $(includedir)
	@cd $(srcdir)/dbinc/ && $(cp) -p $(INCINC) $(includedir)
	@cd $(includedir) && $(chmod) $(fmode) $(INCDOT) $(INCINC)

uninstall_include:
	@cd $(includedir) && $(rm) -f $(INCDOT) $(INCINC)

install_lib:
	@echo "Installing DB library: $(libdir) ..."
	@test -d $(libdir) || \
	    ($(mkdir) -p $(libdir) && $(chmod) $(dmode) $(libdir))
	@cd $(libdir) && $(rm) -f $(LIB_INSTALL_FILE_LIST)
	@$(INSTALLER) $(libso_target) $(libdir)
	@(cd $(libdir) && \
	    test -f $(libso) && $(ln) -s $(libso) $(libso_default); \
	    test -f $(libso) && $(ln) -s $(libso) $(libso_major); \
	    test -f $(libso_static) && $(ln) -s $(libso_static) $(libdb); \
	    test -f $(libxso) && $(ln) -s $(libxso) $(libxso_default); \
	    test -f $(libxso) && $(ln) -s $(libxso) $(libxso_major); \
	    test -f $(libxso_static) && $(ln) -s $(libxso_static) $(libcxx); \
	    test -f $(libtso) && $(ln) -s $(libtso) $(libtso_default); \
	    test -f $(libtso) && $(ln) -s $(libtso) $(libtso_major); \
	    test -f $(libjso) && $(ln) -s $(libjso) $(libjso_default); \
	    test -f $(libjso) && $(ln) -s $(libjso) $(libjso_major); \
	    test -f $(libjso) && $(ln) -s $(libjso) $(libjso_g)) || true
	@(test -f $(libj_jarfile) && \
	    $(cp) $(libj_jarfile) $(libdir) && \
	    $(chmod) $(fmode) $(libdir)/$(libj_jarfile)) || true

uninstall_lib:
	@cd $(libdir) && $(rm) -f $(LIB_INSTALL_FILE_LIST)

install_utilities:
	echo "Installing DB utilities: $(bindir) ..."
	@test -d $(bindir) || \
	    ($(mkdir) -p $(bindir) && $(chmod) $(dmode) $(bindir))
	@for i in $(UTIL_PROGS); do \
		$(rm) -f $(bindir)/$$i $(bindir)/$$i.exe; \
		test -f $$i.exe && i=$$i.exe || true; \
		$(INSTALLER) $$i $(bindir)/$$i; \
		test -f $(strip) && $(strip) $(bindir)/$$i || true; \
		$(chmod) $(emode) $(bindir)/$$i; \
	done

uninstall_utilities:
	@(cd $(bindir); for i in $(UTIL_PROGS); do \
		$(rm) -f $$i $$i.exe; \
	done)

DOCLIST=\
	api_c api_cxx api_java api_tcl images index.html ref reftoc.html \
	sleepycat utility
install_docs:
	@echo "Installing documentation: $(docdir) ..."
	@test -d $(docdir) || \
	    ($(mkdir) -p $(docdir) && $(chmod) $(dmode) $(docdir))
	@cd $(docdir) && $(rm) -rf $(DOCLIST)
	@cd $(srcdir)/docs && $(cp) -pr $(DOCLIST) $(docdir)/

uninstall_docs:
	@cd $(docdir) && $(rm) -rf $(DOCLIST)

##################################################
# RPM, Embedix build and install.
##################################################
RPM_ARCHIVE=db-4.1.24.tar.gz
rpm_build:
	@$(rm) -rf BUILD RPMS SOURCES SPECS SRPMS RPM_INSTALL
	@$(mkdir) -p BUILD && $(chmod) $(dmode) BUILD
	@$(mkdir) -p RPMS/i386 && $(chmod) $(dmode) RPMS RPMS/i386
	@$(mkdir) -p SOURCES && $(chmod) $(dmode) SOURCES
	@$(mkdir) -p SPECS && $(chmod) $(dmode) SPECS
	@$(mkdir) -p SRPMS && $(chmod) $(dmode) SRPMS
	$(cp) /$(RPM_ARCHIVE) SOURCES/
	$(cp) db.spec SPECS/db.spec
	$(rpm) --rcfile /X/src/rpm/db/build_unix/rpmrc -ba SPECS/db.spec

rpm_install:

RPM_SRPMS=db-4.1.24-1.src.rpm
embedix_install:
	$(cp) db.ecd /config-data/ecds/db.ecd
	$(chmod) $(fmode) /config-data/ecds/db.ecd
	$(cp) SRPMS/$(RPM_SRPMS) \
	    /Packages/SRPMS/$(RPM_SRPMS)
	$(chmod) $(fmode) \
	    /Packages/SRPMS/$(RPM_SRPMS)

##################################################
# Remaining standard Makefile targets.
##################################################
CLEAN_LIST=\
	berkeley_db_svc berkeley_db_cxxsvc berkeley_db_javasvc \
	db_dump185 db_perf dbs bench_001 \
	ex_access ex_apprec ex_btrec ex_dbclient ex_env ex_lock ex_mpool \
	ex_repquote ex_thread ex_tpcb excxx_access excxx_btrec excxx_env \
	excxx_lock excxx_mpool excxx_tpcb rpmrc

mostly-clean clean:
	$(rm) -rf $(C_OBJS)
	$(rm) -rf $(CXX_OBJS) $(JAVA_OBJS) $(TCL_OBJS)
	$(rm) -rf $(RPC_CLIENT_OBJS) $(RPC_SRV_OBJS) $(RPC_CXXSRV_OBJS)
	$(rm) -rf $(UTIL_PROGS) *.exe $(CLEAN_LIST)
	$(rm) -rf $(JAVA_CLASSTOP) $(JAVA_RPCCLASSES) $(rpc_jarfile)
	$(rm) -rf tags *.lo *.o *.o.lock *.lo core *.core
	$(rm) -rf ALL.OUT.* BUILD PARALLEL_TESTDIR.* RPMS RPM_INSTALL
	$(rm) -rf RUN_LOG RUNQUEUE SOURCES SPECS SRPMS TESTDIR TESTDIR.A
	$(rm) -rf logtrack_seen.db tm .libs $(LIB_INSTALL_FILE_LIST)

REALCLEAN_LIST=\
	Makefile confdefs.h config.cache config.log config.status db.h \
	db.spec db185_int.h db_185.h db_config.h db_cxx.h db_int.h \
	db_int_def.h include.tcl

distclean maintainer-clean realclean: clean
	$(rm) -rf $(REALCLEAN_LIST)
	$(rm) -rf libtool

check depend dvi info obj TAGS:
	@echo "$@: make target not supported" && true

dist:
	@echo "$@: make target not supported" && false

##################################################
# Multi-threaded testers, benchmarks.
##################################################
dbs.lo: $(srcdir)/test_server/dbs.c
	$(CC) $(CFLAGS) $?
dbs_am.lo: $(srcdir)/test_server/dbs_am.c
	$(CC) $(CFLAGS) $?
dbs_checkpoint.lo: $(srcdir)/test_server/dbs_checkpoint.c
	$(CC) $(CFLAGS) $?
dbs_debug.lo: $(srcdir)/test_server/dbs_debug.c
	$(CC) $(CFLAGS) $?
dbs_handles.lo: $(srcdir)/test_server/dbs_handles.c
	$(CC) $(CFLAGS) $?
dbs_log.lo: $(srcdir)/test_server/dbs_log.c
	$(CC) $(CFLAGS) $?
dbs_qam.lo: $(srcdir)/test_server/dbs_qam.c
	$(CC) $(CFLAGS) $?
dbs_spawn.lo: $(srcdir)/test_server/dbs_spawn.c
	$(CC) $(CFLAGS) $?
dbs_trickle.lo: $(srcdir)/test_server/dbs_trickle.c
	$(CC) $(CFLAGS) $?
dbs_util.lo: $(srcdir)/test_server/dbs_util.c
	$(CC) $(CFLAGS) $?
dbs_yield.lo: $(srcdir)/test_server/dbs_yield.c
	$(CC) $(CFLAGS) $?
DBS_OBJS=\
	dbs.lo dbs_am.lo dbs_checkpoint.lo dbs_debug.lo dbs_handles.lo \
	dbs_log.lo dbs_qam.lo dbs_spawn.lo dbs_trickle.lo dbs_util.lo \
	dbs_yield.lo
dbs: $(DBS_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(DBS_OBJS) $(DEF_LIB)  -lpthread -lm -lnsl $(LIBS)
	$(POSTLINK) $@

db_perf.lo: $(srcdir)/test_perf/db_perf.c
	$(CC) $(CFLAGS) $?
perf_cache_check.lo: $(srcdir)/test_perf/perf_cache_check.c
	$(CC) $(CFLAGS) $?
perf_checkpoint.lo: $(srcdir)/test_perf/perf_checkpoint.c
	$(CC) $(CFLAGS) $?
perf_config.lo: $(srcdir)/test_perf/perf_config.c
	$(CC) $(CFLAGS) $?
perf_dbs.lo: $(srcdir)/test_perf/perf_dbs.c
	$(CC) $(CFLAGS) $?
perf_debug.lo: $(srcdir)/test_perf/perf_debug.c
	$(CC) $(CFLAGS) $?
perf_file.lo: $(srcdir)/test_perf/perf_file.c
	$(CC) $(CFLAGS) $?
perf_key.lo: $(srcdir)/test_perf/perf_key.c
	$(CC) $(CFLAGS) $?
perf_log.lo: $(srcdir)/test_perf/perf_log.c
	$(CC) $(CFLAGS) $?
perf_misc.lo: $(srcdir)/test_perf/perf_misc.c
	$(CC) $(CFLAGS) $?
perf_op.lo: $(srcdir)/test_perf/perf_op.c
	$(CC) $(CFLAGS) $?
perf_parse.lo: $(srcdir)/test_perf/perf_parse.c
	$(CC) $(CFLAGS) $?
perf_rand.lo: $(srcdir)/test_perf/perf_rand.c
	$(CC) $(CFLAGS) $?
perf_spawn.lo: $(srcdir)/test_perf/perf_spawn.c
	$(CC) $(CFLAGS) $?
perf_thread.lo: $(srcdir)/test_perf/perf_thread.c
	$(CC) $(CFLAGS) $?
perf_trickle.lo: $(srcdir)/test_perf/perf_trickle.c
	$(CC) $(CFLAGS) $?
perf_txn.lo: $(srcdir)/test_perf/perf_txn.c
	$(CC) $(CFLAGS) $?
perf_util.lo: $(srcdir)/test_perf/perf_util.c
	$(CC) $(CFLAGS) $?
perf_vx.lo: $(srcdir)/test_perf/perf_vx.c
	$(CC) $(CFLAGS) $?
DBPERF_OBJS=\
	db_perf.lo perf_cache_check.lo perf_checkpoint.lo perf_config.lo \
	perf_dbs.lo perf_debug.lo perf_file.lo perf_key.lo perf_log.lo \
	perf_misc.lo perf_op.lo perf_parse.lo perf_rand.lo \
	perf_spawn.lo perf_thread.lo perf_trickle.lo perf_txn.lo \
	perf_util.lo perf_vx.lo
db_perf: $(DBPERF_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(DBPERF_OBJS) $(DEF_LIB)  -lpthread -lm -lnsl $(LIBS)
	$(POSTLINK) $@

tm.lo: $(srcdir)/mutex/tm.c
	$(CC) $(CFLAGS) $?
tm: tm.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) tm.lo $(DEF_LIB)  -lpthread -lm -lnsl $(LIBS)
	$(POSTLINK) $@

##################################################
# Example programs for C.
##################################################
bench_001.lo: $(srcdir)/examples_c/bench_001.c
	$(CC) $(CFLAGS) $?
bench_001: bench_001.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) bench_001.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_access.lo: $(srcdir)/examples_c/ex_access.c
	$(CC) $(CFLAGS) $?
ex_access: ex_access.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_access.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_apprec.lo: $(srcdir)/examples_c/ex_apprec/ex_apprec.c
	$(CC) $(CFLAGS) $?
ex_apprec_auto.lo: $(srcdir)/examples_c/ex_apprec/ex_apprec_auto.c
	$(CC) $(CFLAGS) $?
ex_apprec_rec.lo: $(srcdir)/examples_c/ex_apprec/ex_apprec_rec.c
	$(CC) $(CFLAGS) $?
EX_APPREC_OBJS=ex_apprec.lo ex_apprec_auto.lo ex_apprec_rec.lo
ex_apprec: $(EX_APPREC_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(EX_APPREC_OBJS) $(DEF_LIB)  -lpthread -lm -lnsl $(LIBS)

ex_btrec.lo: $(srcdir)/examples_c/ex_btrec.c
	$(CC) $(CFLAGS) $?
ex_btrec: ex_btrec.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_btrec.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_dbclient.lo: $(srcdir)/examples_c/ex_dbclient.c
	$(CC) $(CFLAGS) $?
ex_dbclient: ex_dbclient.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_dbclient.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_env.lo: $(srcdir)/examples_c/ex_env.c
	$(CC) $(CFLAGS) $?
ex_env: ex_env.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_env.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_lock.lo: $(srcdir)/examples_c/ex_lock.c
	$(CC) $(CFLAGS) $?
ex_lock: ex_lock.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_lock.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_mpool.lo: $(srcdir)/examples_c/ex_mpool.c
	$(CC) $(CFLAGS) $?
ex_mpool: ex_mpool.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_mpool.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

ex_rq_client.lo: $(srcdir)/examples_c/ex_repquote/ex_rq_client.c
	$(CC) $(CFLAGS) $?
ex_rq_main.lo:	$(srcdir)/examples_c/ex_repquote/ex_rq_main.c
	$(CC) $(CFLAGS) $?
ex_rq_master.lo: $(srcdir)/examples_c/ex_repquote/ex_rq_master.c
	$(CC) $(CFLAGS) $?
ex_rq_net.lo: $(srcdir)/examples_c/ex_repquote/ex_rq_net.c
	$(CC) $(CFLAGS) $?
ex_rq_util.lo: $(srcdir)/examples_c/ex_repquote/ex_rq_util.c
	$(CC) $(CFLAGS) $?
EX_RQ_OBJS=\
	ex_rq_client.lo ex_rq_main.lo ex_rq_master.lo ex_rq_net.lo ex_rq_util.lo
ex_repquote: $(EX_RQ_OBJS) $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) $(EX_RQ_OBJS) $(DEF_LIB)  -lpthread -lm -lnsl $(LIBS)
	$(POSTLINK) $@

ex_thread.lo: $(srcdir)/examples_c/ex_thread.c
	$(CC) $(CFLAGS) $?
ex_thread: ex_thread.lo $(DEF_LIB)
	$(CCLINK) -o $@ \
	    $(LDFLAGS) ex_thread.lo $(DEF_LIB)  -lpthread -lm -lnsl $(LIBS)
	$(POSTLINK) $@

ex_tpcb.lo: $(srcdir)/examples_c/ex_tpcb.c
	$(CC) $(CFLAGS) $?
ex_tpcb: ex_tpcb.lo $(DEF_LIB)
	$(CCLINK) -o $@ $(LDFLAGS) ex_tpcb.lo $(DEF_LIB) $(LIBS)
	$(POSTLINK) $@

##################################################
# Example programs for C++.
##################################################
AccessExample.lo: $(srcdir)/examples_cxx/AccessExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_access: AccessExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) AccessExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

BtRecExample.lo: $(srcdir)/examples_cxx/BtRecExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_btrec: BtRecExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) BtRecExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

EnvExample.lo: $(srcdir)/examples_cxx/EnvExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_env: EnvExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) EnvExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

LockExample.lo: $(srcdir)/examples_cxx/LockExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_lock: LockExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) LockExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

MpoolExample.lo: $(srcdir)/examples_cxx/MpoolExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_mpool: MpoolExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) MpoolExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

TpcbExample.lo: $(srcdir)/examples_cxx/TpcbExample.cpp
	$(CXX) $(CXXFLAGS) $?
excxx_tpcb: TpcbExample.lo $(DEF_LIB_CXX)
	$(CXXLINK) -o $@ $(LDFLAGS) TpcbExample.lo $(DEF_LIB_CXX) $(LIBS)
	$(POSTLINK) $@

##################################################
# C API build rules.
##################################################
aes_method.lo: $(srcdir)/crypto/aes_method.c
	 $(CC) $(CFLAGS) $?
bt_compare.lo: $(srcdir)/btree/bt_compare.c
	 $(CC) $(CFLAGS) $?
bt_conv.lo: $(srcdir)/btree/bt_conv.c
	 $(CC) $(CFLAGS) $?
bt_curadj.lo: $(srcdir)/btree/bt_curadj.c
	 $(CC) $(CFLAGS) $?
bt_cursor.lo: $(srcdir)/btree/bt_cursor.c
	 $(CC) $(CFLAGS) $?
bt_delete.lo: $(srcdir)/btree/bt_delete.c
	 $(CC) $(CFLAGS) $?
bt_method.lo: $(srcdir)/btree/bt_method.c
	 $(CC) $(CFLAGS) $?
bt_open.lo: $(srcdir)/btree/bt_open.c
	 $(CC) $(CFLAGS) $?
bt_put.lo: $(srcdir)/btree/bt_put.c
	 $(CC) $(CFLAGS) $?
bt_rec.lo: $(srcdir)/btree/bt_rec.c
	 $(CC) $(CFLAGS) $?
bt_reclaim.lo: $(srcdir)/btree/bt_reclaim.c
	 $(CC) $(CFLAGS) $?
bt_recno.lo: $(srcdir)/btree/bt_recno.c
	 $(CC) $(CFLAGS) $?
bt_rsearch.lo: $(srcdir)/btree/bt_rsearch.c
	 $(CC) $(CFLAGS) $?
bt_search.lo: $(srcdir)/btree/bt_search.c
	 $(CC) $(CFLAGS) $?
bt_split.lo: $(srcdir)/btree/bt_split.c
	 $(CC) $(CFLAGS) $?
bt_stack.lo: $(srcdir)/btree/bt_stack.c
	 $(CC) $(CFLAGS) $?
bt_stat.lo: $(srcdir)/btree/bt_stat.c
	 $(CC) $(CFLAGS) $?
bt_upgrade.lo: $(srcdir)/btree/bt_upgrade.c
	 $(CC) $(CFLAGS) $?
bt_verify.lo: $(srcdir)/btree/bt_verify.c
	 $(CC) $(CFLAGS) $?
btree_auto.lo: $(srcdir)/btree/btree_auto.c
	 $(CC) $(CFLAGS) $?
crdel_auto.lo: $(srcdir)/db/crdel_auto.c
	 $(CC) $(CFLAGS) $?
crdel_rec.lo: $(srcdir)/db/crdel_rec.c
	 $(CC) $(CFLAGS) $?
crypto.lo: $(srcdir)/crypto/crypto.c
	 $(CC) $(CFLAGS) $?
db185.lo: $(srcdir)/db185/db185.c
	 $(CC) $(CFLAGS) $?
db.lo: $(srcdir)/db/db.c
	 $(CC) $(CFLAGS) $?
db_am.lo: $(srcdir)/db/db_am.c
	 $(CC) $(CFLAGS) $?
db_auto.lo: $(srcdir)/db/db_auto.c
	 $(CC) $(CFLAGS) $?
db_byteorder.lo: $(srcdir)/common/db_byteorder.c
	 $(CC) $(CFLAGS) $?
db_cam.lo: $(srcdir)/db/db_cam.c
	 $(CC) $(CFLAGS) $?
db_conv.lo: $(srcdir)/db/db_conv.c
	 $(CC) $(CFLAGS) $?
db_dispatch.lo: $(srcdir)/db/db_dispatch.c
	 $(CC) $(CFLAGS) $?
db_dup.lo: $(srcdir)/db/db_dup.c
	 $(CC) $(CFLAGS) $?
db_err.lo: $(srcdir)/common/db_err.c
	 $(CC) $(CFLAGS) $?
db_getlong.lo: $(srcdir)/common/db_getlong.c
	 $(CC) $(CFLAGS) $?
db_idspace.lo: $(srcdir)/common/db_idspace.c
	 $(CC) $(CFLAGS) $?
db_iface.lo: $(srcdir)/db/db_iface.c
	 $(CC) $(CFLAGS) $?
db_join.lo: $(srcdir)/db/db_join.c
	 $(CC) $(CFLAGS) $?
db_log2.lo: $(srcdir)/common/db_log2.c
	 $(CC) $(CFLAGS) $?
db_meta.lo: $(srcdir)/db/db_meta.c
	 $(CC) $(CFLAGS) $?
db_method.lo: $(srcdir)/db/db_method.c
	 $(CC) $(CFLAGS) $?
db_open.lo: $(srcdir)/db/db_open.c
	 $(CC) $(CFLAGS) $?
db_overflow.lo: $(srcdir)/db/db_overflow.c
	 $(CC) $(CFLAGS) $?
db_pr.lo: $(srcdir)/db/db_pr.c
	 $(CC) $(CFLAGS) $?
db_rec.lo: $(srcdir)/db/db_rec.c
	 $(CC) $(CFLAGS) $?
db_reclaim.lo: $(srcdir)/db/db_reclaim.c
	 $(CC) $(CFLAGS) $?
db_rename.lo: $(srcdir)/db/db_rename.c
	 $(CC) $(CFLAGS) $?
db_remove.lo: $(srcdir)/db/db_remove.c
	 $(CC) $(CFLAGS) $?
db_ret.lo: $(srcdir)/db/db_ret.c
	 $(CC) $(CFLAGS) $?
db_salloc.lo: $(srcdir)/env/db_salloc.c
	 $(CC) $(CFLAGS) $?
db_shash.lo: $(srcdir)/env/db_shash.c
	 $(CC) $(CFLAGS) $?
db_truncate.lo: $(srcdir)/db/db_truncate.c
	 $(CC) $(CFLAGS) $?
db_upg.lo: $(srcdir)/db/db_upg.c
	 $(CC) $(CFLAGS) $?
db_upg_opd.lo: $(srcdir)/db/db_upg_opd.c
	 $(CC) $(CFLAGS) $?
db_vrfy.lo: $(srcdir)/db/db_vrfy.c
	 $(CC) $(CFLAGS) $?
db_vrfyutil.lo: $(srcdir)/db/db_vrfyutil.c
	 $(CC) $(CFLAGS) $?
dbm.lo: $(srcdir)/dbm/dbm.c
	 $(CC) $(CFLAGS) $?
dbreg.lo: $(srcdir)/dbreg/dbreg.c
	 $(CC) $(CFLAGS) $?
dbreg_auto.lo: $(srcdir)/dbreg/dbreg_auto.c
	 $(CC) $(CFLAGS) $?
dbreg_rec.lo: $(srcdir)/dbreg/dbreg_rec.c
	 $(CC) $(CFLAGS) $?
dbreg_util.lo: $(srcdir)/dbreg/dbreg_util.c
	 $(CC) $(CFLAGS) $?
env_file.lo: $(srcdir)/env/env_file.c
	 $(CC) $(CFLAGS) $?
env_method.lo: $(srcdir)/env/env_method.c
	 $(CC) $(CFLAGS) $?
env_open.lo: $(srcdir)/env/env_open.c
	 $(CC) $(CFLAGS) $?
env_recover.lo: $(srcdir)/env/env_recover.c
	 $(CC) $(CFLAGS) $?
env_region.lo: $(srcdir)/env/env_region.c
	 $(CC) $(CFLAGS) $?
fileops_auto.lo: $(srcdir)/fileops/fileops_auto.c
	 $(CC) $(CFLAGS) $?
fop_basic.lo: $(srcdir)/fileops/fop_basic.c
	 $(CC) $(CFLAGS) $?
fop_rec.lo: $(srcdir)/fileops/fop_rec.c
	 $(CC) $(CFLAGS) $?
fop_util.lo: $(srcdir)/fileops/fop_util.c
	 $(CC) $(CFLAGS) $?
hash.lo: $(srcdir)/hash/hash.c
	 $(CC) $(CFLAGS) $?
hash_auto.lo: $(srcdir)/hash/hash_auto.c
	 $(CC) $(CFLAGS) $?
hash_conv.lo: $(srcdir)/hash/hash_conv.c
	 $(CC) $(CFLAGS) $?
hash_dup.lo: $(srcdir)/hash/hash_dup.c
	 $(CC) $(CFLAGS) $?
hash_func.lo: $(srcdir)/hash/hash_func.c
	 $(CC) $(CFLAGS) $?
hash_meta.lo: $(srcdir)/hash/hash_meta.c
	 $(CC) $(CFLAGS) $?
hash_method.lo: $(srcdir)/hash/hash_method.c
	 $(CC) $(CFLAGS) $?
hash_open.lo: $(srcdir)/hash/hash_open.c
	 $(CC) $(CFLAGS) $?
hash_page.lo: $(srcdir)/hash/hash_page.c
	 $(CC) $(CFLAGS) $?
hash_rec.lo: $(srcdir)/hash/hash_rec.c
	 $(CC) $(CFLAGS) $?
hash_reclaim.lo: $(srcdir)/hash/hash_reclaim.c
	 $(CC) $(CFLAGS) $?
hash_stat.lo: $(srcdir)/hash/hash_stat.c
	 $(CC) $(CFLAGS) $?
hash_upgrade.lo: $(srcdir)/hash/hash_upgrade.c
	 $(CC) $(CFLAGS) $?
hash_verify.lo: $(srcdir)/hash/hash_verify.c
	 $(CC) $(CFLAGS) $?
hmac.lo: $(srcdir)/hmac/hmac.c
	 $(CC) $(CFLAGS) $?
hsearch.lo: $(srcdir)/hsearch/hsearch.c
	 $(CC) $(CFLAGS) $?
lock.lo: $(srcdir)/lock/lock.c
	 $(CC) $(CFLAGS) $?
lock_deadlock.lo:$(srcdir)/lock/lock_deadlock.c
	 $(CC) $(CFLAGS) $?
lock_method.lo:$(srcdir)/lock/lock_method.c
	 $(CC) $(CFLAGS) $?
lock_region.lo:$(srcdir)/lock/lock_region.c
	 $(CC) $(CFLAGS) $?
lock_stat.lo:$(srcdir)/lock/lock_stat.c
	 $(CC) $(CFLAGS) $?
lock_util.lo:$(srcdir)/lock/lock_util.c
	 $(CC) $(CFLAGS) $?
log.lo: $(srcdir)/log/log.c
	 $(CC) $(CFLAGS) $?
log_archive.lo: $(srcdir)/log/log_archive.c
	 $(CC) $(CFLAGS) $?
log_compare.lo: $(srcdir)/log/log_compare.c
	 $(CC) $(CFLAGS) $?
log_get.lo: $(srcdir)/log/log_get.c
	 $(CC) $(CFLAGS) $?
log_method.lo: $(srcdir)/log/log_method.c
	 $(CC) $(CFLAGS) $?
log_put.lo: $(srcdir)/log/log_put.c
	 $(CC) $(CFLAGS) $?
mp_alloc.lo: $(srcdir)/mp/mp_alloc.c
	 $(CC) $(CFLAGS) $?
mp_bh.lo: $(srcdir)/mp/mp_bh.c
	 $(CC) $(CFLAGS) $?
mp_fget.lo: $(srcdir)/mp/mp_fget.c
	 $(CC) $(CFLAGS) $?
mp_fopen.lo: $(srcdir)/mp/mp_fopen.c
	 $(CC) $(CFLAGS) $?
mp_fput.lo: $(srcdir)/mp/mp_fput.c
	 $(CC) $(CFLAGS) $?
mp_fset.lo: $(srcdir)/mp/mp_fset.c
	 $(CC) $(CFLAGS) $?
mp_method.lo: $(srcdir)/mp/mp_method.c
	 $(CC) $(CFLAGS) $?
mp_region.lo: $(srcdir)/mp/mp_region.c
	 $(CC) $(CFLAGS) $?
mp_register.lo: $(srcdir)/mp/mp_register.c
	 $(CC) $(CFLAGS) $?
mp_stat.lo: $(srcdir)/mp/mp_stat.c
	 $(CC) $(CFLAGS) $?
mp_sync.lo: $(srcdir)/mp/mp_sync.c
	 $(CC) $(CFLAGS) $?
mp_trickle.lo: $(srcdir)/mp/mp_trickle.c
	 $(CC) $(CFLAGS) $?
mt19937db.lo: $(srcdir)/crypto/mersenne/mt19937db.c
	 $(CC) $(CFLAGS) $?
mut_fcntl.lo: $(srcdir)/mutex/mut_fcntl.c
	 $(CC) $(CFLAGS) $?
mut_pthread.lo: $(srcdir)/mutex/mut_pthread.c
	 $(CC) $(CFLAGS) $?
mut_tas.lo: $(srcdir)/mutex/mut_tas.c
	 $(CC) $(CFLAGS) $?
mutex.lo: $(srcdir)/mutex/mutex.c
	 $(CC) $(CFLAGS) $?
os_abs.lo: $(srcdir)/os/os_abs.c
	 $(CC) $(CFLAGS) $?
os_alloc.lo: $(srcdir)/os/os_alloc.c
	 $(CC) $(CFLAGS) $?
os_clock.lo: $(srcdir)/os/os_clock.c
	 $(CC) $(CFLAGS) $?
os_config.lo: $(srcdir)/os/os_config.c
	 $(CC) $(CFLAGS) $?
os_dir.lo: $(srcdir)/os/os_dir.c
	 $(CC) $(CFLAGS) $?
os_errno.lo: $(srcdir)/os/os_errno.c
	 $(CC) $(CFLAGS) $?
os_fid.lo: $(srcdir)/os/os_fid.c
	 $(CC) $(CFLAGS) $?
os_fsync.lo: $(srcdir)/os/os_fsync.c
	 $(CC) $(CFLAGS) $?
os_id.lo: $(srcdir)/os/os_id.c
	 $(CC) $(CFLAGS) $?
os_handle.lo: $(srcdir)/os/os_handle.c
	 $(CC) $(CFLAGS) $?
os_map.lo: $(srcdir)/os/os_map.c
	 $(CC) $(CFLAGS) $?
os_method.lo: $(srcdir)/os/os_method.c
	 $(CC) $(CFLAGS) $?
os_oflags.lo: $(srcdir)/os/os_oflags.c
	 $(CC) $(CFLAGS) $?
os_open.lo: $(srcdir)/os/os_open.c
	 $(CC) $(CFLAGS) $?
os_region.lo: $(srcdir)/os/os_region.c
	 $(CC) $(CFLAGS) $?
os_rename.lo: $(srcdir)/os/os_rename.c
	 $(CC) $(CFLAGS) $?
os_root.lo: $(srcdir)/os/os_root.c
	 $(CC) $(CFLAGS) $?
os_rpath.lo: $(srcdir)/os/os_rpath.c
	 $(CC) $(CFLAGS) $?
os_rw.lo: $(srcdir)/os/os_rw.c
	 $(CC) $(CFLAGS) $?
os_seek.lo: $(srcdir)/os/os_seek.c
	 $(CC) $(CFLAGS) $?
os_sleep.lo: $(srcdir)/os/os_sleep.c
	 $(CC) $(CFLAGS) $?
os_spin.lo: $(srcdir)/os/os_spin.c
	 $(CC) $(CFLAGS) $?
os_stat.lo: $(srcdir)/os/os_stat.c
	 $(CC) $(CFLAGS) $?
os_tmpdir.lo: $(srcdir)/os/os_tmpdir.c
	 $(CC) $(CFLAGS) $?
os_unlink.lo: $(srcdir)/os/os_unlink.c
	 $(CC) $(CFLAGS) $?
qam.lo: $(srcdir)/qam/qam.c
	 $(CC) $(CFLAGS) $?
qam_auto.lo: $(srcdir)/qam/qam_auto.c
	 $(CC) $(CFLAGS) $?
qam_conv.lo: $(srcdir)/qam/qam_conv.c
	 $(CC) $(CFLAGS) $?
qam_files.lo: $(srcdir)/qam/qam_files.c
	 $(CC) $(CFLAGS) $?
qam_method.lo: $(srcdir)/qam/qam_method.c
	 $(CC) $(CFLAGS) $?
qam_open.lo: $(srcdir)/qam/qam_open.c
	 $(CC) $(CFLAGS) $?
qam_rec.lo: $(srcdir)/qam/qam_rec.c
	 $(CC) $(CFLAGS) $?
qam_stat.lo: $(srcdir)/qam/qam_stat.c
	 $(CC) $(CFLAGS) $?
qam_upgrade.lo: $(srcdir)/qam/qam_upgrade.c
	 $(CC) $(CFLAGS) $?
qam_verify.lo: $(srcdir)/qam/qam_verify.c
	 $(CC) $(CFLAGS) $?
rep_method.lo: $(srcdir)/rep/rep_method.c
	 $(CC) $(CFLAGS) $?
rep_record.lo: $(srcdir)/rep/rep_record.c
	 $(CC) $(CFLAGS) $?
rep_region.lo: $(srcdir)/rep/rep_region.c
	 $(CC) $(CFLAGS) $?
rep_util.lo: $(srcdir)/rep/rep_util.c
	 $(CC) $(CFLAGS) $?
rijndael-alg-fst.lo: $(srcdir)/crypto/rijndael/rijndael-alg-fst.c
	$(CC) $(CFLAGS) $?
rijndael-api-fst.lo: $(srcdir)/crypto/rijndael/rijndael-api-fst.c
	$(CC) $(CFLAGS) $?
sha1.lo: $(srcdir)/hmac/sha1.c
	$(CC) $(CFLAGS) $?
txn.lo: $(srcdir)/txn/txn.c
	 $(CC) $(CFLAGS) $?
txn_auto.lo: $(srcdir)/txn/txn_auto.c
	 $(CC) $(CFLAGS) $?
txn_method.lo: $(srcdir)/txn/txn_method.c
	 $(CC) $(CFLAGS) $?
txn_rec.lo: $(srcdir)/txn/txn_rec.c
	 $(CC) $(CFLAGS) $?
txn_recover.lo: $(srcdir)/txn/txn_recover.c
	 $(CC) $(CFLAGS) $?
txn_region.lo: $(srcdir)/txn/txn_region.c
	 $(CC) $(CFLAGS) $?
txn_stat.lo: $(srcdir)/txn/txn_stat.c
	 $(CC) $(CFLAGS) $?
txn_util.lo: $(srcdir)/txn/txn_util.c
	 $(CC) $(CFLAGS) $?
util_cache.lo: $(srcdir)/common/util_cache.c
	 $(CC) $(CFLAGS) $?
util_log.lo: $(srcdir)/common/util_log.c
	 $(CC) $(CFLAGS) $?
util_sig.lo: $(srcdir)/common/util_sig.c
	 $(CC) $(CFLAGS) $?
uts4_cc.lo: $(srcdir)/mutex/uts4_cc.s
	$(AS) $(ASFLAGS) -o $@ $?
xa.lo: $(srcdir)/xa/xa.c
	 $(CC) $(CFLAGS) $?
xa_db.lo: $(srcdir)/xa/xa_db.c
	 $(CC) $(CFLAGS) $?
xa_map.lo: $(srcdir)/xa/xa_map.c
	 $(CC) $(CFLAGS) $?

##################################################
# C++ API build rules.
##################################################
cxx_db.lo: $(srcdir)/cxx/cxx_db.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_dbc.lo: $(srcdir)/cxx/cxx_dbc.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_dbt.lo: $(srcdir)/cxx/cxx_dbt.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_env.lo: $(srcdir)/cxx/cxx_env.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_except.lo: $(srcdir)/cxx/cxx_except.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_lock.lo: $(srcdir)/cxx/cxx_lock.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_logc.lo: $(srcdir)/cxx/cxx_logc.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_mpool.lo: $(srcdir)/cxx/cxx_mpool.cpp
	$(CXX) $(CXXFLAGS) $?
cxx_txn.lo: $(srcdir)/cxx/cxx_txn.cpp
	$(CXX) $(CXXFLAGS) $?

##################################################
# Java API build rules.
##################################################
java_Db.lo::$(srcdir)/libdb_java/java_Db.c
	$(CC) $(CFLAGS) $?
java_DbEnv.lo: $(srcdir)/libdb_java/java_DbEnv.c
	$(CC) $(CFLAGS) $?
java_DbLock.lo: $(srcdir)/libdb_java/java_DbLock.c
	$(CC) $(CFLAGS) $?
java_DbLogc.lo: $(srcdir)/libdb_java/java_DbLogc.c
	$(CC) $(CFLAGS) $?
java_DbLsn.lo: $(srcdir)/libdb_java/java_DbLsn.c
	$(CC) $(CFLAGS) $?
java_DbTxn.lo: $(srcdir)/libdb_java/java_DbTxn.c
	$(CC) $(CFLAGS) $?
java_DbUtil.lo: $(srcdir)/libdb_java/java_DbUtil.c
	$(CC) $(CFLAGS) $?
java_Dbc.lo: $(srcdir)/libdb_java/java_Dbc.c
	$(CC) $(CFLAGS) $?
java_Dbt.lo: $(srcdir)/libdb_java/java_Dbt.c
	$(CC) $(CFLAGS) $?
java_info.lo: $(srcdir)/libdb_java/java_info.c
	$(CC) $(CFLAGS) $?
java_locked.lo: $(srcdir)/libdb_java/java_locked.c
	$(CC) $(CFLAGS) $?
java_util.lo: $(srcdir)/libdb_java/java_util.c
	$(CC) $(CFLAGS) $?
java_stat_auto.lo: $(srcdir)/libdb_java/java_stat_auto.c
	$(CC) $(CFLAGS) $?

##################################################
# Tcl API build rules.
##################################################
tcl_compat.lo: $(srcdir)/tcl/tcl_compat.c
	$(CC) $(CFLAGS) $(TCFLAGS) $?
tcl_db.lo: $(srcdir)/tcl/tcl_db.c
	$(CC) $(CFLAGS) $(TCFLAGS) $?
tcl_db_pkg.lo: $(srcdir)/tcl/tcl_db_pkg.c
	$(CC) $(CFLAGS) $(TCFLAGS) $?
tcl_dbcursor.lo: $(srcdir)/tcl/tcl_dbcursor.c
	$(CC) $(CFLAGS) $(TCFLAGS) $?
tcl_env.lo: $(srcdir)/tcl/tcl_env.c
	$(CC) $(CFLAGS) $(TCFLAGS) $?
tcl_internal.lo: $(srcdir)/tcl/tcl_internal.c
	$(CC) $(CFLAGS) $(TCFLAGS) $?
tcl_lock.lo: $(srcdir)/tcl/tcl_lock.c
	$(CC) $(CFLAGS) $(TCFLAGS) $?
tcl_log.lo: $(srcdir)/tcl/tcl_log.c
	$(CC) $(CFLAGS) $(TCFLAGS) $?
tcl_mp.lo: $(srcdir)/tcl/tcl_mp.c
	$(CC) $(CFLAGS) $(TCFLAGS) $?
tcl_rep.lo: $(srcdir)/tcl/tcl_rep.c
	$(CC) $(CFLAGS) $(TCFLAGS) $?
tcl_txn.lo: $(srcdir)/tcl/tcl_txn.c
	$(CC) $(CFLAGS) $(TCFLAGS) $?
tcl_util.lo: $(srcdir)/tcl/tcl_util.c
	$(CC) $(CFLAGS) $(TCFLAGS) $?

##################################################
# RPC build rules.
##################################################
# RPC client files
client.lo: $(srcdir)/rpc_client/client.c
	$(CC) $(CFLAGS) $?
db_server_clnt.lo: $(srcdir)/rpc_client/db_server_clnt.c
	$(CC) $(CFLAGS) $?
gen_client.lo: $(srcdir)/rpc_client/gen_client.c
	$(CC) $(CFLAGS) $?
gen_client_ret.lo: $(srcdir)/rpc_client/gen_client_ret.c
	$(CC) $(CFLAGS) $?

# RPC server files
db_server_proc.lo: $(srcdir)/rpc_server/c/db_server_proc.c
	$(CC) $(CFLAGS) $?
db_server_svc.lo: $(srcdir)/rpc_server/c/db_server_svc.c
	$(CC) $(CFLAGS) $?
db_server_util.lo: $(srcdir)/rpc_server/c/db_server_util.c
	$(CC) $(CFLAGS) $?
db_server_xdr.lo: $(srcdir)/rpc_server/c/db_server_xdr.c
	$(CC) $(CFLAGS) $?
gen_db_server.lo: $(srcdir)/rpc_server/c/gen_db_server.c
	$(CC) $(CFLAGS) $?
db_server_cxxproc.lo: $(srcdir)/rpc_server/cxx/db_server_cxxproc.cpp
	$(CXX) $(CXXFLAGS) $?
db_server_cxxutil.lo: $(srcdir)/rpc_server/cxx/db_server_cxxutil.cpp
	$(CXX) $(CXXFLAGS) $?

##################################################
# Utility build rules.
##################################################
db_archive.lo: $(srcdir)/db_archive/db_archive.c
	$(CC) $(CFLAGS) $?
db_checkpoint.lo: $(srcdir)/db_checkpoint/db_checkpoint.c
	$(CC) $(CFLAGS) $?
db_deadlock.lo: $(srcdir)/db_deadlock/db_deadlock.c
	$(CC) $(CFLAGS) $?
db_dump.lo: $(srcdir)/db_dump/db_dump.c
	$(CC) $(CFLAGS) $?
db_dump185.lo: $(srcdir)/db_dump185/db_dump185.c
	$(CC) $(DB185INC) $?
db_load.lo: $(srcdir)/db_load/db_load.c
	$(CC) $(CFLAGS) $?
db_printlog.lo: $(srcdir)/db_printlog/db_printlog.c
	$(CC) $(CFLAGS) $?
db_recover.lo: $(srcdir)/db_recover/db_recover.c
	$(CC) $(CFLAGS) $?
db_stat.lo: $(srcdir)/db_stat/db_stat.c
	$(CC) $(CFLAGS) $?
db_upgrade.lo: $(srcdir)/db_upgrade/db_upgrade.c
	$(CC) $(CFLAGS) $?
db_verify.lo: $(srcdir)/db_verify/db_verify.c
	$(CC) $(CFLAGS) $?

##################################################
# C library replacement files.
##################################################
getcwd.lo: $(srcdir)/clib/getcwd.c
	$(CC) $(CFLAGS) $?
getopt.lo: $(srcdir)/clib/getopt.c
	$(CC) $(CFLAGS) $?
memcmp.lo: $(srcdir)/clib/memcmp.c
	$(CC) $(CFLAGS) $?
memcpy.lo: $(srcdir)/clib/memmove.c
	$(CC) -DMEMCOPY $(CFLAGS) $? -o $@
memmove.lo: $(srcdir)/clib/memmove.c
	$(CC) -DMEMMOVE $(CFLAGS) $?
raise.lo: $(srcdir)/clib/raise.c
	$(CC) $(CFLAGS) $?
strcasecmp.lo: $(srcdir)/clib/strcasecmp.c
	$(CC) $(CFLAGS) $?
strdup.lo: $(srcdir)/clib/strdup.c
	$(CC) $(CFLAGS) $?
snprintf.lo: $(srcdir)/clib/snprintf.c
	$(CC) $(CFLAGS) $?
strerror.lo: $(srcdir)/clib/strerror.c
	$(CC) $(CFLAGS) $?
vsnprintf.lo: $(srcdir)/clib/vsnprintf.c
	$(CC) $(CFLAGS) $?

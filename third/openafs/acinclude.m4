dnl This file contains the common configuration code which would
dnl otherwise be duplicated between configure and configure-libafs.
dnl
dnl NB: Because this code is a macro, references to positional shell
dnl parameters must be done like $[]1 instead of $1

AC_DEFUN(OPENAFS_CONFIGURE_COMMON,[

AC_CANONICAL_HOST
SRCDIR_PARENT=`pwd`

#BOZO_SAVE_CORES pam sia
AC_ARG_WITH(afs-sysname,
[  --with-afs-sysname=sys    use sys for the afs sysname]
)
AC_ARG_ENABLE( obsolete,
[  --enable-obsolete 			enable obsolete portions of AFS (mpp, ntp and package)],, enable_obsolete="no")
AC_ARG_ENABLE( insecure,
[  --enable-insecure 			enable insecure portions of AFS (ftpd, inetd, rcp, rlogind and rsh)],, enable_insecure="no")
AC_ARG_ENABLE( afsdb,
[  --disable-afsdb 			disable AFSDB RR support],, enable_afsdb="yes")
AC_ARG_ENABLE( bos-restricted-mode,
[  --enable-bos-restricted-mode 	enable bosserver restricted mode which disables certain bosserver functionality],, enable_bos_restricted_mode="no")
AC_ARG_ENABLE( bos-new-config,
[  --enable-bos-new-config	 	enable bosserver pickup of BosConfig.new on restarts],, enable_bos_new_config="no")
AC_ARG_ENABLE( namei-fileserver,
[  --enable-namei-fileserver 		force compilation of namei fileserver in preference to inode fileserver],, enable_namei_fileserver="no")
AC_ARG_ENABLE( fast-restart,
[  --enable-fast-restart 		enable fast startup of file server without salvaging],, enable_fast_restart="no")
AC_ARG_ENABLE( bitmap-later,
[  --enable-bitmap-later 		enable fast startup of file server by not reading bitmap till needed],, enable_bitmap_later="no")
AC_ARG_ENABLE( full-vos-listvol-switch,
[  --enable-full-vos-listvol-switch     enable vos full listvol switch for formatted output],, enable_full_vos_listvol_switch="no")
AC_ARG_WITH(dux-kernel-headers,
[  --with-dux-kernel-headers=path    	use the kernel headers found at path(optional, defaults to first match in /usr/sys)]
)
AC_ARG_WITH(linux-kernel-headers,
[  --with-linux-kernel-headers=path    	use the kernel headers found at path(optional, defaults to /usr/src/linux)]
)
AC_ARG_ENABLE(kernel-module,
[  --disable-kernel-module             	disable compilation of the kernel module (defaults to enabled)],, enable_kernel_module="yes"
)
AC_ARG_ENABLE(redhat-buildsys,
[  --enable-redhat-buildsys		enable compilation of the redhat build system kernel (defaults to disabled)],, enable_redhat_buildsys="no"
)
AC_ARG_ENABLE(transarc-paths,
[  --enable-transarc-paths              	Use Transarc style paths like /usr/afs and /usr/vice],, enable_transarc_paths="no"
)
AC_ARG_ENABLE(tivoli-tsm,
[  --enable-tivoli-tsm              	Enable use of the Tivoli TSM API libraries for butc support],, enable_tivoli_tsm="no"
)
AC_ARG_ENABLE(debug-kernel,
[  --enable-debug-kernel		enable compilation of the kernel module with debugging information (defaults to disabled)],, enable_debug_kernel="no"
)

dnl weird ass systems
AC_AIX
AC_ISC_POSIX
AC_MINIX

dnl Various compiler setup.
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SIGNAL

dnl Checks for programs.
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_RANLIB
AC_PROG_YACC
AM_PROG_LEX

OPENAFS_CHECK_BIGENDIAN

KERN_DEBUG_OPT=
if test "x$enable_debug_kernel" = "xyes"; then
  KERN_DEBUG_OPT=-g
fi

AC_MSG_CHECKING(your OS)
system=$host
case $system in
        *-linux*)
		MKAFS_OSTYPE=LINUX
		if test "x$enable_redhat_buildsys" = "xyes"; then
		 AC_DEFINE(ENABLE_REDHAT_BUILDSYS, 1, [define if you have redhat buildsystem])
		fi
		if test "x$enable_kernel_module" = "xyes"; then
		 if test "x$with_linux_kernel_headers" != "x"; then
		   LINUX_KERNEL_PATH="$with_linux_kernel_headers"
		 else
		   LINUX_KERNEL_PATH="/usr/src/linux-2.4"
		   if test ! -f "$LINUX_KERNEL_PATH/include/linux/version.h"; then
		     LINUX_KERNEL_PATH="/usr/src/linux"
		   fi
		 fi
		 if test -f "$LINUX_KERNEL_PATH/include/linux/version.h"; then
		  linux_kvers=`fgrep UTS_RELEASE $LINUX_KERNEL_PATH/include/linux/version.h |awk 'BEGIN { FS="\"" } { print $[]2 }'|tail -1`
		  if test "x$linux_kvers" = "x"; then
		    if test -f "$LINUX_KERNEL_PATH/include/linux/version-up.h"; then
		      linux_kvers=`fgrep UTS_RELEASE $LINUX_KERNEL_PATH/include/linux/version-up.h |awk 'BEGIN { FS="\"" } { print $[]2 }'|tail -1`
		      if test "x$linux_kvers" = "x"; then

		        AC_MSG_ERROR(Linux headers lack version definition [2])
		        exit 1
		      else
		        LINUX_VERSION="$linux_kvers"
                      fi
                    else
                      AC_MSG_ERROR(Linux headers lack version definition)
		      exit 1
		    fi
		  else
		    LINUX_VERSION="$linux_kvers"
		  fi
		 else
                    enable_kernel_module="no"
                 fi
		 if test ! -f "$LINUX_KERNEL_PATH/include/linux/autoconf.h"; then
		     enable_kernel_module="no"
		 fi
		 if test "x$enable_kernel_module" = "xno"; then
		  if test "x$with_linux_kernel_headers" != "x"; then
		   AC_MSG_ERROR(No usable linux headers found at $LINUX_KERNEL_PATH)
		   exit 1
		  else
		   AC_MSG_WARN(No usable linux headers found at $LINUX_KERNEL_PATH so disabling kernel module)
		  fi
		 fi
		fi
		AC_MSG_RESULT(linux)
		if test "x$enable_kernel_module" = "xyes"; then
		 OMIT_FRAME_POINTER=
		 if test "x$enable_debug_kernel" = "xno"; then
			OMIT_FRAME_POINTER=-fomit-frame-pointer
		 fi
		 AC_SUBST(OMIT_FRAME_POINTER)
		 OPENAFS_GCC_SUPPORTS_MARCH
		 AC_SUBST(P5PLUS_KOPTS)
		 OPENAFS_GCC_NEEDS_NO_STRENGTH_REDUCE
		 OPENAFS_GCC_NEEDS_NO_STRICT_ALIASING
		 OPENAFS_GCC_SUPPORTS_NO_COMMON
		 AC_SUBST(LINUX_GCC_KOPTS)
	         ifdef([OPENAFS_CONFIGURE_LIBAFS],
	           [LINUX_BUILD_VNODE_FROM_INODE(config,afs)],
	           [LINUX_BUILD_VNODE_FROM_INODE(src/config,src/afs/LINUX)]
	         )
		 LINUX_COMPLETION_H_EXISTS
		 LINUX_DEFINES_FOR_EACH_PROCESS
		 LINUX_DEFINES_PREV_TASK
		 LINUX_EXPORTS_TASKLIST_LOCK
	         LINUX_FS_STRUCT_ADDRESS_SPACE_HAS_PAGE_LOCK
	         LINUX_FS_STRUCT_ADDRESS_SPACE_HAS_GFP_MASK
		 LINUX_FS_STRUCT_INODE_HAS_I_ALLOC_SEM
		 LINUX_FS_STRUCT_INODE_HAS_I_TRUNCATE_SEM
		 LINUX_FS_STRUCT_INODE_HAS_I_DIRTY_DATA_BUFFERS
		 LINUX_FS_STRUCT_INODE_HAS_I_DEVICES
	  	 LINUX_INODE_SETATTR_RETURN_TYPE
		 LINUX_KERNEL_LINUX_SYSCALL_H
		 LINUX_NEED_RHCONFIG
		 LINUX_RECALC_SIGPENDING_ARG_TYPE
		 LINUX_SCHED_STRUCT_TASK_STRUCT_HAS_PARENT
		 LINUX_SCHED_STRUCT_TASK_STRUCT_HAS_REAL_PARENT
		 LINUX_SCHED_STRUCT_TASK_STRUCT_HAS_SIG
		 LINUX_SCHED_STRUCT_TASK_STRUCT_HAS_SIGHAND
		 LINUX_SCHED_STRUCT_TASK_STRUCT_HAS_SIGMASK_LOCK
		 LINUX_WHICH_MODULES
                 if test "x$ac_cv_linux_config_modversions" = "xno"; then
                   AC_MSG_WARN([Cannot determine sys_call_table status. assuming it's exported])
                   ac_cv_linux_exports_sys_call_table=yes
		   if test -f "$LINUX_KERNEL_PATH/include/asm/ia32_unistd.h"; then
		     ac_cv_linux_exports_ia32_sys_call_table=yes
		   fi
                 else
                   LINUX_EXPORTS_INIT_MM
                   LINUX_EXPORTS_KALLSYMS_ADDRESS
                   LINUX_EXPORTS_KALLSYMS_SYMBOL
                   LINUX_EXPORTS_SYS_CALL_TABLE
                   LINUX_EXPORTS_IA32_SYS_CALL_TABLE
                   LINUX_EXPORTS_SYS_CHDIR
                   LINUX_EXPORTS_SYS_CLOSE
                   LINUX_EXPORTS_SYS_WAIT4
                   if test "x$ac_cv_linux_exports_sys_call_table" = "xno"; then
                         linux_syscall_method=none
                         if test "x$ac_cv_linux_exports_init_mm" = "xyes"; then
                            linux_syscall_method=scan
                            if test "x$ac_cv_linux_exports_kallsyms_address" = "xyes"; then
                               linux_syscall_method=scan_with_kallsyms_address
                            fi
                         fi
                         if test "x$ac_cv_linux_exports_kallsyms_symbol" = "xyes"; then
                            linux_syscall_method=kallsyms_symbol
                         fi
                         if test "x$linux_syscall_method" = "xnone"; then
                        AC_MSG_ERROR([no available sys_call_table access method])
                         fi
                   fi
                 fi
		 if test "x$ac_cv_linux_exports_sys_chdir" = "xyes" ; then
		  AC_DEFINE(EXPORTED_SYS_CHDIR, 1, [define if your linux kernel exports sys_chdir])
		 fi
		 if test "x$ac_cv_linux_exports_sys_close" = "xyes" ; then
		  AC_DEFINE(EXPORTED_SYS_CLOSE, 1, [define if your linux kernel exports sys_close])
		 fi
		 if test "x$ac_cv_linux_exports_sys_wait4" = "xyes" ; then
		  AC_DEFINE(EXPORTED_SYS_WAIT4, 1, [define if your linux kernel exports sys_wait4])
		 fi
		 if test "x$ac_cv_linux_exports_tasklist_lock" = "xyes" ; then
		  AC_DEFINE(EXPORTED_TASKLIST_LOCK, 1, [define if your linux kernel exports tasklist_lock])
		 fi
                 if test "x$ac_cv_linux_exports_sys_call_table" = "xyes"; then
                  AC_DEFINE(EXPORTED_SYS_CALL_TABLE)
                 fi
                 if test "x$ac_cv_linux_exports_ia32_sys_call_table" = "xyes"; then
                  AC_DEFINE(EXPORTED_IA32_SYS_CALL_TABLE)
                 fi
                 if test "x$ac_cv_linux_exports_kallsyms_symbol" = "xyes"; then
                  AC_DEFINE(EXPORTED_KALLSYMS_SYMBOL)
                 fi
                 if test "x$ac_cv_linux_exports_kallsyms_address" = "xyes"; then
                  AC_DEFINE(EXPORTED_KALLSYMS_ADDRESS)
                 fi
		 if test "x$ac_cv_linux_completion_h_exists" = "xyes" ; then
		  AC_DEFINE(COMPLETION_H_EXISTS, 1, [define if completion_h exists])
		 fi
		 if test "x$ac_cv_linux_defines_for_each_process" = "xyes" ; then
		  AC_DEFINE(DEFINED_FOR_EACH_PROCESS, 1, [define if for_each_process defined])
		 fi
		 if test "x$ac_cv_linux_defines_prev_task" = "xyes" ; then
		  AC_DEFINE(DEFINED_PREV_TASK, 1, [define if prev_task defined])
		 fi
		 if test "x$ac_cv_linux_func_inode_setattr_returns_int" = "xyes" ; then
		  AC_DEFINE(INODE_SETATTR_NOT_VOID, 1, [define if your setattr return return non-void])
		 fi
		 if test "x$ac_cv_linux_fs_struct_address_space_has_page_lock" = "xyes"; then 
		  AC_DEFINE(STRUCT_ADDRESS_SPACE_HAS_PAGE_LOCK, 1, [define if your struct address_space has page_lock])
		 fi
		 if test "x$ac_cv_linux_fs_struct_address_space_has_gfp_mask" = "xyes"; then 
		  AC_DEFINE(STRUCT_ADDRESS_SPACE_HAS_GFP_MASK, 1, [define if your struct address_space has gfp_mask])
		 fi
		 if test "x$ac_cv_linux_fs_struct_inode_has_i_truncate_sem" = "xyes"; then 
		  AC_DEFINE(STRUCT_INODE_HAS_I_TRUNCATE_SEM, 1, [define if your struct inode has truncate_sem])
		 fi
		 if test "x$ac_cv_linux_fs_struct_inode_has_i_alloc_sem" = "xyes"; then 
		  AC_DEFINE(STRUCT_INODE_HAS_I_ALLOC_SEM, 1, [define if your struct inode has alloc_sem])
		 fi
		 if test "x$ac_cv_linux_fs_struct_inode_has_i_devices" = "xyes"; then 
		  AC_DEFINE(STRUCT_INODE_HAS_I_DEVICES, 1, [define if you struct inode has i_devices])
		 fi
		 if test "x$ac_cv_linux_fs_struct_inode_has_i_dirty_data_buffers" = "xyes"; then 
		  AC_DEFINE(STRUCT_INODE_HAS_I_DIRTY_DATA_BUFFERS, 1, [define if your struct inode has data_buffers])
		 fi
		 if test "x$ac_cv_linux_func_recalc_sigpending_takes_void" = "xyes"; then 
		  AC_DEFINE(RECALC_SIGPENDING_TAKES_VOID, 1, [define if your recalc_sigpending takes void])
		 fi
		 if test "x$ac_linux_syscall" = "xyes" ; then
		  AC_DEFINE(HAVE_KERNEL_LINUX_SYSCALL_H, 1, [define if your linux kernel has linux/syscall.h])
		 fi
		 if test "x$ac_cv_linux_sched_struct_task_struct_has_parent" = "xyes"; then 
		  AC_DEFINE(STRUCT_TASK_STRUCT_HAS_PARENT, 1, [define if your struct task_struct has parent])
		 fi
		 if test "x$ac_cv_linux_sched_struct_task_struct_has_real_parent" = "xyes"; then 
		  AC_DEFINE(STRUCT_TASK_STRUCT_HAS_REAL_PARENT, 1, [define if your struct task_struct has real_parent])
		 fi
		 if test "x$ac_cv_linux_sched_struct_task_struct_has_sigmask_lock" = "xyes"; then 
		  AC_DEFINE(STRUCT_TASK_STRUCT_HAS_SIGMASK_LOCK, 1, [define if your struct task_struct has sigmask_lock])
		 fi
		 if test "x$ac_cv_linux_sched_struct_task_struct_has_sighand" = "xyes"; then 
		  AC_DEFINE(STRUCT_TASK_STRUCT_HAS_SIGHAND, 1, [define if your struct task_struct has sighand])
		 fi
		 if test "x$ac_cv_linux_sched_struct_task_struct_has_sig" = "xyes"; then 
		  AC_DEFINE(STRUCT_TASK_STRUCT_HAS_SIG, 1, [define if your struct task_struct has sig])
		 fi
                :
		fi
                ;;
        *-solaris*)
		MKAFS_OSTYPE=SOLARIS
                AC_MSG_RESULT(sun4)
		SOLARIS_UFSVFS_HAS_DQRWLOCK
		SOLARIS_PROC_HAS_P_COREFILE
		SOLARIS_FS_HAS_FS_ROLLED
                ;;
        *-sunos*)
		MKAFS_OSTYPE=SUNOS
		enable_kernel_module=no
                AC_MSG_RESULT(sun4)
                ;;
        *-hpux*)
		MKAFS_OSTYPE=HPUX
                AC_MSG_RESULT(hp_ux)
                ;;
        *-irix*)
		if test -d /usr/include/sys/SN/SN1; then
		 IRIX_BUILD_IP35="IP35"
		fi
		MKAFS_OSTYPE=IRIX
                AC_MSG_RESULT(sgi)
                ;;
        *-aix*)
		MKAFS_OSTYPE=AIX
                AC_MSG_RESULT(rs_aix)
                ;;
        *-osf*)
		MKAFS_OSTYPE=DUX
                AC_MSG_RESULT(alpha_dux)
		if test "x$enable_kernel_module" = "xyes"; then
		 if test "x$with_dux_kernel_headers" != "x"; then
		   HEADER_RT=`ls ${with_dux_kernel_headers}/rt_preempt.h | head -1 | sed 's,/rt_preempt.h,,;s,/usr/sys/,,'`
		 else
 		   HEADER_RT=`ls /usr/sys/*/rt_preempt.h | head -1 | sed 's,/rt_preempt.h,,;s,/usr/sys/,,'`
		 fi
		fi
		if test "$HEADER_RT" = "*" ; then
			AC_MSG_ERROR([Need a configured kernel directory])
		fi
		AC_SUBST([HEADER_RT])
                ;;
        *-darwin*)
		MKAFS_OSTYPE=DARWIN
                AC_MSG_RESULT(ppc_darwin)
                ;;
	*-freebsd*)
		MKAFS_OSTYPE=FBSD
		AC_MSG_RESULT(i386_fbsd)
		;;
	*-openbsd*)
		MKAFS_OSTYPE=OBSD
		AC_MSG_RESULT(i386_obsd)
		;;
        *)
                AC_MSG_RESULT($system)
                ;;
esac
AC_SUBST(KERN_DEBUG_OPT)

if test "x$with_afs_sysname" != "x"; then
        AFS_SYSNAME="$with_afs_sysname"
else
	AC_MSG_CHECKING(your AFS sysname)
	case $host in
		i?86-*-freebsd4.2*)
			AFS_SYSNAME="i386_fbsd_42"
			;;
		i?86-*-freebsd4.3*)
			AFS_SYSNAME="i386_fbsd_43"
			;;
		i?86-*-freebsd4.4*)
			AFS_SYSNAME="i386_fbsd_44"
			;;
		i?86-*-freebsd4.5*)
			AFS_SYSNAME="i386_fbsd_45"
			;;
		i?86-*-freebsd4.6*)
			AFS_SYSNAME="i386_fbsd_46"
			;;
		hppa*-hp-hpux11.0*)
			AFS_SYSNAME="hp_ux110"
			;;
		hppa*-hp-hpux11.11)
			 AFS_SYSNAME="hp_ux11i"
			;;
		ia64-hp-hpux*)
			AFS_SYSNAME="ia64_hpux1122"
			;;
		hppa*-hp-hpux10*)
			AFS_SYSNAME="hp_ux102"
			;;
		powerpc-apple-darwin1.2*)
			AFS_SYSNAME="ppc_darwin_12"
			;;
		powerpc-apple-darwin1.3*)
			AFS_SYSNAME="ppc_darwin_13"
			;;
		powerpc-apple-darwin1.4*)
			AFS_SYSNAME="ppc_darwin_14"
			;;
		powerpc-apple-darwin5.1*)
			AFS_SYSNAME="ppc_darwin_14"
			;;
		powerpc-apple-darwin5.2*)
			AFS_SYSNAME="ppc_darwin_14"
			;;
		powerpc-apple-darwin5.3*)
			AFS_SYSNAME="ppc_darwin_14"
			;;
		powerpc-apple-darwin5.4*)
			AFS_SYSNAME="ppc_darwin_14"
			;;
		powerpc-apple-darwin5.5*)
			AFS_SYSNAME="ppc_darwin_14"
			;;
		powerpc-apple-darwin6.0*)
			AFS_SYSNAME="ppc_darwin_60"
			;;
		powerpc-apple-darwin6.1*)
			AFS_SYSNAME="ppc_darwin_60"
			;;
		powerpc-apple-darwin6.2*)
			AFS_SYSNAME="ppc_darwin_60"
			;;
		powerpc-apple-darwin6.3*)
			AFS_SYSNAME="ppc_darwin_60"
			;;
		powerpc-apple-darwin6.4*)
			AFS_SYSNAME="ppc_darwin_60"
			;;
		powerpc-apple-darwin6.5*)
			AFS_SYSNAME="ppc_darwin_60"
			;;
		sparc-sun-solaris2.5*)
			AFS_SYSNAME="sun4x_55"
			;;
		sparc-sun-solaris2.6)
			AFS_SYSNAME="sun4x_56"
			;;
		sparc-sun-solaris2.7)
			AFS_SYSNAME="sun4x_57"
			;;
		sparc-sun-solaris2.8)
			AFS_SYSNAME="sun4x_58"
			;;
		sparc-sun-solaris2.9)
			AFS_SYSNAME="sun4x_59"
			;;
		sparc-sun-sunos4*)
			AFS_SYSNAME="sun4_413"
			;;
		i386-pc-solaris2.7)
			AFS_SYSNAME="sunx86_57"
			;;
		i386-pc-solaris2.8)
			AFS_SYSNAME="sunx86_58"
			;;
		i386-pc-solaris2.9)
			AFS_SYSNAME="sunx86_59"
			;;
		alpha*-dec-osf4.0*)
			AFS_SYSNAME="alpha_dux40"
			;;
		alpha*-dec-osf5.0*)
			AFS_SYSNAME="alpha_dux50"
			;;
		alpha*-dec-osf5.1*)
			AFS_SYSNAME="alpha_dux51"
			;;
		mips-sgi-irix6.5)
			AFS_SYSNAME="sgi_65"
			;;
		ia64-*-linux*)
			AFS_SYSNAME="ia64_linuxXX"
			;;
		powerpc-*-linux*)
			AFS_SYSNAME="ppc_linuxXX"
			;;
		alpha*-linux*)
			AFS_SYSNAME="alpha_linux_XX"
			;;
		s390-*-linux*)
			AFS_SYSNAME="s390_linuxXX"
			;;
		sparc-*-linux*)
			AFS_SYSNAME="sparc_linuxXX"
			;;
		sparc64-*-linux*)
			AFS_SYSNAME="sparc64_linuxXX"
			;;
		i?86-*-linux*)
			AFS_SYSNAME="i386_linuxXX"
			;;
		parisc-*-linux-gnu)
			AFS_SYSNAME="parisc_linuxXX"
			;;
		power*-ibm-aix4.2*)
			AFS_SYSNAME="rs_aix42"
			;;
		power*-ibm-aix4.3*)
			AFS_SYSNAME="rs_aix42"
			;;
		x86_64-*-linux-gnu)
			AFS_SYSNAME="amd64_linuxXX"
			;;
		*)
			AC_MSG_ERROR(An AFS sysname is required)
			exit 1
			;;
	esac
	case $AFS_SYSNAME in
		*_linux*)
			AFS_SYSKVERS=`echo $LINUX_VERSION | awk -F\. '{print $[]1 $[]2}'`
			if test "x${AFS_SYSKVERS}" = "x"; then
			 AC_MSG_ERROR(Couldn't guess your Linux version. Please use the --with-afs-sysname option to configure an AFS sysname.)
			fi
			_AFS_SYSNAME=`echo $AFS_SYSNAME|sed s/XX\$/$AFS_SYSKVERS/`
			AFS_SYSNAME="$_AFS_SYSNAME"
			if test -f "$LINUX_KERNEL_PATH/include/linux/autoconf.h"; then
			 AFS_ISUML=`awk '$[]2 == "CONFIG_USERMODE"{print $[]3}' $LINUX_KERNEL_PATH/include/linux/autoconf.h`
			 if test "x${AFS_ISUML}" = "x1"; then
			  _AFS_SYSNAME=`echo $AFS_SYSNAME|sed s/linux/umlinux/`
			 fi
			 AFS_SYSNAME="$_AFS_SYSNAME"
			fi
			;;
	esac
        AC_MSG_RESULT($AFS_SYSNAME)
fi

# KDUMP64 defaults to KDUMP for systems without a separate kdump64
KDUMP64='${KDUMP}'
KDUMP=kdump
case $AFS_SYSNAME in
	sgi_6?)
		KDUMP=kdump.IP20;;
	sun4x_5[789] | hp_ux11*)
		KDUMP=kdump32
		KDUMP64=kdump64;;
	*linux*)
		KDUMP='kdump-${LINUX_VERSION}';;
esac
AC_SUBST(KDUMP)
AC_SUBST(KDUMP64)

case $AFS_SYSNAME in
	*_darwin*)
		DARWIN_PLIST=src/libafs/afs.${AFS_SYSNAME}.plist
		DARWIN_INFOFILE=afs.${AFS_SYSNAME}.plist
		;;
esac
AC_CACHE_VAL(ac_cv_sockaddr_len,
[
AC_MSG_CHECKING([if struct sockaddr has sa_len field])
AC_TRY_COMPILE( [#include <sys/types.h>
#include <sys/socket.h>],
[struct sockaddr *a;
a->sa_len=0;], ac_cv_sockaddr_len=yes, ac_cv_sockaddr_len=no)
AC_MSG_RESULT($ac_cv_sockaddr_len)])
if test "$ac_cv_sockaddr_len" = "yes"; then
   AC_DEFINE(STRUCT_SOCKADDR_HAS_SA_LEN, 1, [define if you struct sockaddr sa_len])
fi
if test "x${MKAFS_OSTYPE}" = "xIRIX"; then
        echo Skipping library tests because they confuse Irix.
else
  AC_CHECK_FUNCS(socket)

  if test "$ac_cv_func_socket" = no; then
    for lib in socket inet; do
        if test "$HAVE_SOCKET" != 1; then
                AC_CHECK_LIB(${lib}, socket,LIBS="$LIBS -l$lib";HAVE_SOCKET=1;AC_DEFINE(HAVE_SOCKET, 1, [define if you have socket]))
        fi
    done
  fi
  
  AC_CHECK_FUNCS(connect)       

  if test "$ac_cv_func_connect" = no; then
    for lib in nsl; do
        if test "$HAVE_CONNECT" != 1; then
                AC_CHECK_LIB(${lib}, connect,LIBS="$LIBS -l$lib";HAVE_CONNECT=1;AC_DEFINE(HAVE_CONNECT, 1, [define if you have connect]))
        fi
    done
  fi

  AC_CHECK_FUNCS(gethostbyname)
  if test "$ac_cv_func_gethostbyname" = no; then
        for lib in dns nsl resolv; do
          if test "$HAVE_GETHOSTBYNAME" != 1; then
            AC_CHECK_LIB(${lib}, gethostbyname, LIBS="$LIBS -l$lib";HAVE_GETHOSTBYNAME=1;AC_DEFINE(HAVE_GETHOSTBYNAME, 1, [define if you have gethostbyname]))
          fi
        done    
  fi    

  openafs_save_libs="$LIBS"
  AC_MSG_CHECKING([for res_search])
  AC_FUNC_RES_SEARCH

  if test "$ac_cv_func_res_search" = no; then
      for lib in dns nsl resolv; do
        if test "$ac_cv_func_res_search" != yes; then
	  LIBS="-l$lib $LIBS"
          AC_FUNC_RES_SEARCH
          LIBS="$openafs_save_libs"
        fi
      done    
      if test "$ac_cv_func_res_search" = yes; then
        LIB_res_search="-l$lib"       
	AC_DEFINE(HAVE_RES_SEARCH, 1, [])
        AC_MSG_RESULT([yes, in lib$lib])
      else
        AC_MSG_RESULT(no)
      fi
  else
    AC_DEFINE(HAVE_RES_SEARCH, 1, [])
    AC_MSG_RESULT(yes)
  fi
  
fi

PTHREAD_LIBS=error
AC_CHECK_LIB(pthread, pthread_attr_init,
             PTHREAD_LIBS="-lpthread")
if test "x$PTHREAD_LIBS" = xerror; then
        AC_CHECK_LIB(pthreads, pthread_attr_init,
                PTHREAD_LIBS="-lpthreads")
fi
if test "x$PTHREAD_LIBS" = xerror; then
        AC_CHECK_LIB(c_r, pthread_attr_init,
                PTHREAD_LIBS="-lc_r")
fi
if test "x$PTHREAD_LIBS" = xerror; then
        AC_CHECK_FUNC(pthread_attr_init, PTHREAD_LIBS="")
fi
if test "x$PTHREAD_LIBS" = xerror; then
        AC_MSG_WARN(*** Unable to locate working posix thread library ***)
fi
AC_SUBST(PTHREAD_LIBS)

WITH_OBSOLETE=NO
if test "$enable_obsolete" = "yes"; then
	WITH_OBSOLETE=YES
fi

WITH_INSECURE=NO
if test "$enable_insecure" = "yes"; then
	WITH_INSECURE=YES
fi

# Fast restart
if test "$enable_fast_restart" = "yes"; then
	AC_DEFINE(FAST_RESTART, 1, [define if you want to have fast restart])
fi

if test "$enable_bitmap_later" = "yes"; then
	AC_DEFINE(BITMAP_LATER, 1, [define if you want to salvager to check bitmasks later])
fi

if test "$enable_full_vos_listvol_switch" = "yes"; then
	AC_DEFINE(FULL_LISTVOL_SWITCH, 1, [define if you want to want listvol switch])
fi

if test "$enable_bos_restricted_mode" = "yes"; then
	AC_DEFINE(BOS_RESTRICTED_MODE, 1, [define if you want to want bos restricted mode])
fi

if test "$enable_bos_new_config" = "yes"; then
	AC_DEFINE(BOS_NEW_CONFIG, 1, [define if you want to enable automatic renaming of BosConfig.new to BosConfig at startup])
fi

if test "$enable_namei_fileserver" = "yes"; then
	AC_DEFINE(AFS_NAMEI_ENV, 1, [define if you want to want namei fileserver])
fi

if test "$enable_afsdb" = "yes"; then
	LIB_AFSDB="$LIB_res_search"
	AC_DEFINE(AFS_AFSDB_ENV, 1, [define if you want to want search afsdb rr])
fi

dnl check for tivoli
AC_MSG_CHECKING(for tivoli tsm butc support)
XBSA_CFLAGS=""
if test "$enable_tivoli_tsm" = "yes"; then
	XBSADIR1=/usr/tivoli/tsm/client/api/bin/xopen
	XBSADIR2=/opt/tivoli/tsm/client/api/bin/xopen

	if test -e "$XBSADIR1/xbsa.h"; then
		XBSA_CFLAGS="-Dxbsa -I$XBSADIR1"
		AC_MSG_RESULT([yes, $XBSA_CFLAGS])
	elif test -e "$XBSADIR2/xbsa.h"; then
		XBSA_CFLAGS="-Dxbsa -I$XBSADIR2"
		AC_MSG_RESULT([yes, $XBSA_CFLAGS])
	else
		AC_MSG_RESULT([no, missing xbsa.h header file])
	fi
else
	AC_MSG_RESULT([no])
fi
AC_SUBST(XBSA_CFLAGS)

dnl checks for header files.
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_HEADER_DIRENT
AC_CHECK_HEADERS(stdlib.h string.h unistd.h fcntl.h sys/time.h sys/file.h)
AC_CHECK_HEADERS(netinet/in.h netdb.h sys/fcntl.h sys/mnttab.h sys/mntent.h)
AC_CHECK_HEADERS(mntent.h sys/vfs.h sys/param.h sys/fs_types.h)
AC_CHECK_HEADERS(sys/mount.h strings.h termios.h signal.h)
AC_CHECK_HEADERS(windows.h malloc.h winsock2.h direct.h io.h)
AC_CHECK_HEADERS(security/pam_modules.h siad.h usersec.h ucontext.h)

if test "$ac_cv_header_security_pam_modules_h" = "yes"; then
	HAVE_PAM="yes"
else
	HAVE_PAM="no"
fi
AC_SUBST(HAVE_PAM)

AC_CHECK_FUNCS(utimes random srandom getdtablesize snprintf re_comp re_exec)
AC_CHECK_FUNCS(setprogname getprogname sigaction mkstemp strerror)
AC_CHECK_TYPE(ssize_t, int)

AC_CHECK_FUNCS(timegm)

dnl Directory PATH handling
if test "x$enable_transarc_paths" = "xyes"  ; then 
    afsconfdir=${afsconfdir=/usr/afs/etc}
    viceetcdir=${viceetcdir=/usr/vice/etc}
    afskerneldir=${afskerneldir=${viceetcdir}}
    afssrvbindir=${afssrvbindir=/usr/afs/bin}
    afssrvsbindir=${afssrvsbindir=/usr/afs/bin}
    afssrvlibexecdir=${afssrvlibexecdir=/usr/afs/bin}
    afsdbdir=${afsdbdir=/usr/afs/db}
    afslogsdir=${afslogsdir=/usr/afs/logs}
    afslocaldir=${afslocaldir=/usr/afs/local}
    afsbackupdir=${afsbackupdir=/usr/afs/backup}
    afsbosconfigdir=${afsbosconfigdir=/usr/afs/local}
else 
    afsconfdir=${afsconfdir='${sysconfdir}/openafs/server'}
    viceetcdir=${viceetcdir='${sysconfdir}/openafs'}
    afskerneldir=${afskerneldir='${libdir}/openafs'}
    afssrvbindir=${afssrvbindir='${bindir}'}
    afssrvsbindir=${afssrvsbindir='${sbindir}'}
    afssrvlibexecdir=${afssrvlibexecdir='${libexecdir}/openafs'}
    afsdbdir=${afsdbdir='${localstatedir}/openafs/db'}
    afslogsdir=${afslogsdir='${localstatedir}/openafs/logs'}
    afslocaldir=${afslocaldir='${localstatedir}/openafs'}
    afsbackupdir=${afsbackupdir='${localstatedir}/openafs/backup'}
    afsbosconfigdir=${afsbosconfigdir='${sysconfdir}/openafs'}
fi
AC_SUBST(afsconfdir)
AC_SUBST(viceetcdir)
AC_SUBST(afskerneldir)
AC_SUBST(afssrvbindir)
AC_SUBST(afssrvsbindir)
AC_SUBST(afssrvlibexecdir)
AC_SUBST(afsdbdir)
AC_SUBST(afslogsdir)
AC_SUBST(afslocaldir)
AC_SUBST(afsbackupdir)
AC_SUBST(afsbosconfigdir)

if test "x$enable_kernel_module" = "xyes"; then
ENABLE_KERNEL_MODULE=libafs
fi

AC_SUBST(AFS_SYSNAME)
AC_SUBST(ENABLE_KERNEL_MODULE)
AC_SUBST(LIB_AFSDB)
AC_SUBST(LINUX_KERNEL_PATH)
AC_SUBST(LINUX_VERSION)
AC_SUBST(MKAFS_OSTYPE)
AC_SUBST(TOP_SRCDIR)
AC_SUBST(TOP_INCDIR)
AC_SUBST(TOP_LIBDIR)
AC_SUBST(DEST)
AC_SUBST(WITH_OBSOLETE)
AC_SUBST(WITH_INSECURE)
AC_SUBST(DARWIN_INFOFILE)
AC_SUBST(IRIX_BUILD_IP35)

])
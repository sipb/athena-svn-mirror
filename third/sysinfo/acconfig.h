/*
 * $Id: acconfig.h,v 1.1.1.1 2001-03-20 18:01:50 ghudson Exp $
 * 
 * acconfig.h
 */

#if	!defined(__autoconfig_h__)
#define __autoconfig_h__

/* printf("%lld") */
#undef HAVE_PRINTF_LL

/* config dir */
#undef CFDIR

/* OS version */
#undef OSVER

/* OS Type */
#undef OSTYPE

/* OS Major Version */
#undef OSMVER

/* Kernel ISA */
#undef KISA

/* Do we have DLPI support */
#undef HAVE_DLPI

/* ddi_prop_t */
#undef HAVE_DDI_PROP

/* struct ifnet */
#undef HAVE_IFNET

/* struct in_ifaddr */
#undef HAVE_IN_IFADDR

/* struct ether_addr */
#undef HAVE_ETHER_ADDR 

/* sysinfo() */
#undef HAVE_SYSINFO

/* struct anoninfo */
#undef HAVE_ANONINFO

/* dad kio */
#undef HAVE_DADKIO

/* ifnet->if_version */
#undef HAVE_IF_VERSION

/* <varargs.h> */
#undef HAVE_VARARGS

/* Have <kvm.h> */
#undef HAVE_KVM_H

@TOP@

@BOTTOM@

#if	!defined(HAVE_INT64_T) && defined(SIZEOF_INT64_T) && SIZEOF_INT64_T > 0
#define HAVE_INT64_T
#endif
#if	!defined(HAVE_UINT64_T) && defined(SIZEOF_UINT64_T) && SIZEOF_UINT64_T > 0
#define HAVE_UINT64_T
#endif

/*
 * Short cuts
 */
#if	!defined(HAVE_KSTAT) && defined(HAVE_LIBKSTAT)
#define HAVE_KSTAT
#endif
#if	!defined(HAVE_VOLMGT) && defined(HAVE_LIBVOLMGT)
#define HAVE_VOLMGT
#endif

#endif	/* __autoconfig_h__ */

/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/include.h,v 1.1 1994-03-09 08:27:20 vrt Exp $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.1  93/06/18  14:36:04  root
 * first cut at solaris port
 * 
 * Revision 2.0  92/04/22  01:45:08  tom
 * release 7.4
 * 	KERNEL define change for rs6000
 * 
 * Revision 1.1  90/05/26  13:42:15  tom
 * Initial revision
 * 
 * Revision 1.1  89/11/03  15:43:27  snmpdev
 * Initial revision
 * 
 */

/*
 * THIS SOFTWARE IS THE CONFIDENTIAL AND PROPRIETARY PRODUCT OF PERFORMANCE
 * SYSTEMS INTERNATIONAL, INC. ANY UNAUTHORIZED USE, REPRODUCTION, OR TRANSFER 
 * OF THIS SOFTWARE IS STRICTLY PROHIBITED.  COPYRIGHT (C) 1990 PSI, INC.  
 * (SUBJECT TO LIMITED DISTRIBUTION AND RESTRICTED DISCLOSURE ONLY.) 
 * ALL RIGHTS RESERVED.
 */
/*
 * THIS SOFTWARE IS THE CONFIDENTIAL AND PROPRIETARY PRODUCT OF NYSERNET,
 * INC.  ANY UNAUTHORIZED USE, REPRODUCTION, OR TRANSFER OF THIS SOFTWARE
 * IS STRICTLY PROHIBITED.  (C) 1989 NYSERNET, INC.  (SUBJECT TO 
 * LIMITED DISTRIBUTION AND RESTRICTED DISCLOSURE ONLY.)  ALL RIGHTS RESERVED.
 */

/*
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/include.h,v 1.1 1994-03-09 08:27:20 vrt Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988
 */
/*
 *  This file contains all of the system, local, and SNMP
 *  header files needed.  All source files should only have
 *  to include this.
 *
 */

#ifdef POSIX
#undef POSIX
#endif

#include "config.h"

#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socketvar.h>
#include <sys/protosw.h>
#ifndef SOLARIS
#include <sys/mbuf.h>
#endif /* SOLARIS */
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <sys/fcntl.h>

#ifdef MIT
#ifdef BSD
#undef BSD
#endif BSD
#include <sys/param.h>
#endif MIT

#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp_var.h>
#include <netinet/ip_var.h>

#include <nlist.h>
#include <ctype.h>
#include <strings.h>
#include <signal.h>
#include <setjmp.h>

#include <net/if.h>

#if defined(BSD43) || defined(ULTRIX2_2)
#include <netinet/in_var.h>
#endif BSD43 || ULTRIX2_2
/*
 *  we want all of route.h compiled in.
 */
#ifdef RSPOS
#define _KERNEL
#else  /* RSPOS */
#define KERNEL
#endif /* RSPOS */

#include <net/route.h>

#ifdef RSPOS
#undef _KERNEL
#else  /* RSPOS */
#undef  KERNEL
#endif /* RSPOS */

#include <netinet/in_pcb.h>
#include <netinet/tcp.h>
#include <netinet/tcpip.h>
#include <netinet/tcp_seq.h>
#include <netinet/tcp_fsm.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/tcp_debug.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <netinet/if_ether.h>

#ifdef RSPOS
#include <netinet/in_netarp.h>
#endif /* RSPOS */

#include <syslog.h>

#ifdef DEVELOP
#include "/usr/dev/src/snmp/v3.2/snmp/h/snmp_hs.h"
#else
#include "../../h/snmp_hs.h"
#endif DEVELOP

#include "var_tree.h"
#include "snmpd.h"


/*
 * ==========================================================================
 * Confidential and Proprietary.  Copyright 1987 by Apollo Computer Inc.,
 * Chelmsford, Massachusetts.  Unpublished -- All Rights Reserved Under
 * Copyright Laws Of The United States.
 *
 * Apollo Computer Inc. reserves all rights, title and interest with respect
 * to copying, modification or the distribution of such software programs
 * and associated documentation, except those rights specifically granted
 * by Apollo in a Product Software Program License, Source Code License
 * or Commercial License Agreement (APOLLO NETWORK COMPUTING SYSTEM) between
 * Apollo and Licensee.  Without such license agreements, such software
 * programs may not be used, copied, modified or distributed in source
 * or object code form.  Further, the copyright notice must appear on the
 * media, the supporting documentation and packaging as set forth in such
 * agreements.  Such License Agreements do not grant any rights to use
 * Apollo Computer's name or trademarks in advertising or publicity, with
 * respect to the distribution of the software programs without the specific
 * prior written permission of Apollo.  Trademark agreements may be obtained
 * in a separate Trademark License Agreement.
 * ==========================================================================
 *
 * File of standard "Unix" things.  This file include standard "Unix"
 * include files.  On systems that don't look sufficiently like bsd4.x,
 * various shenanigans are used to make it look more like bsd4.x.
 */

#ifndef STD_INCLUDED

#define STD_INCLUDED

/******************************************************************************/

#ifdef MSDOS
/*
 * These are defined here because we're running out of room on the command
 * line that invokes the Microsoft C compiler.
 */
#  define NON_REENTRANT_SOCKETS
#  define EMBEDDED_LLBD
#  define NO_STATS
#  define LESS_SPACE_MORE_TIME
#  define CONVENTIONAL_ALIGNMENT
#  if defined(MAX_DEBUG) && !defined(SOCKET_DEBUG)
#    define SOCKET_DEBUG
#  endif
#endif

/******************************************************************************/

#if defined(BSD) || defined(SYS5)
#  define UNIX
#endif

/******************************************************************************/

#if defined(UNIX) || defined(vms)
#  ifndef SYS5_SOCKETS_TYPE1
#    include <sys/types.h>
#    if defined(vms) && defined(__TYPES)
       /* VAXC types.h was used, must complement it with ucx$typedef.h */
#      include "ucx$typedef.h"
#    endif
#  else
#    include <sys/net/types.h>
#  endif
#endif

#if (defined(SYS5) && ! defined(SYS5_SOCKETS_TYPE5)) || defined(MSDOS)
#if defined(SYS5_SOCKETS_TYPE2) || defined(MSDOS)
typedef unsigned long u_long;
typedef unsigned short u_short;
typedef unsigned char u_char;
#endif
#if ! defined(SYS5_SOCKETS_TYPE3) && ! defined(SYS5_SOCKETS_TYPE6)
typedef unsigned u_int;
#endif
#endif

#ifdef _IBMR2
#include <sys/select.h>
#endif

/*
 * Select uses bit masks of file descriptors in longs.  These macros
 * manipulate such bit fields (the filesystem macros use chars).  FD_SETSIZE
 * may be defined by the user, but the default here should be >= NOFILE
 * (param.h).
 */

#ifndef FD_SETSIZE
#  if defined(apollo) && defined(pre_sr10)
#    define fd_set fd_set_nck
#    define FD_SETSIZE 256
#  else
#    define FD_SETSIZE 32
#  endif
#endif

#ifndef howmany
#  define howmany(x, y) (((x)+((y)-1))/(y))
#endif

#if (defined(apollo) && defined(pre_sr10)) || defined(MSDOS) || (defined(SYS5) && ! defined(SYS5_SOCKETS_TYPE5) && ! defined(SYS5_SOCKETS_TYPE6))
typedef struct fd_set {
    long fds_bits[howmany(FD_SETSIZE, sizeof(long))];
} fd_set;
#endif

#define FD_ZERO_MACRO(p) bzero((char *)(p), sizeof(*(p)))

#ifdef vms
/*
 * TWG has an "fdzero" function, but Excelan doesn't.  Don't use it to bzero for simplicity.
 */
#  define fdzero FD_ZERO_MACRO
#endif

#ifndef NFDBITS
#  define NFDBITS (sizeof(long) * 8) /* bits per mask */
#  define FD_SET(n, p)  ((p)->fds_bits[(n)/NFDBITS] |= (1L << ((n) % NFDBITS)))
#  define FD_CLR(n, p)  ((p)->fds_bits[(n)/NFDBITS] &= ~(1L << ((n) % NFDBITS)))
#  define FD_ISSET(n, p)((p)->fds_bits[(n)/NFDBITS] & (1L << ((n) % NFDBITS)))
#  define FD_ZERO FD_ZERO_MACRO
#endif

#define FD_COPY(src, dst) bcopy((char *)(src), (char *)(dst), sizeof(*(src)))

/******************************************************************************/

#ifndef SUPPRESS_STDH_SOCKET_INCLUDE

#if defined(BSD) || defined(vms) || defined(SYS5_SOCKETS_TYPE2) || defined(SYS5_SOCKETS_TYPE3) || defined(SYS5_SOCKETS_TYPE6)
#  include <sys/socket.h>
#  ifdef apollo
#    undef SO_BROADCAST
#  endif
#endif

#ifdef SYS5_SOCKETS_TYPE1
#  include <sys/net/socket.h>
#endif

#ifdef MSDOS

#  define SOCK_STREAM       1  /* stream socket */
#  define SOCK_DGRAM        2  /* datagram socket */
#  define SOCK_RAW          3  /* raw-protocol interface */
#  define SOCK_RDM          4  /* reliably-delivered message */
#  define SOCK_SEQPACKET    5  /* sequenced packet stream */

struct sockaddr {
    u_short sa_family;
    char sa_data[14];
};

#endif

#endif

/******************************************************************************/

/*
 * "close_socket" and "set_socket_non_blocking" are the canonical ways
 * to close something created by "socket" and to arrange that calling
 * "recvfrom" on a socket doesn't block if the socket is empty.  On
 * Unix system these are just aliased to "close" and "fcntl".  Other
 * systems might have other ways of doing this and must supply routines
 * with these names that have the intended affect.
 */

#ifdef UNIX
#  define close_socket close
#  define set_socket_non_blocking(sock) fcntl(sock, F_SETFL, O_NDELAY)
#endif

/******************************************************************************/

#if defined(vms) || (defined(SYS5) && ! defined(SYS5_HAS_SELECT))

/*
 * On some System V systems and some VAX/VMS TCP/IP systems, the socket
 * support contains routines named "select", "recvfrom", and "sendto" that
 * are busted.  We alias those names here.  Thus, NCK will contains external
 * references to routines named "..._nck" which can then in turn call the
 * non-"..._nck" versions (and deal with the bugs), if they chose to.
 */

#define select      select_nck
#define recvfrom    recvfrom_nck
#define sendto      sendto_nck

#endif


#ifdef cray

/*
 * Interludes are necessary due to the Cray's non-standard "struct sockaddr"
 * and "struct sockaddr_in" layouts.
 */

#define bind        bind_nck
#define sendto      sendto_nck
#define recvfrom    recvfrom_nck
#define getsockname getsockname_nck

#endif

#ifdef MSDOS

/*
 * Alias socket calls for MS/DOS.  Here's the motivation:
 *
 * MS/DOS NCK consists of two libraries:  a network-independent library
 * and a network-dependent library.  There is only one instance of the
 * first kind of library.  There are multiple instances of the second
 * library; e.g. one that interfaces with the Apollo DDS protocol, one
 * that interfaces with FTP Software's IP support, and one that interfaces
 * with Excelan's IP support.  An MS/DOS NCK-based application links with
 * the first library and exactly one of the second kind of library.
 *
 * We want it to be possible for people to create new network-dependent
 * libraries without having the entire set of NCK sources.  A
 * network-dependent library will typically interface with yet another
 * library supplied by (for example) the vendor of the IP software -- a
 * "vendor library".  The vendor library may or may not define routines
 * with the names "select", "sendto", etc.  If it does, these routines
 * may or may not work (at all, or correctly, or whatever).  If this is
 * the case, the writer of the network-dependent library will want to supply
 * his own shells over these routines that then perhaps call the vendor
 * library routines.  To make this possible, we must alias the routines
 * that you might want use as these shells.  For "perfect" vendor libraries,
 * you just have to write "..._nck" routines that simply call the vendor's
 * routines.
 *
 * NOTE: The functions select, sendto, and recvfrom are called from
 * an interrupt.  If their implementation is not (or might not be) reentrant,
 * some precaution must be taken ensure that they are not called while
 * any socket implementation code is executing.  Some effort to do this
 * is already made in the network-independent library (see "rpc_util.c").
 */

#define select      select_nck
#define sendto      sendto_nck
#define recvfrom    recvfrom_nck
#define socket      socket_nck
#define bind        bind_nck
#define getsockname getsockname_nck

#endif

/******************************************************************************/

#if defined(BSD) || defined(SYS5_SOCKETS_TYPE6)

#include <sys/time.h>

#else

#ifdef SYS5_SOCKETS_TYPE5

#include <time.h>

#else

#ifndef __SOCKET
struct timeval {
    long tv_sec;
    long tv_usec;
};
#endif

struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

#define timerclear(tvp) (tvp)->tv_sec = (tvp)->tv_usec = 0

#endif

#endif

/******************************************************************************/

#ifdef BSD
#  include <sys/errno.h>
#endif

#ifdef vms
#  include errno                /* VMS VAX C errno */
#  include <sys/errno.h>        /* VMS TWG errno */
#endif

#ifdef SYS5
#  ifdef SYS5_SOCKETS_TYPE1
#    include <sys/net/errno.h>
#  else
#    include <sys/errno.h>
#  endif
#endif

#ifdef MSDOS
#  define EINTR           4
#  define EBADF           9
#  define EFAULT          14
#  define EINVAL          22
#  define EMFILE          24
#  define EWOULDBLOCK     35
#  define ENOTSOCK        38
#  define EMSGSIZE        40
#  define ESOCKTNOSUPPORT 44
#  define EAFNOSUPPORT    47
#  define EADDRINUSE      48
#  define EADDRNOTAVAIL   49
#  define ENOBUFS         55
#endif

#if ! defined(vms) && ! defined(MSDOS)
extern int errno;
#endif

/******************************************************************************/

#include <signal.h>

#ifdef BSD
#  ifndef sigmask
#    define sigmask(m) (1L << ((m)-1))
#  endif
#endif

/******************************************************************************/

#if defined(UNIX) || defined(vms)
#  include <sys/file.h>
#endif

#ifndef vms
#  include <fcntl.h>
#endif

#ifndef MSDOS
#  define O_BINARY 0
#endif

#ifndef L_SET
#  define L_SET   0
#  define L_INCR  1
#  define L_XTND  2
#endif

extern long lseek();

/******************************************************************************/

#ifndef MSDOS

#include <setjmp.h>
#ifndef _JBLEN
#  define _JBLEN (sizeof(jmp_buf) / sizeof(int))
#endif

#else

#define _JBLEN 20                   /* Must match idl/pfm.h */
typedef char jmp_buf[_JBLEN];

#endif

/******************************************************************************/

#ifdef BSD
#  include <strings.h>
#endif

#if defined(SYS5) || defined(MSDOS) || defined(vms)
#  ifdef vms
#    define __STDDEF
#  endif
#  include <string.h>
#  define index strchr
#  define rindex strrchr
#endif

/******************************************************************************/

#if defined(SYS5) || defined(MSDOS)
#include <memory.h>
#endif

#if defined(SYS5) || defined(MSDOS) || defined(vms)
#define bcopy(src, dst, len) memcpy((dst), (src), (len))
#define bcmp memcmp
#define bzero(dst, len) memset((dst), 0, (len))
#else
#ifndef SOLARIS
extern bcopy();
extern bcmp();
extern bzero();
#endif
#endif

/******************************************************************************/

#include <stdio.h>

/******************************************************************************/

#include <assert.h>

#if defined(MSDOS) && !defined(MAX_DEBUG)
#  define ASSERT(cond)
#else
#  define ASSERT(cond)  assert(cond)
#endif

/******************************************************************************/

#ifdef MSDOS

#include <stdlib.h>
#include <malloc.h>

#else

#ifdef __STDC__
void *malloc(unsigned);
#else
char *malloc();
#endif

#endif

/******************************************************************************/

#ifdef MSDOS
#  ifdef NO_NEAR
#    define register
#    define NEAR
#  else
#    define NEAR   near
#  endif
#else
#  define NEAR
#endif

/******************************************************************************/

#ifdef MSDOS

/*
 * Override the following standard MS/DOS C library functions.
 */

#define time(tp) time_nck()      /*??? N.B. tp assumed to be NULL */
#define setjmp   setjmp_nck
#define longjmp  longjmp_nck

extern long time_nck(void);
extern long setjmp_nck(jmp_buf);

#endif

#endif

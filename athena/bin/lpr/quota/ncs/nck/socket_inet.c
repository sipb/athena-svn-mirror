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
 * S O C K E T _ I N E T
 *
 * These procedures implement the generic address family routines for Internet sockets.
 * This file is included by "socket.c".
 */

#ifdef INET

#define SUPPRESS_STDH_SOCKET_INCLUDE

#include "std.h"

#ifdef alliant
#  include <sys/mplock.h>
#endif

#if defined(BSD) || defined(vms) || defined(SYS5_SOCKETS_TYPE2) || defined(SYS5_SOCKETS_TYPE3) || defined(SYS5_SOCKETS_TYPE6)
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  ifdef UCX
#    include "ucx$ifdef.h"
#    include <sys/ucx$inetdef.h>
#  else
#    include <net/if.h>
#  endif
#  ifdef SYS5_SOCKETS_TYPE2
#    include <sys/bsdioctl.h>
#  else
#    ifndef UCX
#      include <sys/ioctl.h>
#    endif
#  endif
#  include <netdb.h>
#  ifdef SYS5_SOCKETS_TYPE3
#    undef SIOCGIFFLAGS
#  endif
#endif

#ifdef FTP
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#  define OLD_INET_ADDR_TYPE
#endif

#ifdef MICOM
#  include <pc_types.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
   typedef char* caddr_t;  /*??? missing from pc_types.h */
#  undef INADDR_ANY /* Use tcpioctl? */
#  define INADDR_ANY 0xffffffffL
#endif

#ifdef PC_EXCELAN
#  include <sys/socket.h>
#  include <netinet/in.h>
   long inet_addr(char *);
   char *inet_ntoa(struct in_addr);
   struct in_addr inet_makeaddr(long, long);
   long inet_netof(struct in_addr);
   u_long ntohl(), htonl();
#endif

#if defined(PC_EXCELAN) || defined(FTP)
#  define IN_CLASSA(i)  (((long)(i) & 0x80000000) == 0)
#  define IN_CLASSA_NET 0xff000000l
#  define IN_CLASSB(i)  (((long)(i) & 0xc0000000) == 0x80000000)
#  define IN_CLASSB_NET 0xffff0000l
#  define IN_CLASSC(i)  (((long)(i) & 0xc0000000) == 0xc0000000)
#  define IN_CLASSC_NET 0xffffff00l
#endif

#ifdef SYS5_SOCKETS_TYPE1
#  include <sys/net/socket.h>
#  include <sys/net/in.h>
#  include <sys/net/inet.h>
#  include <sys/net/netdb.h>
#endif

#ifdef SYS5_SOCKETS_TYPE5
#  include <netinet/in.h>
#  include "hp9000s300.h"
#  include <netdb.h>
#endif

#ifndef INADDR_BROADCAST
#  define INADDR_BROADCAST INADDR_ANY
#endif  

#if defined(sun) && defined(SIOCGIFBRDADDR) && ! defined(ifr_broadaddr)
/*
 * This might work for systems other than SunOS, but who knows...
 *
 * If we seem to be able to inquire an i/f's broadcast address but we don't
 * have "ifr_broadaddr" (typically a macro that extracts the "ifru_broadaddr"
 * field from a "struct ifreq"), then just alias "ifr_broadaddr" to
 * "ifr_addr".  Both fields are really just two different arms of a union
 * (i.e. equivalenced to the same storage).  Thus, the object code should
 * work both pre- and post- SunOS 4.0.
 */
#define ifr_broadaddr ifr_addr
#endif

#include "pbase.h"

#ifdef DSEE
#  include "$(socket.idl).h"
#else
#  include "socket.h"
#endif

#include "socket_p.h"


/*
 * the following foolishness is due to certain machine's "bogus" internal
 * definition of various components of a socket address and/or the interaction
 * with "sizeof".
 */
#define INET_SIZEOF_SIN_PORT    2               /* sizeof(struct sockaddr_in.sin_port) */
#define INET_SIZEOF_SIN_ADDR    4               /* sizeof(struct sockaddr_in.sin_addr) */
#define INET_SIZEOF_SIN_ZERO    8               /* sizeof(struct sockaddr_in.sin_zero) */
/*
 * sizeof socket_$addr_t.data for inet
 */
#define INET_SIZEOF_DATA    (INET_SIZEOF_SIN_PORT+INET_SIZEOF_SIN_ADDR+INET_SIZEOF_SIN_ZERO)

#ifdef cray
struct sockaddr_in_ncs {
    short   sin_family;
    u_int   sin_port: 16;
    u_int   sin_addr: 32;
};
#else
#  define sockaddr_in_ncs sockaddr_in
#endif

#if defined(apollo) && defined(pre_sr10)

/*
 * The following nightmarish procedures exist to deal with the fact
 * that some sites won't have "/lib/unixlib" and hence won't have the
 * procedures:  gethostbyaddr, gethostbyname, gethostname, inet_ntoa,
 * inet_addr.
 */

extern std_$call int (*kg_$lookup())();

internal struct hostent *gethostbyaddr(addr, len, type)
char *addr;
int len, type;
{
    int (*p)() = kg_$lookup("GETHOSTBYADDR                   ");
    return (p == NULL ? NULL : (struct hostent *) (*p)(addr, len, type));
}

internal struct hostent *gethostbyname(name)
int name;
{
    int (*p)() = kg_$lookup("GETHOSTBYNAME                   ");
    return (p == NULL ? NULL : (struct hostent *) (*p)(name));
}

internal gethostname(name, namelen)
int name;
int namelen;
{
    int (*p)() = kg_$lookup("GETHOSTNAME                     ");
    return (p == NULL ? -1 : (int) (*p)(name, namelen));
}

internal char *inet_ntoa(addr)
struct in_addr addr;
{
    int (*p)() = kg_$lookup("INET_NTOA                       ");
    return (p == NULL ? "?.?.?.?" : (char *) (*p)(addr));
}

internal int inet_addr(cp)
char *cp;
{
    int (*p)() = kg_$lookup("INET_ADDR                       ");
    return (p == NULL ? 0 : (int) (*p)(cp));
}

#endif


#if defined(apollo) && defined(GLOBAL_LIBRARY)
#  include "/us/ins/pm.ins.c"
#endif

#ifdef SYS5_SOCKETS_TYPE3
#  define SADDR_LONG(saddr) (saddr)->sin_addr
#else
#  define SADDR_LONG(saddr) (saddr)->sin_addr.s_addr
#endif

#define MAX_UDP_DGRAM   1024

#if defined(SIOCGIFCONF) && defined(SIOCGIFADDR) && defined(SIOCGIFFLAGS) && ! defined(EXCELAN)
#  define CAN_GET_INTERFACE_CONFIG
#endif


/*
 * Hairball stuff to deal with the fact that you can't take the address
 * of fields in packed structs and that on Crays, "struct sockaddr_in" is
 * a packed struct.
 */

#ifdef cray
#  define INT32TOCHARS(i) ((char *) &(i) + 4)
#else
#  define INT32TOCHARS(i) ((char *) &(i))
#endif

#define SET_SIN_ADDR(saddr, p) { \
    u_long sin_addr; \
    char *dst = INT32TOCHARS(sin_addr); \
    char *src = (char *) p; \
    *dst++ = *src++; \
    *dst++ = *src++; \
    *dst++ = *src++; \
    *dst++ = *src++; \
    SADDR_LONG(saddr) = sin_addr; \
}
#define MIN_SOCKADDR_LEN (socket_$sizeof_family + INET_SIZEOF_DATA)

#define check_sockaddr_format(saddr, slen, st) \
    if (slen < MIN_SOCKADDR_LEN) { \
        (st)->all = -1; \
        return; \
    }

#define MIN_NETADDR_LEN (socket_$sizeof_family + INET_SIZEOF_SIN_ADDR)

#define check_netaddr_format(saddr, slen, st) \
    if (slen < MIN_NETADDR_LEN) { \
        (st)->all = -1; \
        return; \
    }

#define MIN_HOSTID_LEN (socket_$sizeof_family + INET_SIZEOF_SIN_ADDR)

#define check_hostid_format(saddr, slen, st) \
    if (slen < MIN_HOSTID_LEN) { \
        (st)->all = -1; \
        return; \
    }

#ifdef CAN_GET_INTERFACE_CONFIG

#define IS_DULL_INTERFACE(s, ifreq) ( \
    (! is_up_interface(s, ifreq)) || \
    (! is_inet_interface(s, ifreq)) || \
    is_loopback_interface(s, ifreq) \
)

/*
 * is_loopback_interface
 */

static boolean is_loopback_interface(s, ifr)
int s;
struct ifreq *ifr;
{
    struct ifreq ifreq;

    ifreq = *ifr;

#ifdef IFF_LOOPBACK
    if (ioctl(s, SIOCGIFFLAGS, &ifreq) < 0)
        return (false);

    return ((ifreq.ifr_flags & IFF_LOOPBACK) != 0);
#else
    if (ioctl(s, SIOCGIFADDR, &ifreq) < 0)
        return (false);

    return (ifreq.ifr_addr.sa_data[2] == 127 && 
            ifreq.ifr_addr.sa_data[3] == 0  && 
            ifreq.ifr_addr.sa_data[4] == 0);
#endif
}


/*
 * is_up_interface
 */

static boolean is_up_interface(s, ifr)
int s;
struct ifreq *ifr;
{
    struct ifreq ifreq;

    ifreq = *ifr;

    if (ioctl(s, SIOCGIFFLAGS, &ifreq) < 0)
        return (false);

    return ((ifreq.ifr_flags & IFF_UP) != 0);
}


/*
 * is_inet_interface
 */

static boolean is_inet_interface(s, ifr)
int s;
struct ifreq *ifr;
{
    struct ifreq ifreq;

    ifreq = *ifr;

    if (ioctl(s, SIOCGIFADDR, &ifreq) < 0)
        return (false);

    return (ifreq.ifr_addr.sa_family == AF_INET);
}


/*
 * is_broadcast_interface
 */

static boolean is_broadcast_interface(s, ifr)
int s;
struct ifreq *ifr;
{
    struct ifreq ifreq;

    ifreq = *ifr;

    if (IS_DULL_INTERFACE(s, ifr))
        return (false);

    if (ioctl(s, SIOCGIFFLAGS, &ifreq) < 0)
        return (false);

    return ((ifreq.ifr_flags & IFF_BROADCAST) != 0);
}
                               
#endif

#if defined(PC_EXCELAN) || defined(EXCELAN)

/*
 * hostbyname (via Excelan rhost)
 */

internal u_long hostbyname(name)
char *name;
{       
    u_long rhost();
    u_long haddr;

#ifdef PC_EXCELAN
    FILE *dummy;
    int retval;

    /*???
     * Temp hack to get around bug in Excelan's raddr() and rhost()
     * functions (don't close "hosts" file).
     */
    dummy = fopen("con", "r");
    ASSERT(dummy != NULL);
    fclose(dummy);
#endif

    haddr = rhost(&name);

#ifdef PC_EXCELAN
    /*??? close file again, since rhost left one open */
    retval = fclose(dummy);
/*???
    ASSERT(retval == 0);
*/
#endif

    if (haddr != -1)
        free(name);

    return (haddr);
}

/*
 * hostbyaddr (via Excelan rhost)
 */

internal char *hostbyaddr(haddr)
u_long haddr;
{
    char *raddr();
    static char sname[64];
    char *name;

#ifdef PC_EXCELAN
    FILE *dummy;
    int retval;

    /*???
     * Temp hack to get around bug in Excelan's raddr() and rhost()
     * functions (don't close "hosts" file).
     */
    dummy = fopen("con", "r");
    ASSERT(dummy != NULL);
    fclose(dummy);
#endif

    name = raddr(haddr);

#ifdef PC_EXCELAN
    /*??? close file again, since raddr left one open */
    retval = fclose(dummy);
/*???
    ASSERT(retval == 0);
*/
#endif

    if (name == NULL) 
        return (NULL);

    strcpy(sname, name);
    free(name);
    return (sname);
}

#else

/*
 * hostbyname (via bsd gethostbyname)
 */

internal u_long hostbyname(name)
char *name;
{       
    char *p;
    struct hostent *he;
    u_long addr;
    
    he = gethostbyname(name);
    if (he == NULL)
        return (-1);

#if defined(apollo) && defined(GLOBAL_LIBRARY) && ! defined(pre_sr10)
    p = pm_$unix_release >= pm_$sr10_unix ? he->h_addr_list[0] : (char *) he->h_addr_list;
#else
    p = he->h_addr;
#endif
    /*
     * Avoid an unaligned longword fetch here, which does really evil
     * things on IBM RT's.
     */
    bcopy (p, &addr, sizeof(u_long));
    return addr;
}


/*
 * hostbyaddr (via bsd gethostbyaddr)
 */

internal char *hostbyaddr(haddr)
u_long haddr;
{
    struct hostent *he;

    if ((he = gethostbyaddr((char *) &haddr, sizeof(struct in_addr), AF_INET)) == NULL)
        return (NULL);

    return (he->h_name);
}

#endif


/*
 * inq_port
 */

internal u_long inet_$inq_port(xaddr, len, st)
struct socket_$addr_t *xaddr;
u_long len;
status_$t *st;
{
    struct sockaddr_in_ncs *addr = (struct sockaddr_in_ncs *) xaddr;

    check_sockaddr_format(addr, len, st);
    return (ntohs(addr->sin_port));
}


/*
 * set_port
 */

internal void inet_$set_port(xaddr, len, port, st)
struct socket_$addr_t *xaddr;
u_long *len;
u_long port;
status_$t *st;
{
    struct sockaddr_in_ncs *addr = (struct sockaddr_in_ncs *) xaddr;

    check_sockaddr_format(addr, *len, st);
    st->all = status_$ok;
    addr->sin_port = htons(port);
}


/*
 * set_wk_port
 */

internal void inet_$set_wk_port(xaddr, len, port, st)
struct socket_$addr_t *xaddr;
u_long *len;
u_long port;
status_$t *st;
{
    struct sockaddr_in_ncs *addr = (struct sockaddr_in_ncs *) xaddr;

    check_sockaddr_format(addr, *len, st);
    st->all = status_$ok;

    switch ((int) port) {
        case socket_$wk_fwd:
            addr->sin_port = htons(135);
            break;
    }
}


/*
 * from_name
 */

internal void inet_$from_name(name, namelen, numeric, port, xsaddr, slen, st)
char *name;
u_long namelen;
boolean numeric;
u_long port;
struct socket_$addr_t *xsaddr;
u_long *slen;
status_$t *st;
{
    struct sockaddr_in_ncs *saddr = (struct sockaddr_in_ncs *) xsaddr;
    char tname[100];
    u_long haddr;
    u_int b[4];
    char *p;

    if (namelen > sizeof(tname)) {
        st->all = socket_$buff_too_large;
        return;
    }

    st->all = status_$ok;

    bzero(saddr, *slen);
    saddr->sin_family = AF_INET;

    inet_$set_port(saddr, slen, port, st);

    strncpy(tname, name, (int) namelen);
    tname[namelen] = 0;

    if (numeric) {
        long addr;
#ifdef OLD_INET_ADDR_TYPE
        struct in_addr iaddr;
        iaddr = inet_addr(tname);
        addr = iaddr.s_addr;
#else
        addr = inet_addr(tname);
#endif

        if (addr == -1) {
            st->all = socket_$bad_numeric_name;
            return;
        }

        SADDR_LONG(saddr) = addr;
    }
    else {
        if ((haddr = hostbyname(tname)) == -1) {
            st->all = socket_$cant_find_name;
            return;
        }
        SET_SIN_ADDR(saddr, INT32TOCHARS(haddr));
    }

    *slen = socket_$sizeof_family + INET_SIZEOF_DATA;
}


/*
 * to_numeric_name
 */

internal void inet_$to_numeric_name(xsaddr, slen, name, namelen, st)
struct socket_$addr_t *xsaddr;
u_long slen;
char *name;
u_long *namelen;
status_$t *st;
{
    struct sockaddr_in_ncs *saddr = (struct sockaddr_in_ncs *) xsaddr;
    char *s;

    check_sockaddr_format(saddr, slen, st);
    s = inet_ntoa(saddr->sin_addr);

    if (*namelen < strlen(s) + 1) {
        st->all = socket_$buff_too_small;
        return;
    }

    strcpy(name, s);
    *namelen = strlen(name);
    st->all = status_$ok;
}


/*
 * to_name
 */

internal void inet_$to_name(xsaddr, slen, name, namelen, st)
struct socket_$addr_t *xsaddr;
u_long slen;
char *name;
u_long *namelen;
status_$t *st;
{
    struct sockaddr_in_ncs *saddr = (struct sockaddr_in_ncs *) xsaddr;
    char *hname;
    int len;

    check_sockaddr_format(saddr, slen, st);
    st->all = status_$ok;

    if (SADDR_LONG(saddr) == 0) {
        if (gethostname(name, *namelen) < 0) {
            st->all = socket_$cant_get_local_name;
            return;
        }
    }
    else {
        if ((hname = hostbyaddr(SADDR_LONG(saddr))) == NULL) {
            st->all = socket_$cant_cvrt_addr_to_name;
            return;
        }

        strncpy(name, hname, (int) *namelen);
    }

    len = strlen(name);
    if (len < *namelen)
        *namelen = len;
}


/*
 * max_pkt_size
 */

internal u_long inet_$max_pkt_size(family, st)
u_long family;
status_$t *st;
{
    st->all = status_$ok;
    return (MAX_UDP_DGRAM);
}


/*
 * inq_my_netaddr
 */

internal void inet_$inq_my_netaddr(family, naddr, nlen, st)
u_long family;
socket_$net_addr_t *naddr;
u_long *nlen;
status_$t *st;
{
#ifdef CAN_GET_INTERFACE_CONFIG
    int sock;
    char buf[1024];
    int n;
    struct ifconf ifc;
    struct ifreq *ifr, ifreq;
#else
    char host[100];
    u_long haddr;
#endif

    bzero(naddr->data, sizeof naddr->data);
    *nlen = socket_$sizeof_family + INET_SIZEOF_SIN_ADDR;

#ifdef CAN_GET_INTERFACE_CONFIG

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        st->all = socket_$cant_create_socket;
        return;
    }

    ifc.ifc_len = sizeof (buf);
    ifc.ifc_buf = buf;
    if (ioctl (sock, (int) SIOCGIFCONF, (caddr_t) &ifc) < 0) {
        close_socket(sock);
        st->all = socket_$cant_get_if_config;
        return;
    }

    n = ifc.ifc_len / sizeof (struct ifreq);
    for (ifr = ifc.ifc_req; --n >= 0; ifr++) {
        if (! IS_DULL_INTERFACE(sock, ifr)) {
            struct sockaddr_in *inet_addr = (struct sockaddr_in *) &ifr->ifr_addr;
            u_long sin_addr = SADDR_LONG(inet_addr);

            bcopy(INT32TOCHARS(sin_addr), naddr->data, INET_SIZEOF_SIN_ADDR);
            st->all = status_$ok;
            break;
        }
    }

    close_socket(sock);
    st->all = status_$ok;
    return;

#else

    if (gethostname(host, sizeof host) < 0) {
        st->all = socket_$cant_get_local_name;
        return;
    }

    if ((haddr = hostbyname(host)) == -1) {
        st->all = socket_$cant_find_name;
        return;
    }


    st->all = status_$ok;
    bcopy(INT32TOCHARS(haddr), naddr->data, INET_SIZEOF_SIN_ADDR);

#endif
}


/*
 * inq_netaddr
 */

internal void inet_$inq_netaddr(xsaddr, slen, naddr, nlen, st)
struct socket_$addr_t *xsaddr;
u_long slen;
socket_$net_addr_t *naddr;
u_long *nlen;
status_$t *st;
{
    struct sockaddr_in_ncs *saddr = (struct sockaddr_in_ncs *) xsaddr;
    u_long sin_addr = SADDR_LONG(saddr);

    check_sockaddr_format(saddr, slen, st);
    st->all = status_$ok;
    *nlen = socket_$sizeof_family + INET_SIZEOF_SIN_ADDR;
    bcopy(INT32TOCHARS(sin_addr), naddr->data, INET_SIZEOF_SIN_ADDR);
}


/*
 * set_netaddr
 */

internal void inet_$set_netaddr(xsaddr, slen, naddr, nlen, st)
struct socket_$addr_t *xsaddr;
u_long *slen;
socket_$net_addr_t *naddr;
u_long nlen;
status_$t *st;
{
    struct sockaddr_in_ncs *saddr = (struct sockaddr_in_ncs *) xsaddr;
    u_long sin_addr;

    check_netaddr_format(naddr, nlen, st);
    st->all = status_$ok;
    *slen = socket_$sizeof_family + INET_SIZEOF_DATA;
    bcopy(naddr->data, INT32TOCHARS(sin_addr), INET_SIZEOF_SIN_ADDR);
    SADDR_LONG(saddr) = sin_addr;
}


/*
 * inq_hostid
 */

internal void inet_$inq_hostid(xsaddr, slen, hid, hlen, st)
struct socket_$addr_t *xsaddr;
u_long slen;
socket_$host_id_t *hid;
u_long *hlen;
status_$t *st;
{
    struct sockaddr_in_ncs *saddr = (struct sockaddr_in_ncs *) xsaddr;
    u_long sin_addr = SADDR_LONG(saddr);

    check_sockaddr_format(saddr, slen, st);
    st->all = status_$ok;
    *hlen = socket_$sizeof_family + INET_SIZEOF_SIN_ADDR;
    bcopy(INT32TOCHARS(sin_addr), hid->data, INET_SIZEOF_SIN_ADDR);
}


/*
 * set_hostid
 */

internal void inet_$set_hostid(xsaddr, slen, hid, hlen, st)
struct socket_$addr_t *xsaddr;
u_long *slen;
socket_$host_id_t *hid;
u_long hlen;
status_$t *st;
{
    struct sockaddr_in_ncs *saddr = (struct sockaddr_in_ncs *) xsaddr;
    u_long sin_addr;

    check_hostid_format(hid, hlen, st);
    st->all = status_$ok;
    *slen = socket_$sizeof_family + INET_SIZEOF_DATA;
    bcopy(hid->data, INT32TOCHARS(sin_addr), INET_SIZEOF_SIN_ADDR);
    SADDR_LONG(saddr) = sin_addr;
}


/*
 * eq_network
 */

internal boolean inet_$eq_network(xsaddr1, slen1, xsaddr2, slen2, st)
struct socket_$addr_t *xsaddr1;
u_long slen1;
struct socket_$addr_t *xsaddr2;
u_long slen2;
status_$t *st;
{
    struct sockaddr_in_ncs *saddr1 = (struct sockaddr_in_ncs *) xsaddr1;
    struct sockaddr_in_ncs *saddr2 = (struct sockaddr_in_ncs *) xsaddr2;
    struct sockaddr_in_ncs sin_addr1, sin_addr2;
    u_long ia1, ia2;

    check_sockaddr_format(saddr1, slen1, st);
    check_sockaddr_format(saddr2, slen2, st);

    bcopy(saddr1, &sin_addr1, slen1);
    bcopy(saddr2, &sin_addr2, slen2);

    st->all = status_$ok;
    
    ia1 = ntohl(SADDR_LONG(&sin_addr1));
    ia2 = ntohl(SADDR_LONG(&sin_addr2));

    return (
        IN_CLASSA(ia1) && IN_CLASSA(ia2) &&
            (IN_CLASSA_NET & ia1) == (IN_CLASSA_NET & ia2) 
        ||
        IN_CLASSB(ia1) && IN_CLASSB(ia2) &&
            (IN_CLASSB_NET & ia1) == (IN_CLASSB_NET & ia2)
        ||
        IN_CLASSC(ia1) && IN_CLASSC(ia2) &&
            (IN_CLASSC_NET & ia1) == (IN_CLASSC_NET & ia2)
        );
}


/*
 * inq_broad_addrs
 */

internal void inet_$inq_broad_addrs(family, port, brd_addrs, brd_addrs_len, len, st)
u_long family;
u_long port;
socket_$addr_list_t brd_addrs;
socket_$len_list_t brd_addrs_len;
u_long *len;
status_$t *st;
{
    struct sockaddr_in_ncs *bap;
    struct sockaddr_in *inet_addr;
#ifdef CAN_GET_INTERFACE_CONFIG
    int sock;
    char buf[1024];
    struct ifconf ifc;
    struct ifreq *ifr, ifreq;
    int i, n;
#else
    char host[100];
    u_long haddr;
    struct sockaddr_in_ncs saddr;
#endif

    if (*len == 0) {
        st->all = socket_$buff_too_small;
        return;
    }

#ifdef CAN_GET_INTERFACE_CONFIG

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        st->all = socket_$cant_create_socket;
        return;
    }

    ifc.ifc_len = sizeof (buf);
    ifc.ifc_buf = buf;
    if (ioctl (sock, (int) SIOCGIFCONF, (caddr_t) &ifc) < 0) {
        close_socket(sock);
        st->all = socket_$cant_get_if_config;
        return;
    }

    n = ifc.ifc_len / sizeof (struct ifreq);

    i = 0;
    for (ifr = ifc.ifc_req; --n >= 0 && i < *len; ifr++)
    {
        bap = (struct sockaddr_in_ncs *) &brd_addrs[i];
        ifreq = *ifr;

        if (! is_broadcast_interface(sock, ifr))
            continue;

        brd_addrs_len[i] = socket_$sizeof_family + INET_SIZEOF_DATA;
        bap->sin_port   = htons(port);
        bap->sin_family = AF_INET;

#ifdef SIOCGIFBRDADDR
        if (ioctl(sock, (int) SIOCGIFBRDADDR, &ifreq) == -1) {
            perror("(socket_inet/set interface broadcast address): ");
            continue;
        }
        inet_addr = (struct sockaddr_in *) &ifreq.ifr_broadaddr;
        bcopy ((char *) &inet_addr->sin_addr, (char *) &bap->sin_addr, sizeof(struct in_addr));
#else
        if (ioctl(sock, (int) SIOCGIFADDR, &ifreq) == -1) {
            perror("(socket_inet/set interface address): ");
            continue;
        }
        inet_addr = (struct sockaddr_in *) &ifr->ifr_addr;
        bap->sin_addr = inet_makeaddr(inet_netof(inet_addr->sin_addr), INADDR_ANY);
#endif
        ++i;

    }
    *len = i;
    st->all = status_$ok;
    close_socket(sock);

#else

    if (gethostname(host, sizeof host) < 0) {
        st->all = socket_$cant_get_local_name;
        return;
    }

    if ((haddr = hostbyname(host)) == -1) {
        st->all = socket_$cant_find_name;
        return;
    }

    saddr.sin_port = htons(port);
    saddr.sin_family = AF_INET;
    saddr.sin_addr = inet_makeaddr(inet_netof(* (struct in_addr *) &haddr), (u_long) INADDR_BROADCAST);
    bcopy(&saddr, &brd_addrs[0], sizeof(struct socket_$addr_t));
    brd_addrs_len[0] = socket_$sizeof_family + INET_SIZEOF_DATA;

    *len = 1;
    st->all = status_$ok;

#endif
}


/*
 * to_local_rep
 */

internal void inet_$to_local_rep(saddr, lcl_saddr, st)
socket_$addr_t *saddr;
struct sockaddr *lcl_saddr;
status_$t *st;
{
    struct sockaddr_in_ncs *sin_ncs = (struct sockaddr_in_ncs *) saddr;
    struct sockaddr_in *sin = (struct sockaddr_in *) lcl_saddr;

    st->all = status_$ok;
    sin->sin_family = sin_ncs->sin_family;
    sin->sin_port = sin_ncs->sin_port;
    sin->sin_addr = sin_ncs->sin_addr;
}


/*
 * from_local_rep
 */

internal void inet_$from_local_rep(saddr, lcl_saddr, st)
socket_$addr_t *saddr;
struct sockaddr *lcl_saddr;
status_$t *st;
{
    struct sockaddr_in_ncs *sin_ncs = (struct sockaddr_in_ncs *) saddr;
    struct sockaddr_in *sin = (struct sockaddr_in *) lcl_saddr;

    st->all = status_$ok;
    sin_ncs->sin_family = sin->sin_family;
    sin_ncs->sin_port = sin->sin_port;
    sin_ncs->sin_addr = sin->sin_addr;
}


globaldef socket_$handler_rec_t inet_$socket_handler = {
        inet_$inq_port,
        inet_$set_port,
        inet_$set_wk_port,
        inet_$from_name,
        inet_$to_numeric_name,
        inet_$to_name,
        inet_$max_pkt_size,
        inet_$inq_my_netaddr,
        inet_$inq_netaddr,
        inet_$set_netaddr,
        inet_$inq_hostid,
        inet_$set_hostid,
        inet_$eq_network,
        inet_$inq_broad_addrs,
        inet_$to_local_rep,
        inet_$from_local_rep,
    };

#endif

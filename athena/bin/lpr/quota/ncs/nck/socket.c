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
 * S O C K E T
 *
 * Extensions to the BSD Unix socket abstraction to make it possible to
 * write address/protocol-independent applications.
 *
 */

#include "std.h"

#ifdef DSEE
#  include "$(nbase.idl).h"
#  include "$(socket.idl).h"
#else
#  include "nbase.h"
#  include "socket.h"
#endif

#include "socket_p.h"

#define AF_NMAX 14

internal socket_$handler_rec_t * NEAR socket_$handlers[AF_NMAX] = {
        NULL,                   /*  0 AF_UNSPEC    */
        NULL,                   /*  1 AF_UNIX      */
#ifdef INET
        &inet_$socket_handler,  /*  2 AF_INET      */
#else
        NULL,                   /*  2              */
#endif
        NULL,                   /*  3 AF_IMPLINK   */
        NULL,                   /*  4 AF_PUP       */
        NULL,                   /*  5 AF_CHAOS     */
        NULL,                   /*  6 AF_NS        */
        NULL,                   /*  7 AF_NBS       */
        NULL,                   /*  8 AF_ECMA      */
        NULL,                   /*  9 AF_DATAKIT   */
        NULL,                   /* 10 AF_CCITT     */
        NULL,                   /* 11 AF_SNA       */
        NULL,                   /* 12              */
#ifdef DDS
        &dds_$socket_handler,   /* 13 AF_DDS       */
#else
        NULL,                   /* 13              */
#endif
};


#define dispatch_family(family, op) \
    (*socket_$handlers[(short) family]->op)

#define dispatch_sockaddr(saddr, op) \
    dispatch_family((saddr)->family, op)

#define check_family(family, st) \
    if (! socket_$valid_family((u_long) family, st)) return

#define check_sockaddr(saddr, st) \
    check_family((saddr)->family, st)


/*
 * valid_sockaddr
 */

ndr_$boolean socket_$valid_family(family, st)
u_long family;
status_$t *st;
{
    if (family >= 0 &&
        family <= AF_NMAX - 1 &&
        socket_$handlers[(short) family] != NULL)
    {
        st->all = status_$ok;
        return(true);
    }
    else {
        st->all = socket_$family_not_valid;
        return(false);
    }
}


/*
 * valid_families - return the list of valid families
 */

void socket_$valid_families(num, families, st)
u_long *num;
socket_$addr_family_t families[];
status_$t *st;
{
    register int cur;
    register int i;

    st->all = status_$ok;

    cur = 0;
    for (i = 0; i < AF_NMAX; i++) {
        if (cur == *num) {
            st->all = socket_$buff_too_small;
            break;
        }
        if (socket_$handlers[(short) i] != NULL) {
            families[cur] = (socket_$addr_family_t) i;
            cur++;
        }
    }
    *num = cur;
}


/*
 * equal
 */

ndr_$boolean socket_$equal(addr1, len1, addr2, len2, flags, st)
socket_$addr_t *addr1, *addr2;
u_long len1, len2;
u_long flags;
status_$t *st;
{
    socket_$host_id_t hid1, hid2;
    u_long hlen1, hlen2;
    socket_$net_addr_t naddr1, naddr2;
    u_long nlen1, nlen2;

    st->all = status_$ok;

    if (addr1->family != addr2->family)
        return(false);

    if (! socket_$valid_family((u_long) addr1->family, st))
        goto BCMP;

    if (flags & socket_$eq_network) {
        boolean b = dispatch_sockaddr(addr1, eq_network)(addr1, len1, addr2, len2, st);
        if (st->all != status_$ok)
            goto BCMP;
        if (! b)
            return(false);
    }
    else if (flags & socket_$eq_netaddr) {
        socket_$inq_netaddr(addr1, len1, &naddr1, &nlen1, st);
        if (st->all != status_$ok)
            goto BCMP;
        socket_$inq_netaddr(addr2, len2, &naddr2, &nlen2, st);
        if (st->all != status_$ok)
            goto BCMP;
        if (nlen1 != nlen2 || bcmp((char *) &naddr1, (char *) &naddr2, (int) nlen1) != 0)
            return(false);
    }
    else if (flags & socket_$eq_hostid) {
        socket_$inq_hostid(addr1, len1, &hid1, &hlen1, st);
        if (st->all != status_$ok)
            goto BCMP;
        socket_$inq_hostid(addr2, len2, &hid2, &hlen2, st);
        if (st->all != status_$ok)
            goto BCMP;
        if (hlen1 != hlen2 || bcmp((char *) &hid1, (char *) &hid2, (int) hlen1) != 0)
            return(false);
    }

    if (flags & socket_$eq_port)
        if (socket_$inq_port(addr1, len1, st) != socket_$inq_port(addr2, len2, st))
            return(false);

    return(true);

BCMP:

    st->all = status_$ok;

    return (len1 == len2 &&
            bcmp((char *) addr1, (char *) addr2, (int) len1) == 0
            );
}


/*
 * inq_port
 */

ndr_$ulong_int socket_$inq_port(saddr, slen, st)
socket_$addr_t *saddr;
u_long slen;
status_$t *st;
{
    check_sockaddr(saddr, st);
    return(dispatch_sockaddr(saddr, inq_port)(saddr, slen, st));
}


/*
 * set_port
 */

void socket_$set_port(saddr, slen, port, st)
socket_$addr_t *saddr;
u_long *slen;
u_long port;
status_$t *st;
{
    check_sockaddr(saddr, st);
    *slen = socket_$sizeof_family + socket_$sizeof_data; /* Ignore input length; set to max len */
    dispatch_sockaddr(saddr, set_port)(saddr, slen, port, st);
}


/*
 * set_wk_port
 */

void socket_$set_wk_port(saddr, slen, port, st)
socket_$addr_t *saddr;
u_long *slen;
u_long port;
status_$t *st;
{
    check_sockaddr(saddr, st);
    *slen = socket_$sizeof_family + socket_$sizeof_data; /* Ignore input length; set to max len */
    dispatch_sockaddr(saddr, set_wk_port)(saddr, slen, port, st);
}


/*
 * family_from_name
 */

ndr_$ulong_int socket_$family_from_name(name, namelen, st)
socket_$string_t name;
u_long namelen;
status_$t *st;
{
    st->all = status_$ok;

#define STREQL(cs) \
    (namelen == sizeof(cs) - 1 && strncmp((char *) name, (cs), (int) namelen) == 0)

    if (STREQL("dds"))
        return(socket_$dds);
    if (STREQL("ip"))
        return(socket_$internet);
    if (STREQL("ns"))
        return(socket_$ns);
    if (STREQL("unspec"))
        return(socket_$unspec);

#undef STREQL

    st->all = socket_$family_not_valid;
    return(-1);
}


/*
 * family_to_name
 */

void socket_$family_to_name(family, name, namelen, st)
u_long family;
socket_$string_t name;
u_long *namelen;
status_$t *st;
{
    char *p;
    register int len;

    switch ((int) family) {
        case socket_$unspec:
            p = "unspec";
            break;
        case socket_$internet:
            p = "ip";
            break;
        case socket_$ns:
            p = "ns";
            break;
        case socket_$dds:
            p = "dds";
            break;
        default:
            st->all = socket_$family_not_valid;
            return;
    }

    strncpy((char *) name, p, (int) *namelen);
    len = strlen(p);
    if (len < *namelen)
        *namelen = len;
    st->all = status_$ok;
}


/*
 * from_name
 */

void socket_$from_name(family, name, namelen, port, saddr, slen, st)
u_long family;
socket_$string_t name;
u_long namelen;
u_long port;
socket_$addr_t *saddr;
u_long *slen;
status_$t *st;
{
    char *p, *q;
    u_long lport;
    char buf[276];

    *slen = socket_$sizeof_family + socket_$sizeof_data; /* Ignore input length; set to max len */

    /* make a copy of the caller's string so that we can be free to zap it */

    if (namelen >= sizeof(buf)) {
        st->all = socket_$buff_too_large;
        return;
    }
    strncpy(buf, (char *) name, sizeof(buf));
    buf[namelen] = '\0';
    name = (ndr_$char *) buf;

    if (family == socket_$unspec) {
        if ((p = index((char *) name, ':')) == NULL) {
            st->all = socket_$invalid_name_format;
            return;
        }

        *p = 0;     /* Zap the ":" */

        family = socket_$family_from_name(name, (u_long) (p - (char *) name), st);
        if (st->all != status_$ok)
            return;

        namelen -= p - ((char *) name) + 1;
        name = (ndr_$char *) p + 1;
    }

    check_family(family, st);
    saddr->family = family;

    if ((p = index((char *) name, '[')) == NULL)
        lport = port;
    else {
        *p = 0;     /* Zap the "[" */

        if ((q = index(p + 1, ']')) == NULL) {
            st->all = socket_$invalid_name_format;
            return;
        }

        *q = 0;     /* Zap the "]" */
        lport = atoi(p + 1);
        namelen = p - (char *) name;
    }

    if (*name != 0) {
        boolean numeric = (name[0] == '#');
        if (numeric)
            name++;
        dispatch_family(family, from_name)(name, namelen, numeric, lport, saddr, slen, st);
    }
    else {
        socket_$net_addr_t naddr;
        u_long nlen = socket_$sizeof_family + socket_$sizeof_ndata;     /* set to max length */

        socket_$inq_my_netaddr(family, &naddr, &nlen, st);
        socket_$set_netaddr(saddr, slen, &naddr, nlen, st);
        socket_$set_port(saddr, slen, lport, st);
    }
}


/*
 * to_numeric_name
 */

void socket_$to_numeric_name(saddr, slen, name, namelen, port, st)
socket_$addr_t *saddr;
u_long slen;
socket_$string_t name;
u_long *namelen;
u_long *port;
status_$t *st;
{
    socket_$string_t tname, fname;
    u_long tnamelen = sizeof(tname);
    u_long fnamelen = sizeof(fname);
    char buf[276];
    int buflen;

    check_sockaddr(saddr, st);

    socket_$family_to_name((u_long) saddr->family, fname, &fnamelen, st);
    if (st->all != status_$ok)
        return;
    fname[fnamelen] = '\0';

    dispatch_sockaddr(saddr, to_numeric_name)(saddr, slen, tname, &tnamelen, st);
    if (st->all != status_$ok)
        return;
    tname[tnamelen] = '\0';

    sprintf(buf, "%s:#%s", fname, tname);
    strncpy((char *) name, buf, (int) *namelen);
    buflen = strlen(buf);
    if (*namelen > buflen) {
        *namelen = strlen((char *) name);
    }
    *port = socket_$inq_port(saddr, slen, st);
}


/*
 * to_name
 */

void socket_$to_name(saddr, slen, name, namelen, port, st)
socket_$addr_t *saddr;
u_long slen;
socket_$string_t name;
u_long *namelen;
u_long *port;
status_$t *st;
{
    socket_$string_t tname, fname;
    u_long tnamelen = sizeof(tname);
    u_long fnamelen = sizeof(fname);
    char buf[276];
    int buflen;

    check_sockaddr(saddr, st);

    socket_$family_to_name((u_long) saddr->family, fname, &fnamelen, st);
    if (st->all != status_$ok)
        return;
    fname[fnamelen] = '\0';

    dispatch_sockaddr(saddr, to_name)(saddr, slen, tname, &tnamelen, st);
    if (st->all == status_$ok) {
        tname[tnamelen] = '\0';
        sprintf(buf, "%s:%s", fname, tname);
    }
    else {
        dispatch_sockaddr(saddr, to_numeric_name)(saddr, slen, tname, &tnamelen, st);
        if (st->all != status_$ok)
            return;
        tname[tnamelen] = '\0';
        sprintf(buf, "%s:#%s", fname, tname);
    }

    strncpy((char *) name, buf, (int) *namelen);
    buflen = strlen(buf);
    if (*namelen > buflen) {
        *namelen = strlen((char *) name);
    }
    *port = socket_$inq_port(saddr, slen, st);
}


/*
 * set_broadcast
 */

void socket_$set_broadcast(addr, len, st)
socket_$addr_t *addr;
u_long *len;
status_$t *st;
{
    u_long n_addrs = 1;
    socket_$addr_t brd_addrs[1];
    u_long brd_lens[1];
    u_long port;

    port = socket_$inq_port(addr, *len, st);
    if (st->all != status_$ok)
        return;

    socket_$inq_broad_addrs((u_long) addr->family, port, brd_addrs, brd_lens, &n_addrs, st);
    if (st->all != status_$ok)
        return;

    *addr = brd_addrs[0];
    *len = brd_lens[0];
}


/*
 * max_pkt_size
 */

ndr_$ulong_int socket_$max_pkt_size(family, st)
u_long family;
status_$t *st;
{
    check_family(family, st);
    return(dispatch_family(family, max_pkt_size)(family, st));
}


/*
 * inq_my_netaddr
 */

void socket_$inq_my_netaddr(family, naddr, nlen, st)
u_long family;
socket_$net_addr_t *naddr;
u_long *nlen;
status_$t *st;
{
    check_family(family, st);
    *nlen = socket_$sizeof_family + socket_$sizeof_ndata; /* Ignore input length; set to max len */
    bzero(naddr, sizeof *naddr);
    naddr->family = family;
    dispatch_family(family, inq_my_netaddr)(family, naddr, nlen, st);
}


/*
 * inq_netaddr
 */

void socket_$inq_netaddr(saddr, slen, naddr, nlen, st)
socket_$addr_t *saddr;
u_long slen;
socket_$net_addr_t *naddr;
u_long *nlen;
status_$t *st;
{
    check_sockaddr(saddr, st);
    *nlen = socket_$sizeof_family + socket_$sizeof_ndata; /* Ignore input length; set to max len */
    bzero(naddr, sizeof *naddr);
    naddr->family = saddr->family;
    dispatch_sockaddr(saddr, inq_netaddr)(saddr, slen, naddr, nlen, st);
}


/*
 * set_netaddr
 */

void socket_$set_netaddr(saddr, slen, naddr, nlen, st)
socket_$addr_t *saddr;
u_long *slen;
socket_$net_addr_t *naddr;
u_long nlen;
status_$t *st;
{
    saddr->family = naddr->family;
    *slen = socket_$sizeof_family + socket_$sizeof_data; /* Ignore input length; set to max len */
    check_sockaddr(saddr, st);
    dispatch_sockaddr(saddr, set_netaddr)(saddr, slen, naddr, nlen, st);
}


/*
 * inq_hostid
 */

void socket_$inq_hostid(saddr, slen, hid, hlen, st)
socket_$addr_t *saddr;
u_long slen;
socket_$host_id_t *hid;
u_long *hlen;
status_$t *st;
{
    check_sockaddr(saddr, st);
    *hlen = socket_$sizeof_family + socket_$sizeof_hdata; /* Ignore input length; set to max len */
    bzero(hid, sizeof *hid);
    hid->family = saddr->family;
    dispatch_sockaddr(saddr, inq_hostid)(saddr, slen, hid, hlen, st);
}


/*
 * set_hostid
 */

void socket_$set_hostid(saddr, slen, hid, hlen, st)
socket_$addr_t *saddr;
u_long *slen;
socket_$host_id_t *hid;
u_long hlen;
status_$t *st;
{
    saddr->family = hid->family;
    *slen = socket_$sizeof_family + socket_$sizeof_data; /* Ignore input length; set to max len */
    check_sockaddr(saddr, st);
    dispatch_sockaddr(saddr, set_hostid)(saddr, slen, hid, hlen, st);
}

/*
 * inq_broad_addrs
 */

void socket_$inq_broad_addrs(family, port, brd_addrs, brd_addrs_len, len, st)
u_long family;
u_long port;
socket_$addr_list_t brd_addrs;
socket_$len_list_t brd_addrs_len;
u_long *len;
status_$t *st ;
{
    check_family(family, st) ;
    dispatch_family(family, inq_broad_addrs)(family, port, brd_addrs, brd_addrs_len, len, st) ;
}

/*
 * to_local_rep
 */

void socket_$to_local_rep(saddr, _lcl_saddr, st)
socket_$addr_t *saddr;
socket_$local_sockaddr_t _lcl_saddr;
status_$t *st ;
{
    struct sockaddr *lcl_saddr = (struct sockaddr *) _lcl_saddr;

    check_sockaddr(saddr, st) ;
    dispatch_sockaddr(saddr, to_local_rep)(saddr, lcl_saddr, st) ;
}

/*
 * from_local_rep
 */

void socket_$from_local_rep(saddr, _lcl_saddr, st)
socket_$addr_t *saddr;
socket_$local_sockaddr_t _lcl_saddr;
status_$t *st ;
{
    struct sockaddr *lcl_saddr = (struct sockaddr *) _lcl_saddr;

    check_sockaddr((socket_$addr_t *)lcl_saddr, st) ;
    dispatch_sockaddr((socket_$addr_t *)lcl_saddr, from_local_rep)(saddr, lcl_saddr, st) ;
}


#ifdef FTN_INTERLUDES

ndr_$boolean socket_$valid_family_(family, st)
u_long *family;
status_$t *st;
{
    socket_$valid_family(*family, st);
}

void socket_$valid_families_(num, families, st)
u_long *num;
socket_$addr_family_t families[];
status_$t *st;
{
    socket_$valid_families(num, families, st);
}

ndr_$boolean socket_$equal_(addr1, len1, addr2, len2, flags, st)
socket_$addr_t *addr1, *addr2;
u_long *len1, *len2;
u_long *flags;
status_$t *st;
{
    socket_$equal(addr1, *len1, addr2, *len2, *flags, st);
}

ndr_$ulong_int socket_$inq_port_(saddr, slen, st)
socket_$addr_t *saddr;
u_long *slen;
status_$t *st;
{
    socket_$inq_port(saddr, *slen, st);
}

void socket_$set_port_(saddr, slen, port, st)
socket_$addr_t *saddr;
u_long *slen;
u_long *port;
status_$t *st;
{
    socket_$set_port(saddr, slen, *port, st);
}

void socket_$set_wk_port_(saddr, slen, port, st)
socket_$addr_t *saddr;
u_long *slen;
u_long *port;
status_$t *st;
{
    socket_$set_wk_port(saddr, slen, *port, st);
}

ndr_$ulong_int socket_$family_from_name_(name, namelen, st)
socket_$string_t name;
u_long *namelen;
status_$t *st;
{
    socket_$family_from_name(name, *namelen, st);
}

void socket_$family_to_name_(family, name, namelen, st)
u_long *family;
socket_$string_t name;
u_long *namelen;
status_$t *st;
{
    socket_$family_to_name(*family, name, namelen, st);
}

void socket_$from_name_(family, name, namelen, port, saddr, slen, st)
u_long *family;
socket_$string_t name;
u_long *namelen;
u_long *port;
socket_$addr_t *saddr;
u_long *slen;
status_$t *st;
{
    socket_$from_name(*family, name, *namelen, *port, saddr, slen, st);
}

void socket_$to_numeric_name_(saddr, slen, name, namelen, port, st)
socket_$addr_t *saddr;
u_long *slen;
socket_$string_t name;
u_long *namelen;
u_long *port;
status_$t *st;
{
    socket_$to_numeric_name(saddr, *slen, name, namelen, port, st);
}

void socket_$to_name_(saddr, slen, name, namelen, port, st)
socket_$addr_t *saddr;
u_long *slen;
socket_$string_t name;
u_long *namelen;
u_long *port;
status_$t *st;
{
    socket_$to_name(saddr, *slen, name, namelen, port, st);
}

void socket_$set_broadcast_(addr, len, st)
socket_$addr_t *addr;
u_long *len;
status_$t *st;
{
    socket_$set_broadcast(addr, len, st);
}

ndr_$ulong_int socket_$max_pkt_size_(family, st)
u_long *family;
status_$t *st;
{
    socket_$max_pkt_size(*family, st);
}

void socket_$inq_my_netaddr_(family, naddr, nlen, st)
u_long *family;
socket_$net_addr_t *naddr;
u_long *nlen;
status_$t *st;
{
    socket_$inq_my_netaddr(*family, naddr, nlen, st);
}

void socket_$inq_netaddr_(saddr, slen, naddr, nlen, st)
socket_$addr_t *saddr;
u_long *slen;
socket_$net_addr_t *naddr;
u_long *nlen;
status_$t *st;
{
    socket_$inq_netaddr(saddr, *slen, naddr, nlen, st);
}

void socket_$set_netaddr_(saddr, slen, naddr, nlen, st)
socket_$addr_t *saddr;
u_long *slen;
socket_$net_addr_t *naddr;
u_long *nlen;
status_$t *st;
{
    socket_$set_netaddr(saddr, slen, naddr, *nlen, st);
}

void socket_$inq_hostid_(saddr, slen, hid, hlen, st)
socket_$addr_t *saddr;
u_long *slen;
socket_$host_id_t *hid;
u_long *hlen;
status_$t *st;
{
    socket_$inq_hostid(saddr, *slen, hid, hlen, st);
}

void socket_$set_hostid_(saddr, slen, hid, hlen, st)
socket_$addr_t *saddr;
u_long *slen;
socket_$host_id_t *hid;
u_long *hlen;
status_$t *st;
{
    socket_$set_hostid(saddr, slen, hid, *hlen, st);
}

void socket_$inq_broad_addrs_(family, port, brd_addrs, brd_addrs_len, len, st)
u_long *family;
u_long *port;
socket_$addr_list_t brd_addrs;
socket_$len_list_t brd_addrs_len;
u_long *len;
status_$t *st ;
{
    socket_$inq_broad_addrs(*family, *port, brd_addrs, brd_addrs_len, len, st);
}

void socket_$to_local_rep_(saddr, _lcl_saddr, st)
socket_$addr_t *saddr;
socket_$local_sockaddr_t _lcl_saddr;
status_$t *st ;
{
    socket_$to_local_rep(saddr, _lcl_saddr, st);
}

void socket_$from_local_rep_(saddr, _lcl_saddr, st)
socket_$addr_t *saddr;
socket_$local_sockaddr_t _lcl_saddr;
status_$t *st ;
{
    socket_$from_local_rep(saddr, _lcl_saddr, st);
}

#endif


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
 * S O C K E T _ D D S
 *
 * These routines implement the generic address family routines for DDS sockets
 * This file is included by "socket.c".
 */

#ifdef DDS

#include "std.h"

#ifndef MSDOS
#  include <netinet/in.h>
#else
#  define htons(s) swab_$short(s)
#  define ntohs(s) swab_$short(s)
#  define htonl(l) swab_$long(l)
#  define ntohl(l) swab_$long(l)
extern short swab_$short(short);
extern long swab_$long(long);
#endif

#include "pbase.h"

#ifdef DSEE
#  include "$(socket.idl).h"
#else
#  include "socket.h"
#endif

#include "socket_p.h"

#ifdef MSDOS
#  include "xport.h"
#  define DDSHOSTS_FILE "c:\\ncs\\ddshosts.txt"
#endif

#define internal static

/*
 * Extracted from "/us/ins/task.ins.c".
 */

#define AF_DDS    13
#define dds_$broadcast_node 0xefffffff

struct dds_addr {
    long        network;
    long        node;
    };

struct sockaddr_dds {
    short           sdds_family;
    short           sdds_port;
    struct dds_addr sdds_addr;
    };

struct dds_net_addr {
    short family;
    struct dds_addr na;
};

struct dds_host_id {
    short family;
    long node;
};


#define MIN_SOCKADDR_LEN sizeof(struct sockaddr_dds)

#define check_sockaddr_format(saddr, slen, st) \
    if (slen < MIN_SOCKADDR_LEN) { \
        (st)->all = socket_$buff_too_small; \
        return; \
    }

#define MIN_NETADDR_LEN 8

#define check_netaddr_format(saddr, slen, st) \
    if (slen < MIN_NETADDR_LEN) { \
        (st)->all = socket_$buff_too_small; \
        return; \
    }

#define MIN_HOSTID_LEN 4

#define check_hostid_format(saddr, slen, st) \
    if (slen < MIN_HOSTID_LEN) { \
        (st)->all = socket_$buff_too_small; \
        return; \
    }

/*** Local functions: */
#ifdef MSDOS
internal boolean get_one_host_entry(FILE *fp, u_long* node, char *host_name);
#endif

/*
 * inq_port
 */

internal u_long dds_$inq_port(addr, len, st)
struct sockaddr_dds *addr;
u_long len;
status_$t *st;
{
    check_sockaddr_format(addr, len, st);
    st->all = status_$ok;
    return ((long) ntohs(addr->sdds_port));
}


/*
 * set_port
 */

internal void dds_$set_port(addr, len, port, st)
struct sockaddr_dds *addr;
u_long *len;
u_long port;
status_$t *st;
{
    check_sockaddr_format(addr, *len, st);
    st->all = status_$ok;
    addr->sdds_port = htons((short) port);
}


/*
 * set_wk_port
 */

internal void dds_$set_wk_port(addr, len, port, st)
struct sockaddr_dds *addr;
u_long *len;
u_long port;
status_$t *st;
{
    check_sockaddr_format(addr, *len, st);
    st->all = status_$ok;
    switch ((int) port) {
        case socket_$wk_fwd:
            addr->sdds_port = htons(12);
            break;
    }
}


/*
 * from_name
 */

internal void dds_$from_name(name, namelen, numeric, port, saddr, slen, st)
char *name;
u_long namelen;
boolean numeric;
u_long port;
struct sockaddr_dds *saddr;
u_long *slen;
status_$t *st;
{
#ifdef apollo
    uid_$t uid;
    std_$call void name_$resolve(), file_$locatei();
    std_$call msg_$get_my_net();
#endif

    saddr->sdds_family = AF_DDS;

    dds_$set_port(saddr, slen, port, st);

    if (numeric) {
        if (sscanf(name, "%lx.%lx", &saddr->sdds_addr.network, &saddr->sdds_addr.node) < 2) {
            st->all = socket_$bad_numeric_name;
            return;
        }
        saddr->sdds_addr.network = htonl(saddr->sdds_addr.network);
        saddr->sdds_addr.node = htonl(saddr->sdds_addr.node);
    }
    else {
#ifdef apollo
        name_$resolve(*name, (short) namelen, uid, *st);
        if (st->all != status_$ok)
            return;

        file_$locatei(uid, saddr->sdds_addr, *st);
        if (st->all != status_$ok)
            return;

        if (saddr->sdds_addr.network == 0) {
            msg_$get_my_net(saddr->sdds_addr.network);
        }
#endif
#ifdef MSDOS
        /*
         * The following lookup uses the MSDOS filesystem and hence
         * is not re-entrant (cannot be called from an interrupt).
         * Example host name dbase file:
         *     d5d1  jig
         *     aeb5  argus
         *     ed4a  sinker
         */

        FILE *fp;
        char *fname;
        u_long node;
        char host_name[100];

        st->all = socket_$cant_find_name;
        fname = DDSHOSTS_FILE;
        if ((fp = fopen(fname, "r")) == NULL)
            return;
        if (name[0] == '/' && name[1] == '/')
            name += 2;
        while (get_one_host_entry(fp, &node, host_name)) {
            if (strcmp(name, host_name) == 0) {
                saddr->sdds_addr.node = htonl(node);
                saddr->sdds_addr.network = 0;  /* local net for now ???*/
                st->all = status_$ok;
                break;
            }
        }
        fclose(fp);
#endif
    }

    *slen = sizeof(struct sockaddr_dds);
}


/*
 * to_numeric_name
 */

internal void dds_$to_numeric_name(saddr, slen, name, namelen, st)
struct sockaddr_dds *saddr;
u_long slen;
char *name;
u_long *namelen;
status_$t *st;
{
    check_sockaddr_format(saddr, slen, st);

    if (*namelen < sizeof("nnnnn.nnnnn")) {
        st->all = socket_$buff_too_small;
        return;
    }

    sprintf(name, "%lx.%lx", ntohl(saddr->sdds_addr.network), ntohl(saddr->sdds_addr.node));
    *namelen = strlen(name);
    st->all = status_$ok;
}


/*
 * to_name
 */

internal void dds_$to_name(saddr, slen, name, namelen, st)
struct sockaddr_dds *saddr;
u_long slen;
char *name;
u_long *namelen;
status_$t *st;
{
#ifdef apollo
    std_$call void name_$find_uid_cc(), name_$resolve();
    extern void get_node_root();
    name_$pname_t lname;
    short l;
    uid_$t uid, ss;

    check_sockaddr_format(saddr, slen, st);

    get_node_root(saddr->sdds_addr.node, saddr->sdds_addr.network, &uid, st);
    if (st->all != status_$ok)
        return;

    name_$resolve("//", (short) 2, ss, *st);
    if (st->all != status_$ok)
        return;

    lname[0] = lname[1] = '/';

    name_$find_uid_cc(ss, uid, lname[2], l, *st);
    if (st->all != status_$ok)
        return;

    l += 2;     /* Account for leading "//" */

    if (l < *namelen) {
        bcopy(lname, name, (int) l);
        name[l] = 0;                /* Gag me with a spoon */
        *namelen = (u_long) l;
    }
    else
        bcopy(lname, name, (int) *namelen);
#endif
#ifdef MSDOS
        /*
         * The following lookup uses the MSDOS filesystem and hence
         * is not re-entrant (cannot be called from an interrupt).
         */
        FILE *fp;
        char *fname;
        u_long node;
        char host_name[100];

        st->all = status_$ok;
        fname = DDSHOSTS_FILE;
        if ((fp = fopen(fname, "r")) == NULL) {
            st->all = socket_$cant_cvrt_addr_to_name;
            return;
        }
        while (get_one_host_entry(fp, &node, host_name)) {
            if (node == ntohl(saddr->sdds_addr.node)) {
                strcpy(name, "//");
                strncat(name, host_name, (int) *namelen-2);
                fclose(fp);
                return;
            }
        }
        fclose(fp);
        st->all = socket_$cant_cvrt_addr_to_name;
#endif
}

#ifdef MSDOS

/*
 * read a line from the ddshosts.txt file.
 * ignore:
 *     - comments (from '#' to end of line)
 *     - empty lines
 *     - lines with bad syntax
 * Example host name dbase file:
 *    # Sample ddshosts file
 *    d5d1  jig     # 570T
 *    aeb5  argus   # dn3000
 *    ed4a  sinker  # pc
 */
internal boolean get_one_host_entry(fp, node, host_name)
FILE *fp;
u_long* node;
char *host_name;
{
    char line[256];
    char *cp;

    while (fgets(line, sizeof(line), fp) != NULL) {
        if ((cp = strchr(line, '#')) != NULL)
            *cp = '\0';
        if (sscanf(line, "%lx %s", node, host_name) != 2)
            continue;
        if (host_name[0] == '/' && host_name[1] == '/')
            memmove(host_name, &host_name[2], strlen(host_name)-1);
        return(true);
    }
    return(false);
}
#endif

/*
 * max_pkt_size
 */

internal u_long dds_$max_pkt_size(family, st)
u_long family;
status_$t *st;
{
    st->all = status_$ok;
#ifdef apollo
    return (1024);
#endif
#ifdef MSDOS
    /*** The following constant is ses_data_max_size in xptypes.h of
         Apollo xport code */
    return (1024 - (4 + 2 + 6));
#endif
}


/*
 * inq_my_netaddr
 */

internal void dds_$inq_my_netaddr(family, naddr, nlen, st)
u_long family;
struct dds_net_addr *naddr;
u_long *nlen;
status_$t *st;
{
#ifdef apollo
    std_$call msg_$get_my_net(), msg_$get_my_node();

    st->all = status_$ok;
    *nlen = sizeof *naddr;

    msg_$get_my_net(naddr->na.network);
    msg_$get_my_node(naddr->na.node);
#endif
#ifdef MSDOS
    xport_$tcb_rep rtcb;
    xport_$tcb tcb = &rtcb;
    xport_$status_rep rstat;
    xport_$status stat = &rstat;
    extern void far xport_$null_handler();

    *nlen = sizeof *naddr;
    xport_$zero_tcb(tcb);
    xport_$zero_status(stat);

    /*
     * Get status from xport driver (returns my addr in status buffer)
     */

    tcb->command = XPORT_$STATUS;
    tcb->cid = 0;
    tcb->length = sizeof(xport_$status_rep);
    tcb->baddr = (char far *) stat;
    tcb->async = xport_$null_handler;

    if (xport_$command(tcb) != 0) {
        st->all = socket_$internal_error;  /* error, can't get status */
        return;
    }
    naddr->na.network = htonl(* ((long *) &stat->net_number[0]));
    naddr->na.node = htonl(* ((long *) &stat->host_address[0]));
    st->all = status_$ok;
#endif
}


/*
 * inq_netaddr
 */

internal void dds_$inq_netaddr(saddr, slen, naddr, nlen, st)
struct sockaddr_dds *saddr;
u_long slen;
struct dds_net_addr *naddr;
u_long *nlen;
status_$t *st;
{
    check_sockaddr_format(saddr, slen, st);
    st->all = status_$ok;
    *nlen = sizeof *naddr;
    naddr->na = saddr->sdds_addr;
}


/*
 * set_netaddr
 */

internal void dds_$set_netaddr(saddr, slen, naddr, nlen, st)
struct sockaddr_dds *saddr;
u_long *slen;
struct dds_net_addr *naddr;
u_long nlen;
status_$t *st;
{
    check_netaddr_format(naddr, nlen, st);
    st->all = status_$ok;
    *slen = sizeof *saddr;
    saddr->sdds_addr = naddr->na;
}


/*
 * inq_hostid
 */

internal void dds_$inq_hostid(saddr, slen, hid, hlen, st)
struct sockaddr_dds *saddr;
u_long slen;
struct dds_host_id *hid;
u_long *hlen;
status_$t *st;
{
    check_sockaddr_format(sadd, slen, st);
    st->all = status_$ok;
    *hlen = sizeof *hid;
    hid->node = saddr->sdds_addr.node;
}


/*
 * set_hostid
 */

internal void dds_$set_hostid(saddr, slen, hid, hlen, st)
struct sockaddr_dds *saddr;
u_long *slen;
struct dds_host_id *hid;
u_long hlen;
status_$t *st;
{
    check_hostid_format(hid, hlen, st);
    st->all = status_$ok;
    *slen = sizeof *saddr;
    saddr->sdds_addr.node = hid->node;
}


/*
 * eq_network
 */

internal boolean dds_$eq_network(saddr1, slen1, saddr2, slen2, st)
struct sockaddr_dds *saddr1;
u_long slen1;
struct sockaddr_dds *saddr2;
u_long slen2;
status_$t *st;
{
    check_sockaddr_format(saddr1, slen1, st);
    check_sockaddr_format(saddr2, slen2, st);

    st->all = status_$ok;
    if (saddr1->sdds_addr.network == saddr2->sdds_addr.network)
        return true;

    return false;
}


internal void dds_$inq_broad_addrs(family, port, brd_addrs, brd_addrs_len, len, st)
u_long family;
u_long port;
socket_$addr_list_t brd_addrs;
socket_$len_list_t brd_addrs_len;
u_long *len;
status_$t *st;
{  
    struct sockaddr_dds *addr;

    if (*len == 0) {
        st->all = socket_$buff_too_small;
        return;
    }

    st->all = status_$ok;
      
    brd_addrs_len[0] = sizeof(struct sockaddr_dds);
    addr = (struct sockaddr_dds *) brd_addrs;
    addr->sdds_family       = family;
    addr->sdds_port         = htons((short) port);
    addr->sdds_addr.network = 0;
    addr->sdds_addr.node    = htonl(dds_$broadcast_node);
    *len = 1;
}

/*
 * to_local_rep
 */
internal void dds_$to_local_rep(saddr, lcl_saddr, st)
socket_$addr_t *saddr;
struct sockaddr *lcl_saddr;
status_$t *st;
{
    st->all = status_$ok;
    *lcl_saddr = *(struct sockaddr *)saddr;
}

/*
 * from_local_rep
 */
internal void dds_$from_local_rep(saddr, lcl_saddr, st)
socket_$addr_t *saddr;
struct sockaddr *lcl_saddr;
status_$t *st;
{
    st->all = status_$ok;
    *saddr = *(socket_$addr_t *)lcl_saddr;
}


globaldef socket_$handler_rec_t dds_$socket_handler = {
        dds_$inq_port,
        dds_$set_port,
        dds_$set_wk_port,
        dds_$from_name,
        dds_$to_numeric_name,
        dds_$to_name,
        dds_$max_pkt_size,
        dds_$inq_my_netaddr,
        dds_$inq_netaddr,
        dds_$set_netaddr,
        dds_$inq_hostid,
        dds_$set_hostid,
        dds_$eq_network,
        dds_$inq_broad_addrs,
        dds_$to_local_rep,
        dds_$from_local_rep,
};

#endif

/*      llbd
 *      Location Server - Local/Forwarding Agent - main program
 *
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
 */

#ifdef DSEE
#   include "$(nbase.idl).h"
#   include "$(lb.idl).h"
#   include "$(llb.idl).h"
#   include "$(socket.idl).h"
#   include "$(rproc.idl).h"
#   include "$(rrpc.idl).h"
#else
#   include "nbase.h"
#   include "lb.h"
#   include "llb.h"
#   include "socket.h"
#   include "rproc.h"
#   include "rrpc.h"
#endif

#include "std.h"

#include "lb_p.h"
#include "llb_man.h"

boolean         llb_$debug = false;
extern uuid_$t  uuid_$nil;                        
#define version  "llbd/nck version 1.5.1"


#define MAX_SOCKETS 2 /* Number of sockets to listen on. Currently */
                      /* allow only dds & ip sockets.  If you want */
                      /* to listen on more families, increase this */
                      /* count.                                    */

#define dprintf if (llb_$debug) printf

/*
 * Number of sockets to listen on. Currently allow only dds & ip sockets.
 * If you want to listen on more families, increase this count.
 */ 

#define MAX_SOCKETS 2 
                      
/*
 * The llbd will wake up periodically to see if any random chores need to
 * be done.  WAKEUP_INTERVAL is the number of seconds between wake-ups.
 * Currently, the llbd doesn't actually have any random chores to do,
 * so this number is large.
 */

#define WAKEUP_INTERVAL (60 * 60 * 24)

/*
 * The following is a table of all the interfaces that the llbd supports
 * locally.  I.e. the llbd does not try to forward calls to operations in
 * these interfaces to any other server process.
 */

static struct if_t {
    rpc_$if_spec_t *if_spec;
    rpc_$epv_t *server_epv;
};

#ifdef UNIX

#define N_LLBD_IFS 3

static struct if_t llbd_ifs[N_LLBD_IFS] = {
    {&llb_$if_spec, &llb_$server_epv},
    {&rproc_$if_spec, &rproc_$server_epv},
    {&rrpc_$if_spec, &rrpc_$server_epv}
};

#else

#define N_LLBD_IFS 2

static struct if_t llbd_ifs[N_LLBD_IFS] = {
    {&llb_$if_spec, &llb_$server_epv},
    {&rrpc_$if_spec, &rrpc_$server_epv}
};

#endif

/*
 * The following table will hold the list of address families to listen to.
 * n_families is the size of the table (an empty table implies that all
 * available address families should be tried). 
 */

static socket_$addr_family_t    families[MAX_SOCKETS];
static u_long                   n_families = 0;

static boolean match_command ( key, str, min_len )
    char *key;
    char *str;
    int min_len;
{
    int i = 0;

    if (*key) while (*key == *str) {
        i++;
        key++;
        str++;
        if (*str == '\0' || *key == '\0')
            break;
    }
    if (*str == '\0' && i >= min_len)
        return true;

    return false;
}

static boolean
get_entry(object, obj_type, obj_interface, from_addr, target_entry)
    uuid_$t     *object;
    uuid_$t     *obj_type;
    uuid_$t     *obj_interface;
    socket_$addr_t *from_addr;
    lb_$entry_t *target_entry;
{
    lb_$lookup_handle_t handle;
    u_long              num_results;
    lb_$entry_t         result;
    boolean             found;
    int                 specificity;
    int                 target_specificity;
    status_$t           s;
    extern uuid_$t      uuid_$nil;

    handle = lb_$default_lookup_handle;
    found = false;
    do {
        llb_$lookup_match(NULL, object, obj_type, obj_interface, &handle,
                        1, false, &num_results, &result, &s);
        if (num_results == 1) {
            if (from_addr->family == result.saddr.family) {
                specificity = 0;
                if (!EQ_UID(result.object, uuid_$nil))
                    specificity += 0x100;
                if (!EQ_UID(result.obj_type, uuid_$nil))
                    specificity += 0x10;
                if (!EQ_UID(result.obj_interface, uuid_$nil))
                    specificity += 0x1;
                if (!found) {
                    found = true;
                    *target_entry = result;
                    target_specificity = specificity;
                } else if (specificity > target_specificity) {
                    *target_entry = result;
                    target_specificity = specificity;
                }
            }
        }
    } while (num_results == 1);

    return found;
}
                                                  
static 
set_fds(socket_mask, socket_array, n_sockets) 
   fd_set   *socket_mask;
   int      *socket_array;
   int      n_sockets;
{
   int      i ;

   for (i=0; i<n_sockets; i++)
      FD_SET(socket_array[i], socket_mask) ;
}

static 
find_socket(socket_mask, socket_array, n_sockets)
   fd_set   *socket_mask;
   int      *socket_array;
   int      n_sockets;
{
   int      i ;

   for (i=0; i<n_sockets; i++) {
      if (FD_ISSET(socket_array[i], socket_mask)) {
         FD_CLR(socket_array[i], socket_mask) ;
         return socket_array[i] ;
      }
   }

   dprintf("LLBD: (find_socket) called with no ready sockets\n") ;
   exit(2) ;
}

static 
process_requests(socket_array, n_sockets)
     int    *socket_array ;
     int    n_sockets ;
{
    status_$t       s;
    lb_$entry_t     target;
    rpc_$ppkt_t     packet;
    rpc_$cksum_t    cksum;
    socket_$addr_t  from_addr;
    u_long          from_len;
    uuid_$t         object;
    uuid_$t         interface;
    u_long          ptype;
    fd_set          readfds;
    int             nfound;
    int             sock;
    boolean         handled_it;
    u_short         i;

    FD_ZERO(&readfds);

    while (true) {
        set_fds(&readfds, socket_array, n_sockets) ;
        nfound = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
        if (nfound < 0)
            if (errno != EINTR)
                return;
            else
                continue;
            
        while (nfound > 0) {
            sock = find_socket(&readfds, socket_array, n_sockets) ;
            nfound--;

            rpc_$listen_recv((u_long) sock, &packet, &cksum, &from_addr, &from_len, 
                                    &ptype, &object, &interface, &s);
            if (!STATUS_OK(&s)) {
                rpc_$status_print("LLBD: ", s);
                continue;
            }
            if (llb_$debug) {
                print_socket_info("(process_requests) input from",
                                        &from_addr, from_len);
            }

            /*
            ** See if we support the interface ourselves.  If we do, dispatch
            ** it.  Otherwise, see if there's some other registered server
            ** capable of handling the request and if there is, forward it
            ** to it.
            */

            handled_it = false;

            for (i = 0; i < N_LLBD_IFS; i++)
                if (EQ_UID(interface, llbd_ifs[i].if_spec->id)) {
                    handled_it = true;
                    rpc_$listen_dispatch((u_long) sock, &packet, cksum, &from_addr, from_len, &s);
                    if (!STATUS_OK(&s))
                        rpc_$status_print("LLBD: ", s);
                    break;
                }
    
            if (! handled_it && get_entry(&object, NULL, &interface, &from_addr, &target)) {
                if (llb_$debug) {
                    print_socket_info("(process_requests) forwarding to",
                                            &target.saddr, target.saddr_len);
                }
                rpc_$forward((u_long) sock, &from_addr, from_len, &target.saddr, 
                                   target.saddr_len, &packet, &s);
                if (!STATUS_OK(&s)) {
                    rpc_$status_print("LLBD: ", s);
                }
            }
    
            /*
            ** Otherwise, we just drop the packet - maybe someone
            ** else can handle it on another machine.  
            */
        }
    }
}

static  
print_socket_info(info, addr, len)
    char    *info;
    socket_$addr_t *addr;
    int     len;
    
{
    socket_$string_t name;
    u_long namelen = sizeof(name);
    u_long port;
    status_$t s;

    socket_$to_name(addr, len, name, &namelen, &port, &s);
    if (s.all != status_$ok) {
        strcpy((char *) name, "< name not available >");
        namelen = strlen((char *) name);
    }
    printf("%s %.*s[%d]\n", info, namelen, name, port);
}

static int
use_family(family)
    int family;
{
    int sock;
    socket_$addr_t saddr;
    u_long slen;
    status_$t st;

    sock = socket(family, SOCK_DGRAM, 0);
    if (sock < 0) {
        dprintf("LLBD: (use_family) Can't create socket\n");
        return -1;
    }

    slen = sizeof(saddr);
    bzero(&saddr, slen);

    saddr.family = family;
    socket_$set_wk_port(&saddr, &slen, (u_long) socket_$wk_fwd, &st);

    if (bind(sock, &saddr, slen) < 0) {
        close_socket(sock);
        dprintf("LLBD: (use_family) Can't bind socket\n");
        return -1;
    }

    if (llb_$debug) {
        print_socket_info("Listening on:", &saddr, slen); 
    }

    return sock;
}

static void
clear_listen_families ( )
{
    n_families = 0;
}

static void listen_family ( fname, st )
    char        *fname;
    status_$t   *st;
{
    u_long      len;
    u_long      family;

    len = strlen(fname);
    family = socket_$family_from_name((ndr_$char *) fname, len, st);
    if (st->all == status_$ok) {
        if (!(n_families < MAX_SOCKETS)) {
            printf("LLBD: Too many families specified, %s ignored\n", fname);
        } else {
            families[n_families++] = (socket_$addr_family_t) family;
        }
    }
}

static void get_families ( st )
    status_$t   *st;
{
    /*
     * If the address families were not selected on the command line,
     * then we should listen to all valid families.
     */
    if (n_families == 0) {
        n_families = MAX_SOCKETS;
        socket_$valid_families(&n_families, families, st);
    } else {
        st->all == status_$ok;
    }
}

static int
open_sockets(socket_array, n_sockets)
    int   *socket_array;
    int   *n_sockets;
{
    int   sock, i;
    status_$t st;

    *n_sockets = 0;

    get_families(&st);

    for (i = 0; i < n_families; i++) {
        sock = use_family(families[i]);
        if (sock == -1) {
            socket_$string_t fname;
            u_long fnamelen = sizeof fname;

            socket_$family_to_name(families[i], fname, &fnamelen, &st);
            dprintf("LLBD: unable to obtain %.*s family port\n", fnamelen, fname);
        }
        else {
            socket_array[(*n_sockets)++] = sock;
        }
    }
}

static int
close_sockets(socket_array, n_sockets) 
   int   *socket_array ;
   int   n_sockets ;
{
   int   i ;

   for (i=0; i<n_sockets; i++)
      close_socket(socket_array[i]) ;
}

main(argc,argv)
    int argc;
    char *argv[];
{
    status_$t status;
    extern boolean rpc_$debug;
    int  sockets[MAX_SOCKETS];
    int  n_sockets;
    u_short i;

    lb_$process_args_i(argc, argv, NULL, NULL, version);
    llb_$debug = rpc_$debug;

    /*
    ** If not debugging, fork off a process to be the llbd.  The parent exits.
    */

    if (!llb_$debug) {
#ifdef apollo
        std_$call pm_$set_my_name();
        static char name[] = "llbd";
        short namelen = sizeof(name)-1;
#endif
#ifdef UNIX
        int pid;
#endif

#ifdef UNIX
        pid = fork();
        if (pid > 0)
            exit(0);
#endif 
#ifdef SYS5
        setpgrp();
#endif
#ifdef apollo
        pm_$set_my_name(name, namelen, status);
#endif
    }

    /*
    ** Register all the interfaces we implement ourselves.
    */

    for (i = 0; i < N_LLBD_IFS; i++) {
        rpc_$register(llbd_ifs[i].if_spec, *llbd_ifs[i].server_epv, &status);
        if (!STATUS_OK(&status))
            rpc_$status_print("Can't register interface: ", status);
    }

    /*
     * Allow the user to restrict the set of address families that should be 
     * used.
     */
    for (i = 1; i < argc; i++) {
        if (match_command("-listen", argv[i], 2)) {
            clear_listen_families();
            while ((i+1) < argc) {
                if (*argv[i + 1] == '-') 
                    break;
                i++;
                listen_family(argv[i], &status);
                if (status.all != status_$ok) {
                    printf("LLBD: Unable to use family %s\n", argv[i]);
                }
             }
        }
    }

    /*
    ** Start our real work.
    */

    while (true) {
        open_sockets(sockets, &n_sockets);
        if (n_sockets == 0) {
            printf("LLBD: Unable to obtain any sockets\n");
            exit(2);
        }

        process_requests(sockets, n_sockets);
        close_sockets(sockets, n_sockets);
    }
}

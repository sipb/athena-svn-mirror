/*
 * The FX (File Exchange) Server
 *
 * $Author: danw $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/neos/server/main.c,v $
 * $Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/server/main.c,v 1.5 1998-02-17 19:48:53 danw Exp $
 *
 * Copyright 1989, 1990 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include <mit-copyright.h>

/*
 * This file contains routines used to start the server and handle
 * incoming client connections.
 */

#ifndef lint
static char rcsid_main_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/neos/server/main.c,v 1.5 1998-02-17 19:48:53 danw Exp $";
#endif /* lint */

#include <fxserver.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/file.h>
#include <signal.h>

/* Some rpcgen's, including NetBSD's, have timeout support.  If you
 * generate a server stub file with no main function, it leaves in
 * the code to set and unset _rpcsvcdirty, but declares the variable
 * extern.  So we have to define it ourselves. */
int _rpcsvcdirty;

struct _Connection Connection[NOFILE], *curconn;
int curconn_num;

char my_hostname[256], my_canonhostname[256];

server_stats stats;

char *root_dir;

#ifdef MULTI
int do_update_server = 0;
#endif /* MULTI */

#ifdef MALLOC_LEAK
void dump_mem()
{
    malloc_dump_all("./fxserver");
    exit(0);
}
#endif /* MALLOC_LEAK */

#ifdef MULTI
void child_dead()
{
    int pid, i;
    extern int update_server_npids;
    extern int update_server_pids[64]; /* XXX */

    pid = wait(0); /* XXX Should be wait3 maybe? */
    DebugMulti(("Pid %d has died!\n", pid));
    for (i=0; i<update_server_npids; i++) {
	if (update_server_pids[i] == pid) {
	    memmove(update_server_pids+i, update_server_pids+i+1,
		  (update_server_npids-i-1)*sizeof(int));
	    update_server_npids--;
	    DebugMulti(("We got one!  npids is %d\n", update_server_npids));
	    return;
	}
    }
}
#endif /* MULTI */
	    
main(argc, argv)
    int argc;
    char *argv[];
{
    void fxserver_1();
    SVCXPRT *transp;
    int i, nfound;
    fd_set readfds;
    struct hostent *hent;
    struct sigaction action;
#ifdef MULTI
    struct timeval seltimeout;
    int elapsed;
#endif /* MULTI */

    TOUCH(argc);
    TOUCH(argv);

    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
#ifdef MALLOC_LEAK
    action.sa_handler = dump_mem;
    sigaction(SIGFPE, &action, NULL);
#endif /* MALLOC_LEAK */

    root_dir = ROOT_DIR;
    
    /*
     * Ignore broken pipes due to dropped TCP connections
     */
    action.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &action, NULL);

    /*
     * Get my hostname.
     */

    if (gethostname(my_hostname, sizeof(my_hostname)))
	fatal("Can't get hostname of local host");

    hent = gethostbyname(my_hostname);
    if (!hent)
	fatal("Can't canonicalize local host name");

    (void) strncpy(my_canonhostname, hent->h_name, sizeof(my_canonhostname));
    my_canonhostname[sizeof(my_canonhostname) - 1] = '\0';

#ifdef MULTI
    action.sa_handler = child_dead;
    sigaction(SIGCHLD, &action, NULL);
    multi_init();
#endif /* MULTI */
    
    memset(&stats, 0, sizeof(stats));
    stats.start_time = time(0);
    
    init_fxsv_err_tbl();

    db_init();
    
    /*
     * Do RPC initialization stuff.
     */
    
    Debug(("Unmapping with portmapper\n"));
    
    pmap_unset(FXSERVER, FXVERS);

    Debug(("Creating tcp socket to listen on\n"));
    
    transp = svctcp_create(RPC_ANYSOCK, 0, 0);
    if (transp == NULL) {
      fprintf(stderr, "cannot create tcp service.\n");
      exit(1);
    }
    Debug(("Registering with portmapper\n"));
    if (!svc_register(transp, FXSERVER, FXVERS, fxserver_1, IPPROTO_TCP)) {
      fprintf(stderr, "unable to register (FXSERVER, FXVERS, tcp).\n");
      exit(1);
    }

    memset(Connection, 0, sizeof(Connection));
    
#ifdef MULTI
    /*
     * Initialize multi-server support.
     */
    multi_ping_servers();
    memset(&seltimeout, 0, sizeof(seltimeout));
    
#endif /* MULTI */

    log_info("Server up");
    
    Debug(("Entering main dispatch loop\n"));

    for (;;) {
	for (i=0; i<NOFILE; i++) {
	    if (!FD_ISSET(i, &svc_fdset) &&
		(Connection[i].inited || Connection[i].server_num)) {
		Debug(("Dropping connection %d\n", i));
		if (Connection[i].inited)
		    db_close(Connection[i].index);
		if (Connection[i].sendrecvfp)
		    fclose(Connection[i].sendrecvfp);
		/* XXX Leaves garbage file around when sending */
#ifdef MULTI
		if (Connection[i].server_num)
		    multi_conn_dropped(Connection[i].server_num-1);
#endif /* MULTI */
		memset((char*)&Connection[i], 0, sizeof(struct _Connection));
	    }
	}
	Debug(("Selecting...\n"));
	readfds = svc_fdset;
#ifdef MULTI
	if (seltimeout.tv_sec <= 0) {
	    seltimeout.tv_sec = PINGINTERVAL;
	    seltimeout.tv_usec = 0;
	}
	elapsed = time(0);
#endif /* MULTI */
	nfound = select(NOFILE, &readfds, (fd_set *)NULL, (fd_set *)NULL,
#ifdef MULTI
			&seltimeout);
	seltimeout.tv_sec -= time(0)-elapsed;
#else
	                (struct timeval *)NULL);
#endif /* MULTI */
	/* Ignore errors during select */
	if (nfound == -1)
	    continue;
	/* Check for timeout */
	if (!nfound)
#ifdef MULTI
	    multi_ping_servers();
#else
            continue;
#endif /* MULTI */
	svc_getreqset(&readfds);
#ifdef MULTI
        if (do_update_server) {
	    multi_update_server(do_update_server-1);
	    do_update_server = 0;
	}
#endif /* MULTI */
    }
}

#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/main.c,v 1.2 1995-07-12 03:36:52 cfields Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.1  1994/09/18  12:55:54  cfields
 * Initial revision
 *
 * Revision 2.2  93/06/18  14:35:36  root
 * first cut at solaris port
 * 
 * Revision 2.1  93/02/19  15:14:32  tom
 * fixed syslog message
 * 
 * Revision 2.0  92/04/22  02:01:15  tom
 * release 7.4
 * 	port to decmips & rs6000
 * 	nlist interface change and syserr suppression
 * 
 * Revision 1.4  90/07/17  14:26:13  tom
 * *** empty log message ***
 * 
 * Revision 1.3  90/06/05  16:07:55  tom
 * added setuid to daemon
 * ci -u krb_grp.c
 * 
 * Revision 1.2  90/05/26  13:38:43  tom
 * athena release 7.0e - added some ultrix-isms (patch 15)
 * 
 * Revision 1.1  90/04/26  17:15:29  tom
 * Initial revision
 * 
 * Revision 1.1  89/11/03  15:42:55  snmpdev
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
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/main.c,v 1.2 1995-07-12 03:36:52 cfields Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *
 *  Copyright (c) NYSERNet Incorporated, 1988, All Rights Reserved
 */

/*
 *  This is where life begins for snmpd.  Start up the time, parse
 *  the command line arg list, fork and detach from controlling tty
 *  if needed, set up the snmp socket, and wait for the good stuff.
 */

#include "include.h"

#ifdef ultrix 
#include <sys/proc.h>
#include <sys/sysinfo.h>
#endif

#ifdef SOLARIS
#define k_fltset_t unsigned long
#include <kvm.h>
#include <fcntl.h>
#endif

#ifdef MIT
#include <pwd.h>
#endif MIT

main(argc, argv)
	int   argc;
	char *argv[];
{
#ifdef SOLARIS
        struct sigaction vec, ovec;
#else
	struct sigvec vec, ovec;
#endif
	FILE  *fp;		/* config file descriptor */
	int selectbits;
#ifdef ultrix
	int buf[2];
#endif
#ifdef MIT
	int login_state, current_state;
	struct timeval timeout;
	timeout.tv_sec  = 30;   /* tune for select timeout BE CAREFUL */
	timeout.tv_usec = 0;
#endif MIT

#ifdef ultrix
	buf[0] = SSIN_UACPARNT;
	buf[1] = UAC_MSGOFF;
	setsysinfo((unsigned)SSI_NVPAIRS, (char *)buf, (unsigned) 1, 
		(unsigned)0, (unsigned)0);
#endif

	getod();		/* initialize start time */

	/*
	 *  zero out statistics structure.
	 */
	bzero((char *)&s_stat, sizeof(s_stat));
	bzero((char *)&ovec, sizeof(ovec));
	bzero((char *)&vec, sizeof(vec));

	/*
	 * check arguments for turning on debugging and a log file
	 */

	commparse(argc, argv);

	if (debuglevel == 0) {   /*  detach and ride..... */
		int t;

		if (fork())
			exit(0);
		for (t = 0; t < 20; t++)
			(void) close(t);
		(void) open("/", O_RDONLY);
		(void) dup2(0, 1);
		(void) dup2(0, 2);
		t = open("/dev/tty", O_RDWR);
		if (t >= 0) {
			(void) ioctl(t, TIOCNOTTY, (char *)NULL);
			(void) close(t);
		}
	}
	if (logfile != NULL)
		(void) traceon(logfile);

	/*
	 *  Open up the syslog facility
	 */
#if defined(BSD43) && !defined(ULTRIX3)
	openlog("snmpd", LOG_PID|LOG_CONS|LOG_NDELAY, LOG_DAEMON);
/*	(void) setlogmask(LOG_UPTO(LOG_NOTICE));*/
#else
	openlog("snmpd", LOG_PID);
#endif

	syslog(LOG_NOTICE, "Start snmpd version %s at %s\n", VERSION, strtime);

 	if (debuglevel > 0) 
	  (void) printf("Start snmpd version %s at %s\n", VERSION, strtime);
		
	if (debuglevel != 0)
		syslog(LOG_NOTICE, "Debug ON, Level: %d", debuglevel);

	/*
 	 *  try to open kmem and fill in namelist array here.
	 *  If we fail, we will call quit().
	 */

#ifdef RSPOS
	if (init_kmem("/unix") < 0) {
#endif /* RSPOS */
#ifdef SOLARIS
	if (init_kmem("/kernel/unix") < 0) {
#endif /* SOLARIS */
#if !defined(RSPOS) && !defined(SOLARIS)
	if (init_kmem("/vmunix") < 0) {
#endif

		syslog(LOG_ERR, "main: problems in init_kmem");
		quit();
	}

	/*
	 *  Open and initialize the config file.
	 */
	fp = fopen(SNMPINITFILE, "r");
  	if (fp == NULL) {
		syslog(LOG_WARNING, "main: file %s missing\n", SNMPINITFILE);
		quit();
	}

	/*
	 *  Parse the config file and initialize the values
	 *  that pertain to it.  Call quit() on problems.
	 */
	if (parse_config(fp) < 0) {
		syslog(LOG_ERR, "main: problems in parse_config");
		quit();
	}

	/*
	 * open SNMP socket!  Call quit() if problems arise.
	 */
	if ((snmp_socket = snmp_init()) < 0) {
		syslog(LOG_ERR, "main: snmp_init: can't open SNMP socket");
		quit();
	}

	/*
	 * open agent socket!  Call quit() if problems arise.
	 */
	if ((agent_socket = agent_init()) < 0) {
		syslog(LOG_ERR, "main: agent_init: can't open agent socket");
		quit();
	}

#ifdef MIT	
#ifdef TIMED

	/*
	 * don't care if it fails
	 */
	timed_init_socket();

#endif TIMED
#endif MIT

	/*
	 *  write proces id number to a file for easy
	 *  lookup.  if problems send to syslog.
	 */
	fp = fopen(PIDFILE, "w");
	if (fp != NULL) {
		(void) fprintf(fp, "%d\n", getpid());
		(void) fclose(fp);
	}
	else
		syslog(LOG_ERR, "Couldn't open PID file %s", PIDFILE);

#ifdef MIT
	
	/*
	 * suid to daemon
	 */
	 
	if(setuid(1) < 0)  {
	     syslog(LOG_ERR, "Unable to set uid process");
	     quit(1);
	}
#endif MIT

	/*
	 *  make SNMP variable tree
	 */
	if (make_var_tree() <= 0) {
		syslog(LOG_ERR, "main: error in building SNMP var tree");
		quit();
	}

	/*
	 *  Find out my local address and store it in a global
	 *  variable "local" for later use.  Saves headaches and
	 *  time later.  Don't quit().  minor trouble.
	 */
	if (get_my_address(&local) <= 0) {
		syslog(LOG_ERR, "main: error in finding local address");
	}

	/*
	 *  setup abort signal handler
	 */

#ifdef SOLARIS
	vec.sa_handler = quit;
	if (sigaction(SIGTERM, &vec, &ovec)) {
		syslog(LOG_ERR, "main: sigvec SIGTERM %m");
		quit();
	}
#else
	vec.sv_handler = quit;
	if (sigvec(SIGTERM, &vec, &ovec)) {
		syslog(LOG_ERR, "main: sigvec SIGTERM %m");
		quit();
	}
#endif

#ifdef MIT
	/*
	 *  check initial login state for inuse condition. MIT  
	 */
	get_inuse(&login_state);
	if(login_state > 0) 
		login_state = INUSE;
	    else
		login_state = UNUSED;

#endif MIT

	/*
	 *  Send out a trap to tell the NOC we are restarting.
	 */
	if (send_snmp_trap(COLDSTART,0) < 0)
		syslog(LOG_ERR,"main: trouble sending initial trap");

	/*
	 *  Set up select() sockets to listen on.
	 */
	selectbits = 0;
	selectbits |= 1 << snmp_socket;
	selectbits |= 1 << agent_socket;

	/*
	 *  wait to receive SNMP requests!
	 */
	while (ITS_COOL) {
		int ibits;
		register int n;

		ibits = selectbits;
#ifndef MIT
		n = select(20, (int *)&ibits, (int *)0, 
			   (int *)0, (struct timeval *) 0);
#else   MIT
		n = select(20, (int *)&ibits, (int *)0, (int *)0, &timeout); 
#endif MIT			   

		if (n < 0) {
			if (errno == EINVAL) {
				syslog(LOG_ERR, "main: select: %m");
				quit();
			}
			if (errno == EINTR) {
                                syslog(LOG_ERR, "main: select: %m");
                                quit();
                        }

		}
#ifdef MIT
		if (n == 0) {
       			get_inuse(&current_state);
       			if(current_state > 0) 
                		current_state = INUSE;
            	            else
               			current_state = UNUSED;
		
			if (current_state != login_state)
			  if (send_snmp_trap(VENDOR,current_state) < 0) 
			    syslog(LOG_WARNING, 
				   "main: trouble sending inuse trap \n");
			  else 
			    login_state = current_state;
		}  
		
		if (n > 0) {
#endif MIT
			if (ibits & (1 << snmp_socket))
				recvpkt(snmp_socket);
			else if (ibits & (1 << agent_socket))
				recvpkt(agent_socket);
#ifdef MIT
		}
#endif MIT
	}
}

/*
 *  receive a SNMP packet and pass it on for processing.
 */
recvpkt(sock)
	int sock;
{
	char packet[SNMPMAXPKT];  	/* packet buffer */
	struct sockaddr from;
	struct sockaddr_in *from_in;
	int fromlen = sizeof(from), count, omask;

	/*
	 *  If we are not doing debugging, we do
	 *  not need the time of day.
	 */
	if (debuglevel > 0)
		getod();			/* current time */

	bzero((char *)&from, sizeof(struct sockaddr));
	count = recvfrom(sock, packet, sizeof(packet), 0, &from, &fromlen);
	from_in = (struct sockaddr_in *)&from;
	s_stat.inpkts++;

	if (count <= 0) {
		if ((count < 0) && (errno != EINTR))
			syslog(LOG_ERR, "recvpkt: recvfrom %m");
		s_stat.inerrs++;
		return;
	}
	if (fromlen != sizeof (struct sockaddr)) {
		syslog(LOG_ERR, "recvpkt: fromlen %d invalid\n\n", fromlen);
		s_stat.inerrs++;
		return;
	}
	if (count > sizeof(packet)) {
		syslog(LOG_ERR, "recvfrom: packet discarded, length %d > %d",
                             count, sizeof(packet));
		syslog(LOG_ERR, ", from addr %s\n\n", inet_ntoa(from_in->sin_addr));
		s_stat.inerrs++;
		return;
	}

#define	mask(s)	(1<<((s)-1))

	omask = sigblock(mask(SIGTERM));

	if (sock == snmp_socket)
		snmpin(&from, count, packet);   /* process SNMP packet */
	else if (sock == agent_socket)
		agentin(&from, count, packet);   /* process agent packet */
	else
		syslog(LOG_ERR, "recvpkt: bad socket");

	(void) sigsetmask(omask);
	return;
}

/*
 *  get time of day in seconds and as an ASCII string.
 *  Called at each interrupt and saved in external variables.
 *  This is good so we can report time of events during hard
 *  or troublesome times.
 */
getod()
{
	struct timeval tp;
	struct timezone tzp;

	if (gettimeofday(&tp, &tzp))
		syslog(LOG_ERR, "getod: gettimeofday %m");
	snmptime = tp.tv_sec;			/* time in seconds */
	strtime = ctime(&snmptime);		/* time as an ASCII string */
	return;
}

/*
 *  exit snmpd
 */

#ifdef MIT
#ifdef decmips
void 
#else  decmips
int
#endif decmips
#endif MIT

quit()
{
	if (debuglevel != 0)
		(void) printf("Exit snmpd at %s\n\n", strtime);

	getod();
	syslog(LOG_NOTICE, "Exit snmpd at %s\n\n", strtime);

	exit(1);
}

/*
 *  open kmem and fill in the nl array.
 *  return an error code if any problems.
 */
int
init_kmem(system)
	char *system;		/* usually "/vmunix" */
{
	int d;
#ifdef SOLARIS
	kvm_t *k;

        k = kvm_open(NULL,NULL,NULL,O_RDONLY,NULL);
	if(!k)
	  {
	     syslog(LOG_ERR, "init_kmem: cannot open");
	     return(GEN_ERR);
          }
        d = kvm_nlist(k, nl);
#endif /* SOLARIS */
#ifdef RSPOS	
	d = knlist(nl, 62, sizeof (struct nlist));
#endif /* RSPOS */
#if !defined(SOLARIS) && !defined(RSPOS)
	d = nlist(system, nl);
#endif
	if (d < 0) {
		syslog(LOG_ERR, "%s: No namelist\n", system);
		return(GEN_ERR);
	}
	if (d != 0)

#ifdef MIT
		syslog(LOG_INFO,"warning: %d symbol(s) not found in namelist", d);
#else  MIT
		syslog(LOG_ERR, "warning: %d symbol(s) not found in namelist");
#endif MIT

	kmem = open("/dev/kmem", 0);
	if (kmem < 0) {
		syslog(LOG_ERR, "/dev/kmem: cannot open\n");
		return(GEN_ERR);
	}
	return(GEN_SUCCESS);
}

/*
 *  get my local address and store it in the sockaddr_in structure.
 */
int
get_my_address(myaddstr)
	struct sockaddr_in *myaddstr;
{
	char hname[SNMPSTRLEN];
	struct hostent *hp;

	bzero((char *)myaddstr, sizeof(struct sockaddr_in));

	if (gethostname(hname, SNMPSTRLEN) < 0)   /* get local host name */
		return(BUILD_ERR);

	if ((hp = gethostbyname(hname)) == NULL)
		return(BUILD_ERR);

#ifdef NAMED
	bcopy((char *)&(hp->h_addr),(char *)&myaddstr->sin_addr,hp->h_length);
#else
	bcopy((char *)(hp->h_addr),(char *)&myaddstr->sin_addr,hp->h_length);
#endif /* NAMED */

	myaddstr->sin_family = AF_INET;       /* use internet family */
	return(BUILD_SUCCESS);
}

/*
 * Turn on the log file.
 */
traceon(file)
	char *file;
{
	struct stat stbuf;

	if (ftrace != NULL)
		return;
	if ((stat(file, &stbuf) >= 0) && ((stbuf.st_mode & S_IFMT) != S_IFREG))
		return;
	ftrace = fopen(file, "a");
	if (ftrace == NULL) {
		syslog(LOG_WARNING, "traceon: cannot open log file");
		return;
	}
	(void) dup2(fileno(ftrace), 1);
	(void) dup2(fileno(ftrace), 2);
	return;
}

/*
 *  Parse the command line and set the debug options and log file.
 *  This is always harrowing, should be a generic library routine
 *  to parse the command line.
 */
commparse(ac, av)
	int ac;
	char *av[];
{
        int error = FALSE;
	char	*cp;
	
	debuglevel = 0;
	logfile = NULL;
	ac--;
	if (ac != 0) {
		av++;
		cp = *av;
		if (*cp++ != '-') {
			if (ac > 1)
				error = TRUE;
			else
				logfile = *av;
		}
		else if (*cp != 'd')
			error = TRUE;
		else {
			av++;
			ac--;
			cp = *av;
			if (ac == 0) {
				(void) fprintf(stderr, "must specify a debug level 0-9\n");
				error = TRUE;
			}
			else if (((*cp >= '0') && (*cp <= '9')) && (*(cp+1) == '\0')) {
				debuglevel = *cp - '0';
				(void) fprintf(stderr, "Debugging level: %d\n", debuglevel);
				ac--;
				if (ac == 0) {
					(void) fprintf(stderr, "must specify a log file with debug option.\n");
					error = TRUE;
				}
				else {
					av++;
					logfile = *av;
					ac--;
					if (ac != 0) {
						(void) fprintf(stderr, "Warning, further command line arguments are ignored.\n");
					}
				}
			} else {
				(void) fprintf(stderr, "invalid debug level\n");
				error = TRUE;
			}
		}
		if (error) {
			(void) fprintf(stderr, "Usage: snmpd [-d <level>] [logfile]\n");
			(void) fprintf(stderr, "       where level = 0 - 9\n");
			exit(1);
		}
	}
}


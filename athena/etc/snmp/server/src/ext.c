#ifndef lint
static char *RCSid = "$Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/ext.c,v 2.0 1992-04-22 02:04:48 tom Exp $";
#endif

/*
 * $Log: not supported by cvs2svn $
 * Revision 1.9  90/07/17  14:18:10  tom
 * removed kcKey variable
 * commented out variables for build on decmips
 * 
 * Revision 1.8  90/07/16  21:57:55  tom
 * maybe this is it... all TIMES which really represented dates have been 
 * changed to strings. This was done to avoid confusion with the sysUpTime
 * variable (more of a standard thing) which measures time ticks in hundreths
 * of a second. The kernel variables measuring times in other smaller units
 * are left as INTS. 
 * 
 * Revision 1.7  90/07/15  17:59:31  tom
 * changed file mod time vars to be of type TIME
 * 
 * Revision 1.6  90/05/26  13:37:10  tom
 * cleaned up a naming conflict
 * 
 * Revision 1.5  90/04/26  16:30:03  tom
 * changed some ifdefs
 * 
 * Revision 1.4  90/04/25  00:19:55  tom
 * backward compat. for relversion
 * 
 * Revision 1.3  90/04/24  23:41:35  tom
 * decided to keep athena 1
 * 
 * Revision 1.1  90/04/23  14:29:00  tom
 * Initial revision
 * 
 * Revision 1.1  89/11/03  15:42:37  snmpdev
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
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/ext.c,v 2.0 1992-04-22 02:04:48 tom Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988, All Rights Reserved
 */
/*
 *  This file contains all of the external (global) variables
 *  used by snmpd.  These variables are extern'ed in snmpd.h
 */

#include "include.h"

int 	snmp_socket;			/* snmp socket descriptor */
int 	agent_socket;			/* agent socket descriptor */
char 	*logfile;			/* the log file */
long	snmptime;			/* time in secs */
char	*strtime;			/* time of day as an ASCII string */
FILE 	*ftrace = NULL;			/* file descriptor of trace file */
struct	servent *sp;			/* service entry for SNMP */
struct	servent *trapport;		/* trap pointer to service entry */
struct	servent snmptrap;		/* service entry for SNMP traps */
struct	sockaddr_in addr;		/* addr to bind SNMP socket to */
struct	sockaddr_in local;		/* my local address */
struct  snmp_tree_node *top;		/* root (top) of SNMP variable tree */
int	kmem;				/* file descripter for kmem */
int	debuglevel;			/* debugging level of snmpd */
int	newpacket = 0;			/* are we processing a new packet? */
struct	intf_info *iilst = NULL;	/* list for interface info */
struct	snmp_session *sessions = NULL;	/* list for SNMP sessions */
char	gw_version_id[SNMPSTRLEN];	/* ID of the gateway */
struct	snmp_stats s_stat;		/* stats for SNMPD kept here */
int	send_authen_traps;		/* do we send authen. traps? */
struct  set_struct *setlst;		/* linked list of sets */
int	adminstat[MAXIFS];		/* intf admin status list */
int	tcprtoalg;			/* TCP RTO algorithm variable */

#ifdef MIT
char    lbuf[BUFSIZ];                   /* random buffer */

char    supp_sysdescr[SNMPSTRLEN];     /* supplementary sysdescr */
char    mail_q[SNMPSTRLEN];             /* path for mailq directory */
char    mail_alias_file[SNMPSTRLEN];    /* path for mail aliases */
char    rc_file[SNMPSTRLEN];            /* path for rc config file */
char    rpc_cred_file[SNMPSTRLEN];      /* path for rpc cred file */
char    afs_cache_file[SNMPSTRLEN];     /* path for afs cache file */
char    afs_suid_file[SNMPSTRLEN];      /* path for afs setuid file */
char    afs_cell_file[SNMPSTRLEN];      /* path for afs cell file */
char    afs_cellserv_file[SNMPSTRLEN];  /* path for cellsrvdb */
char    login_file[SNMPSTRLEN];         /* path for utmp */
char    version_file[SNMPSTRLEN];       /* path for version file */
char    syspack_file[SNMPSTRLEN];       /* path for syspack file */
char    dns_stat_file[SNMPSTRLEN];      /* path for dns stat file */
char    srv_file[SNMPSTRLEN];           /* path for mksrv file */
char    user[SNMPSTRLEN];               /* default uid to run as */

#ifdef RSPOS
struct   mbuf *rthost[RTHASHSIZ];       /* routing table structs for rs6k */
struct   mbuf *rtnet[RTHASHSIZ];
#endif /* RSPOS */


int     logintrap;                      /* whether to send login trap */
#endif MIT

/*
 *  C library calls.
 */
long lseek();
u_long inet_addr();
time_t time();
char *malloc();
char *inet_ntoa();

/*
 *  Sys Object ID for the sysObjectID variable.
 */
objident sys_obj_id = SYS_OBJ_ID;

/*
 *  MIB variables, used in tree info structure.
 */
/*
 *  system group
 */
objident sysID = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 1, 1
};
objident sysObjectID = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 1, 2
};
objident sysUpTime = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 1, 3
};
/*
 *  interfaces group
 */
objident ifNumber = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 2, 1
};
/*
 *  ifTable
 */
objident ifIndex = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 2, 2, 1, 1
};
objident ifName = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 2, 2, 1, 2
};
objident ifType = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 2, 2, 1, 3
};
objident ifMtu = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 2, 2, 1, 4
};
objident ifSpeed = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 2, 2, 1, 5
};
objident ifAdminStatus = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 2, 2, 1, 7
};
objident ifOperStatus = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 2, 2, 1, 8
};
objident ifLastChange = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 2, 2, 1, 9
};
objident ifInUcastPkts = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 2, 2, 1, 11
};
objident ifInErrors = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 2, 2, 1, 14
};
objident ifOutUcastPkts = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 2, 2, 1, 17
};
objident ifOutErrors = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 2, 2, 1, 20
};
objident ifOutQLen = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 2, 2, 1, 21
};
/*
 *  AT group
 */
objident atIfIndex = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 3, 1, 1, 1
};
objident atPhysAddress = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 3, 1, 1, 2
};
objident atNetAddress = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 3, 1, 1, 3
};
/*
 *  IP group
 */
objident ipForwarding = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 1
};
objident ipDefaultTTL = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 2
};
objident ipInReceives = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 3
};
objident ipInHdrErrors = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 4
};
objident ipInAddrErrors = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 5
};
objident ipForwDatagrams = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 6
};
objident ipOutNoRoutes = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 12
};
objident ipReasmTimeout = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 13
};
objident ipReasmReqds = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 14
};
objident ipReasmFails = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 16
};
/*
 *  IP addr table
 */
objident ipAdEntAddr = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 20, 1, 1
};
objident ipAdEntIndex = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 20, 1, 2
};
objident ipAdEntNetMask = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 20, 1, 3
};
objident ipAdEntBcastAddr = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 20, 1, 4
};
/*
 *  IP route table
 */
objident ipRouteDest = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 21, 1, 1
};
objident ipRouteMetric1 = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 21, 1, 3
};
objident ipRouteMetric2 = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 21, 1, 4
};
objident ipRouteMetric3 = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 21, 1, 5
};
objident ipRouteMetric4 = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 21, 1, 6
};
objident ipRouteNextHop = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 21, 1, 7
};
objident ipRouteType = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 21, 1, 8
};
objident ipRouteProto = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 21, 1, 9
};
objident ipRouteAge = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 4, 21, 1, 10
};
/*
 *  ICMP Group
 */
objident icmpInMsgs = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 1
};
objident icmpInErrors = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 2
};
objident icmpInDestUnreachs = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 3
};
objident icmpInTimeExcds = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 4
};
objident icmpInParmProbs = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 5
};
objident icmpInSrcQuenchs = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 6
};
objident icmpInRedirects = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 7
};
objident icmpInEchos = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 8
};
objident icmpInEchoReps = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 9
};
objident icmpInTimestamps = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 10
};
objident icmpInTimestampReps = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 11
};
objident icmpInAddrMasks = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 12
};
objident icmpInAddrMaskReps = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 13
};
objident icmpOutMsgs = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 14
};
objident icmpOutErrors = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 15
};
objident icmpOutDestUnreachs = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 16
};
objident icmpOutTimeExcds = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 17
};
objident icmpOutParmProbs = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 18
};
objident icmpOutSrcQuenchs = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 19
};
objident icmpOutRedirects = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 20
};
objident icmpOutEchos = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 21
};
objident icmpOutEchoReps = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 22
};
objident icmpOutTimestamps = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 23
};
objident icmpOutTimestampReps = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 24
};
objident icmpOutAddrMasks = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 25
};
objident icmpOutAddrMaskReps = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 5, 26
};
/*
 *  TCP group
 */
objident tcpRtoAlgorithm = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 6, 1
};
objident tcpRtoMin = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 6, 2
};
objident tcpRtoMax = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 6, 3
};
objident tcpMaxConn = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 6, 4
};
objident tcpActiveOpens = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 6, 5
};
objident tcpAttemptFails = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 6, 7
};
objident tcpEstabResets = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 6, 8
};
objident tcpCurrEstab = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 6, 9
};
objident tcpInSegs = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 6, 10
};
objident tcpOutSegs = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 6, 11
};
objident tcpRetransSegs = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 6, 12
};
/*
 *  TCP conn table
 */
objident tcpConnState = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 6, 13, 1, 1
};
objident tcpConnLocalAddress = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 6, 13, 1, 2
};
objident tcpConnLocalPort = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 6, 13, 1, 3
};
objident tcpConnRemAddress = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 6, 13, 1, 4
};
objident tcpConnRemPort = {
	10,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 6, 13, 1, 5
};
/*
 *  UDP Group
 */
objident udpInErrors = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 7, 3
};
/*
 *  EGP Group
 */
objident egpInMsgs = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 8, 1
};
objident egpInErrors = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 8, 2
};
objident egpOutMsgs = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 8, 3
};
objident egpOutErrors = {
	8,					/* Length of variable */
	1, 3, 6, 1, 2, 1, 8, 4
};

/***************** MIT ******************/

#ifdef MIT
#ifdef ATHENA
objident machType = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 1, 1
};

objident machNDisplay = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 1, 2
};

objident machDisplay = {
        13,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 1, 3, 2, 1, 1
};

objident machNDisks = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 1, 4
};

objident machDisks = {
        13,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 1, 5, 2, 1, 1
};

objident machMemory = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 1, 6
};

objident machName = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 1, 7
};

objident rcHOST = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 1
};

objident rcADDR = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 2
};

objident rcMACHINE = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 3
};

objident rcSYSTEM = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 4
};

objident rcWS = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 5
};

objident rcTOEHOLD = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 6
};

objident rcPUBLIC = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 7
};

objident rcERRHALT = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 8
};

objident rcLPD = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 9
};

objident rcRVDSRV = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 10
};

objident rcRVDCLIENT = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 11
};

objident rcNFSSRV = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 12
};

objident rcNFSCLIENT = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 13
};

objident rcAFSSRV = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 14
};

objident rcAFSCLIENT = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 15
};

objident rcRPC = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 16
};

objident rcSAVECORE = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 17
};

objident rcPOP = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 18
};

objident rcSENDMAIL = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 19
};

objident rcQUOTAS = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 20
};

objident rcACCOUNT = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 21
};

objident rcOLC = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 22
};

objident rcTIMESRV = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 23
};

objident rcPCNAMED = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 24
};

objident rcNEWMAILCF = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 25
};

objident rcKNETD = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 26
};

objident rcTIMEHUB = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 27
};

objident rcZCLIENT = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 28
};

objident rcZSERVER = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 29
};

objident rcSMSUPDATE = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 30
};

objident rcINETD = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 31
};

objident rcNOCREATE = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 32
};

objident rcNOATTACH = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 33
};

objident rcAFSADJUST = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 34
};

objident rcSNMP = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 35
};

objident rcAUTOUPDATE = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 36
};

objident rcTIMECLIENT = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 37
};

objident rcKRBSRV = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 38
};

objident rcKADMSRV = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 39
};

objident rcNIPSRV = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 1, 40
};

objident rcFile = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 2, 1
};

objident relVersion = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 3, 1
};

objident relDate = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 3, 2
};

objident packNum = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 5, 1
};

objident packName = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 5, 2, 1
};

objident packType = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 5, 2, 2
};

objident packVersion = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 5, 2, 3
};

objident packDate = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 5, 2, 4
};

objident srvNumber = {
        11,                                     /* Length of variable */
	1, 3, 6, 1, 4, 1, 20, 1, 2, 6, 1
};

objident srvName = {
        13,                                     /* Length of variable */
	1, 3, 6, 1, 4, 1, 20, 1, 2, 6, 2, 1, 1
};

objident srvType = {
        13,                                     /* Length of variable */
	1, 3, 6, 1, 4, 1, 20, 1, 2, 6, 2, 1, 2
};

objident srvVersion = {
        13,                                     /* Length of variable */
	1, 3, 6, 1, 4, 1, 20, 1, 2, 6, 2, 1, 3
};

objident srvFile = {
        11,                                     /* Length of variable */
	1, 3, 6, 1, 4, 1, 20, 1, 2, 6, 3
};

#endif ATHENA

objident snmpVersion = {
        12,                                     /* Length of variable */
	1, 3, 6, 1, 4, 1, 20, 1, 2, 10, 1, 1        
};

objident snmpCompTime = {
        12,                                     /* Length of variable */
	1, 3, 6, 1, 4, 1, 20, 1, 2, 10, 1, 2
};

objident osVendor = { 
        12,                                     /* Length of variable */
	1, 3, 6, 1, 4, 1, 20, 1, 2, 10, 2, 1
};

objident osType = {
        12,                                     /* Length of variable */
	1, 3, 6, 1, 4, 1, 20, 1, 2, 10, 2, 2
};

objident osVersion = {
        12,                                     /* Length of variable */
	1, 3, 6, 1, 4, 1, 20, 1, 2, 10, 2, 3
};

objident kernVersion = {
        13,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 10, 2, 4, 1
};

objident kernDate = {
        13,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 10, 2, 4, 2
};

objident kernBuilder = {
        13,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 10, 2, 4, 3
};

objident machtypeVersion = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 2, 10, 6, 1
};

#ifdef ATHENA
/* compatibility for pre 7.3 */

objident statOVersion = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 1
};

#endif ATHENA

objident statTime = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 2
};

objident loadRunTime = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 3, 1, 1, 1
};

objident loadCPU = {
        11,
	1, 3, 6, 1, 4, 1, 20, 1, 3, 3, 2, 1, 1
};

objident statLogin = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 4
};

/* @begin(backward compatibility for pre 7.4) */

objident statDkNParts = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 5, 1, 1
};

objident statDkPath = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 5, 1, 2
};

objident statDkDname = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 5, 1, 3
};

objident statDkTotal = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 5, 1, 4
};

objident statDkUsed = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 5, 1, 5
};

objident statDkFree = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 5, 1, 6
};

objident statDkAvail = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 5, 1, 7
};

objident statDkITotal = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 5, 1, 8
};

objident statDkIUsed = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 5, 1, 9
};

objident statDkIFree = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 5, 1, 10
};

objident statDkType = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 5, 1, 15
};
/* @end(backward compatibility for pre 7.4) */


objident vpR = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 1, 1
};

objident vpB = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 1, 2
};

objident vpW = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 1, 3
};

objident vpAvm = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 1, 4
};

objident vpFre = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 1, 5
};

objident vpRe = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 2, 1
};

objident vpAt = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 2, 2
};

objident vpPi = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 2, 3
};

objident vpPo = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 2, 4
};

objident vpFr = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 2, 5
};

objident vpDe = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 2, 6
};

objident vpSr = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 2, 7
};

objident vsSwpIn = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 1
};

objident vsSwpOut = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 2
};

objident vsPgSwpIn = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 3
};

objident vsPgSwpOut = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 4
};

objident vsAtFlt = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 5
};

objident vsPgSeqFre = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 6
};

objident vsPgRec = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 7
};

objident vsPgFRec = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 8
};

objident vsTotRec = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 9
};

objident vsFreRec = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 10
};

objident vsBlkPgFlt = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 11
};

objident vsZFCreat = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 12
};

objident vsZFFault = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 13
};

objident vsEFCreat = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 14
};

objident vsEFFault = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 15
};

objident vsSwpFree = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 16
};

objident vsInodeFre = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 17
};

objident vsFFCreat = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 18
};

objident vsFFFault = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 19
};

objident vsPgClock = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 20
};

objident vsClkRev = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 21
};

objident vsClkPgFre = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 22
};

objident vsConSwtch = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 23
};

objident vsDevInt = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 24
};

objident vsSoftInt = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 25
};

objident vsDMAInt = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 26
};

objident vsTrap = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 27
};

objident vsSysCall = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 28
};

objident vsNameLkp = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 3, 29
};

objident vxCall = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 4, 1
};

objident vxHit = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 4, 2
};

objident vxStick = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 4, 3
};

objident vxFlush = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 4, 4
};

objident vxUnuse = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 4, 5
};

objident vxFreCall = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 4, 6
};

objident vxFreInuse = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 4, 7
};

objident vxFreCache = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 4, 8
};

objident vxFreSwap = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 4, 9
};

objident vfFork = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 5, 1
};

objident vfPage = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 5, 2
};

objident vfVFork = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 5, 3
};

objident vfVPage = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 5, 4
};

objident vtRec = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 6, 1
};

objident vtPgIn = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 6, 2
};

objident vncsGood = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 7, 1
};

objident vncsBad = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 7, 2
};

objident vncsFalse = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 7, 3
};

objident vncsMiss = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 7, 4
};

objident vncsLong = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 7, 5
};

objident vncsTotal = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 7, 6
};

objident vnchGood = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 8, 1
};

objident vnchBad = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 8, 2
};

objident vnchFalse = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 8, 3
};

objident vnchMiss = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 8, 4
};

objident vnchLong = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 8, 5
};

objident vnchTotal = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 8, 6
};

objident vcUs = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 9, 1
};

objident vcNi = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 9, 2
};

objident vcSy = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 9, 3
};

objident vcId = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 9, 4
};

objident vmmIn = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 10, 1
};

objident vmmSy = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 10, 2
};

objident vmmCs = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 10, 10, 3
};

objident psTotal = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 11, 1, 1
};

objident psUsed = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 11, 1, 2
};

objident psTUsed = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 11, 1, 3
};

objident psFree = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 11, 1, 4
};

objident psWasted = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 11, 1, 5
};

objident psMissing = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 11, 1, 6
};

objident pfTotal = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 11, 2, 1
};

objident pfUsed = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 11, 2, 2
};

objident piTotal = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 11, 3, 1
};

objident piUsed = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 11, 3, 2
};

objident ppTotal = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 11, 4, 1
};

objident ppUsed = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 11, 4, 2
};

objident ptTotal = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 11, 5, 1
};

objident ptUsed = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 11, 5, 2
};

objident ptActive = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 3, 11, 5, 3
};


#ifdef RPC
objident rpcCall = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 5, 1, 1
};

objident rpcBadCall = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 5, 1, 2
};

objident rpcRetrans = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 5, 1, 3
};

objident rpcBadXid = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 5, 1, 4
};

objident rpcTimeout = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 5, 1, 5
};

objident rpcWait = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 5, 1, 6
};

objident rpcNewCred = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 5, 1, 7
};

objident rpsCall = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 5, 2, 1
};

objident rpsBadCall = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 5, 2, 2
};

objident rpsNullRecv = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 5, 2, 3
};

objident rpsBadLen = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 5, 2, 4
};

objident rpsXDRCall = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 5, 2, 5
};

objident rpsCredUt = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 5, 2, 10, 1
};

objident rpsCredUtPag = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 5, 2, 10, 2
};

objident rpsCredUtDir = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 5, 2, 10, 3
};

objident rpsCredFile = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 5, 2, 10, 5
};


#endif RPC

#ifdef NFS
objident ncCall = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 1
};

objident ncBadCall = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 2
};

objident ncNULL = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 3
};

objident ncGetAddr = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 4
};

objident ncSetAddr = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 5
};

objident ncRoot = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 6
};

objident ncLookup = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 7
};

objident ncReadLink = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 8
};

objident ncRead = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 9
};

objident ncWRCache = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 10
};

objident ncWrite = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 11
};

objident ncCreate = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 12
};

objident ncRemove = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 13
};

objident ncRename = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 14
};

objident ncLink = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 15
};

objident ncSymLink = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 16
};

objident ncMkDir = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 17
};

objident ncRmDir = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 18
};

objident ncReadDir = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 19
};

objident ncFSStat = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 20
};

objident ncNCLGet = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 21
};

objident ncNCLSleep = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 1, 22
};

objident nsCall = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 1
};

objident nsBadCall = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 2
};

objident nsNull = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 3
};

objident nsGetAddr = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 4
};

objident nsSetAddr = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 5
};

objident nsRoot = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 6
};

objident nsLookup = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 7
};

objident nsReadLink = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 8
};

objident nsRead = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 9
};

objident nsWRCache = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 10
};

objident nsWrite = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 11
};

objident nsCreate = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 12
};

objident nsRemove = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 13
};

objident nsRename = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 14
};

objident nsLink = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 15
};

objident nsSymLink = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 16
};

objident nsMkDir = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 17
};

objident nsRmDir = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 18
};

objident nsReadDir = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 19
};

objident nsFSStat = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 6, 2, 20
};
#endif NFS

#ifdef RVD

objident rvcBadBlock = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 1
};

objident rvcBadCkSum = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 2
};

objident rvcBadType = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 3
};

objident rvcBadState = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 4
};

objident rvcBadFormat = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 5
};

objident rvcTimeout = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 6
};

objident rvcBadNonce = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 7
};

objident rvcErrorRecv = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 8
};

objident rvcBadData = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 9
};

objident rvcBadVers = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 10
};

objident rvcPktRej = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 11
};

objident rvcPush = {
        11,			        	/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 12
};

objident rvcPktSent = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 13
};

objident rvcPktRecv = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 14
};

objident rvcQkRetrans = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 15
};

objident rvcLgRetrans = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 16
};

objident rvcBlockRd = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 17
};

objident rvcBlockWr = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 7, 1, 18
};
#endif RVD

#ifdef AFS

objident acCacheSize = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 8, 1, 1, 1
};

objident acCacheFile = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 8, 1, 1, 2
};

objident acThisCell = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 8, 1, 2, 1
};

objident acCellFile = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 8, 1, 2, 2
};

objident acNSuidCell = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 8, 1, 3, 1
};

objident acSuidCell = {
        14,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 8, 1, 3, 2, 1, 1
};

objident acSuidFile = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 8, 1, 3, 3
};

objident acNCellSrv = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 8, 1, 9, 1
};

objident acCellSrvName = {
        14,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 8, 1, 9, 2, 1, 1
};

objident acCellSrvAddr = {
        14,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 8, 1, 9, 2, 1, 2
};

objident acCellSrvCom = {
        14,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 8, 1, 9, 2, 1, 3
};

objident acCellSrvFile = {
        12,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 8, 1, 9, 3
};

#endif AFS

#ifdef KERBEROS
objident kcRealm = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 9, 1, 2
};

#endif KERBEROS

#ifdef ZEPHYR
objident zcPVersion = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 10, 1, 1
};

objident zcServer = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 10, 1, 2
};

objident zcQueue = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 10, 1, 3
};

objident zcServChange = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 10, 1, 4
};

objident zcHeader = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 10, 1, 5
};

objident zcLooking = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 10, 1, 6
};

objident zcUptime = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 10, 1, 7
};

objident zcSize = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 10, 1, 8
};

objident zsPVersion = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 10, 2, 1
};

objident zsHeader = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 10, 2, 2
};

objident zsPacket = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 10, 2, 3
};

objident zsUptime = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 10, 2, 4
};

objident zsState = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 10, 2, 5
};
#endif ZEPHYR

objident mQueue = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 11, 1
};

objident mAliasUt = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 11, 2, 1
};

objident mAliasUtPag = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 11, 2, 2
};

objident mAliasUtDir = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 11, 2, 3
};

objident mAliasFile = {
        11,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 11, 2, 5
};

#ifdef DNS
objident dnsUpdateTime = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 1
};

objident dnsBootTime = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 2
};

objident dnsResetTime = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 3
};

objident dnsPacketIn = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 4
};

objident dnsPacketOut = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 5
};

objident dnsQuery = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 6
};

objident dnsIQuery = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 7
};

objident dnsDupQuery = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 8
};

objident dnsResponse = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 9
};

objident dnsDupResp = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 10
};

objident dnsOK = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 11
};

objident dnsFail = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 12
};

objident dnsFormErr = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 13
};

objident dnsSysQuery = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 14
};

objident dnsPrimeCache = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 15
};

objident dnsCheckNS = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 16
};

objident dnsBadResp = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 17
};

objident dnsMartian = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 18
};

objident dnsUnknown = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 19
};

objident dnsA = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 20
};

objident dnsNS = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 21
};

objident dnsCName = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 22
};

objident dnsSOA = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 23
};

objident dnsWKS = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 24
};

objident dnsPTR = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 25
};

objident dnsHInfo = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 26
};

objident dnsMX = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 27
};

objident dnsTXT = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 28
};

objident dnsUNSPECA = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 29
};

objident dnsAXFR = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 30
};

objident dnsANY = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 31
};

objident dnsStatFile = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 12, 40
};

#endif DNS

#ifdef TIMED
objident tdMaster = {
        10,					/* Length of variable */
        1, 3, 6, 1, 4, 1, 20, 1, 13, 1
};
#endif TIMED

#endif MIT


/*
 *  Structure which helps us build the SNMP variable tree.
 */
struct snmp_tree_info  var_tree_info[] = {  /* must be NULL terminated */
/*
 *   VARIABLE	  LOOKUP    SET       LOOKUP
 *   OBJECT ID,   FUNCTION, FUNCTION, INDEX,   FLAGS
 */
/*
 *  System Group as specified in RFC 1066.	3/3 variables
 */

{ &sysID,             lu_vers,  NULL, N_VERID, NULL_OBJINST|VAL_STR },
{ &sysObjectID,       lu_vers,  NULL, N_VEREV, NULL_OBJINST|VAL_OBJ },
#ifdef RSPOS
{ &sysUpTime,         lu_status,NULL, N_LINIT, NULL_OBJINST|VAL_TIME },
#else /* RSPOS */
{ &sysUpTime,         lu_vers,  NULL, N_LINIT, NULL_OBJINST|VAL_TIME },
#endif /* RSPOS */
/*
 *  Interfaces Group as specified in RFC 1066.	14/22 variables
 */
{ &ifNumber,          lu_nnets, NULL, N_NNETS, NULL_OBJINST|VAL_INT },
{ &ifIndex,           lu_intf,  NULL, N_INDEX, VAL_INT },
{ &ifName,            lu_intf,  NULL, N_SCOPE, VAL_STR },
{ &ifType,            lu_intf,  NULL, N_IFTYP, VAL_INT },
{ &ifMtu,             lu_intf,  NULL, N_IFMTU, VAL_INT },
{ &ifSpeed,           lu_intf,  NULL, N_IFSPD, VAL_GAUGE },
{ &ifAdminStatus,     lu_intf,  NULL, N_IFATA, WRITE_VAR|VAL_INT },
{ &ifOperStatus,      lu_intf,  NULL, N_IFSTA, VAL_INT },
{ &ifLastChange,      lu_intf,  NULL, N_IFLCG, VAL_TIME },
{ &ifInUcastPkts,     lu_intf,  NULL, N_IPKTS, VAL_CNTR },
{ &ifInErrors,        lu_intf,  NULL, N_IERRS, VAL_CNTR },
{ &ifOutUcastPkts,    lu_intf,  NULL, N_OPKTS, VAL_CNTR },
{ &ifOutErrors,       lu_intf,  NULL, N_OERRS, VAL_CNTR },
{ &ifOutQLen,         lu_intf,  NULL, N_OQLEN, VAL_GAUGE },
/*
 *  AT Group as specified in RFC 1066.		3/3 variables
 */
{ &atIfIndex,         lu_atent, NULL, N_AINDE, VAL_INT },
{ &atPhysAddress,     lu_atent, NULL, N_AETH,  VAL_STR },
{ &atNetAddress,      lu_atent, NULL, N_ARPA,  VAL_IPADD },
/*
 *  IP Group as specified in RFC 1066.		19/33 variables
 */
#ifndef RSPOS
{ &ipForwarding,      lu_ipforw, NULL, N_IPFDI, NULL_OBJINST|VAL_INT },
#endif /* RSPOS */
{ &ipDefaultTTL,      lu_ipstat, NULL, N_IPTTL, NULL_OBJINST|VAL_INT },
{ &ipInReceives,      lu_ipstat, NULL, N_IPINR, NULL_OBJINST|VAL_CNTR },
{ &ipInHdrErrors,     lu_ipstat, NULL, N_IPHRE, NULL_OBJINST|VAL_CNTR },
{ &ipInAddrErrors,    lu_ipstat, NULL, N_IPADE, NULL_OBJINST|VAL_CNTR },
{ &ipForwDatagrams,   lu_ipstat, NULL, N_IPFOR, NULL_OBJINST|VAL_CNTR },
{ &ipOutNoRoutes,     lu_ipstat, NULL, N_IPNOR, NULL_OBJINST|VAL_CNTR },
{ &ipReasmTimeout,    lu_ipstat, NULL, N_IPRTO, NULL_OBJINST|VAL_INT },
{ &ipReasmReqds,      lu_ipstat, NULL, N_IPRAS, NULL_OBJINST|VAL_CNTR },
{ &ipReasmFails,      lu_ipstat, NULL, N_IPRFL, NULL_OBJINST|VAL_CNTR },
/*
 *  IP address table
 */
{ &ipAdEntAddr,       lu_ipadd, NULL, N_IPADD, VAL_IPADD },
{ &ipAdEntIndex,      lu_ipadd, NULL, N_IPIND, VAL_INT },
{ &ipAdEntNetMask,    lu_ipadd, NULL, N_IPMSK, VAL_IPADD },
{ &ipAdEntBcastAddr,  lu_ipadd, NULL, N_IPBRD, VAL_IPADD },
/*
 *  IP route table
 */
{ &ipRouteDest,       lu_rtent, NULL, N_RTDST, WRITE_VAR|VAL_IPADD },
{ &ipRouteMetric1,    NULL,     NULL, 0,       WRITE_VAR|VAL_INT },
{ &ipRouteMetric2,    lu_rtent, NULL, N_RTMT2, WRITE_VAR|VAL_INT },
{ &ipRouteMetric3,    lu_rtent, NULL, N_RTMT3, WRITE_VAR|VAL_INT },
{ &ipRouteMetric4,    lu_rtent, NULL, N_RTMT4, WRITE_VAR|VAL_INT },
{ &ipRouteNextHop,    lu_rtent, NULL, N_RTGAT, WRITE_VAR|VAL_IPADD },
{ &ipRouteType,       lu_rtent, NULL, N_RTTYP, WRITE_VAR|VAL_INT },
{ &ipRouteProto,      NULL,     NULL, 0,       WRITE_VAR|VAL_INT },
{ &ipRouteAge,        NULL,     NULL, 0,       WRITE_VAR|VAL_INT },
/*
 *  ICMP Group as specified in RFC 1066.	26/26 variables
 */
{ &icmpInMsgs,           lu_icmp, NULL, N_INMSG, NULL_OBJINST|VAL_CNTR },
{ &icmpInErrors,         lu_icmp, NULL, N_ICERR, NULL_OBJINST|VAL_CNTR },
{ &icmpInDestUnreachs,   lu_icmp, NULL, N_DSTUN, NULL_OBJINST|VAL_CNTR },
{ &icmpInTimeExcds,      lu_icmp, NULL, N_TTLEX, NULL_OBJINST|VAL_CNTR },
{ &icmpInParmProbs,      lu_icmp, NULL, N_IBHDR, NULL_OBJINST|VAL_CNTR },
{ &icmpInSrcQuenchs,     lu_icmp, NULL, N_SRCQE, NULL_OBJINST|VAL_CNTR },
{ &icmpInRedirects,      lu_icmp, NULL, N_IREDI, NULL_OBJINST|VAL_CNTR },
{ &icmpInEchos,          lu_icmp, NULL, N_ECREQ, NULL_OBJINST|VAL_CNTR },
{ &icmpInEchoReps,       lu_icmp, NULL, N_ECREP, NULL_OBJINST|VAL_CNTR },
{ &icmpInTimestamps,     lu_icmp, NULL, N_ITIME, NULL_OBJINST|VAL_CNTR },
{ &icmpInTimestampReps,  lu_icmp, NULL, N_ITREP, NULL_OBJINST|VAL_CNTR },
{ &icmpInAddrMasks,      lu_icmp, NULL, N_MSKRQ, NULL_OBJINST|VAL_CNTR },
{ &icmpInAddrMaskReps,   lu_icmp, NULL, N_MSKRP, NULL_OBJINST|VAL_CNTR },
{ &icmpOutMsgs,          lu_icmp, NULL, N_ORESP, NULL_OBJINST|VAL_CNTR },
{ &icmpOutErrors,        lu_icmp, NULL, N_OERRR, NULL_OBJINST|VAL_CNTR },
{ &icmpOutDestUnreachs,  lu_icmp, NULL, N_OSTUN, NULL_OBJINST|VAL_CNTR },
{ &icmpOutTimeExcds,     lu_icmp, NULL, N_OTLEX, NULL_OBJINST|VAL_CNTR },
{ &icmpOutParmProbs,     lu_icmp, NULL, N_OBHDR, NULL_OBJINST|VAL_CNTR },
{ &icmpOutSrcQuenchs,    lu_icmp, NULL, N_ORCQE, NULL_OBJINST|VAL_CNTR },
{ &icmpOutRedirects,     lu_icmp, NULL, N_OREDI, NULL_OBJINST|VAL_CNTR },
{ &icmpOutEchos,         lu_icmp, NULL, N_OCREQ, NULL_OBJINST|VAL_CNTR },
{ &icmpOutEchoReps,      lu_icmp, NULL, N_OCREP, NULL_OBJINST|VAL_CNTR },
{ &icmpOutTimestamps,    lu_icmp, NULL, N_OTIME, NULL_OBJINST|VAL_CNTR },
{ &icmpOutTimestampReps, lu_icmp, NULL, N_OTREP, NULL_OBJINST|VAL_CNTR },
{ &icmpOutAddrMasks,     lu_icmp, NULL, N_OSKRQ, NULL_OBJINST|VAL_CNTR },
{ &icmpOutAddrMaskReps,  lu_icmp, NULL, N_OSKRP, NULL_OBJINST|VAL_CNTR },
/*
 *  TCP group as specified in RFC 1066.		16/17 variables
 */
{ &tcpRtoAlgorithm, 	 lu_tcpstat, NULL, N_RTOAL, NULL_OBJINST|VAL_INT },
{ &tcpRtoMin, 	 	 lu_tcprtos, NULL, N_TRMI,  NULL_OBJINST|VAL_INT },
{ &tcpRtoMax, 	 	 lu_tcprtos, NULL, N_TRMX,  NULL_OBJINST|VAL_INT },
{ &tcpMaxConn,	 	 lu_tcpstat, NULL, N_TMXCO, NULL_OBJINST|VAL_INT },
{ &tcpActiveOpens,	 lu_tcpstat, NULL, N_TACTO, NULL_OBJINST|VAL_CNTR },
{ &tcpAttemptFails,	 lu_tcpstat, NULL, N_TATFA, NULL_OBJINST|VAL_CNTR },
{ &tcpEstabResets,	 lu_tcpstat, NULL, N_TESRE, NULL_OBJINST|VAL_CNTR },
{ &tcpCurrEstab,	 lu_tcpstat, NULL, N_TCEST, NULL_OBJINST|VAL_GAUGE },
{ &tcpInSegs,	 	 lu_tcpstat, NULL, N_TISEG, NULL_OBJINST|VAL_CNTR },
{ &tcpOutSegs,	 	 lu_tcpstat, NULL, N_TOSEG, NULL_OBJINST|VAL_CNTR },
{ &tcpRetransSegs,	 lu_tcpstat, NULL, N_TRXMI, NULL_OBJINST|VAL_CNTR },
{ &tcpConnState,	 lu_tcpconnent, NULL, N_TSTE, VAL_INT },
{ &tcpConnLocalAddress,	 lu_tcpconnent, NULL, N_LADD, VAL_INT },
{ &tcpConnLocalPort,	 lu_tcpconnent, NULL, N_LPRT, VAL_INT },
{ &tcpConnRemAddress,    lu_tcpconnent, NULL, N_FADD, VAL_INT },
{ &tcpConnRemPort,	 lu_tcpconnent, NULL, N_FPRT, VAL_INT },
/*
 *  UDP Group as specified in RFC 1066.		1/4 variables
 */
{ &udpInErrors,	 	 lu_udpstat, NULL, N_UIERR, NULL_OBJINST|VAL_CNTR },
/*
 *  EGP Group as specified in RFC 1066.		4/6 variables
 */
{ &egpInMsgs,  		 NULL,    NULL, 0, 	   NULL_OBJINST|VAL_CNTR },
{ &egpInErrors, 	 NULL,    NULL, 0, 	   NULL_OBJINST|VAL_CNTR },
{ &egpOutMsgs,  	 NULL,    NULL, 0, 	   NULL_OBJINST|VAL_CNTR },
{ &egpOutErrors,  	 NULL,    NULL, 0, 	   NULL_OBJINST|VAL_CNTR },

/*********************** MIT *************************/
#ifdef MIT
#ifdef ATHENA
{ &machType,     lu_machtype, NULL, N_MACHTYPE,     NULL_OBJINST|VAL_STR  },
{ &machNDisplay, lu_machtype, NULL, N_MACHNDISPLAY, NULL_OBJINST|VAL_INT  },
{ &machDisplay,  lu_machtype, NULL, N_MACHDISPLAY,  VAL_STR  },
{ &machNDisks,   lu_machtype, NULL, N_MACHNDISK,    NULL_OBJINST|VAL_INT  },
{ &machDisks,    lu_machtype, NULL, N_MACHDISK,     VAL_STR  },
{ &machMemory,   lu_machtype, NULL, N_MACHMEMORY,   NULL_OBJINST|VAL_INT  },

{ &rcHOST,       lu_rcvar,    NULL, N_RCHOST,       NULL_OBJINST|VAL_STR  },
{ &rcADDR,       lu_rcvar,    NULL, N_RCADDR,       NULL_OBJINST|VAL_STR  },
{ &rcMACHINE,    lu_rcvar,    NULL, N_RCMACHINE,    NULL_OBJINST|VAL_STR  },
{ &rcSYSTEM,     lu_rcvar,    NULL, N_RCSYSTEM,     NULL_OBJINST|VAL_STR  },
{ &rcWS,         lu_rcvar,    NULL, N_RCWS,         NULL_OBJINST|VAL_STR  },
{ &rcTOEHOLD,    lu_rcvar,    NULL, N_RCTOEHOLD,    NULL_OBJINST|VAL_STR  },
{ &rcPUBLIC,     lu_rcvar,    NULL, N_RCPUBLIC,     NULL_OBJINST|VAL_STR  },
{ &rcERRHALT,    lu_rcvar,    NULL, N_RCERRHALT,    NULL_OBJINST|VAL_STR  },
{ &rcLPD,        lu_rcvar,    NULL, N_RCLPD,        NULL_OBJINST|VAL_STR  },
{ &rcRVDSRV,     lu_rcvar,    NULL, N_RCRVDSRV,     NULL_OBJINST|VAL_STR  },
{ &rcRVDCLIENT,  lu_rcvar,    NULL, N_RCRVDCLIENT,  NULL_OBJINST|VAL_STR  },
{ &rcNFSSRV,     lu_rcvar,    NULL, N_RCNFSSRV,     NULL_OBJINST|VAL_STR  },
{ &rcNFSCLIENT,  lu_rcvar,    NULL, N_RCNFSCLIENT,  NULL_OBJINST|VAL_STR  },
{ &rcAFSSRV,     lu_rcvar,    NULL, N_RCAFSSRV,     NULL_OBJINST|VAL_STR  },
{ &rcAFSCLIENT,  lu_rcvar,    NULL, N_RCAFSCLIENT,  NULL_OBJINST|VAL_STR  },
{ &rcRPC,        lu_rcvar,    NULL, N_RCRPC,        NULL_OBJINST|VAL_STR  },
{ &rcSAVECORE,   lu_rcvar,    NULL, N_RCSAVECORE,   NULL_OBJINST|VAL_STR  },
{ &rcPOP,        lu_rcvar,    NULL, N_RCPOP,        NULL_OBJINST|VAL_STR  },
{ &rcSENDMAIL,   lu_rcvar,    NULL, N_RCSENDMAIL,   NULL_OBJINST|VAL_STR  },
{ &rcQUOTAS,     lu_rcvar,    NULL, N_RCQUOTAS,     NULL_OBJINST|VAL_STR  },
{ &rcACCOUNT,    lu_rcvar,    NULL, N_RCACCOUNT,    NULL_OBJINST|VAL_STR  },
{ &rcOLC,        lu_rcvar,    NULL, N_RCOLC,        NULL_OBJINST|VAL_STR  },
{ &rcTIMESRV,    lu_rcvar,    NULL, N_RCTIMESRV,    NULL_OBJINST|VAL_STR  },
{ &rcPCNAMED,    lu_rcvar,    NULL, N_RCPCNAMED,    NULL_OBJINST|VAL_STR  },
{ &rcNEWMAILCF,  lu_rcvar,    NULL, N_RCNEWMAILCF,  NULL_OBJINST|VAL_STR  },
{ &rcKNETD,      lu_rcvar,    NULL, N_RCKNETD,      NULL_OBJINST|VAL_STR  },
{ &rcTIMEHUB,    lu_rcvar,    NULL, N_RCTIMEHUB,    NULL_OBJINST|VAL_STR  },
{ &rcZCLIENT,    lu_rcvar,    NULL, N_RCZCLIENT,    NULL_OBJINST|VAL_STR  },
{ &rcZSERVER,    lu_rcvar,    NULL, N_RCZSERVER,    NULL_OBJINST|VAL_STR  },
{ &rcSMSUPDATE,  lu_rcvar,    NULL, N_RCSMSUPDATE,  NULL_OBJINST|VAL_STR  },
{ &rcINETD,      lu_rcvar,    NULL, N_RCINETD,      NULL_OBJINST|VAL_STR  },
{ &rcNOCREATE,   lu_rcvar,    NULL, N_RCNOCREATE,   NULL_OBJINST|VAL_STR  },
{ &rcNOATTACH,   lu_rcvar,    NULL, N_RCNOATTACH,   NULL_OBJINST|VAL_STR  },
{ &rcAFSADJUST,  lu_rcvar,    NULL, N_RCAFSADJUST,  NULL_OBJINST|VAL_STR  },
{ &rcSNMP,       lu_rcvar,    NULL, N_RCSNMP,       NULL_OBJINST|VAL_STR  },
{ &rcAUTOUPDATE, lu_rcvar,    NULL, N_RCAUTOUPDATE, NULL_OBJINST|VAL_STR  },
{ &rcTIMECLIENT, lu_rcvar,    NULL, N_RCTIMECLIENT, NULL_OBJINST|VAL_STR  },
{ &rcKRBSRV,     lu_rcvar,    NULL, N_RCKRBSRV,     NULL_OBJINST|VAL_STR  },
{ &rcKADMSRV,    lu_rcvar,    NULL, N_RCKADMSRV,    NULL_OBJINST|VAL_STR  },
{ &rcNIPSRV,     lu_rcvar,    NULL, N_RCNIPSRV,     NULL_OBJINST|VAL_STR  },
{ &rcFile,       lu_rcvar,    NULL, N_RCFILE,       NULL_OBJINST|VAL_STR  },
{ &relVersion,   lu_relvers,  NULL, N_RELVERSION,   NULL_OBJINST|VAL_STR  },
{ &relDate,      lu_relvers,  NULL, N_RELVERSDATE,  NULL_OBJINST|VAL_STR  },
{ &statOVersion, lu_relvers,  NULL, N_RELVERSSTR,   NULL_OBJINST|VAL_STR  },
{ &packNum,      lu_spnum,    NULL, N_SYSPACKNUM,   NULL_OBJINST|VAL_INT  },
{ &packName,     lu_spvers,   NULL, N_SYSPACKNAME,  VAL_STR  },
{ &packType,     lu_spvers,   NULL, N_SYSPACKTYPE,  VAL_STR  },
{ &packVersion,  lu_spvers,   NULL, N_SYSPACKVERS,  VAL_STR  },
{ &packDate,     lu_spvers,   NULL, N_SYSPACKDATE,  VAL_STR  },
{ &srvNumber,    lu_service,  NULL, N_SRVNUMBER,    NULL_OBJINST|VAL_INT  },
{ &srvName,      lu_servtbl,  NULL, N_SRVNAME,      VAL_STR  },
{ &srvType,      lu_servtbl,  NULL, N_SRVTYPE,      VAL_STR  },
{ &srvVersion,   lu_servtbl,  NULL, N_SRVVERSION,   VAL_STR  },
{ &srvFile,      lu_service,  NULL, N_SRVFILE,      NULL_OBJINST|VAL_STR  },
#endif ATHENA

{ &snmpVersion,  lu_snmpvers, NULL, N_SNMPVERS,     NULL_OBJINST|VAL_STR  },
{ &snmpCompTime, lu_snmpvers, NULL, N_SNMPBUILD,    NULL_OBJINST|VAL_STR  },

#ifdef ATHENA
{ &osVendor,     lu_machtype, NULL, N_MACHOSVENDOR, NULL_OBJINST|VAL_STR  },
{ &osType,       lu_machtype, NULL, N_MACHOS,       NULL_OBJINST|VAL_STR  },
{ &osVersion,    lu_machtype, NULL, N_MACHOSVERSION,NULL_OBJINST|VAL_STR  },
#endif /* ATHENA */

{ &statTime,     lu_status,   NULL, N_STATTIME,     NULL_OBJINST|VAL_INT  },
#ifndef RSPOS
{ &loadRunTime,  lu_status,   NULL, N_STATLOAD,     NULL_OBJINST|VAL_INT  },
#endif /* RSPOS */
{ &statLogin, 	 lu_status,   NULL, N_STATLOGIN,    NULL_OBJINST|VAL_INT  },

#if !defined(ultrix) && !defined(RSPOS)
{ &statDkNParts, lu_ndparts,  NULL, N_PTTOTAL,      NULL_OBJINST|VAL_INT  },
{ &statDkPath,   lu_disk,     NULL, N_DKPATH,       VAL_STR  },
{ &statDkDname,  lu_disk,     NULL, N_DKDNAME,      VAL_STR  },
{ &statDkTotal,  lu_disk,     NULL, N_PTTOTAL,      VAL_INT  },
{ &statDkUsed,   lu_disk,     NULL, N_PTUSED,       VAL_INT  },
{ &statDkFree,   lu_disk,     NULL, N_PTFREE,       VAL_INT  },
{ &statDkAvail,  lu_disk,     NULL, N_PTAVAIL,      VAL_INT  },
{ &statDkITotal, lu_disk,     NULL, N_PITOTAL,      VAL_INT  },
{ &statDkIUsed,  lu_disk,     NULL, N_PIUSED,       VAL_INT  },
{ &statDkIFree,  lu_disk,     NULL, N_PIFREE,       VAL_INT  },
{ &statDkType,   lu_disk,     NULL, N_DKTYPE,       VAL_STR  },
#endif

#ifndef RSPOS
#ifndef decmips
{ &vpR,          lu_vmstat,   NULL, N_VMPROCR,      NULL_OBJINST|VAL_INT  },
{ &vpB,          lu_vmstat,   NULL, N_VMPROCB,      NULL_OBJINST|VAL_INT  },
{ &vpW,          lu_vmstat,   NULL, N_VMPROCW,      NULL_OBJINST|VAL_INT  },
{ &vpAvm,        lu_vmstat,   NULL, N_VMMEMAVM,     NULL_OBJINST|VAL_INT  },
{ &vpFre,        lu_vmstat,   NULL, N_VMMEMFRE,     NULL_OBJINST|VAL_INT  },

{ &vpRe,         lu_vmstat,   NULL, N_VMPAGERE,     NULL_OBJINST|VAL_GAUGE },
{ &vpAt,         lu_vmstat,   NULL, N_VMPAGEAT,     NULL_OBJINST|VAL_GAUGE },
{ &vpPi,         lu_vmstat,   NULL, N_VMPAGEPI,     NULL_OBJINST|VAL_GAUGE },
{ &vpPo,         lu_vmstat,   NULL, N_VMPAGEPO,     NULL_OBJINST|VAL_GAUGE },
{ &vpFr,         lu_vmstat,   NULL, N_VMPAGEFR,     NULL_OBJINST|VAL_GAUGE },
{ &vpDe,         lu_vmstat,   NULL, N_VMPAGEDE,     NULL_OBJINST|VAL_GAUGE },
{ &vpSr,         lu_vmstat,   NULL, N_VMPAGESR,     NULL_OBJINST|VAL_GAUGE },

{ &vsSwpIn,      lu_vmstat,   NULL, N_VMSWAPIN,     NULL_OBJINST|VAL_CNTR },
{ &vsSwpOut,     lu_vmstat,   NULL, N_VMSWAPOUT,    NULL_OBJINST|VAL_CNTR },
{ &vsPgSwpIn,    lu_vmstat,   NULL, N_VMPGSWAPOUT,  NULL_OBJINST|VAL_CNTR },
{ &vsPgSwpOut,   lu_vmstat,   NULL, N_VMPGSWAPOUT,  NULL_OBJINST|VAL_CNTR },
{ &vsAtFlt,      lu_vmstat,   NULL, N_VMATFAULTS,   NULL_OBJINST|VAL_CNTR },
{ &vsPgSeqFre,   lu_vmstat,   NULL, N_VMPGSEQFREE,  NULL_OBJINST|VAL_CNTR },
{ &vsPgRec,      lu_vmstat,   NULL, N_VMPGREC,      NULL_OBJINST|VAL_CNTR },
{ &vsPgFRec,     lu_vmstat,   NULL, N_VMPGFASTREC,  NULL_OBJINST|VAL_CNTR },
{ &vsTotRec,     lu_vmstat,   NULL, N_VMFLRECLAIM,  NULL_OBJINST|VAL_CNTR },
{ &vsBlkPgFlt,   lu_vmstat,   NULL, N_VMITBLKPGFAULT,NULL_OBJINST|VAL_CNTR },
{ &vsZFCreat,    lu_vmstat,   NULL, N_VMZFPGCREATE, NULL_OBJINST|VAL_CNTR },
{ &vsZFFault,    lu_vmstat,   NULL, N_VMZFPGFAULT,  NULL_OBJINST|VAL_CNTR },
{ &vsEFCreat,    lu_vmstat,   NULL, N_VMEFPGCREATE, NULL_OBJINST|VAL_CNTR },
{ &vsEFFault,    lu_vmstat,   NULL, N_VMEFPGFAULT,  NULL_OBJINST|VAL_CNTR },
{ &vsSwpFree,    lu_vmstat,   NULL, N_VMSTPGFRE,    NULL_OBJINST|VAL_CNTR },
{ &vsInodeFre,   lu_vmstat,   NULL, N_VMITPGFRE,    NULL_OBJINST|VAL_CNTR },
{ &vsFFCreat,    lu_vmstat,   NULL, N_VMFFPGCREATE, NULL_OBJINST|VAL_CNTR },
{ &vsFFFault,    lu_vmstat,   NULL, N_VMFFPGFAULT,  NULL_OBJINST|VAL_CNTR },
{ &vsPgClock,    lu_vmstat,   NULL, N_VMPGSCAN,     NULL_OBJINST|VAL_CNTR },
{ &vsClkRev,     lu_vmstat,   NULL, N_VMCLKREV,     NULL_OBJINST|VAL_CNTR },
{ &vsClkPgFre,   lu_vmstat,   NULL, N_VMCLKFREE,    NULL_OBJINST|VAL_CNTR },
{ &vsConSwtch,   lu_vmstat,   NULL, N_VMCSWITCH,    NULL_OBJINST|VAL_CNTR },
{ &vsDevInt,     lu_vmstat,   NULL, N_VMDINTR,      NULL_OBJINST|VAL_CNTR },
{ &vsSoftInt,    lu_vmstat,   NULL, N_VMSINTR,      NULL_OBJINST|VAL_CNTR },
#if defined (vax)
{ &vsDMAInt,     lu_vmstat,   NULL, N_VMPDMAINTR,   NULL_OBJINST|VAL_CNTR },
#endif
{ &vsTrap,       lu_vmstat,   NULL, N_VMTRAP,       NULL_OBJINST|VAL_CNTR },
{ &vsSysCall,    lu_vmstat,   NULL, N_VMSYSCALL,    NULL_OBJINST|VAL_CNTR },

{ &vxCall,       lu_vmstat,   NULL, N_VMXACALL,     NULL_OBJINST|VAL_CNTR  },
{ &vxHit,        lu_vmstat,   NULL, N_VMXAHIT,      NULL_OBJINST|VAL_CNTR  },
{ &vxStick,      lu_vmstat,   NULL, N_VMXASTICK,    NULL_OBJINST|VAL_CNTR  },
{ &vxFlush,      lu_vmstat,   NULL, N_VMXAFLUSH,    NULL_OBJINST|VAL_CNTR  },
{ &vxUnuse,      lu_vmstat,   NULL, N_VMXAUNUSE,    NULL_OBJINST|VAL_CNTR  },
{ &vxFreCall,    lu_vmstat,   NULL, N_VMXFRECALL,   NULL_OBJINST|VAL_CNTR  },
{ &vxFreInuse,   lu_vmstat,   NULL, N_VMXFREINUSE,  NULL_OBJINST|VAL_CNTR  },
{ &vxFreCache,   lu_vmstat,   NULL, N_VMXFRECACHE,  NULL_OBJINST|VAL_CNTR  },
{ &vxFreSwap,    lu_vmstat,   NULL, N_VMXFRESWP,    NULL_OBJINST|VAL_CNTR  },

{ &vfFork,       lu_vmstat,   NULL, N_VMFORK,       NULL_OBJINST|VAL_CNTR  },
{ &vfPage,       lu_vmstat,   NULL, N_VMFKPAGE,     NULL_OBJINST|VAL_CNTR  },
{ &vfVFork,      lu_vmstat,   NULL, N_VMVFORK,      NULL_OBJINST|VAL_CNTR  },
{ &vfVPage,      lu_vmstat,   NULL, N_VMVFKPAGE,    NULL_OBJINST|VAL_CNTR  },

{ &vtRec,        lu_vmstat,   NULL, N_VMRECTIME,    NULL_OBJINST|VAL_INT   },
{ &vtPgIn,       lu_vmstat,   NULL, N_VMPGINTIME,   NULL_OBJINST|VAL_INT   },

#ifdef VFS
{ &vncsGood,     lu_vmstat,   NULL, N_VMNCSGOOD,    NULL_OBJINST|VAL_CNTR  },
{ &vncsBad,      lu_vmstat,   NULL, N_VMNCSBAD,     NULL_OBJINST|VAL_CNTR  },
{ &vncsFalse,    lu_vmstat,   NULL, N_VMNCSFALSE,   NULL_OBJINST|VAL_CNTR  },
{ &vncsMiss,     lu_vmstat,   NULL, N_VMNCSMISS,    NULL_OBJINST|VAL_CNTR  },
{ &vncsLong,     lu_vmstat,   NULL, N_VMNCSLONG,    NULL_OBJINST|VAL_CNTR  },
{ &vncsTotal,    lu_vmstat,   NULL, N_VMNCSTOTAL,   NULL_OBJINST|VAL_CNTR  },
#else  VFS
{ &vnchGood,     lu_vmstat,   NULL, N_VMNCHGOOD,    NULL_OBJINST|VAL_CNTR  },
{ &vnchBad,      lu_vmstat,   NULL, N_VMNCHBAD,     NULL_OBJINST|VAL_CNTR  },
{ &vnchFalse,    lu_vmstat,   NULL, N_VMNCHFALSE,   NULL_OBJINST|VAL_CNTR  },
{ &vnchMiss,     lu_vmstat,   NULL, N_VMNCHMISS,    NULL_OBJINST|VAL_CNTR  },
{ &vnchLong,     lu_vmstat,   NULL, N_VMNCHLONG,    NULL_OBJINST|VAL_CNTR  },
{ &vnchTotal,    lu_vmstat,   NULL, N_VMNCHTOTAL,   NULL_OBJINST|VAL_CNTR  },
#endif VFS

{ &vcUs,         lu_vmstat,   NULL, N_VMCPUUS,      NULL_OBJINST|VAL_INT  },
{ &vcNi,         lu_vmstat,   NULL, N_VMCPUNI,      NULL_OBJINST|VAL_INT  },
{ &vcSy,         lu_vmstat,   NULL, N_VMCPUSY,      NULL_OBJINST|VAL_INT  },
{ &vcId,         lu_vmstat,   NULL, N_VMCPUID,      NULL_OBJINST|VAL_INT  },
		 			
{ &vmmIn,        lu_vmstat,   NULL, N_VMMISCIN,     NULL_OBJINST|VAL_GAUGE },
{ &vmmSy,        lu_vmstat,   NULL, N_VMMISCSY,     NULL_OBJINST|VAL_GAUGE },
{ &vmmCs,        lu_vmstat,   NULL, N_VMMISCCS,     NULL_OBJINST|VAL_GAUGE },
	  
{ &psTotal,      lu_pstat,    NULL, N_PSTOTAL,      NULL_OBJINST|VAL_INT  },
{ &psUsed,       lu_pstat,    NULL, N_PSUSED,       NULL_OBJINST|VAL_INT  },
{ &psTUsed,      lu_pstat,    NULL, N_PSTUSED,      NULL_OBJINST|VAL_INT  },
{ &psFree,       lu_pstat,    NULL, N_PSFREE,       NULL_OBJINST|VAL_INT  },
{ &psWasted,     lu_pstat,    NULL, N_PSWASTED,     NULL_OBJINST|VAL_INT  },
{ &psMissing,    lu_pstat,    NULL, N_PSMISSING,    NULL_OBJINST|VAL_INT  },
{ &pfTotal,      lu_pstat,    NULL, N_PFTOTAL,      NULL_OBJINST|VAL_INT  },
{ &pfUsed,       lu_pstat,    NULL, N_PFUSED,       NULL_OBJINST|VAL_INT  },
{ &piTotal,      lu_pstat,    NULL, N_PITOTAL,      NULL_OBJINST|VAL_INT  },
{ &piUsed,       lu_pstat,    NULL, N_PIUSED,       NULL_OBJINST|VAL_INT  },
{ &ppTotal,      lu_pstat,    NULL, N_PPTOTAL,      NULL_OBJINST|VAL_INT  },
{ &ppUsed,       lu_pstat,    NULL, N_PPUSED,       NULL_OBJINST|VAL_INT  },
{ &ptTotal,      lu_pstat,    NULL, N_PTTOTAL,      NULL_OBJINST|VAL_INT  },
{ &ptUsed,       lu_pstat,    NULL, N_PTUSED,       NULL_OBJINST|VAL_INT  },
{ &ptActive,     lu_pstat,    NULL, N_PTACTIVE,     NULL_OBJINST|VAL_INT  },
#endif /* decmips */
#endif /* RSPOS */		 	
	
#ifdef RPC		 
{ &rpcCall,      lu_rpccl,    NULL, N_RPCCCALL,     NULL_OBJINST|VAL_CNTR },
{ &rpcBadCall,   lu_rpccl,    NULL, N_RPCCBADCALL,  NULL_OBJINST|VAL_CNTR },
{ &rpcRetrans,   lu_rpccl,    NULL, N_RPCCRETRANS,  NULL_OBJINST|VAL_CNTR },
{ &rpcBadXid,    lu_rpccl,    NULL, N_RPCCBADXID,   NULL_OBJINST|VAL_CNTR },
{ &rpcTimeout,   lu_rpccl,    NULL, N_RPCCTIMEOUT,  NULL_OBJINST|VAL_CNTR },
{ &rpcWait,      lu_rpccl,    NULL, N_RPCCWAIT,     NULL_OBJINST|VAL_CNTR },
{ &rpcNewCred,   lu_rpccl,    NULL, N_RPCCNEWCRED,  NULL_OBJINST|VAL_CNTR },
		 	      		  
{ &rpsCall,      lu_rpcsv,    NULL, N_RPCSCALL,     NULL_OBJINST|VAL_CNTR },
{ &rpsBadCall,   lu_rpcsv,    NULL, N_RPCSBADCALL,  NULL_OBJINST|VAL_CNTR },
{ &rpsNullRecv,  lu_rpcsv,    NULL, N_RPCSNULLRECV, NULL_OBJINST|VAL_CNTR },
{ &rpsBadLen,    lu_rpcsv,    NULL, N_RPCSBADLEN,   NULL_OBJINST|VAL_CNTR },
{ &rpsXDRCall,   lu_rpcsv,    NULL, N_RPCSXDRCALL,  NULL_OBJINST|VAL_CNTR },
{ &rpsCredUt,    lu_tuchtime, NULL, N_RPCCRED,      NULL_OBJINST|VAL_STR  },
{ &rpsCredUtPag, lu_tuchtime, NULL, N_RPCCREDPAG,   NULL_OBJINST|VAL_STR  },
{ &rpsCredUtDir, lu_tuchtime, NULL, N_RPCCREDDIR,   NULL_OBJINST|VAL_STR  },
{ &rpsCredFile,  lu_tuchtime, NULL, N_RPCCREDFILE,  NULL_OBJINST|VAL_STR  },
#endif RPC

#ifdef NFS		 				  
{ &ncCall,       lu_nfscl,    NULL, N_NFSCCALL,     NULL_OBJINST|VAL_CNTR },
{ &ncBadCall,    lu_nfscl,    NULL, N_NFSCBADCALL,  NULL_OBJINST|VAL_CNTR },
{ &ncNULL,       lu_nfscl,    NULL, N_NFSCNULL,     NULL_OBJINST|VAL_CNTR },
{ &ncGetAddr,    lu_nfscl,    NULL, N_NFSCGETADDR,  NULL_OBJINST|VAL_CNTR },
{ &ncSetAddr,    lu_nfscl,    NULL, N_NFSCSETADDR,  NULL_OBJINST|VAL_CNTR },
{ &ncRoot,       lu_nfscl,    NULL, N_NFSCROOT,     NULL_OBJINST|VAL_CNTR },
{ &ncLookup,     lu_nfscl,    NULL, N_NFSCLOOKUP,   NULL_OBJINST|VAL_CNTR },
{ &ncReadLink,   lu_nfscl,    NULL, N_NFSCREADLINK, NULL_OBJINST|VAL_CNTR },
{ &ncRead,       lu_nfscl,    NULL, N_NFSCREAD,     NULL_OBJINST|VAL_CNTR },
{ &ncWRCache,    lu_nfscl,    NULL, N_NFSCWRCACHE,  NULL_OBJINST|VAL_CNTR },
{ &ncWrite,      lu_nfscl,    NULL, N_NFSCWRITE,    NULL_OBJINST|VAL_CNTR },
{ &ncCreate,     lu_nfscl,    NULL, N_NFSCWRITE,    NULL_OBJINST|VAL_CNTR },
{ &ncRemove,     lu_nfscl,    NULL, N_NFSCREMOVE,   NULL_OBJINST|VAL_CNTR },
{ &ncLink,       lu_nfscl,    NULL, N_NFSCLINK,     NULL_OBJINST|VAL_CNTR },
{ &ncSymLink,    lu_nfscl,    NULL, N_NFSCSYMLINK,  NULL_OBJINST|VAL_CNTR },
{ &ncMkDir,      lu_nfscl,    NULL, N_NFSCMKDIR,    NULL_OBJINST|VAL_CNTR },
{ &ncRmDir,      lu_nfscl,    NULL, N_NFSCRMDIR,    NULL_OBJINST|VAL_CNTR },
{ &ncReadDir,    lu_nfscl,    NULL, N_NFSCREADDIR,  NULL_OBJINST|VAL_CNTR },
{ &ncFSStat,     lu_nfscl,    NULL, N_NFSCFSSTAT,   NULL_OBJINST|VAL_CNTR },
{ &ncNCLGet,     lu_nfscl,    NULL, N_NFSCNCLGET,   NULL_OBJINST|VAL_CNTR },
{ &ncNCLSleep,   lu_nfscl,    NULL, N_NFSCNCLSLEEP, NULL_OBJINST|VAL_CNTR },

{ &nsCall,       lu_nfssv,    NULL, N_NFSSCALL,     NULL_OBJINST|VAL_CNTR },
{ &nsBadCall,    lu_nfssv,    NULL, N_NFSSBADCALL,  NULL_OBJINST|VAL_CNTR },
{ &nsNull,       lu_nfssv,    NULL, N_NFSSNULL,     NULL_OBJINST|VAL_CNTR },
{ &nsGetAddr,    lu_nfssv,    NULL, N_NFSSGETADDR,  NULL_OBJINST|VAL_CNTR },
{ &nsSetAddr,    lu_nfssv,    NULL, N_NFSSSETADDR,  NULL_OBJINST|VAL_CNTR },
{ &nsRoot,       lu_nfssv,    NULL, N_NFSSROOT,     NULL_OBJINST|VAL_CNTR },
{ &nsLookup,     lu_nfssv,    NULL, N_NFSSLOOKUP,   NULL_OBJINST|VAL_CNTR },
{ &nsReadLink,   lu_nfssv,    NULL, N_NFSSREADLINK, NULL_OBJINST|VAL_CNTR },
{ &nsRead,       lu_nfssv,    NULL, N_NFSSREAD,     NULL_OBJINST|VAL_CNTR },
{ &nsWRCache,    lu_nfssv,    NULL, N_NFSSWRCACHE,  NULL_OBJINST|VAL_CNTR },
{ &nsWrite,      lu_nfssv,    NULL, N_NFSSWRITE,    NULL_OBJINST|VAL_CNTR },
{ &nsCreate,     lu_nfssv,    NULL, N_NFSSCREATE,   NULL_OBJINST|VAL_CNTR },
{ &nsRemove,     lu_nfssv,    NULL, N_NFSSREMOVE,   NULL_OBJINST|VAL_CNTR },
{ &nsRename,     lu_nfssv,    NULL, N_NFSSRENAME,   NULL_OBJINST|VAL_CNTR },
{ &nsLink,       lu_nfssv,    NULL, N_NFSSLINK,     NULL_OBJINST|VAL_CNTR },
{ &nsSymLink,    lu_nfssv,    NULL, N_NFSSSYMLINK,  NULL_OBJINST|VAL_CNTR },
{ &nsMkDir,      lu_nfssv,    NULL, N_NFSSMKDIR,    NULL_OBJINST|VAL_CNTR },
{ &nsRmDir,      lu_nfssv,    NULL, N_NFSSRMDIR,    NULL_OBJINST|VAL_CNTR },
{ &nsReadDir,    lu_nfssv,    NULL, N_NFSSREADDIR,  NULL_OBJINST|VAL_CNTR },
{ &nsFSStat,     lu_nfssv,    NULL, N_NFSSFSSTAT,   NULL_OBJINST|VAL_CNTR },
#endif NFS

#ifdef RVD
{ &rvcBadBlock,  lu_rvdcl,    NULL, N_RVDCBADBLOCK, NULL_OBJINST|VAL_CNTR },
{ &rvcBadCkSum,  lu_rvdcl,    NULL, N_RVDCBADCKSUM, NULL_OBJINST|VAL_CNTR },
{ &rvcBadType,   lu_rvdcl,    NULL, N_RVDCBADTYPE,  NULL_OBJINST|VAL_CNTR },
{ &rvcBadState,  lu_rvdcl,    NULL, N_RVDCBADSTATE, NULL_OBJINST|VAL_CNTR },
{ &rvcBadFormat, lu_rvdcl,    NULL, N_RVDCBADFORMAT,NULL_OBJINST|VAL_CNTR },
{ &rvcTimeout,   lu_rvdcl,    NULL, N_RVDCTIMEOUT,  NULL_OBJINST|VAL_CNTR },
{ &rvcBadNonce,  lu_rvdcl,    NULL, N_RVDCBADNONCE, NULL_OBJINST|VAL_CNTR },
{ &rvcErrorRecv, lu_rvdcl,    NULL, N_RVDCERRORRECV,NULL_OBJINST|VAL_CNTR },
{ &rvcBadData,   lu_rvdcl,    NULL, N_RVDCBADDATA,  NULL_OBJINST|VAL_CNTR },
{ &rvcBadVers,   lu_rvdcl,    NULL, N_RVDCBADVERS,  NULL_OBJINST|VAL_CNTR },
{ &rvcPktRej,    lu_rvdcl,    NULL, N_RVDCPKTREJ,   NULL_OBJINST|VAL_CNTR },
{ &rvcPush,      lu_rvdcl,    NULL, N_RVDCPUSH,     NULL_OBJINST|VAL_CNTR },
{ &rvcPktSent,   lu_rvdcl,    NULL, N_RVDCPKTSENT,  NULL_OBJINST|VAL_CNTR },
{ &rvcPktRecv,   lu_rvdcl,    NULL, N_RVDCPKTRECV,  NULL_OBJINST|VAL_CNTR },
{ &rvcQkRetrans, lu_rvdcl,    NULL, N_RVDCQKRETRANS,NULL_OBJINST|VAL_CNTR },
{ &rvcLgRetrans, lu_rvdcl,    NULL, N_RVDCLGRETRANS,NULL_OBJINST|VAL_CNTR },
{ &rvcBlockRd,   lu_rvdcl,    NULL, N_RVDCBLOCKRD,  NULL_OBJINST|VAL_CNTR },
{ &rvcBlockWr,   lu_rvdcl,    NULL, N_RVDCBLOCKWR,  NULL_OBJINST|VAL_CNTR },
#endif RVD

#ifdef AFS
{ &acCacheSize,  lu_afs,      NULL, N_AFSCACHESIZE, NULL_OBJINST|VAL_INT  },
{ &acCacheFile,  lu_afs,      NULL, N_AFSCACHEFILE, NULL_OBJINST|VAL_STR  },
{ &acThisCell,   lu_afs,      NULL, N_AFSTHISCELL,  NULL_OBJINST|VAL_STR  },
{ &acCellFile,   lu_afs,      NULL, N_AFSCELLFILE,  NULL_OBJINST|VAL_STR  },
{ &acSuidCell,   lu_afsdb,    NULL, N_AFSSUIDCELL,  VAL_STR  },
{ &acSuidFile,   lu_afs,      NULL, N_AFSSUIDFILE,  NULL_OBJINST|VAL_STR  },
{ &acCellSrvName,lu_afsdb,    NULL, N_AFSDBNAME,    VAL_STR  },
{ &acCellSrvAddr,lu_afsdb,    NULL, N_AFSDBADDR,    VAL_STR  },
{ &acCellSrvCom, lu_afsdb,    NULL, N_AFSDBCOMMENT, VAL_STR  },
{ &acCellSrvFile,lu_afs,      NULL, N_AFSSRVFILE,   NULL_OBJINST|VAL_STR  },
#endif AFS

#ifdef KERBEROS
{ &kcRealm,      lu_kerberos, NULL, N_KRBCREALM,    NULL_OBJINST|VAL_STR  },
#endif KERBEROS

#ifdef ZEPHYR
{ &zcPVersion,   lu_zephyr,   NULL, N_ZCPVERSION,   NULL_OBJINST|VAL_STR  },
{ &zcServer,     lu_zephyr,   NULL, N_ZCSERVER,     NULL_OBJINST|VAL_STR  },
{ &zcQueue,      lu_zephyr,   NULL, N_ZCQUEUE,      NULL_OBJINST|VAL_INT  },
{ &zcServChange, lu_zephyr,   NULL, N_ZCPSERVCHANGE,NULL_OBJINST|VAL_CNTR },
{ &zcHeader,     lu_zephyr,   NULL, N_ZCHEADER,     NULL_OBJINST|VAL_STR  },
{ &zcLooking,    lu_zephyr,   NULL, N_ZCLOOKING,    NULL_OBJINST|VAL_INT  },
{ &zcUptime,     lu_zephyr,   NULL, N_ZCUPTIME,     NULL_OBJINST|VAL_INT  },
{ &zcSize,       lu_zephyr,   NULL, N_ZCSIZE,       NULL_OBJINST|VAL_INT  },

{ &zsPVersion,   lu_zephyr,   NULL, N_ZSPVERSION,   NULL_OBJINST|VAL_STR  },
{ &zsHeader,     lu_zephyr,   NULL, N_ZSHEADER,     NULL_OBJINST|VAL_STR  },
{ &zsPacket,     lu_zephyr,   NULL, N_ZSPACKET,     NULL_OBJINST|VAL_CNTR },
{ &zsUptime,     lu_zephyr,   NULL, N_ZSUPTIME,     NULL_OBJINST|VAL_INT  },
{ &zsState,      lu_zephyr,   NULL, N_ZSSTATE,      NULL_OBJINST|VAL_STR  },
#endif ZEPHYR

{ &mQueue,       lu_mail,     NULL, N_MAILALIAS,    NULL_OBJINST|VAL_INT  },
{ &mAliasUt,     lu_tuchtime, NULL, N_MAILALIAS,    NULL_OBJINST|VAL_STR  },
{ &mAliasUtPag,  lu_tuchtime, NULL, N_MAILALIASPAG, NULL_OBJINST|VAL_STR  },
{ &mAliasUtDir,  lu_tuchtime, NULL, N_MAILALIASDIR, NULL_OBJINST|VAL_STR  },
{ &mAliasFile,   lu_tuchtime, NULL, N_MAILALIASFILE,NULL_OBJINST|VAL_STR  },

#ifdef DNS
{ &dnsUpdateTime,lu_dns,      NULL, N_DNSUPDATETIME,NULL_OBJINST|VAL_STR  },
{ &dnsBootTime,  lu_dns,      NULL, N_DNSBOOTTIME,  NULL_OBJINST|VAL_TIME },
{ &dnsResetTime, lu_dns,      NULL, N_DNSRESETTIME, NULL_OBJINST|VAL_TIME },
{ &dnsPacketIn,  lu_dns,      NULL, N_DNSPACKETIN,  NULL_OBJINST|VAL_CNTR },
{ &dnsPacketOut, lu_dns,      NULL, N_DNSPACKETOUT, NULL_OBJINST|VAL_CNTR },
{ &dnsQuery,     lu_dns,      NULL, N_DNSQUERY,     NULL_OBJINST|VAL_CNTR },
{ &dnsIQuery,    lu_dns,      NULL, N_DNSIQUERY,    NULL_OBJINST|VAL_CNTR },
{ &dnsDupQuery,  lu_dns,      NULL, N_DNSDUPQUERY,  NULL_OBJINST|VAL_CNTR },
{ &dnsResponse,  lu_dns,      NULL, N_DNSRESPONSE,  NULL_OBJINST|VAL_CNTR },
{ &dnsDupResp,   lu_dns,      NULL, N_DNSDUPRESP,   NULL_OBJINST|VAL_CNTR },
{ &dnsOK,        lu_dns,      NULL, N_DNSOK,        NULL_OBJINST|VAL_CNTR },
{ &dnsFail,      lu_dns,      NULL, N_DNSFAIL,      NULL_OBJINST|VAL_CNTR },
{ &dnsFormErr,   lu_dns,      NULL, N_DNSFORMERR,   NULL_OBJINST|VAL_CNTR },
{ &dnsSysQuery,  lu_dns,      NULL, N_DNSSYSQUERY,  NULL_OBJINST|VAL_CNTR },
{ &dnsPrimeCache,lu_dns,      NULL, N_DNSPRIMECACHE,NULL_OBJINST|VAL_CNTR },
{ &dnsCheckNS,   lu_dns,      NULL, N_DNSCHECKNS,   NULL_OBJINST|VAL_CNTR },
{ &dnsBadResp,   lu_dns,      NULL, N_DNSBADRESP,   NULL_OBJINST|VAL_CNTR },
{ &dnsMartian,   lu_dns,      NULL, N_DNSMARTIAN,   NULL_OBJINST|VAL_CNTR },
{ &dnsUnknown,   lu_dns,      NULL, N_DNSUNKNOWN,   NULL_OBJINST|VAL_CNTR },
{ &dnsA,         lu_dns,      NULL, N_DNSA,         NULL_OBJINST|VAL_CNTR },
{ &dnsNS,        lu_dns,      NULL, N_DNSNS,        NULL_OBJINST|VAL_CNTR },
{ &dnsCName,     lu_dns,      NULL, N_DNSCNAME,     NULL_OBJINST|VAL_CNTR },
{ &dnsSOA,       lu_dns,      NULL, N_DNSSOA,       NULL_OBJINST|VAL_CNTR },
{ &dnsWKS,       lu_dns,      NULL, N_DNSWKS,       NULL_OBJINST|VAL_CNTR },
{ &dnsPTR,       lu_dns,      NULL, N_DNSPTR,       NULL_OBJINST|VAL_CNTR },
{ &dnsHInfo,     lu_dns,      NULL, N_DNSHINFO,     NULL_OBJINST|VAL_CNTR },
{ &dnsMX,        lu_dns,      NULL, N_DNSMX,        NULL_OBJINST|VAL_CNTR },
{ &dnsTXT,       lu_dns,      NULL, N_DNSTXT,       NULL_OBJINST|VAL_CNTR },
{ &dnsUNSPECA,   lu_dns,      NULL, N_DNSUNSPECA,   NULL_OBJINST|VAL_CNTR },
{ &dnsAXFR,      lu_dns,      NULL, N_DNSAXFR,      NULL_OBJINST|VAL_CNTR },
{ &dnsANY,       lu_dns,      NULL, N_DNSANY,       NULL_OBJINST|VAL_CNTR },
{ &dnsStatFile,  lu_dns,      NULL, N_DNSSTATFILE,  NULL_OBJINST|VAL_STR  },
#endif DNS

#ifdef TIMED
{ &tdMaster,     lu_timed,    NULL, N_TIMEDMASTER,  NULL_OBJINST|VAL_STR  },
#endif TIMED
#endif MIT

/*
 *  NULL termination
 */
{ NULL,              NULL,       NULL, -1,            0                    },
};

/*
 *  namelist structure for nl() call.  This will allow us to
 *  get variables from the kernel.
 */

/* 
 * It's people like IBM who create crap like AIX that gets me upset.
 */

#ifndef RSPOS

struct nlist nl[] = 
{
  { "_ifnet" },		/* N_IFNET    - 0  */
  { "_ipstat" },	/* N_IPSTAT   - 1  */
  { "_rtnet" },		/* N_RTNET    - 2  */
  { "_rthashsize" },	/* N_RTHASH   - 3  */
  { "_rtstat" },	/* N_RTSTAT   - 4  */
  { "_mbstat" },	/* N_MBSTAT   - 5  */
  { "_icmpstat" },	/* N_ICMPSTAT - 6  */
  { "_tcb" },		/* N_TCB      - 7  */
  { "_boottime" },	/* N_BOOT     - 8  */
  { "_rthost" },	/* N_RTHOST   - 9  */
  { "_tcpstat" },	/* N_TCPSTAT  - 10 */
  { "_udpstat" },	/* N_UDPSTAT  - 11 */
  { "_ipforwarding" },	/* N_IPFORWD  - 12 */
  { "_arptab" },	/* N_ARPTAB   - 13 */
  { "_arptab_size" },	/* N_ARPSIZE  - 14 */

  /*
   * mit additions - here we jumble things up a bit based on machine type
   * so we do get too contorted with the indices.
   */

#ifdef MIT
  { "_avenrun" },       /* N_AVENRUN  - 15 */
  { "_hz" },            /* N_HZ       - 16 */
  { "_cp_time" },       /* N_CPTIME   - 17 */
  { "_rate" },          /* N_RATE     - 18 */
  { "_total" },         /* N_TOTAL    - 19 */
  { "_deficit" },       /* N_DEFICIT  - 20 */
  { "_forkstat" },      /* N_FORKSTAT - 21 */
  { "_sum" },           /* N_SUM      - 22 */
  { "_firstfree" },     /* N_FIRSTFRE - 23 */
  { "_maxfree" },       /* N_MAXFREE  - 24 */
  { "_dk_xfer" },       /* N_DKXFER   - 25 */
  { "_rectime" },       /* N_RECTIME  - 26 */
  { "_pgintime" },      /* N_PGINTIME - 27 */
  { "_phz" },           /* N_PHZ      - 28 */
  { "_intrnames" },     /* N_INTRNAM  - 29 */
  { "_eintrnames" },    /* N_EINTERNAM- 30 */
  { "_intrcnt" },       /* N_INTCRNT  - 31 */
  { "_eintrcnt" },      /* N_EINTCRNT - 32 */
  { "_dk_ndrive" },     /* N_DKNDRIVE - 33 */
  { "_xstats" },        /* N_XSTATS   - 34 */
#ifdef decmips
  { "_gnode" },         /* N_INODE    - 35 */
#else  decmips
  { "_inode" },         /* N_INODE    - 35 */
#endif decmips
  { "_text" },          /* N_XTEXT    - 36 */
  { "_proc" },          /* N_PROC     - 37 */
  { "_cons" },          /* N_CONS     - 38 */
  { "_file" },          /* N_FILE     - 39 */
  { "_Usrptmap "},      /* N_USRPTMAP - 40 */
  { "_usrpt" },         /* N_USRPT    - 41 */
  { "_swapmap" },       /* N_SWAPMAP  - 42 */
  { "_nproc" },         /* N_NPROC    - 43 */
  { "_ntext" },         /* N_NTEXT    - 44 */
  { "_nfile" },         /* N_NFILE    - 45 */
#ifdef decmips
  { "_ngnode" },        /* N_NINODE   - 46 */
#else  decmips
  { "_ninode" },        /* N_NINODE   - 46 */
#endif decmips
  { "_nswapmap" },      /* N_NSWAPMAP - 47 */
  { "_pt_tty" },        /* N_PTTTY    - 48 */
  { "_npty" },          /* N_NPTY     - 49 */
  { "_dmmin" },         /* N_DMMIN    - 50 */
  { "_dmmax" },         /* N_DMMAX    - 51 */
  { "_nswdev" },        /* N_NSWDEV   - 52 */
  { "_swdevt" },        /* N_SWDEVT   - 53 */    
  { "_rec" },           /* N_REC      - 54 */
  { "_pgin" },          /* N_PGIN     - 55 */   
  { "_Sysmap" },        /* N_SYSMAP   - 56 */
#ifdef VFS  
  { "_ncstats" },       /* N_NCSTATS  - 57 */
#else  VFS
  { "_nchstats" },      /* N_NCHSTATS - 57 */
#endif VFS
#ifdef RPC
  { "_rcstat" },        /* N_RCSTAT   - 58 */
  { "_rsstat" },        /* N_RSSTAT   - 59 */
#endif RPC
#ifdef NFS
  { "_clstat" },        /* N_CLSSTAT  - 60 */
  { "_svstat" },        /* N_SVSSTAT  - 61 */
#endif NFS
#if defined(vax)
  { "_mbdinit" },       /* N_MBDINIT  - 62 */
  { "_ubdinit" },       /* N_UBDINIT  - 63 */
  { "_dz_tty" },        /* N_DZTTY    - 64 */
  { "_dz_cnt" },        /* N_DZCNT    - 65 */
  { "_dh11" },          /* N_DH11     - 66 */
  { "_ndh11" },         /* N_NDH11    - 67 */
#endif 
#if defined(ibm032)
  { "_iocdinit" },      /* N_IOCDINIT - 62 */
  { "_asy" },           /* N_ASY      - 63 */
  { "_nasy"},           /* N_NASY     - 64 */
  { "_psp" },           /* N_PSP      - 65 */
  { "_npsp" },          /* N_NPSP     - 66 */
#endif 
#endif MIT
    "",
};

#else /* RSPOS */


struct nlist nl[] = 
{
  { "ifnet" },		/* N_IFNET    - 0  */
  { "ipstat" },	        /* N_IPSTAT   - 1  */
  { "rtnet" },		/* N_RTNET    - 2  */
  { "rthashsize" },	/* N_RTHASH   - 3  */
  { "rtstat" },	        /* N_RTSTAT   - 4  */
  { "mbstat" },	        /* N_MBSTAT   - 5  */
  { "icmpstat" },	/* N_ICMPSTAT - 6  */
  { "tcb" },		/* N_TCB      - 7  */
  { "boottime" },       /* N_BOOT     - 8  */
  { "rthost" },	        /* N_RTHOST   - 9  */
  { "tcpstat" },	/* N_TCPSTAT  - 10 */
  { "udpstat" },	/* N_UDPSTAT  - 11 */
  { "ipforwarding" },	/* N_IPFORWD  - 12 */
  { "arptab" },	        /* N_ARPTAB   - 13 */
  { "arptab_size" },	/* N_ARPSIZE  - 14 */

  /*
   * mit additions - here we jumble things up a bit based on machine type
   * so we do get too contorted with the indices.
   */

#ifdef MIT
  { "avenrun" },       /* N_AVENRUN  - 15 */
  { "hz" },            /* N_HZ       - 16 */
  { "cp_time" },       /* N_CPTIME   - 17 */
  { "rate" },          /* N_RATE     - 18 */
  { "total" },         /* N_TOTAL    - 19 */
  { "deficit" },       /* N_DEFICIT  - 20 */
  { "forkstat" },      /* N_FORKSTAT - 21 */
  { "sum" },           /* N_SUM      - 22 */
  { "firstfree" },     /* N_FIRSTFRE - 23 */
  { "maxfree" },       /* N_MAXFREE  - 24 */
  { "dk_xfer" },       /* N_DKXFER   - 25 */
  { "rectime" },       /* N_RECTIME  - 26 */
  { "pgintime" },      /* N_PGINTIME - 27 */
  { "phz" },           /* N_PHZ      - 28 */
  { "intrnames" },     /* N_INTRNAM  - 29 */
  { "eintrnames" },    /* N_EINTERNAM- 30 */
  { "intrcnt" },       /* N_INTCRNT  - 31 */
  { "eintrcnt" },      /* N_EINTCRNT - 32 */
  { "dk_ndrive" },     /* N_DKNDRIVE - 33 */
  { "xstats" },        /* N_XSTATS   - 34 */
  { "inode" },         /* N_INODE    - 35 */
  { "text" },          /* N_XTEXT    - 36 */
  { "proc" },          /* N_PROC     - 37 */
  { "cons" },          /* N_CONS     - 38 */
  { "file" },          /* N_FILE     - 39 */
  { "Usrptmap "},      /* N_USRPTMAP - 40 */
  { "usrpt" },         /* N_USRPT    - 41 */
  { "swapmap" },       /* N_SWAPMAP  - 42 */
  { "nproc" },         /* N_NPROC    - 43 */
  { "ntext" },         /* N_NTEXT    - 44 */
  { "nfile" },         /* N_NFILE    - 45 */
  { "ninode" },        /* N_NINODE   - 46 */
  { "nswapmap" },      /* N_NSWAPMAP - 47 */
  { "pt_tty" },        /* N_PTTTY    - 48 */
  { "npty" },          /* N_NPTY     - 49 */
  { "dmmin" },         /* N_DMMIN    - 50 */
  { "dmmax" },         /* N_DMMAX    - 51 */
  { "nswdev" },        /* N_NSWDEV   - 52 */
  { "swdevt" },        /* N_SWDEVT   - 53 */    
  { "rec" },           /* N_REC      - 54 */
  { "pgin" },          /* N_PGIN     - 55 */   
  { "Sysmap" },        /* N_SYSMAP   - 56 */
#ifdef VFS  
  { "ncstats" },       /* N_NCSTATS  - 57 */
#else  VFS
  { "nchstats" },      /* N_NCHSTATS - 57 */
#endif VFS
#ifdef RPC
  { "rcstat" },        /* N_RCSTAT   - 58 */
  { "rsstat" },        /* N_RSSTAT   - 59 */
#endif RPC
#ifdef NFS
  { "clstat" },        /* N_CLSSTAT  - 60 */
  { "svstat" },        /* N_SVSSTAT  - 61 */
#endif NFS
#endif MIT
    "",
};

#endif /* RSPOS */

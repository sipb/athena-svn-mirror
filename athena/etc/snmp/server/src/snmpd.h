/*
 * $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/snmpd.h,v 2.1 1993-06-18 14:33:33 tom Exp $
 *
 * $Log: not supported by cvs2svn $
 * Revision 2.0  92/04/22  01:45:58  tom
 * release 7.4
 * 	added variables to allow customization of various config files
 * 
 * Revision 1.4  90/07/17  14:19:30  tom
 * commented out function declarations not functional on decmips
 * 
 * Revision 1.3  90/05/26  13:42:27  tom
 * athena release 7.0e + patch 15
 * 
 * Revision 1.2  90/04/26  18:02:01  tom
 * *** empty log message ***
 * 
 * Revision 1.1  90/04/26  16:35:06  tom
 * Initial revision
 * 
 * Revision 1.2  89/12/08  15:21:36  snmpdev
 * added chris tengi's patch for default snmm[trap] port numbers (in case
 * snmp and snmptrap don't occure in /etc/services -- kolb
 * 
 * Revision 1.1  89/11/03  15:43:29  snmpdev
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
 *  $Header: /afs/dev.mit.edu/source/repository/athena/etc/snmp/server/src/snmpd.h,v 2.1 1993-06-18 14:33:33 tom Exp $
 *
 *  June 28, 1988 - Mark S. Fedor
 *  Copyright (c) NYSERNet Incorporated, 1988
 */
/*
 *  this file contains all of the general/miscellaneous definitions
 *  for snmpd.
 */
/*
 *  general definitions
 */
#ifndef RSPOS
#define TRUE	 0x1
#define FALSE	 0x0
#endif /* RSPOS */

#define GEN_ERR	 	-1
#define GEN_SUCCESS	0x1

#define ITS_COOL	0x1		/* used in main.c */

/*
 *  This wil be used to hold a parsed packet.
 */
#define MAXBUFSIZE	3584		/* 3.5 kbytes */

/*
 * UDP port numbers to be used if services are undefined
 */
#define SNMPPORT	161
#define SNMPTRAPPORT	162

/*
 *  Community types that snmpd supports.
 */
#define READ_ONLY		0x1		/* read-only community */
#define READ_WRITE		0x2		/* can read and change vars */
#define TRAP_SESS		0x4		/* community generates traps */

/*
 *  Flag types for the set functions.  Do we set to a new value or
 *  do we go back to an old value.
 */
#define SETNEW			44
#define SETOLD			45

/*
 *  set structure.  We form a linked list of these things and perform
 *  each set as if simultaneously.
 */
struct set_struct {
	struct snmp_tree_node *tptr;		/* pointer to tree node */
	objident	      ob_inst;		/* Object instance */
	objval	      	      setv;		/* the set value */
	objval	      	      oldv;		/* The old value */
	struct set_struct     *next;		/* pointer to next in list */
	struct set_struct     *back;		/* pointer to prev in list */
};

/*
 *  This is the static interface list which contains values for
 *  interface variables that need static answers.
 */
struct intf_info {
	char	name[SNMPSTRLEN]; /* interface name */
	int	itype;		  /* interface type, ie. apple, FDDI, etc. */
	int	speed;		  /* interface speed, in mbits/sec */
	struct	intf_info *next;  /* next interface in list */
};

/*  This is just a linked list of inet addresses.  Used by session
 *  so we can have multiple addresses using the session.
 */
struct inaddrlst {			/* address list */
	struct in_addr sess_addr;	/* who can use session */
	struct inaddrlst *nxt;		/* next valid user */
};

/*
 *  This is the structure which defines an SNMP session.
 *  It is very simple now, but can be expanded as needed.
 *  The sessions will be stored as a linked list.
 */
struct snmp_session {
	char name[SNMPSTRLEN];		/* session name */
	struct inaddrlst *userlst;	/* who can use this session */
	int flags;			/* what kind of session? */
	struct snmp_session *next;	/* next session in list */
};

/*
 *  This is the snmpd stats structure.  We will try and keep
 *  interesting statistics here.
 */
struct snmp_stats {
	long	inpkts;			/* number of snmp pkts in */
	long	outpkts;		/* number os snmp pkts out */
	long	inhist[TRP + 1];	/* input histogram */
	long	outhist[TRP + 1];	/* output histogram */
	long	inerrs;			/* number of input errors */
	long	outerrs;		/* number of output errors */
	long 	parseerrs;		/* ASN.1 parse errors */
	long	badsession;		/* bad session use */
	long	procerr;		/* error in replying */
	long	toobig;			/* number of too big replies */
	long	badtype;		/* bad SNMP types received */
	long	totreqvars;		/* total number of requested vars */
	long	totsetvars;		/* total number of set-req's */
};

/*
 *  external variable definitions
 */
extern int	errno;     		/* so we can access proper error */
extern FILE 	*ftrace;		/* trace file descriptor */
extern char 	*logfile;		/* the log file */
extern int	snmp_socket;		/* snmp socket */
extern int	agent_socket;		/* agent socket */
extern long	snmptime;		/* time of day, seconds, of event */
extern char	*strtime;		/* time of day as an ASCII string */
extern struct   servent *sp;		/* service entry for SNMP */
extern struct   servent *trapport;	/* ptr to service entry for traps */
extern struct   servent snmptrap;	/* service entry for SNMP traps */
extern struct   sockaddr_in addr; 	/* addr to bind SNMP socket to */
extern struct   sockaddr_in local; 	/* my local address */
extern struct   snmp_tree_node *top;	/* root of SNMP variable tree */
extern struct   nlist nl[];		/* namelist info array */
extern int	kmem;			/* file descriptor fpr kmem */
extern int	debuglevel;		/* snmpd debug level */
extern struct	intf_info *iilst;	/* interface info list */
extern struct	snmp_session *sessions;	/* snmp session list */
#ifndef SOLARIS
#if defined(ultrix)
extern struct rtentry *rtnet[RTHASHSIZ];/* buffer for net routing tables */
#else
extern struct	mbuf *rtnet[RTHASHSIZ]; /* buffer for net routing tables */
#endif
#endif /* SOLARIS */
extern int	newpacket;		/* are we processing a new packet? */
extern struct	snmp_stats s_stat;	/* SNMPD stats kept here */
extern int	send_authen_traps;	/* do we send authen traps? */
extern objident sys_obj_id;		/* object id descr for this box */
extern struct   set_struct *setlst;	/* list of sets */
extern int	adminstat[MAXIFS];	/* intf admin status list */
extern int	tcprtoalg;		/* TCP RTO Algorithm variable */

extern char	gw_version_id[SNMPSTRLEN];	  /* ID of the gateway */
extern struct   snmp_tree_info  var_tree_info[];  /* Static tree info array */

#ifdef MIT
extern char     lbuf[BUFSIZ];                  /* random buffer */

extern char     supp_sysdescr[SNMPSTRLEN];     /* supplementary sysdescr */
extern char     mail_q[SNMPSTRLEN];            /* path for mailq directory */
extern char     mail_alias_file[SNMPSTRLEN];   /* path for mail aliases */
extern char     rc_file[SNMPSTRLEN];           /* path for rc config file */
extern char     rpc_cred_file[SNMPSTRLEN];     /* path for rpc cred file */
extern char     afs_cache_file[SNMPSTRLEN];    /* path for afs cache file */
extern char     afs_suid_file[SNMPSTRLEN];     /* path for afs setuid file */
extern char     afs_cell_file[SNMPSTRLEN];     /* path for afs cell file */
extern char     afs_cellserv_file[SNMPSTRLEN]; /* path for cellsrvdb */
extern char     login_file[SNMPSTRLEN];        /* path for utmp */
extern char     version_file[SNMPSTRLEN];      /* path for version file */
extern char     syspack_file[SNMPSTRLEN];      /* path for srvd info  file */
extern char     dns_stat_file[SNMPSTRLEN];     /* path for dns stat file */
extern char     srv_file[SNMPSTRLEN];          /* path for mksrv file */
extern char     user[SNMPSTRLEN];              /* default uid to run as */
extern char     weather_tty[SNMPSTRLEN];      /* tty for weather device */
extern char     weather_location[SNMPSTRLEN];  /* location string for wthr st*/
extern int      logintrap;                     /* whether to send login trap */

#ifdef RSPOS
extern char     *sys_errlist[];
#endif /* RSPOS */
#endif MIT


/*
 *  function type declarations
 */
/*
 *  system functions
 */
extern char *malloc();
extern char *inet_ntoa();
extern long lseek();
extern u_long inet_addr();
extern time_t time();
#ifndef RSPOS 
extern char *strcpy();
#endif /* RSPOS */

/*
 *  SNMP library functions.
 */
extern short snmpdecipher();
extern short snmpservsend();
extern int oidcmp();

/*
 *  routines specific to SNMPD
 */

#ifdef MIT
#ifdef decmips
extern void quit();
#else  decmips
extern int quit();
#endif decmips
#else  MIT
extern int quit();
#endif MIT

extern int init_kmem();
extern int get_my_address();
extern int varlist_free();
extern int add_sess();
extern int add_intf_var();
extern int parse_config();
extern int build_snmp_tree();
extern int connect_node();
extern int make_var_tree();
extern int snmp_init();
extern int agent_init();
extern int get_snmp_socket();
extern int proc_snmp_msg();
extern int procreq();
extern int procset();
extern int do_all_sets();
extern int send_snmp_trap();
extern int find_a_rt();
extern int find_ifnet_bynum();
extern int find_ifnet_byaddr();
extern int find_nnets();
extern time_t get_sysuptime();
extern int pvartree();
extern int ptreeinfo();
extern int pr_pkt();
extern int pr_var_list();
extern struct snmp_tree_node *init_var_tree_node();
extern struct snmp_tree_node *get_lex_next();

/*
 *  routines in SNMPD to extract info from the kernel
 */
extern int lu_vers();
extern int lu_nnets();
extern int lu_intf();
extern int lu_ipadd();
extern int lu_rtent();
extern int lu_tcpconnent();
extern int lu_atent();
extern int lu_tcpstat();
extern int lu_tcprtos();
extern int lu_udpstat();
extern int lu_ipstat();
extern int lu_icmp();
extern int lu_ipforw();
extern int get_agent_var();
extern int set_intf();

/* MIT Athena Stuff */

#ifdef MIT
extern int lu_kernvers();
extern int lu_status();
#ifndef RSPOS
extern int lu_vmstat();
extern int lu_pstat();
extern int lu_disk();
#endif /* RSPOS */
extern int lu_ndparts();
extern int lu_tuchtime();
extern int get_inuse();         /* for trap generation in main   */
extern int lu_mail();

#ifdef ATHENA
extern int lu_rcvar();
extern int lu_relvers();
extern int lu_spnum();
extern int lu_spvers();
extern int lu_snmpvers();
extern int lu_service();
extern int lu_servtbl();
extern int lu_machtype();
#endif ATHENA

#ifdef RPC
extern int lu_rpccl();
extern int lu_rpcsv();
#endif RPC
#ifdef NFS
extern int lu_nfscl();
extern int lu_nfssv();
#endif NFS
#ifdef RVD
extern int lu_rvdcl();
#endif RVD
#ifdef AFS
extern int lu_afs();
extern int lu_afsdb();
#endif AFS
#ifdef KERBEROS
extern int lu_kerberos();
#endif KERBEROS
#ifdef DNS
extern int lu_dns();
#endif DNS
#ifdef TIMED
extern int lu_timed();
#endif TIMED
extern int lu_weather();

extern char *stattime();
#endif MIT

/*
 * include/kerberosIV/krb.h
 *
 * Copyright 1987, 1988, 1994 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 * 
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 * Include file for the Kerberos V4 library. 
 */

/* Only one time, please */
#ifndef	KRB_DEFS
#define KRB_DEFS

/* Need some defs from des.h	 */
#include <kerberosIV/des.h>

/* Text describing error codes */
#define		MAX_KRB_ERRORS	256
extern const char *const krb_err_txt[MAX_KRB_ERRORS];

/* These are not defined for at least SunOS 3.3 and Ultrix 2.2 */
#if defined(ULTRIX022) || (defined(SunOS) && SunOS < 40)
#define FD_ZERO(p)  ((p)->fds_bits[0] = 0)
#define FD_SET(n, p)   ((p)->fds_bits[0] |= (1 << (n)))
#define FD_ISSET(n, p)   ((p)->fds_bits[0] & (1 << (n)))
#endif /* ULTRIX022 || SunOS */

/* General definitions */
#define		KSUCCESS	0
#define		KFAILURE	255

#ifndef __alpha
#define KRB4_32	long
#else
#define KRB4_32	int
#endif

#ifdef NO_UIDGID_T
typedef unsigned short uid_t;
typedef unsigned short gid_t;
#endif /* NO_UIDGID_T */

/*
 * Kerberos specific definitions 
 *
 * KRBLOG is the log file for the kerberos master server. KRB_CONF is
 * the configuration file where different host machines running master
 * and slave servers can be found. KRB_MASTER is the name of the
 * machine with the master database.  The admin_server runs on this
 * machine, and all changes to the db (as opposed to read-only
 * requests, which can go to slaves) must go to it. KRB_HOST is the
 * default machine * when looking for a kerberos slave server.  Other
 * possibilities are * in the KRB_CONF file. KRB_REALM is the name of
 * the realm. 
 */

#ifdef notdef
this is server - only, does not belong here;
#define 	KRBLOG 		"/kerberos/kerberos.log"
are these used anyplace '?';
#define		VX_KRB_HSTFILE	"/etc/krbhst"
#define		PC_KRB_HSTFILE	"\\kerberos\\krbhst"
#endif

#define		KRB_CONF	"/etc/krb.conf"
#define		KRB_RLM_TRANS	"/etc/krb.realms"
#define		KRB_MASTER	"kerberos"
#define		KRB_HOST	 KRB_MASTER
#define		KRB_REALM	"ATHENA.MIT.EDU"

/* The maximum sizes for aname, realm, sname, and instance +1 */
#define 	ANAME_SZ	40
#define		REALM_SZ	40
#define		SNAME_SZ	40
#define		INST_SZ		40
/* include space for '.' and '@' */
#define		MAX_K_NAME_SZ	(ANAME_SZ + INST_SZ + REALM_SZ + 2)
#define		KKEY_SZ		100
#define		VERSION_SZ	1
#define		MSG_TYPE_SZ	1
#define		DATE_SZ		26	/* RTI date output */

#define		MAX_HSTNM	100

#ifndef DEFAULT_TKT_LIFE		/* allow compile-time override */
#define		DEFAULT_TKT_LIFE	96 /* default lifetime for krb_mk_req
					      & co., 8 hrs */
#endif

/* Definition of text structure used to pass text around */
#define		MAX_KTXT_LEN	1250

struct ktext {
    int     length;		/* Length of the text */
    unsigned char dat[MAX_KTXT_LEN];	/* The data itself */
    unsigned long mbz;		/* zero to catch runaway strings */
};

typedef struct ktext *KTEXT;
typedef struct ktext KTEXT_ST;


/* Definitions for send_to_kdc */
#define	CLIENT_KRB_TIMEOUT	4	/* time between retries */
#define CLIENT_KRB_RETRY	5	/* retry this many times */
#define	CLIENT_KRB_BUFLEN	512	/* max unfragmented packet */

/* Definitions for ticket file utilities */
#define	R_TKT_FIL	0
#define	W_TKT_FIL	1

/* Definitions for cl_get_tgt */
#ifdef PC
#define CL_GTGT_INIT_FILE		"\\kerberos\\k_in_tkts"
#else
#define CL_GTGT_INIT_FILE		"/etc/k_in_tkts"
#endif /* PC */

/* Parameters for rd_ap_req */
/* Maximum alloable clock skew in seconds */
#define 	CLOCK_SKEW	5*60
/* Filename for readservkey */
#define		KEYFILE	     ((char*)krb__get_srvtabname("/etc/athena/srvtab"))

/* Structure definition for rd_ap_req */

struct auth_dat {
    unsigned char k_flags;	/* Flags from ticket */
    char    pname[ANAME_SZ];	/* Principal's name */
    char    pinst[INST_SZ];	/* His Instance */
    char    prealm[REALM_SZ];	/* His Realm */
    unsigned KRB4_32 checksum;	/* Data checksum (opt) */
    C_Block session;		/* Session Key */
    int     life;		/* Life of ticket */
    unsigned KRB4_32 time_sec;	/* Time ticket issued */
    unsigned KRB4_32 address;	/* Address in ticket */
    KTEXT_ST reply;		/* Auth reply (opt) */
};

typedef struct auth_dat AUTH_DAT;

/* Structure definition for credentials returned by get_cred */

struct credentials {
    char    service[ANAME_SZ];	/* Service name */
    char    instance[INST_SZ];	/* Instance */
    char    realm[REALM_SZ];	/* Auth domain */
    C_Block session;		/* Session key */
    int     lifetime;		/* Lifetime */
    int     kvno;		/* Key version number */
    KTEXT_ST ticket_st;		/* The ticket itself */
    KRB4_32 issue_date;		/* The issue time */
    char    pname[ANAME_SZ];	/* Principal's name */
    char    pinst[INST_SZ];	/* Principal's instance */
};

typedef struct credentials CREDENTIALS;

/* Structure definition for rd_private_msg and rd_safe_msg */

struct msg_dat {
    unsigned char *app_data;	/* pointer to appl data */
    unsigned KRB4_32 app_length;	/* length of appl data */
    unsigned KRB4_32 hash;	/* hash to lookup replay */
    int     swap;		/* swap bytes? */
    KRB4_32  time_sec;		/* msg timestamp seconds */
    unsigned char time_5ms;	/* msg timestamp 5ms units */
};

typedef struct msg_dat MSG_DAT;


/* Location of ticket file for save_cred and get_cred */
#ifdef PC
#define TKT_FILE        "\\kerberos\\ticket.ses"
#else
#define TKT_FILE        tkt_string()
#define TKT_ROOT        "/tmp/tkt"
#endif /* PC */

/* Error codes returned from the KDC */
#define		KDC_OK		0	/* Request OK */
#define		KDC_NAME_EXP	1	/* Principal expired */
#define		KDC_SERVICE_EXP	2	/* Service expired */
#define		KDC_AUTH_EXP	3	/* Auth expired */
#define		KDC_PKT_VER	4	/* Protocol version unknown */
#define		KDC_P_MKEY_VER	5	/* Wrong master key version */
#define		KDC_S_MKEY_VER 	6	/* Wrong master key version */
#define		KDC_BYTE_ORDER	7	/* Byte order unknown */
#define		KDC_PR_UNKNOWN	8	/* Principal unknown */
#define		KDC_PR_N_UNIQUE 9	/* Principal not unique */
#define		KDC_NULL_KEY   10	/* Principal has null key */
#define		KDC_GEN_ERR    20	/* Generic error from KDC */


/* Values returned by get_credentials */
#define		GC_OK		0	/* Retrieve OK */
#define		RET_OK		0	/* Retrieve OK */
#define		GC_TKFIL       21	/* Can't read ticket file */
#define		RET_TKFIL      21	/* Can't read ticket file */
#define		GC_NOTKT       22	/* Can't find ticket or TGT */
#define		RET_NOTKT      22	/* Can't find ticket or TGT */


/* Values returned by mk_ap_req	 */
#define		MK_AP_OK	0	/* Success */
#define		MK_AP_TGTEXP   26	/* TGT Expired */

/* Values returned by rd_ap_req */
#define		RD_AP_OK	0	/* Request authentic */
#define		RD_AP_UNDEC    31	/* Can't decode authenticator */
#define		RD_AP_EXP      32	/* Ticket expired */
#define		RD_AP_NYV      33	/* Ticket not yet valid */
#define		RD_AP_REPEAT   34	/* Repeated request */
#define		RD_AP_NOT_US   35	/* The ticket isn't for us */
#define		RD_AP_INCON    36	/* Request is inconsistent */
#define		RD_AP_TIME     37	/* delta_t too big */
#define		RD_AP_BADD     38	/* Incorrect net address */
#define		RD_AP_VERSION  39	/* protocol version mismatch */
#define		RD_AP_MSG_TYPE 40	/* invalid msg type */
#define		RD_AP_MODIFIED 41	/* message stream modified */
#define		RD_AP_ORDER    42	/* message out of order */
#define		RD_AP_UNAUTHOR 43	/* unauthorized request */

/* Values returned by get_pw_tkt */
#define		GT_PW_OK	0	/* Got password changing tkt */
#define		GT_PW_NULL     51	/* Current PW is null */
#define		GT_PW_BADPW    52	/* Incorrect current password */
#define		GT_PW_PROT     53	/* Protocol Error */
#define		GT_PW_KDCERR   54	/* Error returned by KDC */
#define		GT_PW_NULLTKT  55	/* Null tkt returned by KDC */


/* Values returned by send_to_kdc */
#define		SKDC_OK		0	/* Response received */
#define		SKDC_RETRY     56	/* Retry count exceeded */
#define		SKDC_CANT      57	/* Can't send request */

/*
 * Values returned by get_intkt
 * (can also return SKDC_* and KDC errors)
 */

#define		INTK_OK		0	/* Ticket obtained */
#define		INTK_W_NOTALL  61	/* Not ALL tickets returned */
#define		INTK_BADPW     62	/* Incorrect password */
#define		INTK_PROT      63	/* Protocol Error */
#define		INTK_ERR       70	/* Other error */

/* Values returned by get_adtkt */
#define         AD_OK           0	/* Ticket Obtained */
#define         AD_NOTGT       71	/* Don't have tgt */

/* Error codes returned by ticket file utilities */
#define		NO_TKT_FIL	76	/* No ticket file found */
#define		TKT_FIL_ACC	77	/* Couldn't access tkt file */
#define		TKT_FIL_LCK	78	/* Couldn't lock ticket file */
#define		TKT_FIL_FMT	79	/* Bad ticket file format */
#define		TKT_FIL_INI	80	/* tf_init not called first */

/* Error code returned by kparse_name */
#define		KNAME_FMT	81	/* Bad Kerberos name format */

/* Error code returned by krb_mk_safe */
#define		SAFE_PRIV_ERROR	-1	/* syscall error */

/*
 * macros for byte swapping; also scratch space
 * u_quad  0-->7, 1-->6, 2-->5, 3-->4, 4-->3, 5-->2, 6-->1, 7-->0
 * u_long  0-->3, 1-->2, 2-->1, 3-->0
 * u_short 0-->1, 1-->0
 */

#define     swap_u_16(x) {\
 unsigned KRB4_32   _krb_swap_tmp[4];\
 swab(((char *) x) +0, ((char *)  _krb_swap_tmp) +14 ,2); \
 swab(((char *) x) +2, ((char *)  _krb_swap_tmp) +12 ,2); \
 swab(((char *) x) +4, ((char *)  _krb_swap_tmp) +10 ,2); \
 swab(((char *) x) +6, ((char *)  _krb_swap_tmp) +8  ,2); \
 swab(((char *) x) +8, ((char *)  _krb_swap_tmp) +6 ,2); \
 swab(((char *) x) +10,((char *)  _krb_swap_tmp) +4 ,2); \
 swab(((char *) x) +12,((char *)  _krb_swap_tmp) +2 ,2); \
 swab(((char *) x) +14,((char *)  _krb_swap_tmp) +0 ,2); \
 memcpy((char *)x,(char *)_krb_swap_tmp,16);\
                            }

#define     swap_u_12(x) {\
 unsigned KRB4_32   _krb_swap_tmp[4];\
 swab(( char *) x,     ((char *)  _krb_swap_tmp) +10 ,2); \
 swab(((char *) x) +2, ((char *)  _krb_swap_tmp) +8 ,2); \
 swab(((char *) x) +4, ((char *)  _krb_swap_tmp) +6 ,2); \
 swab(((char *) x) +6, ((char *)  _krb_swap_tmp) +4 ,2); \
 swab(((char *) x) +8, ((char *)  _krb_swap_tmp) +2 ,2); \
 swab(((char *) x) +10,((char *)  _krb_swap_tmp) +0 ,2); \
 memcpy((char *)x,(char *)_krb_swap_tmp,12);\
                            }

#define     swap_C_Block(x) {\
 unsigned KRB4_32   _krb_swap_tmp[4];\
 swab(( char *) x,    ((char *)  _krb_swap_tmp) +6 ,2); \
 swab(((char *) x) +2,((char *)  _krb_swap_tmp) +4 ,2); \
 swab(((char *) x) +4,((char *)  _krb_swap_tmp) +2 ,2); \
 swab(((char *) x) +6,((char *)  _krb_swap_tmp)    ,2); \
 memcpy((char *)x,(char *)_krb_swap_tmp,8);\
                            }
#define     swap_u_quad(x) {\
 unsigned KRB4_32   _krb_swap_tmp[4];\
 swab(( char *) &x,    ((char *)  _krb_swap_tmp) +6 ,2); \
 swab(((char *) &x) +2,((char *)  _krb_swap_tmp) +4 ,2); \
 swab(((char *) &x) +4,((char *)  _krb_swap_tmp) +2 ,2); \
 swab(((char *) &x) +6,((char *)  _krb_swap_tmp)    ,2); \
 memcpy((char *)&x,(char *)_krb_swap_tmp,8);\
                            }

#define     swap_u_long(x) {\
 unsigned KRB4_32   _krb_swap_tmp[4];\
 swab((char *)  &x,    ((char *)  _krb_swap_tmp) +2 ,2); \
 swab(((char *) &x) +2,((char *)  _krb_swap_tmp),2); \
 x = _krb_swap_tmp[0];   \
                           }

#define     swap_u_short(x) {\
 unsigned short	_krb_swap_sh_tmp; \
 swab((char *)  &x,    ( &_krb_swap_sh_tmp) ,2); \
 x = (unsigned short) _krb_swap_sh_tmp; \
                            }

/* Kerberos ticket flag field bit definitions */
#define K_FLAG_ORDER    0       /* bit 0 --> lsb */
#define K_FLAG_1                /* reserved */
#define K_FLAG_2                /* reserved */
#define K_FLAG_3                /* reserved */
#define K_FLAG_4                /* reserved */
#define K_FLAG_5                /* reserved */
#define K_FLAG_6                /* reserved */
#define K_FLAG_7                /* reserved, bit 7 --> msb */

#ifndef PC
char *tkt_string();
#endif	/* PC */

#ifdef	OLDNAMES
#define krb_mk_req	mk_ap_req
#define krb_rd_req	rd_ap_req
#define krb_kntoln	an_to_ln
#define krb_set_key	set_serv_key
#define krb_get_cred	get_credentials
#define krb_mk_priv	mk_private_msg
#define krb_rd_priv	rd_private_msg
#define krb_mk_safe	mk_safe_msg
#define krb_rd_safe	rd_safe_msg
#define krb_mk_err	mk_appl_err_msg
#define krb_rd_err	rd_appl_err_msg
#define krb_ck_repl	check_replay
#define	krb_get_pw_in_tkt	get_in_tkt
#define krb_get_svc_in_tkt	get_svc_in_tkt
#define krb_get_pw_tkt		get_pw_tkt
#define krb_realmofhost		krb_getrealm
#define krb_get_phost		get_phost
#define krb_get_krbhst		get_krbhst
#define krb_get_lrealm		get_krbrlm
#endif	/* OLDNAMES */

/* Defines for krb_sendauth and krb_recvauth */

#define	KOPT_DONT_MK_REQ 0x00000001 /* don't call krb_mk_req */
#define	KOPT_DO_MUTUAL   0x00000002 /* do mutual auth */

#define	KOPT_DONT_CANON  0x00000004 /*
				     * don't canonicalize inst as
				     * a hostname
				     */

#define	KRB_SENDAUTH_VLEN 8	    /* length for version strings */

#ifdef ATHENA_COMPAT
#define	KOPT_DO_OLDSTYLE 0x00000008 /* use the old-style protocol */
#endif /* ATHENA_COMPAT */

/* until we do V4 compat under DOS, just turn this off */
#define INTERFACE
#define FAR
#define	_fmemcpy	memcpy
#define	_fstrncpy	strncpy
#define	far_fputs	fputs
/* and likewise, just drag in the unix time interface */
#define	TIME_GMT_UNIXSEC	unix_time_gmt_unixsec((unsigned KRB4_32 *)0)
#define	TIME_GMT_UNIXSEC_US(us)	unix_time_gmt_unixsec((us))
#define	CONVERT_TIME_EPOCH	((long)0)	/* Unix epoch is Krb epoch */

#if (defined(__STDC__) || defined(_WINDOWS)) && !defined(KRB5_NO_PROTOTYPES)
#define PROTOTYPE(x) x
#else
#define PROTOTYPE(x) ()
#endif /* STDC or PROTOTYPES */

/* Define u_char, u_short, u_int, and u_long. */
#include <sys/types.h>

/* If this source file requires it, define struct sockaddr_in
   (and possibly other things related to network I/O).  FIXME.  */
#ifdef DEFINE_SOCKADDR
#include <netinet/in.h>		/* For struct sockaddr_in and in_addr */
#include <arpa/inet.h>		/* For inet_ntoa */
#include <netdb.h>		/* For struct hostent, gethostbyname, etc */
#include <sys/param.h>		/* For MAXHOSTNAMELEN */
#include <sys/socket.h>		/* For SOCK_*, AF_*, etc */
#include <sys/time.h>		/* For struct timeval */
#ifdef NEED_TIME_H
#include <time.h>		/* For localtime, etc */
#endif
#endif
/*
 * Compatability with WinSock calls on MS-Windows...
 */
#define	SOCKET		unsigned int
#define	INVALID_SOCKET	((SOCKET)~0)
#define	closesocket	close
#define	ioctlsocket	ioctl
#define	SOCKET_ERROR	(-1)

/* Some of our own infrastructure where the WinSock stuff was too hairy
   to dump into a clean Unix program...  */

#define	SOCKET_INITIALIZE()	(0)	/* No error (or anything else) */
#define	SOCKET_CLEANUP()	/* nothing */
#define	SOCKET_ERRNO		errno
#define	SOCKET_SET_ERRNO(x)	(errno = (x))
#define SOCKET_NFDS(f)		((f)+1)	/* select() arg for a single fd */
#define SOCKET_READ		read
#define SOCKET_WRITE		write
#define SOCKET_EINTR		EINTR

/* ask to disable IP address checking in the library */
extern int krb_ignore_ip_address;

/* Debugging printfs shouldn't even be compiled on many systems that don't
   support printf!  Use it like  DEB (("Oops - %s\n", string));  */

#ifdef DEBUG
#define	DEB(x)	if (krb_debug) printf x
extern int krb_debug;
#else
#define	DEB(x)	/* nothing */
#endif


/*
 * Some Unixes don't declare errno in <errno.h>...
 * Move this out to individual c-*.h files if it becomes troublesome.
 */
#ifndef errno
extern int errno;
#endif

/* Define a couple of function types including parameters.  These
   are needed on MS-Windows to convert arguments of the function pointers
   to the proper types during calls.  */
typedef int (*key_proc_type) PROTOTYPE ((char *, char *, char *,
					     char *, C_Block));
#define KEY_PROC_TYPE_DEFINED
typedef int (*decrypt_tkt_type) PROTOTYPE ((char *, char *, char *, char *,
				     key_proc_type, KTEXT *));
#define DECRYPT_TKT_TYPE_DEFINED

#ifndef _KRB4_PROTO_H__
#define _KRB4_PROTO_H__

/*
 * Function Prototypes for Kerberos V4.
 */

#include <stdio.h>
struct sockaddr_in;

/* add_ticket.c */
int add_ticket PROTOTYPE((KTEXT , int , char *, int , char *, char *, char *, int , KTEXT ));

/* cr_err_reply.c */
void cr_err_reply PROTOTYPE((KTEXT , char *, char *, char *, u_long , u_long , char *));

/* create_auth_reply.c */
KTEXT create_auth_reply PROTOTYPE((char *, char *, char *, long , int , unsigned long , int , KTEXT ));

/* create_ciph.c */
int create_ciph PROTOTYPE((KTEXT , C_Block , char *, char *, char *, unsigned long , int , KTEXT , unsigned long , C_Block ));

/* create_death_packet.c */
KTEXT krb_create_death_packet PROTOTYPE((char *));

/* create_ticket.c */
int krb_create_ticket PROTOTYPE((KTEXT , unsigned int , char *, char *, char *, long , char *, int , long , char *, char *, C_Block ));

/* debug_decl.c */

/* decomp_ticket.c */
int decomp_ticket PROTOTYPE((KTEXT , unsigned char *, char *, char *, char *, unsigned KRB4_32 *, C_Block , int *, unsigned KRB4_32 *, char *, char *, C_Block , Key_schedule ));

/* dest_tkt.c */
int dest_tkt PROTOTYPE((void ));

/* extract_ticket.c */
int extract_ticket PROTOTYPE((KTEXT , int , char *, int *, int *, char *, KTEXT ));

/* fgetst.c */
int fgetst PROTOTYPE((FILE *, char *, int ));

/* get_ad_tkt.c */
int get_ad_tkt PROTOTYPE((char *, char *, char *, int ));

/* get_admhst.c */
int krb_get_admhst PROTOTYPE((char *, char *, int ));

/* get_cred.c */
int krb_get_cred PROTOTYPE((char *, char *, char *, CREDENTIALS *));

/* get_in_tkt.c */
int krb_get_pw_in_tkt PROTOTYPE((char *, char *, char *, char *, char *, int , char *));
int placebo_read_password PROTOTYPE((des_cblock *, char *, int ));
int placebo_read_pw_string PROTOTYPE((char *, int , char *, int ));

/* get_krbhst.c */
int krb_get_krbhst PROTOTYPE((char *, char *, int ));

/* get_krbrlm.c */
int krb_get_lrealm PROTOTYPE((char *, int ));

/* get_phost.c */
char *krb_get_phost PROTOTYPE((char *));

/* get_pw_tkt.c */
int get_pw_tkt PROTOTYPE((char *, char *, char *, char *));

/* get_request.c */
int get_request PROTOTYPE((KTEXT , int , char **, char **));

/* get_svc_in_tkt.c */
int krb_get_svc_in_tkt PROTOTYPE((char *, char *, char *, char *, char *, int , char *));

/* get_tf_fullname.c */
int krb_get_tf_fullname PROTOTYPE((char *, char *, char *, char *));

/* get_tf_realm.c */
int krb_get_tf_realm PROTOTYPE((char *, char *));

/* getrealm.c */
char *krb_realmofhost PROTOTYPE((char *));

/* getst.c */
int getst PROTOTYPE((int , char *, int ));

/* in_tkt.c */
int in_tkt PROTOTYPE((char *, char *));

/* k_gethostname.c */
int k_gethostname PROTOTYPE((char *, int ));

/* klog.c */
char *klog PROTOTYPE((int , char *, char * , char * , char * , char * , char * , char * , char * , char * , char * , char * ));
int kset_logfile PROTOTYPE((char *));

/* kname_parse.c */
int kname_parse PROTOTYPE((char *, char *, char *, char *));
int k_isname PROTOTYPE((char *));
int k_isinst PROTOTYPE((char *));
int k_isrealm PROTOTYPE((char *));

/* kntoln.c */
int krb_kntoln PROTOTYPE((AUTH_DAT *, char *));

/* krb_err_txt.c */

/* krb_get_in_tkt.c */
int krb_get_in_tkt PROTOTYPE((char *, char *, char *, char *, char *, int , int (*key_proc )(), int (*decrypt_proc )(), char *));

/* kuserok.c */
int kuserok PROTOTYPE((AUTH_DAT *, char *));

/* mk_err.c */
long krb_mk_err PROTOTYPE((u_char *, long , char *));

/* mk_priv.c */
long krb_mk_priv PROTOTYPE((u_char *, u_char *, u_long , Key_schedule , C_Block , struct sockaddr_in *, struct sockaddr_in *));

/* mk_req.c */
int krb_mk_req PROTOTYPE((KTEXT , char *, char *, char *, long ));
int krb_set_lifetime PROTOTYPE((int ));

/* mk_safe.c */
long krb_mk_safe PROTOTYPE((u_char *, u_char *, u_long , C_Block *, struct sockaddr_in *, struct sockaddr_in *));

/* month_sname.c */
char *month_sname PROTOTYPE((int ));

/* netread.c */
int krb_net_read PROTOTYPE((int , char *, int ));

/* netwrite.c */
int krb_net_write PROTOTYPE((int , char *, int ));

/* one.c */

/* pkt_cipher.c */
KTEXT pkt_cipher PROTOTYPE((KTEXT ));

/* pkt_clen.c */
int pkt_clen PROTOTYPE((KTEXT ));

/* rd_err.c */
int krb_rd_err PROTOTYPE((u_char *, u_long , long *, MSG_DAT *));

/* rd_priv.c */
long krb_rd_priv PROTOTYPE((u_char *, u_long , Key_schedule , C_Block *, struct sockaddr_in *, struct sockaddr_in *, MSG_DAT *));

/* rd_req.c */
int krb_set_key PROTOTYPE((char *, int ));
int krb_rd_req PROTOTYPE((KTEXT , char *, char *, unsigned KRB4_32 , AUTH_DAT *, char *));

/* rd_safe.c */
long krb_rd_safe PROTOTYPE((u_char *, u_long , C_Block *, struct sockaddr_in *, struct sockaddr_in *, MSG_DAT *));

/* read_service_key.c */
int read_service_key PROTOTYPE((char *, char *, char *, int , char *, char *));

/* recvauth.c */
int krb_recvauth PROTOTYPE((long , int , KTEXT , char *, char *, struct sockaddr_in *, struct sockaddr_in *, AUTH_DAT *, char *, Key_schedule , char *));

/* save_credentials.c */
int krb_save_credentials PROTOTYPE((char *, char *, char *, C_Block , int , int , KTEXT , long ));

/* send_to_kdc.c */
int send_to_kdc PROTOTYPE((KTEXT , KTEXT , char *));

/* sendauth.c */
int krb_sendauth PROTOTYPE((long , int , KTEXT , char *, char *, char *, u_long , MSG_DAT *, CREDENTIALS *, Key_schedule , struct sockaddr_in *, struct sockaddr_in *, char *));
int krb_sendsvc PROTOTYPE((int , char *));

/* stime.c */
char *krb_stime PROTOTYPE((long *));

/* tf_shm.c */
int krb_shm_create PROTOTYPE((char *));
int krb_is_diskless PROTOTYPE((void ));
int krb_shm_dest PROTOTYPE((char *));

/* tf_util.c */
int tf_init PROTOTYPE((char *, int ));
int tf_get_pname PROTOTYPE((char *));
int tf_get_pinst PROTOTYPE((char *));
int tf_get_cred PROTOTYPE((CREDENTIALS *));
int tf_close PROTOTYPE((void ));
int tf_save_cred PROTOTYPE((char *, char *, char *, C_Block , int , int , KTEXT , long ));

/* tkt_string.c */
char *tkt_string PROTOTYPE((void ));
void krb_set_tkt_string PROTOTYPE((char *));

/* util.c */
void ad_print PROTOTYPE((AUTH_DAT *));
int placebo_cblock_print PROTOTYPE((des_cblock ));

#endif /*  _KRB4_PROTO_H__ */
#endif	/* KRB_DEFS */

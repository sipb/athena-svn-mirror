/*
 * kerberos.h
 *
 * Copyright 1987, 1988 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 * Include file with external interfaces for the Kerberos library. 
 */

/* Only one time, please */
#ifndef	KRBIF_DEFS
#define KRBIF_DEFS

#include "mit-copyright.h"		/* Needed??? FIXME!  */

/* Need some defs from des.h	 */
#include "des.h"			/* FIXME, pull in just what needed */

/* General definitions */
#define		KSUCCESS	0
#define		KFAILURE	255

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

#ifndef DEFAULT_TKT_LIFE		/* allow compile-time override */
#define		DEFAULT_TKT_LIFE	120 /* default lifetime 10 hrs */
#endif

/* Definition of text structure used to pass text around */
#define		MAX_KTXT_LEN	1250

struct ktext {
    int     length;		/* Length of the text */
    unsigned char dat[MAX_KTXT_LEN];	/* The data itself */
    unsigned long mbz;		/* zero to catch runaway strings */
};

typedef struct ktext FAR *KTEXT;
typedef struct ktext KTEXT_ST;


/* Definitions for send_to_kdc */
#define	CLIENT_KRB_TIMEOUT	4	/* time between retries */
#define CLIENT_KRB_RETRY	5	/* retry this many times */
#define	CLIENT_KRB_BUFLEN	512	/* max unfragmented packet */

/* Definitions for ticket file utilities */
#define	R_TKT_FIL	0
#define	W_TKT_FIL	1

/* Structure definition for rd_ap_req */

struct auth_dat {
    unsigned char k_flags;	/* Flags from ticket */
    char    pname[ANAME_SZ];	/* Principal's name */
    char    pinst[INST_SZ];	/* His Instance */
    char    prealm[REALM_SZ];	/* His Realm */
    unsigned KRB_INT32 checksum; /* Data checksum (opt) */
    C_Block session;		/* Session Key */
    int     life;		/* Life of ticket */
    unsigned KRB_INT32 time_sec; /* Time ticket issued */
    unsigned KRB_INT32 address;	/* Address in ticket */
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
    long    issue_date;		/* The issue time */
    char    pname[ANAME_SZ];	/* Principal's name */
    char    pinst[INST_SZ];	/* Principal's instance */
};

typedef struct credentials CREDENTIALS;

/* Structure definition for rd_private_msg and rd_safe_msg */

struct msg_dat {
    unsigned char *app_data;	/* pointer to appl data */
    unsigned KRB_INT32 app_length; /* length of appl data */
    unsigned long hash;	/* hash to lookup replay */
    int     swap;		/* swap bytes? */
    KRB_INT32    time_sec;	/* msg timestamp seconds */
    unsigned char time_5ms;	/* msg timestamp 5ms units */
};

typedef struct msg_dat MSG_DAT;


/* Location of default ticket file for save_cred and get_cred */
#ifndef	TKT_FILE
#define TKT_FILE        ((char *)0)
#endif /* TKT_FILE */

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
#define		INTK_PW_NULL   51	/* Current PW is null */
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
 * Function declarations
 *
 * In alphabetical order.
 */

#ifndef PC
/* FIXME:  Some equivalent to these (e.g. krb_tkt_cache_name) should
   appear in all platforms.  For now, as is, FIXME... */
extern void krb_set_tkt_string PROTOTYPE ((char *));
extern char *tkt_string PROTOTYPE ((void));
#endif	/* PC */

#ifdef __STDC__
/* Forward declarations of structs used in prototype arguments */
struct sockaddr_in;
#endif

extern int INTERFACE
des_cbc_encrypt PROTOTYPE ((des_cblock FAR *, des_cblock FAR *, long,
		des_key_schedule FAR, des_cblock FAR *, int));

extern int INTERFACE
des_key_sched PROTOTYPE ((des_cblock FAR, des_key_schedule FAR));

extern int INTERFACE
des_string_to_key PROTOTYPE ((char FAR *, des_cblock FAR));

extern int INTERFACE
in_tkt PROTOTYPE ((char FAR *, char FAR *));

extern int INTERFACE
dest_tkt PROTOTYPE ((void));

extern int INTERFACE
kname_parse PROTOTYPE ((char FAR *, char FAR *, char FAR *, char FAR *));

extern int INTERFACE
krb_check_auth PROTOTYPE ((KTEXT, unsigned KRB_INT32, MSG_DAT FAR *,
		C_Block FAR, Key_schedule FAR, struct sockaddr_in FAR *,
		struct sockaddr_in FAR *));

extern int INTERFACE
krb_delete_cred PROTOTYPE ((char FAR *sname, char FAR *sinstance,
			    char FAR *srealm));

extern int INTERFACE
krb_end_session PROTOTYPE ((char FAR *));

extern int INTERFACE
krb_get_admhst PROTOTYPE ((char FAR *, char FAR *, int));

extern int INTERFACE
krb_get_cred PROTOTYPE ((char FAR *, char FAR *, char FAR *,
		CREDENTIALS FAR *));

extern char FAR * INTERFACE
krb_get_default_user PROTOTYPE ((void));

extern const char FAR * INTERFACE
krb_get_err_text PROTOTYPE ((int));

/* Needed in interface? FAR */
	extern int INTERFACE
	krb_get_krbhst PROTOTYPE ((char FAR *, char FAR *, int));

extern int INTERFACE
krb_get_lrealm PROTOTYPE ((char FAR *, int));

extern int INTERFACE
krb_get_nth_cred PROTOTYPE ((char FAR *sname, char FAR *sinstance,
			     char FAR *srealm, int n));

extern int INTERFACE
krb_get_num_cred PROTOTYPE ((void));

extern char FAR * INTERFACE
krb_get_phost PROTOTYPE ((char FAR *));


extern int INTERFACE
krb_get_pw_in_tkt PROTOTYPE ((char FAR *, char FAR *, char FAR *,
		char FAR *, char FAR *, int, char FAR *));

extern int INTERFACE
krb_get_pw_in_tkt_preauth PROTOTYPE ((char FAR *, char FAR *, char FAR *,
		char FAR *, char FAR *, int, char FAR *));

/* Needed in interface?  For password changing... */
	extern int INTERFACE
	krb_get_pw_tkt PROTOTYPE ((char FAR *, char FAR *, char FAR *,
		char FAR *));

	extern int INTERFACE
	krb_get_svc_in_tkt PROTOTYPE ((char FAR *, char FAR *, char FAR *,
		char FAR *, char FAR *, int, char FAR *));

	extern int INTERFACE
	krb_get_tf_fullname PROTOTYPE ((char FAR *, char FAR *, char FAR *,
		char FAR *));

extern int INTERFACE
krb_get_ticket_for_service PROTOTYPE ((char FAR *, char FAR *,
				       unsigned KRB_INT32 FAR *, int, 
				       des_cblock FAR, Key_schedule FAR,
				       char FAR *, int));
 
/* Needed?  For local naming */
	extern int INTERFACE
	krb_kntoln PROTOTYPE ((AUTH_DAT FAR *, char FAR *));

extern int INTERFACE
krb_mk_auth PROTOTYPE ((long, KTEXT, char FAR *, char FAR *, char FAR *, 
		unsigned KRB_INT32, char FAR *, KTEXT));

	extern long INTERFACE
	krb_mk_err PROTOTYPE ((unsigned char FAR *, KRB_INT32, char FAR *));

extern long INTERFACE
krb_mk_priv PROTOTYPE ((unsigned char FAR *, unsigned char FAR *,
			unsigned KRB_INT32, Key_schedule FAR,
			C_Block FAR *, struct sockaddr_in FAR *,
			struct sockaddr_in FAR *));

extern int INTERFACE
krb_mk_req PROTOTYPE ((KTEXT, char FAR *, char FAR *, char FAR *, KRB_INT32));

	extern long INTERFACE
	krb_mk_safe PROTOTYPE ((unsigned char FAR *, unsigned char FAR *,
				unsigned KRB_INT32, C_Block FAR *,
				struct sockaddr_in FAR *,
				struct sockaddr_in FAR *));

	extern int INTERFACE
	krb_rd_err PROTOTYPE ((unsigned char FAR *, unsigned long,
				long FAR *, MSG_DAT FAR *));

extern long INTERFACE
krb_rd_priv PROTOTYPE ((unsigned char FAR *, unsigned KRB_INT32,
			Key_schedule FAR, C_Block FAR *, struct sockaddr_in FAR *,
			struct sockaddr_in FAR *, MSG_DAT FAR *));

	extern int INTERFACE
	krb_rd_req PROTOTYPE ((KTEXT, char FAR *, char FAR *,
			       unsigned KRB_INT32, AUTH_DAT FAR *,
			       char FAR *));

	extern long INTERFACE
	krb_rd_safe PROTOTYPE ((unsigned char FAR *, unsigned KRB_INT32,
		C_Block FAR *, struct sockaddr_in FAR *,
		struct sockaddr_in FAR *, MSG_DAT FAR *));

extern char FAR * INTERFACE
krb_realmofhost PROTOTYPE ((char FAR *));

	/* Needs to be broke up into something without a file descriptor */
	extern int INTERFACE
	krb_recvauth PROTOTYPE ((long, int, KTEXT, char FAR *, char FAR *,
		struct sockaddr_in FAR *, struct sockaddr_in FAR *,
		AUTH_DAT FAR *, char FAR *, Key_schedule FAR, char FAR *));

extern int INTERFACE
krb_save_credentials PROTOTYPE ((char FAR *, char FAR *, char FAR *, 
		C_Block FAR, int, int, KTEXT, long));

extern int INTERFACE
krb_set_default_user PROTOTYPE ((char FAR *));

extern int INTERFACE
krb_start_session PROTOTYPE ((char FAR *));

/* FIXME jcm - this def here for testing, needs unixlike wrapper. */

/* kadm library interfaces */
extern const char FAR * INTERFACE
kadm_get_err_text PROTOTYPE ((int));

extern int INTERFACE
kadm_change_pw PROTOTYPE ((des_cblock FAR));

extern int INTERFACE
kadm_change_pw2 PROTOTYPE ((des_cblock FAR, char FAR *, u_char FAR* FAR*));

extern int INTERFACE
kadm_init_link PROTOTYPE ((char FAR *, char FAR *, char FAR *));

/* Get Windows unique broadcast message number */
#ifdef _WINDOWS
extern unsigned int INTERFACE
krb_get_notification_message PROTOTYPE ((void));
#endif

/* Defines for krb_sendauth, krb_mk_auth, krb_check_auth, and krb_recvauth */

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

#endif	/* KRBIF_DEFS */

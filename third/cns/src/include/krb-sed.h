/*
 * krb-sed.h
 *
 * Copyright 1987, 1988 by the Massachusetts Institute of Technology. 
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>. 
 *
 * Internal defintions for the Kerberos library.  External definitions
 * (visible to Kerberos library callers) should be in kerberos.h now.
 */

/* Only one time, please */
#ifndef	KRB_DEFS
#define KRB_DEFS

#include "mit-copyright.h"

/* Include the external definitions */
#include "kerberos.h"

#define MAX_KRB_ERRORS 256

/* Text describing error codes */
#ifdef MULTIDIMENSIONAL_ERR_TXT
 extern const char krb_err_txt[MAX_KRB_ERRORS][60];
#else 
 extern const char *const krb_err_txt [MAX_KRB_ERRORS];
#endif

/* Debugging printfs shouldn't even be compiled on many systems that don't
   support printf!  Use it like  DEB (("Oops - %s\n", string));  */

#ifdef DEBUG
#define	DEB(x)	if (krb_debug) printf x
extern int krb_debug;
#else
#define	DEB(x)	/* nothing */
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
 * default machine when looking for a kerberos slave server.  Other
 * possibilities are in the KRB_CONF file. KRB_REALM is the name of
 * the realm. 
 */

#ifdef VMS
#define		KRB_CONF	"multinet:kerberos.configuration"
#define		KRB_RLM_TRANS	"multinet:kerberos.realm"
#else
#define		KRB_CONF	"CONFDIR/krb.conf"
#define		KRB_RLM_TRANS	"CONFDIR/krb.realms"
#endif
#define		KRB_MASTER	"kerberos"
#define		KRB_HOST	 KRB_MASTER
#define		KRB_REALM	"SITE_KRB_REALM"

#define		KKEY_SZ		100
#define		VERSION_SZ	1
#define		MSG_TYPE_SZ	1
#define		DATE_SZ		26	/* RTI date output */

#define		MAX_HSTNM	100

#ifndef DEFAULT_TKT_LIFE		/* allow compile-time override */
#define		DEFAULT_TKT_LIFE	120 /* default lifetime 10 hrs */
#endif

/* Definition of text structure used to pass text around */
#define		MAX_KTXT_LEN	1250

/* Definitions for send_to_kdc */
#define	CLIENT_KRB_TIMEOUT	4	/* time between retries */
#define CLIENT_KRB_RETRY	5	/* retry this many times */
#define	CLIENT_KRB_BUFLEN	512	/* max unfragmented packet */

/* Definitions for ticket file utilities */
#define	R_TKT_FIL	0
#define	W_TKT_FIL	1

/* Parameters for rd_ap_req */
/* Maximum alloable clock skew in seconds */
#define 	CLOCK_SKEW	5*60
/* Filename for readservkey */
#ifdef VMS
#define         KEYFILE         "multinet:kerberos.srvtab"
#else
#define		KEYFILE		"SRVTAB_FILE"
#endif

/* Prefix string of default Unix ticket file for save_cred and get_cred */
#define TKT_ROOT        "/tmp/tkt"

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
 * Values returned by get_in_tkt
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
 * macros for byte swapping; also scratch space
 * u_quad  0-->7, 1-->6, 2-->5, 3-->4, 4-->3, 5-->2, 6-->1, 7-->0
 * u_long  0-->3, 1-->2, 2-->1, 3-->0
 * u_short 0-->1, 1-->0
 */

#define     swap_u_16(x) {\
 unsigned KRB_INT32   _krb_swap_tmp[4];\
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
 unsigned KRB_INT32   _krb_swap_tmp[4];\
 swab(( char *) x,     ((char *)  _krb_swap_tmp) +10 ,2); \
 swab(((char *) x) +2, ((char *)  _krb_swap_tmp) +8 ,2); \
 swab(((char *) x) +4, ((char *)  _krb_swap_tmp) +6 ,2); \
 swab(((char *) x) +6, ((char *)  _krb_swap_tmp) +4 ,2); \
 swab(((char *) x) +8, ((char *)  _krb_swap_tmp) +2 ,2); \
 swab(((char *) x) +10,((char *)  _krb_swap_tmp) +0 ,2); \
 memcpy((char *)x,(char *)_krb_swap_tmp,12);\
                            }

#define     swap_C_Block(x) {\
 unsigned KRB_INT32   _krb_swap_tmp[4];\
 swab(( char *) x,    ((char *)  _krb_swap_tmp) +6 ,2); \
 swab(((char *) x) +2,((char *)  _krb_swap_tmp) +4 ,2); \
 swab(((char *) x) +4,((char *)  _krb_swap_tmp) +2 ,2); \
 swab(((char *) x) +6,((char *)  _krb_swap_tmp)    ,2); \
 memcpy((char *)x,(char *)_krb_swap_tmp,8);\
                            }
#define     swap_u_quad(x) {\
 unsigned KRB_INT32   _krb_swap_tmp[4];\
 swab(( char *) &x,    ((char *)  _krb_swap_tmp) +6 ,2); \
 swab(((char *) &x) +2,((char *)  _krb_swap_tmp) +4 ,2); \
 swab(((char *) &x) +4,((char *)  _krb_swap_tmp) +2 ,2); \
 swab(((char *) &x) +6,((char *)  _krb_swap_tmp)    ,2); \
 memcpy((char *)&x,(char *)_krb_swap_tmp,8);\
                            }

#define     swap_u_long(x) {\
 unsigned KRB_INT32   _krb_swap_tmp[4];\
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

/* Function declarations */

#ifndef PC
/* FIXME:  Some equivalent to these (e.g. krb_tkt_cache_name) should
   appear in all platforms.  For now, as is, FIXME... */
extern void krb_set_tkt_string PROTOTYPE ((char *));
extern char *tkt_string PROTOTYPE ((void));
#endif	/* PC */

extern int krb_set_key PROTOTYPE ((char *, int));

/* Define a couple of function types including parameters.  These
   are needed on MS-Windows to convert arguments of the function pointers
   to the proper types during calls.  */
typedef int (*key_proc_type) PROTOTYPE ((char *, char *, char *,
					     char *, C_Block));
typedef int (*decrypt_tkt_type) PROTOTYPE ((char *, char *, char *, char *,
				     key_proc_type, KTEXT *));
#define	KEY_PROC_TYPE_DEFINED
#define	DECRYPT_TKT_TYPE_DEFINED

extern int
krb_get_in_tkt PROTOTYPE ((char *, char *, char *, char *, char *, int,
			   key_proc_type, decrypt_tkt_type, char *arg));

extern int
krb_get_in_tkt_preauth PROTOTYPE ((char *, char *, char *, char *, char *, int,
		key_proc_type, decrypt_tkt_type, char *arg, char *, int));

extern int
krb_mk_preauth PROTOTYPE ((char **, int *, key_proc_type, char *, char *,
			   char *, char *, C_Block));

extern int
passwd_to_key PROTOTYPE ((char *, char *, char *, char *, C_Block));

#ifdef DEBUG
extern KTEXT krb_create_death_packet PROTOTYPE ((char *));
#endif /* DEBUG */

/* ask to disable IP address checking in the library */
extern int krb_ignore_ip_address;

#endif	/* KRB_DEFS */

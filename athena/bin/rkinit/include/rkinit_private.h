/* 
 * $Id: rkinit_private.h,v 1.5 1994-03-30 15:52:19 cfields Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/include/rkinit_private.h,v $
 * $Author: cfields $
 *
 * Header file for rkinit library and server internal use
 */

#ifndef __RKINIT_PRIVATE_H__
#define __RKINIT_PRIVATE_H__

#if !defined(lint) && !defined(SABER)
static char *rcsid_rkinit_private_h = "$Id: rkinit_private.h,v 1.5 1994-03-30 15:52:19 cfields Exp $";
#endif /* lint || SABER */

#include <sys/types.h>
#include <netinet/in.h>

#ifdef __STDC__
#define RK_PROTO(x) x
#else
#define RK_PROTO(x) ()
#endif /* __STDC__ */

/* Lowest and highest versions supported */
#define RKINIT_LVERSION 3
#define RKINIT_HVERSION 3

/* Service to be used; port number to fall back on if service isn't found */
#define SERVENT "rkinit"
#define PORT 2108

/* Key for kerberos authentication */
#define KEY "rcmd"

/* Packet format information */
#define PKT_TYPE 0
#define PKT_LEN 1
#define PKT_DATA (PKT_LEN + sizeof(long))

/* 
 * Message types for packets.  Make sure that rki_mt_to_string is right in 
 * rk_util.c
 */
#define MT_STATUS 0
#define MT_CVERSION 1
#define MT_SVERSION 2
#define MT_RKINIT_INFO 3
#define MT_SKDC 4
#define MT_CKDC 5
#define MT_AUTH 6
#define MT_DROP 7

/* Miscellaneous protocol constants */
#define VERSION_INFO_SIZE 2

/* Useful definitions */
#define BCLEAR(a) memset((char *)(a), 0, sizeof(a))
#define SBCLEAR(a) memset((char *)&(a), 0, sizeof(a))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/* Function declarations */
int rki_key_proc RK_PROTO((char *, char *, char *, char *, des_cblock));
int rki_get_tickets RK_PROTO((int, char *, char *, rkinit_info *));
int rki_send_packet RK_PROTO((int, char, u_long, char *));
int rki_get_packet RK_PROTO((int, char, u_long *, char *));
int rki_setup_rpc RK_PROTO((char *));
int rki_rpc_exchange_version_info RK_PROTO((int, int, int *, int *));
int rki_rpc_send_rkinit_info RK_PROTO((rkinit_info *));
int rki_rpc_get_status RK_PROTO((void));
int rki_rpc_get_ktext RK_PROTO((int, KTEXT, u_char));
int rki_rpc_sendauth RK_PROTO((KTEXT));
int rki_rpc_get_skdc RK_PROTO((KTEXT));
int rki_rpc_send_ckdc RK_PROTO((MSG_DAT *));
int rki_get_csaddr RK_PROTO((struct sockaddr_in *, struct sockaddr_in *));
void rki_drop_server RK_PROTO((void));
void rki_cleanup_rpc RK_PROTO((void));
void rki_dmsg RK_PROTO((char *));
char *rki_mt_to_string RK_PROTO((int));
int rki_choose_version RK_PROTO((int *));
int rki_send_rkinit_info RK_PROTO((int, rkinit_info *));


#endif /* __RKINIT_PRIVATE_H__ */

/* $Id: rkinit.h,v 1.2 1999-12-09 22:23:55 danw Exp $ */

/* Copyright 1997, 1998 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* Main header file for rkinit library users */

#ifndef __RKINIT_H__
#define __RKINIT_H__

#include <krb.h>
#include <sys/param.h>
#include <sys/types.h>
#include <netinet/in.h>

typedef struct {
    char aname[ANAME_SZ + 1];
    char inst[INST_SZ + 1];
    char realm[REALM_SZ + 1];
    char sname[ANAME_SZ + 1];
    char sinst[INST_SZ + 1];
    char username[9];		/* max local name length + 1 */
    char tktfilename[MAXPATHLEN + 1];
    long lifetime;
} rkinit_info;

#define RKINIT_SUCCESS 0

/* Lowest and highest versions supported */
#define RKINIT_LVERSION 3
#define RKINIT_HVERSION 4

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
int rkinit(char *, char *, rkinit_info *, int);
char *rkinit_errmsg(char *);
int rki_get_tickets(int, char *, char *, rkinit_info *);
int rki_send_packet(int, char, u_long, char *);
int rki_get_packet(int, char, u_long *, char *);
int rki_setup_rpc(char *);
int rki_rpc_exchange_version_info(int, int, int *, int *);
int rki_rpc_send_rkinit_info(rkinit_info *);
int rki_rpc_get_status(void);
int rki_rpc_get_ktext(int, KTEXT, u_char);
int rki_rpc_sendauth(KTEXT);
int rki_rpc_get_skdc(KTEXT);
int rki_rpc_send_ckdc(MSG_DAT *);
int rki_get_csaddr(struct sockaddr_in *, struct sockaddr_in *);
void rki_drop_server(void);
void rki_cleanup_rpc(void);
char *rki_mt_to_string(int);
int rki_choose_version(int *);
int rki_send_rkinit_info(int, rkinit_info *);

#endif /* __RKINIT_H__ */

/* 
 * $Header: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/include/rkinit_private.h,v 1.2 1989-11-13 20:04:53 qjb Exp $
 * $Source: /afs/dev.mit.edu/source/repository/athena/bin/rkinit/include/rkinit_private.h,v $
 * $Author: qjb $
 *
 */

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

/* Miscellaneous protocol constants */
#define VERSION_INFO_SIZE 2

/* Useful definitions */
#define BCLEAR(a) bzero((char *)(a), sizeof(a))
#define SBCLEAR(a) bzero((char *)&(a), sizeof(a))
#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

/*
 * decomp_tkt.c
 * 
 * Copyright 1985, 1986, 1987, 1988 by the Massachusetts Institute
 * of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 */

#include "mit-copyright.h"
#include "des.h"
#include "krb.h"
#include "prot.h"
#include <string.h>

#ifdef KRB_CRYPT_DEBUG
extern int krb_debug;
#endif

/*
 * This routine takes a ticket and pointers to the variables that
 * should be filled in based on the information in the ticket.  It
#ifndef NOENCRYPTION
 * decrypts the ticket using the given key, and 
#endif
 * fills in values for its arguments.
 *
 * Note: if the client realm field in the ticket is the null string,
 * then the "prealm" variable is filled in with the local realm (as
 * defined by KRB_REALM).
 *
 * If the ticket byte order is different than the host's byte order
 * (as indicated by the byte order bit of the "flags" field), then
 * the KDC timestamp "time_sec" is byte-swapped.  The other fields
 * potentially affected by byte order, "paddress" and "session" are
 * not byte-swapped.
 *
 * The routine returns KFAILURE if any of the "pname", "pinstance",
 * or "prealm" fields is too big, otherwise it returns KSUCCESS.
 *
 * The corresponding routine to generate tickets is create_ticket.
 * When changes are made to this routine, the corresponding changes
 * should also be made to that file.
 *
 * See create_ticket.c for the format of the ticket packet.
 */

int
decomp_ticket(tkt, flags, pname, pinstance, prealm, paddress, session,
              life, time_sec, sname, sinstance, key, key_s)
    KTEXT tkt;			/* The ticket to be decoded */
    unsigned char *flags;       /* Kerberos ticket flags */
    char *pname;		/* Authentication name */
    char *pinstance;		/* Principal's instance */
    char *prealm;		/* Principal's authentication domain */
    unsigned KRB4_32 *paddress; /* Net address of entity
                                 * requesting ticket */
    C_Block session;		/* Session key inserted in ticket */
    int *life; 		        /* Lifetime of the ticket */
    unsigned KRB4_32 *time_sec; /* Issue time and date */
    char *sname;		/* Service name */
    char *sinstance;		/* Service instance */
    C_Block key;		/* Service's secret key
                                 * (to decrypt the ticket) */
    Key_schedule key_s;		/* The precomputed key schedule */
{
    static int tkt_swap_bytes;
    unsigned char *uptr;
    char *ptr = (char *)tkt->dat;

#ifndef NOENCRYPTION
    /* Do the decryption */
#ifdef KRB_CRYPT_DEBUG
    if (krb_debug) {
	FILE *fp;
	char *keybuf[BUFSIZ];	/* Avoid secret stuff in stdio buffers */

	fp = fopen("/kerberos/tkt.des", "wb");
	setbuf(fp, keybuf);
	fwrite(tkt->dat, 1, tkt->length, fp);
	fclose(fp);
	memset(keybuf, sizeof(keybuf), 0);	/* Clear the buffer */
    }
#endif
    pcbc_encrypt((C_Block *)tkt->dat,(C_Block *)tkt->dat,
                 (long) tkt->length,key_s,(C_Block *) key,0);
#endif /* ! NOENCRYPTION */
#ifdef KRB_CRYPT_DEBUG
    if (krb_debug) {
	FILE *fp;
	char *keybuf[BUFSIZ];	/* Avoid secret stuff in stdio buffers */

	fp = fopen("/kerberos/tkt.clear", "wb");
	setbuf(fp, keybuf);
	fwrite(tkt->dat, 1, tkt->length, fp);
	fclose(fp);
	memset(keybuf, sizeof(keybuf), 0);	/* Clear the buffer */
    }
#endif

    *flags = *ptr;              /* get flags byte */
    ptr += sizeof(*flags);
    tkt_swap_bytes = 0;
    if (HOST_BYTE_ORDER != ((*flags >> K_FLAG_ORDER)& 1))
        tkt_swap_bytes++;

    if (strlen(ptr) > ANAME_SZ)
        return(KFAILURE);
    (void) strcpy(pname,ptr);   /* pname */
    ptr += strlen(pname) + 1;

    if (strlen(ptr) > INST_SZ)
        return(KFAILURE);
    (void) strcpy(pinstance,ptr); /* instance */
    ptr += strlen(pinstance) + 1;

    if (strlen(ptr) > REALM_SZ)
        return(KFAILURE);
    (void) strcpy(prealm,ptr);  /* realm */
    ptr += strlen(prealm) + 1;
    /* temporary hack until realms are dealt with properly */
    if (*prealm == 0)
        (void) strcpy(prealm,KRB_REALM);

    memcpy((char *)paddress, ptr, 4); /* net address */
    ptr += 4;

    memcpy((char *)session, ptr, 8); /* session key */
    ptr+= 8;
#ifdef notdef /* DONT SWAP SESSION KEY spm 10/22/86 */
    if (tkt_swap_bytes)
        swap_C_Block(session);
#endif

    /* get lifetime, being certain we don't get negative lifetimes */
    uptr = (unsigned char *) ptr++;
    *life = (int) *uptr;

    memcpy((char *) time_sec, ptr, 4); /* issue time */
    ptr += 4;
    if (tkt_swap_bytes)
        swap_u_long(*time_sec);

    (void) strcpy(sname,ptr);   /* service name */
    ptr += 1 + strlen(sname);

    (void) strcpy(sinstance,ptr); /* instance */
    ptr += 1 + strlen(sinstance);

#ifdef KRB_CRYPT_DEBUG
    if (krb_debug) {
	krb_log("service=%s.%s len(sname)=%d, len(sinstance)=%d",
		sname, sinstance, strlen(sname), strlen(sinstance));
	krb_log("ptr - tkt->dat=%d",(char *)ptr - (char *)tkt->dat);
    }
#endif

    return(KSUCCESS);
}

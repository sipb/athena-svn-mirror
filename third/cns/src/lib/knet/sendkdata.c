/*
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <krb.h>

char *PrincipalHostname();

/*
 * SendKerberosData
 * 
 * create and transmit a ticket over the file descriptor for service.host
 * return Kerberos failure codes if appropriate, or KSUCCESS if we
 * get the ticket and write it to the file descriptor
 */

int SendKerberosData(fd, ticket, service, host)
int fd;					/* file descriptor to write onto */
KTEXT ticket;				/* where to put ticket (return) */
char *service, *host;			/* service name, foreign host */
{
    int rem, serv_length;
    char phost[64], p[32];
    char krb_realm[REALM_SZ + 1];

    /* send service name, then authenticator */
    serv_length = htonl(strlen(service));
    write(fd, &serv_length, sizeof(long));
    write(fd, service, strlen(service));

    rem=KSUCCESS;

    memset(krb_realm, 0, sizeof(krb_realm));
    (void) strncpy(krb_realm, krb_realmofhost(host), sizeof(krb_realm) - 1);
    if (rem != KSUCCESS)
      return(rem);

    (void) strncpy(phost,PrincipalHostname(host), sizeof(phost));
    rem = krb_mk_req( ticket, service, phost, krb_realm, (u_long)0 );
    if (rem != KSUCCESS)
      return(rem);

    (void) sprintf(p,"%d ",ticket->length);
    (void) write(fd, p, strlen(p));
    (void) write(fd, ticket->dat, ticket->length);
    return(rem);
}

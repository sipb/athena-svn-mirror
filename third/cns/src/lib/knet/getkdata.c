/*
 * Copyright 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 */

#include <krb.h>
#include <sys/types.h>
#include <netinet/in.h>

#define	FAIL	'\01'
#define SUCCEED	'\0'

/*
 * GetKerberosData
 *
 * get ticket from file descriptor and decode it.
 * Return KFAILURE if we barf on reading the ticket, else return
 * the value of rd_ap_req() applied to the ticket.
 */
int
GetKerberosData(fd, haddr, kdata, service)
	int fd;				/* file descr. to read from */
	struct in_addr haddr;		/* address of foreign host on fd */
	AUTH_DAT *kdata;		/* kerberos data (returned) */
	char *service;			/* service principal desired */
{

	char p[20];
	KTEXT_ST ticket;	/* will get Kerberos ticket from client */
	int i;
	char instance[INST_SZ];
	unsigned long vers;
	char vers_buf[sizeof(vers)];
	char fail = FAIL, succeed = SUCCEED;
	int do_handshake = 0;
	int retval;

	/* We may be talking to one of two types of client:
	   1) sends a longword (net order) of version number
	   (currently 0), then ascii length
	   2) Sends ascii value of length

	   We assume 1) until proven otherwise.
	 */
	for (i = 0; i < sizeof(vers); i++) {
		if (read(fd, &vers_buf[i], 1) != 1) {
			return(KFAILURE);
		}
	}
	(void) memcpy(&vers, vers_buf, sizeof(vers));

	vers = ntohs(vers);
	if (vers == 0) {		/* version # 0 */
		do_handshake = 1;
		i = 0;
	} else {
	    /* old style */
	    memcpy(p, vers_buf, sizeof(vers));
	    for (i = 0; i < sizeof(vers); i++)
		if (p[i] == ' ') {	/* delimiter */
		    register int j;
		    p[i] = '\0';
		    ticket.length = atoi(p);
		    if (ticket.length <= 0 || ticket.length >MAX_KTXT_LEN) {
			return(KFAILURE);
		    }
		    i++;
		    /* copy beginning of ticket to ticket */
		    for (j = 0; i < sizeof(vers); i++, j++)
			ticket.dat[j] = p[i];
		    i = j;
		    goto readticket;
		}
	}

	/*
	 * Get the Kerberos ticket.  The first few characters, terminated
	 * by a blank, should give us a length; then get than many chars
	 * which will be the ticket proper.
	 */
	for (; i<20; i++) {
		if (read(fd, &p[i], 1) != 1) {
		    if (do_handshake)
			    write(fd, "\01", 1);
		    return(KFAILURE);
		}
		if (p[i] == ' ') {
		    p[i] = '\0';
		    break;
		}
	}
	ticket.length = atoi(p);
	if ((i==20) || (ticket.length<=0) || (ticket.length>MAX_KTXT_LEN)) {
		    if (do_handshake)
			    write(fd, &fail, 1);
		    return(KFAILURE);
	}
	i = 0;
readticket:
	for (; i<ticket.length; i++) {
	    if (read(fd, &(ticket.dat[i]), 1) != 1) {
		    if (do_handshake)
			    write(fd, &fail, 1);
		    return(KFAILURE);
	    }
	}
	/*
	 * now have the ticket.  use it to get the authenticated
	 * data from Kerberos.
	 */
	strcpy(instance,"*");	/* let Kerberos fill it in */

	retval = krb_rd_req(&ticket,service,instance,haddr.s_addr,kdata,"");
	if (do_handshake)
		write(fd, (retval == KSUCCESS) ? &succeed : &fail, 1);
	return(retval);
}

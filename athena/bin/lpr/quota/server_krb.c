/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/server_krb.c,v $
 *	$Author: epeisach $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/server_krb.c,v 1.2 1990-04-25 11:52:47 epeisach Exp $
 */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#if (!defined(lint) && !defined(SABER))
static char quota_server_rcsid[] = "$Header: /afs/dev.mit.edu/source/repository/athena/bin/lpr/quota/server_krb.c,v 1.2 1990-04-25 11:52:47 epeisach Exp $";
#endif (!defined(lint) && !defined(SABER))

#include "mit-copyright.h"
#include "quota.h"
#include <krb.h>
#include "quota_ncs.h"
#include <des.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>

/* Returns non-zero on error */

check_krb_auth(h, ticket, ad)
KTEXT ticket;
AUTH_DAT *ad;
handle_t h;
{

    int ret;
    char instance[INST_SZ];
    socket_$addr_t saddr;
    struct sockaddr_in *sin = (struct sockaddr_in *)&saddr;
    
    unsigned long slen = sizeof(socket_$addr_t);
    status_$t st;

    rpc_$inq_binding(h, &saddr, &slen, &st);
    if (st.all !=0)
	pfm_$signal(st); /* "Can't happen" */

    strcpy(instance, "*");
    ret = krb_rd_req(ticket, KLPQUOTA_SERVICE, instance,
		     sin->sin_addr.s_addr, ad, "");
    return((ret == RD_AP_OK) ? 0 : ret);
}

/* Form a complete string name consisting of principal,
 * instance and realm
 */
make_kname(principal, instance, realm, out_name)
char *principal, *instance, *realm, *out_name;
{
    if ((instance[0] == '\0') && (realm[0] == '\0'))
	strcpy(out_name, principal);
    else {
	if (realm[0] == '\0')
	    sprintf(out_name, "%s.%s", principal, instance);
	else {
	    if (instance[0] == '\0')
		sprintf(out_name, "%s@%s", principal, realm);
	    else
		sprintf(out_name, "%s.%s@%s", principal,
			instance, realm);
	}
    }
}

is_suser(principal)
char *principal;
{
    extern char aclname[];
    return(acl_check(aclname, principal));
}

is_sacct(principal)
char *principal;
{
    extern char saclname[];
    return(acl_check(saclname, principal));
}

parse_username(name, qprincipal, qinstance, qrealm)
char *name, *qprincipal, *qinstance, *qrealm;
{
#ifdef KERBEROS
    extern char my_realm[];

    qprincipal[0] = '\0';
    qinstance[0] = '\0';
    qrealm[0] = '\0';

    if(kname_parse(qprincipal, qinstance, qrealm, name)) {
	return;
    }

    if(qrealm[0] == (char) 0)
	strcpy(qrealm, my_realm);

#else !(KERBEROS)
    strcpy(qprincipal, name);
    qinstance[0] = '\0';
    qrealm[0] = '\0';
#endif !(KERBEROS)
    return;
}

char *set_service(in)
char *in;
{
    if(!in || in[0] == '\0') return DEFSERVICE;
    else return in;
}

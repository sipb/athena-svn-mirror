/*
 *	$Id: server_krb.c,v 1.6 1999-01-22 23:11:15 ghudson Exp $
 */

/*
 * Copyright (c) 1990 by the Massachusetts Institute of Technology.
 * For copying and distribution information, see the file "mit-copyright.h."
 */

#if (!defined(lint) && !defined(SABER))
static char quota_server_rcsid[] = "$Id: server_krb.c,v 1.6 1999-01-22 23:11:15 ghudson Exp $";
#endif (!defined(lint) && !defined(SABER))

#include "mit-copyright.h"
#include "quota.h"
#include <krb.h>
#include "quota_limits.h"
#include "quota_ncs.h"
#include <des.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <syslog.h>

/* Returns non-zero on error */

check_krb_auth(h, ticket, ad)
#if 0
KTEXT ticket;
#else
krb_ktext *ticket;
#endif
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

    (void) strcpy(instance, "*");
    ret = krb_rd_req(ticket, KLPQUOTA_SERVICE, instance,
		     sin->sin_addr.s_addr, ad, "");
    return((ret == RD_AP_OK) ? 0 : ret);
}

/* Form a complete string name consisting of principal,
 * instance and realm
 */
#ifdef __STDC__
void make_kname(char *principal, char *instance, char *realm, char *out_name)
#else
void make_kname(principal, instance, realm, out_name)
char *principal, *instance, *realm, *out_name;
#endif
{
    if(realm == NULL) realm = "";
    if(instance == NULL) instance = "";
    if(principal == NULL) principal = "";

    if ((instance[0] == '\0') && (realm[0] == '\0'))
	(void) strcpy(out_name, principal);
    else {
	if (realm[0] == '\0')
	    (void) sprintf(out_name, "%s.%s", principal, instance);
	else {
	    if (instance[0] == '\0')
		(void) sprintf(out_name, "%s@%s", principal, realm);
	    else
		(void) sprintf(out_name, "%s.%s@%s", principal,
			instance, realm);
	}
    }
}

#ifdef __STDC__
is_suser(char *principal)
#else
is_suser(principal)
char *principal;
#endif
{
    extern char aclname[];
    return(acl_check(aclname, principal));
}

#ifdef __STDC__
is_sacct(char *principal, char *qservice)
#else
is_sacct(principal, qservice)
char *principal;
char *qservice;
#endif
{
    extern char saclname[];
    char aclkey[MAX_K_NAME_SZ + SERV_SZ + 1];

    (void) sprintf(aclkey, "%s:%s", principal, qservice);
    return(acl_check(saclname, aclkey));
}

#ifdef __STDC__
parse_username(unsigned char *name, char *qprincipal, 
	       char *qinstance, char *qrealm)
#else
parse_username(name, qprincipal, qinstance, qrealm)
#endif
unsigned char *name;
char *qprincipal, *qinstance, *qrealm;
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
	(void) strcpy(qrealm, my_realm);

#else !(KERBEROS)
    (void) strcpy(qprincipal, name);
    qinstance[0] = '\0';
    qrealm[0] = '\0';
#endif !(KERBEROS)
    return;
}

#ifdef __STDC__
char *set_service(char *in)
#else
char *set_service(in)
char *in;
#endif
{
    if(!in || in[0] == '\0') return DEFSERVICE;
    else return in;
}

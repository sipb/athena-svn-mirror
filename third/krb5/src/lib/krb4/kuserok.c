/*
 * kuserok.c
 *
 * Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 *
 * For copying and distribution information, please see the file
 * <mit-copyright.h>.
 *
 * kuserok: check if a kerberos principal has
 * access to a local account
 */

#include "mit-copyright.h"

#include "krb5.h"
#include "krb.h"

#define OK 0
#define NOTOK 1

/*
 * Given a Kerberos principal "kdata", and a local username "luser",
 * determine whether user is authorized to login according to the
 * authorization file ("~luser/.klogin" by default).  Returns OK
 * if authorized, NOTOK if not authorized.
 *
 * If there is no account for "luser" on the local machine, returns
 * NOTOK.  If there is no authorization file, and the given Kerberos
 * name "kdata" translates to the same name as "luser" (using
 * krb_kntoln()), returns OK.  Otherwise, if the authorization file
 * can't be accessed, returns NOTOK.  Otherwise, the file is read for
 * a matching principal name, instance, and realm.  If one is found,
 * returns OK, if none is found, returns NOTOK.
 *
 * The file entries are in the format:
 *
 *	name.instance@realm
 *
 * one entry per line.
 */

kuserok(kdata, luser)
    AUTH_DAT *kdata;
    char   *luser;
{
    krb5_context context;
    krb5_principal principal;
    int status = NOTOK;

    krb5_init_context(&context);
    if (krb5_425_conv_principal(context, kdata->pname, kdata->pinst,
				kdata->prealm, &principal) == 0) {
	if (krb5_kuserok(context, principal, luser))
	    status = OK;
	krb5_free_principal(context, principal);
    }
    krb5_free_context(context);
    return status;
}

/*
 * lib/krb4/kuserok.c
 *
 * Copyright 1987, 1988 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 *
 * kuserok: check if a kerberos principal has
 * access to a local account
 */

#include "krb5.h"
#include "krb.h"

#if !defined(_WIN32)

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

int KRB5_CALLCONV
kuserok(kdata, luser)
    AUTH_DAT	*kdata;
    char	*luser;
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

#endif

/*
 * lib/krb5/keytab/any/kta_resolv.c
 *
 * Copyright 1990 by the Massachusetts Institute of Technology.
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
 * permission.  M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 * 
 *
 * This is an implementation specific resolver.  It tries to resolve a
 * list of keytab types in sequence and returns the first one which
 * succeeds.
 */

#include "k5-int.h"
#include "ktany.h"

krb5_error_code
krb5_ktany_resolve(context, name, id)
    krb5_context context;
    const char *name;
    krb5_keytab *id;
{
    char *buf, *p, *q;
    krb5_error_code kerror;

    /* Make a copy of name which we can modify. */
    buf = malloc(strlen(name) + 1);
    if (buf == NULL)
	return(ENOMEM);
    strcpy(buf, name);

    p = buf;
    while ((q = strchr(p, ',')) != NULL) {
	*q = 0;
	kerror = krb5_kt_resolve(context, p, id);
	if (kerror != ENOENT) {
	    krb5_xfree(buf);
	    return(kerror);
	}
	p = q + 1;
    }
    krb5_xfree(buf);
    return(krb5_kt_resolve(context, name + (p - buf), id));
}

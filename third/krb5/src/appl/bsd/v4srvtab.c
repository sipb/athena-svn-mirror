/*
 * appl/bsd/v4srvtab.c
 *
 * Copyright 1997 by the Massachusetts Institute of Technology.
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
 */

#ifdef KERBEROS

#include "k5-int.h"

extern char *krb5_defkeyname;

/* The maximum sizes for V4 aname, realm, sname, and instance +1 */
/* Taken from krb.h */
#define 	ANAME_SZ	40
#define		REALM_SZ	40
#define		SNAME_SZ	40
#define		INST_SZ		40

static krb5_error_code ktv4_resolve(krb5_context context, const char *name,
				    krb5_keytab *id);
static krb5_error_code ktv4_get_name(krb5_context context, krb5_keytab id,
				     char *name, int len);
static krb5_error_code ktv4_close(krb5_context context, krb5_keytab id);
static krb5_error_code ktv4_get_entry(krb5_context context, krb5_keytab id,
				      krb5_principal principal, krb5_kvno kvno,
				      krb5_enctype enctype,
				      krb5_keytab_entry *entry);

krb5_kt_ops ktv4_ops = {
    0,
    "V4SRVTAB",	/* Prefix -- this string should not appear anywhere else! */
    ktv4_resolve,
    ktv4_get_name, 
    ktv4_close,
    ktv4_get_entry,
    0,
    0,
    0,
    0,
    0,
    NULL
};

typedef struct _ktv4_data {
    char *name;
} ktv4_data;

void ktv4_init(krb5_context context)
{
    krb5_kt_register(context, &ktv4_ops);
    krb5_defkeyname = "V4SRVTAB:/etc/athena/srvtab";
}

static krb5_error_code ktv4_resolve(krb5_context context, const char *name,
				    krb5_keytab *id)
{
    ktv4_data *data;

    if ((*id = (krb5_keytab) malloc(sizeof(**id))) == NULL)
	return(ENOMEM);

    (*id)->ops = &ktv4_ops;
    if ((data = (ktv4_data *)malloc(sizeof(ktv4_data))) == NULL) {
	krb5_xfree(*id);
	return(ENOMEM);
    }

    if ((data->name = (char *)malloc(strlen(name) + 1)) == NULL) {
	krb5_xfree(data);
	krb5_xfree(*id);
	return(ENOMEM);
    }

    (void) strcpy(data->name, name);

    (*id)->data = (krb5_pointer)data;
    (*id)->magic = KV5M_KEYTAB;
    return(0);
}

static krb5_error_code ktv4_get_name(krb5_context context, krb5_keytab id,
				     char *name, int len)
{
    ktv4_data *data = (ktv4_data *)id->data;

    if (len < strlen(id->ops->prefix) + strlen(data->name) + 2)
	return(KRB5_KT_NAME_TOOLONG);
    sprintf(name, "%s:%s", id->ops->prefix, data->name);
    return(0);
}

static krb5_error_code ktv4_close(krb5_context context, krb5_keytab id)
{
    krb5_xfree(((ktv4_data *)id->data)->name);
    krb5_xfree(id->data);
    id->ops = 0;
    krb5_xfree(id);
    return(0);
}

static krb5_error_code ktv4_get_entry(krb5_context context, krb5_keytab id,
				      krb5_principal principal, krb5_kvno kvno,
				      krb5_enctype enctype,
				      krb5_keytab_entry *entry)
{
    char *filename = ((ktv4_data *)id->data)->name;
    krb5_error_code code;
    int v4code, v4kvno = kvno;
    char v4name[ANAME_SZ], v4inst[INST_SZ], v4realm[REALM_SZ], v4key[8];

    /* Translate to a Kerberos 4 principal. */
    code = krb5_524_conv_principal(context, principal, v4name, v4inst,
				   v4realm);
    if (code)
	return(code);

    /* Look up the requested key in the srvtab. */
    v4code = get_service_key(v4name, v4inst, v4realm, &v4kvno, filename,
			     v4key);
    if (v4code)
	return(KRB5_KT_NOTFOUND);

    /* In case v4inst is modified (it could be a wildcard, maybe), convert
     * back to a Kerberos 5 principal for entry->principal. */
    code = krb5_425_conv_principal(context, v4name, v4inst, v4realm,
				   &entry->principal);
    if (code)
	return(code);

    /* Set up the rest of principal and return. */
    entry->magic = KV5M_KEYTAB_ENTRY;
    entry->timestamp = 0;
    entry->vno = v4kvno;
    entry->key.magic = KV5M_KEYBLOCK;
    entry->key.enctype = ENCTYPE_DES_CBC_CRC;
    entry->key.length = sizeof(v4key);
    entry->key.contents = (krb5_octet *)malloc(sizeof(v4key));
    if (!entry->key.contents)
	return(ENOMEM);
    memcpy(entry->key.contents, v4key, sizeof(v4key));
    return 0;
}

#endif /* KERBEROS */

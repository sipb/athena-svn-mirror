/*
 * lib/krb5/keytab/any/kta_ops.c
 *
 * Copyright 1998, 1999 by the Massachusetts Institute of Technology.
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
 * krb5_kta_ops
 */

#include "k5-int.h"

typedef struct _krb5_ktany_data {
    char *name;
    krb5_keytab *choices;
    int nchoices;
} krb5_ktany_data;

typedef struct _krb5_ktany_cursor_data {
    int which;
    krb5_kt_cursor cursor;
} krb5_ktany_cursor_data;

static krb5_error_code krb5_ktany_resolve
        (krb5_context,
		   const char *,
		   krb5_keytab *);
static krb5_error_code krb5_ktany_get_name
	(krb5_context context,
		   krb5_keytab id,
		   char *name,
		   int len);
static krb5_error_code krb5_ktany_close
	(krb5_context context,
		   krb5_keytab id);
static krb5_error_code krb5_ktany_get_entry
	(krb5_context context,
		   krb5_keytab id,
		   krb5_const_principal principal,
		   krb5_kvno kvno,
		   krb5_enctype enctype,
		   krb5_keytab_entry *entry);
static krb5_error_code krb5_ktany_start_seq_get
	(krb5_context context,
		   krb5_keytab id,
		   krb5_kt_cursor *cursorp);
static krb5_error_code krb5_ktany_next_entry
	(krb5_context context,
		   krb5_keytab id,
		   krb5_keytab_entry *entry,
		   krb5_kt_cursor *cursor);
static krb5_error_code krb5_ktany_end_seq_get
	(krb5_context context,
		   krb5_keytab id,
		   krb5_kt_cursor *cursor);
static void cleanup
	(krb5_context context,
		   krb5_ktany_data *data,
		   int nchoices);

struct _krb5_kt_ops krb5_kta_ops = {
    0,
    "ANY", 	/* Prefix -- this string should not appear anywhere else! */
    krb5_ktany_resolve,
    krb5_ktany_get_name,
    krb5_ktany_close,
    krb5_ktany_get_entry,
    krb5_ktany_start_seq_get,
    krb5_ktany_next_entry,
    krb5_ktany_end_seq_get,
    0,
    0,
    0
};

static krb5_error_code
krb5_ktany_resolve(context, name, id)
    krb5_context context;
    const char *name;
    krb5_keytab *id;
{
    const char *p, *q;
    char *copy;
    krb5_error_code kerror;
    krb5_ktany_data *data;
    int i;

    /* Allocate space for our data and remember a copy of the name. */
    if ((data = (krb5_ktany_data *)malloc(sizeof(krb5_ktany_data))) == NULL)
	return(ENOMEM);
    if ((data->name = (char *)malloc(strlen(name) + 1)) == NULL) {
	krb5_xfree(data);
	return(ENOMEM);
    }
    strcpy(data->name, name);

    /* Count the number of choices and allocate memory for them. */
    data->nchoices = 1;
    for (p = name; (q = strchr(p, ',')) != NULL; p = q + 1)
	data->nchoices++;
    if ((data->choices = (krb5_keytab *)
	 malloc(data->nchoices * sizeof(krb5_keytab))) == NULL) {
	krb5_xfree(data->name);
	krb5_xfree(data);
	return(ENOMEM);
    }

    /* Resolve each of the choices. */
    i = 0;
    for (p = name; (q = strchr(p, ',')) != NULL; p = q + 1) {
	/* Make a copy of the choice name so we can terminate it. */
	if ((copy = (char *)malloc(q - p + 1)) == NULL) {
	    cleanup(context, data, i);
	    return(ENOMEM);
	}
	memcpy(copy, p, q - p);
	copy[q - p] = 0;

	/* Try resolving the choice name. */
	kerror = krb5_kt_resolve(context, copy, &data->choices[i]);
	krb5_xfree(copy);
	if (kerror) {
	    cleanup(context, data, i);
	    return(kerror);
	}
	i++;
    }
    if ((kerror = krb5_kt_resolve(context, p, &data->choices[i]))) {
	cleanup(context, data, i);
	return(kerror);
    }

    /* Allocate and fill in an ID for the caller. */
    if ((*id = (krb5_keytab)malloc(sizeof(**id))) == NULL) {
	cleanup(context, data, i);
	return(ENOMEM);
    }
    (*id)->ops = &krb5_kta_ops;
    (*id)->data = (krb5_pointer)data;
    (*id)->magic = KV5M_KEYTAB;

    return(0);
}

static krb5_error_code
krb5_ktany_get_name(context, id, name, len)
    krb5_context context;
    krb5_keytab id;
    char *name;
    int len;
{
    krb5_ktany_data *data = (krb5_ktany_data *)id->data;

    if (len < strlen(data->name) + 1)
	return(KRB5_KT_NAME_TOOLONG);
    strcpy(name, data->name);
    return(0);
}

static krb5_error_code
krb5_ktany_close(context, id)
    krb5_context context;
    krb5_keytab id;
{
    krb5_ktany_data *data = (krb5_ktany_data *)id->data;

    cleanup(context, data, data->nchoices);
    id->ops = 0;
    krb5_xfree(id);
    return(0);
}

static krb5_error_code
krb5_ktany_get_entry(context, id, principal, kvno, enctype, entry)
    krb5_context context;
    krb5_keytab id;
    krb5_const_principal principal;
    krb5_kvno kvno;
    krb5_enctype enctype;
    krb5_keytab_entry *entry;
{
    krb5_ktany_data *data = (krb5_ktany_data *)id->data;
    krb5_error_code kerror;
    int i;

    for (i = 0; i < data->nchoices; i++) {
	if ((kerror = krb5_kt_get_entry(context, data->choices[i], principal,
					kvno, enctype, entry)) != ENOENT)
	    return kerror;
    }
    return kerror;
}

static krb5_error_code
krb5_ktany_start_seq_get(context, id, cursorp)
    krb5_context context;
    krb5_keytab id;
    krb5_kt_cursor *cursorp;
{
    krb5_ktany_data *data = (krb5_ktany_data *)id->data;
    krb5_ktany_cursor_data *cdata;
    krb5_kt_cursor cursor;
    krb5_error_code kerror;
    int i;

    if ((cdata = (krb5_ktany_cursor_data *)
	 malloc(sizeof(krb5_ktany_cursor_data))) == NULL)
	return(ENOMEM);

    /* Find a choice which can handle the serialization request. */
    for (i = 0; i < data->nchoices; i++) {
	if ((kerror = krb5_kt_start_seq_get(context, data->choices[i],
					    &cdata->cursor)) == 0)
	    break;
	else if (kerror != ENOENT) {
	    krb5_xfree(cdata);
	    return(kerror);
	}
    }

    if (i == data->nchoices) {
	/* Everyone returned ENOENT, so no go. */
	krb5_xfree(cdata);
	return(kerror);
    }

    cdata->which = i;
    *cursorp = (krb5_kt_cursor)cdata;
    return(0);
}

static krb5_error_code
krb5_ktany_next_entry(context, id, entry, cursor)
    krb5_context context;
    krb5_keytab id;
    krb5_keytab_entry *entry;
    krb5_kt_cursor *cursor;
{
    krb5_ktany_data *data = (krb5_ktany_data *)id->data;
    krb5_ktany_cursor_data *cdata = (krb5_ktany_cursor_data *)*cursor;
    krb5_keytab choice_id;

    choice_id = data->choices[cdata->which];
    return(krb5_kt_next_entry(context, choice_id, entry, &cdata->cursor));
}

static krb5_error_code
krb5_ktany_end_seq_get(context, id, cursor)
    krb5_context context;
    krb5_keytab id;
    krb5_kt_cursor *cursor;
{
    krb5_ktany_data *data = (krb5_ktany_data *)id->data;
    krb5_ktany_cursor_data *cdata = (krb5_ktany_cursor_data *)*cursor;
    krb5_keytab choice_id;
    krb5_error_code kerror;

    choice_id = data->choices[cdata->which];
    kerror = krb5_kt_end_seq_get(context, choice_id, &cdata->cursor);
    krb5_xfree(cdata);
    return(kerror);
}

static void
cleanup(context, data, nchoices)
    krb5_context context;
    krb5_ktany_data *data;
    int nchoices;
{
    int i;

    krb5_xfree(data->name);
    for (i = 0; i < nchoices; i++)
	krb5_kt_close(context, data->choices[i]);
    krb5_xfree(data->choices);
    krb5_xfree(data);
}
/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-fault.c: representation of a SOAP fault.
 *
 * Authors:
 *      Rodrigo Moya (rodrigo@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include "soup-fault.h"

struct _SoupFault {
	guchar *faultcode;
	guchar *faultstring;
	guchar *faultactor;
	guchar *faultdetail;
};

/**
 * soup_fault_new:
 * @code: the FaultCode element.
 * @string: the FaultString element.
 * @actor: the FaultActor element.
 * @detail: the FaultDetail element.
 * 
 * Creates a new SoupFault.
 *
 * Return value: the newly allocated SoupFault, or NULL if @code is not
 * supplied.
 */
SoupFault *
soup_fault_new (const guchar *code,
		const guchar *string,
		const guchar *actor,
		const guchar *detail)
{
	SoupFault *fault;

	g_return_val_if_fail (code != NULL, NULL);

	fault = g_new0 (SoupFault, 1);
	fault->faultcode   = g_strdup (code);
	fault->faultstring = g_strdup (string);
	fault->faultactor  = g_strdup (actor);
	fault->faultdetail = g_strdup (detail);

	return fault;
}

const guchar *
soup_fault_get_code (SoupFault *fault)
{
	g_return_val_if_fail (fault != NULL, NULL);
	return (const guchar *) fault->faultcode;
}

const guchar *
soup_fault_get_string (SoupFault *fault)
{
	g_return_val_if_fail (fault != NULL, NULL);
	return (const guchar *) fault->faultstring;
}

const guchar *
soup_fault_get_actor (SoupFault *fault)
{
	g_return_val_if_fail (fault != NULL, NULL);
	return (const guchar *) fault->faultactor;
}

const guchar *
soup_fault_get_detail (SoupFault *fault)
{
	g_return_val_if_fail (fault != NULL, NULL);
	return (const guchar *) fault->faultdetail;
}

void
soup_fault_set_code (SoupFault *fault, const guchar *code)
{
	g_return_if_fail (fault != NULL);

	if (fault->faultcode != NULL)
		g_free (fault->faultcode);
	fault->faultcode = g_strdup (code);
}

void
soup_fault_set_string (SoupFault *fault, const guchar *string)
{
	g_return_if_fail (fault != NULL);

	if (fault->faultstring != NULL)
		g_free (fault->faultstring);
	fault->faultstring = g_strdup (string);
}

void
soup_fault_set_actor (SoupFault *fault, const guchar *actor)
{
	g_return_if_fail (fault != NULL);

	if (fault->faultactor != NULL)
		g_free (fault->faultactor);

	fault->faultactor = g_strdup (actor);
}

void
soup_fault_set_detail (SoupFault *fault, const guchar *detail)
{
	g_return_if_fail (fault != NULL);

	if (fault->faultdetail)
		g_free (fault->faultdetail);

	fault->faultdetail = g_strdup (detail);
}

/**
 * soup_fault_free:
 * @fault: the %SoupFault to be destroyed.
 * 
 * Destroyes the SoupFault pointed to by @fault, freeing all members.
 */
void
soup_fault_free (SoupFault *fault)
{
	g_return_if_fail (fault != NULL);

	if (fault->faultcode)
		g_free (fault->faultcode);
	if (fault->faultstring)
		g_free (fault->faultstring);
	if (fault->faultactor)
		g_free (fault->faultactor);
	if (fault->faultdetail)
		g_free (fault->faultdetail);

	g_free ((gpointer) fault);
}

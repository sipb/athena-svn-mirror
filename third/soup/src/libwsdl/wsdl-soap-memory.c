/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-soap-memory.c: Runtime memory management
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <glib.h>
#include <string.h>
#include <stdlib.h>

#include "wsdl-param.h"
#include "wsdl-soap-memory.h"

/**
 * wsdl_soap_initialise:
 * @params: a pointer to an array of #wsdl_param, terminated by a set
 * of #NULL elements
 *
 * Clears the pointers within the @params array that point to memory
 * that will be filled in by wsdl_soap_parse().
 */
void
wsdl_soap_initialise (const wsdl_param * const params)
{
	const wsdl_param *param;

	/* Zero the memory that will be filled in with type data */
	param = params;

	while (param->name != NULL) {
		if (param->typecode == NULL) {
			g_warning (G_GNUC_FUNCTION
				   ": Parameter %s has no typecode!",
				   param->name);
		} else {
			if (param->param == NULL) {
				g_warning (G_GNUC_FUNCTION
					   ": Parameter %s has no "
					   "memory location!",
					   param->name);
			} else {
				memset (param->param, 
					'\0',
					wsdl_typecode_size (param->typecode));
			}
		}

		param++;
	}
}

/**
 * wsdl_soap_free:
 * @params: a pointer to an array of #wsdl_param, terminated by a set
 * of #NULL elements
 *
 * Frees the memory pointed to within the @params array that contains
 * C instances of typecodes.
 */
void
wsdl_soap_free (const wsdl_param * const params)
{
	const wsdl_param *param;

	/* Free the memory that pointed to by '->param' if it isnt NULL */
	param = params;

	while (param->name != NULL) {
		if (param->typecode == NULL) {
			g_warning (G_GNUC_FUNCTION
				   ": Parameter %s has no typecode!",
				   param->name);
		} else {
			if (param->param != NULL &&
			    *(guchar **) param->param != NULL &&
			    param->typecode->free_func != NULL) {
				param->typecode->free_func (
					*(guchar **) param->param);
			}
		}

		param++;
	}
}

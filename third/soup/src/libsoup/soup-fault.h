/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * soup-fault.h: representation of a SOAP fault.
 *
 * Authors:
 *      Rodrigo Moya (rodrigo@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef SOUP_FAULT_H
#define SOUP_FAULT_H

#include <glib.h>

typedef struct _SoupFault SoupFault;

SoupFault    *soup_fault_new        (const guchar *code,
				     const guchar *string,
				     const guchar *actor,
				     const guchar *detail);
void          soup_fault_free       (SoupFault    *fault);

const guchar *soup_fault_get_code   (SoupFault    *fault);
void          soup_fault_set_code   (SoupFault    *fault, 
				     const guchar *code);

const guchar *soup_fault_get_string (SoupFault    *fault);
void          soup_fault_set_string (SoupFault    *fault, 
				     const guchar *string);

const guchar *soup_fault_get_actor  (SoupFault    *fault);
void          soup_fault_set_actor  (SoupFault    *fault, 
				     const guchar *actor);

const guchar *soup_fault_get_detail (SoupFault    *fault);
void          soup_fault_set_detail (SoupFault    *fault, 
				     const guchar *detail);

#endif

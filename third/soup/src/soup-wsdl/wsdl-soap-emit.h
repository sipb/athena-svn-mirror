/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-soap-emit.h: Common routines used for generating code
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_SOAP_EMIT_H_
#define _WSDL_SOAP_EMIT_H_

#include <glib.h>
#include <stdio.h>

void wsdl_emit_part_list (FILE                *out,
			  const GSList *const  parts,
			  const guchar        *fmt);

#endif /* _WSDL_SOAP_EMIT_H_ */

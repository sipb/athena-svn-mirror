/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-common.h: Emit code common to both stubs and skels
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_SOAP_COMMON_H_
#define _WSDL_SOAP_COMMON_H_

#include "wsdl-parse.h"

void wsdl_emit_soap_common (const guchar                  *outdir,
			    const guchar                  *fileroot,
			    const wsdl_definitions *const  definitions);

#endif /* _WSDL_SOAP_COMMON_H_ */

/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-stubs.h: Emit stub routines for clients
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_SOAP_STUBS_H_
#define _WSDL_SOAP_STUBS_H_

#include "wsdl-parse.h"

void wsdl_emit_soap_stubs (const guchar                   *outdir,
			   const guchar                   *fileroot,
			   const wsdl_definitions * const  definitions);

#endif /* _WSDL_SOAP_STUBS_H_ */

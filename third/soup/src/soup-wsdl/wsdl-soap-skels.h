/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-skels.h: Emit skeleton code for servers
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_SOAP_SKELS_H_
#define _WSDL_SOAP_SKELS_H_

#include "wsdl-parse.h"

void wsdl_emit_soap_skels (const guchar                   *outdir,
			   const guchar                   *fileroot,
			   const wsdl_definitions * const  definitions);

#endif /* _WSDL_SOAP_SKELS_H_ */

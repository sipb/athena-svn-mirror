/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-soap-memory.h: Runtime memory management
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_SOAP_MEMORY_H_
#define _WSDL_SOAP_MEMORY_H_

#include <libwsdl/wsdl-param.h>

void wsdl_soap_initialise (const wsdl_param * const params);
void wsdl_soap_free       (const wsdl_param * const params);

#endif /* _WSDL_SOAP_MEMORY_H_ */

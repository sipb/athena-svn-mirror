/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-param.h: Type definitions for SOAP parser and marshaller
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_PARAM_H_
#define _WSDL_PARAM_H_

#include <glib.h>
#include <libwsdl/wsdl-typecodes.h>

typedef struct {
	const guchar        *name;
	void                *param;
	const wsdl_typecode *typecode;
} wsdl_param;

/* WSDL SOAP flags */
enum {
	WSDL_SOAP_FLAGS_REQUEST  = 0x01,
	WSDL_SOAP_FLAGS_RESPONSE = 0x02
};

#endif /* _WSDL_PARAM_H_ */

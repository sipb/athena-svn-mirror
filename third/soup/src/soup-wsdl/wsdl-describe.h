/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-describe.h: Print text description of WSDL tree
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_DESCRIBE_H_
#define _WSDL_DESCRIBE_H_

#include "wsdl-parse.h"

typedef enum {
	WSDL_DESCRIBE_INPUT,
	WSDL_DESCRIBE_OUTPUT,
	WSDL_DESCRIBE_FAULT
} wsdl_describe_iof_t;

void wsdl_describe_types                  (guint                               ind,
					   wsdl_types                         *types);
void wsdl_describe_message_part           (guint                               ind,
					   wsdl_message_part                  *part);
void wsdl_describe_message                (guint                               ind,
					   wsdl_message                       *message);
void wsdl_describe_porttype_operation_iof (guint                               ind,
					   wsdl_porttype_operation_inoutfault *iof,
					   wsdl_describe_iof_t                 type);
void wsdl_describe_porttype_operation     (guint                               ind,
					   wsdl_porttype_operation            *op);
void wsdl_describe_porttype               (guint                               ind,
					   wsdl_porttype                      *porttype);
void wsdl_describe_soap_operation         (guint                               ind,
					   wsdl_soap_operation                *soap_operation);
void wsdl_describe_soap_body              (guint                               ind,
					   wsdl_soap_body                     *soap_body);
void wsdl_describe_soap_header            (guint                               ind,
					   wsdl_soap_header                   *soap_header);
void wsdl_describe_soap_fault             (guint                               ind,
					   wsdl_soap_fault                    *soap_fault);
void wsdl_describe_binding_operation_iof  (guint                               ind,
					   wsdl_binding_operation_inoutfault  *iof,
					   wsdl_describe_iof_t                 type);
void wsdl_describe_binding_operation      (guint                               ind,
					   wsdl_binding_operation             *op);
void wsdl_describe_soap_binding           (guint                               ind,
					   wsdl_soap_binding                  *binding);
void wsdl_describe_binding                (guint                               ind,
					   wsdl_binding                       *binding);
void wsdl_describe_soap_address           (guint                               ind,
					   wsdl_soap_address                  *address);
void wsdl_describe_service_port           (guint                               ind,
					   wsdl_service_port                  *port);
void wsdl_describe_service                (guint                               ind,
					   wsdl_service                       *service);
void wsdl_describe_definitions            (guint                               ind,
					   wsdl_definitions                   *definitions);

#endif /* _WSDL_DESCRIBE_H_ */

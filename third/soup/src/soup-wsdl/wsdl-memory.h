/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-memory.h: Free memory allocated to WSDL structs
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_MEMORY_H_
#define _WSDL_MEMORY_H_

#include "wsdl-parse.h"

void wsdl_free_types                  (wsdl_types                         *types);
void wsdl_free_message_part           (wsdl_message_part                  *part);
void wsdl_free_message                (wsdl_message                       *message);
void wsdl_free_porttype_operation_iof (wsdl_porttype_operation_inoutfault *iof);
void wsdl_free_porttype_operation     (wsdl_porttype_operation            *operation);
void wsdl_free_porttype               (wsdl_porttype                      *porttype);
void wsdl_free_soap_binding           (wsdl_soap_binding                  *binding);
void wsdl_free_soap_operation         (wsdl_soap_operation                *operation);
void wsdl_free_soap_body              (wsdl_soap_body                     *body);
void wsdl_free_soap_header            (wsdl_soap_header                   *header);
void wsdl_free_soap_fault             (wsdl_soap_fault                    *fault);
void wsdl_free_binding_operation_iof  (wsdl_binding_operation_inoutfault  *iof);
void wsdl_free_binding_operation      (wsdl_binding_operation             *operation);
void wsdl_free_binding                (wsdl_binding                       *binding);
void wsdl_free_soap_address           (wsdl_soap_address                  *address);
void wsdl_free_service_port           (wsdl_service_port                  *port);
void wsdl_free_service                (wsdl_service                       *service);
void wsdl_free_definitions            (wsdl_definitions                   *definitions);

#endif /* _WSDL_MEMORY_H_ */

/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-locate.h: Find an element structure, given a name
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_LOCATE_H_
#define _WSDL_LOCATE_H_

#include "wsdl-parse.h"

wsdl_porttype_operation *wsdl_locate_porttype_operation (guchar           *name,
							 wsdl_porttype    *porttype);
wsdl_message_part *      wsdl_locate_message_part       (guchar           *name,
							 wsdl_message     *message);
wsdl_message *           wsdl_locate_message            (guchar           *name,
							 wsdl_definitions *definitions);
wsdl_porttype *          wsdl_locate_porttype           (guchar           *name,
							 wsdl_definitions *definitions);
wsdl_binding *           wsdl_locate_binding            (guchar           *name,
							 wsdl_definitions *definitions);

#endif /* _WSDL_LOCATE_H_ */

/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-typecodes-c.h: Write C code from typecodes
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_TYPECODES_C_H_
#define _WSDL_TYPECODES_C_H_

#include <glib.h>
#include <stdio.h>
#include <stdarg.h>
#include <libwsdl/wsdl-typecodes.h>

void wsdl_typecode_write_c_definition  (FILE                       *out,
					const wsdl_typecode *const  tc);
void wsdl_typecode_write_c_declaration (FILE                       *out,
					const wsdl_typecode *const  tc);
void wsdl_typecode_write_c_mm          (FILE                       *out,
					const wsdl_typecode *const  tc);
void wsdl_typecode_write_c_mm_decl     (FILE                       *out,
					const wsdl_typecode *const  tc);

#endif /* _WSDL_TYPECODES_C_H_ */

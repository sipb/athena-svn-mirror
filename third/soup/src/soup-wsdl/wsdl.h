/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl.h: Common definitions for the WSDL parser
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_WSDL_H_
#define _WSDL_WSDL_H_

extern gboolean      option_warnings_are_errors;
extern gboolean      option_show_doc;
extern gboolean      option_show_xml_tree;
extern gboolean      option_emit_stubs;
extern gboolean      option_emit_skels;
extern gboolean      option_emit_common;
extern gboolean      option_emit_headers;
extern const guchar *option_outdir;
extern const guchar *option_debug_level;
extern const guchar *option_debug_modules;

#endif /* _WSDL_WSDL_H_ */

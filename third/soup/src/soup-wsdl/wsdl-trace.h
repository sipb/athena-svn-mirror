/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-trace.h: Emit skeleton code for servers
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#ifndef _WSDL_TRACE_H_
#define _WSDL_TRACE_H_

#include <stdio.h>
#include <stdarg.h>

#define WSDL_LOG_DOMAIN_STUBS   1<<0
#define WSDL_LOG_DOMAIN_SKELS   1<<1
#define WSDL_LOG_DOMAIN_HEADERS 1<<2
#define WSDL_LOG_DOMAIN_COMMON  1<<3
#define WSDL_LOG_DOMAIN_PARSER  1<<4
#define WSDL_LOG_DOMAIN_THREAD  1<<5
#define WSDL_LOG_DOMAIN_MAIN    1<<6

void  wsdl_parse_debug_domain_string (const guchar *string);
void  wsdl_parse_debug_level_string  (const guchar *string);
void  wsdl_debug                     (guint         domain,
				      guint         level,
				      const guchar *format,
				      ...);

FILE *wsdl_get_output_file (void);
void  wsdl_set_output_file (FILE *file);

#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define W(...) fprintf (wsdl_get_output_file (), __VA_ARGS__)
#elif defined (__GNUC__)
#define W(format...) fprintf (wsdl_get_output_file (), format)
#else /* !__GNUC__ */
static void W (const gchar *format, ...)
{
        va_list args;
        va_start (args, format);
        vfprintf (wsdl_get_output_file (), format, args);
        va_end (args);
}
#endif


#endif /* _WSDL_SKELS_H_ */

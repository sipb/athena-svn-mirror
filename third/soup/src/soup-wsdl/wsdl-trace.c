/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-trace.c: Debugging messages
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <glib.h>
#include <string.h>

#include "wsdl-trace.h"

/**
 * WSDL_LOG_DOMAIN_STUBS:
 *
 * The bit to set to output debugging messages to do with stub
 * generation
 */

/**
 * WSDL_LOG_DOMAIN_SKELS:
 *
 * The bit to set to output debugging messages to do with server
 * skeleton generation
 */

/**
 * WSDL_LOG_DOMAIN_HEADERS:
 *
 * The bit to set to output debugging messages to do with header
 * generation
 */

/**
 * WSDL_LOG_DOMAIN_COMMON:
 *
 * The bit to set to output debugging messages to do with common code
 * generation
 */

/**
 * WSDL_LOG_DOMAIN_PARSER:
 *
 * The bit to set to output debugging messages to do with parsing of
 * the WSDL XML file.
 */

/**
 * WSDL_LOG_DOMAIN_THREAD:
 *
 * The bit to set to output debugging messages to do with threading
 * together the elements of a WSDL specification.
 */

/**
 * WSDL_LOG_DOMAIN_MAIN:
 *
 * The bit to set to output debugging messages to do with the main
 * parser driver.
 */

/* GDebugKey doesnt define the 'key' element const :-(
 * To keep the writable-strings warnings I need to cast the strings, but
 * that stops me using -Wcast-qual
 */
static GDebugKey domain_keys[] = {
	{ (gchar *) "stubs", WSDL_LOG_DOMAIN_STUBS },
	{ (gchar *) "skels", WSDL_LOG_DOMAIN_SKELS },
	{ (gchar *) "headers", WSDL_LOG_DOMAIN_HEADERS },
	{ (gchar *) "common", WSDL_LOG_DOMAIN_COMMON },
	{ (gchar *) "parser", WSDL_LOG_DOMAIN_PARSER },
	{ (gchar *) "thread", WSDL_LOG_DOMAIN_THREAD },
	{ (gchar *) "main", WSDL_LOG_DOMAIN_MAIN },
};

static GDebugKey level_keys[] = {
	{ (gchar *) "warning", G_LOG_LEVEL_WARNING },
	{ (gchar *) "message", G_LOG_LEVEL_MESSAGE },
	{ (gchar *) "info", G_LOG_LEVEL_INFO },
	{ (gchar *) "debug", G_LOG_LEVEL_DEBUG }
};

static guint wsdl_log_domain_mask = 0;
static guint wsdl_log_level_mask = 0;

/**
 * wsdl_parse_debug_domain_string:
 * @string: a string containing a colon-separated set of tokens
 *
 * Parses @string, and sets the internal logging mask.
 */
void
wsdl_parse_debug_domain_string (const guchar * string)
{
	if (string != NULL) {
		wsdl_log_domain_mask =
			g_parse_debug_string (string, 
					      domain_keys,
					      sizeof (domain_keys) /
					      sizeof (domain_keys[0]));
	} else {
		wsdl_log_domain_mask = 0;
	}
}

/**
 * wsdl_parse_debug_level_string:
 * @string: a string containing a colon-separated set of tokens
 *
 * Parses @string and sets the internal logging priority level.
 */
void
wsdl_parse_debug_level_string (const guchar * string)
{
	if (string != NULL) {
		guint bit;
		guint mask = g_parse_debug_string (string, 
						   level_keys,
						   sizeof (level_keys) /
						   sizeof (level_keys[0]));

		/* find the highest set bit, and set all the bits below it */
		bit = g_bit_nth_msf (mask, -1);
		wsdl_log_level_mask = mask | ~(~1 << bit);
	} else {
		wsdl_log_level_mask = 0;
	}
}

/**
 * wsdl_debug:
 * @domain: an integer with one or more #WSDL_LOG_DOMAIN_ bits set
 * @level: an integer with one or more #G_LOG_LEVEL_ bits set
 * @format: a printf-style string
 * @...: Arguments required by @format
 *
 * If @domain and @level match bits set in the internal logging mask
 * and level, then g_logv() is called with arguments "SOUP-WSDL",
 * @level, @format, and a #va_list parameter consisting of ... .
 */
void
wsdl_debug (guint domain, guint level, const guchar * format, ...)
{
	va_list args;

	if ((domain & wsdl_log_domain_mask) && (level & wsdl_log_level_mask)) {
		va_start (args, format);
		g_logv (G_LOG_DOMAIN, level, format, args);
		va_end (args);
	}
}

static FILE *out_file = NULL;

FILE *
wsdl_get_output_file (void)
{
	return out_file;
}

void
wsdl_set_output_file (FILE *file)
{
	out_file = file;
}

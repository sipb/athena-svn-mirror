/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * wsdl-soap-emit.c: Common routines used for generating code
 *
 * Authors:
 *	Dick Porter (dick@ximian.com)
 *
 * Copyright (C) 2001, Ximian, Inc.
 */

#include <glib.h>
#include <string.h>

#include "wsdl-soap-emit.h"
#include "wsdl-parse.h"

static void
wsdl_emit_part (FILE * out, 
		const guchar * fmt,
		const wsdl_message_part * const part)
{
	guchar *pc, *copy, *c, *type;

	pc = strchr (fmt, '%');
	if (pc == NULL) {
		fprintf (out, "%s", fmt);
		return;
	}

	copy = g_strdup (fmt);
	c = copy;

	while ((pc = strchr (c, '%')) != NULL) {
		*pc++ = '\0';

		fprintf (out, "%s", c);

		switch (*pc) {
		case 'p':
			fprintf (out, "%s", part->name);
			break;
		case 't':
			type = wsdl_typecode_param_type (part->typecode);
			fprintf (out, "%s", type);
			g_free (type);
			break;
		case 'n':
			fprintf (out, "%s", wsdl_typecode_ns (part->typecode));
			break;
		case 'N':
			fprintf (out, "%s",
				 wsdl_typecode_name (part->typecode));
			break;
		default:
			fprintf (out, "%%%c", *pc);
			break;
		}

		c = pc + 1;
	}

	/* Print out any left over format chars */
	fprintf (out, "%s", c);

	g_free (copy);
}

/**
 * wsdl_emit_part_list:
 * @out: a stdio FILE pointer
 * @parts: a GSList of #wsdl_message_part structs
 * @fmt: a printf-style format string, but with non-standard
 * formatting codes
 *
 * For each #wsdl_message_part structure in @parts, @fmt is printed to @out
 * with the following formatting codes expanded:
 *
 * %%p: the name of each #wsdl_message_part
 *
 * %%t: the name of the typecode of each #wsdl_message_part
 *
 * %%n: the namespace reference of the typecode of each
 * #wsdl_message_part
 *
 * %%N: the namespace URI of the typecode of each #wsdl_message_part
 */
void
wsdl_emit_part_list (FILE * out, const GSList * const parts, const guchar * fmt)
{
	const GSList *iter = parts;

	while (iter != NULL) {
		wsdl_emit_part (out, fmt, iter->data);

		iter = iter->next;
	}
}

/*
* gok-log.c
*
* Copyright 2001,2002 Sun Microsystems, Inc.,
* Copyright 2001,2002 University Of Toronto
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public
* License along with this library; if not, write to the
* Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "gok-log.h"

#define INDENT 2

static int indent = 0;

void put_indent(FILE* stream) {
    char* indent_string;
    int i;

    indent_string = malloc ( (indent + 1) * sizeof(char) );
    for (i = 0; i < indent ; i++)
	indent_string[i] = ' ';
    indent_string[indent] = 0;
    fputs (indent_string, stream);
    free (indent_string);
}

/* if line is -1 it is not displayed */

void gok_log_impl (const char* file, int line,
		   const char* func, const char* format, ...)
{
    va_list ap;
    va_start (ap, format);
    gok_log_impl_v (file, line, func, format, ap);
    va_end (ap);
}

/* if line is -1 it is not displayed */

void gok_log_impl_v (const char* file, int line,
		     const char* func, const char* format, va_list arg)
{
    int had_prefix = 0;

    put_indent(stderr);
    if ( strlen(file) != 0 ) {
	fprintf (stderr, "%s:", file);
	had_prefix = 1;
    }
    if ( line != -1 ) {
	fprintf (stderr, "%d:", line);
	had_prefix = 1;
    }
    if ( strlen(func) != 0 ) {
	fprintf (stderr, "%s:", func);
	had_prefix = 1;
    }
    if ( had_prefix ) {
	fputs (" ", stderr);
    }
    vfprintf (stderr, format, arg);
    fputs ("\n", stderr);
}

void gok_log_enter_impl (const char* prefix)
{
    put_indent(stderr);
    if ( strlen(prefix) != 0 ) fprintf (stderr, "%s {\n", prefix);
    else fputs ("{\n", stderr);
    indent += INDENT;
}

void gok_log_leave_impl ()
{
    indent -= INDENT;
    if (indent < 0) indent = 0;
    put_indent(stderr);
    fputs ("}\n", stderr);
}

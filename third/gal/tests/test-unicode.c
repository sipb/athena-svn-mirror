/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * test-unicode.c
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *   Jon Trowbridge <trow@ximian.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License, version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <config.h>
#include <glib.h>
#include <gal/unicode/gunicode.h>

static void
test_sizes (void)
{
	gunichar c, cdown;
	gint sz1, sz2;

	for (c=1; c<0xffff; ++c) {
		if (1) {
			cdown = g_unichar_totitle (c);

			if (cdown != c && cdown != 0) {
				sz1 = g_unichar_to_utf8 (c, NULL);
				sz2 = g_unichar_to_utf8 (cdown, NULL);

				if (sz1 != sz2)
					g_print ("%4x => %4x: %d vs %d\n", c, cdown, sz1, sz2);
			}
		}
	}
}

/* Leaks like crazy, but I don't care. */
static void
test_transforms (void)
{
	const gchar * test_cases[] = { "AsSBarn!",
				       "Franis",
				       NULL };
	gchar *s;
	gint i;
		
	for (i=0; test_cases[i]; ++i) {

		g_print ("%s: ", test_cases[i]);

		s = g_strdup (test_cases[i]);
		g_utf8_strdown (s);
		g_print ("%s, ", s);
		g_free (s);

		s = g_strdup (test_cases[i]);
		g_utf8_strup (s);
		g_print ("%s, ", s);
		g_free (s);

		s = g_strdup (test_cases[i]);
		g_utf8_strtitle (s);
		g_print ("%s\n", s);
		g_free (s);
	}
}


gint
main ()
{
	test_sizes ();
	test_transforms ();
	
	return 0;
}

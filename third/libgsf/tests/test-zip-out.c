/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * test-zip-out.c:
 *
 * Copyright (C) 2002-2003 Jon K Hellan (hellan@acm.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2.1 of the GNU Lesser General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <gsf/gsf-utils.h>
#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-outfile.h>
#include <gsf/gsf-outfile-zip.h>

#include <stdio.h>
#include <string.h>

static gboolean
test_write_once (GsfOutput *output)
{
	const char *str = "The cat sat on the mat. 2 cats sat on the mat. The quick brown fox is afraid of the cats.\n";

	return gsf_output_write (output, strlen (str), str);
}

static int
test (int argc, char *argv[])
{
	GsfOutput  *output;
	GsfOutfile *outfile;
	GsfOutput  *child;
	GError   *err;

	if (argc != 2) {
		fprintf (stderr, "Usage : %s outfile\n", argv[0]);
		return 1;
	}

	output = GSF_OUTPUT (gsf_output_stdio_new (argv[1], &err));
	if (output == NULL) {
		g_return_val_if_fail (err != NULL, 1);

		g_warning ("'%s' error: %s", argv[1], err->message);
		g_error_free (err);
		return 1;
	}
	outfile = GSF_OUTFILE (gsf_outfile_zip_new (output, &err));
	if (output == NULL) {
		g_return_val_if_fail (err != NULL, 1);

		g_warning ("'%s' error: %s",
			   "gsf_outfile_zip_new", err->message);
		g_error_free (err);
		return 1;
	}
	g_object_unref (G_OBJECT (output));
	child = gsf_outfile_new_child  (outfile, "child", FALSE);

	if (!test_write_once (child))
		return 1;
	if (!gsf_output_close ((GsfOutput *) child))
		return 1;
	if (!gsf_output_close ((GsfOutput *) outfile))
		return 1;
	g_object_unref (G_OBJECT (outfile));
	g_object_unref (child);

	return 0;
}

int
main (int argc, char *argv[])
{
	int res;

	gsf_init ();
	res = test (argc, argv);
	gsf_shutdown ();

	return res;
}
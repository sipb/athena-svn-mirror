/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Unit tests for GPdf Bonobo Control
 *
 * Copyright (C) 2003 Martin Kretzschmar
 *
 * Author:
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * GPdf is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * GPdf is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 */

#include "aconf.h"
#include "gpdf-g-switch.h"
#  include <gtk/gtk.h>
#include "gpdf-g-switch.h"
#include "gpdf-view.h"
#include <ggv-document.h>

#define TEST_INITIALIZATION \
	gtk_init (&argc, &argv);
#include <unit-test.h>

#define TEST_STR(expected, got)						\
	TESTING_MESSAGE ("string value of " #got);			\
	if (strcmp (expected, got) != 0) {				\
		g_print ("\n Expected \"%s\", got \"%s\"", expected, got); \
		FAIL_TEST ();						\
	}								\
	g_print ("Okay!\n");

PDFDoc *simple_links_doc;

static void
setup ()
{
	simple_links_doc = new PDFDoc (
		new GString (TESTFILES_DIR "/simple-links.pdf"));
}

static void
tear_down ()
{
	delete simple_links_doc;
	simple_links_doc = NULL;
}

TEST_BEGIN (GPdfView, setup, tear_down)

TEST_NEW (view_implements_ggv_document)
{
	GPdfView *view;
	char **page_names;
	char **p;

	view = GPDF_VIEW (g_object_new (GPDF_TYPE_VIEW,
					"aa", TRUE,	/* FIXME: a pref ? */
					"pdf_doc", simple_links_doc, 
					NULL));
	TEST (view != NULL);
	TEST (GPDF_IS_VIEW (view));
	TEST (GGV_IS_DOCUMENT (view));

	TEST (2 == ggv_document_get_page_count (GGV_DOCUMENT (view)));

	page_names = ggv_document_get_page_names (GGV_DOCUMENT (view));

	TEST (NULL != page_names);
	TEST_STR ("1", page_names [0]);
	TEST_STR ("2", page_names [1]);
	TEST (NULL == page_names [2]);

	for (p = page_names; *p; ++p)
		g_free (*p);

	g_free (page_names);

	gtk_object_sink (GTK_OBJECT (view));
}

TEST_END ()

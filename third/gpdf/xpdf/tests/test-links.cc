/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* Unit tests for PDF links UI
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

#include "gpdf-links-canvas-layer.h"
#include "gpdf-link-canvas-item.h"
#include "Object.h"
#include "PDFDoc.h"
#include "Link.h"
#include "gpdf-g-switch.h"
#  include <gtk/gtk.h>
#include "gpdf-g-switch.h"

#define TEST_INITIALIZATION	\
	gtk_init (&argc, &argv);

#include <unit-test.h>

GnomeCanvas *canvas;

static void
setup ()
{
	canvas = GNOME_CANVAS (gnome_canvas_new_aa ());
}

static void
tear_down ()
{
	gtk_object_sink (GTK_OBJECT (canvas));
	canvas = NULL;
}

static Links *
pdf_doc_links_for_page (PDFDoc *pdf_doc, int n)
{
	Page *page;
	Object obj;
	Links *links;

	page = pdf_doc->getCatalog ()->getPage (n);
	links = new Links (page->getAnnots (&obj),
			   pdf_doc->getCatalog ()->getBaseURI ());
	obj.free ();
	return links;
}

static void
enter_cb (GPdfLinkCanvasItem *link_item, Link *link, Link **user_data)
{
	*user_data = link;
}

TEST_BEGIN (GPdfLinks, setup, tear_down)

TEST_NEW (empty_links)
{
	GPdfLinksCanvasLayer *links_layer;

	links_layer = GPDF_LINKS_CANVAS_LAYER (
		gnome_canvas_item_new (gnome_canvas_root (canvas),
				       GPDF_TYPE_LINKS_CANVAS_LAYER,
				       "links", NULL,
				       NULL));
	TEST (gpdf_links_canvas_layer_get_num_links (links_layer) == 0);
}

TEST_NEW (links_layer_one_link)
{
	PDFDoc *pdf_doc;
	Links *links;
	GPdfLinksCanvasLayer *links_layer;

	pdf_doc = new PDFDoc (
		new GString (TESTFILES_DIR "/simple-links.pdf"));
	links = pdf_doc_links_for_page (pdf_doc, 1);
	TEST (links->getNumLinks () == 1);

	links_layer = GPDF_LINKS_CANVAS_LAYER (
		gnome_canvas_item_new (gnome_canvas_root (canvas),
				       GPDF_TYPE_LINKS_CANVAS_LAYER,
				       "links", links,
				       NULL));
	TEST (gpdf_links_canvas_layer_get_num_links (links_layer) 
	      == links->getNumLinks ());

	delete pdf_doc;
}

TEST_NEW (link_appearance)
{
	PDFDoc *pdf_doc;
	Links *links;
	Link *link;
	GnomeCanvasItem *link_item;
	double x1, x2, y1, y2, ex1, ex2, ey1, ey2/* , ew */;
/* 	int old_w, w; */

	pdf_doc = new PDFDoc (
		new GString (TESTFILES_DIR "/simple-links.pdf"));
	links = pdf_doc_links_for_page (pdf_doc, 1);
	TEST (links->getNumLinks () == 1);

	link = links->getLink (0);
	link_item = gnome_canvas_item_new (gnome_canvas_root (canvas),
					   GPDF_TYPE_LINK_CANVAS_ITEM,
					   "link", link,
					   NULL);

	link->getRect (&ex1, &ey1, &ex2, &ey2);

	g_object_get (link_item, "x1", &x1, NULL);
	g_object_get (link_item, "y1", &y1, NULL);
	g_object_get (link_item, "x2", &x2, NULL);
	g_object_get (link_item, "y2", &y2, NULL);

	TEST (x1==ex1 && y1==ey1 && x2==ex2 && y2==ey2);

/* 	g_object_get (link_item, "width_pixels", &old_w, NULL); */
/* 	g_object_set (link_item, "width_units", ew, NULL); */
/* 	g_object_get (link_item, "width_pixels", &w, NULL); */
/* 	TEST (old_w==w); */

	delete pdf_doc;
}

TEST_NEW (link_item_enter_leave)
{
	PDFDoc *pdf_doc;
	Links *links;
	Link *link;
	Link *received_link;
	GPdfLinkCanvasItem *link_item;
	gboolean using_hand_cursor;

	pdf_doc = new PDFDoc (
		new GString (TESTFILES_DIR "/simple-links.pdf"));
	links = pdf_doc_links_for_page (pdf_doc, 1);

	link = links->getLink (0);
	link_item = GPDF_LINK_CANVAS_ITEM (
		gnome_canvas_item_new (gnome_canvas_root (canvas),
				       GPDF_TYPE_LINK_CANVAS_ITEM,
				       "link", link,
				       NULL));

	using_hand_cursor = TRUE;
	g_object_get (G_OBJECT (link_item),
		      "using_hand_cursor", &using_hand_cursor, NULL);
	TEST (using_hand_cursor == FALSE);

	g_signal_connect (G_OBJECT (link_item), "enter",
			  G_CALLBACK (enter_cb), &received_link);
	gpdf_link_canvas_item_mouse_enter (link_item);
	TEST (link == received_link);

	using_hand_cursor = FALSE;
	g_object_get (G_OBJECT (link_item),
		      "using_hand_cursor", &using_hand_cursor, NULL);
	TEST (using_hand_cursor == TRUE);

	received_link = NULL;
	g_signal_connect (G_OBJECT (link_item), "leave",
			  G_CALLBACK (enter_cb), &received_link);
	gpdf_link_canvas_item_mouse_leave (link_item);
	TEST (link == received_link);

	using_hand_cursor = TRUE;
	g_object_get (G_OBJECT (link_item),
		      "using_hand_cursor", &using_hand_cursor, NULL);
	TEST (using_hand_cursor == FALSE);

	delete links;
}

TEST_NEW (link_item_click)
{
	PDFDoc *pdf_doc;
	Links *links;
	Link *link;
	Link *received_link;
	GPdfLinkCanvasItem *link_item;
	gboolean using_hand_cursor;

	pdf_doc = new PDFDoc (
		new GString (TESTFILES_DIR "/simple-links.pdf"));
	links = pdf_doc_links_for_page (pdf_doc, 1);
	link = links->getLink (0);

	link_item = GPDF_LINK_CANVAS_ITEM (
		gnome_canvas_item_new (gnome_canvas_root (canvas),
				       GPDF_TYPE_LINK_CANVAS_ITEM,
				       "link", link,
				       NULL));

	using_hand_cursor = FALSE;
	gpdf_link_canvas_item_mouse_enter (link_item);
	g_object_get (G_OBJECT (link_item),
		      "using_hand_cursor", &using_hand_cursor, NULL);
	TEST (using_hand_cursor == TRUE);

	g_signal_connect (G_OBJECT (link_item), "clicked",
			  G_CALLBACK (enter_cb), &received_link);
	gpdf_link_canvas_item_click (link_item);
	TEST (link == received_link);

	g_object_get (G_OBJECT (link_item),
		      "using_hand_cursor", &using_hand_cursor, NULL);
	TEST (using_hand_cursor == FALSE);

	delete links;
}

TEST_END ()

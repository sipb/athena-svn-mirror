/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * test-define-views.c - Tests define views dialog.
 * Copyright 2000, 2001, Ximian, Inc.
 *
 * Authors:
 *   Chris Lahey <clahey@ximian.com>
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
#include <stdio.h>
#include <gnome.h>
#include <gal/menus/gal-define-views-dialog.h>
#include <gal/menus/gal-view-collection.h>
#include <gal/menus/gal-view-factory-etable.h>

/* ETable creation */
#define SPEC "<ETableSpecification cursor-mode=\"line\" draw-grid=\"true\">" \
	     "<ETableColumn model_col= \"0\" _title=\"Name\" expansion=\"1.0\" minimum_width=\"18\" resizable=\"true\" cell=\"string\" compare=\"string\"/>" \
             "<ETableState> <column source=\"0\"/> <grouping> </grouping> </ETableState>" \
	     "</ETableSpecification>"

static void
dialog_clicked (GnomeCanvas *canvas,
		int button,
		GalViewCollection *collection)
{
	if (button == 0) {
		gal_view_collection_save(collection);
	}
	gtk_main_quit();
}

int
main(int argc, char *argv[])
{
	GalViewCollection *collection;
	GalViewFactory *factory;
	ETableSpecification *spec;
	GtkWidget *dialog;

	gnome_init ("DefineViewsExample", "DefineViewsExample", argc, argv);

	glade_gnome_init();

	gtk_widget_push_visual (gdk_rgb_get_visual ());
	gtk_widget_push_colormap (gdk_rgb_get_cmap ());

	spec = e_table_specification_new();
	e_table_specification_load_from_string(spec, SPEC);
	collection = gal_view_collection_new();
	factory = gal_view_factory_etable_new(spec);
	/* This leaks memory, but it's a test, so I'm not fixing it. */
	gal_view_collection_set_storage_directories(collection,
						    gnome_util_prepend_user_home("/evolution/system/"),
						    gnome_util_prepend_user_home("/evolution/galview/"));
	gal_view_collection_add_factory(collection,
					factory);

	gal_view_collection_load(collection);

	dialog = gal_define_views_dialog_new(collection);

	gtk_signal_connect(GTK_OBJECT(dialog), "clicked",
			   dialog_clicked, collection);

	gtk_widget_show(dialog);

	gtk_main ();

	return 0;
}

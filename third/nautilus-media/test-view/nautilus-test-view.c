/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* 
 * Copyright (C) 2000 Eazel, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Maciej Stachowiak <mjs@eazel.com>
 */

/* nautilus-test-view.c - test test view component
   This component displays a simple label of the URI
   and demonstrates merging menu items & toolbar buttons. 
   It should be a good basis for writing out-of-proc content views.
 */

/* WHAT YOU NEED TO CHANGE: You need to rename everything. Then look
 * for the individual CHANGE comments to see some things you could
 * change to make your view do what you want.  
 */

#include <config.h>

#include <bonobo/bonobo-i18n.h>
#include <gtk/gtklabel.h>
#include <libgnome/gnome-macros.h>
#include <string.h>
#include "nautilus-test-view.h"

/* CHANGE: You probably want some different widget than a label to be
 * your main view contents.  
 */
struct NautilusTestViewDetails {
	char *location;
	GtkWidget *label;
};

BONOBO_CLASS_BOILERPLATE (NautilusTestView, nautilus_test_view,
			  NautilusView, NAUTILUS_TYPE_VIEW)

static void
nautilus_test_view_finalize (GObject *object)
{
	NautilusTestView *view;

	view = NAUTILUS_TEST_VIEW (object);

	g_print ("DEBUG: finalize: clearing all memory we hold\n");
	g_free (view->details->location);
	g_free (view->details);

	G_OBJECT_CLASS (object)->finalize (object);
}

static void
load_location (NautilusTestView *view,
	       const char *location)
{
	char *label_text;

	g_assert (NAUTILUS_IS_TEST_VIEW (view));
	g_assert (location != NULL);
	
	g_free (view->details->location);
	view->details->location = g_strdup (location);

	label_text = g_strdup_printf (_("%s\n\nThis is a test Nautilus content view component."), location);
	gtk_label_set_text (GTK_LABEL (view->details->label), label_text);
	g_free (label_text);
}

static void
test_destroy_callback (NautilusView *nautilus_view,
		       gpointer user_data)
{
	g_print ("The view has been destroyed.\n");
}

/* CHANGE: Do your own loading here. If loading can be a long-running
 * operation, you should consider doing it async, in which case you
 * should only call load_complete when the load is actually done.
 */

static void
test_load_location_callback (NautilusView *nautilus_view, 
			       const char *location,
			       gpointer user_data)
{
	NautilusTestView *view;
	
	g_assert (NAUTILUS_IS_VIEW (nautilus_view));
	g_assert (location != NULL);
	
	view = NAUTILUS_TEST_VIEW (nautilus_view);
	
	/* It's mandatory to send an underway message once the
	 * component starts loading, otherwise nautilus will assume it
	 * failed. In a real component, this will probably happen in
	 * some sort of callback from whatever loading mechanism it is
	 * using to load the data; this component loads no data, so it
	 * gives the progress update here.
	 */
	nautilus_view_report_load_underway (nautilus_view);
	
	/* Do the actual load. */
	load_location (view, location);
	
	/* It's mandatory to call report_load_complete once the
	 * component is done loading successfully, or
	 * report_load_failed if it completes unsuccessfully. In a
	 * real component, this will probably happen in some sort of
	 * callback from whatever loading mechanism it is using to
	 * load the data; this component loads no data, so it gives
	 * the progress update here.
	 */
	nautilus_view_report_load_complete (nautilus_view);
}

static void
bonobo_test_callback (BonoboUIComponent *ui, 
			gpointer           user_data, 
			const char        *verb)
{
 	NautilusTestView *view;
	char *label_text;

	g_assert (BONOBO_IS_UI_COMPONENT (ui));
        g_assert (verb != NULL);

	view = NAUTILUS_TEST_VIEW (user_data);

	if (strcmp (verb, "Test Menu Item") == 0) {
		label_text = g_strdup_printf (_("%s\n\nYou selected the Test menu item."),
					      view->details->location);
	} else {
		g_assert (strcmp (verb, "Test Dock Item") == 0);
		label_text = g_strdup_printf (_("%s\n\nYou clicked the Test toolbar button."),
					      view->details->location);
	}
	
	gtk_label_set_text (GTK_LABEL (view->details->label), label_text);
	g_free (label_text);
}

/* CHANGE: Do your own menu/toolbar merging here. */
static void
test_merge_bonobo_items_callback (BonoboControl *control, 
				    gboolean       state, 
				    gpointer       user_data)
{
 	NautilusTestView *view;
	BonoboUIComponent *ui_component;
	BonoboUIVerb verbs [] = {
		BONOBO_UI_VERB ("Test Menu Item", bonobo_test_callback),
		BONOBO_UI_VERB ("Test Dock Item", bonobo_test_callback),
		BONOBO_UI_VERB_END
	};

	g_assert (BONOBO_IS_CONTROL (control));
	
	view = NAUTILUS_TEST_VIEW (user_data);

	if (state) {
		ui_component = nautilus_view_set_up_ui (NAUTILUS_VIEW (view),
							DATADIR,
							"nautilus-test-view-ui.xml",
							"nautilus-test-view");

		bonobo_ui_component_add_verb_list_with_data (ui_component, verbs, view);
	}

        /* Note that we do nothing if state is FALSE. Nautilus content
         * views are activated when installed, but never explicitly
         * deactivated. When the view changes to another, the content
         * view object is destroyed, which ends up calling
         * bonobo_ui_handler_unset_container, which removes its merged
         * menu & toolbar items.
	 */
}

static void
nautilus_test_view_class_init (NautilusTestViewClass *class)
{
	g_print ("DEBUG: class_init: setting handlers\n");
	G_OBJECT_CLASS (class)->finalize = nautilus_test_view_finalize;
}

static void
nautilus_test_view_instance_init (NautilusTestView *view)
{
	g_print ("DEBUG: instance_init\n");
	view->details = g_new0 (NautilusTestViewDetails, 1);
	view->details->label = gtk_label_new (_("(none)"));
	gtk_widget_show (view->details->label);
	nautilus_view_construct (NAUTILUS_VIEW (view), view->details->label);
	g_signal_connect (view, "load_location",
			  G_CALLBACK (test_load_location_callback), NULL);
	g_signal_connect (view, "destroy",
			  G_CALLBACK (test_destroy_callback), NULL);

	/* Get notified when our bonobo control is activated so we can
	 * merge menu & toolbar items into the shell's UI.
	 */
	/*
        g_signal_connect_object (nautilus_view_get_bonobo_control (NAUTILUS_VIEW (view)), "activate",
				 G_CALLBACK (test_merge_bonobo_items_callback), view, 0);
				 */
}

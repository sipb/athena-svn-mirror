/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* 
 * Copyright (C) 2002 Thomas Vander Stichele
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more av.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author: Thomas Vander Stichele <thomas at apestaart dot org>
 */

/* nautilus-audio-view.c - audio view component
   component. This component displays a simple label of the URI
   and demonstrates merging menu items & toolbar buttons.
   It should be a good basis for writing out-of-proc content views.
 */

/* WHAT YOU NEED TO CHANGE: You need to rename everything. Then look
 * for the individual CHANGE comments to see some things you could
 * change to make your view do what you want.
 */

#include <config.h>

#include <bonobo/bonobo-i18n.h>

#include <gtk/gtk.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkliststore.h>
#include <gtk/gtkcellrenderertext.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtktreeview.h>

#include <gtk/gtkstock.h>		/* stock icons */

#include <libgnomevfs/gnome-vfs-directory.h>
#include <libgnome/gnome-macros.h>

#include <gst/gconf/gconf.h>

#include <string.h>

#include "../media-info/media-info.h"
#include "audio-view.h"
#include "nautilus-audio-view.h"

BONOBO_CLASS_BOILERPLATE (NautilusAudioView, nautilus_audio_view,
			  NautilusView, NAUTILUS_TYPE_VIEW)

struct NautilusAudioViewDetails {
	AudioView *audio_view;
	char *location;
};

/* CHANGE: Do your own loading here. If loading can be a long-running
 * operation, you should consider doing it async, in which case you
 * should only call load_complete when the load is actually done.
 */

static void
sample_load_location_callback (NautilusView *nautilus_view,
			       const char *location,
			       gpointer user_data)
{
	NautilusAudioView *view;

	g_assert (NAUTILUS_IS_VIEW (nautilus_view));
	g_assert (location != NULL);

	view = NAUTILUS_AUDIO_VIEW (nautilus_view);

	/* It's mandatory to send an underway message once the
	 * component starts loading, otherwise nautilus will assume it
	 * failed. In a real component, this will probably happen in
	 * some sort of callback from whatever loading mechanism it is
	 * using to load the data; this component loads no data, so it
	 * gives the progress update here.
	 */
	nautilus_view_report_load_underway (nautilus_view);

	/* Do the actual load. */
	g_print ("DEBUG: doing load on location %s\n", location);
	audio_view_load_location (view->details->audio_view, location);

	/* It's mandatory to call report_load_complete once the
	 * component is done loading successfully, or
	 * report_load_failed if it completes unsuccessfully. In a
	 * real component, this will probably happen in some sort of
	 * callback from whatever loading mechanism it is using to
	 * load the data; this component loads no data, so it gives
	 * the progress update here.
	 */
	/* FIXME: maybe we really should do this right ? */
	nautilus_view_report_load_complete (nautilus_view);
}
/* destroy callback */
static void
nautilus_audio_view_callback_destroy (GObject *object, gpointer data)
{
	g_print ("The audio view got destroyed\n");
}

/* class functions */
static void nautilus_audio_view_finalize (GObject *object);
static void nautilus_audio_view_dispose (GObject *object);

/* initialize the class */
static void
nautilus_audio_view_class_init (NautilusAudioViewClass *class)
{
	parent_class = g_type_class_peek_parent (class);
	
	G_OBJECT_CLASS (class)->finalize = nautilus_audio_view_finalize;
	G_OBJECT_CLASS (class)->dispose = nautilus_audio_view_dispose;
	g_print ("DEBUG: class_init\n");
}

static void
nautilus_audio_view_instance_init (NautilusAudioView *view)
{
	GError *error = NULL;
	GstElement *audio_sink;
	GtkWidget *image;

	view->details = g_new0 (NautilusAudioViewDetails, 1);

	/* create the ui */
	view->details->audio_view = audio_view_new ();
	if (view->details->audio_view == NULL)
	{
		/* FIXME: we get a ui warning, but can we do more ? */
		g_free (view->details);
		view->details = NULL;
		return;
	}

	/* throw the ui widget at the nautilus view */
	nautilus_view_construct (NAUTILUS_VIEW (view),
			         audio_view_get_widget (view->details->audio_view));
	g_signal_connect (view, "load_location",
			  G_CALLBACK (sample_load_location_callback), NULL);

	g_signal_connect (view, "destroy",
			  G_CALLBACK (nautilus_audio_view_callback_destroy),
			  NULL);


	gtk_widget_show_all (audio_view_get_widget (view->details->audio_view));

	/* Get notified when our bonobo control is activated so we can
	 * merge menu & toolbar items into the shell's UI.
	 */
#ifdef DONTDEF
        g_signal_connect_object (
		nautilus_view_get_bonobo_control (NAUTILUS_VIEW (view)),
		"activate",
		G_CALLBACK (sample_merge_bonobo_items_callback), view, 0);
#endif
}

static void
nautilus_audio_view_dispose (GObject *object)
{
        NautilusAudioView *view;

        view = NAUTILUS_AUDIO_VIEW (object);

	g_print ("DEBUG: nav_dispose\n");
	audio_view_dispose (view->details->audio_view);

	GNOME_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
nautilus_audio_view_finalize (GObject *object)
{
        NautilusAudioView *view;

        view = NAUTILUS_AUDIO_VIEW (object);

	g_print ("DEBUG: nav_finalize\n");

	audio_view_finalize (view->details->audio_view);
	g_free (view->details->audio_view);
	
	g_free (view->details->location);
	g_free (view->details);

	/* FIXME: if I keep in this finalizer, then stuff goes horribly wrong
	 *        when the view never got created properly.  So I removed it.
	 *        My guess is that this makes it very leaky though. */

	GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}


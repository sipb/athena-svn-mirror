#define _EBROWSER_C_

/*  This file is part of the GtkHTML library.

    Copyright (C) 2000 Helix Code, Inc.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    Author: Lauris Kaplinski  <lauris@helixcode.com>
*/
#include <config.h>
#include <gnome.h>
#include <liboaf/liboaf.h>
#include <bonobo.h>
#include "ebrowser-widget.h"
#include "ebrowser-stream.h"
#include "ebrowser.h"
#include "ebrowser-ui.h"
#ifdef GTKHTML_HAVE_GCONF
#include <gconf/gconf-client.h>
#endif

static gint refcount = 0;

enum {
	ARG_0,
	ARG_URL,
	ARG_HTTP_PROXY,
	ARG_FOLLOW_LINKS,
	ARG_FOLLOW_REDIRECT,
	ARG_ALLOW_SUBMIT,
	ARG_DEFAULT_BGCOLOR,
	ARG_DEFAULT_FONT,
	ARG_HISTORY_SIZE
};

static void
get_prop (BonoboPropertyBag * bag, BonoboArg * arg, guint arg_id, 
	  CORBA_Environment *ev, gpointer data)
{
	EBrowser * ebr;

	ebr = EBROWSER (data);

	switch (arg_id) {
	case ARG_URL:
		BONOBO_ARG_SET_STRING (arg, ebr->url);
		if (ebr->history)
			ebrowser_history_push (ebr->history, ebr->url);
		break;
	case ARG_HTTP_PROXY:
		BONOBO_ARG_SET_STRING (arg, ebr->http_proxy);
		break;
	case ARG_FOLLOW_LINKS:
		BONOBO_ARG_SET_BOOLEAN (arg, ebr->followlinks);
		break;
	case ARG_FOLLOW_REDIRECT:
		BONOBO_ARG_SET_BOOLEAN (arg, ebr->followredirect);
		break;
	case ARG_ALLOW_SUBMIT:
		BONOBO_ARG_SET_BOOLEAN (arg, ebr->allowsubmit);
		break;
	case ARG_DEFAULT_BGCOLOR:
		BONOBO_ARG_SET_INT (arg, ebr->defaultbgcolor);
		break;
	case ARG_DEFAULT_FONT:
		BONOBO_ARG_SET_STRING (arg, ebr->defaultfont);
		break;
	case ARG_HISTORY_SIZE:
		if (ebr->history)
			BONOBO_ARG_SET_INT (arg, ebr->history->max_size);
		else
			BONOBO_ARG_SET_INT (arg, 0);
		break;
	default:
		break;
	}
	g_print ("arg %d queried\n", arg_id);
}

static void
set_prop (BonoboPropertyBag * bag, const BonoboArg * arg, guint arg_id, 
	  CORBA_Environment *ev, gpointer browser)
{
	switch (arg_id) {
	case ARG_URL:
		gtk_object_set (GTK_OBJECT (browser), "url", BONOBO_ARG_GET_STRING (arg), NULL);
		break;
	case ARG_HTTP_PROXY:
		gtk_object_set (GTK_OBJECT (browser), "http_proxy", BONOBO_ARG_GET_STRING (arg), NULL);
		break;
	case ARG_FOLLOW_LINKS:
		gtk_object_set (GTK_OBJECT (browser), "follow_links", BONOBO_ARG_GET_BOOLEAN (arg), NULL);
		break;
	case ARG_FOLLOW_REDIRECT:
		gtk_object_set (GTK_OBJECT (browser), "follow_redirect", BONOBO_ARG_GET_BOOLEAN (arg), NULL);
		break;
	case ARG_ALLOW_SUBMIT:
		gtk_object_set (GTK_OBJECT (browser), "allow_submit", BONOBO_ARG_GET_BOOLEAN (arg), NULL);
		break;
	case ARG_DEFAULT_BGCOLOR:
		gtk_object_set (GTK_OBJECT (browser), "default_bgcolor", BONOBO_ARG_GET_INT (arg), NULL);
		break;
	case ARG_DEFAULT_FONT:
		gtk_object_set (GTK_OBJECT (browser), "default_font", BONOBO_ARG_GET_STRING (arg), NULL);
		break;
	case ARG_HISTORY_SIZE:
		gtk_object_set (GTK_OBJECT (browser), "history_size", BONOBO_ARG_GET_INT (arg), NULL);
	default:
		g_print ("Arg %d set\n", arg_id);
		break;
	}
}

static void
browser_url_set (EBrowser * ebr, const gchar * url, gpointer data)
{
	BonoboPropertyBag * pb;
	BonoboArg * arg;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	pb = (BonoboPropertyBag *) data;

	arg = bonobo_arg_new (BONOBO_ARG_STRING);
	BONOBO_ARG_SET_STRING (arg, url);

	bonobo_property_bag_notify_listeners (pb, "url", arg, &ev);

	if (BONOBO_EX (&ev)) {
		g_warning ("Notify listeners exception: %s\n", bonobo_exception_get_text (&ev));
	}

	bonobo_arg_release (arg);

	CORBA_exception_free (&ev);
}

static void
browser_status_set (EBrowser * ebr, const gchar * status, gpointer data)
{
	BonoboControl * control;
	BonoboUIComponent * component;

	control = BONOBO_CONTROL (data);
	component = bonobo_control_get_ui_component (control);
	g_assert (component != NULL);

	bonobo_ui_component_set_status (component, status, NULL);
}

static void
control_destroy_cb (BonoboControl * control, gpointer data)
{
	bonobo_object_unref (BONOBO_OBJECT (data));
	if (--refcount < 1)
		gtk_main_quit ();
}

static BonoboObject *
ebrowser_factory (BonoboGenericFactory * factory, void * closure)
{
	BonoboControl * control;
	BonoboPersistStream * stream;
	BonoboPropertyBag * pbag;
	GtkWidget * w, * browser;

	w = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (w),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);
	gtk_widget_show (w);
	
	browser = ebrowser_new ();
	gtk_container_add (GTK_CONTAINER (w), browser);
	gtk_widget_show (browser);

	control = bonobo_control_new (w);

	bonobo_control_set_automerge (control, TRUE);

	if (!control) {
		gtk_object_destroy (GTK_OBJECT (w));
		return NULL;
	}

	stream = bonobo_persist_stream_new (ebrowser_ps_load, NULL, NULL, ebrowser_ps_types, browser);
	bonobo_object_add_interface (BONOBO_OBJECT (control), BONOBO_OBJECT (stream));

	pbag = bonobo_property_bag_new (get_prop, set_prop, browser);
	bonobo_control_set_properties (control, pbag);

	bonobo_property_bag_add (pbag, "url", ARG_URL, BONOBO_ARG_STRING, NULL,
				 "Main URL to display", 0);
	bonobo_property_bag_add (pbag, "http_proxy", ARG_HTTP_PROXY, BONOBO_ARG_STRING, NULL,
				 "HTTP proxy", 0);
	bonobo_property_bag_add (pbag, "follow_links", ARG_FOLLOW_LINKS, BONOBO_ARG_BOOLEAN, NULL,
				 "Whether to follow HTML links", 0);
	bonobo_property_bag_add (pbag, "follow_redirect", ARG_FOLLOW_REDIRECT, BONOBO_ARG_BOOLEAN, NULL,
				 "Whether to follow HTML redirect", 0);
	bonobo_property_bag_add (pbag, "allow_submit", ARG_ALLOW_SUBMIT, BONOBO_ARG_BOOLEAN, NULL,
				 "Whether to send HTML FORM data", 0);
	bonobo_property_bag_add (pbag, "default_bgcolor", ARG_DEFAULT_BGCOLOR, BONOBO_ARG_INT, NULL,
				 "Whether to send HTML FORM data", 0);
	bonobo_property_bag_add (pbag, "default_font", ARG_DEFAULT_FONT, BONOBO_ARG_STRING, NULL,
				 "Whether to send HTML FORM data", 0);

	bonobo_object_add_interface (BONOBO_OBJECT (control), 
				     BONOBO_OBJECT (pbag));

	gtk_signal_connect (GTK_OBJECT (browser), "url_set",
			    GTK_SIGNAL_FUNC (browser_url_set), pbag);
	gtk_signal_connect (GTK_OBJECT (browser), "status_set",
			    GTK_SIGNAL_FUNC (browser_status_set), control);

	gtk_signal_connect (GTK_OBJECT (control), "activate",
			    ebrowser_control_activate_cb, browser);

	gtk_signal_connect (GTK_OBJECT (w), "destroy",
			    control_destroy_cb, control);

	refcount++;

	return BONOBO_OBJECT (control);
}

void
ebrowser_factory_init (void)
{
	static BonoboGenericFactory * ebfact = NULL;

	if (!ebfact) {
		ebfact = bonobo_generic_factory_new (EBROWSER_FACTORY_OAFIID, ebrowser_factory, NULL);
		if (!ebfact)
			g_error ("Cannot create ebrowser factory");
	}
}

int main (int argc, gchar ** argv)
{
#ifdef GTKHTML_HAVE_GCONF
	GError  *gconf_error  = NULL;
#endif
	CORBA_Environment ev;
	CORBA_ORB orb;

	CORBA_exception_init (&ev);

	gnome_init_with_popt_table ("EBrowser", VERSION,
				    argc, argv,
				    oaf_popt_options, 0, NULL);

	orb = oaf_init (argc, argv);

	if (bonobo_init (orb, NULL, NULL) == FALSE)
		g_error ("Couldn't initialize Bonobo");

#ifdef GTKHTML_HAVE_GCONF
	if (!gconf_init (argc, argv, &gconf_error)) {
		g_assert (gconf_error != NULL);
		g_error ("GConf init failed:\n  %s", gconf_error->message);
		return 1;
	}
#endif

	gdk_rgb_init ();
	gtk_widget_set_default_colormap (gdk_rgb_get_cmap ());
	gtk_widget_set_default_visual (gdk_rgb_get_visual ());

	ebrowser_factory_init ();

	bonobo_main ();

	return 0;
}

#define _TEST_EBROWSER_C_

/*
 * Copyright 2000 Helix Code, Inc.
 *
 * Author: Lauris Kaplinski  <lauris@helixcode.com>
 *
 * License: GPL
 */

#include <gnome.h>
#include <liboaf/liboaf.h>
#include <bonobo.h>
#include <ebrowser.h>
#include "test-ebrowser.h"

static void
destroy_browser (GtkObject * object, gpointer data)
{
	BonoboUIContainer * uic;

	g_print ("destroy\n");

	uic = BONOBO_UI_CONTAINER (data);
	bonobo_object_unref (BONOBO_OBJECT (uic));
}

static void
url_activate (GtkWidget * widget, gpointer data)
{
	GtkEntry * entry;
	BonoboWidget * w;
	gchar * str;

	entry = gtk_object_get_data (GTK_OBJECT (data), "url");
	w = gtk_object_get_data (GTK_OBJECT (data), "control");

	str = gtk_entry_get_text (entry);

	bonobo_widget_set_property (w, "url", str, NULL);
}

static void
http_proxy_activate (GtkWidget * widget, gpointer data)
{
	GtkEntry * entry;
	BonoboWidget * w;
	gchar * str;

	entry = gtk_object_get_data (GTK_OBJECT (data), "http_proxy");
	w = gtk_object_get_data (GTK_OBJECT (data), "control");

	str = gtk_entry_get_text (entry);

	bonobo_widget_set_property (w, "http_proxy", str, NULL);
}

static void
activate_toggled (GtkToggleButton * tb, gpointer data)
{
	BonoboControlFrame * cf;
	BonoboWidget * w;

	w = gtk_object_get_data (GTK_OBJECT (data), "control");

	cf = bonobo_widget_get_control_frame (BONOBO_WIDGET (w));

	if (gtk_toggle_button_get_active (tb)) {
		bonobo_control_frame_control_activate (cf);
	} else {
		bonobo_control_frame_control_deactivate (cf);
	}
}

static void
property_toggled (GtkToggleButton * tb, gpointer data)
{
	BonoboControlFrame * cf;
	BonoboWidget * w;

	w = gtk_object_get_data (GTK_OBJECT (tb), "control");

	cf = bonobo_widget_get_control_frame (BONOBO_WIDGET (w));

	if (gtk_toggle_button_get_active (tb)) {
		bonobo_widget_set_property (w, data, TRUE, NULL);
	} else {
		bonobo_widget_set_property (w, data, FALSE, NULL);
	}
}

static void 
prop_changed_cb (BonoboListener    *listener,
		 char              *event_name, 
		 CORBA_any         *arg,
		 CORBA_Environment *ev,
		 gpointer           user_data)
{
	GtkEntry * entry;

	entry = gtk_object_get_data (GTK_OBJECT (user_data), "url");

	gtk_entry_set_text (entry, BONOBO_ARG_GET_STRING (arg));
}

static void
load_file (GtkWidget * widget, gpointer data)
{
	CORBA_Object interface;
	BonoboObjectClient * object_client;
	const gchar * name;
	BonoboStream * stream;
	CORBA_Environment ev;

	name = gtk_entry_get_text (GTK_ENTRY (widget));

	object_client = bonobo_widget_get_server (BONOBO_WIDGET (data));

	interface = bonobo_object_client_query_interface (object_client, "IDL:Bonobo/PersistStream:1.0", NULL);
	if (interface == CORBA_OBJECT_NIL) {
		g_warning ("EBrowser does not give us PersistStream interface");
		return;
	}

	CORBA_exception_init (&ev);

	stream = bonobo_stream_open (BONOBO_IO_DRIVER_FS, name, Bonobo_Storage_READ, 0);

	if (stream == NULL) {
		g_warning ("Couldn't load %s", name);
	} else {
		BonoboObject * stream_object;
		Bonobo_Stream corba_stream;

		stream_object = BONOBO_OBJECT (stream);
		corba_stream = bonobo_object_corba_objref (stream_object);
		Bonobo_PersistStream_load (interface, corba_stream, "text/html", &ev);
	}

	Bonobo_Unknown_unref (interface, &ev);
	CORBA_Object_release (interface, &ev);

	CORBA_exception_free (&ev);
}

static void
stop_loading (GtkWidget * widget, gpointer data)
{
	g_print ("Stop loading\n");
}

static BonoboUIVerb verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("Stop", stop_loading),
	BONOBO_UI_VERB_END
};

static gchar * ui = 
"<Root>"
"  <commands>"
"    <cmd name=\"Stop\" _label=\"Stop\" _tip=\"Stop loading\" pixtype=\"stock\" pixname=\"Stop\"/>"
"  </commands>"
"  <menu>"
"    <submenu name=\"File\" _label=\"File\">"
"      <menuitem name=\"Stop\" verb=\"\"/>"
"    </submenu>"
"  </menu>"
"  <status>"
"    <item name=\"main\"/>"
"  </status>"
"</Root>";


static void
open_browser (GtkButton * button, gpointer data)
{
	BonoboUIComponent * component;
	BonoboUIContainer * container;
	BonoboControlFrame * cf;
	Bonobo_PropertyBag pb = CORBA_OBJECT_NIL;
	GtkWidget * bwin;
	GtkWidget * vb, * t, * entry, * hb, * w, * c;

	/* BonoboWindow */

	bwin = bonobo_window_new ("test-ebrowser", "Test EBrowser");
	gtk_window_set_default_size (GTK_WINDOW (bwin), 320, 240);

	/* UI Container */

	component = bonobo_ui_component_new ("test-ebrowser");
	container = bonobo_ui_container_new ();
	bonobo_ui_container_set_win (container, BONOBO_WINDOW (bwin));
	bonobo_ui_component_set_container (component, bonobo_object_corba_objref (BONOBO_OBJECT (container)));

#if 0
	bonobo_ui_component_add_verb_list_with_data (component, verbs, bwin);
#endif
#if 0
	bonobo_ui_component_set_translate (component, "/", ui, NULL);
#endif

	gtk_signal_connect (GTK_OBJECT (bwin), "destroy",
			    GTK_SIGNAL_FUNC (destroy_browser), container);

	vb = gtk_vbox_new (FALSE, 0);
	bonobo_window_set_contents (BONOBO_WINDOW (bwin), vb);

	/* Create URL entry */

	t = gtk_table_new (2, 2, FALSE);
	gtk_box_pack_start (GTK_BOX (vb), t, FALSE, FALSE, 0);
	gtk_widget_show (t);

	w = gtk_label_new ("Url:");
	gtk_table_attach (GTK_TABLE (t), w, 0, 1, 0, 1, 0, 0, 4, 2);
	gtk_widget_show (w);
	
	entry = gtk_entry_new ();
	gtk_object_set_data (GTK_OBJECT (bwin), "url", entry);
	gtk_signal_connect (GTK_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (url_activate), bwin);
	gtk_table_attach (GTK_TABLE (t), entry, 1, 2, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 4, 2);
	gtk_widget_show (entry);

	/* Create proxy entry */

	w = gtk_label_new ("Proxy:");
	gtk_table_attach (GTK_TABLE (t), w, 0, 1, 1, 2, 0, 0, 4, 2);
	gtk_widget_show (w);
	
	entry = gtk_entry_new ();
	gtk_object_set_data (GTK_OBJECT (bwin), "http_proxy", entry);
	gtk_signal_connect (GTK_OBJECT (entry), "activate", GTK_SIGNAL_FUNC (http_proxy_activate), bwin);
	gtk_table_attach (GTK_TABLE (t), entry, 1, 2, 1, 2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 4, 2);
	gtk_widget_show (entry);

	/* Activation toggle */

	w = gtk_toggle_button_new_with_label ("Activate");
	gtk_signal_connect (GTK_OBJECT (w), "toggled", GTK_SIGNAL_FUNC (activate_toggled), bwin);
	gtk_table_attach (GTK_TABLE (t), w, 2, 3, 0, 2, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 4, 2);
	gtk_widget_show (w);

	/* Create ebrowser */

#if 0
	corba_uic = bonobo_object_corba_objref (BONOBO_OBJECT (uic));
	gtk_object_set_data (GTK_OBJECT (bwin), "corba_uicontainer", corba_uic);
#endif

	c = bonobo_widget_new_control (EBROWSER_OAFIID, bonobo_object_corba_objref (BONOBO_OBJECT (container)));

	/* Property bar */

	hb = gtk_hbox_new (FALSE, 4);
	gtk_box_pack_start (GTK_BOX (vb), hb, FALSE, FALSE, 4);
	gtk_widget_show (hb);

	w = gtk_toggle_button_new_with_label ("follow links");
	gtk_object_set_data (GTK_OBJECT (w), "control", c);
	gtk_box_pack_start (GTK_BOX (hb), w, FALSE, FALSE, 4);
	gtk_signal_connect (GTK_OBJECT (w), "toggled", GTK_SIGNAL_FUNC (property_toggled), "follow_links");
	gtk_widget_show (w);

	w = gtk_toggle_button_new_with_label ("follow redirect");
	gtk_object_set_data (GTK_OBJECT (w), "control", c);
	gtk_box_pack_start (GTK_BOX (hb), w, FALSE, FALSE, 4);
	gtk_signal_connect (GTK_OBJECT (w), "toggled", GTK_SIGNAL_FUNC (property_toggled), "follow_redirect");
	gtk_widget_show (w);

	w = gtk_toggle_button_new_with_label ("allow submit");
	gtk_object_set_data (GTK_OBJECT (w), "control", c);
	gtk_box_pack_start (GTK_BOX (hb), w, FALSE, FALSE, 4);
	gtk_signal_connect (GTK_OBJECT (w), "toggled", GTK_SIGNAL_FUNC (property_toggled), "allow_submit");
	gtk_widget_show (w);

	/* Client side file picker */

	w = gnome_file_entry_new ("ebrowser", "Select client-side file");
	gtk_box_pack_start (GTK_BOX (hb), w, FALSE, FALSE, 4);
	gtk_widget_show (w);
	w = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (w));
	gtk_signal_connect (GTK_OBJECT (w), "activate", GTK_SIGNAL_FUNC (load_file), c);

	/* Pack ebrowser */

	gtk_box_pack_start (GTK_BOX (vb), c, TRUE, TRUE, 0);
	gtk_widget_show (c);

	/* fixme */
	gtk_object_set_data (GTK_OBJECT (bwin), "control", c);

	cf = bonobo_widget_get_control_frame (BONOBO_WIDGET (c));
	pb = bonobo_control_frame_get_control_property_bag (cf, NULL);

	bonobo_event_source_client_add_listener (pb, prop_changed_cb, 
						 "Bonobo/Property:change:url",
						 NULL, bwin);
	bonobo_object_release_unref (pb, NULL); 

	gtk_widget_show_all (bwin);
}

static gint
delete_main (GtkWidget w, GdkEventAny * event, gpointer data)
{
	gtk_main_quit ();

	return FALSE;
}

static void
create_window (void)
{
	GtkWidget * w, * b, * bt;

	w = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (w), "test-ebrowser");
	gtk_window_set_default_size (GTK_WINDOW (w), 200, 200);
	gtk_signal_connect (GTK_OBJECT (w), "delete_event",
			    GTK_SIGNAL_FUNC (delete_main), NULL);

	b = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (b);
	gtk_container_add (GTK_CONTAINER (w), b);

	bt = gtk_button_new_with_label ("Open browser");
	gtk_widget_show (bt);
	gtk_box_pack_start (GTK_BOX (b), bt, TRUE, TRUE, 0);
	gtk_signal_connect (GTK_OBJECT (bt), "clicked",
			    GTK_SIGNAL_FUNC (open_browser), NULL);

	gtk_widget_show (w);
}

int main (int argc, char ** argv)
{
	CORBA_Environment ev;
	CORBA_ORB orb;

	CORBA_exception_init (&ev);

	gnome_init_with_popt_table ("test-ebrowser", "0.0",
				    argc, argv,
				    oaf_popt_options, 0, NULL);

	orb = oaf_init (argc, argv);

	if (bonobo_init (orb, NULL, NULL) == FALSE) {
		g_error ("Couldn't initialize Bonobo");
	}

	gdk_rgb_init ();
	gtk_widget_set_default_colormap (gdk_rgb_get_cmap ());
	gtk_widget_set_default_visual (gdk_rgb_get_visual ());

	create_window ();

	bonobo_main ();

	return 0;
}

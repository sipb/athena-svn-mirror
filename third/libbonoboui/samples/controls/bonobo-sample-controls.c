/*
 * bonobo-clock-control.c
 *
 * Author:
 *    Michael Meeks (michael@ximian.com)
 *
 * Copyright 2001, Ximian, Inc.
 */

#include <config.h>
#include <string.h>
#include <gdk/gdkx.h>
#include <libbonoboui.h>
#include <libgnomecanvas/gnome-canvas-widget.h>
#undef USE_SCROLLED

static void
activate_cb (GtkEditable *editable, BonoboControl *control)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new (
		NULL, 0, GTK_MESSAGE_INFO,
		GTK_BUTTONS_OK,
		"This dialog demonstrates transient dialogs");

	bonobo_control_set_transient_for (
		control, GTK_WINDOW (dialog), NULL);

	gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);
}

BonoboObject *
bonobo_entry_control_new (void)
{
	BonoboPropertyBag  *pb;
	BonoboControl      *control;
	GtkWidget	   *entry;
	GParamSpec        **pspecs;
	guint               n_pspecs;
	GtkWidget          *box;
	int i;

	
	/* Create the control. */

	box = gtk_vbox_new (FALSE, 0);
	for (i = 0; i < 3; i++) {
		entry = gtk_entry_new ();
		gtk_box_pack_start (GTK_BOX (box), entry, FALSE, FALSE, 0);
		gtk_widget_show (entry);
	}

	gtk_widget_show (box);

#ifdef USE_SCROLLED
	{
		GtkWidget *canvas, *scrolled;
		GnomeCanvasItem *item;
		
		canvas = gnome_canvas_new ();
		gtk_widget_show (canvas);
		
		item = gnome_canvas_item_new (
			gnome_canvas_root (GNOME_CANVAS (canvas)),
			GNOME_TYPE_CANVAS_WIDGET,
			"x", 0.0, "y", 0.0, "width", 100.0,
			"height", 100.0, "widget", box, NULL);
		gnome_canvas_item_show (item);

		scrolled = gtk_scrolled_window_new (NULL, NULL);
		gtk_scrolled_window_set_policy (
			GTK_SCROLLED_WINDOW (scrolled),
			GTK_POLICY_AUTOMATIC,
			GTK_POLICY_AUTOMATIC);

		gtk_container_add (
			GTK_CONTAINER (scrolled), canvas);
		gtk_widget_show (scrolled);

		control = bonobo_control_new (scrolled);
	}
#else
	control = bonobo_control_new (box);
#endif
	pb = bonobo_property_bag_new (NULL, NULL, NULL);
	bonobo_control_set_properties (control, BONOBO_OBJREF (pb), NULL);
	bonobo_object_unref (BONOBO_OBJECT (pb));

	g_signal_connect (
		GTK_OBJECT (entry), "activate",
		G_CALLBACK (activate_cb), control);

	pspecs = g_object_class_list_properties (
		G_OBJECT_GET_CLASS (entry), &n_pspecs);

	bonobo_property_bag_map_params (
		pb, G_OBJECT (entry), (const GParamSpec **)pspecs, n_pspecs);

	g_free (pspecs);

	bonobo_control_life_instrument (control);

	return BONOBO_OBJECT (control);
}

static BonoboObject *
control_factory (BonoboGenericFactory *this,
		 const char           *object_id,
		 void                 *data)
{
	BonoboObject *object = NULL;
	
	g_return_val_if_fail (object_id != NULL, NULL);

	if (!strcmp (object_id, "OAFIID:Bonobo_Sample_Entry"))

		object = bonobo_entry_control_new ();

	return object;
}

int
main (int argc, char *argv [])
{
	int retval;
	char *iid;

	if (!bonobo_ui_init ("bonobo-sample-controls-2",
			     VERSION, &argc, argv))
		g_error (_("Could not initialize Bonobo UI"));

	iid = bonobo_activation_make_registration_id (
		"OAFIID:Bonobo_Sample_ControlFactory",
		DisplayString (gdk_display));

	retval = bonobo_generic_factory_main (iid, control_factory, NULL);

	g_free (iid);

	return retval;
}                                                                             

/*
 * config-moniker-demo.c: a small demo for the configuration moniker
 *
 * Author:
 *   Dietmar Maurer (dietmar@ximian.com)
 *
 * Copyright 2001 Ximian, Inc.
 */
#include <bonobo.h>
#include <bonobo-conf/bonobo-config-database.h>
#include <bonobo-conf/bonobo-preferences.h>
#include <bonobo-conf/bonobo-config-control.h>
#include <bonobo-conf/bonobo-property-editor.h>
#include <bonobo-conf/bonobo-property-frame.h>

/*
 * display a moniker as Bonobo/Control
 */
static void
display_as_control (const char *moniker, 
		    CORBA_Environment *ev)
{
	Bonobo_Control     control;
	GtkWidget         *widget;
	BonoboUIContainer *ui_container;
	GtkWidget         *window;

	control = bonobo_get_object (moniker, "IDL:Bonobo/Control:1.0", ev);
	if (BONOBO_EX (ev) || !control)
		g_error ("Couldn't get Bonobo/Control interface");

	window = bonobo_window_new ("config: moniker test", moniker);
	ui_container = bonobo_ui_container_new ();
	bonobo_ui_container_set_win (ui_container, BONOBO_WINDOW (window));

	gtk_window_set_default_size (GTK_WINDOW (window), 400, 350);

	widget = bonobo_widget_new_control_from_objref (control,
		BONOBO_OBJREF (ui_container));
	gtk_widget_show (widget);

	bonobo_object_unref (BONOBO_OBJECT (ui_container));

	bonobo_control_frame_control_activate (
		bonobo_widget_get_control_frame (BONOBO_WIDGET (widget)));

	bonobo_window_set_contents (BONOBO_WINDOW (window), widget);

	gtk_signal_connect (GTK_OBJECT (window), "destroy",
			    GTK_SIGNAL_FUNC (gtk_widget_destroy), NULL);

	gtk_widget_show_all (window);
}

/*
 * display a whole directory ("bonobo-conf-test")
 */
static void
demo2 ()
{
	CORBA_Environment ev;
	char *mname;

	CORBA_exception_init (&ev);

	mname = g_strconcat ("xmldb:" , g_get_current_dir (), 
			     "/../tests/test-config.xmldb", 
			     "#config:/bonobo-conf-test", NULL);
	
	display_as_control (mname, &ev); 
	g_assert (!BONOBO_EX (&ev));
}

			  
/*
 * manually create the layout of a property page
 */
static GtkWidget *
demo3_get_fn (BonoboConfigControl *control,
	      Bonobo_PropertyBag   pb,
	      gpointer             closure,
	      CORBA_Environment   *ev)
{
	GtkWidget *w, *table;
	BonoboPEditor *ed;

	table = gtk_table_new (3, 3, FALSE);

	w = gtk_label_new ("A normal integer value");
	gtk_table_attach_defaults (GTK_TABLE (table), w, 0, 1, 0, 1);

	ed = BONOBO_PEDITOR (bonobo_peditor_new (pb, "test-long2", TC_long, 
						 NULL));
	w = bonobo_peditor_get_widget (ed);
	gtk_table_attach_defaults (GTK_TABLE (table), w, 1, 2, 0, 1);

	w = gtk_label_new ("Enumeration Types");
	gtk_table_attach_defaults (GTK_TABLE (table), w, 0, 1, 1, 2);

	ed = BONOBO_PEDITOR (bonobo_peditor_new (pb, "test-enum", 
						 TC_Bonobo_StorageType, NULL));
	w = bonobo_peditor_get_widget (ed);
	gtk_table_attach_defaults (GTK_TABLE (table), w, 1, 2, 1, 2);

	w = gtk_label_new ("Filename");
	gtk_table_attach_defaults (GTK_TABLE (table), w, 0, 1, 2, 3);

	ed = BONOBO_PEDITOR (bonobo_peditor_new (pb, "test-filename", 
	        TC_Bonobo_Config_FileName, NULL));
	w = bonobo_peditor_get_widget (ed);
	gtk_table_attach_defaults (GTK_TABLE (table), w, 1, 2, 2, 3);

	gtk_widget_show_all (table);

	return table;
}

/*
 * create a preferences dialog
 */
static void
demo3 ()
{
	CORBA_Environment ev;
	BonoboConfigControl *config_control;
	GtkWidget *pref;
	Bonobo_PropertyBag pb;
	char *mname;

	CORBA_exception_init (&ev);

	mname = g_strconcat ("xmldb:" , g_get_current_dir (), 
			     "/../tests/test-config.xmldb", 
			     "#config:/bonobo-conf-test", NULL);

	pb = bonobo_get_object (mname, "Bonobo/PropertyBag", &ev);
	g_assert (!BONOBO_EX (&ev));

	config_control = bonobo_config_control_new (NULL);
	g_assert (config_control != NULL);

	bonobo_config_control_add_page (config_control, 
					"Manually created Page",
					pb, demo3_get_fn, NULL);
	bonobo_config_control_add_page (config_control, "Auto generated page1",
					pb, NULL, NULL);
	bonobo_config_control_add_page (config_control, "Auto generated page2",
					pb, NULL, NULL);

	pref = bonobo_preferences_new (BONOBO_OBJREF (config_control));

	bonobo_object_unref (BONOBO_OBJECT (config_control));

	gtk_widget_show (pref);
}

/*
 * just some tests for localized values
 */
static void
demo4 ()
{
	Bonobo_ConfigDatabase db;
	CORBA_Environment ev;
	CORBA_any *value;
	char *mname;

	CORBA_exception_init (&ev);

	mname = g_strconcat ("xmldb:" , g_get_current_dir (), 
			     "/../tests/test-config.xmldb", NULL);

	db = bonobo_get_object (mname, "Bonobo/ConfigDatabase", &ev);
	g_assert (!BONOBO_EX (&ev));

	value = Bonobo_ConfigDatabase_getValue 
		(db, "doc/bonobo-conf-test/test-string", "", &ev);

	g_assert (!BONOBO_EX (&ev));
	printf ("FOUND VALUE: %s\n", BONOBO_ARG_GET_STRING (value));

	value = Bonobo_ConfigDatabase_getValue 
		(db, "doc/bonobo-conf-test/test-string", "de", &ev);

	g_assert (!BONOBO_EX (&ev));
	printf ("FOUND VALUE: %s\n", BONOBO_ARG_GET_STRING (value));

	value = Bonobo_ConfigDatabase_getValue 
		(db, "doc/bonobo-conf-test/test-string", "xyz", &ev);

	g_assert (!BONOBO_EX (&ev));
	printf ("FOUND VALUE: %s\n", BONOBO_ARG_GET_STRING (value));

	mname = g_strconcat ("xmldb:" , g_get_current_dir (), 
			     "/../tests/test-config.xmldb", 
			     "#xmldb:/tmp/test.xmldb", 
			     "#config:", "/bonobo-conf-test", NULL);

	display_as_control (mname, &ev);
	g_assert (!BONOBO_EX (&ev));
}

/* a callback if you want to listen to changes immediately (demo5) */
static void
demo5_cb  (BonoboListener    *listener,
	   char              *event_name, 
	   CORBA_any         *any,
	   CORBA_Environment *ev,
	   gpointer           user_data)
{
	printf ("received property event: %s\n", event_name);
}

static void
demo5 ()
{
	GtkWidget *w, *d, *l, *v0, *v, *h, *pf, *f;
	BonoboPEditor *ed;
	CORBA_Environment ev;
	Bonobo_PropertyBag bag;
	char *titles[] = { "Tiled", "Centered", "Scaled (keep aspect)", 
			   "Scaled", NULL };
	char *mname;

	CORBA_exception_init (&ev);

	/* we create a dialog with apply/revert buttons */
	d = gnome_property_box_new ();

	/* the name of a PropertyBag */
 	mname = g_strconcat ("xmldb:" , g_get_current_dir (), 
			     "/../tests/test-config.xmldb", 
			     "#config:/demo5", NULL);
	

        /* we create one PropertyFrame for each bag - it connects
	 * automatically to the apply/revert buttons of the above created
	 * GnomePropertyBox, and handles apply/revert automatically.
	 */
 	pf = bonobo_property_frame_new (NULL, mname);
	bag = BONOBO_OBJREF (BONOBO_PROPERTY_FRAME (pf)->proxy);

	v0 = gtk_vbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (pf), v0);

	f = gtk_frame_new ("Wallpaper");
	gtk_box_pack_start_defaults (GTK_BOX (v0), f);

	v = gtk_vbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (f), v);

	ed = BONOBO_PEDITOR (bonobo_peditor_new (bag, "test-filename", 
	        TC_Bonobo_Config_FileName, NULL));
	w = bonobo_peditor_get_widget (ed);
	bonobo_peditor_set_guard (w, bag, "use-gnome");
	gtk_box_pack_start_defaults (GTK_BOX (v), w);

	ed = BONOBO_PEDITOR (bonobo_peditor_option_new (1, titles));
	w = bonobo_peditor_get_widget (ed);
	bonobo_peditor_set_property (BONOBO_PEDITOR (ed),
				     bag, "test-radio", TC_ulong, NULL);
	bonobo_peditor_set_guard (w, bag, "use-gnome");
	gtk_box_pack_start_defaults (GTK_BOX (v), w);
	
	/* we create a PEditor for long integers */
	ed = BONOBO_PEDITOR (bonobo_peditor_new (bag, "test-long2", TC_long, 
						 NULL));
	w = bonobo_peditor_get_widget (ed);
	bonobo_peditor_set_guard (w, bag, "use-gnome");
	gtk_box_pack_start_defaults (GTK_BOX (v), w);

	/* Color */

	f = gtk_frame_new ("Color");
	gtk_box_pack_start_defaults (GTK_BOX (v0), f);

	v = gtk_vbox_new (FALSE, 5);
	gtk_container_add (GTK_CONTAINER (f), v);

	/* an editor for the same property, just to show that it works */
	ed = BONOBO_PEDITOR (bonobo_peditor_new (bag, "test-long2", TC_long, 
						 NULL));
	w = bonobo_peditor_get_widget (ed);
	bonobo_peditor_set_guard (w, bag, "use-gnome");
	gtk_box_pack_start_defaults (GTK_BOX (v), w);

	h = gtk_hbox_new (FALSE, 5);
	gtk_box_pack_start_defaults (GTK_BOX (v), h);

	l = gtk_label_new ("Primary Color");
	gtk_box_pack_start_defaults (GTK_BOX (h), l);

	/* add a PEditor for the primary colors */

	ed = BONOBO_PEDITOR (bonobo_peditor_new (bag, "primary-color", 
	        TC_Bonobo_Config_Color, NULL));
	w = bonobo_peditor_get_widget (ed);
	bonobo_peditor_set_guard (w, bag, "use-gnome");

	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	h = gtk_hbox_new (FALSE, 5);
	gtk_box_pack_start_defaults (GTK_BOX (v), h);

	l = gtk_label_new ("Secondary Color");
	gtk_box_pack_start_defaults (GTK_BOX (h), l);


	/* add a PEditor for the secondary colors */

	ed = BONOBO_PEDITOR (bonobo_peditor_new (bag, "secondary-color", 
	        TC_Bonobo_Config_Color, NULL));
	w = bonobo_peditor_get_widget (ed);
	bonobo_peditor_set_guard (w, bag, "use-gnome");
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);


	h = gtk_hbox_new (FALSE, 5);
	gtk_box_pack_start_defaults (GTK_BOX (v0), h);

	l = gtk_label_new ("Use GNOME to set background");
	gtk_box_pack_start_defaults (GTK_BOX (h), l);

	/* a guard property */
	ed = BONOBO_PEDITOR (bonobo_peditor_new (bag, "use-gnome", TC_boolean,
						 NULL));
	w = bonobo_peditor_get_widget (ed);
	gtk_box_pack_start (GTK_BOX (h), w, 0, 0, 0);

	bonobo_event_source_client_add_listener (bag, demo5_cb, NULL, NULL, 
						 NULL);

	gtk_widget_show_all (pf);

	l = gtk_label_new ("Test");
	gnome_property_box_append_page (GNOME_PROPERTY_BOX (d), pf, l);

	gtk_widget_show (d);
}

int
main (int argc, char **argv)
{
	CORBA_Environment ev;
	GtkWidget *win, *b, *vbox;

	CORBA_exception_init (&ev);

	gnome_init ("moniker-test", "0.0", argc, argv);

	if ((oaf_init (argc, argv)) == NULL)
		g_error ("Cannot init oaf");

	if (bonobo_init (NULL, NULL, NULL) == FALSE)
		g_error ("Cannot init bonobo");
	

	win = gnome_app_new ("config-moniker-demo", 
			    "Bonobo Configuration System Demo");

	gtk_signal_connect (GTK_OBJECT (win), "delete_event",
			    GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
	
	gtk_widget_show (win);
	
	vbox = gtk_vbox_new (0, FALSE);

	b = gtk_button_new_with_label ("Demo 2");
	gtk_box_pack_start_defaults (GTK_BOX(vbox), b);
	gtk_signal_connect (GTK_OBJECT (b), "pressed",
			    GTK_SIGNAL_FUNC (demo2), NULL);

	b = gtk_button_new_with_label ("Demo 3");
	gtk_box_pack_start_defaults (GTK_BOX(vbox), b);
	gtk_signal_connect (GTK_OBJECT (b), "pressed",
			    GTK_SIGNAL_FUNC (demo3), NULL);

	b = gtk_button_new_with_label ("Demo 4");
	gtk_box_pack_start_defaults (GTK_BOX(vbox), b);
	gtk_signal_connect (GTK_OBJECT (b), "pressed",
			    GTK_SIGNAL_FUNC (demo4), NULL);

	b = gtk_button_new_with_label ("Demo 5");
	gtk_box_pack_start_defaults (GTK_BOX(vbox), b);
	gtk_signal_connect (GTK_OBJECT (b), "pressed",
			    GTK_SIGNAL_FUNC (demo5), NULL);

	gtk_widget_show_all (vbox);

	gnome_app_set_contents (GNOME_APP (win), vbox);

	bonobo_main ();

	return 0;
}



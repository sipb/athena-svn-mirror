/*
 * test-ui.c: A test application to hammer the Bonobo UI api.
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000, 2001 Ximian, Inc.
 */
#include <config.h>
#include <string.h>
#include <stdlib.h>
#include <libbonoboui.h>
#include <gconf/gconf-client.h>
#include <gdk/gdkkeysyms.h>

#include <bonobo/bonobo-ui-xml.h>
#include <bonobo/bonobo-ui-util.h>
#include <bonobo/bonobo-window.h>

#include <bonobo/bonobo-i18n.h>
#include <bonobo/bonobo-ui-main.h>
#include <bonobo/bonobo-ui-preferences.h>

BonoboUIComponent *global_component;

#define PRINT_PREF(spref, pref) \
	fprintf (stderr, "\t" spref " : %s\n", \
		 bonobo_ui_preferences_get_ ## pref () ? "True" : "False")

static void
dump_prefs (void)
{
	fprintf (stderr, "--- UI Preferences ---\n");

	fprintf (stderr, "Toolbar:\n");

	PRINT_PREF ("detachable", toolbar_detachable);

	fprintf (stderr, "Menus:\n");

	PRINT_PREF ("have icons", menus_have_icons);
	PRINT_PREF ("have tearoff", menus_have_tearoff);

	fprintf (stderr, "Menubar:\n");

	PRINT_PREF ("detachable", menubar_detachable);
}

#undef PRINT_PREF

static void
cb_do_quit (GtkWindow *window, gpointer dummy)
{
	bonobo_main_quit ();
}

#define bonobo_window_dump(w,msg) \
	bonobo_ui_engine_dump (bonobo_window_get_ui_engine (w), stderr, msg)

static void
cb_do_dump (GtkWindow *window, BonoboWindow *win)
{
	bonobo_window_dump (win, "on User input");
}

static void
cb_do_popup (GtkWindow *window, BonoboWindow *win)
{
	GtkWidget *menu;

	menu = gtk_menu_new ();

	bonobo_window_add_popup (win, GTK_MENU (menu), "/popups/MyStuff");

	gtk_widget_show (menu);
	gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, 3, 0);
}

static void
cb_do_hide_toolbar (GtkWindow *window, BonoboWindow *win)
{
	const char path [] = "/Toolbar";
	char *val;

	val = bonobo_ui_component_get_prop (global_component, path, "hidden", NULL);
	if (val && atoi (val))
		bonobo_ui_component_set_prop (global_component, path, "hidden", "0", NULL);
	else
		bonobo_ui_component_set_prop (global_component, path, "hidden", "1", NULL);
}

static void
cb_set_state (GtkEntry *state_entry, GtkEntry *path_entry)
{
	const char *path, *state;
	char *txt, *str;

	path = gtk_entry_get_text (path_entry);
	state = gtk_entry_get_text (state_entry);

	g_warning ("Set state on '%s' to '%s'", path, state);

	bonobo_ui_component_set_prop (
		global_component, path, "state", state, NULL);

	txt = bonobo_ui_component_get_prop (
		global_component, path, "state", NULL);

	g_warning ("Re-fetched state was '%s'", txt);

	str = g_strdup_printf ("The state is now '%s'", txt);
	bonobo_ui_component_set_status (global_component, str, NULL);
	g_free (str);

	g_free (txt);
}

static void
toggled_cb (BonoboUIComponent           *component,
	    const char                  *path,
	    Bonobo_UIComponent_EventType type,
	    const char                  *state,
	    gpointer                     user_data)
{
	fprintf (stderr, "toggled to '%s' type '%d' path '%s'\n",
		 state, type, path);
}

static void
disconnect_progress (GtkObject *progress, gpointer dummy)
{
	gtk_timeout_remove (GPOINTER_TO_UINT (dummy));
}

static gboolean
update_progress (GtkProgressBar *progress)
{
	double pos = gtk_progress_bar_get_fraction (progress);

	if (pos < 0.95)
		pos += 0.05;
	else
		pos = 0;

	gtk_progress_bar_set_fraction (progress, pos);

	return TRUE;
}

static void
slow_size_request (GtkWidget      *widget,
		   GtkRequisition *requisition,
		   gpointer        user_data)
{
/*	sleep (2);*/
}

static void
file_exit_cmd (BonoboUIComponent *uic,
	       gpointer           user_data,
	       const char        *verbname)
{
	exit (0);
}

static void
file_open_cmd (BonoboUIComponent *uic,
	       gpointer           user_data,
	       const char        *verbname)
{
	g_warning ("File Open");
}


static gboolean
do_sane_popup (GtkWidget      *widget,
	       GdkEventButton *event,
	       BonoboControl  *control)
{
	if (event->button == 3)
		return bonobo_control_do_popup (
			control, event->button, event->time);

	return FALSE;
}

BonoboUIVerb verbs [] = {
	BONOBO_UI_VERB ("FileExit", file_exit_cmd),
	BONOBO_UI_VERB ("FileOpen", file_open_cmd),

	BONOBO_UI_VERB_END
};

int
main (int argc, char **argv)
{
	BonoboWindow *win;
	BonoboUIComponent *componenta;
	BonoboUIComponent *componentb;
	BonoboUIComponent *componentc;
	BonoboUIContainer *container;
	Bonobo_UIContainer corba_container;
	CORBA_Environment  real_ev, *ev;
	char *txt, *fname;
	int i;

	char simplea [] =
		"<menu>\n"
		"	<submenu name=\"File\" _label=\"_Ga'\">\n"
		"		<menuitem name=\"open\" pos=\"bottom\" _label=\"_Open\" verb=\"FileOpen\" pixtype=\"stock\" pixname=\"Open\" _tip=\"Wibble\"/>\n"
		"		<control name=\"MyControl\"/>\n"
		"		<control name=\"ThisIsEmpty\"/>\n"
		"		<menuitem name=\"close\" noplace=\"1\" verb=\"FileExit\" _label=\"_CloseA\" _tip=\"hi\""
		"		pixtype=\"stock\" pixname=\"Close\" accel=\"*Control*q\"/>\n"
		"	</submenu>\n"
		"</menu>";
	char simpleb [] =
		"<submenu name=\"File\" _label=\"_File\">\n"
		"	<menuitem name=\"open\" _label=\"_OpenB\" pixtype=\"stock\" pixname=\"Open\" _tip=\"Open you fool\"/>\n"
		"       <separator/>\n"
		"       <menuitem name=\"toggle\" type=\"toggle\" id=\"MyFoo\" _label=\"_ToggleMe\" _tip=\"a\" accel=\"*Control*t\"/>\n"
		"       <placeholder name=\"Nice\" delimit=\"top\"/>\n"
		"	<menuitem name=\"close\" noplace=\"1\" verb=\"FileExit\" _label=\"_CloseB\" _tip=\"hi\""
		"        pixtype=\"stock\" pixname=\"Close\" accel=\"*Control*q\"/>\n"
		"</submenu>\n";
	char simplec [] =
		"<submenu name=\"File\" _label=\"_FileC\" _tip=\"what!\">\n"
		"    <placeholder name=\"Nice\" delimit=\"top\" hidden=\"0\">\n"
		"	<menuitem name=\"fooa\" _label=\"_FooA\" type=\"radio\" group=\"foogroup\" _tip=\"Radio1\"/>\n"
		"	<menuitem name=\"foob\" _label=\"_FooB\" type=\"radio\" group=\"foogroup\" _tip=\"kippers\"/>\n"
		"	<menuitem name=\"wibble\" verb=\"ThisForcesAnError\" _label=\"_Baa\""
		"        pixtype=\"stock\" pixname=\"Open\" sensitive=\"0\" _tip=\"fish\"/>\n"
		"       <separator/>\n"
		"    </placeholder>\n"
		"</submenu>\n";
	char simpled [] =
		"<menuitem name=\"save\" _label=\"_SaveD\" pixtype=\"stock\" pixname=\"Save\" _tip=\"tip1\"/>\n";
	char simplee [] =
		"<menuitem name=\"fish\" _label=\"_Inplace\" pixtype=\"stock\" pixname=\"Save\" _tip=\"tip2\"/>\n";
	char toola [] =
		"<dockitem name=\"Toolbar\" homogeneous=\"0\" vlook=\"icon\">\n"
		"	<toolitem type=\"toggle\" name=\"foo2\" id=\"MyFoo\" pixtype=\"stock\" pixname=\"Save\""
		"        _label=\"TogSave\" _tip=\"My tooltip\" priority=\"1\"/>\n"
		"	<separator/>\n"
		"	<toolitem name=\"baa\" pixtype=\"stock\" pixname=\"Open\" _label=\"baa\" _tip=\"My 2nd tooltip\" verb=\"testme\"/>\n"
		"	<control name=\"AControl\" _tip=\"a tip on a control\" hidden=\"0\" vdisplay=\"button\"\n"
		"	pixtype=\"stock\" pixname=\"Attach\"/>\n"
		"</dockitem>";
	char toolb [] =
		"<dockitem name=\"Toolbar\" look=\"icon\" relief=\"none\">\n"
		"	<toolitem name=\"foo1\" _label=\"Insensitive\" sensitive=\"0\" hidden=\"0\" priority=\"1\"/>\n"
		"	<toolitem type=\"toggle\" name=\"foo5\" id=\"MyFoo\" pixtype=\"stock\" pixname=\"Close\""
		"	 _label=\"TogSame\" _tip=\"My tooltip\"/>\n"
		"</dockitem>";
/*	char statusa [] =
		"<item name=\"main\">Kippers</item>\n";*/
	char statusb [] =
		"<status>\n"
		"	<item name=\"main\"/>\n"
		"	<control name=\"Progress\"/>\n"
		"</status>";

	ev = &real_ev;
	CORBA_exception_init (ev);

	free (malloc (8));

	gnome_program_init ("test-ui", VERSION,
			    LIBBONOBOUI_MODULE,
			    argc, argv, NULL);

	textdomain (GETTEXT_PACKAGE);

	bonobo_activate ();

	dump_prefs ();

	win = BONOBO_WINDOW (bonobo_window_new ("Win", "My Test Application"));
	container = bonobo_window_get_ui_container (win);
	
	bonobo_ui_engine_config_set_path (bonobo_window_get_ui_engine (win),
					  "/test-ui/UIConfig/kvps");

	corba_container = BONOBO_OBJREF (container);

	{
		GtkWidget *box = gtk_vbox_new (FALSE, 0);
		GtkWidget *button;
		GtkWidget *path_entry, *state_entry;

		button = gtk_button_new_with_label ("Press me to test!");
		g_signal_connect (GTK_OBJECT (button), "clicked",
				    (GtkSignalFunc) cb_do_quit, NULL);
		gtk_widget_show (GTK_WIDGET (button));
		gtk_box_pack_start_defaults (GTK_BOX (box), button);

		button = gtk_button_new_with_label ("Dump Xml tree");
		g_signal_connect (GTK_OBJECT (button), "clicked",
				    (GtkSignalFunc) cb_do_dump, win);
		gtk_widget_show (GTK_WIDGET (button));
		gtk_box_pack_start_defaults (GTK_BOX (box), button);

		button = gtk_button_new_with_label ("Popup");
		g_signal_connect (GTK_OBJECT (button), "clicked",
				    (GtkSignalFunc) cb_do_popup, win);
		gtk_widget_show (GTK_WIDGET (button));
		gtk_box_pack_start_defaults (GTK_BOX (box), button);

		button = gtk_button_new_with_label ("Hide toolbar");
		g_signal_connect (GTK_OBJECT (button), "clicked",
				    (GtkSignalFunc) cb_do_hide_toolbar, win);
		gtk_widget_show (GTK_WIDGET (button));
		gtk_box_pack_start_defaults (GTK_BOX (box), button);

		path_entry = gtk_entry_new ();
		gtk_entry_set_text (GTK_ENTRY (path_entry), "/menu/File/toggle");
		gtk_widget_show (GTK_WIDGET (path_entry));
		gtk_box_pack_start_defaults (GTK_BOX (box), path_entry);

		state_entry = gtk_entry_new ();
		gtk_entry_set_text (GTK_ENTRY (state_entry), "1");
		g_signal_connect (GTK_OBJECT (state_entry), "changed",
				    (GtkSignalFunc) cb_set_state, path_entry);
		gtk_widget_show (GTK_WIDGET (state_entry));
		gtk_box_pack_start_defaults (GTK_BOX (box), state_entry);

		gtk_widget_show (GTK_WIDGET (box));
		bonobo_window_set_contents (win, box);
	}

	g_signal_connect (GTK_OBJECT (win), "size_request", 
			    G_CALLBACK (slow_size_request), NULL);

	componenta = bonobo_ui_component_new ("A");
	bonobo_object_unref (BONOBO_OBJECT (componenta));

	componenta = bonobo_ui_component_new ("A");
	componentb = bonobo_ui_component_new ("B");
	componentc = bonobo_ui_component_new ("C");


	bonobo_ui_component_set_container (componenta, corba_container, NULL);
	bonobo_ui_component_set_container (componentb, corba_container, NULL);
	bonobo_ui_component_set_container (componentc, corba_container, NULL);

	global_component = componenta;

	fname = bonobo_ui_util_get_ui_fname (NULL, "../doc/std-ui.xml");
	if (fname && g_file_test (fname, G_FILE_TEST_EXISTS)) {
		fprintf (stderr, "\n\n--- Add std-ui.xml ---\n\n\n");
		bonobo_ui_util_set_ui (componenta, NULL, "../doc/std-ui.xml",
				       "gdm", NULL);

/*		bonobo_ui_component_set_prop (
			componenta, "/menu/Preferences",
			"pixname", "/demo/a.xpm", NULL);*/

		gtk_widget_show (GTK_WIDGET (win));

		bonobo_main ();
	} else {
		g_warning ("Can't find ../doc/std-ui.xml");
		gtk_widget_show (GTK_WIDGET (win));
	}
	g_free (fname);


	bonobo_ui_component_freeze (componenta, NULL);

	fprintf (stderr, "\n\n--- Remove A ---\n\n\n");
	bonobo_ui_component_rm (componenta, "/", ev);
	g_assert (!BONOBO_EX (ev));

/*	bonobo_ui_component_set_translate (componentb, "/status", statusa, ev);
	g_assert (!BONOBO_EX (ev));*/

	bonobo_ui_component_set_translate (componenta, "/", simplea, ev);
	g_assert (!BONOBO_EX (ev));

	bonobo_ui_component_set_translate (componentb, "/",
				 "<popups> <popup name=\"MyStuff\"/> </popups>", ev);
	g_assert (!BONOBO_EX (ev));

	bonobo_ui_component_set_translate (componenta, "/popups/MyStuff", simpleb, ev);
	g_assert (!BONOBO_EX (ev));

	bonobo_ui_component_set_translate (componentb, "/",   toola, ev);
	g_assert (!BONOBO_EX (ev));

	{
		GtkWidget *widget = gtk_button_new_with_label ("My Label");
		BonoboControl *control = bonobo_control_new (widget);
		BonoboUIComponent *componentp;

		g_signal_connect (GTK_OBJECT (widget), "button_press_event",
				    G_CALLBACK (do_sane_popup), control);
		componentp = bonobo_control_get_popup_ui_component (control);
#if 1
		bonobo_ui_component_set (componentp, "/", "<popups>"
					 "<popup name=\"button3\"/></popups>", ev);
		g_assert (!BONOBO_EX (ev));
		bonobo_ui_component_set_translate (
			componentp, "/popups/button3", simpleb, ev);
		g_assert (!BONOBO_EX (ev));
#endif

		gtk_widget_show (widget);
		bonobo_ui_component_object_set (componenta,
						"/menu/File/MyControl",
						BONOBO_OBJREF (control),
						ev);
		bonobo_object_unref (BONOBO_OBJECT (control));
		g_assert (!BONOBO_EX (ev));
	}

	{
		GtkWidget *widget = gtk_entry_new ();
		BonoboControl *control = bonobo_control_new (widget);
		
		gtk_entry_set_text (GTK_ENTRY (widget), "Example text");
		gtk_widget_show (widget);
		bonobo_ui_component_object_set (componenta,
						"/Toolbar/AControl",
						BONOBO_OBJREF (control),
						ev);
		bonobo_object_unref (BONOBO_OBJECT (control));
		g_assert (!BONOBO_EX (ev));
	}

	bonobo_ui_component_add_listener (componentb, "MyFoo", toggled_cb, ev);
	g_assert (!BONOBO_EX (ev));

	bonobo_ui_component_set_translate (componentb, "/",     statusb, ev);
	g_assert (!BONOBO_EX (ev));

	/* Duplicate set */
	bonobo_ui_component_set_translate (componenta, "/", simplea, ev);
	g_assert (!BONOBO_EX (ev));

	bonobo_ui_component_add_verb_list_with_data (
		componenta, verbs, GUINT_TO_POINTER (15));

	bonobo_ui_component_thaw (componenta, ev);
	g_assert (!BONOBO_EX (ev));

	bonobo_ui_component_set_status (componenta, "WhatA1", ev);
	g_assert (!BONOBO_EX (ev));
	bonobo_ui_component_set_status (componenta, "WhatA1", ev);
	g_assert (!BONOBO_EX (ev));
	bonobo_ui_component_set_status (componentb, "WhatB2", ev);
	g_assert (!BONOBO_EX (ev));
	bonobo_ui_component_set_status (componenta, "WhatA3", ev);
	g_assert (!BONOBO_EX (ev));
	bonobo_ui_component_rm (componenta, "/status", ev);
	g_assert (!BONOBO_EX (ev));
	bonobo_ui_component_set_status (componentb, "WhatB4", ev);
	g_assert (!BONOBO_EX (ev));
	bonobo_ui_component_set_status (componenta, "WhatA5", ev);
	g_assert (!BONOBO_EX (ev));
	bonobo_ui_component_set_status (componenta, "WhatA6>", ev);
	g_assert (!BONOBO_EX (ev));
	bonobo_ui_component_set_status (componentb, "WhatB7", ev);
	g_assert (!BONOBO_EX (ev));
	bonobo_ui_component_set_status (componentb, "", ev);
	g_assert (!BONOBO_EX (ev));

	g_assert (bonobo_ui_component_get_prop (
		componentb, "/status/main", "non-existant", ev) == NULL);
	g_assert (!strcmp (BONOBO_EX_REPOID (ev), ex_Bonobo_UIContainer_NonExistentAttr));
	CORBA_exception_free (ev);

  	{
  		char *txt = bonobo_ui_component_get (componenta, "/status/main", TRUE, NULL);
 		const char *good = "<item name=\"main\">WhatA6&gt;</item>\n";
 
 		if (!txt || strcmp (txt, good)) {
 			g_warning ("Broken merging code '%s' should be '%s'", txt, good);
 			bonobo_window_dump (win, "on fatal error");
  			g_assert_not_reached ();
  		}

		CORBA_free (txt);
  	}

	bonobo_main ();

	bonobo_ui_component_freeze (componenta, ev);
	g_assert (!BONOBO_EX (ev));

	bonobo_ui_component_set_translate (componentb, "/menu", simpleb, ev);
	g_assert (!BONOBO_EX (ev));
	bonobo_ui_component_set_translate (componenta, "/",     toolb, ev);
	g_assert (!BONOBO_EX (ev));

	bonobo_ui_component_set_prop (componenta, "/menu/File", "label", "_Goo-wan>", ev);
	g_assert (!BONOBO_EX (ev));

	/* A 'transparent' node merge */
	txt = bonobo_ui_component_get_prop (componenta, "/Toolbar", "look", ev);
	g_assert (!BONOBO_EX (ev));
	printf ("Before merge look '%s'\n", txt);
	bonobo_ui_component_set_translate (componenta, "/", "<dockitem name=\"Toolbar\"/>", ev);
	g_assert (!BONOBO_EX (ev));
	g_free (txt);
	txt = bonobo_ui_component_get_prop (componenta, "/Toolbar", "look", ev);
	g_assert (!BONOBO_EX (ev));
	printf ("After merge look '%s'\n", txt);
	if (txt == NULL || strcmp (txt, "icon"))
		g_warning ("Serious transparency regression");
	g_free (txt);

	bonobo_ui_component_set_translate (componenta, "/menu/File/Nice", simplee, ev);
	g_assert (!BONOBO_EX (ev));

	{
		GtkWidget *widget = gtk_progress_bar_new ();
		BonoboControl *control = bonobo_control_new (widget);
		guint id;

		gtk_widget_show (widget);
		bonobo_ui_component_object_set (componenta, "/status/Progress",
						BONOBO_OBJREF (control),
						NULL);

		id = gtk_timeout_add (100, (GSourceFunc) update_progress, widget);
		g_signal_connect (GTK_OBJECT (widget), "destroy",
				    G_CALLBACK (disconnect_progress), GUINT_TO_POINTER (id));
		bonobo_object_unref (BONOBO_OBJECT (control));
	}

	bonobo_ui_component_set_status (componenta, "This is a very long status message "
					"that should cause the window to be resized if "
					"there is in fact a bug in it", ev);
	g_assert (!BONOBO_EX (ev));

	bonobo_ui_component_thaw (componenta, ev);
	g_assert (!BONOBO_EX (ev));

	bonobo_main ();

	g_warning ("Begginning stress test, this may take some time ...");
	for (i = 0; i < 100; i++) {
		bonobo_ui_component_freeze (componentc, ev);
		g_assert (!BONOBO_EX (ev));
		
		bonobo_ui_component_set_translate (componentc, "/commands",
						   "<cmd name=\"MyFoo\" sensitive=\"0\"/>", ev);
		g_assert (!BONOBO_EX (ev));

		bonobo_ui_component_set_translate (componentc, "/menu", simplec, ev);
		g_assert (!BONOBO_EX (ev));
	
		bonobo_ui_component_set_translate (componentc, "/menu/File", simpled, ev);
		g_assert (!BONOBO_EX (ev));

		bonobo_ui_component_thaw (componentc, ev);
		g_assert (!BONOBO_EX (ev));
	}
	g_warning ("Done stress test");
	bonobo_main ();
	bonobo_ui_component_freeze (componenta, ev);
	g_assert (!BONOBO_EX (ev));

	fprintf (stderr, "\n\n--- Remove 2 ---\n\n\n");
	bonobo_ui_component_rm (componentb, "/", ev);
	g_assert (!BONOBO_EX (ev));
	bonobo_ui_component_set_prop (componentc, "/menu/File/save",
				      "label", "SaveC", ev);
	g_assert (!BONOBO_EX (ev));

	bonobo_ui_component_thaw (componenta, ev);
	g_assert (!BONOBO_EX (ev));

	bonobo_main ();

	bonobo_ui_component_freeze (componenta, ev);
	g_assert (!BONOBO_EX (ev));

	fprintf (stderr, "\n\n--- Remove 3 ---\n\n\n");
	bonobo_ui_component_rm (componentc, "/", ev);
	g_assert (!BONOBO_EX (ev));

	bonobo_ui_component_thaw (componenta, ev);
	g_assert (!BONOBO_EX (ev));

	bonobo_main ();

	bonobo_ui_component_freeze (componenta, ev);
	g_assert (!BONOBO_EX (ev));

	fprintf (stderr, "\n\n--- Remove 1 ---\n\n\n");
	bonobo_ui_component_rm (componenta, "/", ev);
	g_assert (!BONOBO_EX (ev));

	bonobo_ui_component_thaw (componenta, ev);
	g_assert (!BONOBO_EX (ev));

	bonobo_main ();

	bonobo_object_unref (BONOBO_OBJECT (componenta));
	bonobo_object_unref (BONOBO_OBJECT (componentb));
	bonobo_object_unref (BONOBO_OBJECT (componentc));

	gtk_widget_destroy (GTK_WIDGET (win));

	CORBA_exception_free (ev);

	return bonobo_ui_debug_shutdown ();
}

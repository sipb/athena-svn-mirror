/*
 * hello.c
 *
 * A hello world application using the Bonobo UI handler
 *
 * Authors:
 *	Michael Meeks    <michael@ximian.com>
 *	Murray Cumming   <murrayc@usa.net>
 *      Havoc Pennington <hp@redhat.com>
 *
 * Copyright (C) 1999 Red Hat, Inc.
 *               2001 Murray Cumming,
 *               2001 Ximian, Inc.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <config.h>
#include <string.h>
#include <stdlib.h>

#include <libbonoboui.h>

/* Keep a list of all open application windows */
static GSList *app_list = NULL;

#define HELLO_UI_XML "Bonobo_Sample_Hello.xml"

/*   A single forward prototype - try and keep these
 * to a minumum by ordering your code nicely */
static GtkWidget *hello_new (void);

static void
hello_on_menu_file_new (BonoboUIComponent *uic,
			gpointer           user_data,
			const gchar       *verbname)
{
	gtk_widget_show_all (hello_new ());
}

static void
show_nothing_dialog(GtkWidget* parent)
{
	GtkWidget* dialog;

	dialog = gtk_message_dialog_new (
		GTK_WINDOW (parent),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
		_("This does nothing; it is only a demonstration."));

	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static void
hello_on_menu_file_open (BonoboUIComponent *uic,
			    gpointer           user_data,
			    const gchar       *verbname)
{
	show_nothing_dialog (GTK_WIDGET (user_data));
}

static void
hello_on_menu_file_save (BonoboUIComponent *uic,
			    gpointer           user_data,
			    const gchar       *verbname)
{
	show_nothing_dialog (GTK_WIDGET (user_data));
}

static void
hello_on_menu_file_saveas (BonoboUIComponent *uic,
			      gpointer           user_data,
			      const gchar       *verbname)
{
	show_nothing_dialog (GTK_WIDGET (user_data));
}

static void
hello_on_menu_file_exit (BonoboUIComponent *uic,
			    gpointer           user_data,
			    const gchar       *verbname)
{
	/* FIXME: quit the mainloop nicely */
	exit (0);
}	

static void
hello_on_menu_file_close (BonoboUIComponent *uic,
			     gpointer           user_data,
			     const gchar       *verbname)
{
	GtkWidget *app = user_data;

	/* Remove instance: */
	app_list = g_slist_remove (app_list, app);

	gtk_widget_destroy (app);

	if (!app_list)
		hello_on_menu_file_exit(uic, user_data, verbname);
}

static void
hello_on_menu_edit_undo (BonoboUIComponent *uic,
			    gpointer           user_data,
			    const gchar       *verbname)
{
	show_nothing_dialog (GTK_WIDGET (user_data));
}	

static void
hello_on_menu_edit_redo (BonoboUIComponent *uic,
			    gpointer           user_data,
			    const gchar       *verbname)
{
	show_nothing_dialog (GTK_WIDGET (user_data));
}	

static void
hello_on_menu_help_about (BonoboUIComponent *uic,
			     gpointer           user_data,
			     const gchar       *verbname)
{
	GtkWidget *dialog;

	dialog = gtk_message_dialog_new (
		GTK_WINDOW (user_data),
		GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
		_("BonoboUI-Hello."));

	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (GTK_WIDGET (dialog));
}

static gchar *
utf8_reverse (const char *string)
{
	int len;
	gchar *result;
	const gchar *p;
	gchar *m, *r, skip;

	len = strlen (string);

	result = g_new (gchar, len + 1);
	r = result + len;
	p = string;
	while (*p)  {
		skip = g_utf8_skip[*(guchar*)p];
		r -= skip;
		for (m = r; skip; skip--)
			*m++ = *p++;
	}
	result[len] = 0;

	return result;
}

static void
hello_on_button_click (GtkWidget* w, gpointer user_data)
{
	gchar    *text;
	GtkLabel *label = GTK_LABEL (user_data);

	text = utf8_reverse (gtk_label_get_text (label));

	gtk_label_set_text (label, text);

	g_free (text);
}

/*
 *   These verb names are standard, see libonobobui/doc/std-ui.xml
 * to find a list of standard verb names.
 *   The menu items are specified in Bonobo_Sample_Hello.xml and
 * given names which map to these verbs here.
 */
static const BonoboUIVerb hello_verbs [] = {
	BONOBO_UI_VERB ("FileNew",    hello_on_menu_file_new),
	BONOBO_UI_VERB ("FileOpen",   hello_on_menu_file_open),
	BONOBO_UI_VERB ("FileSave",   hello_on_menu_file_save),
	BONOBO_UI_VERB ("FileSaveAs", hello_on_menu_file_saveas),
	BONOBO_UI_VERB ("FileClose",  hello_on_menu_file_close),
	BONOBO_UI_VERB ("FileExit",   hello_on_menu_file_exit),

	BONOBO_UI_VERB ("EditUndo",   hello_on_menu_edit_undo),
	BONOBO_UI_VERB ("EditRedo",   hello_on_menu_edit_redo),

	BONOBO_UI_VERB ("HelpAbout",  hello_on_menu_help_about),

	BONOBO_UI_VERB_END
};

static BonoboWindow *
hello_create_main_window (void)
{
	BonoboWindow      *win;
	BonoboUIContainer *ui_container;
	BonoboUIComponent *ui_component;

	win = BONOBO_WINDOW (bonobo_window_new (GETTEXT_PACKAGE, _("Gnome Hello")));

	/* Create Container: */
	ui_container = bonobo_window_get_ui_container (win);

	/* This determines where the UI configuration info. will be stored */
	bonobo_ui_engine_config_set_path (bonobo_window_get_ui_engine (win),
					  "/hello-app/UIConfig/kvps");

	/* Create a UI component with which to communicate with the window */
	ui_component = bonobo_ui_component_new_default ();

	/* Associate the BonoboUIComponent with the container */
	bonobo_ui_component_set_container (
		ui_component, BONOBO_OBJREF (ui_container), NULL);

	/* NB. this creates a relative file name from the current dir,
	 * in production you want to pass the application's datadir
	 * see Makefile.am to see how HELLO_SRCDIR gets set. */
	bonobo_ui_util_set_ui (ui_component, "", /* data dir */
			       HELLO_SRCDIR HELLO_UI_XML,
			       "bonobo-hello", NULL);

	/* Associate our verb -> callback mapping with the BonoboWindow */
	/* All the callback's user_data pointers will be set to 'win' */
	bonobo_ui_component_add_verb_list_with_data (ui_component, hello_verbs, win);

	return win;
}

static gint 
delete_event_cb (GtkWidget *window, GdkEventAny *e, gpointer user_data)
{
	hello_on_menu_file_close (NULL, window, NULL);

	/* Prevent the window's destruction, since we destroyed it 
	 * ourselves with hello_app_close() */
	return TRUE;
}

static GtkWidget *
hello_new (void)
{
	GtkWidget    *label;
	GtkWidget    *frame;
	GtkWidget    *button;
	BonoboWindow *win;

	win = hello_create_main_window();

	/* Create Button: */
	button = gtk_button_new ();
	gtk_container_set_border_width (GTK_CONTAINER (button), 10);

	/* Create Label and put it in the Button: */
	label = gtk_label_new (_("Hello, World!"));
	gtk_container_add (GTK_CONTAINER (button), label);

	/* Connect the Button's 'clicked' signal to the signal handler:
	 * pass label as the data, so that the signal handler can use it. */
	g_signal_connect (
		GTK_OBJECT (button), "clicked",
		G_CALLBACK(hello_on_button_click), label);

	gtk_window_set_resizable (GTK_WINDOW (win), TRUE);
	gtk_window_set_default_size (GTK_WINDOW (win), 250, 350);

	/* Create Frame and add it to the main BonoboWindow: */
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
	gtk_container_add (GTK_CONTAINER (frame), button);
	bonobo_window_set_contents (win, frame);

	/* Connect to the delete_event: a close from the window manager */
	g_signal_connect (GTK_OBJECT (win),
			    "delete_event",
			    G_CALLBACK (delete_event_cb),
			    NULL);

	/* Append ourself to the list of windows */
	app_list = g_slist_prepend (app_list, win);

	return GTK_WIDGET(win);
}

int 
main (int argc, char* argv[])
{
	GtkWidget *app;

	/* Setup translaton domain */
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	textdomain (GETTEXT_PACKAGE);

	if (!bonobo_ui_init ("bonobo-hello", VERSION, &argc, argv))
		g_error (_("Cannot init libbonoboui code"));

	app = hello_new ();

	gtk_widget_show_all (GTK_WIDGET (app));

	bonobo_main ();

	return 0;
}

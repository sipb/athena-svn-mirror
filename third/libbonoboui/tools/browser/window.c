/* Bonobo component browser
 *
 * AUTHORS:
 *      Dan Siemon <dan@coverfire.com>
 *      Rodrigo Moya <rodrigo@gnome-db.org>
 *      Patanjali Somayaji <patanjali@morelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtkbutton.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkstatusbar.h>
#include <gtk/gtkstock.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkentry.h>
#include <bonobo/bonobo-main.h>
#include <bonobo/bonobo-ui-component.h>
#include <bonobo/bonobo-ui-container.h>
#include <bonobo/bonobo-window.h>
#include <bonobo/bonobo-ui-util.h>
#include "bonobo-browser.h"
#include "component-list.h"
#include "component-details.h"

static void verb_FileNewWindow (BonoboUIComponent *uic, void *data, const char *path);
static void verb_FileClose (BonoboUIComponent *uic, void *data, const char *path);
static void verb_HelpAbout (BonoboUIComponent *uic, void *data, const char *path);

static GList *open_windows = NULL;

static BonoboUIVerb window_verbs[] = {
	BONOBO_UI_VERB ("FileNewWindow", verb_FileNewWindow),
	BONOBO_UI_VERB ("FileClose", verb_FileClose),
	BONOBO_UI_VERB ("HelpAbout", verb_HelpAbout),
	BONOBO_UI_VERB_END
};

struct window_info {
	GtkWidget *comp_list;
	GtkWidget *entry;
};

/*********************************
 * Callbacks
 *********************************/
static void
window_closed_cb (GObject *object, gpointer user_data)
{
	BonoboWindow *window = (BonoboWindow *) object;

	g_return_if_fail (BONOBO_IS_WINDOW (window));

	open_windows = g_list_remove (open_windows, window);

	gtk_widget_destroy (GTK_WIDGET (window));
	if (g_list_length (open_windows) <= 0) {
		bonobo_main_quit ();
	}
}

/*
 * Called when the Close button is clicked on a details window.
 */
static void
close_details_window_cb (GObject *object, gint response_id, gpointer user_data)
{
	GtkWidget *window = (GtkWidget *) user_data;

	gtk_widget_destroy (GTK_WIDGET (window));
}

/*
 * Callbacks for the query buttons.
 */
static void
all_query_cb (GObject *object, gpointer data)
{
	struct window_info *info;

	info = (struct window_info *) data;

	component_list_show (COMPONENT_LIST (info->comp_list),
			     "_active || _active == FALSE");

	gtk_entry_set_text (GTK_ENTRY (info->entry),
			    "_active || _active == FALSE");
}

static void
active_query_cb (GObject *object, gpointer data)
{
	struct window_info *info;

	info = (struct window_info *) data;

	component_list_show (COMPONENT_LIST (info->comp_list),
			     "_active");

	gtk_entry_set_text (GTK_ENTRY (info->entry),
			    "_active");
}

static void
inactive_query_cb (GObject *object, gpointer data)
{
	struct window_info *info;

	info = (struct window_info *) data;

	component_list_show (COMPONENT_LIST (info->comp_list),
			     "_active == FALSE");

	gtk_entry_set_text (GTK_ENTRY (info->entry),
			    "_active == FALSE");
}

static void
execute_query_cb (GObject *object, gpointer data)
{
	struct window_info *info;
	char *query;

	info = (struct window_info *) data;

	query = g_strdup_printf ("%s",
				 gtk_entry_get_text (GTK_ENTRY (info->entry)));

	component_list_show (COMPONENT_LIST (info->comp_list), query);
}

/*
 * Creates and shows the details window.
 */
static void
component_details_cb (GObject *object, gpointer data)
{
	GtkWidget *comp_details;
	GtkWidget *window;
	ComponentList *list;
	gchar *iid = NULL;

	list = (ComponentList *) data;

	iid = component_list_get_selected_iid (list);
	if (iid == NULL) {
		/* We do not handle this situation */
		g_assert_not_reached();
	}
	
	window = gtk_dialog_new_with_buttons (_("Component Details"),
					      NULL, 0,
					      GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
					      NULL);
	g_signal_connect (G_OBJECT (window), "response",
			  G_CALLBACK (close_details_window_cb), window);

	comp_details = component_details_new (iid);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (window)->vbox), comp_details, TRUE, TRUE, 0);

	gtk_widget_show_all (window);

	g_free (iid);
}

/*
 * Verbs' commands
 */
static void
verb_FileClose (BonoboUIComponent *uic, void *data, const char *path)
{
	BonoboWindow *window = (BonoboWindow *) data;

	g_return_if_fail (BONOBO_IS_WINDOW (window));
	gtk_widget_destroy (GTK_WIDGET (window));
}

static void
verb_FileNewWindow (BonoboUIComponent *uic, void *data, const char *path)
{
	bonobo_browser_create_window ();
}

static void
verb_HelpAbout (BonoboUIComponent *uic, void *data, const char *path)
{
	static const gchar *authors[] = {
		"Dan Siemon <dan@coverfire.com>",
		"Rodrigo Moya <rodrigo@gnome-db.org>",
		"Patanjali Somayaji <patanjali@morelinux.com>",
		NULL
	};
#if 0
	GtkWidget *about;

	about = gnome_about_new (_("Bonobo Browser"), VERSION,
				 _("Copyright 2001, The GNOME Foundation"),
				 _("Bonobo component browser"),
				 authors,
				 NULL,
				 NULL,
				 NULL);
	gtk_widget_show (about);
#endif
}

/*
 * Public functions
 */
void
bonobo_browser_create_window (void)
{
	GtkWidget *window, *status_bar;
	GtkWidget *all_button, *active_button, *inactive_button;
	GtkWidget *query_label, *execute_button;
	GtkWidget *main_vbox, *hbox;
	struct window_info *info;
	BonoboUIContainer *ui_container;
	BonoboUIComponent *ui_component;
	Bonobo_UIContainer corba_container;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	info = g_malloc (sizeof (struct window_info));

	/* create the window */
	window = bonobo_window_new ("bonobo-browser", _("Component Browser"));
	gtk_window_set_role (GTK_WINDOW (window), "Main window");
	gtk_window_set_type_hint (GTK_WINDOW (window),
				  GDK_WINDOW_TYPE_HINT_NORMAL);
	g_signal_connect (G_OBJECT (window), "delete_event",
			  G_CALLBACK (window_closed_cb), NULL);
	g_signal_connect (G_OBJECT (window), "destroy",
			  G_CALLBACK (window_closed_cb), NULL);

	ui_container = bonobo_window_get_ui_container (BONOBO_WINDOW(window));
	corba_container = BONOBO_OBJREF (ui_container);

	ui_component = bonobo_ui_component_new ("bonobo-browser");
	bonobo_ui_component_set_container (ui_component, corba_container,
					   NULL);

	/* set UI for the window */
	bonobo_ui_component_freeze (ui_component, NULL);
	bonobo_ui_util_set_ui (ui_component, BONOBO_BROWSER_DATADIR,
			       "bonobo-browser.xml",
			       "bonobo-browser", &ev);
	bonobo_ui_component_add_verb_list_with_data (ui_component,
						     window_verbs, window); 
	bonobo_ui_component_thaw (ui_component, NULL);

	/* Create the main window */
	main_vbox = gtk_vbox_new (FALSE, 0);
	bonobo_window_set_contents (BONOBO_WINDOW (window), main_vbox);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (main_vbox), hbox, FALSE, FALSE, 0);

	info->comp_list = component_list_new ();
	gtk_box_pack_start (GTK_BOX (main_vbox), info->comp_list,
			    TRUE, TRUE, 0);

	status_bar = gtk_statusbar_new ();
	gtk_box_pack_start (GTK_BOX (main_vbox), status_bar, FALSE, TRUE, 0);

	/* Fill out the tool bar */
	all_button = gtk_button_new_with_label ("All");
	g_signal_connect (G_OBJECT (all_button), "clicked",
			  G_CALLBACK (all_query_cb), info);
	active_button = gtk_button_new_with_label ("Active");
	g_signal_connect (G_OBJECT (active_button), "clicked",
			  G_CALLBACK (active_query_cb), info);
	inactive_button = gtk_button_new_with_label ("Inactive");
	g_signal_connect (G_OBJECT (inactive_button), "clicked",
			  G_CALLBACK (inactive_query_cb), info);
	query_label = gtk_label_new ("Query:");
	info->entry = gtk_entry_new ();
	g_signal_connect (GTK_ENTRY (info->entry), "activate",
			  G_CALLBACK (execute_query_cb), info);
	execute_button = gtk_button_new_from_stock (GTK_STOCK_EXECUTE);
	g_signal_connect (G_OBJECT (execute_button), "clicked",
			  G_CALLBACK (execute_query_cb), info);

	gtk_box_pack_start (GTK_BOX (hbox), all_button, FALSE, FALSE, 1);
	gtk_box_pack_start (GTK_BOX (hbox), active_button, FALSE, FALSE, 1);
	gtk_box_pack_start (GTK_BOX (hbox), inactive_button, FALSE, FALSE, 1);
	gtk_box_pack_start (GTK_BOX (hbox), query_label, FALSE, FALSE, 1);
	gtk_box_pack_start (GTK_BOX (hbox), info->entry, TRUE, TRUE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), execute_button, FALSE, FALSE, 0);

	/* Attach to the component-details signal */
	g_signal_connect (G_OBJECT (info->comp_list), "component-details",
			  G_CALLBACK (component_details_cb), info->comp_list);

	component_list_show (COMPONENT_LIST (info->comp_list),
			     "_active || _active == FALSE");
	gtk_entry_set_text (GTK_ENTRY (info->entry),
			    "_active || _active == FALSE");

	/* add this window to our list of open windows */
	open_windows = g_list_append (open_windows, window);

	gtk_widget_set_usize (window, 600, 500);
	gtk_widget_show_all (window);
}

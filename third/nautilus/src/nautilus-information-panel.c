 /* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
 * Nautilus
 *
 * Copyright (C) 1999, 2000, 2001 Eazel, Inc.
 *
 * Nautilus is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Nautilus is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Andy Hertzfeld <andy@eazel.com>
 *
 */

#include <config.h>
#include "nautilus-information-panel.h"

#include "nautilus-sidebar-title.h"

#include <bonobo/bonobo-property-bag-client.h>
#include <bonobo/bonobo-ui-util.h>
#include <bonobo/bonobo-exception.h>

#include <eel/eel-background.h>
#include <eel/eel-glib-extensions.h>
#include <eel/eel-gtk-extensions.h>
#include <eel/eel-gtk-macros.h>
#include <eel/eel-stock-dialogs.h>
#include <eel/eel-string.h>
#include <eel/eel-vfs-extensions.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libxml/parser.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkdnd.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkpaned.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtksignal.h>
#include <libgnome/gnome-i18n.h>
#include <libgnomeui/gnome-uidefs.h>
#include <libgnomevfs/gnome-vfs-application-registry.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>
#include <libgnomevfs/gnome-vfs-types.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <libgnomevfs/gnome-vfs-utils.h>
#include <libnautilus-private/nautilus-dnd.h>
#include <libnautilus-private/nautilus-directory.h>
#include <libnautilus-private/nautilus-file-dnd.h>
#include <libnautilus-private/nautilus-file-operations.h>
#include <libnautilus-private/nautilus-file.h>
#include <libnautilus-private/nautilus-global-preferences.h>
#include <libnautilus-private/nautilus-keep-last-vertical-box.h>
#include <libnautilus-private/nautilus-metadata.h>
#include <libnautilus-private/nautilus-mime-actions.h>
#include <libnautilus-private/nautilus-program-choosing.h>
#include <libnautilus-private/nautilus-sidebar-functions.h>
#include <libnautilus-private/nautilus-trash-monitor.h>
#include <math.h>

struct NautilusInformationPanelDetails {
	GtkVBox *container;
	NautilusSidebarTitle *title;
	GtkHBox *button_box_centerer;
	GtkVBox *button_box;
	gboolean has_buttons;
	char *uri;
	NautilusFile *file;
	guint file_changed_connection;
	gboolean background_connected;

	char *default_background_color;
	char *default_background_image;
	char *current_background_color;
	char *current_background_image;
};

/* button assignments */
#define CONTEXTUAL_MENU_BUTTON 3

static void     nautilus_information_panel_class_init      (GtkObjectClass   *object_klass);
static void     nautilus_information_panel_init            (GtkObject        *object);
static gboolean nautilus_information_panel_press_event           (GtkWidget        *widget,
							GdkEventButton   *event);
static void     nautilus_information_panel_destroy               (GtkObject        *object);
static void     nautilus_information_panel_finalize              (GObject          *object);
static void     nautilus_information_panel_drag_data_received    (GtkWidget        *widget,
							GdkDragContext   *context,
							int               x,
							int               y,
							GtkSelectionData *selection_data,
							guint             info,
							guint             time);
static void     nautilus_information_panel_read_defaults         (NautilusInformationPanel  *information_panel);
static void     nautilus_information_panel_style_set             (GtkWidget        *widget,
							GtkStyle         *previous_style);
static void     nautilus_information_panel_theme_changed         (gpointer          user_data);
static void     nautilus_information_panel_confirm_trash_changed (gpointer          user_data);
static void     nautilus_information_panel_update_appearance     (NautilusInformationPanel  *information_panel);
static void     nautilus_information_panel_update_buttons        (NautilusInformationPanel  *information_panel);
static void     add_command_buttons                    (NautilusInformationPanel  *information_panel,
							GList            *application_list);
static void     background_metadata_changed_callback   (NautilusInformationPanel  *information_panel);

static gboolean confirm_trash_auto_value = TRUE;

enum {
	LOCATION_CHANGED,
	LAST_SIGNAL
};

static guint signals[LAST_SIGNAL];

/* drag and drop definitions */

enum {
	TARGET_URI_LIST,
	TARGET_COLOR,
	TARGET_BGIMAGE,
	TARGET_KEYWORD,
	TARGET_BACKGROUND_RESET,
	TARGET_GNOME_URI_LIST
};

static GtkTargetEntry target_table[] = {
	{ "text/uri-list",  0, TARGET_URI_LIST },
	{ "application/x-color", 0, TARGET_COLOR },
	{ "property/bgimage", 0, TARGET_BGIMAGE },
	{ "property/keyword", 0, TARGET_KEYWORD },
	{ "x-special/gnome-reset-background", 0, TARGET_BACKGROUND_RESET },	
	{ "x-special/gnome-icon-list",  0, TARGET_GNOME_URI_LIST }
};

typedef enum {
	NO_PART,
	BACKGROUND_PART,
	ICON_PART,
} InformationPanelPart;

EEL_CLASS_BOILERPLATE (NautilusInformationPanel, nautilus_information_panel, EEL_TYPE_BACKGROUND_BOX)

/* initializing the class object by installing the operations we override */
static void
nautilus_information_panel_class_init (GtkObjectClass *object_klass)
{
	GtkWidgetClass *widget_class;
	GObjectClass *gobject_class;
	
	NautilusInformationPanelClass *klass;

	widget_class = GTK_WIDGET_CLASS (object_klass);
	klass = NAUTILUS_INFORMATION_PANEL_CLASS (object_klass);
	gobject_class = G_OBJECT_CLASS (object_klass);
	
	gobject_class->finalize = nautilus_information_panel_finalize;

	object_klass->destroy = nautilus_information_panel_destroy;
	
	widget_class->drag_data_received  = nautilus_information_panel_drag_data_received;
	widget_class->button_press_event  = nautilus_information_panel_press_event;
	widget_class->style_set = nautilus_information_panel_style_set;

	/* add the "location changed" signal */
	signals[LOCATION_CHANGED] = g_signal_new
		("location_changed",
		 G_TYPE_FROM_CLASS (object_klass),
		 G_SIGNAL_RUN_LAST,
		 G_STRUCT_OFFSET (NautilusInformationPanelClass,
				    location_changed),
		 NULL, NULL,
		 g_cclosure_marshal_VOID__STRING,
		 G_TYPE_NONE, 1, G_TYPE_STRING);
}

/* utility routine to allocate the box the holds the command buttons */
static void
make_button_box (NautilusInformationPanel *information_panel)
{
	information_panel->details->button_box_centerer = GTK_HBOX (gtk_hbox_new (FALSE, 0));
	gtk_box_pack_start_defaults (GTK_BOX (information_panel->details->container),
			    	     GTK_WIDGET (information_panel->details->button_box_centerer));

	information_panel->details->button_box = GTK_VBOX (nautilus_keep_last_vertical_box_new (GNOME_PAD_SMALL));
	gtk_container_set_border_width (GTK_CONTAINER (information_panel->details->button_box), GNOME_PAD);				
	gtk_widget_show (GTK_WIDGET (information_panel->details->button_box));
	gtk_box_pack_start (GTK_BOX (information_panel->details->button_box_centerer),
			    GTK_WIDGET (information_panel->details->button_box),
			    TRUE, FALSE, 0);
	information_panel->details->has_buttons = FALSE;
}

/* initialize the instance's fields, create the necessary subviews, etc. */

static void
nautilus_information_panel_init (GtkObject *object)
{
	GtkWidget *widget;
	static gboolean setup_autos = FALSE;
	NautilusInformationPanel *information_panel;
	
	information_panel = NAUTILUS_INFORMATION_PANEL (object);
	widget = GTK_WIDGET (object);

	information_panel->details = g_new0 (NautilusInformationPanelDetails, 1);

	if (!setup_autos) {
		setup_autos = TRUE;
		eel_preferences_add_auto_boolean (
			NAUTILUS_PREFERENCES_CONFIRM_TRASH,
			&confirm_trash_auto_value);
	}	
	
	/* load the default background */
	nautilus_information_panel_read_defaults (information_panel);

	/* enable mouse tracking */
	gtk_widget_add_events (GTK_WIDGET (information_panel), GDK_POINTER_MOTION_MASK);
	  	
	/* create the container box */
  	information_panel->details->container = GTK_VBOX (gtk_vbox_new (FALSE, 0));
	gtk_container_set_border_width (GTK_CONTAINER (information_panel->details->container), 0);				
	gtk_widget_show (GTK_WIDGET (information_panel->details->container));
	gtk_container_add (GTK_CONTAINER (information_panel),
			   GTK_WIDGET (information_panel->details->container));

	/* allocate and install the index title widget */ 
	information_panel->details->title = NAUTILUS_SIDEBAR_TITLE (nautilus_sidebar_title_new ());
	gtk_widget_show (GTK_WIDGET (information_panel->details->title));
	gtk_box_pack_start (GTK_BOX (information_panel->details->container),
			    GTK_WIDGET (information_panel->details->title),
			    FALSE, FALSE, GNOME_PAD);
	
	/* allocate and install the command button container */
	make_button_box (information_panel);

	/* add a callback for when the theme changes */
	eel_preferences_add_callback (NAUTILUS_PREFERENCES_SIDE_PANE_BACKGROUND_SET, nautilus_information_panel_theme_changed, information_panel);
	eel_preferences_add_callback (NAUTILUS_PREFERENCES_SIDE_PANE_BACKGROUND_COLOR, nautilus_information_panel_theme_changed, information_panel);
	eel_preferences_add_callback (NAUTILUS_PREFERENCES_SIDE_PANE_BACKGROUND_FILENAME, nautilus_information_panel_theme_changed, information_panel);

	/* add a callback for when the preference whether to confirm trashing/deleting file changes */
	eel_preferences_add_callback (NAUTILUS_PREFERENCES_CONFIRM_TRASH, nautilus_information_panel_confirm_trash_changed, information_panel);

	/* prepare ourselves to receive dropped objects */
	gtk_drag_dest_set (GTK_WIDGET (information_panel),
			   GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT | GTK_DEST_DEFAULT_DROP, 
			   target_table, G_N_ELEMENTS (target_table),
			   GDK_ACTION_COPY | GDK_ACTION_MOVE | GDK_ACTION_ASK);
}

static void
nautilus_information_panel_destroy (GtkObject *object)
{
	NautilusInformationPanel *information_panel;

	information_panel = NAUTILUS_INFORMATION_PANEL (object);

	EEL_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

static void
nautilus_information_panel_finalize (GObject *object)
{
	NautilusInformationPanel *information_panel;

	information_panel = NAUTILUS_INFORMATION_PANEL (object);

	if (information_panel->details->file != NULL) {
		nautilus_file_monitor_remove (information_panel->details->file, information_panel);
		nautilus_file_unref (information_panel->details->file);
	}
	
	g_free (information_panel->details->uri);
	g_free (information_panel->details->default_background_color);
	g_free (information_panel->details->default_background_image);
	g_free (information_panel->details->current_background_color);
	g_free (information_panel->details->current_background_image);
	g_free (information_panel->details);
		
	eel_preferences_remove_callback (NAUTILUS_PREFERENCES_SIDE_PANE_BACKGROUND_SET,
					 nautilus_information_panel_theme_changed,
					 information_panel);
	eel_preferences_remove_callback (NAUTILUS_PREFERENCES_SIDE_PANE_BACKGROUND_COLOR,
					 nautilus_information_panel_theme_changed,
					 information_panel);
	eel_preferences_remove_callback (NAUTILUS_PREFERENCES_SIDE_PANE_BACKGROUND_FILENAME,
					 nautilus_information_panel_theme_changed,
					 information_panel);

	eel_preferences_remove_callback (NAUTILUS_PREFERENCES_CONFIRM_TRASH,
					 nautilus_information_panel_confirm_trash_changed,
					 information_panel);


	EEL_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

/* callback to handle resetting the background */
static void
reset_background_callback (GtkWidget *menu_item, GtkWidget *information_panel)
{
	EelBackground *background;

	background = eel_get_widget_background (information_panel);
	if (background != NULL) { 
		eel_background_reset (background); 
	}
}

static gboolean
information_panel_has_background (NautilusInformationPanel *information_panel)
{
	EelBackground *background;
	gboolean has_background;
	char *color;
	char *image;

	background = eel_get_widget_background (GTK_WIDGET(information_panel));

	color = eel_background_get_color (background);
	image = eel_background_get_image_uri (background);
	
	has_background = (color || image);

	return has_background;
}

/* create the context menu */
static GtkWidget *
nautilus_information_panel_create_context_menu (NautilusInformationPanel *information_panel)
{
	GtkWidget *menu, *menu_item;

	menu = gtk_menu_new ();
	gtk_menu_set_screen (GTK_MENU (menu),
			     gtk_widget_get_screen (GTK_WIDGET (information_panel)));
	
	/* add the reset background item, possibly disabled */
	menu_item = gtk_menu_item_new_with_mnemonic (_("Use _Default Background"));
 	gtk_widget_show (menu_item);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), menu_item);
        gtk_widget_set_sensitive (menu_item, information_panel_has_background (information_panel));
	g_signal_connect_object (menu_item, "activate",
				 G_CALLBACK (reset_background_callback), information_panel, 0);

	return menu;
}

/* create a new instance */
NautilusInformationPanel *
nautilus_information_panel_new (void)
{
	return NAUTILUS_INFORMATION_PANEL (gtk_widget_new (nautilus_information_panel_get_type (), NULL));
}

/* set up the default backgrounds and images */
static void
nautilus_information_panel_read_defaults (NautilusInformationPanel *information_panel)
{
	gboolean background_set;
	char *background_color, *background_image;
	
	background_set = eel_preferences_get_boolean (NAUTILUS_PREFERENCES_SIDE_PANE_BACKGROUND_SET);
	
	background_color = NULL;
	background_image = NULL;
	if (background_set) {
		background_color = eel_preferences_get (NAUTILUS_PREFERENCES_SIDE_PANE_BACKGROUND_COLOR);
		background_image = eel_preferences_get (NAUTILUS_PREFERENCES_SIDE_PANE_BACKGROUND_FILENAME);
	}	
	
	g_free (information_panel->details->default_background_color);
	information_panel->details->default_background_color = NULL;
	g_free (information_panel->details->default_background_image);
	information_panel->details->default_background_image = NULL;
			
	if (background_color && strlen (background_color)) {
		information_panel->details->default_background_color = g_strdup (background_color);
	}
			
	/* set up the default background image */
	
	if (background_image && strlen (background_image)) {
		information_panel->details->default_background_image = g_strdup (background_image);
	}

	g_free (background_color);
	g_free (background_image);
}

/* handler for handling theme changes */

static void
nautilus_information_panel_theme_changed (gpointer user_data)
{
	NautilusInformationPanel *information_panel;
	
	information_panel = NAUTILUS_INFORMATION_PANEL (user_data);
	nautilus_information_panel_read_defaults (information_panel);
	nautilus_information_panel_update_appearance (information_panel);
	gtk_widget_queue_draw (GTK_WIDGET (information_panel)) ;	
}

/* handler for handling confirming trash preferences changes */

static void
nautilus_information_panel_confirm_trash_changed (gpointer user_data)
{
	NautilusInformationPanel *information_panel;
	
	information_panel = NAUTILUS_INFORMATION_PANEL (user_data);
	nautilus_information_panel_update_buttons (information_panel);
}

/* hit testing */

static InformationPanelPart
hit_test (NautilusInformationPanel *information_panel,
	  int x, int y)
{
	if (nautilus_sidebar_title_hit_test_icon (information_panel->details->title, x, y)) {
		return ICON_PART;
	}
	
	if (eel_point_in_widget (GTK_WIDGET (information_panel), x, y)) {
		return BACKGROUND_PART;
	}

	return NO_PART;
}

/* utility to test if a uri refers to a local image */
static gboolean
uri_is_local_image (const char *uri)
{
	GdkPixbuf *pixbuf;
	char *image_path;
	
	image_path = gnome_vfs_get_local_path_from_uri (uri);
	if (image_path == NULL) {
		return FALSE;
	}

	pixbuf = gdk_pixbuf_new_from_file (image_path, NULL);
	g_free (image_path);
	
	if (pixbuf == NULL) {
		return FALSE;
	}
	g_object_unref (pixbuf);
	return TRUE;
}

static void
receive_dropped_uri_list (NautilusInformationPanel *information_panel,
			  GdkDragAction action,
			  int x, int y,
			  GtkSelectionData *selection_data)
{
	char **uris;
	gboolean exactly_one;
	GtkWindow *window;
	
	uris = g_strsplit (selection_data->data, "\r\n", 0);
	exactly_one = uris[0] != NULL && (uris[1] == NULL || uris[1][0] == '\0');
	window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (information_panel)));
	
	switch (hit_test (information_panel, x, y)) {
	case NO_PART:
	case BACKGROUND_PART:
		/* FIXME bugzilla.gnome.org 42507: Does this work for all images, or only background images?
		 * Other views handle background images differently from other URIs.
		 */
		if (exactly_one && uri_is_local_image (uris[0])) {
			if (action == GDK_ACTION_ASK) {
				action = nautilus_drag_drop_background_ask (NAUTILUS_DND_ACTION_SET_AS_BACKGROUND | NAUTILUS_DND_ACTION_SET_AS_GLOBAL_BACKGROUND);
			}	

			if (action > 0) {
				eel_background_receive_dropped_background_image
					(eel_get_widget_background (GTK_WIDGET (information_panel)),
					 action,
					 uris[0]);
			}
		} else if (exactly_one) {
			g_signal_emit (information_panel,
					 signals[LOCATION_CHANGED], 0,
			 		 uris[0]);	
		}
		break;
	case ICON_PART:
		/* handle images dropped on the logo specially */
		
		if (!exactly_one) {
			eel_show_error_dialog (
				_("You can't assign more than one custom icon at a time! "
				  "Please drag just one image to set a custom icon."), 
				_("More Than One Image"),
				window);
			break;
		}
		
		if (uri_is_local_image (uris[0])) {
			if (information_panel->details->file != NULL) {
				nautilus_file_set_metadata (information_panel->details->file,
							    NAUTILUS_METADATA_KEY_CUSTOM_ICON,
							    NULL,
							    uris[0]);
				nautilus_file_set_metadata (information_panel->details->file,
							    NAUTILUS_METADATA_KEY_ICON_SCALE,
							    NULL,
							    NULL);
			}
		} else {	
			if (eel_is_remote_uri (uris[0])) {
				eel_show_error_dialog (
					_("The file that you dropped is not local.  "
					  "You can only use local images as custom icons."), 
					_("Local Images Only"),
					window);
			
			} else {
				eel_show_error_dialog (
					_("The file that you dropped is not an image.  "
					  "You can only use local images as custom icons."),
					_("Images Only"),
					window);
			}
		}	
		break;
	}

	g_strfreev (uris);
}

static void
receive_dropped_color (NautilusInformationPanel *information_panel,
		       GdkDragAction action,
		       int x, int y,
		       GtkSelectionData *selection_data)
{
	guint16 *channels;
	char *color_spec;

	if (selection_data->length != 8 || selection_data->format != 16) {
		g_warning ("received invalid color data");
		return;
	}
	
	channels = (guint16 *) selection_data->data;
	color_spec = g_strdup_printf ("#%02X%02X%02X", channels[0] >> 8, channels[1] >> 8, channels[2] >> 8);

	switch (hit_test (information_panel, x, y)) {
	case NO_PART:
		g_warning ("dropped color, but not on any part of information_panel");
		break;
	case ICON_PART:
	case BACKGROUND_PART:
		if (action == GDK_ACTION_ASK) {
			action = nautilus_drag_drop_background_ask (NAUTILUS_DND_ACTION_SET_AS_BACKGROUND | NAUTILUS_DND_ACTION_SET_AS_GLOBAL_BACKGROUND);
		}	

		if (action > 0) {
			/* Let the background change based on the dropped color. */
			eel_background_receive_dropped_color
				(eel_get_widget_background (GTK_WIDGET (information_panel)),
				 GTK_WIDGET (information_panel), 
				 action, x, y, selection_data);
		}
		
		break;
	}
	g_free(color_spec);
}

/* handle receiving a dropped keyword */

static void
receive_dropped_keyword (NautilusInformationPanel *information_panel,
			 int x, int y,
			 GtkSelectionData *selection_data)
{
	nautilus_drag_file_receive_dropped_keyword (information_panel->details->file, selection_data->data);
	
	/* regenerate the display */
	nautilus_information_panel_update_appearance (information_panel);  	
}

static void  
nautilus_information_panel_drag_data_received (GtkWidget *widget, GdkDragContext *context,
					 int x, int y,
					 GtkSelectionData *selection_data,
					 guint info, guint time)
{
	NautilusInformationPanel *information_panel;
	EelBackground *background;

	g_return_if_fail (NAUTILUS_IS_INFORMATION_PANEL (widget));

	information_panel = NAUTILUS_INFORMATION_PANEL (widget);

	switch (info) {
	case TARGET_GNOME_URI_LIST:
	case TARGET_URI_LIST:
		receive_dropped_uri_list (information_panel, context->action, x, y, selection_data);
		break;
	case TARGET_COLOR:
		receive_dropped_color (information_panel, context->action, x, y, selection_data);
		break;
	case TARGET_BGIMAGE:
		if (hit_test (information_panel, x, y) == BACKGROUND_PART)
			receive_dropped_uri_list (information_panel, context->action, x, y, selection_data);
		break;	
	case TARGET_BACKGROUND_RESET:
		background = eel_get_widget_background ( GTK_WIDGET (information_panel));
		if (background != NULL) { 
			eel_background_reset (background); 
		}
		break;
	case TARGET_KEYWORD:
		receive_dropped_keyword (information_panel, x, y, selection_data);
		break;
	default:
		g_warning ("unknown drop type");
	}
}

/* handle the context menu if necessary */
static gboolean
nautilus_information_panel_press_event (GtkWidget *widget, GdkEventButton *event)
{
	NautilusInformationPanel *information_panel;
	GtkWidget *menu;
		
	if (widget->window != event->window) {
		return FALSE;
	}

	information_panel = NAUTILUS_INFORMATION_PANEL (widget);

	/* handle the context menu */
	if (event->button == CONTEXTUAL_MENU_BUTTON) {
		menu = nautilus_information_panel_create_context_menu (information_panel);	
		eel_pop_up_context_menu (GTK_MENU(menu),
				      EEL_DEFAULT_POPUP_MENU_DISPLACEMENT,
				      EEL_DEFAULT_POPUP_MENU_DISPLACEMENT,
				      event);
	}	
	return TRUE;
}

static gboolean
value_different (const char *a, const char *b)
{
	if (!a && !b)
		return FALSE;

	if (!a || !b)
		return TRUE;

	return strcmp (a, b);
}

/* Handle the background changed signal by writing out the settings to metadata.
 */
static void
background_settings_changed_callback (EelBackground *background, GdkDragAction action, NautilusInformationPanel *information_panel)
{
	char *image;
	char *color;

	g_assert (EEL_IS_BACKGROUND (background));
	g_assert (NAUTILUS_IS_INFORMATION_PANEL (information_panel));

	if (information_panel->details->file == NULL) {
		return;
	}
	
	/* Block so we don't respond to our own metadata changes.
	 */
	g_signal_handlers_block_by_func (information_panel->details->file,
					 G_CALLBACK (background_metadata_changed_callback),
					 information_panel);

	color = eel_background_get_color (background);
	image = eel_background_get_image_uri (background);

	if (action != NAUTILUS_DND_ACTION_SET_AS_BACKGROUND) {
		nautilus_file_set_metadata (information_panel->details->file,
					    NAUTILUS_METADATA_KEY_SIDEBAR_BACKGROUND_COLOR,
					    NULL,
					    NULL);

		nautilus_file_set_metadata (information_panel->details->file,
					    NAUTILUS_METADATA_KEY_SIDEBAR_BACKGROUND_IMAGE,
					    NULL,
					    NULL);
		
		eel_preferences_set
			(NAUTILUS_PREFERENCES_SIDE_PANE_BACKGROUND_COLOR, color ? color : "");
		eel_preferences_set
			(NAUTILUS_PREFERENCES_SIDE_PANE_BACKGROUND_FILENAME, image ? image : "");
		eel_preferences_set_boolean
			(NAUTILUS_PREFERENCES_SIDE_PANE_BACKGROUND_SET, TRUE);
	} else {
		nautilus_file_set_metadata (information_panel->details->file,
					    NAUTILUS_METADATA_KEY_SIDEBAR_BACKGROUND_COLOR,
					    NULL,
					    color);
		
		nautilus_file_set_metadata (information_panel->details->file,
					    NAUTILUS_METADATA_KEY_SIDEBAR_BACKGROUND_IMAGE,
					    NULL,
					    image);
	}

	if (value_different (information_panel->details->current_background_color, color)) {
		g_free (information_panel->details->current_background_color);
		information_panel->details->current_background_color = g_strdup (color);
	}
	
	if (value_different (information_panel->details->current_background_image, image)) {
		g_free (information_panel->details->current_background_image);
		information_panel->details->current_background_image = g_strdup (image);
	}

	g_free (color);
	g_free (image);

	g_signal_handlers_unblock_by_func (information_panel->details->file,
					   G_CALLBACK (background_metadata_changed_callback),
					   information_panel);
}

/* handle the background reset signal by writing out NULL to metadata and setting the backgrounds
   fields to their default values */
static void
background_reset_callback (EelBackground *background, NautilusInformationPanel *information_panel)
{
	char *color;
	char *image;
	g_assert (EEL_IS_BACKGROUND (background));
	g_assert (NAUTILUS_IS_INFORMATION_PANEL (information_panel));

	if (information_panel->details->file == NULL) {
		return;
	}

	/* Block so we don't respond to our own metadata changes.
	 */
	g_signal_handlers_block_by_func (information_panel->details->file,
					 G_CALLBACK (background_metadata_changed_callback),
					 information_panel);

	color = nautilus_file_get_metadata (information_panel->details->file,
				    NAUTILUS_METADATA_KEY_SIDEBAR_BACKGROUND_COLOR,
				    NULL);

	image = nautilus_file_get_metadata (information_panel->details->file,
				    NAUTILUS_METADATA_KEY_SIDEBAR_BACKGROUND_IMAGE,
				    NULL);
	if (color || image) {
		nautilus_file_set_metadata (information_panel->details->file,
					    NAUTILUS_METADATA_KEY_SIDEBAR_BACKGROUND_COLOR,
					    NULL,
					    NULL);
		
		nautilus_file_set_metadata (information_panel->details->file,
					    NAUTILUS_METADATA_KEY_SIDEBAR_BACKGROUND_IMAGE,
					    NULL,
					    NULL);
	} else {
		eel_preferences_set_boolean (NAUTILUS_PREFERENCES_SIDE_PANE_BACKGROUND_SET, FALSE);
	}

	g_signal_handlers_unblock_by_func (information_panel->details->file,
					   G_CALLBACK (background_metadata_changed_callback),
					   information_panel);

	/* Force a read from the metadata to set the defaults
	 */
	background_metadata_changed_callback (information_panel);
}

static GtkWindow *
nautilus_information_panel_get_window (NautilusInformationPanel *information_panel)
{
	GtkWidget *result;

	result = gtk_widget_get_ancestor (GTK_WIDGET (information_panel), GTK_TYPE_WINDOW);

	return result == NULL ? NULL : GTK_WINDOW (result);
}

static void
command_button_callback (GtkWidget *button, char *id_str)
{
	NautilusInformationPanel *information_panel;
	GnomeVFSMimeApplication *application;
	
	information_panel = NAUTILUS_INFORMATION_PANEL (g_object_get_data (G_OBJECT (button), "user_data"));

	application = gnome_vfs_application_registry_get_mime_application (id_str);

	if (application != NULL) {
		nautilus_launch_application (application, information_panel->details->file,
					     nautilus_information_panel_get_window (information_panel));	

		gnome_vfs_mime_application_free (application);
	}
}

/* interpret commands for buttons specified by metadata. Handle some built-in ones explicitly, or fork
   a shell to handle general ones */
/* for now, we don't have any of these */
static void
metadata_button_callback (GtkWidget *button, const char *command_str)
{
	NautilusInformationPanel *information_panel;
		
	information_panel = NAUTILUS_INFORMATION_PANEL (g_object_get_data (G_OBJECT (button), "user_data"));
}

static void
nautilus_information_panel_chose_application_callback (GnomeVFSMimeApplication *application,
					     gpointer callback_data)
{
	NautilusInformationPanel *information_panel;

	information_panel = NAUTILUS_INFORMATION_PANEL (callback_data);

	if (application != NULL) {
		nautilus_launch_application
			(application, 
			 information_panel->details->file,
			 nautilus_information_panel_get_window (information_panel));
	}
}

static void
open_with_callback (GtkWidget *button, gpointer ignored)
{
	NautilusInformationPanel *information_panel;
	
	information_panel = NAUTILUS_INFORMATION_PANEL (g_object_get_data (G_OBJECT (button), "user_data"));
	
	g_return_if_fail (information_panel->details->file != NULL);

	nautilus_choose_application_for_file
		(information_panel->details->file,
		 GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (information_panel))),
		 nautilus_information_panel_chose_application_callback,
		 information_panel);
}

/* utility routine that allocates the command buttons from the command list */

static void
add_command_buttons (NautilusInformationPanel *information_panel, GList *application_list)
{
	char *id_string, *temp_str, *file_path;
	GList *p;
	GtkWidget *temp_button;
	GnomeVFSMimeApplication *application;

	/* There's always at least the "Open with..." button */
	information_panel->details->has_buttons = TRUE;

	for (p = application_list; p != NULL; p = p->next) {
	        application = p->data;	        

		temp_str = g_strdup_printf (_("Open with %s"), application->name);
	        temp_button = gtk_button_new_with_label (temp_str);
		g_free (temp_str);
		gtk_box_pack_start (GTK_BOX (information_panel->details->button_box), 
				    temp_button, 
				    FALSE, FALSE, 
				    0);

		/* Get the local path, if there is one */
		file_path = gnome_vfs_get_local_path_from_uri (information_panel->details->uri);
		if (file_path == NULL) {
			file_path = g_strdup (information_panel->details->uri);
		} 

		temp_str = g_shell_quote (file_path);		
		id_string = eel_str_replace_substring (application->id, "%s", temp_str); 		
		g_free (file_path);
		g_free (temp_str);

		eel_gtk_signal_connect_free_data 
			(GTK_OBJECT (temp_button), "clicked",
			 G_CALLBACK (command_button_callback), id_string);

                g_object_set_data (G_OBJECT (temp_button), "user_data", information_panel);
		
		gtk_widget_show (temp_button);
	}

	/* Catch-all button after all the others. */
	temp_button = gtk_button_new_with_label (_("Open with..."));
	g_signal_connect (temp_button, "clicked",
			  G_CALLBACK (open_with_callback), NULL);
	g_object_set_data (G_OBJECT (temp_button), "user_data", information_panel);
	gtk_widget_show (temp_button);
	gtk_box_pack_start (GTK_BOX (information_panel->details->button_box),
			    temp_button, FALSE, FALSE, 0);
}

/* utility to construct command buttons for the information_panel from the passed in metadata string */

static void
add_buttons_from_metadata (NautilusInformationPanel *information_panel, const char *button_data)
{
	char **terms;
	char *current_term, *temp_str;
	char *button_name, *command_string;
	const char *term;
	int index;
	GtkWidget *temp_button;
	
	/* split the button specification into a set of terms */	
	button_name = NULL;
	terms = g_strsplit (button_data, ";", 0);	
	
	/* for each term, either create a button or attach a property to one */
	for (index = 0; (term = terms[index]) != NULL; index++) {
		current_term = g_strdup (term);
		temp_str = strchr (current_term, '=');
		if (temp_str) {
			*temp_str = '\0';
			if (!g_ascii_strcasecmp (current_term, "button")) {
				button_name = g_strdup (temp_str + 1);
			} else if (!g_ascii_strcasecmp (current_term, "script")) {
			        if (button_name != NULL) {
			        	temp_button = gtk_button_new_with_label (button_name);		    
					gtk_box_pack_start (GTK_BOX (information_panel->details->button_box), 
							    temp_button, 
							    FALSE, FALSE, 
							    0);
					information_panel->details->has_buttons = TRUE;
					command_string = g_strdup (temp_str + 1);
					g_free (button_name);
					
					eel_gtk_signal_connect_free_data 
						(GTK_OBJECT (temp_button), "clicked",
						 G_CALLBACK (metadata_button_callback), command_string);
		                	g_object_set_data (G_OBJECT (temp_button), "user_data", information_panel);
					
					gtk_widget_show (temp_button);			
				}
			}
		}
		g_free(current_term);
	}
	g_strfreev (terms);
}

/* handle the hacked-in empty trash command */
static void
empty_trash_callback (GtkWidget *button, gpointer data)
{
	GtkWidget *window;
	
	window = gtk_widget_get_toplevel (button);
	nautilus_file_operations_empty_trash (window);
}

static void
nautilus_information_panel_trash_state_changed_callback (NautilusTrashMonitor *trash_monitor,
						gboolean state, gpointer callback_data)
{
		gtk_widget_set_sensitive (GTK_WIDGET (callback_data), !nautilus_trash_monitor_is_empty ());
}

/*
 * nautilus_information_panel_update_buttons:
 * 
 * Update the list of program-launching buttons based on the current uri.
 */
static void
nautilus_information_panel_update_buttons (NautilusInformationPanel *information_panel)
{
	char *button_data;
	GtkWidget *temp_button;
	GList *short_application_list;
	
	/* dispose of any existing buttons */
	if (information_panel->details->has_buttons) {
		gtk_container_remove (GTK_CONTAINER (information_panel->details->container),
				      GTK_WIDGET (information_panel->details->button_box_centerer)); 
		make_button_box (information_panel);
	}

	/* create buttons from file metadata if necessary */
	button_data = nautilus_file_get_metadata (information_panel->details->file,
						  NAUTILUS_METADATA_KEY_SIDEBAR_BUTTONS,
						  NULL);
	if (button_data) {
		add_buttons_from_metadata (information_panel, button_data);
		g_free(button_data);
	}

	/* here is a hack to provide an "empty trash" button when displaying the trash.  Eventually, we
	 * need a framework to allow protocols to add commands buttons */
	if (eel_istr_has_prefix (information_panel->details->uri, "trash:")) {
		/* FIXME: We don't use spaces to pad labels! */
		temp_button = gtk_button_new_with_mnemonic (_("Empty _Trash"));

		gtk_box_pack_start (GTK_BOX (information_panel->details->button_box), 
					temp_button, FALSE, FALSE, 0);
		gtk_widget_set_sensitive (temp_button, !nautilus_trash_monitor_is_empty ());
		gtk_widget_show (temp_button);
		information_panel->details->has_buttons = TRUE;
					
		g_signal_connect (temp_button, "clicked",
				  G_CALLBACK (empty_trash_callback), NULL);
		
		g_signal_connect_object (nautilus_trash_monitor_get (), "trash_state_changed",
					 G_CALLBACK (nautilus_information_panel_trash_state_changed_callback), temp_button, 0);
	}
	
	/* Make buttons for each item in short list + "Open with..." catchall,
	 * unless there aren't any applications at all in complete list. 
	 */

	if (nautilus_mime_has_any_applications_for_file (information_panel->details->file)) {
		short_application_list = 
			nautilus_mime_get_short_list_applications_for_file (information_panel->details->file);
		add_command_buttons (information_panel, short_application_list);
		gnome_vfs_mime_application_list_free (short_application_list);
	}

	gtk_widget_show (GTK_WIDGET (information_panel->details->button_box_centerer));
}

static void
nautilus_information_panel_update_appearance (NautilusInformationPanel *information_panel)
{
	EelBackground *background;
	char *background_color;
	char *background_image;

	g_return_if_fail (NAUTILUS_IS_INFORMATION_PANEL (information_panel));
	
	/* Connect the background changed signal to code that writes the color. */
	background = eel_get_widget_background (GTK_WIDGET (information_panel));
	if (!information_panel->details->background_connected) {
		information_panel->details->background_connected = TRUE;
		g_signal_connect_object (background,"settings_changed",
					 G_CALLBACK (background_settings_changed_callback), information_panel, 0);
		g_signal_connect_object (background, "reset",
					 G_CALLBACK (background_reset_callback), information_panel, 0);
	}
	
	/* Set up the background color and image from the metadata. */
	background_color = nautilus_file_get_metadata (information_panel->details->file,
						       NAUTILUS_METADATA_KEY_SIDEBAR_BACKGROUND_COLOR,
						       NULL);
	background_image = nautilus_file_get_metadata (information_panel->details->file,
						       NAUTILUS_METADATA_KEY_SIDEBAR_BACKGROUND_IMAGE,
						       NULL);

	if (background_color == NULL && background_image == NULL) {
		background_color = g_strdup (information_panel->details->default_background_color);
		background_image = g_strdup (information_panel->details->default_background_image);
	}
		
	/* Block so we don't write these settings out in response to our set calls below */
	g_signal_handlers_block_by_func (background,
					 G_CALLBACK (background_settings_changed_callback),
					 information_panel);

	if (value_different (information_panel->details->current_background_color, background_color) ||
	    value_different (information_panel->details->current_background_image, background_image)) {
		
		g_free (information_panel->details->current_background_color);
		information_panel->details->current_background_color = g_strdup (background_color);
		g_free (information_panel->details->current_background_image);
		information_panel->details->current_background_image = g_strdup (background_image);

		eel_background_set_image_uri (background, background_image);
		eel_background_set_color (background, background_color);

		nautilus_sidebar_title_select_text_color
			(information_panel->details->title, background,
			 !information_panel_has_background (information_panel));
	}

	g_free (background_color);
	g_free (background_image);	

	g_signal_handlers_unblock_by_func (background,
					   G_CALLBACK (background_settings_changed_callback),
					   information_panel);
}


static void
background_metadata_changed_callback (NautilusInformationPanel *information_panel)
{
	GList *attributes;
	gboolean ready;

	attributes = nautilus_mime_actions_get_minimum_file_attributes ();
	ready = nautilus_file_check_if_ready (information_panel->details->file, attributes);
	g_list_free (attributes);

	if (ready) {
		nautilus_information_panel_update_appearance (information_panel);
		
		/* set up the command buttons */
		nautilus_information_panel_update_buttons (information_panel);
	}
}

/* here is the key routine that populates the information_panel with the appropriate information when the uri changes */

void
nautilus_information_panel_set_uri (NautilusInformationPanel *information_panel, 
			  const char* new_uri,
			  const char* initial_title)
{       
	NautilusFile *file;
	GList *attributes;

	g_return_if_fail (NAUTILUS_IS_INFORMATION_PANEL (information_panel));
	g_return_if_fail (new_uri != NULL);
	g_return_if_fail (initial_title != NULL);

	/* there's nothing to do if the uri is the same as the current one */ 
	if (eel_strcmp (information_panel->details->uri, new_uri) == 0) {
		return;
	}
	
	g_free (information_panel->details->uri);
	information_panel->details->uri = g_strdup (new_uri);
		
	if (information_panel->details->file != NULL) {
		g_signal_handler_disconnect (information_panel->details->file, 
					     information_panel->details->file_changed_connection);
		nautilus_file_monitor_remove (information_panel->details->file, information_panel);
	}


	file = nautilus_file_get (information_panel->details->uri);

	nautilus_file_unref (information_panel->details->file);

	information_panel->details->file = file;
	
	information_panel->details->file_changed_connection =
		g_signal_connect_object (information_panel->details->file, "changed",
					 G_CALLBACK (background_metadata_changed_callback),
					 information_panel, G_CONNECT_SWAPPED);

	attributes = nautilus_mime_actions_get_minimum_file_attributes ();
	nautilus_file_monitor_add (information_panel->details->file, information_panel, attributes);
	g_list_free (attributes);

	background_metadata_changed_callback (information_panel);

	/* tell the title widget about it */
	nautilus_sidebar_title_set_file (information_panel->details->title,
					 information_panel->details->file,
					 initial_title);
}

void
nautilus_information_panel_set_title (NautilusInformationPanel *information_panel, const char* new_title)
{       
	nautilus_sidebar_title_set_text (information_panel->details->title,
					 new_title);
}

/* ::style_set handler for the information_panel */
static void
nautilus_information_panel_style_set (GtkWidget *widget, GtkStyle *previous_style)
{
	NautilusInformationPanel *information_panel;

	information_panel = NAUTILUS_INFORMATION_PANEL (widget);

	nautilus_information_panel_theme_changed (information_panel);
}

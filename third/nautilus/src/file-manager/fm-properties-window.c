/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* fm-properties-window.c - window that lets user modify file properties

   Copyright (C) 2000 Eazel, Inc.

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Authors: Darin Adler <darin@bentspoon.com>
*/

#include <config.h>
#include "fm-properties-window.h"

#include "fm-error-reporting.h"
#include <eel/eel-ellipsizing-label.h>
#include <eel/eel-gdk-pixbuf-extensions.h>
#include <eel/eel-glib-extensions.h>
#include <eel/eel-gnome-extensions.h>
#include <eel/eel-gtk-extensions.h>
#include <eel/eel-labeled-image.h>
#include <eel/eel-stock-dialogs.h>
#include <eel/eel-string.h>
#include <eel/eel-vfs-extensions.h>
#include <eel/eel-wrap-table.h>
#include <gtk/gtkalignment.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkdnd.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkfilesel.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkhbbox.h>
#include <gtk/gtkhseparator.h>
#include <gtk/gtkimage.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtknotebook.h>
#include <gtk/gtkoptionmenu.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkstock.h>
#include <gtk/gtktable.h>
#include <gtk/gtkvbox.h>
#include <libegg/egg-screen-help.h>
#include <libgnome/gnome-i18n.h>
#include <libgnome/gnome-macros.h>
#include <libgnomeui/gnome-dialog.h>
#include <libgnomeui/gnome-uidefs.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libnautilus-private/nautilus-customization-data.h>
#include <libnautilus-private/nautilus-entry.h>
#include <libnautilus-private/nautilus-file-attributes.h>
#include <libnautilus-private/nautilus-global-preferences.h>
#include <libnautilus-private/nautilus-icon-factory.h>
#include <libnautilus-private/nautilus-emblem-utils.h>
#include <libnautilus-private/nautilus-link.h>
#include <libnautilus-private/nautilus-metadata.h>
#include <libnautilus-private/nautilus-undo-signal-handlers.h>
#include <libnautilus-private/nautilus-mime-actions.h>
#include <libnautilus-private/nautilus-view-identifier.h>
#include <libnautilus/nautilus-undo.h>
#include <libnautilus/nautilus-view.h>
#include <bonobo/bonobo-widget.h>
#include <bonobo/bonobo-exception.h>
#include <string.h>

static GHashTable *windows;
static GHashTable *pending_files;

struct FMPropertiesWindowDetails {
	NautilusFile *original_file;
	NautilusFile *target_file;
	GtkNotebook *notebook;
	GtkWidget *remove_image_button;
	
	guint file_changed_handler_id;

	GtkTable *basic_table;
	GtkTable *permissions_table;

	NautilusEntry *name_field;
	char *pending_name;

	GtkLabel *directory_contents_title_field;
	GtkLabel *directory_contents_value_field;
	guint update_directory_contents_timeout_id;

	GList *directory_contents_widgets;
	int directory_contents_row;

	GList *special_flags_widgets;
	int first_special_flags_row;
	int num_special_flags_rows;

	gboolean deep_count_finished;
};

enum {
	PERMISSIONS_CHECKBOXES_OWNER_ROW,
	PERMISSIONS_CHECKBOXES_GROUP_ROW,
	PERMISSIONS_CHECKBOXES_OTHERS_ROW,
	PERMISSIONS_CHECKBOXES_ROW_COUNT
};

enum {
	PERMISSIONS_CHECKBOXES_READ_COLUMN,
	PERMISSIONS_CHECKBOXES_WRITE_COLUMN,
	PERMISSIONS_CHECKBOXES_EXECUTE_COLUMN,
	PERMISSIONS_CHECKBOXES_COLUMN_COUNT
};

enum {
	TITLE_COLUMN,
	VALUE_COLUMN,
	COLUMN_COUNT
};

typedef struct {
	NautilusFile *original_file;
	NautilusFile *target_file;
	FMDirectoryView *directory_view;
} StartupData;

typedef struct {
	FMPropertiesWindow *window;
	GtkWidget *vbox;
	char *view_name;
} ActivationData;


/* drag and drop definitions */

enum {
	TARGET_URI_LIST,
	TARGET_GNOME_URI_LIST,
	TARGET_RESET_BACKGROUND
};

static GtkTargetEntry target_table[] = {
	{ "text/uri-list",  0, TARGET_URI_LIST },
	{ "x-special/gnome-icon-list",  0, TARGET_GNOME_URI_LIST },
	{ "x-special/gnome-reset-background", 0, TARGET_RESET_BACKGROUND }
};

#define DIRECTORY_CONTENTS_UPDATE_INTERVAL	200 /* milliseconds */
#define STANDARD_EMBLEM_HEIGHT			52
#define EMBLEM_LABEL_SPACING			2

static void create_properties_window_callback     (NautilusFile		   *file,
						   gpointer                 data);
static void cancel_group_change_callback          (gpointer                 callback_data);
static void cancel_owner_change_callback          (gpointer                 callback_data);
static void directory_view_destroyed_callback     (FMDirectoryView         *view,
						   gpointer                 callback_data);
static void select_image_button_callback          (GtkWidget               *widget,
						   FMPropertiesWindow      *properties_window);
static void set_icon_callback                     (const char* icon_path, 
						   FMPropertiesWindow *properties_window);
static void remove_image_button_callback          (GtkWidget               *widget,
						   FMPropertiesWindow      *properties_window);
static void remove_pending_file                   (StartupData             *data,
						   gboolean                 cancel_call_when_ready,
						   gboolean                 cancel_timed_wait,
						   gboolean                 cancel_destroy_handler);

static void append_bonobo_pages                   (FMPropertiesWindow *window);

GNOME_CLASS_BOILERPLATE (FMPropertiesWindow, fm_properties_window,
			 GtkWindow, GTK_TYPE_WINDOW);

typedef struct {
	NautilusFile *file;
	char *name;
} FileNamePair;

static FileNamePair *
file_name_pair_new (NautilusFile *file, const char *name)
{
	FileNamePair *new_pair;

	new_pair = g_new0 (FileNamePair, 1);
	new_pair->file = file;
	new_pair->name = g_strdup (name);

	nautilus_file_ref (file);

	return new_pair;
}

static void
file_name_pair_free (FileNamePair *pair)
{
	nautilus_file_unref (pair->file);
	g_free (pair->name);
	
	g_free (pair);
}

static void
add_prompt (GtkVBox *vbox, const char *prompt_text, gboolean pack_at_start)
{
	GtkWidget *prompt;

	prompt = gtk_label_new (prompt_text);
   	gtk_label_set_justify (GTK_LABEL (prompt), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (prompt), TRUE);
	gtk_widget_show (prompt);
	if (pack_at_start) {
		gtk_box_pack_start (GTK_BOX (vbox), prompt, FALSE, FALSE, 0);
	} else {
		gtk_box_pack_end (GTK_BOX (vbox), prompt, FALSE, FALSE, 0);
	}
}

static void
add_prompt_and_separator (GtkVBox *vbox, const char *prompt_text)
{
	GtkWidget *separator_line;

	add_prompt (vbox, prompt_text, FALSE);

 	separator_line = gtk_hseparator_new ();
  	gtk_widget_show (separator_line);
  	gtk_box_pack_end (GTK_BOX (vbox), separator_line, TRUE, TRUE, GNOME_PAD_BIG);
}		   

static GdkPixbuf *
get_pixbuf_for_properties_window (NautilusFile *file)
{
	g_assert (NAUTILUS_IS_FILE (file));
	
	return nautilus_icon_factory_get_pixbuf_for_file (file, NULL, NAUTILUS_ICON_SIZE_STANDARD);
}

static void
update_properties_window_icon (GtkImage *image)
{
	GdkPixbuf	*pixbuf;
	NautilusFile	*file;

	file = g_object_get_data (G_OBJECT (image), "nautilus_file");

	g_assert (NAUTILUS_IS_FILE (file));
	
	pixbuf = get_pixbuf_for_properties_window (file);

	gtk_image_set_from_pixbuf (image, pixbuf);
	
	g_object_unref (pixbuf);
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
reset_icon (FMPropertiesWindow *properties_window)
{
	NautilusFile *file;
	
	file = properties_window->details->original_file;
	
	nautilus_file_set_metadata (file, NAUTILUS_METADATA_KEY_CUSTOM_ICON, NULL, NULL);
	nautilus_file_set_metadata (file, NAUTILUS_METADATA_KEY_ICON_SCALE, NULL, NULL);
	
	gtk_widget_set_sensitive (properties_window->details->remove_image_button, FALSE);
}

static void  
fm_properties_window_drag_data_received (GtkWidget *widget, GdkDragContext *context,
					 int x, int y,
					 GtkSelectionData *selection_data,
					 guint info, guint time)
{
	char **uris;
	gboolean exactly_one;
	GtkImage *image;
 	GtkWindow *window; 

	image = GTK_IMAGE (widget);
 	window = GTK_WINDOW (gtk_widget_get_toplevel (GTK_WIDGET (image)));

	if (info == TARGET_RESET_BACKGROUND) {
		reset_icon (FM_PROPERTIES_WINDOW (window));
		
		return;
	}
	
	uris = g_strsplit (selection_data->data, "\r\n", 0);
	exactly_one = uris[0] != NULL && (uris[1] == NULL || uris[1][0] == '\0');


	if (!exactly_one) {
		eel_show_error_dialog
			(_("You can't assign more than one custom icon at a time! "
			   "Please drag just one image to set a custom icon."), 
			 _("More Than One Image"),
			 window);
	} else {		
		if (uri_is_local_image (uris[0])) {			
			set_icon_callback (gnome_vfs_get_local_path_from_uri (uris[0]), 
					   FM_PROPERTIES_WINDOW (window));
		} else {	
			if (eel_is_remote_uri (uris[0])) {
				eel_show_error_dialog
					(_("The file that you dropped is not local.  "
					   "You can only use local images as custom icons."), 
					 _("Local Images Only"),
					 window);
				
			} else {
				eel_show_error_dialog
					(_("The file that you dropped is not an image.  "
					   "You can only use local images as custom icons."),
					 _("Images Only"),
					 window);
			}
		}		
	}
	g_strfreev (uris);
}

static GtkWidget *
create_image_widget_for_file (NautilusFile *file)
{
 	GtkWidget *image;
	GdkPixbuf *pixbuf;
	
	pixbuf = get_pixbuf_for_properties_window (file);
	
	image = gtk_image_new ();

	/* prepare the image to receive dropped objects to assign custom images */
	gtk_drag_dest_set (GTK_WIDGET (image),
			   GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT | GTK_DEST_DEFAULT_DROP, 
			   target_table, G_N_ELEMENTS (target_table),
			   GDK_ACTION_COPY | GDK_ACTION_MOVE);

	g_signal_connect (image, "drag_data_received",
			  G_CALLBACK (fm_properties_window_drag_data_received), NULL);


	gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);

	g_object_unref (pixbuf);

	nautilus_file_ref (file);
	g_object_set_data_full (G_OBJECT (image), "nautilus_file",
				file, (GDestroyNotify) nautilus_file_unref);

	/* React to icon theme changes. */
	g_signal_connect_object (nautilus_icon_factory_get (),
				 "icons_changed",
				 G_CALLBACK (update_properties_window_icon),
				 image, G_CONNECT_SWAPPED);

	/* Name changes can also change icon (since name is determined by MIME type) */
	g_signal_connect_object (file, "changed",
				 G_CALLBACK (update_properties_window_icon),
				 image, G_CONNECT_SWAPPED);
	return image;
}

static void
name_field_update_to_match_file (NautilusEntry *name_field)
{
	NautilusFile *file;
	const char *original_name;
	char *current_name, *displayed_name;

	file = g_object_get_data (G_OBJECT (name_field), "nautilus_file");

	if (file == NULL || nautilus_file_is_gone (file)) {
		gtk_widget_set_sensitive (GTK_WIDGET (name_field), FALSE);
		gtk_entry_set_text (GTK_ENTRY (name_field), "");
		return;
	}

	original_name = (const char *) g_object_get_data (G_OBJECT (name_field),
						      	    "original_name");

	/* If the file name has changed since the original name was stored,
	 * update the text in the text field, possibly (deliberately) clobbering
	 * an edit in progress. If the name hasn't changed (but some other
	 * aspect of the file might have), then don't clobber changes.
	 */
	current_name = nautilus_file_get_display_name (file);
	if (eel_strcmp (original_name, current_name) != 0) {
		g_object_set_data_full (G_OBJECT (name_field),
					"original_name",
					current_name,
					g_free);

		/* Only reset the text if it's different from what is
		 * currently showing. This causes minimal ripples (e.g.
		 * selection change).
		 */
		displayed_name = gtk_editable_get_chars (GTK_EDITABLE (name_field), 0, -1);
		if (strcmp (displayed_name, current_name) != 0) {
			gtk_entry_set_text (GTK_ENTRY (name_field), current_name);
		}
		g_free (displayed_name);
	} else {
		g_free (current_name);
	}

	/* 
	 * The UI would look better here if the name were just drawn as
	 * a plain label in the case where it's not editable, with no
	 * border at all. That doesn't seem to be possible with GtkEntry,
	 * so we'd have to swap out the widget to achieve it. I don't
	 * care enough to change this now.
	 */
	gtk_widget_set_sensitive (GTK_WIDGET (name_field), 
				  nautilus_file_can_rename (file));
}

static void
name_field_restore_original_name (NautilusEntry *name_field)
{
	const char *original_name;
	char *displayed_name;

	original_name = (const char *) g_object_get_data (G_OBJECT (name_field),
							  "original_name");
	displayed_name = gtk_editable_get_chars (GTK_EDITABLE (name_field), 0, -1);

	if (strcmp (original_name, displayed_name) != 0) {
		gtk_entry_set_text (GTK_ENTRY (name_field), original_name);
	}
	nautilus_entry_select_all (name_field);

	g_free (displayed_name);
}

static void
rename_callback (NautilusFile *file, GnomeVFSResult result, gpointer callback_data)
{
	FMPropertiesWindow *window;
	char *new_name;

	window = FM_PROPERTIES_WINDOW (callback_data);

	/* Complain to user if rename failed. */
	if (result != GNOME_VFS_OK) {
		new_name = window->details->pending_name;
		fm_report_error_renaming_file (file, 
					       window->details->pending_name, 
					       result,
					       GTK_WINDOW (window));
		if (window->details->name_field != NULL) {
			name_field_restore_original_name (window->details->name_field);
		}
	}

	g_object_unref (window);
}

static void
set_pending_name (FMPropertiesWindow *window, const char *name)
{
	g_free (window->details->pending_name);
	window->details->pending_name = g_strdup (name);
}

static void
name_field_done_editing (NautilusEntry *name_field, FMPropertiesWindow *window)
{
	NautilusFile *file;
	char *new_name;
	
	g_assert (NAUTILUS_IS_ENTRY (name_field));

	file = g_object_get_data (G_OBJECT (name_field), "nautilus_file");

	g_assert (NAUTILUS_IS_FILE (file));

	/* This gets called when the window is closed, which might be
	 * caused by the file having been deleted.
	 */
	if (nautilus_file_is_gone (file)) {
		return;
	}

	new_name = gtk_editable_get_chars (GTK_EDITABLE (name_field), 0, -1);

	/* Special case: silently revert text if new text is empty. */
	if (strlen (new_name) == 0) {
		name_field_restore_original_name (NAUTILUS_ENTRY (name_field));
	} else {
		set_pending_name (window, new_name);
		g_object_ref (window);
		nautilus_file_rename (file, new_name,
				      rename_callback, window);
	}

	g_free (new_name);
}

static gboolean
name_field_focus_out (NautilusEntry *name_field,
		      GdkEventFocus *event,
		      gpointer callback_data)
{
	g_assert (FM_IS_PROPERTIES_WINDOW (callback_data));

	if (GTK_WIDGET_SENSITIVE (name_field)) {
		name_field_done_editing (name_field, FM_PROPERTIES_WINDOW (callback_data));
	}

	return FALSE;
}

static void
name_field_activate (NautilusEntry *name_field, gpointer callback_data)
{
	g_assert (NAUTILUS_IS_ENTRY (name_field));
	g_assert (FM_IS_PROPERTIES_WINDOW (callback_data));

	/* Accept changes. */
	name_field_done_editing (name_field, FM_PROPERTIES_WINDOW (callback_data));

	nautilus_entry_select_all_at_idle (name_field);
}

static void
property_button_update (GtkToggleButton *button)
{
	NautilusFile *file;
	char *name;
	GList *keywords, *word;

	file = g_object_get_data (G_OBJECT (button), "nautilus_file");
	name = g_object_get_data (G_OBJECT (button), "nautilus_property_name");

	/* Handle the case where there's nothing to toggle. */
	if (file == NULL || nautilus_file_is_gone (file) || name == NULL) {
		gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);
		gtk_toggle_button_set_active (button, FALSE);
		return;
	}

	/* Check and see if it's in there. */
	keywords = nautilus_file_get_keywords (file);
	word = g_list_find_custom (keywords, name, (GCompareFunc) strcmp);
	gtk_toggle_button_set_active (button, word != NULL);
}

static void
property_button_toggled (GtkToggleButton *button)
{
	NautilusFile *file;
	char *name;
	GList *keywords, *word;

	file = g_object_get_data (G_OBJECT (button), "nautilus_file");
	name = g_object_get_data (G_OBJECT (button), "nautilus_property_name");

	/* Handle the case where there's nothing to toggle. */
	if (file == NULL || nautilus_file_is_gone (file) || name == NULL) {
		return;
	}

	/* Check and see if it's already there. */
	keywords = nautilus_file_get_keywords (file);
	word = g_list_find_custom (keywords, name, (GCompareFunc) strcmp);
	if (gtk_toggle_button_get_active (button)) {
		if (word == NULL) {
			keywords = g_list_prepend (keywords, g_strdup (name));
		}
	} else {
		if (word != NULL) {
			keywords = g_list_remove_link (keywords, word);
			eel_g_list_free_deep (word);
		}
	}
	nautilus_file_set_keywords (file, keywords);
	eel_g_list_free_deep (keywords);
}

static void
update_properties_window_title (GtkWindow *window, NautilusFile *file)
{
	char *name, *title;

	g_assert (NAUTILUS_IS_FILE (file));
	g_assert (GTK_IS_WINDOW (window));

	name = nautilus_file_get_display_name (file);
	title = g_strdup_printf (_("%s Properties"), name);
  	gtk_window_set_title (window, title);

	g_free (name);	
	g_free (title);
}

static void
clear_bonobo_pages (FMPropertiesWindow *window)
{
	int i;
	int num_pages;
	GtkWidget *page;

	num_pages = gtk_notebook_get_n_pages
				(GTK_NOTEBOOK (window->details->notebook));

	for (i=0; i <  num_pages; i++) {
		page = gtk_notebook_get_nth_page
				(GTK_NOTEBOOK (window->details->notebook), i);

		if (g_object_get_data (G_OBJECT (page), "is-bonobo-page")) {
			gtk_notebook_remove_page
				(GTK_NOTEBOOK (window->details->notebook), i);
			num_pages--;
			i--;
		}
	}
}

static void
refresh_bonobo_pages (FMPropertiesWindow *window)
{
	clear_bonobo_pages (window);
	append_bonobo_pages (window);	
}

static void
properties_window_file_changed_callback (FMPropertiesWindow *window, NautilusFile *file)
{
	g_assert (GTK_IS_WINDOW (window));
	g_assert (NAUTILUS_IS_FILE (file));

	if (nautilus_file_is_gone (file)) {
		gtk_widget_destroy (GTK_WIDGET (window));
	} else {
		char *orig_mime_type;
		char *new_mime_type;

		orig_mime_type = nautilus_file_get_mime_type
						(window->details->target_file);
		nautilus_file_unref (window->details->target_file);

		window->details->target_file = nautilus_file_ref (file);

		update_properties_window_title (GTK_WINDOW (window), file);

		new_mime_type = nautilus_file_get_mime_type
						(window->details->target_file);

		if (strcmp (orig_mime_type, new_mime_type) != 0) {
			refresh_bonobo_pages (window);
		}

		g_free (orig_mime_type);
		g_free (new_mime_type);
	}
}

static void
value_field_update_internal (GtkLabel *label, 
			     NautilusFile *file, 
			     gboolean ellipsize_text)
{
	const char *attribute_name;
	char *attribute_value;

	g_assert (GTK_IS_LABEL (label));
	g_assert (NAUTILUS_IS_FILE (file));
	g_assert (!ellipsize_text || EEL_IS_ELLIPSIZING_LABEL (label));

	attribute_name = g_object_get_data (G_OBJECT (label), "file_attribute");
	attribute_value = nautilus_file_get_string_attribute_with_default (file, attribute_name);

	if (ellipsize_text) {
		eel_ellipsizing_label_set_text (EEL_ELLIPSIZING_LABEL (label), attribute_value);
	} else {
		gtk_label_set_text (label, attribute_value);
	}
	g_free (attribute_value);	
}

static void
value_field_update (GtkLabel *label, NautilusFile *file)
{
	value_field_update_internal (label, file, FALSE);
}

static void
ellipsizing_value_field_update (GtkLabel *label, NautilusFile *file)
{
	value_field_update_internal (label, file, TRUE);
}

static GtkLabel *
attach_label (GtkTable *table,
	      int row,
	      int column,
	      const char *initial_text,
	      gboolean right_aligned,
	      gboolean bold,
	      gboolean ellipsize_text,
	      gboolean selectable,
	      gboolean mnemonic)
{
	GtkWidget *label_field;

	if (ellipsize_text) {
		label_field = eel_ellipsizing_label_new (initial_text);
	} else if (mnemonic) {
		label_field = gtk_label_new_with_mnemonic (initial_text);
	} else {
		label_field = gtk_label_new (initial_text);
	}

	if (selectable) {
		gtk_label_set_selectable (GTK_LABEL (label_field), TRUE);
	}
	
	if (bold) {
		eel_gtk_label_make_bold (GTK_LABEL (label_field));
	}
	gtk_misc_set_alignment (GTK_MISC (label_field), right_aligned ? 1 : 0, 0.5);
	gtk_widget_show (label_field);
	gtk_table_attach (table, label_field,
			  column, column + 1,
			  row, row + 1,
			  ellipsize_text
			    ? GTK_FILL | GTK_EXPAND
			    : GTK_FILL,
			  0,
			  0, 0);

	return GTK_LABEL (label_field);
}	      

static GtkLabel *
attach_value_label (GtkTable *table,
	      		  int row,
	      		  int column,
	      		  const char *initial_text)
{
	return attach_label (table, row, column, initial_text, FALSE, FALSE, FALSE, TRUE, FALSE);
}

static GtkLabel *
attach_ellipsizing_value_label (GtkTable *table,
				int row,
				int column,
				const char *initial_text)
{
	return attach_label (table, row, column, initial_text, FALSE, FALSE, TRUE, TRUE, FALSE);
}

static void
attach_value_field_internal (GtkTable *table,
			     int row,
			     int column,
			     NautilusFile *file,
			     const char *file_attribute_name,
			     gboolean ellipsize_text)
{
	GtkLabel *value_field;

	if (ellipsize_text) {
		value_field = attach_ellipsizing_value_label (table, row, column, "");
	} else {
		value_field = attach_value_label (table, row, column, "");
	}

	/* Stash a copy of the file attribute name in this field for the callback's sake. */
	g_object_set_data_full (G_OBJECT (value_field), "file_attribute",
				g_strdup (file_attribute_name), g_free);

	/* Fill in the value. */
	if (ellipsize_text) {
		ellipsizing_value_field_update (value_field, file);
	} else {
		value_field_update (value_field, file);
	}

	/* Connect to signal to update value when file changes. */
	g_signal_connect_object (file, "changed",
				 ellipsize_text
				 ? G_CALLBACK (ellipsizing_value_field_update)
				 : G_CALLBACK (value_field_update),
				 value_field, G_CONNECT_SWAPPED);
}			     

static void
attach_value_field (GtkTable *table,
		    int row,
		    int column,
		    NautilusFile *file,
		    const char *file_attribute_name)
{
	attach_value_field_internal (table, row, column, file, file_attribute_name, FALSE);
}

static void
attach_ellipsizing_value_field (GtkTable *table,
		    	  	int row,
		    		int column,
		    		NautilusFile *file,
		    		const char *file_attribute_name)
{
	attach_value_field_internal (table, row, column, file, file_attribute_name, TRUE);
}

static void
group_change_callback (NautilusFile *file, GnomeVFSResult result, gpointer callback_data)
{
	g_assert (callback_data == NULL);
	
	/* Report the error if it's an error. */
	eel_timed_wait_stop (cancel_group_change_callback, file);
	fm_report_error_setting_group (file, result, NULL);
	nautilus_file_unref (file);
}

static void
cancel_group_change_callback (gpointer callback_data)
{
	NautilusFile *file;

	file = NAUTILUS_FILE (callback_data);
	nautilus_file_cancel (file, group_change_callback, NULL);
	nautilus_file_unref (file);
}

static void
activate_group_callback (GtkMenuItem *menu_item, FileNamePair *pair)
{
	g_assert (pair != NULL);

	/* Try to change file group. If this fails, complain to user. */
	nautilus_file_ref (pair->file);
	eel_timed_wait_start
		(cancel_group_change_callback,
		 pair->file,
		 _("Cancel Group Change?"),
		 _("Changing group"),
		 NULL); /* FIXME bugzilla.gnome.org 42397: Parent this? */
	nautilus_file_set_group
		(pair->file, pair->name,
		 group_change_callback, NULL);
}

static GtkWidget *
create_group_menu_item (NautilusFile *file, const char *group_name)
{
	GtkWidget *menu_item;

	g_assert (NAUTILUS_IS_FILE (file));
	g_assert (group_name != NULL);

	menu_item = gtk_menu_item_new_with_label (group_name);
	gtk_widget_show (menu_item);

	eel_gtk_signal_connect_free_data_custom (GTK_OBJECT (menu_item),
						 "activate",
						 G_CALLBACK (activate_group_callback),
						 file_name_pair_new (file, group_name),
						 (GDestroyNotify)file_name_pair_free);

	return menu_item;
}

static void
synch_groups_menu (GtkOptionMenu *option_menu, NautilusFile *file)
{
	GList *groups;
	GList *node;
	GtkWidget *new_menu;
	GtkWidget *menu_item;
	const char *group_name;
	char *current_group_name;
	int group_index;
	int current_group_index;

	g_assert (GTK_IS_OPTION_MENU (option_menu));
	g_assert (NAUTILUS_IS_FILE (file));

	current_group_name = nautilus_file_get_string_attribute (file, "group");
	current_group_index = -1;

	groups = nautilus_file_get_settable_group_names (file);
	new_menu = gtk_menu_new ();

	for (node = groups, group_index = 0; node != NULL; node = node->next, ++group_index) {
		group_name = (const char *)node->data;
		if (strcmp (group_name, current_group_name) == 0) {
			current_group_index = group_index;
		}
		menu_item = create_group_menu_item (file, group_name);
		gtk_menu_shell_append (GTK_MENU_SHELL (new_menu), menu_item);
	}

	/* If current group wasn't in list, we prepend it (with a separator). 
	 * This can happen if the current group is an id with no matching
	 * group in the groups file.
	 */
	if (current_group_index < 0 && current_group_name != NULL) {
		if (groups != NULL) {
			menu_item = gtk_menu_item_new ();
			gtk_widget_show (menu_item);
			gtk_menu_shell_prepend (GTK_MENU_SHELL (new_menu), menu_item);
		}
		menu_item = create_group_menu_item (file, current_group_name);
		gtk_menu_shell_prepend (GTK_MENU_SHELL (new_menu), menu_item);
		current_group_index = 0;
	}

        /* We create and attach a new menu here because adding/removing
         * items from existing menu screws up the size of the option menu.
         */
        gtk_option_menu_set_menu (option_menu, new_menu);

	gtk_option_menu_set_history (option_menu, current_group_index);

	g_free (current_group_name);
	eel_g_list_free_deep (groups);
}

static GtkOptionMenu *
attach_option_menu (GtkTable *table,
		    int row)
{
	GtkWidget *option_menu;
	GtkWidget *aligner;

	option_menu = gtk_option_menu_new ();
	gtk_widget_show (option_menu);

	/* Put option menu in alignment to make it left-justified
	 * but minimally sized.
	 */	
	aligner = gtk_alignment_new (0, 0.5, 0, 0);
	gtk_widget_show (aligner);

	gtk_container_add (GTK_CONTAINER (aligner), option_menu);
	gtk_table_attach (table, aligner,
			  VALUE_COLUMN, VALUE_COLUMN + 1,
			  row, row + 1,
			  GTK_FILL, 0,
			  0, 0);

	return GTK_OPTION_MENU (option_menu);
}		    	

static GtkOptionMenu*
attach_group_menu (GtkTable *table,
		   int row,
		   NautilusFile *file)
{
	GtkOptionMenu *option_menu;

	option_menu = attach_option_menu (table, row);

	synch_groups_menu (option_menu, file);

	/* Connect to signal to update menu when file changes. */
	g_signal_connect_object (file, "changed",
				 G_CALLBACK (synch_groups_menu),
				 option_menu, G_CONNECT_SWAPPED);	
	return option_menu;
}	

static void
owner_change_callback (NautilusFile *file, GnomeVFSResult result, gpointer callback_data)
{
	g_assert (callback_data == NULL);
	
	/* Report the error if it's an error. */
	eel_timed_wait_stop (cancel_owner_change_callback, file);
	fm_report_error_setting_owner (file, result, NULL);
	nautilus_file_unref (file);
}

static void
cancel_owner_change_callback (gpointer callback_data)
{
	NautilusFile *file;

	file = NAUTILUS_FILE (callback_data);
	nautilus_file_cancel (file, owner_change_callback, NULL);
	nautilus_file_unref (file);
}

static void
activate_owner_callback (GtkMenuItem *menu_item, FileNamePair *pair)
{
	g_assert (GTK_IS_MENU_ITEM (menu_item));
	g_assert (pair != NULL);
	g_assert (NAUTILUS_IS_FILE (pair->file));
	g_assert (pair->name != NULL);

	/* Try to change file owner. If this fails, complain to user. */
	nautilus_file_ref (pair->file);
	eel_timed_wait_start
		(cancel_owner_change_callback,
		 pair->file,
		 _("Cancel Owner Change?"),
		 _("Changing owner"),
		 NULL); /* FIXME bugzilla.gnome.org 42397: Parent this? */
	nautilus_file_set_owner
		(pair->file, pair->name,
		 owner_change_callback, NULL);
}

static GtkWidget *
create_owner_menu_item (NautilusFile *file, const char *user_name)
{
	GtkWidget *menu_item;
	char **name_array;
	char *label_text;

	g_assert (NAUTILUS_IS_FILE (file));
	g_assert (user_name != NULL);

	name_array = g_strsplit (user_name, "\n", 2);
	if (name_array[1] != NULL) {
		label_text = g_strdup_printf ("%s - %s", name_array[0], name_array[1]);
	} else {
		label_text = g_strdup (name_array[0]);
	}

	menu_item = gtk_menu_item_new_with_label (label_text);
	g_free (label_text);

	gtk_widget_show (menu_item);

	eel_gtk_signal_connect_free_data_custom (GTK_OBJECT (menu_item),
						 "activate",
						 G_CALLBACK (activate_owner_callback),
						 file_name_pair_new (file, name_array[0]),
						 (GDestroyNotify)file_name_pair_free);
	g_strfreev (name_array);
	return menu_item;
}

static void
synch_user_menu (GtkOptionMenu *option_menu, NautilusFile *file)
{
	GList *users;
	GList *node;
	GtkWidget *new_menu;
	GtkWidget *menu_item;
	const char *user_name;
	char *owner_name;
	int user_index;
	int owner_index;

	g_assert (GTK_IS_OPTION_MENU (option_menu));
	g_assert (NAUTILUS_IS_FILE (file));

	owner_name = nautilus_file_get_string_attribute (file, "owner");
	owner_index = -1;

	users = nautilus_get_user_names ();
	new_menu = gtk_menu_new ();

	for (node = users, user_index = 0; node != NULL; node = node->next, ++user_index) {
		user_name = (const char *)node->data;
		if (strcmp (user_name, owner_name) == 0) {
			owner_index = user_index;
		}
		menu_item = create_owner_menu_item (file, user_name);
		gtk_menu_shell_append (GTK_MENU_SHELL (new_menu), menu_item);
	}

	/* If owner wasn't in list, we prepend it (with a separator). 
	 * This can happen if the owner is an id with no matching
	 * identifier in the passwords file.
	 */
	if (owner_index < 0 && owner_name != NULL) {
		if (users != NULL) {
			menu_item = gtk_menu_item_new ();
			gtk_widget_show (menu_item);
			gtk_menu_shell_prepend (GTK_MENU_SHELL (new_menu), menu_item);
		}
		menu_item = create_owner_menu_item (file, owner_name);
		gtk_menu_shell_prepend (GTK_MENU_SHELL (new_menu), menu_item);
		owner_index = 0;
	}

        /* We create and attach a new menu here because adding/removing
         * items from existing menu screws up the size of the option menu.
         */
        gtk_option_menu_set_menu (option_menu, new_menu);

	gtk_option_menu_set_history (option_menu, owner_index);

	g_free (owner_name);
	eel_g_list_free_deep (users);
}	

static void
attach_owner_menu (GtkTable *table,
		   int row,
		   NautilusFile *file)
{
	GtkOptionMenu *option_menu;

	option_menu = attach_option_menu (table, row);

	synch_user_menu (option_menu, file);

	/* Connect to signal to update menu when file changes. */
	g_signal_connect_object (file, "changed",
				 G_CALLBACK (synch_user_menu),
				 option_menu, G_CONNECT_SWAPPED);	
}

static guint
append_row (GtkTable *table)
{
	guint new_row_count;

	new_row_count = table->nrows + 1;

	gtk_table_resize (table, new_row_count, table->ncols);
	gtk_table_set_row_spacing (table, new_row_count - 1, GNOME_PAD);

	return new_row_count - 1;
}

static GtkWidget *
append_separator (GtkTable *table)
{
	GtkWidget *separator;
	guint last_row;

	last_row = append_row (table);
	separator = gtk_hseparator_new ();
	gtk_widget_show (separator);
	gtk_table_attach (table, separator,
			  TITLE_COLUMN, COLUMN_COUNT,
			  last_row, last_row+1,
			  GTK_FILL, 0,
			  0, 0);
	return separator;				   
}		  	
 
static void
directory_contents_value_field_update (FMPropertiesWindow *window)
{
	NautilusRequestStatus status;
	char *text, *temp;
	guint directory_count;
	guint file_count;
	guint total_count;
	guint unreadable_directory_count;
	GnomeVFSFileSize total_size;
	char *size_string;
	gboolean used_two_lines;
	NautilusFile *file;

	g_assert (FM_IS_PROPERTIES_WINDOW (window));

	file = window->details->target_file;
	g_assert (nautilus_file_is_directory (file) || nautilus_file_is_gone (file));

	status = nautilus_file_get_deep_counts (file, 
						&directory_count, 
						&file_count, 
						&unreadable_directory_count, 
						&total_size);

	/* If we've already displayed the total once, don't do another visible
	 * count-up if the deep_count happens to get invalidated. But still display
	 * the new total, since it might have changed.
	 */
	if (window->details->deep_count_finished && status != NAUTILUS_REQUEST_DONE) {
		return;
	}

	text = NULL;
	total_count = file_count + directory_count;
	used_two_lines = FALSE;

	if (total_count == 0) {
		switch (status) {
		case NAUTILUS_REQUEST_DONE:
			if (unreadable_directory_count == 0) {
				text = g_strdup (_("nothing"));
			} else {
				text = g_strdup (_("unreadable"));
			}
			break;
		default:
			text = g_strdup ("...");
		}
	} else {
		size_string = gnome_vfs_format_file_size_for_display (total_size);
		if (total_count == 1) {
				text = g_strdup_printf (_("1 item, with size %s"), size_string);
		} else {
			text = g_strdup_printf (_("%d items, totalling %s"), total_count, size_string);
		}
		g_free (size_string);

		if (unreadable_directory_count != 0) {
			temp = text;
			text = g_strconcat (temp, "\n", _("(some contents unreadable)"), NULL);
			g_free (temp);
			used_two_lines = TRUE;
		}
	}

	gtk_label_set_text (window->details->directory_contents_value_field, text);
	g_free (text);

	/* Also set the title field here, with a trailing carriage return & space
	 * if the value field has two lines. This is a hack to get the
	 * "Contents:" title to line up with the first line of the 2-line value.
	 * Maybe there's a better way to do this, but I couldn't think of one.
	 */
	text = g_strdup (_("Contents:"));
	if (used_two_lines) {
		temp = text;
		text = g_strconcat (temp, "\n ", NULL);
		g_free (temp);
	}
	gtk_label_set_text (window->details->directory_contents_title_field, text);
	g_free (text);

	if (status == NAUTILUS_REQUEST_DONE) {
		window->details->deep_count_finished = TRUE;
	}
}

static gboolean
update_directory_contents_callback (gpointer data)
{
	FMPropertiesWindow *window;

	window = FM_PROPERTIES_WINDOW (data);

	window->details->update_directory_contents_timeout_id = 0;
	directory_contents_value_field_update (window);

	return FALSE;
}

static void
schedule_directory_contents_update (FMPropertiesWindow *window)
{
	g_assert (FM_IS_PROPERTIES_WINDOW (window));

	if (window->details->update_directory_contents_timeout_id == 0) {
		window->details->update_directory_contents_timeout_id
			= gtk_timeout_add (DIRECTORY_CONTENTS_UPDATE_INTERVAL,
					   update_directory_contents_callback,
					   window);
	}
}

static GtkLabel *
attach_directory_contents_value_field (FMPropertiesWindow *window,
				       GtkTable *table,
				       int row)
{
	GtkLabel *value_field;

	value_field = attach_value_label (table, row, VALUE_COLUMN, "");

	g_assert (window->details->directory_contents_value_field == NULL);
	window->details->directory_contents_value_field = value_field;

	gtk_label_set_line_wrap (value_field, TRUE);

	/* Always recompute from scratch when the window is shown. */
	nautilus_file_recompute_deep_counts (window->details->target_file);

	/* Fill in the initial value. */
	directory_contents_value_field_update (window);

	/* Connect to signal to update value when file changes. */
	g_signal_connect_object (window->details->target_file,
				 "updated_deep_count_in_progress",
				 G_CALLBACK (schedule_directory_contents_update),
				 window, G_CONNECT_SWAPPED);
	
	return value_field;	
}					

static GtkLabel *
attach_title_field (GtkTable *table,
		     int row,
		     const char *title)
{
	return attach_label (table, row, TITLE_COLUMN, title, TRUE, TRUE, FALSE, FALSE, TRUE);
}		      

static guint
append_title_field (GtkTable *table, const char *title, GtkLabel **label)
{
	guint last_row;
	GtkLabel *title_label;

	last_row = append_row (table);
	title_label = attach_title_field (table, last_row, title);

	if (label) {
		*label = title_label;
	}

	return last_row;
}

static guint
append_title_value_pair (GtkTable *table, 
			 const char *title, 
			 NautilusFile *file, 
			 const char *file_attribute_name)
{
	guint last_row;

	last_row = append_title_field (table, title, NULL);
	attach_value_field (table, last_row, VALUE_COLUMN, file, file_attribute_name); 

	return last_row;
}

static guint
append_title_and_ellipsizing_value (GtkTable *table,
				    const char *title,
				    NautilusFile *file,
				    const char *file_attribute_name)
{
	guint last_row;

	last_row = append_title_field (table, title, NULL);
	attach_ellipsizing_value_field (table, last_row, VALUE_COLUMN, file, file_attribute_name);

	return last_row;
}

static void
update_visibility_of_table_rows (GtkTable *table,
		   	 	 gboolean should_show,
		   		 int first_row, 
		   		 int row_count,
		   		 GList *widgets)
{
	GList *node;
	int i;

	for (node = widgets; node != NULL; node = node->next) {
		if (should_show) {
			gtk_widget_show (GTK_WIDGET (node->data));
		} else {
			gtk_widget_hide (GTK_WIDGET (node->data));
		}
	}

	for (i= 0; i < row_count; ++i) {
		gtk_table_set_row_spacing (table, first_row + i, should_show ? GNOME_PAD : 0);
	}
}				   

static void
update_visibility_of_item_count_fields (FMPropertiesWindow *window)
{
	update_visibility_of_table_rows
		(window->details->basic_table,
		 nautilus_file_should_show_directory_item_count (window->details->target_file),
		 window->details->directory_contents_row,
		 1,
		 window->details->directory_contents_widgets);
}

static void
update_visibility_of_item_count_fields_wrapper (gpointer callback_data)
{
	update_visibility_of_item_count_fields (FM_PROPERTIES_WINDOW (callback_data));
}  

static void
remember_directory_contents_widget (FMPropertiesWindow *window, GtkWidget *widget)
{
	g_assert (FM_IS_PROPERTIES_WINDOW (window));
	g_assert (GTK_IS_WIDGET (widget));
	
	window->details->directory_contents_widgets = 
		g_list_prepend (window->details->directory_contents_widgets, widget);
}

static guint
append_directory_contents_fields (FMPropertiesWindow *window,
				  GtkTable *table)
{
	GtkLabel *title_field, *value_field;
	guint last_row;

	last_row = append_row (table);

	title_field = attach_title_field (table, last_row, "");
	window->details->directory_contents_title_field = title_field;
	gtk_label_set_line_wrap (title_field, TRUE);

	value_field = attach_directory_contents_value_field 
		(window, table, last_row);

	remember_directory_contents_widget (window, GTK_WIDGET (title_field));
	remember_directory_contents_widget (window, GTK_WIDGET (value_field));
	window->details->directory_contents_row = last_row;

	update_visibility_of_item_count_fields (window);
	eel_preferences_add_callback_while_alive 
		(NAUTILUS_PREFERENCES_SHOW_DIRECTORY_ITEM_COUNTS,
		 update_visibility_of_item_count_fields_wrapper,
		 window,
		 G_OBJECT (window));
	
	return last_row;
}

static GtkWidget *
create_page_with_vbox (GtkNotebook *notebook,
		       const char *title)
{
	GtkWidget *vbox;

	g_assert (GTK_IS_NOTEBOOK (notebook));
	g_assert (title != NULL);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), GNOME_PAD);
	gtk_notebook_append_page (notebook, vbox, gtk_label_new (title));

	return vbox;
}		       

static void
apply_standard_table_padding (GtkTable *table)
{
	gtk_table_set_row_spacings (table, GNOME_PAD);
	gtk_table_set_col_spacings (table, GNOME_PAD);	
}

static GtkWidget *
create_attribute_value_table (GtkVBox *vbox, int row_count)
{
	GtkWidget *table;

	table = gtk_table_new (row_count, COLUMN_COUNT, FALSE);
	apply_standard_table_padding (GTK_TABLE (table));
	gtk_widget_show (table);
	gtk_box_pack_start (GTK_BOX (vbox), table, FALSE, FALSE, 0);	

	return table;
}

static void
create_page_with_table_in_vbox (GtkNotebook *notebook, 
				const char *title, 
				int row_count, 
				GtkTable **return_table, 
				GtkWidget **return_vbox)
{
	GtkWidget *table;
	GtkWidget *vbox;

	vbox = create_page_with_vbox (notebook, title);
	table = create_attribute_value_table (GTK_VBOX (vbox), row_count);

	if (return_table != NULL) {
		*return_table = GTK_TABLE (table);
	}

	if (return_vbox != NULL) {
		*return_vbox = vbox;
	}
}		

static gboolean
is_merged_trash_directory (NautilusFile *file) 
{
	char *file_uri;
	gboolean result;

	file_uri = nautilus_file_get_uri (file);
	result = eel_uris_match (file_uri, EEL_TRASH_URI);
	g_free (file_uri);

	return result;
}

static gboolean
should_show_custom_icon_buttons (FMPropertiesWindow *window) 
{
	/* FIXME bugzilla.gnome.org 45642:
	 * Custom icons aren't displayed on the the desktop Trash icon, so
	 * we shouldn't pretend that they work by showing them here.
	 * When bug 5642 is fixed we can remove this case.
	 */
	if (is_merged_trash_directory (window->details->target_file)) {
		return FALSE;
	}

	return TRUE;
}

static gboolean
should_show_file_type (FMPropertiesWindow *window) 
{
	/* The trash on the desktop is one-of-a-kind */
	if (is_merged_trash_directory (window->details->target_file)) {
		return FALSE;
	}

	return TRUE;
}

static gboolean
should_show_accessed_date (FMPropertiesWindow *window) 
{
	/* Accessed date for directory seems useless. If we some
	 * day decide that it is useful, we should separately
	 * consider whether it's useful for "trash:".
	 */
	if (nautilus_file_is_directory (window->details->target_file)) {
		return FALSE;
	}

	return TRUE;
}

static gboolean
should_show_mime_type (FMPropertiesWindow *window) 
{
	/* FIXME bugzilla.gnome.org 45652:
	 * nautilus_file_is_directory should return TRUE for special
	 * trash directory, but doesn't. I could trivially fix this
	 * with a check for is_merged_trash_directory here instead.
	 */
	if (nautilus_file_is_directory (window->details->target_file)) {
		return FALSE;
	}

	return TRUE;
}

static gboolean
should_show_link_target (FMPropertiesWindow *window)
{
	if (nautilus_file_is_symbolic_link (window->details->target_file)) {
		return TRUE;
	}

	return FALSE;
}

static void
create_basic_page (FMPropertiesWindow *window)
{
	GtkTable *table;
	GtkWidget *container;
	GtkWidget *icon_pixmap_widget, *icon_aligner, *name_field;
	GtkWidget *button_box, *temp_button;
	char *image_uri;
	NautilusFile *target_file, *original_file;
	GtkWidget *hbox, *name_label;

	target_file = window->details->target_file;
	original_file = window->details->original_file;

	create_page_with_table_in_vbox (window->details->notebook, 
					_("Basic"), 
					1,
					&table, 
					&container);
	window->details->basic_table = table;
	
	/* Icon pixmap */
	hbox = gtk_hbox_new (FALSE, 4);
	gtk_widget_show (hbox);
	gtk_table_attach (table,
			  hbox,
			  TITLE_COLUMN, 
			  TITLE_COLUMN + 1,
			  0, 1,
			  0, 0,
			  0, 0);

	icon_pixmap_widget = create_image_widget_for_file (original_file);
	gtk_widget_show (icon_pixmap_widget);
	
	icon_aligner = gtk_alignment_new (1, 0.5, 0, 0);
	gtk_widget_show (icon_aligner);

	gtk_container_add (GTK_CONTAINER (icon_aligner), icon_pixmap_widget);
	gtk_box_pack_start (GTK_BOX (hbox), icon_aligner, TRUE, TRUE, 0);

	/* Name label */
	name_label = gtk_label_new_with_mnemonic (_("_Name:"));
	eel_gtk_label_make_bold (GTK_LABEL (name_label));
	gtk_widget_show (name_label);
	gtk_box_pack_end (GTK_BOX (hbox), name_label, FALSE, FALSE, 0);

	/* Name field */
	name_field = nautilus_entry_new ();
	window->details->name_field = NAUTILUS_ENTRY (name_field);
	gtk_widget_show (name_field);
	gtk_table_attach (table,
			  name_field,
			  VALUE_COLUMN, 
			  VALUE_COLUMN + 1,
			  0, 1,
			  GTK_FILL, 0,
			  0, 0);
	gtk_label_set_mnemonic_widget (GTK_LABEL (name_label), name_field);

	/* Attach parameters and signal handler. */
	nautilus_file_ref (original_file);
	g_object_set_data_full (G_OBJECT (name_field),"nautilus_file",
				original_file, (GDestroyNotify) nautilus_file_unref);

	/* Update name field initially before hooking up changed signal. */
	name_field_update_to_match_file (NAUTILUS_ENTRY (name_field));

/* FIXME bugzilla.gnome.org 42151:
 * With this (and one place elsewhere in this file, not sure which is the
 * trouble-causer) code in place, bug 2151 happens (crash on quit). Since
 * we've removed Undo from Nautilus for now, I'm just ifdeffing out this
 * code rather than trying to fix 2151 now. Note that it might be possible
 * to fix 2151 without making Undo actually work, it's just not worth the
 * trouble.
 */
#ifdef UNDO_ENABLED
	/* Set up name field for undo */
	nautilus_undo_set_up_nautilus_entry_for_undo ( NAUTILUS_ENTRY (name_field));
	nautilus_undo_editable_set_undo_key (GTK_EDITABLE (name_field), TRUE);
#endif

	g_signal_connect_object (name_field, "focus_out_event",
				 G_CALLBACK (name_field_focus_out), window, 0);                      			    
	g_signal_connect_object (name_field, "activate",
				 G_CALLBACK (name_field_activate), window, 0);

        /* Start with name field selected, if it's sensitive. */
        if (GTK_WIDGET_SENSITIVE (name_field)) {
		nautilus_entry_select_all (NAUTILUS_ENTRY (name_field));
	        gtk_widget_grab_focus (GTK_WIDGET (name_field));
        }
                      			    
	/* React to name changes from elsewhere. */
	g_signal_connect_object (target_file,
				 "changed",
				 G_CALLBACK (name_field_update_to_match_file),
				 name_field, G_CONNECT_SWAPPED);

	if (should_show_file_type (window)) {
		append_title_value_pair (table, _("Type:"), target_file, "type");
	}
	if (nautilus_file_is_directory (target_file)) {
		append_directory_contents_fields (window, table);
	} else {
		append_title_value_pair (table, _("Size:"), target_file, "size");
	}
	append_title_and_ellipsizing_value (table, _("Location:"), target_file, "where");
	if (should_show_link_target (window)) {
		append_title_and_ellipsizing_value (table, _("Link target:"), target_file, "link_target");
	}
	if (should_show_mime_type (window)) {
		append_title_value_pair (table, _("MIME type:"), target_file, "mime_type");
	}				  
	
	/* Blank title ensures standard row height */
	append_title_field (table, "", NULL);
	
	append_title_value_pair (table, _("Modified:"), target_file, "date_modified");

	if (should_show_accessed_date (window)) {
		append_title_value_pair (table, _("Accessed:"), target_file, "date_accessed");
	}

	if (should_show_custom_icon_buttons (window)) {
		/* add command buttons for setting and clearing custom icons */
		button_box = gtk_hbox_new (FALSE, 0);
		gtk_widget_show (button_box);
		gtk_box_pack_end (GTK_BOX(container), button_box, FALSE, FALSE, 4);  
		
	 	temp_button = gtk_button_new_with_mnemonic (_("_Select Custom Icon..."));
		gtk_widget_show (temp_button);
		gtk_box_pack_start (GTK_BOX (button_box), temp_button, FALSE, FALSE, 4);  

		g_signal_connect_object (temp_button, "clicked", G_CALLBACK (select_image_button_callback), window, 0);
	 	
	 	temp_button = gtk_button_new_with_mnemonic (_("_Remove Custom Icon"));
		gtk_widget_show (temp_button);
		gtk_box_pack_start (GTK_BOX(button_box), temp_button, FALSE, FALSE, 4);  

	 	g_signal_connect_object (temp_button, "clicked", G_CALLBACK (remove_image_button_callback), window, 0);

		window->details->remove_image_button = temp_button;
		
		/* de-sensitize the remove button if there isn't a custom image */
		image_uri = nautilus_file_get_metadata 
			(original_file, NAUTILUS_METADATA_KEY_CUSTOM_ICON, NULL);
		gtk_widget_set_sensitive (temp_button, image_uri != NULL);
		g_free (image_uri);
	}
}

static void
create_emblems_page (FMPropertiesWindow *window)
{
	GtkWidget *emblems_table, *button, *scroller;
	char *emblem_name;
	GdkPixbuf *pixbuf;
	char *label;
	NautilusFile *file;
	GList *icons, *l;

	
	file = window->details->target_file;

	/* The emblems wrapped table */
	scroller = eel_scrolled_wrap_table_new (TRUE, &emblems_table);

	gtk_container_set_border_width (GTK_CONTAINER (emblems_table), GNOME_PAD);
	
	gtk_widget_show (scroller);

	gtk_notebook_append_page (window->details->notebook, 
				  scroller, gtk_label_new (_("Emblems")));

	icons = nautilus_emblem_list_availible ();

	l = icons;
	while (l != NULL) {
		emblem_name = l->data;
		l = l->next;
		
		if (!nautilus_emblem_should_show_in_list (emblem_name)) {
			continue;
		}
		
		pixbuf = nautilus_icon_factory_get_pixbuf_from_name (emblem_name, NULL,
								     NAUTILUS_ICON_SIZE_SMALL,
								     &label);

		if (pixbuf == NULL) {
			continue;
		}

		if (label == NULL) {
			label = nautilus_emblem_get_keyword_from_icon_name (emblem_name);
		}
		
		button = eel_labeled_image_check_button_new (label, pixbuf);
		eel_labeled_image_set_fixed_image_height (EEL_LABELED_IMAGE (GTK_BIN (button)->child), STANDARD_EMBLEM_HEIGHT);
		eel_labeled_image_set_spacing (EEL_LABELED_IMAGE (GTK_BIN (button)->child), EMBLEM_LABEL_SPACING);
		
		g_free (label);
		g_object_unref (pixbuf);

		/* Attach parameters and signal handler. */
		g_object_set_data_full (G_OBJECT (button), "nautilus_property_name",
					nautilus_emblem_get_keyword_from_icon_name (emblem_name), g_free);
				     
		nautilus_file_ref (file);
		g_object_set_data_full (G_OBJECT (button), "nautilus_file",
					file, (GDestroyNotify) nautilus_file_unref);
		
		g_signal_connect (button, "toggled",
				  G_CALLBACK (property_button_toggled), NULL);

		/* Set initial state of button. */
		property_button_update (GTK_TOGGLE_BUTTON (button));

		/* Update button when file changes in future. */
		g_signal_connect_object (file, "changed",
					 G_CALLBACK (property_button_update), button,
					 G_CONNECT_SWAPPED);

		gtk_container_add (GTK_CONTAINER (emblems_table), button);
	}
	eel_g_list_free_deep (icons);
	
	gtk_widget_show_all (emblems_table);
}

static void
update_permissions_check_button_state (GtkWidget *check_button, NautilusFile *file)
{
	GnomeVFSFilePermissions file_permissions, permission;
	gulong toggled_signal_id;

	g_assert (GTK_IS_CHECK_BUTTON (check_button));
	g_assert (NAUTILUS_IS_FILE (file));

	if (nautilus_file_is_gone (file)) {
		return;
	}
	
	g_assert (nautilus_file_can_get_permissions (file));

	toggled_signal_id = GPOINTER_TO_UINT (g_object_get_data (G_OBJECT (check_button),
								"toggled_signal_id"));
	permission = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (check_button),
							 "permission"));
	g_assert (toggled_signal_id != 0);
	g_assert (permission != 0);

	file_permissions = nautilus_file_get_permissions (file);
	gtk_widget_set_sensitive (GTK_WIDGET (check_button), 
				  nautilus_file_can_set_permissions (file));

	/* Don't react to the "toggled" signal here to avoid recursion. */
	g_signal_handler_block (check_button, toggled_signal_id);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (check_button),
				      (file_permissions & permission) != 0);
	g_signal_handler_unblock (check_button, toggled_signal_id);
}

static void
permission_change_callback (NautilusFile *file, GnomeVFSResult result, gpointer callback_data)
{
	g_assert (callback_data == NULL);
	
	/* Report the error if it's an error. */
	fm_report_error_setting_permissions (file, result, NULL);
}

static void
permissions_check_button_toggled (GtkToggleButton *toggle_button, gpointer user_data)
{
	NautilusFile *file;
	GnomeVFSFilePermissions permissions, permission_mask;

	g_assert (NAUTILUS_IS_FILE (user_data));

	file = NAUTILUS_FILE (user_data);
	g_assert (nautilus_file_can_get_permissions (file));
	g_assert (nautilus_file_can_set_permissions (file));

	permission_mask = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (toggle_button),
					  "permission"));

	/* Modify the file's permissions according to the state of this check button. */
	permissions = nautilus_file_get_permissions (file);
	if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (toggle_button))) {
		/* Turn this specific permission bit on. */
		permissions |= permission_mask;
	} else {
		/* Turn this specific permission bit off. */
		permissions &= ~permission_mask;
	}

	/* Try to change file permissions. If this fails, complain to user. */
	nautilus_file_set_permissions
		(file, permissions,
		 permission_change_callback, NULL);
}

static void
set_up_permissions_checkbox (GtkWidget *check_button, 
			     NautilusFile *file,
			     GnomeVFSFilePermissions permission)
{
	gulong toggled_signal_id;

	toggled_signal_id = g_signal_connect (check_button, "toggled",
					      G_CALLBACK (permissions_check_button_toggled),
					      file);

	/* Load up the check_button with data we'll need when updating its state. */
        g_object_set_data (G_OBJECT (check_button), "toggled_signal_id", 
			   GUINT_TO_POINTER (toggled_signal_id));
        g_object_set_data (G_OBJECT (check_button), "permission", 
			   GINT_TO_POINTER (permission));
	
	/* Set initial state. */
        update_permissions_check_button_state (check_button, file);

        /* Update state later if file changes. */
	g_signal_connect_object (file, "changed",
				 G_CALLBACK (update_permissions_check_button_state),
				 check_button, G_CONNECT_SWAPPED);
}

static void
add_permissions_checkbox (GtkTable *table, 
			  NautilusFile *file,
			  int row, int column, 
			  GnomeVFSFilePermissions permission_to_check)
{
	GtkWidget *check_button;
	gchar *label;

	if (column == PERMISSIONS_CHECKBOXES_READ_COLUMN) {
		label = _("_Read");
	} else if (column == PERMISSIONS_CHECKBOXES_WRITE_COLUMN) {
		label = _("_Write");
	} else {
		label = _("E_xecute");
	}

	check_button = gtk_check_button_new_with_mnemonic (label);
	gtk_widget_show (check_button);
	gtk_table_attach (table, check_button,
			  column, column + 1,
			  row, row + 1,
			  GTK_FILL, 0,
			  0, 0);

	set_up_permissions_checkbox (check_button, file, permission_to_check);
}

static GtkWidget *
append_special_execution_checkbox (FMPropertiesWindow *window,
				   GtkTable *table,
				   const char *label_text,
				   GnomeVFSFilePermissions permission_to_check)
{
	GtkWidget *check_button;
	guint last_row;

	last_row = append_row (table);

	check_button = gtk_check_button_new_with_mnemonic (label_text);
	gtk_widget_show (check_button);

	gtk_table_attach (table, check_button,
			  VALUE_COLUMN, VALUE_COLUMN + 1,
			  last_row, last_row + 1,
			  GTK_FILL, 0,
			  0, 0);

	set_up_permissions_checkbox (check_button, window->details->target_file, permission_to_check);
	++window->details->num_special_flags_rows;

	return check_button;
}

static void
remember_special_flags_widget (FMPropertiesWindow *window, GtkWidget *widget)
{
	g_assert (FM_IS_PROPERTIES_WINDOW (window));
	g_assert (GTK_IS_WIDGET (widget));
	
	window->details->special_flags_widgets = 
		g_list_prepend (window->details->special_flags_widgets, widget);
}

static void
update_visibility_of_special_flags_widgets (FMPropertiesWindow *window)
{
	update_visibility_of_table_rows 
		(window->details->permissions_table,
		 eel_preferences_get_boolean (NAUTILUS_PREFERENCES_SHOW_SPECIAL_FLAGS),
		 window->details->first_special_flags_row,
		 window->details->num_special_flags_rows,
		 window->details->special_flags_widgets);
}

static void
update_visibility_of_special_flags_widgets_wrapper (gpointer callback_data)
{
	update_visibility_of_special_flags_widgets (FM_PROPERTIES_WINDOW (callback_data));
}

static void
append_special_execution_flags (FMPropertiesWindow *window,
				GtkTable *table)
{
	remember_special_flags_widget (window, append_special_execution_checkbox 
		(window, table, _("Set _user ID"), GNOME_VFS_PERM_SUID));

	window->details->first_special_flags_row = table->nrows - 1;

	remember_special_flags_widget (window, GTK_WIDGET (attach_title_field 
		(table, table->nrows - 1, _("Special flags:"))));

	remember_special_flags_widget (window, append_special_execution_checkbox 
		(window, table, _("Set gro_up ID"), GNOME_VFS_PERM_SGID));
	remember_special_flags_widget (window, append_special_execution_checkbox 
		(window, table, _("_Sticky"), GNOME_VFS_PERM_STICKY));

	remember_special_flags_widget (window, append_separator (table));
	++window->details->num_special_flags_rows;

	update_visibility_of_special_flags_widgets (window);
	eel_preferences_add_callback_while_alive 
		(NAUTILUS_PREFERENCES_SHOW_SPECIAL_FLAGS,
		 update_visibility_of_special_flags_widgets_wrapper,
		 window,
		 G_OBJECT (window));
	
}

static void
create_permissions_page (FMPropertiesWindow *window)
{
	GtkWidget *vbox;
	GtkTable *page_table, *check_button_table;
	char *file_name, *prompt_text;
	NautilusFile *file;
	guint last_row;
	guint checkbox_titles_row;
	GtkLabel *group_label;
	GtkOptionMenu *group_menu;

	file = window->details->target_file;

	vbox = create_page_with_vbox (window->details->notebook, _("Permissions"));

	if (nautilus_file_can_get_permissions (file)) {
		if (!nautilus_file_can_set_permissions (file)) {
			add_prompt_and_separator (
				GTK_VBOX (vbox), 
				_("You are not the owner, so you can't change these permissions."));
		}

		page_table = GTK_TABLE (gtk_table_new (1, COLUMN_COUNT, FALSE));
		window->details->permissions_table = page_table;

		apply_standard_table_padding (page_table);
		last_row = 0;
		gtk_widget_show (GTK_WIDGET (page_table));
		gtk_box_pack_start (GTK_BOX (vbox), 
				    GTK_WIDGET (page_table), 
				    TRUE, TRUE, 0);

		attach_title_field (page_table, last_row, _("File owner:"));
		if (nautilus_file_can_set_owner (file)) {
			/* Option menu in this case. */
			attach_owner_menu (page_table, last_row, file);
		} else {
			/* Static text in this case. */
			attach_value_field (page_table, last_row, VALUE_COLUMN, file, "owner"); 
		}

		if (nautilus_file_can_set_group (file)) {
			last_row = append_title_field (page_table,
						       _("_File group:"),
						       &group_label);
			/* Option menu in this case. */
			group_menu = attach_group_menu (page_table, last_row,
							file);
			gtk_label_set_mnemonic_widget (GTK_LABEL (group_label),
						       GTK_WIDGET (group_menu));
		} else {
			last_row = append_title_field (page_table,
						       _("File group:"),
						       NULL);
			/* Static text in this case. */
			attach_value_field (page_table, last_row, VALUE_COLUMN, file, "group"); 
		}

		append_separator (page_table);

		checkbox_titles_row = append_title_field (page_table, _("Owner:"), NULL);
		append_title_field (page_table, _("Group:"), NULL);
		append_title_field (page_table, _("Others:"), NULL);
		
		check_button_table = GTK_TABLE (gtk_table_new 
						   (PERMISSIONS_CHECKBOXES_ROW_COUNT, 
						    PERMISSIONS_CHECKBOXES_COLUMN_COUNT, 
						    FALSE));
		apply_standard_table_padding (check_button_table);
		gtk_widget_show (GTK_WIDGET (check_button_table));
		gtk_table_attach (page_table, GTK_WIDGET (check_button_table),
				  VALUE_COLUMN, VALUE_COLUMN + 1, 
				  checkbox_titles_row, checkbox_titles_row + PERMISSIONS_CHECKBOXES_ROW_COUNT,
				  0, 0,
				  0, 0);

		add_permissions_checkbox (check_button_table, file, 
					  PERMISSIONS_CHECKBOXES_OWNER_ROW,
					  PERMISSIONS_CHECKBOXES_READ_COLUMN,
					  GNOME_VFS_PERM_USER_READ);

		add_permissions_checkbox (check_button_table, file, 
					  PERMISSIONS_CHECKBOXES_OWNER_ROW,
					  PERMISSIONS_CHECKBOXES_WRITE_COLUMN,
					  GNOME_VFS_PERM_USER_WRITE);

		add_permissions_checkbox (check_button_table, file, 
					  PERMISSIONS_CHECKBOXES_OWNER_ROW,
					  PERMISSIONS_CHECKBOXES_EXECUTE_COLUMN,
					  GNOME_VFS_PERM_USER_EXEC);

		add_permissions_checkbox (check_button_table, file, 
					  PERMISSIONS_CHECKBOXES_GROUP_ROW,
					  PERMISSIONS_CHECKBOXES_READ_COLUMN,
					  GNOME_VFS_PERM_GROUP_READ);

		add_permissions_checkbox (check_button_table, file, 
					  PERMISSIONS_CHECKBOXES_GROUP_ROW,
					  PERMISSIONS_CHECKBOXES_WRITE_COLUMN,
					  GNOME_VFS_PERM_GROUP_WRITE);

		add_permissions_checkbox (check_button_table, file, 
					  PERMISSIONS_CHECKBOXES_GROUP_ROW,
					  PERMISSIONS_CHECKBOXES_EXECUTE_COLUMN,
					  GNOME_VFS_PERM_GROUP_EXEC);

		add_permissions_checkbox (check_button_table, file, 
					  PERMISSIONS_CHECKBOXES_OTHERS_ROW,
					  PERMISSIONS_CHECKBOXES_READ_COLUMN,
					  GNOME_VFS_PERM_OTHER_READ);

		add_permissions_checkbox (check_button_table, file, 
					  PERMISSIONS_CHECKBOXES_OTHERS_ROW,
					  PERMISSIONS_CHECKBOXES_WRITE_COLUMN,
					  GNOME_VFS_PERM_OTHER_WRITE);

		add_permissions_checkbox (check_button_table, file, 
					  PERMISSIONS_CHECKBOXES_OTHERS_ROW,
					  PERMISSIONS_CHECKBOXES_EXECUTE_COLUMN,
					  GNOME_VFS_PERM_OTHER_EXEC);

		append_separator (page_table);

		append_special_execution_flags (window, page_table);

		append_title_value_pair (page_table, _("Text view:"), file, "permissions");		
		append_title_value_pair (page_table, _("Number view:"), file, "octal_permissions");
		append_title_value_pair (page_table, _("Last changed:"), file, "date_permissions");
		
	} else {
		file_name = nautilus_file_get_display_name (file);
		prompt_text = g_strdup_printf (_("The permissions of \"%s\" could not be determined."), file_name);
		g_free (file_name);
		add_prompt (GTK_VBOX (vbox), prompt_text, TRUE);
		g_free (prompt_text);
	}
}

static GtkWidget *
bonobo_page_error_message (const char *view_name,
			   const char *msg)
{
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *image;

	hbox = gtk_hbox_new (FALSE, GNOME_PAD);
	image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_ERROR,
					  GTK_ICON_SIZE_DIALOG);

	msg = g_strdup_printf ("There was an error while trying to create the view named `%s':  %s", view_name, msg);
	label = gtk_label_new (msg);

	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);

	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
	gtk_widget_show_all (hbox);

	return hbox;
}

static void
bonobo_page_activate_callback (CORBA_Object obj,
			       const char *error_reason,
			       gpointer user_data)
{
	ActivationData *data;
	FMPropertiesWindow *window;
	GtkWidget *widget;
	CORBA_Environment ev;

	data = (ActivationData *)user_data;
	window = data->window;

	g_return_if_fail (FM_IS_PROPERTIES_WINDOW (window));

	CORBA_exception_init (&ev);
	widget = NULL;

	if (obj != CORBA_OBJECT_NIL) {
		Bonobo_Control control;
		Bonobo_PropertyBag pb;
		BonoboArg *arg;
		char *uri;
		
		uri = nautilus_file_get_uri (window->details->target_file);

		control = Bonobo_Unknown_queryInterface
				(obj, "IDL:Bonobo/Control:1.0", &ev);


		pb = Bonobo_Control_getProperties (control, &ev);

		if (!BONOBO_EX (&ev)) {
			arg = bonobo_arg_new (BONOBO_ARG_STRING);
			BONOBO_ARG_SET_STRING (arg, uri);

			bonobo_pbclient_set_value_async (pb, "URI", arg, &ev);
			bonobo_arg_release (arg);
			bonobo_object_release_unref (pb, NULL);

			if (!BONOBO_EX (&ev)) {
				widget = bonobo_widget_new_control_from_objref
							(control, CORBA_OBJECT_NIL);
				bonobo_object_release_unref (control, &ev);
			}
		}

		g_free (uri);
	}

	if (widget == NULL) {
		widget = bonobo_page_error_message (data->view_name,
						    error_reason);
	}

	gtk_container_add (GTK_CONTAINER (data->vbox), widget);
	gtk_widget_show (widget);

	g_free (data->view_name);
	g_free (data);
}

static void
append_bonobo_pages (FMPropertiesWindow *window)
{
	GList *components, *l;
	CORBA_Environment ev;

	/* find all the property pages for this file */
	components = nautilus_mime_get_property_components_for_file
					(window->details->target_file);
	
	CORBA_exception_init (&ev);

	l = components;
	while (l != NULL) {
		NautilusViewIdentifier *view_id;
		Bonobo_ServerInfo *server;
		ActivationData *data;
		GtkWidget *vbox;

		server = l->data;
		l = l->next;

		view_id = nautilus_view_identifier_new_from_property_page (server);
		vbox = create_page_with_vbox (window->details->notebook,
					      view_id->name);

		/* just a tag...the value doesn't matter */
		g_object_set_data (G_OBJECT (vbox), "is-bonobo-page",
				  vbox);

		data = g_new (ActivationData, 1);
		data->window = window;
		data->vbox = vbox;
		data->view_name = g_strdup (view_id->name);

		bonobo_activation_activate_from_id_async (view_id->iid,
					0, bonobo_page_activate_callback,
					data, &ev);
	}

}

static gboolean
should_show_emblems (FMPropertiesWindow *window) 
{
	/* FIXME bugzilla.gnome.org 45643:
	 * Emblems aren't displayed on the the desktop Trash icon, so
	 * we shouldn't pretend that they work by showing them here.
	 * When bug 5643 is fixed we can remove this case.
	 */
	if (is_merged_trash_directory (window->details->target_file)) {
		return FALSE;
	}

	return TRUE;
}

static gboolean
should_show_permissions (FMPropertiesWindow *window) 
{
	/* Don't show permissions for the Trash since it's not
	 * really a file system object.
	 */
	if (is_merged_trash_directory (window->details->target_file)) {
		return FALSE;
	}

	return TRUE;
}

static StartupData *
startup_data_new (NautilusFile *original_file, 
		  NautilusFile *target_file,
		  FMDirectoryView *directory_view)
{
	StartupData *data;

	data = g_new0 (StartupData, 1);
	data->original_file = nautilus_file_ref (original_file);
	data->target_file = nautilus_file_ref (target_file);
	data->directory_view = directory_view;

	return data;
}

static void
startup_data_free (StartupData *data)
{
	nautilus_file_unref (data->original_file);
	nautilus_file_unref (data->target_file);
	g_free (data);
}

static void
help_button_callback (GtkWidget *widget, GtkWidget *property_window)
{
	GError *error = NULL;
	char *message;

	egg_help_display_desktop_on_screen (NULL, "user-guide", "wgosnautilus.xml", "gosnautilus-51",
					    gtk_window_get_screen (GTK_WINDOW (property_window)),
&error);

	if (error) {
		message = g_strdup_printf (_("There was an error displaying help: \n%s"),
					   error->message);
		eel_show_error_dialog (message, _("Couldn't show help"),
				       GTK_WINDOW (property_window));
		g_error_free (error);
		g_free (message);
	}
}

static FMPropertiesWindow *
create_properties_window (StartupData *startup_data)
{
	FMPropertiesWindow *window;
	GList *attributes;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *button;

	window = FM_PROPERTIES_WINDOW (gtk_widget_new (fm_properties_window_get_type (), NULL));

	window->details->original_file = nautilus_file_ref (startup_data->original_file);
	window->details->target_file = nautilus_file_ref (startup_data->target_file);
	
	gtk_window_set_wmclass (GTK_WINDOW (window), "file_properties", "Nautilus");
	gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
	gtk_window_set_screen (GTK_WINDOW (window),
			       gtk_widget_get_screen (GTK_WIDGET (startup_data->directory_view)));

	/* Set initial window title */
	update_properties_window_title (GTK_WINDOW (window), window->details->target_file);

	/* Start monitoring the file attributes we display. Note that some
	 * of the attributes are for the original file, and some for the
	 * target file.
	 */
	attributes = nautilus_icon_factory_get_required_file_attributes ();
	attributes = g_list_prepend (attributes,
				     NAUTILUS_FILE_ATTRIBUTE_DISPLAY_NAME);
	nautilus_file_monitor_add (window->details->original_file, window, attributes);
	g_list_free (attributes);

	attributes = NULL;
	if (nautilus_file_is_directory (window->details->target_file)) {
		attributes = g_list_prepend (attributes,
					     NAUTILUS_FILE_ATTRIBUTE_DEEP_COUNTS);
	}
	attributes = g_list_prepend (attributes,
				     NAUTILUS_FILE_ATTRIBUTE_METADATA);
	nautilus_file_monitor_add (window->details->target_file, window, attributes);
	g_list_free (attributes);

	/* React to future property changes and file deletions. */
	window->details->file_changed_handler_id =
		g_signal_connect_object (window->details->target_file, "changed",
					 G_CALLBACK (properties_window_file_changed_callback),
					 window, G_CONNECT_SWAPPED);

	/* Create box for notebook and button box. */
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 5);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (window),
			   GTK_WIDGET (vbox));

	/* Create the notebook tabs. */
	window->details->notebook = GTK_NOTEBOOK (gtk_notebook_new ());
	gtk_widget_show (GTK_WIDGET (window->details->notebook));
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (window->details->notebook),
			    TRUE, TRUE, 0);

	/* Create the pages. */
	create_basic_page (window);

	if (should_show_emblems (window)) {
		create_emblems_page (window);
	}

	if (should_show_permissions (window)) {
		create_permissions_page (window);
	}

	/* append pages from available views */
	append_bonobo_pages (window);

	/* Create box for help and close buttons. */
	hbox = gtk_hbutton_box_new ();
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (hbox), FALSE, TRUE, 5);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbox), GTK_BUTTONBOX_EDGE);

	button = gtk_button_new_from_stock (GTK_STOCK_HELP);
 	gtk_widget_show (button);
	gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (button),
			    FALSE, TRUE, 0);
	g_signal_connect_object (button, "clicked",
				 G_CALLBACK (help_button_callback),
				 window, 0);
	
	button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_widget_show (button);
	gtk_box_pack_end (GTK_BOX (hbox), GTK_WIDGET (button),
			    FALSE, TRUE, 0);
	g_signal_connect_swapped (button, "clicked",
				  G_CALLBACK (gtk_widget_destroy),
				  window);

	return window;
}

static NautilusFile *
get_target_file (NautilusFile *file)
{
	NautilusFile *target_file;
	char *uri;
	char *uri_to_display;

	target_file = NULL;
	if (nautilus_file_is_nautilus_link (file)) {
		/* Note: This will only work on local files. For now
		 * that seems fine since the links we care about are
		 * all on the desktop.
		 */
		if (nautilus_file_is_local (file)) {
			uri = nautilus_file_get_uri (file);

			switch (nautilus_link_local_get_link_type (uri, NULL)) {
			case NAUTILUS_LINK_MOUNT:
			case NAUTILUS_LINK_TRASH:
			case NAUTILUS_LINK_HOME:
				/* map to linked URI for these types of links */
				uri_to_display = nautilus_link_local_get_link_uri (uri);
				target_file = nautilus_file_get (uri_to_display);
				g_free (uri_to_display);
				break;
			case NAUTILUS_LINK_GENERIC:
				/* don't for these types */
				break;
			}
			
			g_free (uri);
		}
	}

	if (target_file != NULL) {
		return target_file;
	}

	/* Ref passed-in file here since we've decided to use it. */
	nautilus_file_ref (file);
	return file;
}

static void
create_properties_window_callback (NautilusFile *file, gpointer callback_data)
{
	FMPropertiesWindow *new_window;
	StartupData *startup_data;

	startup_data = (StartupData *)callback_data;

	new_window = create_properties_window (startup_data);

	g_hash_table_insert (windows, startup_data->original_file, new_window);

	remove_pending_file (startup_data, FALSE, TRUE, TRUE);

/* FIXME bugzilla.gnome.org 42151:
 * See comment elsewhere in this file about bug 2151.
 */
#ifdef UNDO_ENABLED
	nautilus_undo_share_undo_manager (GTK_OBJECT (new_window),
					  GTK_OBJECT (callback_data));
#endif	
	gtk_window_present (GTK_WINDOW (new_window));
}

static void
cancel_create_properties_window_callback (gpointer callback_data)
{
	remove_pending_file ((StartupData *)callback_data, TRUE, FALSE, TRUE);
}

static void
directory_view_destroyed_callback (FMDirectoryView *view, gpointer callback_data)
{
	g_assert (view == ((StartupData *)callback_data)->directory_view);
	
	remove_pending_file ((StartupData *)callback_data, TRUE, TRUE, FALSE);
}

static void
remove_pending_file (StartupData *startup_data,
		     gboolean cancel_call_when_ready,
		     gboolean cancel_timed_wait,
		     gboolean cancel_destroy_handler)
{
	if (cancel_call_when_ready) {
		nautilus_file_cancel_call_when_ready 
			(startup_data->target_file, create_properties_window_callback, startup_data);
	}
	if (cancel_timed_wait) {
		eel_timed_wait_stop 
			(cancel_create_properties_window_callback, startup_data);
	}
	if (cancel_destroy_handler) {
		g_signal_handlers_disconnect_by_func (startup_data->directory_view,
						      G_CALLBACK (directory_view_destroyed_callback),
						      startup_data);
	}
	g_hash_table_remove (pending_files, startup_data->original_file);
	startup_data_free (startup_data);
}

void
fm_properties_window_present (NautilusFile *original_file, FMDirectoryView *directory_view)
{
	GtkWindow *existing_window;
	GtkWidget *parent_window;
	NautilusFile *target_file;
	StartupData *startup_data;
	GList attribute_list;

	g_return_if_fail (NAUTILUS_IS_FILE (original_file));
	g_return_if_fail (FM_IS_DIRECTORY_VIEW (directory_view));

	/* Create the hash tables first time through. */
	if (windows == NULL) {
		windows = eel_g_hash_table_new_free_at_exit
			(NULL, NULL, "property windows");
	}
	
	if (pending_files == NULL) {
		pending_files = eel_g_hash_table_new_free_at_exit
			(NULL, NULL, "pending property window files");
	}
	
	/* Look to see if there's already a window for this file. */
	existing_window = g_hash_table_lookup (windows, original_file);
	if (existing_window != NULL) {
		gtk_window_set_screen (existing_window,
				       gtk_widget_get_screen (GTK_WIDGET (directory_view)));
		gtk_window_present (existing_window);
		return;
	}

	/* Look to see if we're already waiting for a window for this file. */
	if (g_hash_table_lookup (pending_files, original_file) != NULL) {
		return;
	}

	target_file = get_target_file (original_file);
	startup_data = startup_data_new (original_file, target_file, directory_view);
	nautilus_file_unref (target_file);

	/* Wait until we can tell whether it's a directory before showing, since
	 * some one-time layout decisions depend on that info. 
	 */
	
	g_hash_table_insert (pending_files, target_file, target_file);
	g_signal_connect (directory_view, "destroy",
			  G_CALLBACK (directory_view_destroyed_callback), startup_data);

	parent_window = gtk_widget_get_ancestor (GTK_WIDGET (directory_view), GTK_TYPE_WINDOW);
	eel_timed_wait_start
		(cancel_create_properties_window_callback,
		 startup_data,
		 _("Cancel Showing Properties Window?"),
		 _("Creating Properties window"),
		 parent_window == NULL ? NULL : GTK_WINDOW (parent_window));
	attribute_list.data = NAUTILUS_FILE_ATTRIBUTE_IS_DIRECTORY;
	attribute_list.next = NULL;
	attribute_list.prev = NULL;
	nautilus_file_call_when_ready
		(target_file, &attribute_list,
		 create_properties_window_callback, startup_data);
}

static void
real_destroy (GtkObject *object)
{
	FMPropertiesWindow *window;

	window = FM_PROPERTIES_WINDOW (object);

	g_hash_table_remove (windows, window->details->original_file);
	
	if (window->details->original_file != NULL) {
		nautilus_file_monitor_remove (window->details->original_file, window);
		nautilus_file_unref (window->details->original_file);
		window->details->original_file = NULL;
	}

	if (window->details->target_file != NULL) {
		g_signal_handler_disconnect (window->details->target_file,
					     window->details->file_changed_handler_id);
		nautilus_file_monitor_remove (window->details->target_file, window);
		nautilus_file_unref (window->details->target_file);
		window->details->target_file = NULL;
	}

	window->details->name_field = NULL;
	
	g_list_free (window->details->directory_contents_widgets);
	window->details->directory_contents_widgets = NULL;

	g_list_free (window->details->special_flags_widgets);
	window->details->special_flags_widgets = NULL;

	if (window->details->update_directory_contents_timeout_id != 0) {
		gtk_timeout_remove (window->details->update_directory_contents_timeout_id);
		window->details->update_directory_contents_timeout_id = 0;
	}

	GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
real_finalize (GObject *object)
{
	FMPropertiesWindow *window;

	window = FM_PROPERTIES_WINDOW (object);

	g_free (window->details->pending_name);
	g_free (window->details);

	G_OBJECT_CLASS (parent_class)->finalize (object);
}

/* icon selection callback to set the image of the file object to the selected file */
static void
set_icon_callback (const char* icon_path, FMPropertiesWindow *properties_window)
{
	NautilusFile *file;
	char *icon_uri;
	
	g_return_if_fail (properties_window != NULL);
	g_return_if_fail (FM_IS_PROPERTIES_WINDOW (properties_window));

	if (icon_path != NULL) {
		file = properties_window->details->original_file;
		icon_uri = gnome_vfs_get_uri_from_local_path (icon_path);
		nautilus_file_set_metadata (file, NAUTILUS_METADATA_KEY_CUSTOM_ICON, NULL, icon_uri);
		g_free (icon_uri);
		nautilus_file_set_metadata (file, NAUTILUS_METADATA_KEY_ICON_SCALE, NULL, NULL);

		/* re-enable the property window's clear image button */ 
		gtk_widget_set_sensitive (properties_window->details->remove_image_button, TRUE);
	} else {
	}
}


/* handle the "select icon" button */
static void
select_image_button_callback (GtkWidget *widget, FMPropertiesWindow *properties_window)
{
	GtkWidget *dialog;

	g_assert (FM_IS_PROPERTIES_WINDOW (properties_window));

	dialog = eel_gnome_icon_selector_new (_("Select an icon:"),
					      NULL,
					      GTK_WINDOW (properties_window),
					      (EelIconSelectionFunction) set_icon_callback,
					      properties_window);						   
}

static void
remove_image_button_callback (GtkWidget *widget, FMPropertiesWindow *properties_window)
{
	g_assert (FM_IS_PROPERTIES_WINDOW (properties_window));

	nautilus_file_set_metadata (properties_window->details->original_file,
				    NAUTILUS_METADATA_KEY_ICON_SCALE,
				    NULL, NULL);
	nautilus_file_set_metadata (properties_window->details->original_file,
				    NAUTILUS_METADATA_KEY_CUSTOM_ICON,
				    NULL, NULL);
	
	gtk_widget_set_sensitive (widget, FALSE);
}

static void
fm_properties_window_class_init (FMPropertiesWindowClass *class)
{
	G_OBJECT_CLASS (class)->finalize = real_finalize;
	GTK_OBJECT_CLASS (class)->destroy = real_destroy;
}

static void
fm_properties_window_instance_init (FMPropertiesWindow *window)
{
	window->details = g_new0 (FMPropertiesWindowDetails, 1);

	eel_gtk_window_set_up_close_accelerator (GTK_WINDOW (window));
}

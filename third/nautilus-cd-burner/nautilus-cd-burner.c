/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*-

   nautilus-cd-burner.c: easy to use cd burner software
 
   Copyright (C) 2002 Red Hat, Inc.
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
  
   You should have received a copy of the GNU General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
  
   Authors: Alexander Larsson <alexl@redhat.com>
*/
#include <gtk/gtk.h>
#include <gtk/gtkfilesel.h>
#include <gtk/gtkseparatormenuitem.h>
#include <glade/glade.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnome/gnome-help.h>
#include <libgnomeui/gnome-ui-init.h>
#include <gconf/gconf-client.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <bacon-cd-selection.h>

#include "nautilus-cd-burner.h"
#include "make-iso.h"

#define MAX_ISO_NAME_LEN 32

enum {
	CANCEL_NONE,
	CANCEL_MAKE_ISO,
	CANCEL_CD_RECORD,
};

GtkWidget * target_optionmenu_create (void);

static GladeXML *xml;

static CDRecorder *cdrecorder = NULL;
static int cancel = CANCEL_NONE;
/* For the image spinning */
static GList *image_list;

static GtkWidget *
ncb_hig_dialog (GtkMessageType type, char *title, char *reason,
		GtkWindow *parent)
{
	GtkWidget *error_dialog;
	char *title_esc, *reason_esc;

	if (reason == NULL)
		g_warning (" called with reason == NULL");

	title_esc = g_markup_escape_text (title, -1);
	reason_esc = g_markup_escape_text (reason, -1);

	error_dialog =
		gtk_message_dialog_new (parent,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				type,
				GTK_BUTTONS_NONE,
				"<b>%s</b>\n%s", title_esc, reason_esc);
	gtk_window_set_title (GTK_WINDOW (error_dialog), title_esc);
	g_free (title_esc);
	g_free (reason_esc);

	gtk_container_set_border_width (GTK_CONTAINER (error_dialog), 5);
	gtk_dialog_set_default_response (GTK_DIALOG (error_dialog),
			GTK_RESPONSE_OK);
	gtk_label_set_use_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (error_dialog)->label), TRUE);
	gtk_window_set_modal (GTK_WINDOW (error_dialog), TRUE);

	return error_dialog;
}

static void
ncb_hig_show_error_dialog (char *title, char *reason, GtkWindow *parent)
{
	GtkWidget *dialog;

	dialog = ncb_hig_dialog (GTK_MESSAGE_ERROR, title, reason, parent);
	gtk_dialog_add_button (GTK_DIALOG (dialog),
			GTK_STOCK_OK, GTK_RESPONSE_OK);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

/* Originally eel_make_valid_utf8 */
static char *
ncb_make_valid_utf8 (const char *name)
{
	GString *string;
	const char *remainder, *invalid;
	int remaining_bytes, valid_bytes;

	string = NULL;
	remainder = name;
	remaining_bytes = strlen (name);

	while (remaining_bytes != 0) {
		if (g_utf8_validate (remainder, remaining_bytes, &invalid)) {
			break;
		}
		valid_bytes = invalid - remainder;

		if (string == NULL) {
			string = g_string_sized_new (remaining_bytes);
		}
		g_string_append_len (string, remainder, valid_bytes);
		g_string_append_c (string, '?');

		remaining_bytes -= valid_bytes + 1;
		remainder = invalid + 1;
	}

	if (string == NULL) {
		return g_strdup (name);
	}

	g_string_append (string, remainder);
	g_string_append (string, _(" (invalid Unicode)"));
	g_assert (g_utf8_validate (string->str, -1, NULL));

	return g_string_free (string, FALSE);
}


GtkWindow *
cd_progress_get_window (void)
{
	return GTK_WINDOW (glade_xml_get_widget (xml, "progress_window"));
}

void 
cd_progress_set_fraction (double fraction)
{
	GtkWidget *progress_bar;
	
	progress_bar = glade_xml_get_widget (xml, "cd_progress");

	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), fraction);
}

void 
cd_progress_set_text (const char *text)
{
	GtkWidget *progress_label;
	char *string;

	progress_label = glade_xml_get_widget (xml, "cd_progress_label");
	string = g_strdup_printf ("<i>%s</i>", text);
	gtk_label_set_markup (GTK_LABEL (progress_label), string);
	g_free (string);
}

static gboolean
cd_progress_set_image (gpointer user_data)
{
	GtkWidget *image;
	GdkPixbuf *pixbuf;

	if (image_list == NULL) {
		return FALSE;
	}

	image = glade_xml_get_widget (xml, "cd_image");
	pixbuf = image_list->data;

	if (pixbuf == NULL || image == NULL) {
		return FALSE;
	}

	gtk_image_set_from_pixbuf (GTK_IMAGE (image), pixbuf);

	if (image_list->next != NULL) {
		image_list = image_list->next;
	} else {
		image_list = g_list_first (image_list);
	}

	return TRUE;
}

void
cd_progress_set_image_spinning (gboolean spinning)
{
	static guint spin_id = 0;

	if (spinning) {
		if (spin_id == 0) {
			spin_id = g_timeout_add (100, (GSourceFunc) cd_progress_set_image, NULL);
		}
	} else {
		if (spin_id != 0) {
			g_source_remove (spin_id);
			spin_id = 0;
		}
	}
}

static void
cd_progress_image_setup (void)
{
	GdkPixbuf *pixbuf;
	char *filename;
	int i;

	image_list = NULL;
	i = 1;

	/* Setup the pixbuf list */
	filename = g_strdup_printf (DATADIR "/cdspin%d.png", i);
	while (g_file_test (filename, G_FILE_TEST_IS_REGULAR)) {
		pixbuf = gdk_pixbuf_new_from_file (filename, NULL);

		if (pixbuf != NULL) {
			image_list = g_list_prepend (image_list, (gpointer) pixbuf);
		}

		i++;
		g_free (filename);
		filename = g_strdup_printf (DATADIR "/cdspin%d.png", i);
	}

	g_free (filename);
	if (image_list != NULL) {
		image_list = g_list_reverse (image_list);
	}

	/* Set the first image */
	cd_progress_set_image (NULL);
}

static void
cd_progress_image_cleanup (void)
{
	GdkPixbuf *pixbuf;

	image_list = g_list_first (image_list);

	while (image_list != NULL) {
		pixbuf = (GdkPixbuf *) image_list->data;
		if (pixbuf != NULL) {
			gdk_pixbuf_unref (pixbuf);
		}
		image_list = g_list_remove (image_list, image_list->data);
	}
}

static void
animation_changed_cb (CDRecorder *cdrecorder, gboolean spinning, gpointer data)
{
	cd_progress_set_image_spinning (spinning);
}

static void
action_changed_cb (CDRecorder *cdrecorder, CDRecorderActions action,
		CDRecorderMedia media)
{
	const char *text;

	text = NULL;

	switch (action) {
	case PREPARING_WRITE:
		if (media == MEDIA_CD) {
			text = N_("Preparing to write CD");
		} else {
			text = N_("Preparing to write DVD");
		}
		break;
	case WRITING:
		if (media == MEDIA_CD) {
			text = N_("Writing CD");
		} else {
			text = N_("Writing DVD");
		}
		break;
	case FIXATING:
		if (media == MEDIA_CD) {
			text = N_("Fixating CD");
		} else {
			text = N_("Fixating DVD");
		}
		break;
	case BLANKING:
		if (media == MEDIA_CD) {
			text = N_("Erasing CD");
		} else {
			text = N_("Erasing DVD");
		}
		break;
	default:
		g_warning ("Unhandled action in action_changed_cb");
	}

	cd_progress_set_text (text);
}

static void
progress_changed_cb (CDRecorder *cdrecorder, gdouble fraction, gpointer data)
{
	cd_progress_set_fraction (fraction);
}

static gboolean
insert_cd_request_cb (CDRecorder *cdrecorder,
		      gboolean is_reload, gboolean can_rewrite,
		      gboolean busy_cd, gpointer data)
{
	GtkWidget *dialog, *parent;
	const char *msg, *title;
	int res;

	if (busy_cd) {
		msg = N_("Please make sure another application is not using the disc.");
		title = N_("Disc is busy");
	} else if (is_reload && can_rewrite) {
		msg = N_("Please insert a rewritable or blank media in the drive tray.");
		title = N_("Insert rewritable or blank media");
	} else if (is_reload && !can_rewrite) {
		msg = N_("Please insert a blank media in the drive tray.");
		title = N_("Insert blank media");
	} else if (can_rewrite) {
			msg = N_("Please replace the in-drive media by a rewritable or blank media.");
			title = N_("Reload rewritable or blank media");
	} else {
		msg = N_("Please replace the in-drive media by a blank media.");
		title = N_("Reload blank media");
	}

	parent = glade_xml_get_widget (xml, "cdr_dialog");
	dialog = ncb_hig_dialog (GTK_MESSAGE_INFO,
			_(msg), _(title), GTK_WINDOW (parent));
	gtk_dialog_add_buttons (GTK_DIALOG (dialog),
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK,
			NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog),
			GTK_RESPONSE_OK);
	gtk_window_set_title (GTK_WINDOW (dialog), _(title));
	res = gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
	if (res == GTK_RESPONSE_CANCEL) {
		return FALSE;
	}

	return TRUE;
}

static void
details_clicked (GtkButton *button, gpointer data)
{
	GtkWidget *dialog, *label, *text, *scrolled_window;
	GtkTextBuffer *buffer;

	dialog = gtk_dialog_new_with_buttons (_("Disc writing error details"),
					      cd_progress_get_window (),
					      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
					      GTK_STOCK_CLOSE, GTK_RESPONSE_CANCEL,
					      NULL);

	label = gtk_label_new (_("Detailed error output from cdrecord:"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
			    label,
			    FALSE, FALSE, 0);

	buffer = gtk_text_buffer_new (NULL);
	gtk_text_buffer_set_text (buffer,
				  cd_recorder_get_error_message_details (cdrecorder), -1);

	text = gtk_text_view_new_with_buffer (buffer);
	g_object_unref (buffer);
	gtk_text_view_set_editable (GTK_TEXT_VIEW (text), FALSE);
	gtk_widget_show (text);

	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_ALWAYS);

	gtk_container_add (GTK_CONTAINER (scrolled_window), text);
	gtk_widget_show (scrolled_window);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
			    scrolled_window,
			    TRUE, TRUE, 0);


	g_signal_connect (dialog, "response", (GCallback)gtk_widget_destroy, NULL);

	gtk_widget_show (dialog);
}


static void
show_error_message (CDRecorder *cdrecorder)
{
	GtkWidget *button, *dialog;
	char *msg;

	msg = cd_recorder_get_error_message (cdrecorder) ?
		g_strdup_printf (_("There was an error writing to the CD:\n%s"), cd_recorder_get_error_message (cdrecorder))
		: g_strdup (_("There was an error writing to the CD"));
	dialog = ncb_hig_dialog (GTK_MESSAGE_ERROR, _("Error writing to CD"),
			msg, cd_progress_get_window ());
	g_free (msg);
	gtk_dialog_add_button (GTK_DIALOG (dialog),
			GTK_STOCK_OK, GTK_RESPONSE_OK);

	button = gtk_button_new_with_mnemonic (_("_Details"));
	gtk_widget_show (button);
	gtk_box_pack_end (GTK_BOX (GTK_DIALOG (dialog)->action_area),
			  button,
			  FALSE, TRUE, 0);
	g_signal_connect (button, "clicked", (GCallback)details_clicked, cdrecorder);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

static gboolean
estimate_size_callback (const gchar *rel_path,
			GnomeVFSFileInfo *info,
			gboolean recursing_will_loop,
			gpointer data,
			gboolean *recurse)
{
	GnomeVFSFileSize *size = data;

	if (info->type != GNOME_VFS_FILE_TYPE_DIRECTORY) {
		*size += info->size;
	}
	
	*recurse = TRUE;
	return TRUE;
}

static GnomeVFSFileSize
estimate_size (char *uri)
{
	GnomeVFSFileSize size;

	size = 0;
	gnome_vfs_directory_visit (uri,
				   GNOME_VFS_FILE_INFO_DEFAULT,
				   GNOME_VFS_DIRECTORY_VISIT_DEFAULT,
				   (GnomeVFSDirectoryVisitFunc) estimate_size_callback,
				   &size);

	if (size == 0) {
		GList *list;

		if (gnome_vfs_directory_list_load (&list, uri, GNOME_VFS_FILE_INFO_FIELDS_NONE) == GNOME_VFS_OK) {
			if (list != NULL) {
				size = 1;
				gnome_vfs_file_info_list_free (list);
			}
		}
	}

	return size;
}

static void
setup_close_button (void)
{
	GtkWidget *close_button;
	GtkWidget *cancel_button;

	cancel_button = glade_xml_get_widget (xml, "cancel_button");
	close_button = glade_xml_get_widget (xml, "close_button");
	gtk_widget_hide (cancel_button);
	gtk_widget_show (close_button);
	g_signal_connect (close_button, "clicked", (GCallback)gtk_main_quit, NULL);
}

static gboolean 
overwrite_existing_file (GtkWindow *filesel, const char *filename)
{
	GtkWidget *dialog;
	gint ret;
	char *msg;

	msg = g_strdup_printf (_("A file named \"%s\" already exists.\nDo you want to overwrite it?"),
			filename);
	dialog = ncb_hig_dialog (GTK_MESSAGE_QUESTION, _("_Overwrite"),
			msg, filesel);

	/* Add Cancel button */
	gtk_dialog_add_button (GTK_DIALOG (dialog),
			       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);

	/* Add Overwrite button */
	gtk_dialog_add_button (GTK_DIALOG (dialog),
			       _("_Overwrite"), GTK_RESPONSE_YES);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_CANCEL);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);

	ret = gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);

	return (ret == GTK_RESPONSE_YES);
}

static char *
select_iso_filename (void)
{
	GtkWidget *chooser;
	char *filename;
	gint response;
	chooser = gtk_file_chooser_dialog_new_with_backend (_("Choose a filename for the cdrom image"),
							    NULL,
							    GTK_FILE_CHOOSER_ACTION_SAVE,
							    "unix",
							    GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							    GTK_STOCK_OK, GTK_RESPONSE_OK,
							    NULL);
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (chooser),
					   "image.iso");
 reselect:
	filename = NULL;
	response = gtk_dialog_run (GTK_DIALOG (chooser));
	
	if (response == GTK_RESPONSE_OK) {
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
		if (g_file_test (filename, G_FILE_TEST_EXISTS)) {
			if (!overwrite_existing_file (GTK_WINDOW (chooser), filename)) {
				g_free (filename);
				goto reselect;
			}
		}

	}
	gtk_widget_destroy (chooser);

	return filename;
}

static int
burn_cd (const CDDrive *rec, const char *source_iso, const char *label, gboolean eject_cd, int speed, gboolean dummy, gboolean debug, gboolean overburn, gboolean burnproof)
{
	CDRecorderWriteFlags flags;
	GList *tracks;
	char *filename, *str;
	int fd;
	int res = RESULT_ERROR;

	tracks = NULL;
	filename = NULL;

	if (rec->type == CDDRIVE_TYPE_FILE) {
		/* Run the file selector and get the iso file name selected */
		filename = select_iso_filename ();

		/* No file selected, just return; Else proceed */
		if (filename == NULL) {
			return RESULT_CANCEL;
		} else {
			/* Check if you have permission to create or
			 * overwrite the file.
			 */
			if (g_file_test (filename, G_FILE_TEST_EXISTS)) {
				if (access (filename, W_OK) == -1) {
					char *msg;

					msg = g_strdup_printf (_("You do not have permissions to overwrite that file (%s)."), filename);
					ncb_hig_show_error_dialog (_("File image creation failed"), msg, NULL);
					g_free (msg);
					g_free (filename);
					return RESULT_ERROR;
				}
			} else {
				gchar *dir_name;
				dir_name = g_path_get_dirname (filename);

				if (access (dir_name, W_OK) == -1) {
					char *msg;

					msg = g_strdup_printf (_("You do not have permissions to create that file (%s)."), filename);
					ncb_hig_show_error_dialog (_("File image creation failed"), msg, NULL);
					g_free (filename);
					g_free (dir_name);
					g_free (msg);
					return RESULT_ERROR;
				}
				g_free (dir_name);
			}
		}
	} else {
		if (source_iso == NULL) {
			GConfClient *gc;
			const char *path;

			gc = gconf_client_get_default ();
			path = gconf_client_get_string (gc, "/apps/nautilus-cd-burner/temp_iso_dir", NULL);
			g_object_unref (G_OBJECT (gc));
			if (path != NULL && path[0] != '\0') {
				filename = g_build_filename (path, "image.iso.XXXXXX", NULL);
				fd = g_mkstemp(filename);
				close (fd);
			}
		} else {
			filename = g_strdup (source_iso);
		}
	}

	if (source_iso == NULL) {
		cancel = CANCEL_MAKE_ISO;

		if (filename != NULL) {
			res = make_iso (filename, label, FALSE, TRUE, debug);
		} else {
			res = RESULT_RETRY;
		}

		if (res == RESULT_RETRY) {
			g_free (filename);
			filename = g_build_filename (g_get_tmp_dir (), "image.iso.XXXXXX", NULL);
			fd = g_mkstemp(filename);
			close (fd);

			res = make_iso (filename, label, FALSE, TRUE, debug);
			if (res == RESULT_RETRY) {
				g_free (filename);
				filename = g_build_filename (g_get_home_dir (), ".image.iso.XXXXXX", NULL);
				fd = g_mkstemp(filename);
				close (fd);

				res = make_iso (filename, label,
						TRUE, TRUE, debug);
			}

			if (!res) {
				/* User cancelled or we had an error. */
				cancel = CANCEL_NONE;
				goto out;
			}
		}
	}

	if (rec->type == CDDRIVE_TYPE_FILE) {
		str = g_strdup_printf (_("Completed writing %s"), filename);
		cd_progress_set_text (str);
		g_free (str);
		setup_close_button ();
		gtk_main ();
	} else {
		Track *track;

		cdrecorder = cd_recorder_new ();
		cancel = CANCEL_CD_RECORD;

		g_signal_connect (G_OBJECT (cdrecorder),
				  "progress-changed",
				  G_CALLBACK (progress_changed_cb),
				  NULL);
		g_signal_connect (G_OBJECT (cdrecorder),
				  "action-changed",
				  G_CALLBACK (action_changed_cb),
				  NULL);
		g_signal_connect (G_OBJECT (cdrecorder),
				  "animation-changed",
				  G_CALLBACK (animation_changed_cb),
				  NULL);
		g_signal_connect (G_OBJECT (cdrecorder),
				  "insert-cd-request",
				  G_CALLBACK (insert_cd_request_cb),
				  NULL);

		track = g_new0 (Track, 1);
		track->type = TRACK_TYPE_DATA;
		track->contents.data.filename = g_strdup (filename);
		tracks = g_list_prepend (tracks, track);

		flags = 0;
		if (eject_cd) {
			flags |= CDRECORDER_EJECT;
		}
		if (dummy) {
			flags |= CDRECORDER_DUMMY_WRITE;
		}
		if (debug) {
			flags |= CDRECORDER_DEBUG;
		}
		if (overburn) {
			flags |= CDRECORDER_OVERBURN;
		}
		if (burnproof) {
			flags |= CDRECORDER_BURNPROOF;
		}

		res = cd_recorder_write_tracks (cdrecorder, (CDDrive *)rec,
				tracks, speed, flags);

		if (source_iso == NULL) {
			unlink (filename);
		}

		if (res == RESULT_FINISHED) {
			cd_progress_set_text (_("Complete"));
			setup_close_button ();
			gtk_main ();
		} else if (res != RESULT_CANCEL) {
			show_error_message (cdrecorder);
		}

		cancel = CANCEL_NONE;
		g_object_unref (cdrecorder);
	}
	
 out:
	g_free (filename);
	g_list_foreach (tracks, (GFunc)cd_recorder_track_free, NULL);
	g_list_free (tracks);
	return res;
}


static const CDDrive *
lookup_current_recorder (GladeXML *xml)
{
	GtkWidget *bcs;
	const CDDrive *rec;

	bcs = glade_xml_get_widget (xml, "target_optionmenu");
	rec = bacon_cd_selection_get_cdrom (BACON_CD_SELECTION (bcs));

	return rec;
}


static void
refresh_dialog (GladeXML *xml)
{
	GtkWidget *option_menu;
	GtkWidget *menu;
	GtkWidget *item;
	GtkWidget *eject_check;
	GList *l, *children;
	GConfClient *gc;
	char *name;
	int i, default_speed;
	const CDDrive *rec;

	/* Find active recorder: */
	rec = lookup_current_recorder (xml);

	/* add speed items: */
	option_menu = glade_xml_get_widget (xml, "speed_optionmenu");
	menu = gtk_option_menu_get_menu (GTK_OPTION_MENU (option_menu));

	children = gtk_container_get_children (GTK_CONTAINER (menu));
	for (l = children; l != NULL; l = l ->next) {
		gtk_container_remove (GTK_CONTAINER (menu),
				      l->data);
	}
	g_list_free (children);

	gc = gconf_client_get_default ();
	default_speed = gconf_client_get_int (gc, "/apps/nautilus-cd-burner/default_speed", NULL);
	g_object_unref (G_OBJECT (gc));

	item = gtk_menu_item_new_with_label (_("Maximum Possible"));
	g_object_set_data (G_OBJECT (item), "speed", GINT_TO_POINTER (0));
	gtk_widget_show (item);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
	for (i = 1; i <= rec->max_speed_write; i++) {
		name = g_strdup_printf ("%dx", i);
		item = gtk_menu_item_new_with_label (name);
		g_object_set_data (G_OBJECT (item), "speed", GINT_TO_POINTER (i));
		gtk_widget_show (item);
		gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
		g_free (name);
	}
	/* Default to automatic */
	gtk_option_menu_set_history (GTK_OPTION_MENU (option_menu), MIN (default_speed, default_speed));

	/* Disable speed if max speed < 1 */
	gtk_widget_set_sensitive (option_menu, rec->max_speed_write > 0);

	/* Disable eject for burn-to-iso */
	eject_check = glade_xml_get_widget (xml, "eject_check");
	gtk_widget_set_sensitive (eject_check, rec->type != CDDRIVE_TYPE_FILE);
}

static void
target_changed (BaconCdSelection *bcs, const char *device_path, GladeXML *xml)
{
  refresh_dialog (xml);
}

static void
cdname_entry_insert_text (GtkEditable *editable,
			  const gchar *new_text,
			  gint         new_text_length,
			  gint        *position)
{
	char *current_text;

	if (new_text_length < 0) {
		new_text_length = strlen (new_text);
	}

	current_text = gtk_editable_get_chars (editable, 0, -1);
	if (strlen (current_text) +  new_text_length >= MAX_ISO_NAME_LEN) {
		gdk_display_beep (gtk_widget_get_display (GTK_WIDGET (editable)));
		g_signal_stop_emission_by_name (editable, "insert_text");
	}
		
	g_free (current_text);
}

GtkWidget *
target_optionmenu_create (void)
{
	GtkWidget *widget;

	widget = bacon_cd_selection_new ();
	g_object_set (widget, "file-image", TRUE, NULL);
	gtk_widget_show (widget);

	return widget;
}

static void
init_dialog (GladeXML *xml, GtkWidget *dialog, GnomeVFSFileSize size, char *disc_name, gboolean from_iso)
{
	GtkWidget *option_menu;
	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *reuse;
	int size_mb;
	char *str, *last;
	char buf[129];
	GDate *now;

	label = glade_xml_get_widget (xml, "size_label");

	size_mb = (size + 1024 * 1024 - 1)/ (1024 * 1024);
	str = g_strdup_printf (_("%d MB"), size_mb);
	gtk_label_set_markup (GTK_LABEL (label), str);
	g_free (str);

	/* Fill in targets */
	option_menu = glade_xml_get_widget (xml, "target_optionmenu");
	g_signal_connect (option_menu, "device-changed",
			G_CALLBACK (target_changed), xml);

	/* Set default name */
	entry = glade_xml_get_widget (xml, "cdname_entry");
	g_signal_connect (entry, "insert_text", G_CALLBACK (cdname_entry_insert_text),
			  NULL);
	if (from_iso != FALSE) {
		gtk_widget_set_sensitive (entry, FALSE);
		if (disc_name != NULL) {
			gtk_entry_set_text (GTK_ENTRY (entry), disc_name);
			gtk_editable_set_editable (GTK_EDITABLE (entry), FALSE);
		}
	} else {
		now = g_date_new ();

		g_date_set_time (now, time (NULL));

		/*
		translators: see strftime man page for meaning of %b, %d and %Y
		the maximum length for this field is 32 bytes
		*/
		g_date_strftime (buf, sizeof (buf), _("Personal Data, %b %d, %Y"), now);

		g_date_free (now);

		/* Cut off at 32 bytes */
		last = str = buf;
		while (*str != 0 &&
		       (str - buf) < MAX_ISO_NAME_LEN) {
			last = str;
			str = g_utf8_next_char (str);
		}
		if (*str != 0) {
			*last = 0;
		}
		
		gtk_entry_set_text (GTK_ENTRY (entry), buf);
	}

	reuse = glade_xml_get_widget (xml, "reuse_check");
	g_assert (GTK_IS_WIDGET (reuse));
	gtk_widget_set_sensitive (reuse, !from_iso);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (reuse), TRUE);

	refresh_dialog (xml);
}

static gboolean
ask_cancel (void)
{
	GtkWidget *warning_dialog;
	int res;

	warning_dialog = ncb_hig_dialog (GTK_MESSAGE_WARNING,
			_("Cancel write"),
			_("Are you sure you want to cancel the disc write operation?\n"
			"Some optical media drives may require that you restart the machine to get it working again."),
			cd_progress_get_window ());
	gtk_dialog_add_buttons (GTK_DIALOG (warning_dialog),
			_("Continue"), GTK_RESPONSE_NO,
			GTK_STOCK_CANCEL, GTK_RESPONSE_YES,
			NULL);
	res = gtk_dialog_run (GTK_DIALOG (warning_dialog));
	gtk_widget_destroy (warning_dialog);

	return (res == GTK_RESPONSE_YES);
}

static void
do_cancel (void)
{
	if (cancel == CANCEL_NONE) {
		return;
	}

	if (cancel == CANCEL_MAKE_ISO) {
		make_iso_cancel ();
		return;
	}

	if (cd_recorder_cancel (cdrecorder, TRUE) != FALSE) {
		if (ask_cancel () == FALSE) {
			return;
		}
	}

	cd_recorder_cancel (cdrecorder, FALSE);
}

static gboolean
handle_delete_event (GtkWidget	     *widget,
		     GdkEventAny     *event)
{
	do_cancel ();
	return TRUE;
}

static void
cancel_clicked (GtkButton *button, gpointer data)
{
	do_cancel ();
}

static void
help_activate (GtkWindow *parent)
{
	GError *err = NULL;

	if (gnome_help_display_desktop (NULL, "user-guide", "user-guide.xml", "gosnautilus-475", &err) == FALSE) {
		char *msg;

		msg = g_strdup_printf (_("There was a problem displaying the help contents: %s."), err->message);
		ncb_hig_show_error_dialog (_("Cannot display help"),
				msg, parent);
		g_error_free (err);
		g_free (msg);
	}
}

static gboolean
verify_iso (const char *filename, char **iso_label)
{
	FILE *file;
#define BUFFER_SIZE 128
	char buf[BUFFER_SIZE+1];
	int res;
	char *str, *str2;

	file = fopen (filename, "rb");
	if (file == NULL) {
		return FALSE;
	}
	/* Verify we have an ISO image */
	/* This check is for the raw sector images */
	res = fseek (file, 37633L, SEEK_SET);
	if (res) {
		return FALSE;
	}
	res = fread (buf, sizeof (char), 5, file);
	if (res != 5 || strncmp (buf, "CD001", 5) != 0) {
		/* Standard ISO images */
		res = fseek (file, 32769L, SEEK_SET);
		if (res) {
			return FALSE;
		}
		res = fread (buf, sizeof (char), 5, file);
		if (res != 5 || strncmp (buf, "CD001", 5) != 0) {
			/* High Sierra images */
			res = fseek (file, 32776L, SEEK_SET);
			if (res) {
				return FALSE;
			}
			res = fread (buf, sizeof (char), 5, file);
			if (res != 5 || strncmp (buf, "CDROM", 5) != 0) {
				return FALSE;
			}
		}
	}
	/* Extract the volume label from the image */
	res = fseek (file, 32808L, SEEK_SET);
	if (res) {
		return FALSE;
	}
	res = fread (buf, sizeof(char), BUFFER_SIZE, file);
	if (res != BUFFER_SIZE) {
		return FALSE;
	}
	buf[BUFFER_SIZE] = '\0';
	str = g_strdup (g_strstrip (buf));
	if (!g_utf8_validate (str, -1, NULL)) {
		/* Hmm, not UTF-8. Try the current locale. */
		str2 = g_locale_to_utf8 (str, -1, NULL, NULL, NULL);
		if (str2 == NULL) {
			str2 = ncb_make_valid_utf8 (str);
		}
		g_free (str);
		str = str2;
	}
	fclose (file);
	*iso_label = str;
	return TRUE;
}

static GdkPixbuf *
my_gdk_pixbuf_new_from_stock (const char *name)
{
	GtkWidget *label;
	GdkPixbuf *icon;

	label = gtk_label_new ("");
	icon = gtk_widget_render_icon (label, name, GTK_ICON_SIZE_BUTTON, NULL);
	gtk_widget_destroy (label);

	return icon;
}

int
main (int argc, char *argv[])
{
	const CDDrive *rec;
	GtkWidget *dialog, *entry, *eject_check, *reuse_check, *progress_window;
	GtkWidget *cancel_button, *dummy_check, *option_menu, *item;
	const char *label;
	int res, speed, i;
	gboolean eject, reuse, dummy, debug, overburn, burnproof;
	GnomeVFSURI *uri;
	GnomeVFSFileSize size;
	char *source_iso, *disc_name;
	struct stat stat_buf;
	GConfClient *gc;
	GdkPixbuf *icon;

	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	disc_name = NULL;

	gnome_program_init ("nautilus-cd-burner", VERSION,
			    LIBGNOMEUI_MODULE, argc, argv,
			    GNOME_PARAM_APP_DATADIR, SHAREDIR,
			    GNOME_PARAM_NONE);
    
	gnome_vfs_init ();

	if (argc > 2) {
		dialog = ncb_hig_dialog (GTK_MESSAGE_ERROR,
				_("Too many parameters"),
				_("Too many parameters were passed to the application."),
				NULL);
		gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_OK,
				GTK_RESPONSE_OK);
		res = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
		return 1;
	}

	source_iso = NULL;
	for (i = 1; i < argc; i++) {
		if (strncmp ("--", argv[i], 2) != 0) {
			source_iso = argv[i];
			break;
		}
	}

	if (source_iso != NULL) {
		if (!verify_iso (source_iso, &disc_name)) {
			char *msg;

			msg = g_strdup_printf
				(_("The file '%s' is not a valid CD image."),
				 source_iso);
			dialog = ncb_hig_dialog (GTK_MESSAGE_ERROR,
					_("Not a valid CD image"),
					msg, NULL);
			g_free (msg);
			res = gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
			return 1;
		}
		/* TODO: is this close enough for the CD size? */
		res = stat (source_iso, &stat_buf);
		size = stat_buf.st_size;
	} else {
		size = estimate_size ("burn:///");

		if (size == 0) {
			dialog = ncb_hig_dialog (GTK_MESSAGE_ERROR,
					_("No files selected"),
					_("You need to copy the files you want to write to disc to the CD/DVD Creator window. Would you like to open it now?"),
					NULL);
			gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, _("Open CD/DVD Creator"), GTK_RESPONSE_YES, NULL);
			gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
			res = gtk_dialog_run (GTK_DIALOG (dialog));
			gtk_widget_destroy (dialog);
			if (res == GTK_RESPONSE_YES) {
				char *argv[] = {"nautilus", "--no-desktop", "burn:///", NULL};
				g_spawn_async (NULL, argv, NULL,
					       G_SPAWN_SEARCH_PATH,
					       NULL, NULL, NULL, NULL);
			}
			return 1;
		}
	}

	if (g_file_test ("cdburn.glade", G_FILE_TEST_EXISTS) != FALSE) {
		xml = glade_xml_new (DATADIR "/cdburn.glade", NULL, NULL);
	} else {
		xml = glade_xml_new (DATADIR "/cdburn.glade", NULL, NULL);
	}
	dialog = glade_xml_get_widget (xml, "cdr_dialog");

	init_dialog (xml, dialog, size, disc_name, source_iso != NULL);

	if (disc_name != NULL) {
		g_free (disc_name);
	}

	gtk_widget_show (dialog);

	dummy_check = glade_xml_get_widget (xml, "dummy_check");

	gc = gconf_client_get_default ();
	debug = gconf_client_get_bool (gc, "/apps/nautilus-cd-burner/debug", NULL);
	overburn = gconf_client_get_bool (gc, "/apps/nautilus-cd-burner/overburn", NULL);
	burnproof = gconf_client_get_bool (gc, "/apps/nautilus-cd-burner/burnproof", NULL);
	g_object_unref (G_OBJECT (gc));

	if (debug == FALSE) {
		gtk_widget_hide (dummy_check);
	}

	icon = my_gdk_pixbuf_new_from_stock ("gtk-cdrom");
	gtk_window_set_icon (GTK_WINDOW (dialog), icon);
	progress_window = glade_xml_get_widget (xml, "progress_window");
	gtk_window_set_icon (GTK_WINDOW (progress_window), icon);
	gdk_pixbuf_unref (icon);

	do {
		res = gtk_dialog_run (GTK_DIALOG (dialog));

		if (res == GTK_RESPONSE_HELP) {
			help_activate (GTK_WINDOW (dialog));
		}
	} while (res == GTK_RESPONSE_HELP);

	gtk_widget_hide (dialog);
	cd_progress_image_setup ();
	gtk_widget_show (progress_window);
	cancel_button = glade_xml_get_widget (xml, "cancel_button");
	g_signal_connect (cancel_button, "clicked", (GCallback)cancel_clicked, NULL);
	g_signal_connect (progress_window, "delete_event", (GCallback)handle_delete_event, NULL);

	if (res == 0) {
		rec = lookup_current_recorder (xml);

		entry = glade_xml_get_widget (xml, "cdname_entry");
		label = gtk_entry_get_text (GTK_ENTRY (entry));

		eject_check = glade_xml_get_widget (xml, "eject_check");
		eject = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (eject_check));

		dummy = FALSE;

		if (debug) {
			dummy = gtk_toggle_button_get_active
				(GTK_TOGGLE_BUTTON (dummy_check));
		}

		option_menu = glade_xml_get_widget (xml, "speed_optionmenu");
		item = gtk_menu_get_active (GTK_MENU (gtk_option_menu_get_menu (GTK_OPTION_MENU (option_menu))));
		speed = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (item), "speed"));
		gc = gconf_client_get_default ();
		gconf_client_set_int (gc, "/apps/nautilus-cd-burner/default_speed", speed, NULL);
		g_object_unref (G_OBJECT (gc));

		res = burn_cd (rec, source_iso, label, eject, speed, dummy, debug, overburn, burnproof);

		reuse_check = glade_xml_get_widget (xml, "reuse_check");
		reuse = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (reuse_check));

		if (!reuse && res == RESULT_FINISHED) {
			GList l;
			
			uri = gnome_vfs_uri_new ("burn:///");
			
			l.data = uri;
			l.next = NULL;
			l.prev = NULL;

			gnome_vfs_xfer_delete_list (&l,
						    GNOME_VFS_XFER_ERROR_MODE_ABORT,
						    GNOME_VFS_XFER_RECURSIVE,
						    NULL, NULL);
		}
	}

	cd_progress_image_cleanup ();
	gtk_widget_destroy (dialog);

	return 0;
}

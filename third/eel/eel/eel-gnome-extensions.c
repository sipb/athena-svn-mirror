/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/* eel-gnome-extensions.c - implementation of new functions that operate on
                            gnome classes. Perhaps some of these should be
  			    rolled into gnome someday.

   Copyright (C) 1999, 2000, 2001 Eazel, Inc.

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

   Authors: Darin Adler <darin@eazel.com>
*/

#include <config.h>
#include "eel-gnome-extensions.h"

#include "eel-art-extensions.h"
#include "eel-gdk-extensions.h"
#include "eel-glib-extensions.h"
#include "eel-gtk-extensions.h"
#include "eel-stock-dialogs.h"
#include "eel-i18n.h"
#include <X11/Xatom.h>
#include <errno.h>
#include <fcntl.h>
#include <gdk/gdkx.h>
#include <gtk/gtkwidget.h>
#include <libart_lgpl/art_rect.h>
#include <libart_lgpl/art_rgb.h>
#include <libgnome/gnome-exec.h>
#include <libgnome/gnome-util.h>
#include <libgnomeui/gnome-file-entry.h>
#include <libgnomeui/gnome-icon-list.h>
#include <libgnomeui/gnome-icon-sel.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo-activation/bonobo-activation.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

/* structure for the icon selection dialog */
struct IconSelectionData {
	GtkWidget *dialog;
        GtkWidget *icon_selection;
	GtkWidget *file_entry;
	GtkWindow *owning_window;
	gboolean dismissed;
	EelIconSelectionFunction selection_function;
	gpointer callback_data;
};

typedef struct IconSelectionData IconSelectionData;

ArtIRect
eel_gnome_canvas_world_to_canvas_rectangle (GnomeCanvas *canvas,
					    ArtDRect world_rect)
{
	ArtIRect canvas_rect;

	g_return_val_if_fail (GNOME_IS_CANVAS (canvas), eel_art_irect_empty);

	gnome_canvas_w2c (GNOME_CANVAS (canvas),
			  world_rect.x0,
			  world_rect.y0,
			  &canvas_rect.x0,
			  &canvas_rect.y0);
	gnome_canvas_w2c (GNOME_CANVAS (canvas),
			  world_rect.x1,
			  world_rect.y1,
			  &canvas_rect.x1,
			  &canvas_rect.y1);

	return canvas_rect;
}

ArtIRect
eel_gnome_canvas_item_get_current_canvas_bounds (GnomeCanvasItem *item)
{
	ArtIRect bounds;

	g_return_val_if_fail (GNOME_IS_CANVAS_ITEM (item), eel_art_irect_empty);

	bounds.x0 = item->x1;
	bounds.y0 = item->y1;
	bounds.x1 = item->x2;
	bounds.y1 = item->y2;

	return bounds;
}

void
eel_gnome_canvas_item_request_redraw (GnomeCanvasItem *item)
{
	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	gnome_canvas_request_redraw (item->canvas,
				     item->x1, item->y1,
				     item->x2, item->y2);
}

void
eel_gnome_canvas_request_redraw_rectangle (GnomeCanvas *canvas,
					   ArtIRect canvas_rectangle)
{
	g_return_if_fail (GNOME_IS_CANVAS (canvas));
	
	gnome_canvas_request_redraw (canvas,
				     canvas_rectangle.x0, canvas_rectangle.y0,
				     canvas_rectangle.x1, canvas_rectangle.y1);
}

ArtDRect
eel_gnome_canvas_item_get_world_bounds (GnomeCanvasItem *item)
{
	ArtDRect world_bounds;

	g_return_val_if_fail (GNOME_IS_CANVAS_ITEM (item), eel_art_drect_empty);

	gnome_canvas_item_get_bounds (item,
				      &world_bounds.x0,
				      &world_bounds.y0,
				      &world_bounds.x1,
				      &world_bounds.y1);
	if (item->parent != NULL) {
		gnome_canvas_item_i2w (item->parent,
				       &world_bounds.x0,
				       &world_bounds.y0);
		gnome_canvas_item_i2w (item->parent,
				       &world_bounds.x1,
				       &world_bounds.y1);
	}

	return world_bounds;
}

ArtIRect
eel_gnome_canvas_item_get_canvas_bounds (GnomeCanvasItem *item)
{
	ArtDRect world_bounds;

	g_return_val_if_fail (GNOME_IS_CANVAS_ITEM (item), eel_art_irect_empty);

	world_bounds = eel_gnome_canvas_item_get_world_bounds (item);

	return eel_gnome_canvas_world_to_canvas_rectangle 
		(item->canvas, world_bounds);
}

static void
eel_gnome_canvas_draw_pixbuf_helper (art_u8 *dst, int dst_rowstride,
				     const art_u8 *src, int src_rowstride,
				     int copy_width, int copy_height)
{
	art_u8 *dst_limit = dst + copy_height * dst_rowstride;
	int dst_bytes_per_row = copy_width * 3;
	
	while (dst < dst_limit) {
 		memcpy (dst, src, dst_bytes_per_row);
		dst += dst_rowstride;
		src += src_rowstride;
	}
}

static void
eel_gnome_canvas_draw_pixbuf_helper_alpha (art_u8 *dst, int dst_rowstride,
					   const art_u8 *src, int src_rowstride,
					   int copy_width, int copy_height)
{
	art_u8 *dst_limit = dst + copy_height * dst_rowstride;
	int dst_bytes_per_row = copy_width * 3;
	
	while (dst < dst_limit) {
	
		art_u8 *dst_p = dst;
		art_u8 *dst_p_limit = dst + dst_bytes_per_row;
		
		const art_u8 *src_p = src;
		
		while (dst_p < dst_p_limit) {
			int alpha = src_p[3];
			if (alpha) {
				if (alpha == 255) {
					dst_p[0] = src_p[0];
					dst_p[1] = src_p[1];
					dst_p[2] = src_p[2];
				} else {
		  			int tmp;
					art_u8 bg_r = dst_p[0];
					art_u8 bg_g = dst_p[1];
					art_u8 bg_b = dst_p[2];

					tmp = (src_p[0] - bg_r) * alpha;
					dst_p[0] = bg_r + ((tmp + (tmp >> 8) + 0x80) >> 8);
					tmp = (src_p[1] - bg_g) * alpha;
					dst_p[1] = bg_g + ((tmp + (tmp >> 8) + 0x80) >> 8);
					tmp = (src_p[2] - bg_b) * alpha;
					dst_p[2] = bg_b + ((tmp + (tmp >> 8) + 0x80) >> 8);		  
				}
			}
			
			dst_p += 3;
			src_p += 4;
		}
		
		dst += dst_rowstride;
		src += src_rowstride;
	}
}

/* Draws a pixbuf into a canvas update buffer (unscaled). The x,y coords are the location
 * of the pixbuf in canvas space (NOT relative to the canvas buffer).
 */
void
eel_gnome_canvas_draw_pixbuf (GnomeCanvasBuf *buf, const GdkPixbuf *pixbuf, int x, int y)
{
	art_u8 *dst;
	int pixbuf_width, pixbuf_height;

	/* copy_left/top/right/bottom define the rect of the pixbuf (pixbuf relative)
	 * we will copy into the canvas buffer
	 */
	int copy_left, copy_top, copy_right, copy_bottom;
	
	dst = buf->buf;

	pixbuf_width = gdk_pixbuf_get_width (pixbuf);
	pixbuf_height = gdk_pixbuf_get_height (pixbuf);

	if (x > buf->rect.x0) {
		copy_left = 0;
		dst += (x - buf->rect.x0) * 3;
	} else {
		copy_left = buf->rect.x0 - x;
	}
	
	if (x + pixbuf_width > buf->rect.x1) {
		copy_right = buf->rect.x1 - x;
	} else {
		copy_right = pixbuf_width;		
	}
	
	if (copy_left >= copy_right) {
		return;
	}
	
	if (y > buf->rect.y0) {
		dst += (y - buf->rect.y0) * buf->buf_rowstride;
		copy_top = 0;
	} else {
		copy_top = buf->rect.y0 - y;
	}
	
	if (y + pixbuf_height > buf->rect.y1) {
		copy_bottom = buf->rect.y1 - y;
	} else {
		copy_bottom = pixbuf_height;		
	}

	if (copy_top >= copy_bottom) {
		return;
	}

	if (gdk_pixbuf_get_has_alpha (pixbuf)) {
		eel_gnome_canvas_draw_pixbuf_helper_alpha (
			dst,
			buf->buf_rowstride,
			gdk_pixbuf_get_pixels (pixbuf) + copy_left * 4 + copy_top * gdk_pixbuf_get_rowstride (pixbuf),
			gdk_pixbuf_get_rowstride (pixbuf),
			copy_right - copy_left,
			copy_bottom - copy_top);
	} else {
		eel_gnome_canvas_draw_pixbuf_helper (
			dst,
			buf->buf_rowstride,
			gdk_pixbuf_get_pixels (pixbuf) + copy_left * 3 + copy_top * gdk_pixbuf_get_rowstride (pixbuf),
			gdk_pixbuf_get_rowstride (pixbuf),
			copy_right - copy_left,
			copy_bottom - copy_top);
	}
}

void
eel_gnome_canvas_fill_rgb (GnomeCanvasBuf *buf, art_u8 r, art_u8 g, art_u8 b)
{
	art_u8 *dst = buf->buf;
	int width = buf->rect.x1 - buf->rect.x0;
	int height = buf->rect.y1 - buf->rect.y0;

	if (buf->buf_rowstride == width * 3) {
	 	art_rgb_fill_run (dst, r, g, b, width * height);
	} else {
		art_u8 *dst_limit = dst + height * buf->buf_rowstride;
		while (dst < dst_limit) {
	 		art_rgb_fill_run (dst, r, g, b, width);
			dst += buf->buf_rowstride;
		}
	}
}

void
eel_gnome_canvas_item_request_update_deep (GnomeCanvasItem *item)
{
	GList *p;

	gnome_canvas_item_request_update (item);
	if (GNOME_IS_CANVAS_GROUP (item)) {
		for (p = GNOME_CANVAS_GROUP (item)->item_list; p != NULL; p = p->next) {
			eel_gnome_canvas_item_request_update_deep (p->data);
		}
	}
}

void
eel_gnome_canvas_request_update_all (GnomeCanvas *canvas)
{
	eel_gnome_canvas_item_request_update_deep (canvas->root);
}

/* The gnome_canvas_set_scroll_region function doesn't do an update,
 * even though it should. The update is in there with an #if 0 around
 * it, with no explanation of why it's commented out. For now, work
 * around this by requesting an update explicitly.
 * ALEX: The update was causing us to repaint everything on every
 * relayout. An object may have data stored that depends on x1 and y1,
 * since they decide the coordinate space transform. Nothing should
 * depend on x2 and y2 though. This lets us grow the canvas to the
 * right and down without performance problems.
 */
void
eel_gnome_canvas_set_scroll_region (GnomeCanvas *canvas,
				    double x1, double y1,
				    double x2, double y2)
{
	double old_x1, old_y1, old_x2, old_y2;

	/* Change the scroll region and do an update if it changes. */
	gnome_canvas_get_scroll_region (canvas, &old_x1, &old_y1, &old_x2, &old_y2);
	if (old_x1 != x1 || old_y1 != y1 || old_x2 != x2 || old_y2 != y2) {
		gnome_canvas_set_scroll_region (canvas, x1, y1, x2, y2);
		if (old_x1 != x1 || old_y1 != y1) {
			eel_gnome_canvas_request_update_all (canvas);
		}
		gnome_canvas_item_request_update (canvas->root);
	}
}

/* The code in GnomeCanvas (the scroll_to function to be exact) always
 * centers the contents of the canvas if the contents are smaller than
 * the canvas, and it does some questionable math when computing
 * that. This code is working to undo that mistake.
 */
void
eel_gnome_canvas_set_scroll_region_left_justify (GnomeCanvas *canvas,
						 double x1, double y1,
						 double x2, double y2)
{
	double height, width;

	/* To work around the logic in scroll_to that centers the
	 * canvas contents if they are smaller than the canvas widget,
	 * we must do the exact opposite of what it does. The -1 here
	 * is due to the ill-conceived ++ in scroll_to.
	 */
	width = (GTK_WIDGET (canvas)->allocation.width - 1) / canvas->pixels_per_unit;
	height = (GTK_WIDGET (canvas)->allocation.height - 1) / canvas->pixels_per_unit;
	eel_gnome_canvas_set_scroll_region
		(canvas, x1, y1,
		 MAX (x2, x1 + width), MAX (y2, y1 + height));
}

/* Set a new scroll region without eliminating any of the currently-visible area. */
void
eel_gnome_canvas_set_scroll_region_include_visible_area (GnomeCanvas *canvas,
							 double x1, double y1,
							 double x2, double y2)
{
	double old_x1, old_y1, old_x2, old_y2;
	double old_scroll_x, old_scroll_y;
	double height, width;

	gnome_canvas_get_scroll_region (canvas, &old_x1, &old_y1, &old_x2, &old_y2);

	/* The -1 here is due to the ill-conceived ++ in scroll_to. */
	width = (GTK_WIDGET (canvas)->allocation.width - 1) / canvas->pixels_per_unit;
	height = (GTK_WIDGET (canvas)->allocation.height - 1) / canvas->pixels_per_unit;

	old_scroll_x = gtk_layout_get_hadjustment (GTK_LAYOUT (canvas))->value;
	old_scroll_y = gtk_layout_get_vadjustment (GTK_LAYOUT (canvas))->value;

	x1 = MIN (x1, old_x1 + old_scroll_x);
	y1 = MIN (y1, old_y1 + old_scroll_y);
	x2 = MAX (x2, old_x1 + old_scroll_x + width);
	y2 = MAX (y2, old_y1 + old_scroll_y + height);

	eel_gnome_canvas_set_scroll_region
		(canvas, x1, y1, x2, y2);
}


void 
eel_gnome_shell_execute (const char *command)
{
	GError *error = NULL;
	if (!g_spawn_command_line_async (command, &error)) {
		g_warning ("Error starting command '%s': %s\n", command, error->message);
		g_error_free (error);
	}
}

/* Return a command string containing the path to a terminal on this system. */

static char *
try_terminal_command (const char *program,
		      const char *args)
{
	char *program_in_path, *quoted, *result;

	if (program == NULL) {
		return NULL;
	}

	program_in_path = g_find_program_in_path (program);
	if (program_in_path == NULL) {
		return NULL;
	}

	quoted = g_shell_quote (program_in_path);
	if (args == NULL || args[0] == '\0') {
		return quoted;
	}
	result = g_strconcat (quoted, " ", args, NULL);
	g_free (quoted);
	return result;
}

static char *
try_terminal_command_argv (int argc,
			   char **argv)
{
	GString *string;
	int i;
	char *quoted, *result;

	if (argc == 0) {
		return NULL;
	}

	if (argc == 1) {
		return try_terminal_command (argv[0], NULL);
	}
	
	string = g_string_new (argv[1]);
	for (i = 2; i < argc; i++) {
		quoted = g_shell_quote (argv[i]);
		g_string_append_c (string, ' ');
		g_string_append (string, quoted);
		g_free (quoted);
	}
	result = try_terminal_command (argv[0], string->str);
	g_string_free (string, TRUE);

	return result;
}

static char *
get_terminal_command_prefix (gboolean for_command)
{
	int argc;
	char **argv;
	char *command;
	guint i;
	static const char *const commands[][3] = {
		{ "gnome-terminal", "-x",                                      "" },
		{ "dtterm",         "-e",                                      "-ls" },
		{ "nxterm",         "-e",                                      "-ls" },
		{ "color-xterm",    "-e",                                      "-ls" },
		{ "rxvt",           "-e",                                      "-ls" },
		{ "xterm",          "-e",                                      "-ls" },
	};

	/* Try the terminal from preferences. Use without any
	 * arguments if we are just doing a standalone terminal.
	 */
	argc = 0;
	argv = g_new0 (char *, 1);
	gnome_prepend_terminal_to_vector (&argc, &argv);

	command = NULL;
	if (argc != 0) {
		if (for_command) {
			command = try_terminal_command_argv (argc, argv);
		} else {
			/* Strip off the arguments in a lame attempt
			 * to make it be an interactive shell.
			 */
			command = try_terminal_command (argv[0], NULL);
		}
	}

	while (argc != 0) {
		g_free (argv[--argc]);
	}
	g_free (argv);

	if (command != NULL) {
		return command;
	}

	/* Try well-known terminal applications in same order that gmc did. */
	for (i = 0; i < G_N_ELEMENTS (commands); i++) {
		command = try_terminal_command (commands[i][0],
						commands[i][for_command ? 1 : 2]);
		if (command != NULL) {
			break;
		}
	}
	
	return command;
}

char *
eel_gnome_make_terminal_command (const char *command)
{
	char *prefix, *quoted, *terminal_command;

	if (command == NULL) {
		return get_terminal_command_prefix (FALSE);
	}
	prefix = get_terminal_command_prefix (TRUE);
	quoted = g_shell_quote (command);
	terminal_command = g_strconcat (prefix, " /bin/sh -c ", quoted, NULL);
	g_free (prefix);
	g_free (quoted);
	return terminal_command;
}

void
eel_gnome_open_terminal (const char *command)
{
	char *command_line;
	
	command_line = eel_gnome_make_terminal_command (command);
	if (command_line == NULL) {
		g_message ("Could not start a terminal");
		return;
	}
	eel_gnome_shell_execute (command_line);
	g_free (command_line);
}

/* create a new icon selection dialog */
static gboolean
widget_destroy_callback (gpointer callback_data)
{
	IconSelectionData *selection_data;

	selection_data = (IconSelectionData *) callback_data;
	gtk_widget_destroy (selection_data->dialog);
	g_free (selection_data);	
	return FALSE;
}

/* set the image of the file object to the selected file */
static void
icon_selected (IconSelectionData *selection_data)
{
	const char *icon_path;
	struct stat buf;
	GtkWidget *entry;
	
	gnome_icon_selection_stop_loading (GNOME_ICON_SELECTION (selection_data->icon_selection));

	/* Hide the dialog now, so the user can't press the buttons multiple
	   times. */
	gtk_widget_hide (selection_data->dialog);

	/* If we've already acted on the dialog, just return. */
	if (selection_data->dismissed)
		return;

	/* Set the flag to indicate we've acted on the dialog and are about
	   to destroy it. */
	selection_data->dismissed = TRUE;

	entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (selection_data->file_entry));
	icon_path = gtk_entry_get_text (GTK_ENTRY (entry));
	
	/* if a specific file wasn't selected, put up a dialog to tell the
	 * user to pick something, and leave the picker up
	 */
	stat (icon_path, &buf);
	if (S_ISDIR (buf.st_mode)) {
		eel_show_error_dialog (_("No image was selected.  You must click on an image to select it."),
				       _("No selection made"),
				       selection_data->owning_window);
	} else {	 
		/* invoke the callback to inform it of the file path */
		selection_data->selection_function (icon_path, selection_data->callback_data);
	}
		
	/* We have to get rid of the icon selection dialog, but we can't do it now, since the
	 * file entry might still need it. Do it when the next idle rolls around
	 */ 
	gtk_idle_add (widget_destroy_callback, selection_data);
}

/* handle the cancel button being pressed */
static void
icon_cancel_pressed (IconSelectionData *selection_data)
{
	/* nothing to do if it's already been dismissed */
	if (selection_data->dismissed) {
		return;
	}
	
	gtk_widget_destroy (selection_data->dialog);
	g_free (selection_data);
}

/* handle an icon being selected by updating the file entry */
static void
list_icon_selected_callback (GnomeIconList *gil, int num, GdkEvent *event, IconSelectionData *selection_data)
{
	const char *icon;
	GtkWidget *entry;
	
	icon = gnome_icon_selection_get_icon (GNOME_ICON_SELECTION (selection_data->icon_selection), TRUE);
	if (icon != NULL) {
		entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (selection_data->file_entry));
		gtk_entry_set_text (GTK_ENTRY (entry), icon);
	}

	/* handle double-clicks as if the user pressed OK */
	if (event != NULL && event->type == GDK_2BUTTON_PRESS && ((GdkEventButton *) event)->button == 1) {
		icon_selected (selection_data);
	}
}

static void
dialog_response_callback (GtkWidget *dialog, int response_id, IconSelectionData *selection_data)
{
	switch (response_id) {
	case GTK_RESPONSE_OK:
		icon_selected (selection_data);
		break;
	case GTK_RESPONSE_CANCEL:
	case GTK_RESPONSE_DELETE_EVENT:
		icon_cancel_pressed (selection_data);
		break;
	default:
		break;
	}
}

/* handle the file entry changing */
static void
entry_activated_callback (GtkWidget *widget, IconSelectionData *selection_data)
{
	struct stat buf;
	GnomeIconSelection *icon_selection;
	const char *filename;

	filename = gtk_entry_get_text (GTK_ENTRY (widget));
	if (filename == NULL) {
		return;
	}
	
	if (stat (filename, &buf) == 0 && S_ISDIR (buf.st_mode)) {
		icon_selection = GNOME_ICON_SELECTION (selection_data->icon_selection);
		gnome_icon_selection_clear (icon_selection, TRUE);
		gnome_icon_selection_add_directory (icon_selection, filename);
		gnome_icon_selection_show_icons(icon_selection);
	} else {
		/* We pretend like ok has been called */
		icon_selected (selection_data);
	}
}

/* here's the actual routine that creates the icon selector.
   Note that this may return NULL if the dialog was destroyed before the
   icons were all loaded. GnomeIconSelection reenters the main loop while
   it loads the icons, so beware! */

GtkWidget *
eel_gnome_icon_selector_new (const char *title,
			     const char *icon_directory,
			     GtkWindow *owner,
			     EelIconSelectionFunction selected,
			     gpointer callback_data)
{
	GtkWidget *dialog, *icon_selection, *retval;
	GtkWidget *entry, *file_entry;
	IconSelectionData *selection_data;
	
	dialog = gtk_dialog_new_with_buttons (title, owner, 0, 
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OK, GTK_RESPONSE_OK,
					      NULL);

	gtk_window_set_resizable (GTK_WINDOW (dialog), TRUE);

	icon_selection = gnome_icon_selection_new ();

	file_entry = gnome_file_entry_new (NULL,NULL);
	
	selection_data = g_new0 (IconSelectionData, 1);
	selection_data->dialog = dialog;
 	selection_data->icon_selection = icon_selection;
 	selection_data->file_entry = file_entry;
 	selection_data->owning_window = owner;
	selection_data->selection_function = selected;
 	selection_data->callback_data = callback_data;
 	
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
			    file_entry, FALSE, FALSE, 0);

	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
			    icon_selection, TRUE, TRUE, 0);

	gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_MOUSE);
	if (owner != NULL) {
		gtk_window_set_transient_for (GTK_WINDOW (dialog), owner);
 	}
 	gtk_window_set_wmclass (GTK_WINDOW (dialog), "file_selector", "Eel");
	gtk_widget_show_all (dialog);
	
	entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (file_entry));
	
	if (icon_directory == NULL) {
		gtk_entry_set_text (GTK_ENTRY (entry), DATADIR "/pixmaps");
		gnome_icon_selection_add_directory (GNOME_ICON_SELECTION (icon_selection), DATADIR "/pixmaps");	
	} else {
		gtk_entry_set_text (GTK_ENTRY (entry), icon_directory);
		gnome_icon_selection_add_directory (GNOME_ICON_SELECTION (icon_selection), icon_directory);
	}
	
	g_signal_connect (dialog, "response",
			  G_CALLBACK (dialog_response_callback), selection_data);

	g_signal_connect_after (gnome_icon_selection_get_gil (GNOME_ICON_SELECTION (selection_data->icon_selection)),
				"select_icon",
				G_CALLBACK (list_icon_selected_callback), selection_data);

	g_signal_connect (entry, "activate",
			  G_CALLBACK (entry_activated_callback), selection_data);

	/* We add a weak pointer to the dialog, so we know if it has been
	   destroyed while the icons are being loaded. */
	eel_add_weak_pointer (&dialog);

	gnome_icon_selection_show_icons (GNOME_ICON_SELECTION (icon_selection));

	/* eel_remove_weak_pointer() will set dialog to NULL, so we need to
	   remember the dialog pointer here, if there is one. */
	retval = dialog;

	/* Now remove the weak pointer, if the dialog still exists. */
	eel_remove_weak_pointer (&dialog);

	return retval;
}

PangoContext *
eel_gnome_canvas_get_pango_context (GnomeCanvas *canvas)
{
	if (!canvas->aa) {
		return gtk_widget_get_pango_context (GTK_WIDGET (canvas));
	} else {
		return eel_gtk_widget_get_pango_ft2_context (GTK_WIDGET (canvas));
	}
}

void
eel_gnome_canvas_item_send_behind (GnomeCanvasItem *item,
				   GnomeCanvasItem *behind_item)
{
	GList *item_list;
	int item_position, behind_position;

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (item));

	if (behind_item == NULL) {
		gnome_canvas_item_raise_to_top (item);
		return;
	}

	g_return_if_fail (GNOME_IS_CANVAS_ITEM (behind_item));
	g_return_if_fail (item->parent == behind_item->parent);

	item_list = GNOME_CANVAS_GROUP (item->parent)->item_list;

	item_position = g_list_index (item_list, item);
	g_assert (item_position != -1);
	behind_position = g_list_index (item_list, behind_item);
	g_assert (behind_position != -1);
	g_assert (item_position != behind_position);

	if (item_position == behind_position - 1) {
		return;
	}

	if (item_position < behind_position) {
		gnome_canvas_item_raise (item, (behind_position - 1) - item_position);
	} else {
		gnome_canvas_item_lower (item, item_position - behind_position);
	}
}

ArtIRect
eel_gnome_canvas_world_to_widget_rectangle (GnomeCanvas *canvas,
					    ArtDRect world_rect)
{
	ArtIRect widget_rect;
	
	g_return_val_if_fail (GNOME_IS_CANVAS (canvas), eel_art_irect_empty);

	eel_gnome_canvas_world_to_widget (canvas,
					  world_rect.x0, world_rect.y0,
					  &widget_rect.x0, &widget_rect.y0);
	eel_gnome_canvas_world_to_widget (canvas,
					  world_rect.x1, world_rect.y1,
					  &widget_rect.x1, &widget_rect.y1);

	return widget_rect;
}

void
eel_gnome_canvas_widget_to_world (GnomeCanvas *canvas,
				  int          widget_x,
				  int          widget_y,
				  double      *world_x,
				  double      *world_y)
{
	int bin_window_x, bin_window_y;
	
	g_return_if_fail (GNOME_IS_CANVAS (canvas));

	bin_window_x = widget_x + gtk_adjustment_get_value (gtk_layout_get_hadjustment (GTK_LAYOUT (canvas)));
	bin_window_y = widget_y + gtk_adjustment_get_value (gtk_layout_get_vadjustment (GTK_LAYOUT (canvas)));

	gnome_canvas_window_to_world (canvas,
				      bin_window_x, bin_window_y,
				      world_x, world_y);
}

void
eel_gnome_canvas_world_to_widget (GnomeCanvas *canvas,
				  double       world_x,
				  double       world_y,
				  int         *widget_x,
				  int         *widget_y)
{
	double bin_window_x, bin_window_y;

	g_return_if_fail (GNOME_IS_CANVAS (canvas));

	gnome_canvas_world_to_window (canvas,
				      world_x, world_y,
				      &bin_window_x, &bin_window_y);

	if (widget_x) {
		*widget_x = (int) bin_window_x - gtk_adjustment_get_value (gtk_layout_get_hadjustment (GTK_LAYOUT (canvas)));
	}
	if (widget_y) {
		*widget_y = (int) bin_window_y - gtk_adjustment_get_value (gtk_layout_get_vadjustment (GTK_LAYOUT (canvas)));
	}
}

char *
eel_bonobo_make_registration_id (const char *iid)
{
	return bonobo_activation_make_registration_id (
		iid, DisplayString (gdk_display));
}

static ORBit_IMethod *
get_set_value_imethod (void)
{
	static ORBit_IMethod *imethod = NULL;

	if (!imethod) {
		guint i;
		ORBit_IMethods *methods;

		methods = &Bonobo_PropertyBag__iinterface.methods;

		for (i = 0; i < methods->_length; i++) {
			if (!strcmp (methods->_buffer [i].name,
				     "setValue"))
				imethod = &methods->_buffer [i];
		}
		g_assert (imethod != NULL);
	}

	return imethod;
}

static void
do_nothing_cb (CORBA_Object          object,
	       ORBit_IMethod        *m_data,
	       ORBitAsyncQueueEntry *aqe,
	       gpointer              user_data, 
	       CORBA_Environment    *ev)
{
	/* FIXME: we can remove this when people
	   have the fixed ORB more prevelantly */
}

/**
 * eel_bonobo_pbclient_set_value_async:
 * @bag: a reference to the PropertyBag
 * @key: key of the value to set
 * @value: the new value
 * @opt_ev: an optional CORBA_Environment to return failure codes
 *
 * Set a value on the PropertyBag asynchronously, discarding any
 * possible roundtrip exceptions.
 */
void
eel_bonobo_pbclient_set_value_async (Bonobo_PropertyBag  bag,
				     const char         *key,
				     CORBA_any          *value,
				     CORBA_Environment  *opt_ev)
{
	gpointer args [2];
	CORBA_Environment ev, *my_ev;

	g_return_if_fail (key != NULL);
	g_return_if_fail (value != NULL);
	g_return_if_fail (bag != CORBA_OBJECT_NIL);

	if (!opt_ev) {
		CORBA_exception_init (&ev);
		my_ev = &ev;
	} else
		my_ev = opt_ev;

	if (BONOBO_EX (my_ev) || bag == CORBA_OBJECT_NIL) {
		if (!opt_ev)
			CORBA_exception_free (&ev);
		return;
	}

	args [0] = (gpointer) &key;
	args [1] = (gpointer) value;

	ORBit_small_invoke_async
		(bag, get_set_value_imethod (),
		 do_nothing_cb, NULL, args, NULL, my_ev);
	
	if (!opt_ev)
		CORBA_exception_free (&ev);
}

/**
 * eel_glade_get_file:
 * @filename: the XML file name.
 * @root: the widget node in @fname to start building from (or %NULL)
 * @domain: the translation domain for the XML file (or %NULL for default)
 * @first_required_widget: the name of the first widget we require
 * @: NULL terminated list of name, GtkWidget ** pairs.
 * 
 * Loads and parses the glade file, returns widget pointers for the names,
 * ensures that all the names are found.
 * 
 * Return value: the XML file, or NULL.
 **/
GladeXML *
eel_glade_get_file (const char *filename,
		    const char *root,
		    const char *domain,
		    const char *first_required_widget, ...)
{
	va_list     args;
	GladeXML   *gui;
	const char *name;
	GtkWidget **widget_ptr;
	GList      *ptrs, *l;

	va_start (args, first_required_widget);

	if (!(gui = glade_xml_new (filename, root, domain))) {
		g_warning ("Couldn't find necessary glade file '%s'", filename);
		va_end (args);
		return NULL;
	}

	ptrs = NULL;
	for (name = first_required_widget; name; name = va_arg (args, char *)) {
		widget_ptr = va_arg (args, void *);
		
		*widget_ptr = glade_xml_get_widget (gui, name);

		if (!*widget_ptr) {
			g_warning ("Glade file '%s' is missing widget '%s', aborting",
				   filename, name);
			
			for (l = ptrs; l; l = l->next) {
				*((gpointer *)l->data) = NULL;
			}
			g_list_free (ptrs);
			g_object_unref (gui);
			return NULL;
		} else {
			ptrs = g_list_prepend (ptrs, widget_ptr);
		}
	}

	va_end (args);

	return gui;
}

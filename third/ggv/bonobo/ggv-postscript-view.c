/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
/**
 * ggv-postscript-view.c
 *
 * Author:  Jaka Mocnik  <jaka@gnu.org>
 *
 * Copyright (c) 2001, 2002 Free Software Foundation
 */

#include <config.h>
#include <stdio.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkmarshal.h>
#include <gtk/gtktypeutils.h>
#include <gconf/gconf-client.h>

#include <gnome.h>

#include <ggv-postscript-view.h>
#include <ggv-control.h>

#include <gtkgs.h>
#include <ps.h>
#include <gsdefaults.h>
#include <ggvutils.h>
#include <ggv-msg-window.h>
#include <ggv-file-sel.h>
#include <ggv-prefs.h>
#include <ggv-prefs-ui.h>
#include <ggv-sidebar.h>
#include <cursors.h>

#include <bonobo.h>

#include <libgnomevfs/gnome-vfs.h>

struct _GgvPostScriptViewPrivate {
	GtkWidget *gs;

	GNOME_GGV_Size def_size;
	GNOME_GGV_Orientation def_orientation;

	gint magstep;
	gboolean pan;
	gdouble prev_x, prev_y;
	gchar *tmp_name, *uri, *save_path;
	guint gconf_notify_id;

	GgvMsgWindow *msg_win;
	GgvSidebar *sidebar;

	BonoboPropertyBag     *property_bag;
	BonoboPropertyControl *property_control;

	BonoboUIComponent     *uic, *popup_uic;

	gboolean               zoom_fit;

	gboolean pane_auto_jump, page_flip;
};

struct _GgvPostScriptViewClassPrivate {
	int dummy;
};

enum {
	PROP_ANTIALIASING,
	PROP_DEFAULT_ORIENTATION,
	PROP_OVERRIDE_ORIENTATION,
	PROP_DEFAULT_SIZE,
	PROP_OVERRIDE_SIZE,
	PROP_RESPECT_EOF,
	PROP_WATCH_FILE,
	PROP_WIDTH,
	PROP_HEIGHT,
	PROP_ORIENTATION,
	PROP_PAGE,
	PROP_PAGE_COUNT,
	PROP_PAGE_NAMES,
	PROP_TITLE,
	PROP_STATUS
};

enum {
	PROP_CONTROL_TITLE,
};

static GdkCursor *pan_cursor = NULL;

static BonoboObjectClass *ggv_postscript_view_parent_class;

static const gchar *orientation_paths[] = {
	"/commands/OrientationDocument",
	"/commands/OrientationPortrait",
	"/commands/OrientationLandscape",
	"/commands/OrientationUpsideDown",
	"/commands/OrientationSeascape",
	NULL
};

static const gchar *size_paths[] = {
	"/commands/PaperSizeDoc",
	"/commands/PaperSizeBBox",
	"/commands/PaperSizeLetter",
	"/commands/PaperSizeTabloid",
	"/commands/PaperSizeLedger",
	"/commands/PaperSizeLegal",
	"/commands/PaperSizeStatement",
	"/commands/PaperSizeExecutive",
	"/commands/PaperSizeA0",
	"/commands/PaperSizeA1",
	"/commands/PaperSizeA2",
	"/commands/PaperSizeA3",
	"/commands/PaperSizeA4",
	"/commands/PaperSizeA5",
	"/commands/PaperSizeB4",
	"/commands/PaperSizeB5",
	"/commands/PaperSizeFolio",
	"/commands/PaperSizeQuarto",
	"/commands/PaperSize10x14",
	NULL
};

void
get_status(GgvPostScriptView *ps_view, BonoboArg *arg)
{
	const gchar *status;

	g_assert(arg->_type == TC_CORBA_string);
	
	status = GTK_GS(ps_view->priv->gs)->gs_status;;

	*(CORBA_string *)arg->_value = CORBA_string_dup(status);
}

static void
get_title(GgvPostScriptView *ps_view, BonoboArg *arg)
{
	GtkGS *gs = GTK_GS(ps_view->priv->gs);
	gchar *title;

	g_assert(arg->_type == TC_CORBA_string);
	
	if(!gs->loaded)
		title = "No document loaded.";
	else if(!strcmp(gs->gs_filename, "-"))
		title = "(stdin)";
	else
		title = gs->gs_filename;

	*(CORBA_string *)arg->_value = CORBA_string_dup(title);
}

static void
set_file_items_sensitivity(GgvPostScriptView *ps_view, gboolean sens)
{
	gchar *prop_val;
	Bonobo_UIContainer container;

	container = bonobo_ui_component_get_container (ps_view->priv->uic);
	if (container == CORBA_OBJECT_NIL)
		return;

	prop_val = sens?"1":"0";

	bonobo_ui_component_freeze(ps_view->priv->uic, NULL);
	bonobo_ui_component_set_prop(ps_view->priv->uic, "/commands/FileSaveMarked",
								 "sensitive", prop_val, NULL);
	bonobo_ui_component_set_prop(ps_view->priv->uic, "/commands/FilePrintMarked",
								 "sensitive", prop_val, NULL);
	bonobo_ui_component_set_prop(ps_view->priv->uic, "/commands/FilePrintAll",
								 "sensitive", prop_val, NULL);
	bonobo_ui_component_thaw(ps_view->priv->uic, NULL);
}

static void
_set_page_items_sensitivity(GgvPostScriptView *ps_view, BonoboUIComponent *uic)
{
	GtkGS *gs = GTK_GS(ps_view->priv->gs);
	gchar *prop_val;
	Bonobo_UIContainer container;

	container = bonobo_ui_component_get_container (uic);
	if (container == CORBA_OBJECT_NIL)
		return;

	bonobo_ui_component_freeze(uic, NULL);
	prop_val = (gtk_gs_get_current_page(gs) >= gtk_gs_get_page_count(gs) - 1 ||
				gtk_gs_get_current_page(gs) < 0)?
		"0":"1";
	bonobo_ui_component_set_prop(uic, "/commands/NextPage",
								 "sensitive", prop_val, NULL);
	prop_val = (gtk_gs_get_current_page(gs) <= 0 || !gs->structured_doc)?
		"0":"1";
	bonobo_ui_component_set_prop(uic, "/commands/PrevPage",
								 "sensitive", prop_val, NULL);
	bonobo_ui_component_set_prop(uic, "/commands/FirstPage",
								 "sensitive", prop_val, NULL);
	prop_val = (gtk_gs_get_current_page(gs) >= gtk_gs_get_page_count(gs) - 1 ||
				!gs->structured_doc || gtk_gs_get_current_page(gs) < 0)?
		"0":"1";
	bonobo_ui_component_set_prop(uic, "/commands/LastPage",
								 "sensitive", prop_val, NULL);
	bonobo_ui_component_thaw(uic, NULL);
}

static void
set_page_items_sensitivity(GgvPostScriptView *ps_view)
{
	_set_page_items_sensitivity(ps_view, ps_view->priv->uic);
	if(ps_view->priv->popup_uic != NULL)
		_set_page_items_sensitivity(ps_view, ps_view->priv->popup_uic);
}

static void
notify_page_count_change(GgvPostScriptView *ps_view)
{
	BonoboArg *arg;

	set_page_items_sensitivity(ps_view);
	set_file_items_sensitivity(ps_view, ggv_postscript_view_get_page_count(ps_view) > 0);
	if(ps_view->priv->sidebar)
		ggv_sidebar_create_page_list(ps_view->priv->sidebar);

	arg = bonobo_arg_new(TC_CORBA_long);
	BONOBO_ARG_SET_LONG(arg, ggv_postscript_view_get_page_count(ps_view));
	bonobo_event_source_notify_listeners(ps_view->priv->property_bag->es,
										 "Bonobo/Property:change:page_count",
										 arg, NULL);

	bonobo_arg_release(arg);
}

static void
notify_orientation_change(GgvPostScriptView *ps_view)
{
	BonoboArg *arg;

	arg = bonobo_arg_new(TC_GNOME_GGV_Orientation);
	*(GNOME_GGV_Orientation *)arg->_value = gtk_gs_get_orientation(GTK_GS(ps_view->priv->gs));
	bonobo_event_source_notify_listeners(ps_view->priv->property_bag->es,
										 "Bonobo/Property:change:orientation",
										 arg, NULL);
	bonobo_arg_release(arg);
}

static void
notify_page_change(GgvPostScriptView *ps_view)
{
	BonoboArg *arg;

	set_page_items_sensitivity(ps_view);
	ggv_sidebar_page_changed(ps_view->priv->sidebar,
							 gtk_gs_get_current_page(GTK_GS(ps_view->priv->gs)));

	arg = bonobo_arg_new(TC_CORBA_long);
	*(CORBA_long *)arg->_value = gtk_gs_get_current_page(GTK_GS(ps_view->priv->gs));
	bonobo_event_source_notify_listeners(ps_view->priv->property_bag->es,
										 "Bonobo/Property:change:page",
										 arg, NULL);
	bonobo_arg_release(arg);
}

static void
notify_title_change(GgvPostScriptView *ps_view)
{
	BonoboArg *arg;
	arg = bonobo_arg_new(TC_CORBA_string);
	get_title(ps_view, arg);
	bonobo_event_source_notify_listeners(ps_view->priv->property_bag->es,
										 "Bonobo/Property:change:title",
										 arg, NULL);
	bonobo_arg_release(arg);
}

static void
notify_status_change(GgvPostScriptView *ps_view)
{
	BonoboArg *arg;
	arg = bonobo_arg_new(TC_CORBA_string);
	get_status(ps_view, arg);
	bonobo_event_source_notify_listeners(ps_view->priv->property_bag->es,
										 "Bonobo/Property:change:status",
										 arg, NULL);
	bonobo_arg_release(arg);
}

void
ggv_postscript_view_goto_page(GgvPostScriptView *ps_view, gint page)
{
	gint old_page = gtk_gs_get_current_page(GTK_GS(ps_view->priv->gs));

	gtk_gs_goto_page(GTK_GS(ps_view->priv->gs), page);
	if(gtk_gs_get_current_page(GTK_GS(ps_view->priv->gs)) != old_page) {
		notify_page_change(ps_view);
		if(ps_view->priv->pane_auto_jump) {
			gtk_gs_scroll_to_edge(GTK_GS(ps_view->priv->gs),
								  GTK_POS_TOP,  GTK_POS_TOP);
		}
	}
}

gint
ggv_postscript_view_get_current_page(GgvPostScriptView *ps_view)
{
	return gtk_gs_get_current_page(GTK_GS(ps_view->priv->gs));
}

gint
ggv_postscript_view_get_page_count(GgvPostScriptView *ps_view)
{
	return gtk_gs_get_page_count(GTK_GS(ps_view->priv->gs));
}

static void
load_ps(GgvPostScriptView *ps_view, const gchar *fname)
{
	gtk_gs_load(GTK_GS(ps_view->priv->gs), fname);
	notify_title_change(ps_view);
	notify_page_count_change(ps_view);
	notify_orientation_change(ps_view);
	ggv_postscript_view_goto_page(ps_view, 0);
	notify_page_change(ps_view);
	ggv_sidebar_update_coordinates(ps_view->priv->sidebar, 0.0, 0.0);
	set_file_items_sensitivity(ps_view, ggv_postscript_view_get_page_count(ps_view) > 0);
	notify_status_change(ps_view);
}

static void
ps_view_clean_tmp_file(GgvPostScriptView *ps_view)
{
	/* copy stream to a tmp file */
	if(ps_view->priv->tmp_name != NULL) {
		unlink(ps_view->priv->tmp_name);
		g_free(ps_view->priv->tmp_name);
		ps_view->priv->tmp_name = NULL;
	}
}

static FILE *
ps_view_get_tmp_file(GgvPostScriptView *ps_view)
{
	int fd;
	FILE *tmpfile;

	ps_view_clean_tmp_file(ps_view);
	ps_view->priv->tmp_name = g_strconcat(g_get_tmp_dir(), "/ggvXXXXXX", NULL);
	if((fd = mkstemp(ps_view->priv->tmp_name)) < 0) {
		g_free(ps_view->priv->tmp_name),
		ps_view->priv->tmp_name = NULL;
		return;
	}
	tmpfile = fdopen(fd, "w");
	if(!tmpfile) {
		close(fd);
		return NULL;
	}
	return tmpfile;
}

/*
 * Loads an postscript document from a Bonobo_Stream
 */
static void
load_ps_from_stream (BonoboPersistStream *ps,
					 Bonobo_Stream stream,
					 Bonobo_Persist_ContentType type,
					 void *data,
					 CORBA_Environment *ev)
{
	GgvPostScriptView *ps_view;
	Bonobo_Stream_iobuf *buffer;
	CORBA_long len_read;
	FILE *tmpfile;
	int fd;

	g_return_if_fail (data != NULL);
	g_return_if_fail (GGV_IS_POSTSCRIPT_VIEW (data));

	ps_view = GGV_POSTSCRIPT_VIEW(data);

	ps_view_clean_tmp_file(ps_view);
	tmpfile = ps_view_get_tmp_file(ps_view);

	do {
		Bonobo_Stream_read (stream, 32768, &buffer, ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			goto exit_clean;

		len_read = buffer->_length;

		if (buffer->_buffer && len_read)
			if(fwrite(buffer->_buffer, 1, len_read, tmpfile) != len_read) {
				CORBA_free (buffer);
				goto exit_clean;
			}

		CORBA_free (buffer);
	} while (len_read > 0);

	fclose(tmpfile);
	load_ps(ps_view, ps_view->priv->tmp_name);
	return;

 exit_clean:
	fclose (tmpfile);
	return;
}

/* 
 * handlers for mouse actions
 */
static gboolean
view_button_press_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GgvPostScriptView *ps_view;

	g_return_val_if_fail(data != NULL, FALSE);
	g_return_val_if_fail(GGV_IS_POSTSCRIPT_VIEW(data), FALSE);

	ps_view = GGV_POSTSCRIPT_VIEW(data);

	if(event->button == 1 && !ps_view->priv->pan) {
		gint wx = 0, wy = 0;
			
		gtk_widget_grab_focus(ps_view->priv->gs);

		gdk_window_get_pointer(widget->window, &wx, &wy, NULL);
			
		gtk_gs_start_scroll(GTK_GS(ps_view->priv->gs));

		ps_view->priv->pan = TRUE;
		if(pan_cursor == NULL)
			pan_cursor = cursor_get(widget->window, CURSOR_HAND_CLOSED);
			
		gtk_grab_add(widget);
		gdk_pointer_grab(widget->window, FALSE,
						 GDK_POINTER_MOTION_MASK |
						 GDK_BUTTON_RELEASE_MASK, NULL,
						 pan_cursor, GDK_CURRENT_TIME);
		ps_view->priv->prev_x = wx;
		ps_view->priv->prev_y = wy;

		return TRUE;
	}

	return FALSE;
}

static gboolean
view_button_release_cb(GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	GgvPostScriptView *ps_view;

	g_return_val_if_fail(data != NULL, FALSE);
	g_return_val_if_fail(GGV_IS_POSTSCRIPT_VIEW(data), FALSE);

	ps_view = GGV_POSTSCRIPT_VIEW(data);

	if(event->button == 1 && ps_view->priv->pan) {
		ps_view->priv->pan = FALSE;
		gdk_pointer_ungrab(GDK_CURRENT_TIME);
		gtk_grab_remove(widget);
		gtk_gs_end_scroll(GTK_GS(ps_view->priv->gs));

		return TRUE;
	}

	return FALSE;
}

static gboolean
view_motion_cb(GtkWidget *widget, GdkEventMotion *event, gpointer data)
{
	GgvPostScriptView *ps_view;

	g_return_val_if_fail(data != NULL, FALSE);
	g_return_val_if_fail(GGV_IS_POSTSCRIPT_VIEW(data), FALSE);

	ps_view = GGV_POSTSCRIPT_VIEW(data);
	if(ps_view->priv->pan) {
		gtk_gs_scroll(GTK_GS(ps_view->priv->gs),
					  -(gint)event->x + ps_view->priv->prev_x,
					  -(gint)event->y + ps_view->priv->prev_y);
		ps_view->priv->prev_x = (gint)event->x;
		ps_view->priv->prev_y = (gint)event->y;

		return TRUE;
	}
	else {
		GtkGS *gs;

		gs = GTK_GS(ps_view->priv->gs);
		if(event->window == gs->pstarget && gs->doc != NULL) {
			gfloat xcoord, ycoord;
		
			xcoord = event->x/gs->xdpi/gs->zoom_factor;
			ycoord = event->y/gs->ydpi/gs->zoom_factor;
			
			ggv_sidebar_update_coordinates(ps_view->priv->sidebar, xcoord, ycoord);
		}
	}

	return FALSE;
}


/*
 * Loads an postscript document from a Bonobo_File
 */
static gint
load_ps_from_file (BonoboPersistFile *pf, const CORBA_char *filename,
				   CORBA_Environment *ev, void *data)
{
	GnomeVFSResult result;
	GnomeVFSHandle *handle;
	GnomeVFSURI     *uri;
	GnomeVFSFileSize bytes_read = 0;
	guchar *buffer;
	GgvPostScriptView *ps_view;
	gchar *local_file = NULL, *uri_str;
	FILE *tmpfile;

	g_return_val_if_fail (data != NULL, -1);
	g_return_val_if_fail (GGV_IS_POSTSCRIPT_VIEW (data), -1);

	ps_view = GGV_POSTSCRIPT_VIEW(data);

	ps_view_clean_tmp_file(ps_view);

	uri = gnome_vfs_uri_new(filename);
	if(gnome_vfs_uri_is_local(uri)) {
		uri_str = gnome_vfs_uri_to_string(uri, GNOME_VFS_URI_HIDE_NONE);
		local_file = gnome_vfs_get_local_path_from_uri(uri_str);
		g_free(uri_str);
	}
	else {
		/* open uri */
		result = gnome_vfs_open_uri(&handle, uri, GNOME_VFS_OPEN_READ);
		gnome_vfs_uri_unref(uri);
		if(result != GNOME_VFS_OK) {
			return -1;
		}

		tmpfile = ps_view_get_tmp_file(ps_view);

		buffer = g_new0(guchar, 4096);
		do {
			result = gnome_vfs_read (handle, buffer,
									 4096, &bytes_read);
			if (result != GNOME_VFS_OK)
				break;
			
			if (bytes_read > 0)
				if(fwrite(buffer, 1, bytes_read, tmpfile) != bytes_read) {
					result = GNOME_VFS_ERROR_GENERIC;
					break;
				}
		} while(TRUE);

		gnome_vfs_close(handle);
		fclose(tmpfile);

		if(result != GNOME_VFS_ERROR_EOF)
			return -1;
	}
	if(ps_view->priv->tmp_name) {
		load_ps(ps_view, ps_view->priv->tmp_name);
	}
	else if(local_file) {
		load_ps(ps_view, local_file);
		g_free(local_file);
	}

	return 0;
}

static void
listener_Orientation_cb (BonoboUIComponent *uic, const char *path,
						 Bonobo_UIComponent_EventType type, const char *state,
						 gpointer user_data)
{
	GgvPostScriptView *ps_view;
	GtkGSOrientation orientation;
	GtkGS *gs;

	g_return_if_fail(user_data != NULL);
	g_return_if_fail(GGV_IS_POSTSCRIPT_VIEW (user_data));

	if(type != Bonobo_UIComponent_STATE_CHANGED)
		return;

	if(!state || !atoi(state))
		return;

	ps_view = GGV_POSTSCRIPT_VIEW(user_data);
	gs = GTK_GS(ps_view->priv->gs);

	if(!strcmp(path, "OrientationDocument")) {
		if(gs->override_orientation == TRUE) {
			gtk_gs_set_override_orientation(GTK_GS(ps_view->priv->gs), FALSE);
			notify_orientation_change(ps_view);
		}
	}
	else {
		if(!strcmp(path, "OrientationPortrait"))
			orientation = GTK_GS_ORIENTATION_PORTRAIT;
		else if(!strcmp(path, "OrientationLandscape"))
			orientation = GTK_GS_ORIENTATION_LANDSCAPE;
		else if(!strcmp(path, "OrientationUpsideDown"))
			orientation = GTK_GS_ORIENTATION_UPSIDEDOWN;
		else if(!strcmp(path, "OrientationSeascape"))
			orientation = GTK_GS_ORIENTATION_SEASCAPE;
		else {
			g_warning("Unknown orientation `%s'", path);
			return;
		}
		gtk_gs_set_default_orientation(GTK_GS(ps_view->priv->gs), orientation);
		if(gs->override_orientation == FALSE) {
			gtk_gs_set_override_orientation(GTK_GS(ps_view->priv->gs), TRUE);
		}
		notify_orientation_change(ps_view);		
	}
}

static void
sync_orientation_items(GgvPostScriptView *ps_view)
{
	gint i;
	gboolean orient_state;
	
	bonobo_ui_component_freeze(ps_view->priv->uic, NULL);
	for(i = 0; orientation_paths[i] != NULL; i++) {
		if(i > 0)
			orient_state = ((TRUE == GTK_GS(ps_view->priv->gs)->override_orientation) &&
							(i - 1 == gtk_gs_get_default_orientation(GTK_GS(ps_view->priv->gs))));
		else
			orient_state = (FALSE == GTK_GS(ps_view->priv->gs)->override_orientation);
		bonobo_ui_component_set_prop(ps_view->priv->uic, orientation_paths[i],
									 "state", orient_state?"1":"0", NULL);
	}
	bonobo_ui_component_thaw(ps_view->priv->uic, NULL);
}

static void
sync_size_items(GgvPostScriptView *ps_view)
{
	gint i;
	gboolean size_state;
	const gchar **paths;

	paths = size_paths;

	bonobo_ui_component_freeze(ps_view->priv->uic, NULL);
	for(i = 0; paths[i] != NULL; i++) {
		if(i > 0)
			size_state = ((TRUE == GTK_GS(ps_view->priv->gs)->override_size) &&
						  (i - 1 == gtk_gs_get_default_size(GTK_GS(ps_view->priv->gs))));
		else
			size_state = (FALSE == GTK_GS(ps_view->priv->gs)->override_size);
		bonobo_ui_component_set_prop(ps_view->priv->uic, paths[i],
									 "state", size_state?"1":"0", NULL);
	}
	bonobo_ui_component_thaw(ps_view->priv->uic, NULL);
}

static void
listener_Size_cb(BonoboUIComponent *uic, const char *path,
				  Bonobo_UIComponent_EventType type, const char *state,
				  gpointer user_data)
{
	GgvPostScriptView *ps_view;
	gint size;
	GtkGS *gs;

	g_return_if_fail(user_data != NULL);
	g_return_if_fail(GGV_IS_POSTSCRIPT_VIEW(user_data));

	if(type != Bonobo_UIComponent_STATE_CHANGED)
		return;

	if(!state || !atoi(state))
		return;

	ps_view = GGV_POSTSCRIPT_VIEW(user_data);
	gs = GTK_GS(ps_view->priv->gs);

	if(!strcmp(path, "PaperSizeDoc")) {
		if(gs->override_size == TRUE) {
			gtk_gs_set_override_size(GTK_GS(ps_view->priv->gs), FALSE);
			sync_size_items(ps_view);
		}
	}
	else {
		size = gtk_gs_get_size_index(path + strlen("PaperSize"),
									 gtk_gs_defaults_get_paper_sizes());
		gtk_gs_set_default_size(GTK_GS(ps_view->priv->gs), size);
		if(gs->override_size == FALSE) {
			gtk_gs_set_override_size(GTK_GS(ps_view->priv->gs), TRUE);
		}
		sync_size_items(ps_view);
	}
}


static gchar *
ggv_postscript_view_get_ps(GgvPostScriptView *ps_view, gint *active_rows)
{
	gchar *ps;

	if(!active_rows || active_rows[0] == -1)
		ps = gtk_gs_get_postscript(GTK_GS(ps_view->priv->gs), NULL);
	else {
		gint num, i;
		GtkGS *gs;
		gint *page_mask;

		gs = GTK_GS(ps_view->priv->gs);
		num = gtk_gs_get_page_count(gs);
		page_mask = g_new0(gint, num);

		for(i = 0; active_rows[i] != -1; i++) {
			if(active_rows[i] < num)
				page_mask[active_rows[i]] = TRUE;
		}
		ps = gtk_gs_get_postscript(gs, page_mask);
	}
	return ps;
}

static void
ggv_postscript_view_print(GgvPostScriptView *ps_view, gchar *ps)
{
	gchar **argv;
	gint print_in;

	argv = g_strsplit(ggv_print_cmd, " ", 0);
	if(!g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_SEARCH_PATH,
								 NULL, NULL, NULL, &print_in, NULL, NULL,
								 NULL)) {
		GtkWidget *dlg;
		dlg = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(ps_view->priv->gs)),
									 GTK_DIALOG_MODAL,
									 GTK_MESSAGE_ERROR,
									 GTK_BUTTONS_OK,
									 _("Unable to execute print command:\n%s"),
									 ggv_print_cmd);
		gtk_widget_show(dlg);
		gtk_dialog_run(GTK_DIALOG(dlg));
		gtk_widget_destroy(dlg);
	}
	else {
		write(print_in, ps, strlen(ps));
		close(print_in);
	}
	g_strfreev(argv);
}

static gboolean
ps_view_save_doc(GgvPostScriptView *ps_view, const gchar *fname,
				 const gchar *doc, gint len)
{
	GnomeVFSResult result;
	GnomeVFSHandle *handle;
	GnomeVFSURI *uri;
	GnomeVFSFileSize written;

	uri = gnome_vfs_uri_new (fname);
	result = gnome_vfs_create_uri (&handle, uri,
								   GNOME_VFS_OPEN_WRITE, FALSE, 0644);
	gnome_vfs_uri_unref(uri);
	if (result != GNOME_VFS_OK) {
		return FALSE;
	}

	result = gnome_vfs_write(handle, doc, len, &written);
	gnome_vfs_close(handle);

	if(result != GNOME_VFS_OK || written != len)
		return FALSE;
	else
		return TRUE;
}

static void
verb_FileSaveMarked(BonoboUIComponent *uic, gpointer data, const char *cname)
{
	GgvPostScriptView *ps_view = GGV_POSTSCRIPT_VIEW(data);
	gint *active_rows;
	gchar *doc, *fname, *local_path = NULL;

	active_rows = ggv_sidebar_get_active_list(ps_view->priv->sidebar);
	if(active_rows[0] == -1) {
		GtkWidget *dlg;
		
		dlg = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(ps_view->priv->gs)),
									 GTK_DIALOG_MODAL,
									 GTK_MESSAGE_QUESTION,
									 GTK_BUTTONS_YES_NO,
									 _("No pages have been marked.\n"
									   "Do you want to save the whole document?"));
		gtk_widget_show(dlg);
		switch(gtk_dialog_run(GTK_DIALOG(dlg))) {
		case GTK_RESPONSE_NO:
		case GTK_RESPONSE_DELETE_EVENT:
			gtk_widget_destroy(dlg);
			return;
		default:
			break;
		}
		gtk_widget_destroy(dlg);
	}
	g_free(active_rows);
	
	active_rows = ggv_sidebar_get_active_list(ps_view->priv->sidebar);
	doc = ggv_postscript_view_get_ps(ps_view, active_rows);
	g_free(active_rows);

	if(doc != NULL) {
		fname = ggv_file_sel_request_uri(_("Select a file to save pages as"),
										 ps_view->priv->save_path);
		if(fname) {
			if((local_path = gnome_vfs_get_local_path_from_uri(fname)) &&
			   ggv_file_readable(local_path)) {
				GtkWidget *dlg;
		
				g_free(local_path);
				dlg = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(ps_view->priv->gs)),
											 GTK_DIALOG_MODAL,
											 GTK_MESSAGE_QUESTION,
											 GTK_BUTTONS_YES_NO,
											 _("A file with this name already exists.\n"
											   "Do you want to overwrite it?"));
				gtk_widget_show(dlg);
				switch(gtk_dialog_run(GTK_DIALOG(dlg))) {
				case GTK_RESPONSE_NO:
				case GTK_RESPONSE_DELETE_EVENT:
					gtk_widget_destroy(dlg);
					g_free(fname);
					g_free(doc);
					return;
				default:
					break;
				}
				gtk_widget_destroy(dlg);
			}
			ps_view_save_doc(ps_view, fname, doc, strlen(doc));
			if(ps_view->priv->save_path)
				g_free(ps_view->priv->save_path);
			ps_view->priv->save_path = fname;
		}
		g_free(doc);
	}
}

static void
verb_FilePrintMarked(BonoboUIComponent *uic, gpointer data, const char *cname)
{
	GgvPostScriptView *ps_view = GGV_POSTSCRIPT_VIEW(data);
	gchar *ps;
	gint *active_rows;
	
	active_rows = ggv_sidebar_get_active_list(ps_view->priv->sidebar);
	if(active_rows[0] == -1) {
		GtkWidget *dlg;
		
		dlg = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(ps_view->priv->gs)),
									 GTK_DIALOG_MODAL,
									 GTK_MESSAGE_QUESTION,
									 GTK_BUTTONS_YES_NO,
									 _("No pages have been marked.\n"
									   "Do you want to print the whole document?"));
		gtk_widget_show(dlg);
		switch(gtk_dialog_run(GTK_DIALOG(dlg))) {
		case GTK_RESPONSE_NO:
		case GTK_RESPONSE_DELETE_EVENT:
			gtk_widget_destroy(dlg);
			return;
		default:
			break;
		}
		gtk_widget_destroy(dlg);
	}
	if((ps = ggv_postscript_view_get_ps(ps_view, active_rows)) != NULL) {
		ggv_postscript_view_print(ps_view, ps);
		g_free(ps);
	}
	g_free(active_rows);
}

static void
verb_FilePrintAll(BonoboUIComponent *uic, gpointer data, const char *cname)
{
	GgvPostScriptView *ps_view = GGV_POSTSCRIPT_VIEW(data);
	gchar *ps;

	if((ps = ggv_postscript_view_get_ps(ps_view, NULL)) != NULL) {
		ggv_postscript_view_print(ps_view, ps);
		g_free(ps);
	}
}

static void
verb_NextPage_cb(BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	GgvPostScriptView *ps_view;
	GtkGS *gs;

	g_return_if_fail (user_data != NULL);
	g_return_if_fail (GGV_IS_POSTSCRIPT_VIEW (user_data));

	ps_view = GGV_POSTSCRIPT_VIEW (user_data);
	gs = GTK_GS(ps_view->priv->gs);

	ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) + 1);
}

static void
verb_PrevPage_cb(BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	GgvPostScriptView *ps_view;

	g_return_if_fail (user_data != NULL);
	g_return_if_fail (GGV_IS_POSTSCRIPT_VIEW (user_data));

	ps_view = GGV_POSTSCRIPT_VIEW (user_data);

	ggv_postscript_view_goto_page(ps_view,
								  GTK_GS(ps_view->priv->gs)->current_page - 1);

}

static void
verb_LastPage_cb(BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	GgvPostScriptView *ps_view;
	GtkGS *gs;

	g_return_if_fail (user_data != NULL);
	g_return_if_fail (GGV_IS_POSTSCRIPT_VIEW (user_data));

	ps_view = GGV_POSTSCRIPT_VIEW (user_data);
	gs = GTK_GS(ps_view->priv->gs);

	ggv_postscript_view_goto_page(ps_view, gtk_gs_get_page_count(gs) - 1);
}

static void
verb_FirstPage_cb(BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	GgvPostScriptView *ps_view;
	GtkGS *gs;

	g_return_if_fail (user_data != NULL);
	g_return_if_fail (GGV_IS_POSTSCRIPT_VIEW (user_data));

	ps_view = GGV_POSTSCRIPT_VIEW (user_data);
	gs = GTK_GS(ps_view->priv->gs);

	ggv_postscript_view_goto_page(ps_view, 0);
}

static void
verb_SettingsPreferences(BonoboUIComponent *uic, gpointer data, const char *cname)
{
        static GtkWidget *dlg = NULL;

        if(dlg == NULL) {
                dlg = ggv_prefs_dialog_new();
        }
        ggv_prefs_dialog_show(GGV_PREFS_DIALOG(dlg));
		ggv_raise_and_focus_widget(dlg);
}

BonoboUIVerb ggv_postscript_view_verbs[] = {
	BONOBO_UI_VERB("SettingsPreferences", verb_SettingsPreferences),
	BONOBO_UI_VERB("FileSaveMarked", verb_FileSaveMarked),
	BONOBO_UI_VERB("FilePrintMarked", verb_FilePrintMarked),
	BONOBO_UI_VERB("FilePrintAll", verb_FilePrintAll),
	BONOBO_UI_VERB("NextPage", verb_NextPage_cb),
	BONOBO_UI_VERB("PrevPage", verb_PrevPage_cb),
	BONOBO_UI_VERB("LastPage", verb_LastPage_cb),
	BONOBO_UI_VERB("FirstPage", verb_FirstPage_cb),
	BONOBO_UI_VERB_END
};

static void
ggv_postscript_view_create_ui(GgvPostScriptView *ps_view)
{
	gint i;

	g_return_if_fail(ps_view != NULL);
	g_return_if_fail(GGV_IS_POSTSCRIPT_VIEW(ps_view));

	bonobo_ui_component_freeze(ps_view->priv->uic, NULL);
	/* Set up the UI from an XML file. */
	bonobo_ui_util_set_ui(ps_view->priv->uic, DATADIR,
						  "ggv-postscript-view-ui.xml", 
						  "GGV", NULL);
	bonobo_ui_component_set_prop(ps_view->priv->uic, "/Sidebar",
								 "placement", ggv_right_panel?"right":"left", NULL);
	bonobo_ui_component_set_prop(ps_view->priv->uic, "/Sidebar",
								 "hidden", ggv_panel?"0":"1", NULL);
	bonobo_ui_component_add_verb_list_with_data(ps_view->priv->uic,
												ggv_postscript_view_verbs,
												ps_view);
	bonobo_ui_component_thaw(ps_view->priv->uic, NULL);
#if 1
	bonobo_ui_component_object_set(ps_view->priv->uic, "/Sidebar/GgvSidebar",
								   BONOBO_OBJREF(ps_view->priv->sidebar), NULL);
#endif

	for(i = 0; size_paths[i]; i++) 
		bonobo_ui_component_add_listener(ps_view->priv->uic,
										 size_paths[i] + strlen("/commands/"),
										 listener_Size_cb, ps_view);

	for(i = 0; orientation_paths[i]; i++) 
		bonobo_ui_component_add_listener(ps_view->priv->uic,
										 orientation_paths[i] + strlen("/commands/"),
										 listener_Orientation_cb, ps_view);
	set_file_items_sensitivity(ps_view,
							   ggv_postscript_view_get_page_count(ps_view) > 0);
	set_page_items_sensitivity(ps_view);
	sync_orientation_items(ps_view);
	sync_size_items(ps_view);
}

gchar **
ggv_postscript_view_get_page_names(GgvPostScriptView *ps_view)
{
	gchar **names = NULL;
	GtkGS *gs;
	gint i;

	gs = GTK_GS(ps_view->priv->gs);
	if(gs->loaded && gs->doc && gs->structured_doc) {
		names = g_new0(gchar *, gs->doc->numpages + 1);
		for(i = 0; i < gs->doc->numpages; i++) {
			names[i] = g_strdup(gs->doc->pages[i].label);
		}
	}
	return names;
}

static void
ggv_postscript_view_get_prop(BonoboPropertyBag *bag,
							 BonoboArg         *arg,
							 guint              arg_id,
							 CORBA_Environment *ev,
							 gpointer           user_data)
{
	GgvPostScriptView *ps_view;
	gchar *size;

	g_return_if_fail(user_data != NULL);
	g_return_if_fail(GGV_IS_POSTSCRIPT_VIEW(user_data));

	ps_view = GGV_POSTSCRIPT_VIEW(user_data);

	switch(arg_id) {
	case PROP_PAGE: {
		g_assert(arg->_type == TC_CORBA_long);

		*(CORBA_long *)arg->_value = gtk_gs_get_current_page(GTK_GS(ps_view->priv->gs));
		break;
	}
	case PROP_PAGE_COUNT: {
		g_assert(arg->_type == TC_CORBA_long);

		*(CORBA_long *)arg->_value = gtk_gs_get_page_count(GTK_GS(ps_view->priv->gs));
		break;
	}
	case PROP_PAGE_NAMES: {
		GNOME_GGV_PageNameList *names;
		GtkGS *gs;
		int i;

		g_assert(arg->_type == TC_GNOME_GGV_PageNameList);

		names = GNOME_GGV_PageNameList__alloc();
		names->_length = 0;
		names->_buffer = NULL;
		gs = GTK_GS(ps_view->priv->gs);
		if(gs->loaded && gs->doc && gs->structured_doc) {
			names->_length = gs->doc->numpages;
			names->_buffer = CORBA_sequence_CORBA_string_allocbuf(names->_length);
			for(i = 0; i < gs->doc->numpages; i++) {
				names->_buffer[i] = CORBA_string_dup(gs->doc->pages[i].label);
			}
		}
		CORBA_sequence_set_release(names, CORBA_TRUE);
		arg->_value = names;
		break;
	}
	case PROP_TITLE: {
		g_assert(arg->_type == TC_CORBA_string);
		get_title(ps_view, arg);
		break;
	}
	case PROP_STATUS: {
		g_assert(arg->_type == TC_CORBA_string);
		get_status(ps_view, arg);
		break;
	}
	case PROP_ORIENTATION: {
		GNOME_GGV_Orientation orient;

		g_assert(arg->_type == TC_GNOME_GGV_Orientation);

		switch(gtk_gs_get_orientation(GTK_GS(ps_view->priv->gs))) {
		case GTK_GS_ORIENTATION_PORTRAIT:
			orient = GNOME_GGV_ORIENTATION_PORTRAIT;
			break;
		case GTK_GS_ORIENTATION_LANDSCAPE:
			orient = GNOME_GGV_ORIENTATION_LANDSCAPE;
			break;
		case GTK_GS_ORIENTATION_UPSIDEDOWN:
			orient = GNOME_GGV_ORIENTATION_UPSIDEDOWN;
			break;
		case GTK_GS_ORIENTATION_SEASCAPE:
			orient = GNOME_GGV_ORIENTATION_SEASCAPE;
			break;
		default:
			orient = GNOME_GGV_ORIENTATION_PORTRAIT;
			break;
		}
		*(GNOME_GGV_Orientation *)arg->_value = orient;
		break;
	}
	case PROP_WIDTH: {
		CORBA_float w;
		GtkGSOrientation orient;
		GtkGS *gs = GTK_GS(ps_view->priv->gs);

		g_assert(arg->_type == TC_CORBA_float);

        orient = gtk_gs_get_orientation(gs);
		switch(orient) {
		case GTK_GS_ORIENTATION_PORTRAIT:
        case GTK_GS_ORIENTATION_UPSIDEDOWN:
			w = gs->urx - gs->llx;
			break;
        case GTK_GS_ORIENTATION_LANDSCAPE:
        case GTK_GS_ORIENTATION_SEASCAPE:
			w = gs->ury - gs->lly;
			break;
		default:
			w = 0;
			break;
		}
		w = MAX(w, 0);
		*(CORBA_long *)arg->_value = w;
		break;		
	}
	case PROP_HEIGHT: {
		CORBA_float h;
		GtkGSOrientation orient;
		GtkGS *gs = GTK_GS(ps_view->priv->gs);

		g_assert(arg->_type == TC_CORBA_float);

        orient = gtk_gs_get_orientation(gs);
		switch(orient) {
		case GTK_GS_ORIENTATION_PORTRAIT:
        case GTK_GS_ORIENTATION_UPSIDEDOWN:
			h = gs->ury - gs->lly;
			break;
        case GTK_GS_ORIENTATION_LANDSCAPE:
        case GTK_GS_ORIENTATION_SEASCAPE:
			h = gs->urx - gs->llx;
			break;
		default:
			h = 0;
		}
		h = MAX(h, 0);
		*(CORBA_long *)arg->_value = h;
		break;
	}
	case PROP_DEFAULT_ORIENTATION: {
		g_assert(arg->_type == TC_GNOME_GGV_Orientation);

		*(GNOME_GGV_Orientation *)arg->_value = ps_view->priv->def_orientation;
		break;
	}
	case PROP_OVERRIDE_ORIENTATION: {
		g_assert(arg->_type == TC_CORBA_boolean);

		*(CORBA_boolean *)arg->_value =
			gtk_gs_get_override_orientation(GTK_GS(ps_view->priv->gs))?
			CORBA_TRUE:CORBA_FALSE;
		break;
	}
	case PROP_DEFAULT_SIZE: {
		g_assert(arg->_type == TC_GNOME_GGV_Size);

		size = gtk_gs_defaults_get_paper_sizes()
			[gtk_gs_get_default_size(GTK_GS(ps_view->priv->gs))].name;

		*(GNOME_GGV_Size *)arg->_value =
			CORBA_string_dup(size);
		break;
	}
	case PROP_OVERRIDE_SIZE: {
		g_assert(arg->_type == TC_CORBA_boolean);

		*(CORBA_boolean *)arg->_value =
			gtk_gs_get_override_size(GTK_GS(ps_view->priv->gs))?
			CORBA_TRUE:CORBA_FALSE;
		break;
	}
	case PROP_RESPECT_EOF: {
		g_assert(arg->_type == TC_CORBA_boolean);

		*(CORBA_boolean *)arg->_value =
			gtk_gs_get_respect_eof(GTK_GS(ps_view->priv->gs))?
			CORBA_TRUE:CORBA_FALSE;
		break;
	}
	case PROP_ANTIALIASING: {
		g_assert(arg->_type == TC_CORBA_boolean);

		*(CORBA_boolean *)arg->_value =
			gtk_gs_get_antialiasing(GTK_GS(ps_view->priv->gs))?
			CORBA_TRUE:CORBA_FALSE;
		break;
	}
	case PROP_WATCH_FILE: {
		g_assert(arg->_type == TC_CORBA_boolean);

		*(CORBA_boolean *)arg->_value =
			gtk_gs_get_watch_file(GTK_GS(ps_view->priv->gs))?
			CORBA_TRUE:CORBA_FALSE;
		break;
	}
	default:
		g_assert_not_reached();
	}
}

static void
ggv_postscript_view_set_prop(BonoboPropertyBag *bag,
							 const BonoboArg   *arg,
							 guint              arg_id,
							 CORBA_Environment *ev,
							 gpointer           user_data)
{
	GgvPostScriptView *ps_view;
	GgvPostScriptViewClass *klass;

	g_return_if_fail(user_data != NULL);
	g_return_if_fail(GGV_IS_POSTSCRIPT_VIEW(user_data));

	ps_view = GGV_POSTSCRIPT_VIEW(user_data);

	klass = GGV_POSTSCRIPT_VIEW_CLASS(G_OBJECT_GET_CLASS(ps_view));

	switch(arg_id) {
	case PROP_PAGE: {
		g_assert(arg->_type == TC_CORBA_long);
		ggv_postscript_view_goto_page(ps_view, *(CORBA_long *)arg->_value);
		break;
	}
	case PROP_DEFAULT_ORIENTATION: {
		GtkGSOrientation orient = GTK_GS_ORIENTATION_PORTRAIT;

		g_assert(arg->_type == TC_GNOME_GGV_Orientation);

		ps_view->priv->def_orientation = *(GNOME_GGV_Orientation *) arg->_value;

		switch(ps_view->priv->def_orientation) {
		case GNOME_GGV_ORIENTATION_PORTRAIT:
			orient = GTK_GS_ORIENTATION_PORTRAIT;
			break;
		case GNOME_GGV_ORIENTATION_LANDSCAPE:
			orient = GTK_GS_ORIENTATION_LANDSCAPE;
			break;
		case GNOME_GGV_ORIENTATION_UPSIDEDOWN:
			orient = GTK_GS_ORIENTATION_UPSIDEDOWN;
			break;
		case GNOME_GGV_ORIENTATION_SEASCAPE:
			orient = GTK_GS_ORIENTATION_SEASCAPE;
			break;
		default:
			g_assert_not_reached();
		}

		gtk_gs_set_default_orientation(GTK_GS(ps_view->priv->gs), orient);
		notify_orientation_change(ps_view);
		sync_orientation_items(ps_view);
		break;
	}
	case PROP_OVERRIDE_ORIENTATION: {
		gtk_gs_set_override_orientation(GTK_GS(ps_view->priv->gs),
										*(CORBA_boolean *)arg->_value);
		notify_orientation_change(ps_view);
		sync_orientation_items(ps_view);
		break;
	}
	case PROP_DEFAULT_SIZE: {
		gint size;
		gchar *size_name;

		g_assert(arg->_type == TC_GNOME_GGV_Size);
		
		size_name = *(GNOME_GGV_Size *)arg->_value;

		size = gtk_gs_get_size_index(size_name,
									 gtk_gs_defaults_get_paper_sizes());
		gtk_gs_set_default_size(GTK_GS(ps_view->priv->gs), size);
		sync_size_items(ps_view);
		break;
	}
	case PROP_OVERRIDE_SIZE: {
		gtk_gs_set_override_size(GTK_GS(ps_view->priv->gs),
								 *(CORBA_boolean *)arg->_value);
		sync_size_items(ps_view);
		break;
	}
	case PROP_ANTIALIASING: {
		 gtk_gs_set_override_size(GTK_GS(ps_view->priv->gs),
								   *(CORBA_boolean *)arg->_value);
		 break;
	}
	case PROP_RESPECT_EOF: {
		 gtk_gs_set_respect_eof(GTK_GS(ps_view->priv->gs),
								*(CORBA_boolean *)arg->_value);
		 break;
	}
	case PROP_WATCH_FILE: {
		 gtk_gs_set_watch_file(GTK_GS(ps_view->priv->gs),
							   *(CORBA_boolean *)arg->_value);
		 break;
	}
	default:
		g_assert_not_reached();
	}
}
	
static Bonobo_Unknown
ggv_postscript_view_get_object(BonoboItemContainer *item_container,
							   CORBA_char          *item_name,
							   CORBA_boolean       only_if_exists,
							   CORBA_Environment   *ev,
							   GgvPostScriptView   *ps_view)
{
	Bonobo_Unknown corba_object;
	BonoboObject *object = NULL;
	GSList *params, *c;

	g_return_val_if_fail(ps_view != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail(GGV_IS_POSTSCRIPT_VIEW(ps_view), CORBA_OBJECT_NIL);

	g_message ("ggv_postscript_view_get_object: %d - %s",
			   only_if_exists, item_name);

	params = ggv_split_string (item_name, "!");
	for (c = params; c; c = c->next) {
		gchar *name = c->data;

		if ((!strcmp (name, "control") || !strcmp (name, "embeddable"))
		    && (object != NULL)) {
			g_message ("ggv_postscript_view_get_object: "
					   "can only return one kind of an Object");
			continue;
		}

		if (!strcmp (name, "control"))
			object = (BonoboObject *) ggv_control_new (ps_view);
#if 0
		else if (!strcmp (item_name, "embeddable"))
			object = (BonoboObject *) ggv_embeddable_new (image);
#endif
		else
			g_message ("ggv_postscript_view_get_object: "
					   "unknown parameter `%s'",
					   name);
	}

	g_slist_foreach (params, (GFunc) g_free, NULL);
	g_slist_free (params);

	if (object == NULL)
		return NULL;

	corba_object = bonobo_object_corba_objref (object);

	corba_object = bonobo_object_dup_ref(corba_object, ev);

	return corba_object;
}

BonoboObject *
ggv_postscript_view_add_interfaces (GgvPostScriptView *ps_view,
									BonoboObject *to_aggregate)
{
	BonoboPersistFile   *file;
	BonoboPersistStream *stream;
	BonoboItemContainer *item_container;
	
	g_return_val_if_fail (GGV_IS_POSTSCRIPT_VIEW (ps_view), NULL);
	g_return_val_if_fail (BONOBO_IS_OBJECT (to_aggregate), NULL);

	/* Interface Bonobo::PersistStream */
	stream = bonobo_persist_stream_new (load_ps_from_stream, 
										NULL, NULL,
										"OAFIID:GNOME_GGV_PostScriptView",
										ps_view);
	if (!stream) {
		bonobo_object_unref (BONOBO_OBJECT (to_aggregate));
		return NULL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate),
								 BONOBO_OBJECT (stream));

	/* Interface Bonobo::PersistFile */
	file = bonobo_persist_file_new (load_ps_from_file, NULL,
									"OAFIID:GNOME_GGV_PostScriptView", ps_view);
	if (!file) {
		bonobo_object_unref (BONOBO_OBJECT (to_aggregate));
		return NULL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate),
								 BONOBO_OBJECT (file));

	/* BonoboItemContainer */
	item_container = bonobo_item_container_new ();

	g_signal_connect (G_OBJECT (item_container),
					  "get_object",
					  G_CALLBACK (ggv_postscript_view_get_object),
					  ps_view);
	
	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate),
								 BONOBO_OBJECT (item_container));

	return to_aggregate;
}

BonoboPropertyBag *
ggv_postscript_view_get_property_bag(GgvPostScriptView *ps_view)
{
	g_return_val_if_fail(ps_view != NULL, NULL);
	g_return_val_if_fail(GGV_IS_POSTSCRIPT_VIEW(ps_view), NULL);

	bonobo_object_ref(BONOBO_OBJECT(ps_view->priv->property_bag));

	return ps_view->priv->property_bag;
}

BonoboPropertyControl *
ggv_postscript_view_get_property_control(GgvPostScriptView *ps_view)
{
	g_return_val_if_fail(ps_view != NULL, NULL);
	g_return_val_if_fail(GGV_IS_POSTSCRIPT_VIEW(ps_view), NULL);

	bonobo_object_ref(BONOBO_OBJECT(ps_view->priv->property_control));

	return ps_view->priv->property_control;
}

void
ggv_postscript_view_set_ui_container(GgvPostScriptView *ps_view,
				 Bonobo_UIContainer ui_container)
{
	g_return_if_fail(ps_view != NULL);
	g_return_if_fail(GGV_IS_POSTSCRIPT_VIEW(ps_view));
	g_return_if_fail(ui_container != CORBA_OBJECT_NIL);

	bonobo_ui_component_set_container(ps_view->priv->uic, ui_container, NULL);

	ggv_postscript_view_create_ui(ps_view);
}

void
ggv_postscript_view_unset_ui_container(GgvPostScriptView *ps_view)
{
	g_return_if_fail(ps_view != NULL);
	g_return_if_fail(GGV_IS_POSTSCRIPT_VIEW(ps_view));

	bonobo_ui_component_unset_container(ps_view->priv->uic, NULL);
}

GtkWidget *
ggv_postscript_view_get_widget(GgvPostScriptView *ps_view)
{
	g_return_val_if_fail(ps_view != NULL, NULL);
	g_return_val_if_fail(GGV_IS_POSTSCRIPT_VIEW(ps_view), NULL);

	gtk_widget_ref(ps_view->priv->gs);

	return ps_view->priv->gs;
}

float
ggv_postscript_view_get_zoom_factor(GgvPostScriptView *ps_view)
{
	g_return_val_if_fail(ps_view != NULL, 0.0);
	g_return_val_if_fail(GGV_IS_POSTSCRIPT_VIEW(ps_view), 0.0);

	return gtk_gs_get_zoom(GTK_GS(ps_view->priv->gs));
}

void
ggv_postscript_view_set_zoom_factor(GgvPostScriptView *ps_view,
									float zoom_factor)
{
	g_return_if_fail(ps_view != NULL);
	g_return_if_fail(GGV_IS_POSTSCRIPT_VIEW(ps_view));
	g_return_if_fail(zoom_factor > 0.0);

	gtk_gs_set_zoom(GTK_GS(ps_view->priv->gs), zoom_factor);
}

void
ggv_postscript_view_set_zoom(GgvPostScriptView *ps_view,
							 double zoomx, double zoomy)
{
	g_return_if_fail(zoomx > 0.0);
	g_return_if_fail(zoomy > 0.0);
	g_return_if_fail(ps_view != NULL);
	g_return_if_fail(GGV_IS_POSTSCRIPT_VIEW(ps_view));

	gtk_gs_set_zoom(GTK_GS(ps_view->priv->gs), zoomx);
}

gfloat
ggv_postscript_view_zoom_to_fit(GgvPostScriptView *ps_view,
								gboolean fit_width)
{
	g_return_val_if_fail(ps_view != NULL,0.0);
	g_return_val_if_fail(GGV_IS_POSTSCRIPT_VIEW(ps_view),0.0);

	/* we don't know how not to keep the aspect ratio */
	return gtk_gs_zoom_to_fit(GTK_GS(ps_view->priv->gs), fit_width);
}

void
ggv_postscript_view_set_default_orientation(GgvPostScriptView *ps_view,
											GNOME_GGV_Orientation orientation)
{
	BonoboArg *arg;

	g_return_if_fail(ps_view != NULL);
	g_return_if_fail(GGV_IS_POSTSCRIPT_VIEW(ps_view));

	arg = bonobo_arg_new(TC_GNOME_GGV_Orientation);
	BONOBO_ARG_SET_GENERAL(arg, orientation, TC_GNOME_GGV_Orientation,
						   GNOME_GGV_Orientation, NULL);

	bonobo_pbclient_set_value(BONOBO_OBJREF(ps_view->priv->property_bag),
							  "default_orientation", arg, NULL);
}

GNOME_GGV_Orientation
ggv_postscript_view_get_default_orientation(GgvPostScriptView *ps_view)
{
	BonoboArg *arg;
	GNOME_GGV_Orientation orient;

	g_return_val_if_fail(ps_view != NULL, 0);
	g_return_val_if_fail(GGV_IS_POSTSCRIPT_VIEW(ps_view), 0);

	arg = bonobo_pbclient_get_value(BONOBO_OBJREF(ps_view->priv->property_bag),
									"default_orientation", NULL, NULL);
	if(arg == NULL)
		return 0;

	g_assert(arg->_type == TC_GNOME_GGV_Orientation);
	orient = *(GNOME_GGV_Orientation *) arg->_value;
	bonobo_arg_release(arg);
	return orient;
}

static void
ggv_postscript_view_destroy(BonoboObject *object)
{
	GgvPostScriptView *ps_view;

	g_return_if_fail(object != NULL);
	g_return_if_fail(GGV_IS_POSTSCRIPT_VIEW(object));

	ps_view = GGV_POSTSCRIPT_VIEW(object);

	if (ps_view->priv->gconf_notify_id) {
		gconf_client_notify_remove (gtk_gs_defaults_gconf_client (),
									ps_view->priv->gconf_notify_id);
		ps_view->priv->gconf_notify_id = 0;
	}
	if(ps_view->priv->property_bag) {
		bonobo_object_unref(BONOBO_OBJECT(ps_view->priv->property_bag));
		ps_view->priv->property_bag = NULL;
	}
	if(ps_view->priv->gs) {
		gtk_widget_unref(ps_view->priv->gs);
		ps_view->priv->gs = NULL;
	}
	if(ps_view->priv->msg_win) {
		ggv_msg_window_free(ps_view->priv->msg_win);
		ps_view->priv->msg_win = NULL;
	}
	if(ps_view->priv->uic) {
		bonobo_object_unref(BONOBO_OBJECT(ps_view->priv->uic));
		ps_view->priv->uic = NULL;
	}
	if(ps_view->priv->sidebar) {
		bonobo_object_unref(BONOBO_OBJECT(ps_view->priv->sidebar));
		ps_view->priv->sidebar = NULL;
	}
	ps_view_clean_tmp_file(ps_view);

	if (BONOBO_OBJECT_CLASS (ggv_postscript_view_parent_class)->destroy)
		BONOBO_OBJECT_CLASS(ggv_postscript_view_parent_class)->destroy(object);
}

static void
ggv_postscript_view_finalize(GObject *object)
{
	GgvPostScriptView *ps_view;

	g_return_if_fail(object != NULL);
	g_return_if_fail(GGV_IS_POSTSCRIPT_VIEW(object));

	ps_view = GGV_POSTSCRIPT_VIEW(object);

	ps_view_clean_tmp_file(ps_view);

	if(ps_view->priv->save_path) {
		g_free(ps_view->priv->save_path);
		ps_view->priv->save_path = NULL;
	}

	g_free(ps_view->priv);

	G_OBJECT_CLASS(ggv_postscript_view_parent_class)->finalize(object);
}

static CORBA_string
impl_GNOME_GGV_PostScriptView_getDocument(PortableServer_Servant servant,
										  CORBA_Environment *ev)
{
	GgvPostScriptView *ps_view;
	gchar *doc;
	CORBA_string retval;

	ps_view = GGV_POSTSCRIPT_VIEW(bonobo_object_from_servant(servant));
	doc = gtk_gs_get_postscript(GTK_GS(ps_view->priv->gs), NULL);
	retval = CORBA_string_dup(doc);
	g_free(doc);
	return retval;
}

static CORBA_string
impl_GNOME_GGV_PostScriptView_getPages(PortableServer_Servant servant,
									   GNOME_GGV_PageList *pages,
									   CORBA_Environment *ev)
{
	gint *page_mask;
	GgvPostScriptView *ps_view;
	gchar *doc;
	CORBA_string retval;
	GtkGS *gs;
	int i, num;

	ps_view = GGV_POSTSCRIPT_VIEW(bonobo_object_from_servant(servant));
	gs = GTK_GS(ps_view->priv->gs);
	num = gtk_gs_get_page_count(gs);
	page_mask = g_new0(gint, num);
	for(i = 0; i < pages->_length; i++) {
		if(pages->_buffer[i] < num)
			page_mask[pages->_buffer[i]] = TRUE;
	}
	doc = gtk_gs_get_postscript(gs, page_mask);
	retval = CORBA_string_dup(doc);
	g_free(doc);
	return retval;
}

static void
impl_GNOME_GGV_PostScriptView_reload(PortableServer_Servant servant,
									   CORBA_Environment *ev)
{
	GgvPostScriptView *ps_view;

	ps_view = GGV_POSTSCRIPT_VIEW(bonobo_object_from_servant(servant));
	gtk_gs_reload(GTK_GS(ps_view->priv->gs));
	notify_page_count_change(ps_view);
	notify_page_change(ps_view);
	notify_status_change(ps_view);
}

static void
impl_GNOME_GGV_PostScriptView_close(PortableServer_Servant servant,
									CORBA_Environment *ev)
{
	GgvPostScriptView *ps_view;

	ps_view = GGV_POSTSCRIPT_VIEW(bonobo_object_from_servant(servant));
	gtk_gs_load(GTK_GS(ps_view->priv->gs), NULL);
	notify_page_count_change(ps_view);
	notify_page_change(ps_view);
	notify_status_change(ps_view);
}

static void
ggv_postscript_view_class_init(GgvPostScriptViewClass *klass)
{
	GObjectClass *object_class = (GObjectClass *)klass;
	BonoboObjectClass *bonobo_object_class = (BonoboObjectClass *)klass;
	POA_GNOME_GGV_PostScriptView__epv *epv;

	klass->priv = g_new0(GgvPostScriptViewClassPrivate, 1);

	ggv_postscript_view_parent_class = g_type_class_peek_parent(klass);

	bonobo_object_class->destroy = ggv_postscript_view_destroy;
	object_class->finalize = ggv_postscript_view_finalize;
	
	epv = &klass->epv;

	epv->getDocument = impl_GNOME_GGV_PostScriptView_getDocument;
	epv->getPages = impl_GNOME_GGV_PostScriptView_getPages;
	epv->reload = impl_GNOME_GGV_PostScriptView_reload;
	epv->close = impl_GNOME_GGV_PostScriptView_close;

	/* this seems as a nice place to load the prefs */
	ggv_prefs_load();
}

static void
ggv_postscript_view_init(GgvPostScriptView *ps_view)
{
	ps_view->priv = g_new0(GgvPostScriptViewPrivate, 1);
}

BONOBO_TYPE_FUNC_FULL(GgvPostScriptView, 
					  GNOME_GGV_PostScriptView,
					  BONOBO_TYPE_OBJECT,
					  ggv_postscript_view);

#if 0
static void
property_control_get_prop(BonoboPropertyBag *bag,
						  BonoboArg         *arg,
						  guint              arg_id,
						  CORBA_Environment *ev,
						  gpointer           user_data)
{
	switch(arg_id) {
	case PROP_CONTROL_TITLE:
		g_assert(arg->_type == BONOBO_ARG_STRING);
		BONOBO_ARG_SET_STRING(arg, _("Display"));
		break;
	default:
		g_assert_not_reached();
	}
}
#endif

static BonoboControl *
property_control_get_cb(BonoboPropertyControl *property_control,
						int page_number, void *closure)
{
	GgvPostScriptView *ps_view;

	g_return_val_if_fail(closure != NULL, NULL);
	g_return_val_if_fail(GGV_IS_POSTSCRIPT_VIEW(closure), NULL);
	g_return_val_if_fail(page_number == 0, NULL);

	ps_view = GGV_POSTSCRIPT_VIEW(closure);

#if 0
	container = eog_create_preferences_page(ps_view, page_number);

	gtk_widget_show_all(container);

	control = bonobo_control_new(container);

	/* Property Bag */
	property_bag = bonobo_property_bag_new(property_control_get_prop,
										   NULL, control);

	bonobo_property_bag_add(property_bag, "bonobo:title",
							PROP_CONTROL_TITLE, BONOBO_ARG_STRING,
							NULL, NULL, BONOBO_PROPERTY_READABLE);

	bonobo_object_add_interface(BONOBO_OBJECT(control),
								BONOBO_OBJECT(property_bag));

	return control;
#else
	return NULL;
#endif
}

static void
document_changed_cb(GtkGS *gs, gpointer data)
{
	GgvPostScriptView *ps_view = GGV_POSTSCRIPT_VIEW(data);

	gtk_gs_reload(gs);
	notify_page_count_change(GGV_POSTSCRIPT_VIEW(data));
	notify_page_change(GGV_POSTSCRIPT_VIEW(data));
	notify_status_change(GGV_POSTSCRIPT_VIEW(data));
}

static void
interpreter_message_cb(GtkGS *gs, gchar *msg, gpointer data)
{
	GgvPostScriptView *ps_view = GGV_POSTSCRIPT_VIEW(data);

#if 0
	/* actually, these are quite useless, so we won't show them... */
	if(ps_view->priv->msg_win)
		ggv_msg_window_add_text(ps_view->priv->msg_win, msg, TRUE);
#endif

#ifdef BONOBO_DEBUG
	g_warning(msg);
#endif /* BONOBO_DEBUG */
}

static void
interpreter_error_cb(GtkGS *gs, gint status, gpointer data)
{
	notify_title_change(GGV_POSTSCRIPT_VIEW(data));
	notify_page_change(GGV_POSTSCRIPT_VIEW(data));
	notify_page_count_change(GGV_POSTSCRIPT_VIEW(data));
	notify_status_change(GGV_POSTSCRIPT_VIEW(data));
}

static void
view_realized_cb(GtkGS *gs, gpointer data)
{
	GdkCursor *cursor;

	cursor = cursor_get(GTK_WIDGET(gs)->window, CURSOR_HAND_OPEN);
	gdk_window_set_cursor(GTK_WIDGET(gs)->window, cursor);
	gdk_cursor_unref(cursor);
}

static void
ggv_postscript_view_prefs_changed(GConfClient *client, guint cnxn_id,
								  GConfEntry *entry, gpointer user_data)
{
	GgvPostScriptView *ps_view = GGV_POSTSCRIPT_VIEW(user_data);
	GtkGS *gs = GTK_GS(ps_view->priv->gs);

	if(!strcmp(entry->key, "/apps/ggv/gtkgs/respect_eof")) 
		gtk_gs_set_respect_eof(gs, gconf_value_get_bool(entry->value));
	else if(!strcmp(entry->key, "/apps/ggv/gtkgs/override_orientation")) {
		gtk_gs_set_override_orientation(gs, 
										gconf_value_get_bool(entry->value));
		sync_orientation_items(ps_view);
	}
	else if(!strcmp(entry->key, "/apps/ggv/gtkgs/orientation")) {
		gtk_gs_set_default_orientation(gs, gconf_value_get_int(entry->value));
		sync_orientation_items(ps_view);
	}
#if 0
	else if(!strcmp(entry->key, "/apps/ggv/gtkgs/zoom")) {
		gtk_gs_set_zoom(gs, gconf_value_get_float(entry->value));
	}
#endif
	else if(!strcmp(entry->key, "/apps/ggv/gtkgs/size")) {
		gtk_gs_set_default_size(gs, gconf_value_get_int(entry->value));
		sync_size_items(ps_view);
	}
	else if(!strcmp(entry->key, "/apps/ggv/gtkgs/antialiasing"))
		gtk_gs_set_antialiasing(gs, gconf_value_get_bool(entry->value));
	else if(!strcmp(entry->key, "/apps/ggv/gtkgs/override_size")) {
		gtk_gs_set_override_size(gs, gconf_value_get_bool(entry->value));
		sync_size_items(ps_view);
	}
	else if(!strcmp(entry->key, "/apps/ggv/gtkgs/watch_doc"))
		gtk_gs_set_watch_file(gs, gconf_value_get_bool(entry->value));
	else if(!strcmp(entry->key, "/apps/ggv/gtkgs/scrollstep"))
		gtk_gs_set_scroll_step(gs, gconf_value_get_float(entry->value));
	else if(!strcmp(entry->key, "/apps/ggv/gtkgs/show_scroll_rect"))
		gtk_gs_set_show_scroll_rect(gs, gconf_value_get_bool(entry->value));
	else if(!strcmp(entry->key, "/apps/ggv/control/autojump"))
	   ps_view->priv->pane_auto_jump = gconf_value_get_bool(entry->value);
	else if(!strcmp(entry->key, "/apps/ggv/control/pageflip"))
	   ps_view->priv->page_flip = gconf_value_get_bool(entry->value);
}

static gboolean
sidebar_key_press_event(GtkWidget *widget, GdkEventKey *event,
						gpointer data)
{
	GgvPostScriptView *ps_view = GGV_POSTSCRIPT_VIEW(data);
	GtkGS *gs;

	if(ps_view->priv->gs)
		gs = GTK_GS(ps_view->priv->gs);
	else
		return TRUE;

	switch(event->keyval) {
	case GDK_Left:
		if(!gtk_gs_scroll_step(gs, GTK_SCROLL_STEP_LEFT, FALSE) &&
		   ggv_postscript_view_get_page_flip(ps_view))
			ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) - 1);
		break;
	case GDK_Right:
		if(!gtk_gs_scroll_step(gs, GTK_SCROLL_STEP_RIGHT, FALSE) &&
		   ggv_postscript_view_get_page_flip(ps_view))
			ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) + 1);
		break;
	case GDK_Up:
		if(!gtk_gs_scroll_step(gs, GTK_SCROLL_STEP_UP, FALSE) &&
		   ggv_postscript_view_get_page_flip(ps_view))
			ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) - 1);
		break;
	case GDK_Down:
		if(!gtk_gs_scroll_step(gs, GTK_SCROLL_STEP_DOWN, FALSE) &&
		   ggv_postscript_view_get_page_flip(ps_view))
			ggv_postscript_view_goto_page(ps_view, gtk_gs_get_current_page(gs) + 1);
		break;
	default:
		return FALSE;
		break;
	}
	return TRUE;
}

GgvPostScriptView *
ggv_postscript_view_construct(GgvPostScriptView *ps_view,
							  GtkGS *gs, gboolean zoom_fit)
{
	g_return_val_if_fail(ps_view != NULL, NULL);
	g_return_val_if_fail(GGV_IS_POSTSCRIPT_VIEW(ps_view), NULL);
	g_return_val_if_fail(gs != NULL, NULL);
	g_return_val_if_fail(GTK_IS_GS(gs), NULL);
	g_return_val_if_fail(!GTK_WIDGET_REALIZED(gs), NULL);

	/* Make sure GConf is initialized */
	if(!gconf_is_initialized()) {
		gconf_init(0, NULL, NULL);
	}

	ps_view->priv->gconf_notify_id =
		gconf_client_notify_add(gtk_gs_defaults_gconf_client(),
								"/apps/ggv",
								(GConfClientNotifyFunc)ggv_postscript_view_prefs_changed,
								ps_view, NULL, NULL);
															 

	ps_view->priv->msg_win = ggv_msg_window_new(_("GhostScript output"));

	ps_view->priv->gs = GTK_WIDGET(gs);
	g_object_set(gs, "can-focus", TRUE, NULL);
	gtk_widget_set_events(GTK_WIDGET(gs), 
						  GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
						  GDK_POINTER_MOTION_MASK | GDK_KEY_PRESS_MASK);
	g_signal_connect(G_OBJECT(gs), "button_press_event",
					 G_CALLBACK(view_button_press_cb), ps_view);
	g_signal_connect(G_OBJECT(gs), "button_release_event",
					 G_CALLBACK(view_button_release_cb), ps_view);
	g_signal_connect(G_OBJECT(gs), "motion_notify_event",
					 G_CALLBACK(view_motion_cb), ps_view);
	g_signal_connect(G_OBJECT(gs), "interpreter_message",
					 G_CALLBACK(interpreter_message_cb), ps_view);
	g_signal_connect(G_OBJECT(gs), "interpreter_error",
					 G_CALLBACK(interpreter_error_cb), ps_view);
	g_signal_connect(G_OBJECT(gs), "document_changed",
					 G_CALLBACK(document_changed_cb), ps_view);
	g_signal_connect(G_OBJECT(gs), "realize",
					 G_CALLBACK(view_realized_cb), NULL);

	gtk_widget_show(GTK_WIDGET(gs));

	ps_view->priv->zoom_fit = zoom_fit;

	/* Property Bag */
	ps_view->priv->property_bag = bonobo_property_bag_new(ggv_postscript_view_get_prop,
														  ggv_postscript_view_set_prop,
														  ps_view);


	bonobo_property_bag_add(ps_view->priv->property_bag, "title",
							PROP_TITLE, TC_CORBA_string, NULL,
							_("Document title"), 
							BONOBO_PROPERTY_READABLE);
	bonobo_property_bag_add(ps_view->priv->property_bag, "status",
							PROP_STATUS, TC_CORBA_string, NULL,
							_("GGV control status"), 
							BONOBO_PROPERTY_READABLE);
	bonobo_property_bag_add(ps_view->priv->property_bag, "page_count",
							PROP_PAGE_COUNT, TC_CORBA_long, NULL,
							_("Number of pages"), 
							BONOBO_PROPERTY_READABLE);
	bonobo_property_bag_add(ps_view->priv->property_bag, "page_names",
							PROP_PAGE_NAMES, TC_GNOME_GGV_PageNameList, NULL,
							_("Page names"), 
							BONOBO_PROPERTY_READABLE);
	bonobo_property_bag_add(ps_view->priv->property_bag, "page",
							PROP_PAGE, TC_CORBA_long, NULL,
							_("Current page number"), 
							BONOBO_PROPERTY_READABLE | BONOBO_PROPERTY_WRITEABLE);
	bonobo_property_bag_add(ps_view->priv->property_bag, "width",
							PROP_WIDTH, TC_CORBA_float, NULL,
							_("Document width"), 
							BONOBO_PROPERTY_READABLE);
	bonobo_property_bag_add(ps_view->priv->property_bag, "height",
							PROP_HEIGHT, TC_CORBA_float, NULL,
							_("Document height"), 
							BONOBO_PROPERTY_READABLE);
	bonobo_property_bag_add(ps_view->priv->property_bag, "orientation",
							PROP_ORIENTATION, TC_GNOME_GGV_Orientation, NULL,
							_("Document orientation"), 
							BONOBO_PROPERTY_READABLE);
	bonobo_property_bag_add(ps_view->priv->property_bag, "default_orientation",
							PROP_DEFAULT_ORIENTATION, TC_GNOME_GGV_Orientation, NULL,
							_("Default orientation"), 
							BONOBO_PROPERTY_READABLE | BONOBO_PROPERTY_WRITEABLE);
	bonobo_property_bag_add(ps_view->priv->property_bag, "default_size",
							PROP_DEFAULT_SIZE, TC_GNOME_GGV_Size, NULL,
							_("Default size"), 
							BONOBO_PROPERTY_READABLE | BONOBO_PROPERTY_WRITEABLE);
	bonobo_property_bag_add(ps_view->priv->property_bag, "override_orientation",
							PROP_OVERRIDE_ORIENTATION, TC_CORBA_boolean, NULL,
							_("Override document orientation"), 
							BONOBO_PROPERTY_READABLE | BONOBO_PROPERTY_WRITEABLE);
	bonobo_property_bag_add(ps_view->priv->property_bag, "override_size",
							PROP_OVERRIDE_SIZE, TC_CORBA_boolean, NULL,
							_("Override document size"), 
							BONOBO_PROPERTY_READABLE | BONOBO_PROPERTY_WRITEABLE);
	bonobo_property_bag_add(ps_view->priv->property_bag, "respect_eof",
							PROP_RESPECT_EOF, TC_CORBA_boolean, NULL,
							_("Respect EOF comment"), 
							BONOBO_PROPERTY_READABLE | BONOBO_PROPERTY_WRITEABLE);
	bonobo_property_bag_add(ps_view->priv->property_bag, "watch_file",
							PROP_WATCH_FILE, TC_CORBA_boolean, NULL,
							_("Watch displayed file for changes"), 
							BONOBO_PROPERTY_READABLE | BONOBO_PROPERTY_WRITEABLE);
	bonobo_property_bag_add(ps_view->priv->property_bag, "antialiasing",
							PROP_ANTIALIASING, TC_CORBA_boolean, NULL,
							_("Antialiasing"), 
							BONOBO_PROPERTY_READABLE | BONOBO_PROPERTY_WRITEABLE);

	/* Property Control */
	ps_view->priv->property_control = bonobo_property_control_new
		(property_control_get_cb, 1, ps_view);

	/* UI Component */
	ps_view->priv->uic = bonobo_ui_component_new("GgvPostScriptView");

	ps_view->priv->sidebar = ggv_sidebar_new(ps_view);
	g_signal_connect(G_OBJECT(ggv_sidebar_get_checklist(ps_view->priv->sidebar)),
					 "key_press_event", G_CALLBACK(sidebar_key_press_event), ps_view);

	ps_view->priv->pane_auto_jump =
		gconf_client_get_bool(gtk_gs_defaults_gconf_client(),
							  "/apps/ggv/control/autojump",
							  NULL);
	ps_view->priv->page_flip =
		gconf_client_get_bool(gtk_gs_defaults_gconf_client(),
							  "/apps/ggv/control/pageflip",
							  NULL);
	return ps_view;
}

GgvPostScriptView *
ggv_postscript_view_new(GtkGS *gs, gboolean zoom_fit)
{
	GgvPostScriptView *ps_view;
	
	g_return_val_if_fail(gs != NULL, NULL);
	g_return_val_if_fail(GTK_IS_GS(gs), NULL);

	ps_view = g_object_new(GGV_POSTSCRIPT_VIEW_TYPE, NULL);

	return ggv_postscript_view_construct(ps_view, gs, zoom_fit);
}

GtkAdjustment *
ggv_postscript_view_get_hadj(GgvPostScriptView *ps_view)
{
	return GTK_GS(ps_view->priv->gs)->hadj;
}

GtkAdjustment *
ggv_postscript_view_get_vadj(GgvPostScriptView *ps_view)
{
	return GTK_GS(ps_view->priv->gs)->vadj;
}

void
ggv_postscript_view_set_popup_ui_component(GgvPostScriptView *ps_view,
										   BonoboUIComponent *uic)
{
	ps_view->priv->popup_uic = uic;
	_set_page_items_sensitivity(ps_view, uic);
}

gboolean
ggv_postscript_view_get_auto_jump(GgvPostScriptView *ps_view)
{
	return ps_view->priv->pane_auto_jump;
}

gboolean
ggv_postscript_view_get_page_flip(GgvPostScriptView *ps_view)
{
	return ps_view->priv->page_flip;
}

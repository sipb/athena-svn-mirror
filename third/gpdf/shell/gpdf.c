/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * PDF viewer Bonobo container.
 *
 * Author:
 *   Michael Meeks <michael@ximian.com>
 */
#include <aconf.h>
#include "gpdf-window.h"
#include "gpdf-uri-input.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <gnome.h>
#include <bonobo-activation/bonobo-activation.h>
#include <libbonoboui.h>
#include <libgnomevfs/gnome-vfs-uri.h>
#include <libgnomevfs/gnome-vfs-utils.h>

#define noPDF_DEBUG
#ifdef PDF_DEBUG
#  define DBG(x) x
#else
#  define DBG(x)
#endif

static GPdfRecentFacade *recent_facade;
static GPdfURIInput *uri_in;
/* The list of all open windows */
static GList *window_list = NULL;

static void gpdf_window_update_window_title (GPdfWindow *);

static gboolean
create_window (gpointer data)
{
	const gchar *fname;

	fname = (const gchar *)data;

	if (fname)
		gpdf_uri_input_open_shell_arg (uri_in, fname);

	return FALSE;
}

static gboolean
handle_cmdline_args (gpointer data)
{
	poptContext ctx;
	const gchar **startup_files;

	ctx = data;
	startup_files = poptGetArgs (ctx);

	if (startup_files) {
		gint i;
		for (i = 0; startup_files [i]; ++i)
			g_idle_add (create_window, 
				    (gchar *) startup_files [i]);
	} else {
		g_idle_add (create_window, NULL);
	}
	
	poptFreeContext (ctx);

	return FALSE;
}

static gint
no_contents (const GPdfWindow *window, gconstpointer dummy)
{
	g_assert (GPDF_IS_WINDOW (window));

	return gpdf_window_has_contents (window)
		? -1
		: 0;
}

static GPdfWindow *
find_empty_or_create_new_window (GList *windows)
{
	GPdfWindow *window;
	GList *node;

	node = g_list_find_custom (windows, NULL,
				   (GCompareFunc)no_contents);
	if (node == NULL)
		window = GPDF_WINDOW (gpdf_window_new ());
	else
		window = node->data;

	g_assert (GPDF_IS_WINDOW (window));

	return window;
}


static void
open_request_handler (GPdfURIInput *in, const char *uri, gpointer user_data)
{
	GPdfWindow *window;

	window = find_empty_or_create_new_window (window_list);
	if (gpdf_window_open (window, uri)) {
	  	gpdf_window_update_window_title (window);
		gtk_widget_show_now (GTK_WIDGET (window));
	} else
	  if (g_list_length (window_list) > 1)
	    	gtk_widget_destroy (GTK_WIDGET (window));
}

#ifdef PDF_DEBUG
static void
open_request_logger (GPdfURIInput *in, const char *uri, gpointer user_data)
{
	g_message ("Requested URI: '%s'", uri);
}
#endif

int
main (int argc, char **argv)
{
	GnomeProgram *program;
	GnomeClient *client;
	GValue value = { 0 };
	poptContext ctx;

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	program = gnome_program_init ("gpdf", VERSION,
				      LIBGNOMEUI_MODULE, argc, argv,
				      GNOME_PARAM_APP_DATADIR, DATADIR,
				      NULL);

	if (bonobo_ui_init ("GNOME PDF Viewer", VERSION, &argc, argv) == FALSE)
		g_error (_("Could not initialize Bonobo!\n"));

	gtk_window_set_default_icon_from_file (GNOMEICONDIR "/gnome-pdf.png", NULL);

	client = gnome_master_client ();
	
	g_value_init (&value, G_TYPE_POINTER);
	g_object_get_property (G_OBJECT (program), GNOME_PARAM_POPT_CONTEXT,
			       &value);
	ctx = g_value_get_pointer (&value);
	g_value_unset (&value);

	g_idle_add (handle_cmdline_args, ctx);

	uri_in = g_object_new (GPDF_TYPE_URI_INPUT, NULL);
	recent_facade = g_object_new (GPDF_TYPE_RECENT_FACADE, NULL);
	gpdf_uri_input_set_recent_facade (uri_in, recent_facade);
	DBG (g_signal_connect (G_OBJECT (uri_in), "open_request",
			       G_CALLBACK (open_request_logger), NULL));
	g_signal_connect (G_OBJECT (uri_in), "open_request",
			  G_CALLBACK (open_request_handler), NULL);
	gtk_widget_show (gpdf_window_new ());

	bonobo_main ();

	return 0;
}


/* GPdfWindow class */
#include <recent-files/egg-recent-view.h>
#include <recent-files/egg-recent-view-bonobo.h>
#include <recent-files/egg-recent-view-gtk.h>
#include "gpdf-recent-view-toolitem.h"

#define GPDF_CONTROL_IID    "OAFIID:GNOME_PDF_Control"

#define PARENT_TYPE BONOBO_TYPE_WINDOW

GNOME_CLASS_BOILERPLATE (GPdfWindow, gpdf_window, BonoboWindow, PARENT_TYPE);

struct _GPdfWindowPrivate {
	BonoboWidget *bonobo_widget;
	BonoboUIComponent *ui_component;
  
	GtkWidget *slot;

	EggRecentView *recent_view;

	GtkWidget *exit_fullscreen_popup;
};

/* Drag target types */
enum {
	TARGET_URI_LIST
};

#define GPDF_IS_NON_NULL_WINDOW(obj) \
(((obj) != NULL) && (GPDF_IS_WINDOW ((obj))))


gboolean
gpdf_window_has_contents (const GPdfWindow *gpdf_window)
{
	return (gpdf_window->priv->bonobo_widget != NULL
		&& BONOBO_IS_WIDGET (gpdf_window->priv->bonobo_widget));
}

static gboolean
gw_add_control_to_ui (GPdfWindow *gpdf_window)
{
	GPdfWindowPrivate *priv;
	CORBA_Environment ev;
	Bonobo_UIContainer ui_container;
	GtkWidget *bonobo_widget;

	g_return_val_if_fail (GPDF_IS_NON_NULL_WINDOW (gpdf_window), FALSE);

	priv = gpdf_window->priv;

	CORBA_exception_init (&ev);

	ui_container = BONOBO_OBJREF (
		bonobo_window_get_ui_container (BONOBO_WINDOW (gpdf_window)));
	bonobo_widget =
		bonobo_widget_new_control (GPDF_CONTROL_IID, ui_container);

	if (bonobo_widget == NULL)
		return FALSE;

	g_object_ref (G_OBJECT (bonobo_widget));
	priv->bonobo_widget = BONOBO_WIDGET (bonobo_widget);
	gtk_container_add (GTK_CONTAINER (priv->slot), bonobo_widget);
	CORBA_exception_free (&ev);

	return TRUE;
}

static void
gpdf_window_update_window_title (GPdfWindow *gpdf_window)
{
	Bonobo_PropertyBag pb;
	CORBA_Environment ev;
	char *exception_str;
	gboolean is_default;
	char *title;
	
	g_return_if_fail (GPDF_IS_NON_NULL_WINDOW (gpdf_window));
	
	CORBA_exception_init (&ev);
	
	pb = bonobo_control_frame_get_control_property_bag (
		bonobo_widget_get_control_frame (
			gpdf_window->priv->bonobo_widget), &ev);
	
	/* Here we handle a exception when getting PB */
	if (BONOBO_EX (&ev)) {
		/* Report exception & use default title */
		exception_str = bonobo_exception_get_text (&ev);
		g_warning ("Property bag access exception '%s'", exception_str);
		g_free (exception_str);
		title = g_strdup (_("PDF Document"));
	} else {
		title = bonobo_pbclient_get_string_with_default (pb, "title",
								 _("PDF Document"),
								 &is_default);
		bonobo_object_release_unref (pb, &ev);
	}
	
	CORBA_exception_free (&ev);
	
	g_object_set (G_OBJECT (gpdf_window), "title", title, NULL);
	
	g_free (title);
}

static gboolean
gw_control_load_pdf (GPdfWindow *gpdf_window, const char *uri)
{
	GPdfWindowPrivate *priv;
	CORBA_Environment ev;
	Bonobo_Unknown control;	
	Bonobo_PersistFile persist;
	gboolean success = FALSE;

	g_return_val_if_fail (GPDF_IS_NON_NULL_WINDOW (gpdf_window), FALSE);

	priv = gpdf_window->priv;
	control = bonobo_widget_get_objref (priv->bonobo_widget);

	g_return_val_if_fail (control != CORBA_OBJECT_NIL, FALSE);
	
	CORBA_exception_init (&ev);

	persist = Bonobo_Unknown_queryInterface (
		control, "IDL:Bonobo/PersistFile:1.0", &ev);

	if (BONOBO_EX (&ev) || persist == CORBA_OBJECT_NIL) {
		g_warning ("Panic: component doesn't implement PersistFile.");
		goto error_persist;
	}

	Bonobo_PersistFile_load (persist, uri, &ev);

	if (BONOBO_EX (&ev))
		goto error_persist_file;

	success = TRUE;		

error_persist_file:
	bonobo_object_release_unref (persist, &ev);
error_persist:
	CORBA_exception_free (&ev);

	return success;
}

gboolean
gpdf_window_open (GPdfWindow *gpdf_window, const char *name)
{
	GPdfWindowPrivate *priv;

	g_return_val_if_fail (GPDF_IS_NON_NULL_WINDOW (gpdf_window), FALSE);
	g_return_val_if_fail (!gpdf_window_has_contents (gpdf_window), FALSE);

	priv = gpdf_window->priv;

	if (gw_add_control_to_ui (gpdf_window) == FALSE)
		return FALSE;

	if (gw_control_load_pdf (gpdf_window, name) == FALSE) {
	  	gtk_container_remove (GTK_CONTAINER (priv->slot),
				      GTK_WIDGET (priv->bonobo_widget));
		gtk_widget_destroy (GTK_WIDGET (priv->bonobo_widget));
		priv->bonobo_widget = NULL;
		bonobo_ui_component_set_prop (gpdf_window->priv->ui_component,
					      "/commands/ViewFullScreen",
					      "sensitive", "0", NULL);
		return FALSE;
	}

	/* Activate component */
	bonobo_control_frame_control_activate (
	       bonobo_widget_get_control_frame (priv->bonobo_widget));
	gtk_widget_show (GTK_WIDGET (priv->bonobo_widget));

	bonobo_ui_component_set_prop (gpdf_window->priv->ui_component,
				      "/commands/ViewFullScreen",
				      "sensitive", "1", NULL);
	
	return TRUE;
}

static void
gw_drag_data_received (GtkWidget *widget, GdkDragContext *context,
		       gint x, gint y, GtkSelectionData *selection_data,
		       guint info, guint time)
{
	g_return_if_fail (info == TARGET_URI_LIST);

	gpdf_uri_input_open_uri_list (uri_in,
				      (const gchar *)selection_data->data);
}

void
gpdf_window_close (GPdfWindow *gpdf_window)
{
	g_return_if_fail (GPDF_IS_NON_NULL_WINDOW (gpdf_window));

	bonobo_control_frame_set_autoactivate
	  (bonobo_widget_get_control_frame (gpdf_window->priv->bonobo_widget),
	   FALSE);
	bonobo_control_frame_control_deactivate
	  (bonobo_widget_get_control_frame (gpdf_window->priv->bonobo_widget));
	gtk_widget_destroy (GTK_WIDGET (gpdf_window));

	if (window_list == NULL)
		bonobo_main_quit ();
}

static void
gpdf_window_set_fullscreen (GPdfWindow *gpdf_window, gboolean fullscreen)
{
	if (fullscreen)
		gtk_window_fullscreen (GTK_WINDOW (gpdf_window));
	else
		gtk_window_unfullscreen (GTK_WINDOW (gpdf_window));
}

static void
exit_fullscreen_button_clicked_cb (GtkWidget *button, GPdfWindow *window)
{
	gpdf_window_set_fullscreen (window, FALSE);
}

static void
update_exit_fullscreen_popup_position (GPdfWindow *gpdf_window)
{
	GdkRectangle screen_rect;
	int popup_height;

	if (gpdf_window->priv->exit_fullscreen_popup == NULL)
		return;

	gtk_window_get_size (GTK_WINDOW (gpdf_window->priv->exit_fullscreen_popup),
                             NULL, &popup_height);

	gdk_screen_get_monitor_geometry (
		gdk_screen_get_default (),
		gdk_screen_get_monitor_at_window (gdk_screen_get_default (),
						  GTK_WIDGET (gpdf_window)->window),
		&screen_rect);

	gtk_window_move (GTK_WINDOW (gpdf_window->priv->exit_fullscreen_popup),
			 screen_rect.x, screen_rect.height - popup_height);
}

static void
screen_size_changed_cb (GdkScreen *screen, GPdfWindow *gpdf_window)
{
	update_exit_fullscreen_popup_position (gpdf_window);
}

static void
gpdf_window_init_fullscreen_popup (GPdfWindow *gpdf_window)
{
	GtkWidget *popup, *button, *hbox, *icon, *label;
	GtkAccelGroup *accel_group;

	accel_group = gtk_accel_group_new ();
	
	popup = gtk_window_new (GTK_WINDOW_POPUP);
	gpdf_window->priv->exit_fullscreen_popup = popup;

	button = gtk_button_new ();
	g_signal_connect (button, "clicked",
			  G_CALLBACK (exit_fullscreen_button_clicked_cb),
			  gpdf_window);
	gtk_widget_add_accelerator (button, "clicked", accel_group,
				    GDK_Escape, 0,
				    GTK_ACCEL_VISIBLE);
	gtk_widget_add_accelerator (button, "clicked", accel_group,
				    GDK_F11, 0,
				    GTK_ACCEL_VISIBLE);
	gtk_widget_show (button);
	gtk_container_add (GTK_CONTAINER (popup), button);

	hbox = gtk_hbox_new (FALSE, 2);
	gtk_widget_show (hbox);
	gtk_container_add (GTK_CONTAINER (button), hbox);

	icon = gtk_image_new_from_stock (GTK_STOCK_QUIT, GTK_ICON_SIZE_BUTTON);
	gtk_widget_show (icon);
	gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 0);

	label = gtk_label_new (_("Exit Full Screen"));
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	update_exit_fullscreen_popup_position (gpdf_window);

	gtk_widget_show (popup);

	g_signal_connect (G_OBJECT (gdk_screen_get_default ()),
                          "size-changed", G_CALLBACK (screen_size_changed_cb),
			  gpdf_window);

	gtk_window_add_accel_group (GTK_WINDOW (gpdf_window), accel_group);
}

static gboolean
gpdf_window_window_state_changed (GtkWidget *widget, GdkEventWindowState *event,
				  gpointer user_data)
{
	gboolean fullscreen;
	GPdfWindow *gpdf_window;
	BonoboUIComponent *uic;

	if (!(event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN))
		return FALSE;

	g_return_val_if_fail (GPDF_IS_NON_NULL_WINDOW (widget), FALSE);

	gpdf_window = GPDF_WINDOW (widget);
	fullscreen = event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN;

	uic = gpdf_window->priv->ui_component;
	if (uic == NULL)
		return FALSE;

	bonobo_ui_component_set_prop (uic, "/commands/ViewFullScreen", "state",
				      fullscreen ? "1" : "0", NULL);

	if (fullscreen) {
		bonobo_ui_component_freeze (uic, NULL);
		bonobo_ui_component_set_prop (uic,
					      "/menu", "hidden", "1", NULL);
		bonobo_ui_component_set_prop (uic,
					      "/Toolbar", "hidden", "1", NULL);
		bonobo_ui_component_thaw (uic, NULL);

		gpdf_window_init_fullscreen_popup (gpdf_window);
	} else {
		bonobo_ui_component_freeze (uic, NULL);
		bonobo_ui_component_set_prop (uic,
					      "/menu", "hidden", "0", NULL);
		bonobo_ui_component_set_prop (uic,
					      "/Toolbar", "hidden", "0", NULL);
		bonobo_ui_component_thaw (uic, NULL);

		gtk_widget_destroy (gpdf_window->priv->exit_fullscreen_popup);
		gpdf_window->priv->exit_fullscreen_popup = NULL;
        }

	return FALSE;
}

static gboolean
gw_ask_for_uri (GPdfWindow *gpdf_window, gchar ***uri)
{
	GtkFileChooser *chooser;
	GtkFileFilter *pdf_filter, *all_filter;
	gboolean accepted = FALSE;

	g_return_val_if_fail (GPDF_IS_NON_NULL_WINDOW (gpdf_window), FALSE);
	g_return_val_if_fail (uri, FALSE);
	g_return_val_if_fail (*uri == NULL, FALSE);

	chooser = GTK_FILE_CHOOSER (gtk_file_chooser_dialog_new (_("Load file"),
								 GTK_WINDOW (gpdf_window),
								 GTK_FILE_CHOOSER_ACTION_OPEN,
								 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
								 GTK_STOCK_OPEN, GTK_RESPONSE_OK,
								 NULL));
	gtk_file_chooser_set_select_multiple (chooser, TRUE); 
	gtk_window_set_modal (GTK_WINDOW (chooser), TRUE);
	pdf_filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (pdf_filter, _("PDF Documents"));
	gtk_file_filter_add_mime_type (pdf_filter, "application/pdf");
	gtk_file_chooser_add_filter (chooser, pdf_filter);
	all_filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (all_filter, _("All Files"));
	gtk_file_filter_add_pattern (all_filter, "*");
	gtk_file_chooser_add_filter (chooser, all_filter);
	gtk_file_chooser_set_filter (chooser, pdf_filter);
	
	gtk_window_set_position (GTK_WINDOW (chooser), GTK_WIN_POS_MOUSE);

	accepted = (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_OK);

	if (accepted) {
		GSList *files, *iter;
		guint n; 

		files = gtk_file_chooser_get_filenames (chooser);

		*uri = g_new (gchar*, g_slist_length (files) + 1);

		n = 0;
		for (iter = files; iter != NULL; iter = iter->next) {
			(*uri)[n] =
				gnome_vfs_get_uri_from_local_path (iter->data);
			g_free (iter->data);
			n++;
		}
		(*uri)[n] = NULL; 

		g_slist_free (files);
	}

	gtk_widget_destroy (GTK_WIDGET (chooser));

	return accepted;
}

static void
gw_open_dialog (GPdfWindow *gpdf_window)
{
	gchar **uri = NULL;

	g_return_if_fail (GPDF_IS_NON_NULL_WINDOW (gpdf_window));

	if (gw_ask_for_uri (gpdf_window, &uri)) {
		guint n;
		
		g_assert (uri != NULL);

		n = 0; 
		while (uri[n]) {
			gpdf_uri_input_open_uri (uri_in, uri[n]);
			n++;
		}
		g_strfreev (uri); 
	}
}

static gboolean
gw_open_recent_file (EggRecentView *view, EggRecentItem *item, gpointer data)
{
	GPdfURIInput *uri_in;
	gchar *uri;   

	uri_in = GPDF_URI_INPUT (data);

	uri = egg_recent_item_get_uri_utf8 (item);
	gpdf_uri_input_open_uri (uri_in, uri);

	return TRUE;
}


static void
verb_FileOpen_cb (BonoboUIComponent *uic, gpointer user_data,
		  const char *cname)
{
	gw_open_dialog (GPDF_WINDOW (user_data));
}

static void
verb_FileClose_cb (BonoboUIComponent *uic, gpointer user_data,
		   const char *cname)
{
	gpdf_window_close (GPDF_WINDOW (user_data));
}

static void
listener_ViewFullScreen (BonoboUIComponent *uic, const char *path,
                         Bonobo_UIComponent_EventType type, const char *state,
                         gpointer user_data)
{
	g_return_if_fail (GPDF_IS_NON_NULL_WINDOW (user_data));

	if(type != Bonobo_UIComponent_STATE_CHANGED)
		return;

	if(!state)
		return;

	gpdf_window_set_fullscreen(GPDF_WINDOW (user_data), atoi (state));
}


/* Brings attention to a window by raising it and giving it focus */
static void
raise_and_focus (GtkWidget *widget)
{
	g_assert (GTK_WIDGET_REALIZED (widget));
	gdk_window_show (widget->window);
	gtk_widget_grab_focus (widget);
}

static void
verb_HelpContents_cb (BonoboUIComponent *uic, gpointer user_data,
		     const char *cname)
{
	gnome_help_display ("gpdf.xml",
			    NULL,
			    NULL);
}

static void
verb_HelpAbout_cb (BonoboUIComponent *uic, gpointer user_data,
		   const char *cname)
{
	static GtkWidget *about;

	gchar *tmp_str;

	static const gchar *authors[] = {
		N_("Derek B. Noonburg, Xpdf author."),
		N_("Martin Kretzschmar, GNOME port maintainer."),
		N_("Michael Meeks, GNOME port creator."),
		/* please localize as R&eacute;mi (R\303\251mi) */
		N_("Remi Cohen-Scali."),
		N_("Miguel de Icaza."),
		N_("Nat Friedman."),
		N_("Ravi Pratap."),
		NULL
	};

	static const gchar *documenters[] = {
		N_("Chee Bin HOH <cbhoh@mimos.my>"),
		N_("Breda McColgan <BredaMcColgan@sun.com>"),
		NULL
	};

	static const gchar *translators;

	if (!about) {
#ifdef ENABLE_NLS
		int i;
		
		for (i = 0; authors[i] != NULL; i++)
			authors [i] = _(authors [i]);
#endif

		translators = _("translator-credits");

		tmp_str = g_strconcat ("Copyright \xc2\xa9 1996-2004 ",
				     _("Glyph & Cog, LLC and authors"),
				     NULL);

		about = gnome_about_new (
			_("GNOME PDF Viewer"),
			VERSION,
			tmp_str,
			_("A PDF viewer based on Xpdf"),
			authors,
			documenters,
			(strcmp (translators,
				 "translator-credits")
			 ? translators : NULL),
			NULL);
	
		g_signal_connect (about, "destroy",
				  G_CALLBACK (gtk_widget_destroyed),
				  &about);

		g_free (tmp_str);
	}

	gtk_widget_show_now (about);
	raise_and_focus (about);
}


static gint
gw_delete_event (GtkWidget *widget, GdkEventAny *event)
{
	gpdf_window_close (GPDF_WINDOW (widget));
	return TRUE;
}

static void
gw_destroy (GtkObject *object)
{
	GPdfWindow *gpdf_window = GPDF_WINDOW (object);
	GPdfWindowPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_WINDOW (gpdf_window));

	priv = gpdf_window->priv;
	window_list = g_list_remove (window_list, gpdf_window);

	if (priv->bonobo_widget != NULL) {
		g_object_unref  (G_OBJECT (priv->bonobo_widget));
		priv->bonobo_widget = NULL;
	}

	if (priv->recent_view != NULL) {
		g_object_unref (G_OBJECT (priv->recent_view));
		priv->recent_view = NULL;
	}
		
	GNOME_CALL_PARENT (GTK_OBJECT_CLASS, destroy, (object));
}

static void
gw_finalize (GObject *object)
{
	g_return_if_fail (GPDF_IS_NON_NULL_WINDOW (object));

	g_free (GPDF_WINDOW (object)->priv);

	GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

static void
gpdf_window_class_init (GPdfWindowClass *klass)
{
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS (klass);

	G_OBJECT_CLASS (klass)->finalize = gw_finalize;
	GTK_OBJECT_CLASS (klass)->destroy = gw_destroy;

	widget_class->delete_event = gw_delete_event;
	widget_class->drag_data_received = gw_drag_data_received;
}

static void
gpdf_window_instance_init (GPdfWindow *gpdf_window)
{
	g_return_if_fail (GPDF_IS_NON_NULL_WINDOW (gpdf_window));
	
	gpdf_window->priv = g_new0 (GPdfWindowPrivate, 1);
	window_list = g_list_prepend (window_list, gpdf_window);
}


/* Construction */

static void
gw_setup_dnd (GPdfWindow *gpdf_window)
{
	static GtkTargetEntry drag_types[] = {
		{ "text/uri-list", 0, TARGET_URI_LIST },
	};
	static gint n_drag_types =
		sizeof (drag_types) / sizeof (drag_types [0]);

	g_return_if_fail (GPDF_IS_NON_NULL_WINDOW (gpdf_window));

	gtk_drag_dest_set (GTK_WIDGET (gpdf_window),
			   GTK_DEST_DEFAULT_ALL,
			   drag_types, n_drag_types, GDK_ACTION_COPY);
}

static void
gw_setup_local_contents (GPdfWindow *gpdf_window)
{
	g_return_if_fail (GPDF_IS_NON_NULL_WINDOW (gpdf_window));

	gpdf_window->priv->slot = gtk_event_box_new ();
	gtk_widget_show (gpdf_window->priv->slot);
	bonobo_window_set_contents (BONOBO_WINDOW (gpdf_window),
				    gpdf_window->priv->slot);
	gtk_widget_show_all (gpdf_window->priv->slot);
}

/*
 * The menus.
 */
static BonoboUIVerb gw_verbs [] = {
	BONOBO_UI_VERB ("FileOpen",  verb_FileOpen_cb),
	BONOBO_UI_VERB ("FileClose", verb_FileClose_cb),

	BONOBO_UI_VERB ("HelpContents", verb_HelpContents_cb),
	BONOBO_UI_VERB ("HelpAbout", verb_HelpAbout_cb),

	BONOBO_UI_VERB_END
};

static void
gw_setup_toplevel_ui (GPdfWindow *gpdf_window)
{
	GPdfWindowPrivate *priv;
	BonoboUIContainer *ui_container;

	g_return_if_fail (GPDF_IS_NON_NULL_WINDOW (gpdf_window));

	priv = gpdf_window->priv;
	ui_container = bonobo_window_get_ui_container (
		BONOBO_WINDOW (gpdf_window));

	priv->ui_component = bonobo_ui_component_new ("gpdf");
	bonobo_ui_component_set_container (priv->ui_component,
					   BONOBO_OBJREF (ui_container),
					   NULL);
	bonobo_ui_component_add_verb_list_with_data (priv->ui_component,
						     gw_verbs, gpdf_window);
	bonobo_ui_util_set_ui (priv->ui_component, 
			       DATADIR, "gpdf-window-ui.xml", "GPDF",
			       NULL);

	bonobo_ui_component_add_listener(priv->ui_component, "ViewFullScreen",
					 listener_ViewFullScreen, gpdf_window);
}

static void
gw_setup_recent_menu (GPdfWindow *gpdf_window, GPdfRecentFacade *facade,
		      GPdfURIInput *uri_in)
{
	const EggRecentModel *model;
	EggRecentView *view;

	model = gpdf_recent_facade_get_model (facade);
	view = EGG_RECENT_VIEW (egg_recent_view_bonobo_new (
					gpdf_window->priv->ui_component,
					"/menu/File/Recents"));
	egg_recent_view_bonobo_show_icons (EGG_RECENT_VIEW_BONOBO (view),
					   FALSE);
	egg_recent_view_set_model (view, (EggRecentModel *)model);
	g_signal_connect (G_OBJECT (view), "activate",
			  G_CALLBACK (gw_open_recent_file), uri_in);
	gpdf_window->priv->recent_view = view;
}

static void
gw_setup_recent_toolitem (GPdfWindow *gpdf_window, GPdfRecentFacade *facade,
			  GPdfURIInput *uri_in)
{
	GtkWidget *toolitem;
	const EggRecentModel *model;

	toolitem = GTK_WIDGET (g_object_new (GPDF_TYPE_RECENT_VIEW_TOOLITEM,
					   NULL));
	gtk_widget_show_all (GTK_WIDGET (toolitem));       

	model = gpdf_recent_facade_get_model (facade);

	gpdf_recent_view_toolitem_set_model (
		GPDF_RECENT_VIEW_TOOLITEM (toolitem), (EggRecentModel *)model);
	g_signal_connect_object (G_OBJECT (toolitem), "item_activate",
				 G_CALLBACK (gw_open_recent_file),
				 uri_in, 0);

	bonobo_ui_component_widget_set (gpdf_window->priv->ui_component,
					"/Toolbar/FileOpenMenu",
					GTK_WIDGET (toolitem), NULL);
}

GPdfWindow *gpdf_window_construct (GPdfWindow *gpdf_window)
{
	g_return_val_if_fail (GPDF_IS_NON_NULL_WINDOW (gpdf_window), NULL);

	gw_setup_dnd (gpdf_window);
	gw_setup_local_contents (gpdf_window);
	gw_setup_toplevel_ui (gpdf_window);
	gw_setup_recent_menu (gpdf_window, recent_facade, uri_in);
	gw_setup_recent_toolitem (gpdf_window, recent_facade, uri_in);

	g_signal_connect (G_OBJECT (gpdf_window), "window_state_event",
			  G_CALLBACK (gpdf_window_window_state_changed), NULL);

	return gpdf_window;
}

GtkWidget *gpdf_window_new (void)
{
	GPdfWindow *gpdf_window;

	gpdf_window = GPDF_WINDOW (g_object_new (GPDF_TYPE_WINDOW,
						 "win_name", "gpdf",
						 "title", _("PDF Viewer"),
						 "default-width", 600,
						 "default-height", 600,
						 NULL));

	return GTK_WIDGET (gpdf_window_construct (gpdf_window));
}

/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; c-indent-level: 8; tab-width: 8; -*- */
/**
 * gpdf bonobo control
 * nee eog-control.c
 *
 * Authors:
 *   Martin Baulig <baulig@suse.de> (eog-control.c)
 *   Martin Kretzschmar <Martin.Kretzschmar@inf.tu-dresden.de>
 *
 * Copyright 2000 SuSE GmbH.
 * Copyright 2002 Martin Kretzschmar
 */

#define GNOME_PRINT_UNSTABLE_API

#include <math.h>
#include <time.h>
#include <unistd.h>
#include "gpdf-control.h"
#include "gpdf-control-private.h"
#include "gpdf-stock-icons.h"
#include "gpdf-view.h"
#include "gpdf-bookmarks-view.h"
#include "gpdf-thumbnails-view.h"
#include "gpdf-annots-view.h"
#include "gpdf-hig-dialog.h"
#include "gpdf-util.h"
#include "pdf-info-dict-util.h"
#include "gpdf-g-switch.h"
#  include "page-control.h"
#  include "pdf-properties-display.h"
#  include <libgnome/libgnome.h>
#  include <bonobo.h>
#  include <libgnomevfs/gnome-vfs-uri.h>
#  include <libgnomevfs/gnome-vfs-utils.h>
#  include <libgnomeui/gnome-appbar.h>
#  include <libgnomeprint/gnome-print.h>
#  include <libgnomeprint/gnome-print-job.h>
#  include <libgnomeprintui/gnome-print-dialog.h>
#  include <glade/glade.h>
#  include "gpdf-sidebar.h"
#  include "eel-gconf-extensions.h"
#  include "prefs-strings.h"
#include "gpdf-g-switch.h"
#include "gpdf-persist-stream.h"
#include "gpdf-persist-file.h"
#include "PSOutputDev.h"
#include "GlobalParams.h"
#include "gpdf-stock-icons.h"

#define noPDF_DEBUG
#ifdef PDF_DEBUG
#  define DBG(x) x
#else
#  define DBG(x)
#endif

BEGIN_EXTERN_C

#define GPDF_UI_XML_FILE			"gpdf-control-ui.xml"
#define GPDF_EDIT_FIND_DIALOG_GLADE_FILE	DATADIR "/gpdf/glade/gpdf-findtext-dialog.glade"
#define GPDF_GNOME_HELP_DOC_ID			"gpdf"
#define GPDF_GNOME_HELP_FILENAME		"gpdf.xml"

struct _GPdfControlPrivate {
	GPdfPersistStream *persist_stream;
	GPdfPersistFile *persist_file;
	PDFDoc *pdf_doc;
	char *title;

	GtkWidget *vbox, *hpaned;
	GtkWidget *scrolled_window;
	GtkWidget *appbar;
	GtkWidget *gpdf_view;
	GtkWidget *gpdf_sidebar;
	GtkWidget *gpdf_bookmarks_view;
	GtkWidget *gpdf_thumbnails_view;
#ifdef USE_ANNOTS_VIEW
	GtkWidget *gpdf_annots_view;
#endif
	GnomeAppBar *gnome_appbar;
	GtkProgressBar *progress;

	BonoboZoomable *zoomable;
	gboolean has_zoomable_frame;

	gboolean activated;
	gboolean ignore_toggles_changes;
	
	BonoboControl *bonobo_page_control;

	GtkWidget *file_chooser;
	GtkWidget *properties;
	GtkWidget *print_dialog;

	guint idle_id;
};

/* Components paths */
#define TOOLBAR_FIT_WIDTH_PATH		"/Toolbar/ZoomFitWidth"
#define MENU_FIT_WIDTH_PATH		"/menu/View/Zoom Items Placeholder/ZoomFitWidth"
#define TOOLBAR_TOOL_CONTROL_PATH	"/Toolbar/Tool Control Placeholder/ToolControl"
#define TOOLBAR_PAGE_CONTROL_PATH	"/Toolbar/Page Control Placeholder/PageControl"

/* Actions paths */
#define FILE_PRINT_CMDPATH		"/commands/FilePrint"
#define FILE_PROPERTIES_CMDPATH		"/commands/FileProperties"
#define FILE_SAVEAS_CMDPATH		"/commands/FileSaveAs"
#define GO_PAGE_FIRST_CMDPATH		"/commands/GoPageFirst"
#define GO_PAGE_PREV_CMDPATH		"/commands/GoPagePrev"
#define GO_PAGE_NEXT_CMDPATH		"/commands/GoPageNext"
#define GO_PAGE_LAST_CMDPATH		"/commands/GoPageLast"
#define GO_HISTORY_PREV_CMDPATH		"/commands/GoHistoryPrev"
#define GO_HISTORY_NEXT_CMDPATH		"/commands/GoHistoryNext"
#define VIEW_SIDEBAR_CMDPATH		"/commands/ViewSidebar"
#define EDIT_COPY_CMDPATH		"/commands/EditCopy"
#define ZOOM_IN_CMDPATH			"/commands/ZoomIn"
#define ZOOM_OUT_CMDPATH		"/commands/ZoomOut"
#define ZOOM_FIT_CMDPATH		"/commands/ZoomFit"
#define ZOOM_FIT_WIDTH_CMDPATH		"/commands/ZoomFitWidth"

/* Components IDs */
#define VIEW_SIDEBAR_COMP_ID		"ViewSidebar"

#define PARENT_TYPE BONOBO_TYPE_CONTROL
BONOBO_CLASS_BOILERPLATE (GPdfControl, gpdf_control,
			  BonoboControl, PARENT_TYPE);

#define GPDF_IS_NON_NULL_CONTROL(obj) \
	(((obj) != NULL) && (GPDF_IS_CONTROL ((obj))))

enum {
	PROP_0,
	PROP_PERSIST_STREAM,
	PROP_PERSIST_FILE
};

enum {
	PBPROP_TITLE
};

static void gpdf_control_view_sidebar_changed_cb (BonoboUIComponent *component,
						  const char *path,
						  Bonobo_UIComponent_EventType type,
						  const char *state,
						  GPdfControl *control);
static void gpdf_control_bookmark_selected_cb  	 (GPdfBookmarksView *gpdf_bookmarks_view,
						  LinkAction        *link_action, 
						  gpointer           data);
static void gpdf_control_thumbnail_selected_cb 	 (GPdfThumbnailsView *gpdf_thumbnails_view,
						  gint x, gint y,
						  gint w, gint h,
						  gint page,
						  gpointer data);
#ifdef USE_ANNOTS_VIEW
static void gpdf_control_annotation_selected_cb	 (GPdfAnnotsView *gpdf_annots_view,
						  Annot          *annot, 
						  gint            page,
						  gpointer        data);
static void gpdf_control_annotation_toggled_cb	 (GPdfAnnotsView *gpdf_annots_view,
						  Annot          *annot, 
						  gint            page,
						  gboolean	  active, 
						  gpointer        data);
#endif
static void gpdf_control_sidebar_page_changed_cb (GPdfSidebar *sidebar,
						  const gchar *page_id,
						  GPdfControl *control); 


/*
 * Here are the basic services furnished by control for
 * handling user interface. They could finaly go to a
 * specific utils class, or a feedback class (including
 * appbar?).
 */

/* Set the status label without changing widget state */
void
gpdf_control_private_set_status (GPdfControl * control,
				 const gchar * status)
{
	GPdfControlPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));
	priv = control->priv;

	gnome_appbar_set_status (priv->gnome_appbar, status);
}

/* Reset status after some time */
static void
gpdf_control_private_status_timeout (gpointer data)
{
	GPdfControlPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (data));
	priv = GPDF_CONTROL (data)->priv;

	gnome_appbar_clear_stack (priv->gnome_appbar);
	gtk_progress_bar_set_fraction (priv->progress, 0.0);

	g_object_set_data (G_OBJECT (data), "status-timeout-id", NULL);
}

/* Push a status text in status bar */
void
gpdf_control_private_push (GPdfControl * control,
			   const gchar * status)
{
	GPdfControlPrivate *priv;
	guint to_id;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));
	priv = control->priv;

	if ((to_id = (unsigned int)
	     g_object_get_data (G_OBJECT (control),
				"status-timeout-id")) != 0) {
		g_object_set_data (G_OBJECT (control),
				   "status-timeout-id",
				   NULL);
		g_source_remove (to_id);
	}
	
	
	gnome_appbar_push (priv->gnome_appbar, status);
	to_id = g_timeout_add_full (0,
				    20,
				    (GtkFunction)gpdf_control_private_status_timeout,
				    (gpointer)control,
				    NULL);
	g_object_set_data (G_OBJECT (control),
			   "status-timeout-id",
			   (gpointer)to_id);
}

/* Pop a text from status stack */
void
gpdf_control_private_pop (GPdfControl * control)
{
	GPdfControlPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));
	priv = control->priv;

	gnome_appbar_pop (priv->gnome_appbar);
}

/* Empty status stack */
void
gpdf_control_private_clear_stack (GPdfControl * control)
{
	GPdfControlPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));
	priv = control->priv;

	gnome_appbar_clear_stack (priv->gnome_appbar);
}

/* Set fraction on status progess bar */
void
gpdf_control_private_set_fraction (GPdfControl * control,
				   double        fraction)
{
	GPdfControlPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));
	priv = control->priv;

	if (fraction < 0.0)
	  fraction = 0.0;
	if (fraction > 1.0)
	  fraction = 1.0;

	gtk_progress_bar_set_fraction (priv->progress, fraction);
}

/* Set pulse step value when progress used in pluse mode */
void
gpdf_control_private_set_pulse_step (GPdfControl * control,
				     gdouble       fraction)
{
	GPdfControlPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));
	priv = control->priv;

	if (fraction < 0.0)
	  fraction = 0.0;
	if (fraction > 1.0)
	  fraction = 1.0;

	gtk_progress_bar_set_pulse_step (priv->progress, fraction);
}

/* Pulse progress one step */
void
gpdf_control_private_pulse (GPdfControl * control)
{
	GPdfControlPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));
	priv = control->priv;

	gtk_progress_bar_pulse (priv->progress);
}

/* Refresh current state of stack/default. */
void
gpdf_control_private_refresh (GPdfControl * control)
{
	GPdfControlPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));
	priv = control->priv;

	gnome_appbar_refresh (priv->gnome_appbar);
}

/* Set cursor for whole component */
void
gpdf_control_private_set_cursor	(GPdfControl * control,
				 GdkCursor   * cursor)
{
	GPdfControlPrivate *priv;
	GdkWindow *parent_window;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));
	priv = control->priv;
	
	parent_window = gtk_widget_get_parent_window (priv->vbox);
	if (GDK_IS_WINDOW (parent_window))
	  gdk_window_set_cursor (parent_window, cursor); 
	gdk_cursor_unref (cursor);
	gdk_flush();	
}

/* Set wait cursor for whole component */
void
gpdf_control_private_set_wait_cursor (GPdfControl * control)
{
	GPdfControlPrivate *priv;
	GdkDisplay *display;
	GdkCursor *cursor;
	GdkWindow *parent_window;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));
	priv = control->priv;
	
	display = gtk_widget_get_display (priv->vbox);
	cursor = gdk_cursor_new_for_display (display, GDK_WATCH);
	parent_window = gtk_widget_get_parent_window (priv->vbox);
	if (GDK_IS_WINDOW (parent_window))
	  gdk_window_set_cursor (parent_window, cursor); 
	gdk_cursor_unref (cursor);
	gdk_flush();		
}

/* Reset to default cursor for whole component */
void
gpdf_control_private_reset_cursor (GPdfControl * control)
{
	GPdfControlPrivate *priv;
	GdkWindow *parent_window;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));
	priv = control->priv;

	parent_window = gtk_widget_get_parent_window (priv->vbox);
	if (GDK_IS_WINDOW (parent_window))
	  gdk_window_set_cursor (parent_window, NULL); 
	gdk_flush();		
}

/* Get bookmarks view (for other views) */
GtkWidget *
gpdf_control_private_get_bookmarks_view (GPdfControl * control)
{
	GPdfControlPrivate *priv;
	GtkWidget *w = NULL;

	g_return_val_if_fail (GPDF_IS_NON_NULL_CONTROL (control),
			      NULL);
	priv = control->priv;

	if (GTK_IS_WIDGET (priv->gpdf_bookmarks_view))
	  w = gtk_widget_ref (priv->gpdf_bookmarks_view);

	return w;
}

/* Get thumbnails view (for other views) */
GtkWidget *
gpdf_control_private_get_thumbnails_view (GPdfControl * control)
{
	GPdfControlPrivate *priv;
	GtkWidget *w = NULL;

	g_return_val_if_fail (GPDF_IS_NON_NULL_CONTROL (control),
			      NULL);
	priv = control->priv;

	if (GTK_IS_WIDGET (priv->gpdf_thumbnails_view))
	  w = gtk_widget_ref (priv->gpdf_thumbnails_view);

	return w;
}

#ifdef USE_ANNOTS_VIEW
/* Get annotation view (for other views) */
GtkWidget *
gpdf_control_private_get_annots_view (GPdfControl * control)
{
	GPdfControlPrivate *priv;
	GtkWidget *w = NULL;

	g_return_val_if_fail (GPDF_IS_NON_NULL_CONTROL (control),
			      NULL);
	priv = control->priv;

	if (GTK_IS_WIDGET (priv->gpdf_annots_view))
	  w = gtk_widget_ref (priv->gpdf_annots_view);

	return w;
}
#endif

void
destroy_widget_unref_control (GtkWidget *widget, gint arg1, BonoboControl *control)
{
	g_return_if_fail (GTK_IS_WIDGET (widget));
	g_return_if_fail (BONOBO_IS_CONTROL (control));

	gtk_widget_destroy (widget);
	bonobo_object_unref (BONOBO_OBJECT (control));
}

/* Create a HIG compliant dialog, display it and
   wait until response */
void
gpdf_control_private_error_dialog (GPdfControl * control,
				   const gchar *header_text,
				   const gchar *body_text,
				   gboolean modal,
				   gboolean ref_parent)
{
	GtkWidget *dlg, *but;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));
	if (ref_parent)
		bonobo_object_ref (BONOBO_OBJECT (control));

	dlg = gpdf_hig_dialog_new (GTK_STOCK_DIALOG_ERROR,
				   header_text,
				   body_text,
				   modal);
	but = gtk_dialog_add_button (GTK_DIALOG (dlg),
				     GTK_STOCK_OK,
				     GTK_RESPONSE_OK);
	bonobo_control_set_transient_for (BONOBO_CONTROL (control),
					  GTK_WINDOW (dlg),
					  NULL);
	if (modal) {
		gtk_dialog_run (GTK_DIALOG (dlg));
		gtk_widget_destroy (dlg);
		if (ref_parent)
			bonobo_object_unref (BONOBO_OBJECT (control));
	} else {
		g_signal_connect (G_OBJECT (dlg), "response",
				  (ref_parent
				   ? G_CALLBACK (destroy_widget_unref_control)
				   : G_CALLBACK (gtk_widget_destroy)),
				  control);
		gtk_widget_show (dlg);
	}		
}

void
gpdf_control_private_warn_dialog (GPdfControl * control,
				  const gchar *header_text,
				  const gchar *body_text,
				  gboolean modal)
{
	GtkWidget *dlg, *but;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));
	dlg = gpdf_hig_dialog_new (GTK_STOCK_DIALOG_WARNING,
				   header_text,
				   body_text,
				   modal);
	but = gtk_dialog_add_button (GTK_DIALOG (dlg),
				     GTK_STOCK_OK,
				     GTK_RESPONSE_OK);
	bonobo_control_set_transient_for (BONOBO_CONTROL (control),
					  GTK_WINDOW (dlg),
					  NULL);
	if (modal) {
		gtk_dialog_run (GTK_DIALOG (dlg));
		gtk_widget_destroy (dlg);
	} else {
		g_signal_connect (G_OBJECT (dlg), "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);
		gtk_widget_show (dlg);
	}
}

void
gpdf_control_private_info_dialog (GPdfControl * control,
				  const gchar *header_text,
				  const gchar *body_text,
				  gboolean modal)
{
	GtkWidget *dlg, *but;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));
	dlg = gpdf_hig_dialog_new (GTK_STOCK_DIALOG_INFO,
				   header_text,
				   body_text,
				   modal);
	but = gtk_dialog_add_button (GTK_DIALOG (dlg),
				     GTK_STOCK_OK,
				     GTK_RESPONSE_OK);
	bonobo_control_set_transient_for (BONOBO_CONTROL (control),
					  GTK_WINDOW (dlg),
					  NULL);
	if (modal) {
		gtk_dialog_run (GTK_DIALOG (dlg));
		gtk_widget_destroy (dlg);
	} else {
		g_signal_connect (G_OBJECT (dlg), "response",
				  G_CALLBACK (gtk_widget_destroy), NULL);
		gtk_widget_show (dlg);		
	}
}

void
gpdf_control_private_display_help (BonoboControl *control,
				   const gchar *section_id)
{
	GPdfControl *gpdf_control = GPDF_CONTROL (control);
	GPdfControlPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (gpdf_control));
	
	priv = gpdf_control->priv;
	
	if (gnome_help_display_desktop (gnome_program_get (),
				        GPDF_GNOME_HELP_DOC_ID,
					GPDF_GNOME_HELP_FILENAME,
					section_id,
					NULL) == FALSE)
	{
		const gchar *msg = g_strdup_printf ("Cannot access section \"%s\"",
						    section_id);
		gpdf_control_private_error_dialog (gpdf_control,
						   _("Documentation Error"),
						   msg,
						   FALSE,
						   FALSE);
	}
}


typedef struct _GPdfPrintJob GPdfPrintJob;

struct _GPdfPrintJob {
	PDFDoc *pdf_doc;
	GnomePrintJob *gnome_print_job;
	gint page_start;
	gint page_end;
	gint page_next;

	guint idle_id;
	gchar *file_name;
	PSOutputDev *ps_output_dev;

	GtkWidget *progress_dialog;
	GtkProgressBar *progress_bar;
	GtkLabel *progress_label;
	GtkWidget *progress_button;
};

/* FIXME: Does these 6 following funcs could be grouped in only 2? */
static void
gpdf_control_update_bookmarks_view_tools_menu (GPdfControl *gpdf_control)
{
	GPdfControlPrivate *priv;
	GtkWidget *toolsmenu, *old; 

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (gpdf_control));
	priv = gpdf_control->priv;

	toolsmenu =
		gpdf_bookmarks_view_get_tools_menu (
			GPDF_BOOKMARKS_VIEW (priv->gpdf_bookmarks_view));
	old = gpdf_sidebar_set_page_tools_menu (GPDF_SIDEBAR (priv->gpdf_sidebar),
						GPDF_BOOKMARKS_VIEW_PAGE_ID, 
						toolsmenu);
	if (GTK_IS_MENU (old) && old != toolsmenu) gtk_widget_destroy (old); 
}

static void
gpdf_control_bookmarks_view_ready (GtkWidget *w, GPdfControl *gpdf_control)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (gpdf_control));
	gpdf_control_update_bookmarks_view_tools_menu (gpdf_control);
}

static void
gpdf_control_update_thumbnails_view_tools_menu (GPdfControl *gpdf_control)
{
	GPdfControlPrivate *priv;
	GtkWidget *toolsmenu, *old; 

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (gpdf_control));
	priv = gpdf_control->priv;

	toolsmenu =
		gpdf_thumbnails_view_get_tools_menu (
			GPDF_THUMBNAILS_VIEW (priv->gpdf_thumbnails_view));
	old = gpdf_sidebar_set_page_tools_menu (GPDF_SIDEBAR (priv->gpdf_sidebar),
						GPDF_THUMBNAILS_VIEW_PAGE_ID, 
						toolsmenu); 
	if (GTK_IS_MENU (old) && old != toolsmenu) gtk_widget_destroy (old); 
}

static void
gpdf_control_thumbnails_view_ready (GtkWidget *w, GPdfControl *gpdf_control)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (gpdf_control));
	gpdf_control_update_thumbnails_view_tools_menu (gpdf_control);
}

#ifdef USE_ANNOTS_VIEW
static void
gpdf_control_update_annots_view_tools_menu (GPdfControl *gpdf_control)
{
	GPdfControlPrivate *priv;
	GtkWidget *toolsmenu, *old; 

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (gpdf_control));
	priv = gpdf_control->priv;

	toolsmenu =
		gpdf_annots_view_get_tools_menu (
			GPDF_ANNOTS_VIEW (priv->gpdf_annots_view));
	old = gpdf_sidebar_set_page_tools_menu (GPDF_SIDEBAR (priv->gpdf_sidebar),
						GPDF_ANNOTS_VIEW_PAGE_ID, 
						toolsmenu); 
	if (GTK_IS_MENU (old) && old != toolsmenu) gtk_widget_destroy (old); 	
}

static void
gpdf_control_annots_view_ready (GtkWidget *w, GPdfControl *gpdf_control)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (gpdf_control));
	gpdf_control_update_annots_view_tools_menu (gpdf_control);
}
#endif

static void
ui_component_set_sensitive (BonoboUIComponent *ui,
			    const char *path,
			    gboolean sensitive)
{
	bonobo_ui_component_set_prop (ui, path, "sensitive",
				      sensitive ? "1" : "0", NULL);
}

static void
gpdf_control_enable_ui (GPdfControl *control, gboolean sensitive)
{
	BonoboUIComponent *uic;
	GPdfView *gpdf_view;
	gboolean sens_print, sens_go_back, sens_go_fwd, sens_zoom;

	if (!control->priv->activated)
		return;

	uic = bonobo_control_get_ui_component (BONOBO_CONTROL (control));

	bonobo_ui_component_freeze (uic, NULL);

	sens_print = (sensitive
		      && control->priv->pdf_doc
		      && control->priv->pdf_doc->okToPrint ());
	ui_component_set_sensitive (uic, FILE_PRINT_CMDPATH, sens_print);
	ui_component_set_sensitive (uic, FILE_PROPERTIES_CMDPATH, sensitive);
	ui_component_set_sensitive (uic, FILE_SAVEAS_CMDPATH, sensitive);

	gpdf_view = GPDF_VIEW (control->priv->gpdf_view);
	ui_component_set_sensitive (
		uic, GO_HISTORY_PREV_CMDPATH,
		sensitive && !gpdf_view_is_first_history (gpdf_view));
	ui_component_set_sensitive (
		uic, GO_HISTORY_NEXT_CMDPATH,
		sensitive && !gpdf_view_is_last_history (gpdf_view));

	sens_go_fwd = (sensitive && !gpdf_view_is_last_page (gpdf_view));
	sens_go_back = (sensitive && !gpdf_view_is_first_page (gpdf_view));
	ui_component_set_sensitive (uic, GO_PAGE_PREV_CMDPATH, sens_go_back);
	ui_component_set_sensitive (uic, GO_PAGE_NEXT_CMDPATH, sens_go_fwd);
	ui_component_set_sensitive (uic, GO_PAGE_FIRST_CMDPATH, sens_go_back);
	ui_component_set_sensitive (uic, GO_PAGE_LAST_CMDPATH, sens_go_fwd);

	sens_zoom = sensitive && !control->priv->has_zoomable_frame;
	ui_component_set_sensitive (uic, ZOOM_IN_CMDPATH, sens_zoom);
	ui_component_set_sensitive (uic, ZOOM_OUT_CMDPATH, sens_zoom);
	ui_component_set_sensitive (uic, ZOOM_FIT_CMDPATH, sens_zoom);
	ui_component_set_sensitive (uic, ZOOM_FIT_WIDTH_CMDPATH, sensitive);

	gpdf_control_update_bookmarks_view_tools_menu (control);
	gpdf_control_update_thumbnails_view_tools_menu (control);
#ifdef USE_ANNOTS_VIEW
	gpdf_control_update_annots_view_tools_menu (control);
#endif

	bonobo_ui_component_thaw (uic, NULL);	
}

static void
gpdf_print_job_prepare (GPdfPrintJob *pj)
{
	GnomePrintConfig *config;
	gdouble width, height;
	gboolean duplex;

	config = gnome_print_job_get_config (pj->gnome_print_job);

	/* width and height are returned in PS points :-) */
	gnome_print_config_get_page_size (config, &width, &height);
	gnome_print_config_get_boolean (config, 
					(const guchar *)GNOME_PRINT_KEY_DUPLEX,
					&duplex);

	globalParams->setPSPaperWidth ((int)width);
	globalParams->setPSPaperHeight ((int)height);
	globalParams->setPSDuplex (duplex);
}

static gboolean
idle_print_handler (GPdfPrintJob *pj)
{
	if (pj->ps_output_dev == NULL) {
		pj->ps_output_dev = new PSOutputDev (pj->file_name,
						     pj->pdf_doc->getXRef (),
						     pj->pdf_doc->getCatalog (),
						     pj->page_start,
						     pj->page_end,
						     psModePS);

		gtk_widget_set_sensitive (pj->progress_button, TRUE);
		pj->page_next = pj->page_start;
		return TRUE;
	}

	if (pj->page_next <= pj->page_end) {
		gchar *label;

		label = g_strdup_printf (_("Printing: Page %d."), pj->page_next);
		gtk_label_set_text (pj->progress_label, label);
		g_free (label);

		gtk_progress_bar_set_fraction (
			pj->progress_bar,
			((double)pj->page_next - pj->page_start + 1) /
			((double)pj->page_end - pj->page_start + 1));

		pj->pdf_doc->displayPage (pj->ps_output_dev,
					  pj->page_next,
					  72.0, 72.0, 0, gTrue, gFalse);
		pj->page_next++;
		return TRUE;
	} else {
		pj->idle_id = 0;
		delete pj->ps_output_dev;
		pj->ps_output_dev = NULL;
		gtk_dialog_response (GTK_DIALOG (pj->progress_dialog),
				     GTK_RESPONSE_OK);
		return FALSE; /* Remove this idle source */
	}
}

void
gpdf_control_print_job_print (GPdfControl *control, GPdfPrintJob *pj)
{
	gint fd;
	GladeXML *progress_xml;
	GSource *idle_source;
	gint response;

	bonobo_object_ref (BONOBO_OBJECT (control)); /* Keep the pdf-doc alive */
	fd = g_file_open_tmp ("gpdf-print.ps.XXXXXX", &pj->file_name, NULL);
	if (fd <= -1)
		return;

	gnome_print_job_set_file (pj->gnome_print_job, pj->file_name);

	gpdf_print_job_prepare (pj);
	
	progress_xml = glade_xml_new (
		DATADIR "/gpdf/glade/gpdf-print-progress-dialog.glade",
		NULL, NULL);
	glade_xml_signal_autoconnect (progress_xml);
	pj->progress_dialog = 
		glade_xml_get_widget (progress_xml, "print-progress-dialog");
	pj->progress_bar =
		GTK_PROGRESS_BAR (glade_xml_get_widget (progress_xml,
							"progressbar"));
	pj->progress_label =
		GTK_LABEL (glade_xml_get_widget (progress_xml,
						 "status-label"));

	pj->progress_button = glade_xml_get_widget (progress_xml,
						    "cancel-button");
	g_object_unref (G_OBJECT (progress_xml));

	bonobo_control_set_transient_for (BONOBO_CONTROL (control),
					  GTK_WINDOW (pj->progress_dialog),
					  NULL);

	idle_source = g_idle_source_new ();
	g_source_set_priority (idle_source, G_PRIORITY_DEFAULT_IDLE);
	g_source_set_closure (idle_source,
			      g_cclosure_new (G_CALLBACK (idle_print_handler),
					      pj, NULL));
	pj->idle_id = g_source_attach (idle_source, NULL);
	g_source_unref (idle_source);
	
	response = gtk_dialog_run (GTK_DIALOG (pj->progress_dialog));

	gtk_widget_hide (pj->progress_dialog);
	gtk_widget_destroy (pj->progress_dialog);

	if (response == GTK_RESPONSE_OK) {
		gnome_print_job_print (pj->gnome_print_job);
	} else {
		g_assert (pj->idle_id);

		g_source_remove (pj->idle_id);
	}

	close (fd);
	unlink (pj->file_name);
	g_free (pj->file_name);
	g_object_unref (pj->gnome_print_job);
	bonobo_object_unref (BONOBO_CONTROL (control));
}

static gboolean
using_postscript_printer (GnomePrintConfig *config)
{
	const guchar *driver;
	const guchar *transport;

	driver = gnome_print_config_get (
		config, (const guchar *)"Settings.Engine.Backend.Driver");
	
	transport = gnome_print_config_get (
		config, (const guchar *)"Settings.Transport.Backend");
	
	if (driver) {
		if (!strcmp ((const gchar *)driver, "gnome-print-ps"))
			return TRUE;
		else 
			return FALSE;
	} else 	if (transport) {
		if (!strcmp ((const gchar *)transport, "CUPS"))
			return TRUE;
	}
	
	return FALSE;
}

static void
gpdf_control_no_postscript_printer_alert (GPdfControl *control,
					  GnomePrintConfig *config)
{
	GtkWidget *dlg;
	const guchar *driver;
	gchar *secondary;

	driver = gnome_print_config_get (
		config, (const guchar *)"Settings.Engine.Backend.Driver");

	if (driver)
		secondary = g_strdup_printf (
			_("You were trying to print to a printer using the \"%s\" driver. This program requires a PostScript printer driver."),
			(const gchar *)driver);
	else
		secondary = g_strdup (_("You were trying to print to a printer that does not use PostScript. This program requires a PostScript printer driver."));

	gpdf_control_private_error_dialog (
		control,
		_("Printing is not supported on this printer."),
		secondary,
		FALSE,
		FALSE);

	g_free (secondary);
}

static void
gpdf_control_get_range_page (GPdfControl *control, GnomePrintDialog *dialog,
			     gint *start, gint *end)
{
	switch (gnome_print_dialog_get_range (dialog)) {
	case GNOME_PRINT_RANGE_ALL:
		*start = 1;
		*end = control->priv->pdf_doc->getNumPages ();
		break;
	case GNOME_PRINT_RANGE_RANGE:
		gnome_print_dialog_get_range_page (dialog, start, end);
		break;
	case GNOME_PRINT_RANGE_CURRENT:
		g_warning ("Cannot happen: GNOME_PRINT_RANGE_CURRENT");
		break;
	default:
		g_warning ("Unknown page range type");
		break;
	}
}

static GtkWidget *
gpdf_control_print_dialog_new (GPdfControl *control, GnomePrintJob *job)
{
	GtkWidget *dialog;
	gchar *pages_label;

	dialog = gnome_print_dialog_new (job,
					 (const guchar*) _("Print"),
					 GNOME_PRINT_DIALOG_RANGE);

	pages_label = g_strconcat (_("_Pages"), " ", NULL);
	gnome_print_dialog_construct_range_page (
		GNOME_PRINT_DIALOG (dialog),
		/* (GNOME_PRINT_RANGE_CURRENT | would be the default -> very bad */
		(GNOME_PRINT_RANGE_ALL | GNOME_PRINT_RANGE_RANGE),
		1, control->priv->pdf_doc->getNumPages (),
		NULL, (const guchar *)pages_label);
	g_free (pages_label);

	gtk_dialog_set_response_sensitive (GTK_DIALOG(dialog),
					   GNOME_PRINT_DIALOG_RESPONSE_PREVIEW,
					   FALSE);

	bonobo_control_set_transient_for (BONOBO_CONTROL (control),
					  GTK_WINDOW (dialog),
					  NULL);

	return dialog;
}

static void /* FIXME */
print_to_file_workaround (GnomePrintJob *gpj)
{
	const guchar *backend_filename;

	backend_filename = gnome_print_config_get (
		gnome_print_job_get_config (gpj),
		(const guchar*)"Settings.Transport.Backend.FileName");

	if (backend_filename)
		gnome_print_config_set (
			gnome_print_job_get_config (gpj),
			(const guchar*)"Settings.Output.Job.FileName",
			backend_filename);
}

void
gpdf_control_print (GPdfControl *control)
{
	GPdfPrintJob *pj;
	GnomePrintConfig *config;
	GnomePrintJob *job;
	GPdfControlPrivate *priv;
	gint response;
	GdkWindow *view_window;
	GdkDisplay *display;
	GdkCursor *busy_cursor;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));

	gpdf_control_enable_ui (control, FALSE);
	config = gnome_print_config_default ();
	job = gnome_print_job_new (config);

	priv = control->priv;
	
	priv->print_dialog = gpdf_control_print_dialog_new (control, job);
	response = gtk_dialog_run (GTK_DIALOG (priv->print_dialog));
	gtk_widget_hide (priv->print_dialog);

	if (response != GNOME_PRINT_DIALOG_RESPONSE_PRINT)
		goto exit;

	if (!using_postscript_printer (config)) {
		gpdf_control_no_postscript_printer_alert (control, config);
		goto exit;
	}

	pj = g_new0 (GPdfPrintJob, 1);

	gpdf_control_get_range_page (
		control, GNOME_PRINT_DIALOG (priv->print_dialog),
		&pj->page_start, &pj->page_end);

	gtk_widget_destroy (priv->print_dialog);
	priv->print_dialog = NULL;

	print_to_file_workaround (job);

	pj->pdf_doc = control->priv->pdf_doc;
	pj->gnome_print_job = job;

	view_window = GTK_WIDGET (control->priv->gpdf_view)->window;
	display = gtk_widget_get_display (GTK_WIDGET (control->priv->gpdf_view));
	busy_cursor = gdk_cursor_new_for_display (display, GDK_WATCH);
	gdk_window_set_cursor (view_window, busy_cursor);
	gdk_cursor_unref (busy_cursor);
	gdk_flush ();
	gpdf_control_print_job_print (control, pj);
	g_free (pj);
	if (GDK_IS_WINDOW (view_window))
		gdk_window_set_cursor (view_window, NULL);
exit:
	gpdf_control_enable_ui (control, TRUE);
}

static gboolean
overwrite_existing_file (GtkWindow *window, const gchar *file_name)
{
	GtkWidget *msgbox;
	gchar *utf8_file_name;
	gchar *msg;
	AtkObject *obj;
	gint ret;

	utf8_file_name = g_filename_to_utf8 (file_name, -1, NULL, NULL, NULL);
	msg = g_strdup_printf (_("A file named \"%s\" already exists."),
			       utf8_file_name);
	g_free (utf8_file_name);

	msgbox = gpdf_hig_dialog_new (
		GTK_STOCK_DIALOG_QUESTION, msg,
		_("Do you want to replace it with the one you are saving?"),
		FALSE);

	g_free (msg);

	gtk_dialog_add_button (GTK_DIALOG (msgbox), 
			       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);

	gtk_dialog_add_button (GTK_DIALOG (msgbox),
			       _("_Replace"), GTK_RESPONSE_YES);

	gtk_window_set_transient_for (GTK_WINDOW (msgbox), window);

	gtk_dialog_set_default_response (GTK_DIALOG (msgbox),
					 GTK_RESPONSE_CANCEL);

	obj = gtk_widget_get_accessible (msgbox);

	if (GTK_IS_ACCESSIBLE (obj))
		atk_object_set_name (obj, _("Question"));

	ret = gtk_dialog_run (GTK_DIALOG (msgbox));
	gtk_widget_destroy (msgbox);

	return (ret == GTK_RESPONSE_YES);
}

static void
save_error_dialog (GtkWindow *window, const gchar *file_name)
{
	char *msg;
	GtkWidget *error_dialog;

	msg = g_strdup_printf (_("The file could not be saved as \"%s\"."),
			       file_name);

	error_dialog = gpdf_hig_dialog_new (GTK_STOCK_DIALOG_ERROR,
					    msg, NULL, FALSE);

	g_free (msg);

	gtk_dialog_add_button (GTK_DIALOG (error_dialog),
			       GTK_STOCK_CLOSE,
			       GTK_RESPONSE_CLOSE);

	gtk_window_set_transient_for (GTK_WINDOW (error_dialog), window);

	gtk_dialog_run (GTK_DIALOG (error_dialog));
	gtk_widget_destroy (error_dialog);
}

static void
gpdf_control_save_file (BonoboControl *control)
{
	GPdfControlPrivate *priv;
	GtkFileFilter *pdf_filter, *all_filter;
	gchar *pathname = NULL; 

	priv = GPDF_CONTROL (control)->priv;
	priv->file_chooser = gtk_file_chooser_dialog_new (
		_("Save a Copy"),
		NULL, GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_OK,
		NULL);
	gtk_window_set_modal (GTK_WINDOW (priv->file_chooser), TRUE);

	pdf_filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (pdf_filter, _("PDF Documents"));
	gtk_file_filter_add_mime_type (pdf_filter, "application/pdf");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (priv->file_chooser),
				     pdf_filter);

	all_filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (all_filter, _("All Files"));
	gtk_file_filter_add_pattern (all_filter, "*");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (priv->file_chooser),
				     all_filter);
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (priv->file_chooser),
				     pdf_filter);
	
	gtk_dialog_set_default_response (GTK_DIALOG (priv->file_chooser),
					 GTK_RESPONSE_OK);

	bonobo_control_set_transient_for (control,
					  GTK_WINDOW (priv->file_chooser),
					  NULL);
	gtk_widget_show (priv->file_chooser);

	while (gtk_dialog_run (GTK_DIALOG (priv->file_chooser))
	       == GTK_RESPONSE_OK) {

		pathname = gtk_file_chooser_get_filename (
			GTK_FILE_CHOOSER (priv->file_chooser));

		if (g_file_test (pathname, G_FILE_TEST_EXISTS) &&  
		    !overwrite_existing_file (GTK_WINDOW (priv->file_chooser),
					      pathname))
				continue;

		if (gpdf_view_save_as (GPDF_VIEW (priv->gpdf_view), pathname))
			break;
		else
			save_error_dialog (GTK_WINDOW (priv->file_chooser),
					   pathname);    
	}

	gtk_widget_destroy (priv->file_chooser);
	priv->file_chooser = NULL;
}

static void
verb_SaveAs_cb (BonoboUIComponent *uic, gpointer user_data,
		const char *cname)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (user_data));

	gpdf_control_save_file (BONOBO_CONTROL (user_data));
}

static void
properties_response_handler (GtkDialog *dialog, gint response, gpointer data)
{
	gtk_widget_destroy (GTK_WIDGET (dialog));
	GPDF_CONTROL (data)->priv->properties = NULL;
}

static gint
properties_delete_handler (GtkWidget *widget, GdkEventAny *even,
			   gpointer data)
{
	GPDF_CONTROL (data)->priv->properties = NULL;
	return FALSE; /* destroy it in the default handler */
}

static void
verb_FileProperties_cb (BonoboUIComponent *uic, gpointer user_data,
			const char *cname)
{
	GPdfControlPrivate *priv;
	GtkWidget *props;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (user_data));

	priv = GPDF_CONTROL (user_data)->priv;

	priv->properties = gtk_dialog_new_with_buttons (_("PDF Properties"),
							NULL,
							GTK_DIALOG_NO_SEPARATOR,
							GTK_STOCK_CLOSE,
							GTK_RESPONSE_CLOSE,
							NULL);
	gtk_container_set_border_width (GTK_CONTAINER (priv->properties), 5);
	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (priv->properties)->vbox), 2);
	gtk_window_set_resizable (GTK_WINDOW (priv->properties), FALSE);

	g_signal_connect (priv->properties, "response",
			  G_CALLBACK (properties_response_handler), user_data);
	g_signal_connect (priv->properties, "delete-event",
			  G_CALLBACK (properties_delete_handler), user_data);
	bonobo_control_set_transient_for (BONOBO_CONTROL (user_data),
					  GTK_WINDOW (priv->properties), NULL);

	props = GTK_WIDGET (g_object_new (GPDF_TYPE_PROPERTIES_DISPLAY, NULL));
	pdf_doc_process_properties (
		GPDF_CONTROL (user_data)->priv->pdf_doc,
		G_OBJECT (props));

	gtk_widget_show (props);
	gtk_box_pack_start_defaults (
		GTK_BOX (GTK_DIALOG (priv->properties)->vbox), props);
	gtk_widget_show (priv->properties);
}

static void
verb_FilePrint_cb (BonoboUIComponent *uic, gpointer user_data,
		   const char *cname)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (user_data));

	gpdf_control_print (GPDF_CONTROL (user_data));
}


static void
verb_GoHistoryPrev_cb (BonoboUIComponent *uic, gpointer user_data,
		       const char *cname)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (user_data));

	gpdf_view_back_history (
		GPDF_VIEW (GPDF_CONTROL (user_data)->priv->gpdf_view));
}

static void
verb_GoHistoryNext_cb (BonoboUIComponent *uic, gpointer user_data,
		       const char *cname)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (user_data));

	gpdf_view_forward_history (
		GPDF_VIEW (GPDF_CONTROL (user_data)->priv->gpdf_view));
}

static void
verb_GoPagePrev_cb (BonoboUIComponent *uic, gpointer user_data, const char*cname)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (user_data));

	gpdf_view_page_prev (
		GPDF_VIEW (GPDF_CONTROL (user_data)->priv->gpdf_view));
}

static void
verb_GoPageNext_cb (BonoboUIComponent *uic, gpointer user_data, const char*cname)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (user_data));

	gpdf_view_page_next (
		GPDF_VIEW (GPDF_CONTROL (user_data)->priv->gpdf_view));
}

static void
verb_GoPageFirst_cb (BonoboUIComponent *uic, gpointer user_data, const char*cname)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (user_data));

	gpdf_view_page_first (
		GPDF_VIEW (GPDF_CONTROL (user_data)->priv->gpdf_view));
}

static void
verb_GoPageLast_cb (BonoboUIComponent *uic, gpointer user_data, const char*cname)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (user_data));

	gpdf_view_page_last (
		GPDF_VIEW (GPDF_CONTROL (user_data)->priv->gpdf_view));
}

static void
gc_page_changed_enable_page_buttons (GPdfView *gpdf_view, gint page,
				     GPdfControl *control)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));

	gpdf_control_enable_ui (control, TRUE);
}



#define MAGSTEP   1.2
#define MAGSTEP2  MAGSTEP * MAGSTEP
#define MAGSTEP4  MAGSTEP2 * MAGSTEP2
#define IMAGSTEP  0.8333333333
#define IMAGSTEP2 IMAGSTEP * IMAGSTEP
#define IMAGSTEP4 IMAGSTEP2 * IMAGSTEP2

static float preferred_zoom_levels [] = {
	IMAGSTEP4 * IMAGSTEP4, IMAGSTEP4 * IMAGSTEP2 * IMAGSTEP,
	IMAGSTEP4 * IMAGSTEP2, IMAGSTEP4 * IMAGSTEP, IMAGSTEP4,
	IMAGSTEP2 * IMAGSTEP, IMAGSTEP2, IMAGSTEP,
	1.0,
	MAGSTEP, MAGSTEP2, MAGSTEP2 * MAGSTEP, MAGSTEP4,
	MAGSTEP4 * MAGSTEP, MAGSTEP4 * MAGSTEP2, MAGSTEP4 * MAGSTEP2 * MAGSTEP,
	MAGSTEP4 * MAGSTEP4
};

static const gchar *preferred_zoom_level_names [] = {
	"-8", "-7", "-6", "-5", "-4", "-3", "-2", "-1",
	"0",
	"+1", "+2", "+3", "+4", "+5", "+6", "+7", "+8"
};

static const gint n_zoom_levels = (sizeof (preferred_zoom_levels) /
				   sizeof (float));

void
gpdf_control_zoom_in (GPdfControl *gpdf_control)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (gpdf_control));

	gpdf_view_zoom_in (GPDF_VIEW (gpdf_control->priv->gpdf_view));
}

void
gpdf_control_zoom_out (GPdfControl *gpdf_control)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (gpdf_control));

	gpdf_view_zoom_out (GPDF_VIEW (gpdf_control->priv->gpdf_view));
}

/*
 * Idle handler to reset the scrollbar policy
 */
static gboolean
gc_set_policy_idle_cb (gpointer data)
{
	GPdfControlPrivate *priv;

	g_return_val_if_fail (GPDF_IS_NON_NULL_CONTROL (data), FALSE);

	priv = GPDF_CONTROL (data)->priv;

	priv->idle_id = 0;

	gtk_scrolled_window_set_policy (
		GTK_SCROLLED_WINDOW (priv->scrolled_window),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	return FALSE;
}

void
gpdf_control_zoom_fit_width (GPdfControl *gpdf_control)
{
	GPdfControlPrivate *priv;
	GtkScrolledWindow *scrolled_window;
	int w;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (gpdf_control));

	priv = gpdf_control->priv;

	scrolled_window = GTK_SCROLLED_WINDOW (priv->scrolled_window);

	w = GTK_WIDGET (scrolled_window)->allocation.width;
	/* FIXME this doesn't look right to me */
	w -= GTK_WIDGET (scrolled_window->vscrollbar)->allocation.width;
	w -= 2 * GTK_WIDGET (scrolled_window)->style->xthickness;

	gtk_scrolled_window_set_policy (scrolled_window,
					GTK_POLICY_NEVER,
					GTK_POLICY_AUTOMATIC);

	gpdf_view_zoom_fit_width (GPDF_VIEW (priv->gpdf_view), w);

	if (!priv->idle_id)
		priv->idle_id = g_idle_add (gc_set_policy_idle_cb,
					    gpdf_control);
}

void
gpdf_control_zoom_fit (GPdfControl *gpdf_control)
{
	GPdfControlPrivate *priv;
	GtkScrolledWindow *scrolled_window;
	int h, w;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (gpdf_control));

	priv = gpdf_control->priv;

	scrolled_window = GTK_SCROLLED_WINDOW (priv->scrolled_window);

	h = GTK_WIDGET (scrolled_window)->allocation.height;
	h -= 2 * GTK_WIDGET (scrolled_window)->style->ythickness;

	w = GTK_WIDGET (scrolled_window)->allocation.width;
	w -= 2 * GTK_WIDGET (scrolled_window)->style->xthickness;

	gtk_scrolled_window_set_policy (scrolled_window,
					GTK_POLICY_NEVER,
					GTK_POLICY_NEVER);

	gpdf_view_zoom_fit (GPDF_VIEW (priv->gpdf_view), w, h);

	if (!priv->idle_id)
		priv->idle_id = g_idle_add (gc_set_policy_idle_cb,
					    gpdf_control);
}

void
gpdf_control_zoom_default (GPdfControl *gpdf_control)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (gpdf_control));

	gpdf_view_zoom_default (GPDF_VIEW (gpdf_control->priv->gpdf_view));
}

static void
verb_ZoomIn_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	gpdf_control_zoom_in (GPDF_CONTROL (user_data));
}

static void
verb_ZoomOut_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	gpdf_control_zoom_out (GPDF_CONTROL (user_data));
}

static void
verb_ZoomFit_cb (BonoboUIComponent *uic, gpointer user_data, const char *cname)
{
	gpdf_control_zoom_fit (GPDF_CONTROL (user_data));
}

static void
verb_ZoomFitWidth_cb (BonoboUIComponent *uic, gpointer user_data,
		      const char *cname)
{
	gpdf_control_zoom_fit_width (GPDF_CONTROL (user_data));
}

static void
gc_zoom_in_cb (GtkObject *source, gpointer data)
{
	gpdf_control_zoom_in (GPDF_CONTROL (data));
}

static void
gc_zoom_out_cb (GtkObject *source, gpointer data)
{
	gpdf_control_zoom_out (GPDF_CONTROL (data));
}

static void
gc_zoom_to_fit_cb (GtkObject *source, gpointer data)
{
	gpdf_control_zoom_fit (GPDF_CONTROL (data));
}

static void
gc_zoom_to_default_cb (GtkObject *source, gpointer data)
{
	gpdf_control_zoom_default (GPDF_CONTROL (data));
}

#define GDOUBLE_FROM_CORBA_FLOAT(val) (gdouble)val
#define CORBA_FLOAT_FROM_GDOUBLE(val) (CORBA_float)val
static void
gc_zoomable_set_zoom_level_cb (BonoboZoomable *zoomable,
			       CORBA_float new_zoom_level,
			       GPdfControl *control)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));

	gpdf_view_zoom (GPDF_VIEW (control->priv->gpdf_view),
			GDOUBLE_FROM_CORBA_FLOAT (new_zoom_level),
			FALSE);
}

static void
gc_zoomable_set_frame_cb (BonoboZoomable *zoomable, GPdfControl *control)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));

	control->priv->has_zoomable_frame = TRUE;
}


static void
gc_zoom_changed_report_to_zoomable (GPdfView *gpdf_view, gdouble zoom,
				    GPdfControl *control)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));

	bonobo_zoomable_report_zoom_level_changed (
		control->priv->zoomable,
		CORBA_FLOAT_FROM_GDOUBLE (zoom),
		NULL);
}

static void
gc_set_page_cb (GPdfPageControl *page_control, gint page, GPdfControl *control)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));

	gpdf_view_goto_page (GPDF_VIEW (control->priv->gpdf_view), page);
	gpdf_view_scroll_to_top (GPDF_VIEW (control->priv->gpdf_view));
	gtk_widget_grab_focus (GTK_WIDGET (control->priv->gpdf_view));
}

static void
gc_page_changed_update_page_control (GPdfView *gpdf_view, gint page,
				     GPdfControl *control)
{
	GPdfPageControl *pc;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));

	pc = GPDF_PAGE_CONTROL (
		bonobo_control_get_widget (control->priv->bonobo_page_control));
	gpdf_page_control_set_page (pc, page);
}

static void
ui_component_set_hidden (BonoboUIComponent *ui,
			 const char *path,
			 gboolean hidden)
{
	bonobo_ui_component_set_prop (ui, path, "hidden",
				      hidden ? "1" : "0", NULL);
}

static void
gc_set_zoom_items_visibility (GPdfControl *control, gboolean visible)
{
	BonoboUIComponent *ui;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));

	ui = bonobo_control_get_ui_component (BONOBO_CONTROL (control));
	if (ui == NULL)
		return;

	ui_component_set_hidden (ui, ZOOM_IN_CMDPATH, !visible);
	ui_component_set_hidden (ui, ZOOM_OUT_CMDPATH, !visible);
	ui_component_set_hidden (ui, ZOOM_FIT_CMDPATH, !visible);
}


static BonoboUIVerb gc_verbs [] = {
	BONOBO_UI_VERB ("FileSaveAs", 	  verb_SaveAs_cb),
	BONOBO_UI_VERB ("FileProperties", verb_FileProperties_cb),
	BONOBO_UI_VERB ("FilePrint",      verb_FilePrint_cb),

	BONOBO_UI_VERB ("GoHistoryPrev",  verb_GoHistoryPrev_cb),
	BONOBO_UI_VERB ("GoHistoryNext",  verb_GoHistoryNext_cb),

	BONOBO_UI_VERB ("GoPagePrev",     verb_GoPagePrev_cb),
	BONOBO_UI_VERB ("GoPageNext",     verb_GoPageNext_cb),
	BONOBO_UI_VERB ("GoPageFirst",    verb_GoPageFirst_cb),
	BONOBO_UI_VERB ("GoPageLast",     verb_GoPageLast_cb),

	BONOBO_UI_VERB ("ZoomIn",         verb_ZoomIn_cb),
	BONOBO_UI_VERB ("ZoomOut",        verb_ZoomOut_cb),
	BONOBO_UI_VERB ("ZoomFit",        verb_ZoomFit_cb),
	BONOBO_UI_VERB ("ZoomFitWidth",   verb_ZoomFitWidth_cb),

	BONOBO_UI_VERB_END
};

static void
gc_ui_set_menu_toolbar_pixbufs (GtkWidget *w,
				BonoboUIComponent *ui_component,
				const gchar *toolbar_comp_path, 
				const gchar *menu_comp_path, 
				const gchar *icon_id)
{
	GtkIconSet *icon_set;
	GdkPixbuf *pixbuf, *menu_pixbuf;	

	if (ui_component == NULL)
		return;

	icon_set = gtk_style_lookup_icon_set (gtk_widget_get_style (w),
					      icon_id);
	if (icon_set == NULL)
		return;

	pixbuf = gtk_icon_set_render_icon (icon_set,
					   gtk_widget_get_style (w),
					   gtk_widget_get_direction (w),
					   GTK_STATE_NORMAL,
					   GTK_ICON_SIZE_LARGE_TOOLBAR,
					   NULL, NULL);

	bonobo_ui_util_set_pixbuf (ui_component,
				   toolbar_comp_path,
				   pixbuf,
				   NULL);

	menu_pixbuf = gtk_icon_set_render_icon (
		icon_set,
		gtk_widget_get_style (w),
		gtk_widget_get_direction (w),
		GTK_STATE_NORMAL,
		GTK_ICON_SIZE_MENU,
		NULL, NULL);

	bonobo_ui_util_set_pixbuf (
		ui_component,
		menu_comp_path,
		menu_pixbuf,
		NULL);

	gdk_pixbuf_unref (pixbuf);	
}

static void
gc_ui_set_pixbufs (GPdfControl *control)
{
	GtkWidget *gpdf_view;
	BonoboUIComponent *ui_component;

	gpdf_view = control->priv->gpdf_view;

	ui_component =
		bonobo_control_get_ui_component (BONOBO_CONTROL (control));
	if (ui_component == NULL)
		return;

	/* Fit Width icon */
	gc_ui_set_menu_toolbar_pixbufs (gpdf_view,
					ui_component,
					TOOLBAR_FIT_WIDTH_PATH,
					MENU_FIT_WIDTH_PATH,
					GPDF_STOCK_ZOOM_FIT_WIDTH); 
}

static void
gc_ui_add_page_control (GPdfControl *control)
{
	BonoboControl *bonobo_page_control;
	BonoboUIComponent *ui_component;

	bonobo_page_control = control->priv->bonobo_page_control;

	ui_component =
		bonobo_control_get_ui_component (BONOBO_CONTROL (control));
	bonobo_ui_component_object_set (
		ui_component,
		TOOLBAR_PAGE_CONTROL_PATH,
		BONOBO_OBJREF (bonobo_page_control),
		NULL);
}

static void
gc_set_ui_container (GPdfControl *control, Bonobo_UIContainer ui_container)
{
	BonoboUIComponent *ui_component;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));
	g_return_if_fail (ui_container != CORBA_OBJECT_NIL);

	ui_component = bonobo_control_get_ui_component (
		BONOBO_CONTROL (control));
	g_assert (ui_component != NULL);

	bonobo_ui_component_set_container (ui_component, ui_container, NULL);
	bonobo_ui_component_add_verb_list_with_data (ui_component, gc_verbs,
						     control);

	bonobo_ui_util_set_ui (ui_component, DATADIR,
			       GPDF_UI_XML_FILE, "GPDF",
			       NULL);
	gc_ui_set_pixbufs (control);
	gc_ui_add_page_control (control);
	gc_page_changed_update_page_control (
		GPDF_VIEW (control->priv->gpdf_view), 1, control);
	gc_set_zoom_items_visibility (control,
				      !control->priv->has_zoomable_frame);
}

static void
gc_unset_ui_container (GPdfControl *control)
{
	BonoboUIComponent *ui_component;
	GPdfControlPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));

	ui_component = bonobo_control_get_ui_component (
	     BONOBO_CONTROL (control));
	g_assert (ui_component != NULL);

	bonobo_ui_component_unset_container (ui_component, NULL);

	priv = control->priv;
}

static void
gpdf_control_read_sidebar_state (GPdfControl *gpdf_control)
{
	const gchar *page_id = eel_gconf_get_string (CONF_WINDOWS_SIDEBAR_PAGE);
	
	/* Read sidebar state in gconf value show_sidebar */
	if (eel_gconf_get_boolean (CONF_WINDOWS_SHOW_SIDEBAR))
		gpdf_control_view_sidebar_changed_cb
			(bonobo_control_get_ui_component 
			     (BONOBO_CONTROL (gpdf_control)), 
			 VIEW_SIDEBAR_CMDPATH,
			 (Bonobo_UIComponent_EventType)BONOBO_TYPE_UI_COMPONENT,
			 NULL,
			 gpdf_control);

#ifndef USE_ANNOTS_VIEW
	/*
	 * If annots view is disabled and saved page id
	 * is GPDF_ANNOTS_VIEW_PAGE_ID,  change it to Bookmarks page.
	 */
	if (!strncmp(page_id, GPDF_ANNOTS_VIEW_PAGE_ID, strlen(GPDF_ANNOTS_VIEW_PAGE_ID)))
		page_id = GPDF_BOOKMARKS_VIEW_PAGE_ID;
#endif
	
	/* Select sidebar page saved in gconf value sidebar_page */
	gpdf_control_sidebar_page_changed_cb
		(GPDF_SIDEBAR (gpdf_control->priv->gpdf_sidebar),
		 (page_id && *page_id ? page_id : GPDF_BOOKMARKS_VIEW_PAGE_ID),
		 gpdf_control); 
}

static void
gpdf_control_save_sidebar_state (GPdfControl *gpdf_control)
{
	const gchar *cur_page_id = gpdf_sidebar_get_current_page_id
	      (GPDF_SIDEBAR (gpdf_control->priv->gpdf_sidebar));

	DBG (g_message ("gpdf_control_save_sidebar_state: opened = %s",
			GTK_WIDGET_VISIBLE (gpdf_control->priv->gpdf_sidebar) ?
			"TRUE" : "FALSE")); 
	DBG (g_message ("gpdf_control_save_sidebar_state: cur_view = '%s'",
			cur_page_id ? cur_page_id : GPDF_BOOKMARKS_VIEW_PAGE_ID)); 

	/* Save sidebar state */
	eel_gconf_set_boolean
		(CONF_WINDOWS_SHOW_SIDEBAR,
		 GTK_WIDGET_VISIBLE (gpdf_control->priv->gpdf_sidebar));
	
	/* And side bar current page id */
	eel_gconf_set_string
		(CONF_WINDOWS_SIDEBAR_PAGE,
		 cur_page_id ? cur_page_id : GPDF_BOOKMARKS_VIEW_PAGE_ID); 
}

static void
gpdf_control_activate_sidebar (BonoboControl *control, gboolean activate)
{
	GPdfControl *gpdf_control = GPDF_CONTROL (control);

	if (activate) {
		/* Add listener for side bar toggle */
		bonobo_ui_component_add_listener
		  (bonobo_control_get_ui_component (control),
		   VIEW_SIDEBAR_COMP_ID,
		   (BonoboUIListenerFn)gpdf_control_view_sidebar_changed_cb,
		   gpdf_control);

		gpdf_control_read_sidebar_state (gpdf_control); 
	}
	else
	    gpdf_control_save_sidebar_state (gpdf_control); 
}

static void
gpdf_control_activate (BonoboControl *control, gboolean activate)
{
	GPdfControl *gpdf_control = GPDF_CONTROL (control);

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));

	if (activate) {
		Bonobo_UIContainer ui_container =
			bonobo_control_get_remote_ui_container (control, NULL);

		if (ui_container != CORBA_OBJECT_NIL) {
			gc_set_ui_container (gpdf_control, ui_container);
			bonobo_object_release_unref (ui_container, NULL);
		}
	} else
	    gc_unset_ui_container (gpdf_control);

	gpdf_control_activate_sidebar (control, activate);
	
	gpdf_control->priv->activated = activate;

	gpdf_control_enable_ui (gpdf_control, gpdf_control->priv->pdf_doc != NULL);
	
	BONOBO_CALL_PARENT (
		BONOBO_CONTROL_CLASS, activate, (control, activate));
}

static GPdfBookmarksView *
gpdf_control_get_bookmarks_view (GPdfControl *gpdf_control)
{
	GPdfControlPrivate *priv;
	GPdfView *gpdf_view;

	priv = gpdf_control->priv;
	gpdf_view = GPDF_VIEW (priv->gpdf_view);

	/* If not existant, create & connect it */
	if (!priv->gpdf_bookmarks_view)
	{
	    priv->gpdf_bookmarks_view =
	      gpdf_bookmarks_view_new (gpdf_control, priv->gpdf_view);
	    g_object_ref (G_OBJECT (priv->gpdf_bookmarks_view)); 
	    gtk_widget_show (priv->gpdf_bookmarks_view);
	    
	    /* Connect to selection bookmark event to change page */
	    g_signal_connect (G_OBJECT (priv->gpdf_bookmarks_view),
			      "bookmark_selected",
			      G_CALLBACK (gpdf_control_bookmark_selected_cb),
			      gpdf_control);

	    g_signal_connect (G_OBJECT (priv->gpdf_bookmarks_view),
			      "ready",
			      G_CALLBACK (gpdf_control_bookmarks_view_ready),
			      gpdf_control);
	}

	return (GPDF_BOOKMARKS_VIEW (priv->gpdf_bookmarks_view));
}

static GPdfThumbnailsView *
gpdf_control_get_thumbnails_view (GPdfControl *gpdf_control)
{
	GPdfControlPrivate *priv;
	GPdfView *gpdf_view;

	priv = gpdf_control->priv;
	gpdf_view = GPDF_VIEW (priv->gpdf_view);

	/* If not existant, create & connect it */
	if (!priv->gpdf_thumbnails_view)
	{
	    priv->gpdf_thumbnails_view =
	      gpdf_thumbnails_view_new (gpdf_control, priv->gpdf_view);
	    g_object_ref (G_OBJECT (priv->gpdf_thumbnails_view)); 
	    gtk_widget_show (priv->gpdf_thumbnails_view);

	    /* Connect to selection thumbnail event to change page */
	    g_signal_connect (G_OBJECT (priv->gpdf_thumbnails_view),
			      "thumbnail_selected",
			      G_CALLBACK (gpdf_control_thumbnail_selected_cb),
			      gpdf_control);	    

	    g_signal_connect (G_OBJECT (priv->gpdf_thumbnails_view),
			      "ready",
			      G_CALLBACK (gpdf_control_thumbnails_view_ready),
			      gpdf_control);
	}

	return (GPDF_THUMBNAILS_VIEW (priv->gpdf_thumbnails_view));
}

#ifdef USE_ANNOTS_VIEW
static GPdfAnnotsView *
gpdf_control_get_annots_view (GPdfControl *gpdf_control)
{
	GPdfControlPrivate *priv;
	GPdfView *gpdf_view;

	priv = gpdf_control->priv;
	gpdf_view = GPDF_VIEW (priv->gpdf_view);

	/* If not existant, create & connect it */
	if (!priv->gpdf_annots_view)
	{
	    priv->gpdf_annots_view =
	      gpdf_annots_view_new (gpdf_control, priv->gpdf_view);
	    g_object_ref (G_OBJECT (priv->gpdf_annots_view)); 
	    gtk_widget_show (priv->gpdf_annots_view);

	    /* Connect to selection thumbnail event to change page */
	    g_signal_connect (G_OBJECT (priv->gpdf_annots_view),
			      "annotation_selected",
			      G_CALLBACK (gpdf_control_annotation_selected_cb),
			      gpdf_control);

	    /* Connect to selection thumbnail event to change page */
	    g_signal_connect (G_OBJECT (priv->gpdf_annots_view),
			      "annotation_toggled",
			      G_CALLBACK (gpdf_control_annotation_toggled_cb),
			      gpdf_control);

	    g_signal_connect (G_OBJECT (priv->gpdf_annots_view),
			      "ready",
			      G_CALLBACK (gpdf_control_annots_view_ready),
			      gpdf_control);
	}

	return (GPDF_ANNOTS_VIEW (priv->gpdf_annots_view));
}
#endif

static void
gpdf_control_pdf_doc_changed (GPdfControl *gpdf_control)
{
	GPdfControlPrivate *priv;
	GPdfView *gpdf_view;
	GPdfBookmarksView *gpdf_bookmarks_view;
	GPdfThumbnailsView *gpdf_thumbnails_view;
	GPdfAnnotsView *gpdf_annots_view;
	GPdfPageControl *page_control;

	priv = gpdf_control->priv;

	gpdf_view = GPDF_VIEW (priv->gpdf_view);

	gpdf_bookmarks_view = gpdf_control_get_bookmarks_view (gpdf_control); 
	gpdf_thumbnails_view = gpdf_control_get_thumbnails_view (gpdf_control);
#ifdef USE_ANNOTS_VIEW	
	gpdf_annots_view = gpdf_control_get_annots_view (gpdf_control); 
#endif
	
	page_control = GPDF_PAGE_CONTROL (
		bonobo_control_get_widget (priv->bonobo_page_control));
	
	if (priv->pdf_doc) {
		gpdf_view_set_pdf_doc (gpdf_view, priv->pdf_doc);
		gpdf_page_control_set_total_pages (
			page_control,
			priv->pdf_doc->getNumPages ());
		
		gpdf_bookmarks_view_set_pdf_doc (gpdf_bookmarks_view,
						 priv->pdf_doc);
		gpdf_thumbnails_view_set_pdf_doc (gpdf_thumbnails_view,
						  priv->pdf_doc);
#ifdef USE_ANNOTS_VIEW
		gpdf_annots_view_set_pdf_doc (gpdf_annots_view,
					      priv->pdf_doc);
#endif
	} else
	  gpdf_page_control_set_total_pages (page_control, -1);

	gpdf_control_enable_ui (gpdf_control,
				priv->pdf_doc != NULL ? TRUE : FALSE);
}

static void
persist_stream_set_pdf_cb (GPdfPersistStream *persist_stream,
			   gpointer user_data)
{
	GPdfControl *control;

	g_return_if_fail (persist_stream != NULL);
	g_return_if_fail (GPDF_IS_PERSIST_STREAM (persist_stream));
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (user_data));

	control = GPDF_CONTROL (user_data);
	control->priv->pdf_doc =
		gpdf_persist_stream_get_pdf_doc (persist_stream);

	if (control->priv->title)
		g_free (control->priv->title);

	control->priv->title = g_strdup (_("PDF Document"));

	gpdf_control_pdf_doc_changed (control);
}

static void
persist_file_loading_finished_cb (GPdfPersistFile *persist_file,
				  gpointer user_data)
{
	GPdfControl *control;
	const char *uri;
	GnomeVFSURI *vfs_uri;

	g_return_if_fail (persist_file != NULL);
	g_return_if_fail (GPDF_IS_PERSIST_FILE (persist_file));
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (user_data));

	control = GPDF_CONTROL (user_data);
	control->priv->pdf_doc = gpdf_persist_file_get_pdf_doc (persist_file);

	if (control->priv->title)
		g_free (control->priv->title);

	uri = gpdf_persist_file_get_current_uri (persist_file);

	vfs_uri = uri ? gnome_vfs_uri_new (uri) : NULL;

	if (vfs_uri) {
		control->priv->title =
			gnome_vfs_uri_extract_short_name (vfs_uri);
		gnome_vfs_uri_unref (vfs_uri);
	} else {
		control->priv->title = g_strdup (_("PDF Document"));
	}

	gpdf_control_pdf_doc_changed (control);
}

static void
persist_file_loading_failed_cb (GPdfPersistFile *persist_file,
				const char *message,
				gpointer user_data)
{
	GPdfControl *control;
	GtkWidget *dlg;
	gchar *primary;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (user_data));
	
	control = GPDF_CONTROL (user_data);			  

	control->priv->title = g_strdup (_("PDF Document"));
	control->priv->pdf_doc = NULL;
		
	primary = g_strdup_printf (
		_("Loading of %s failed."),
		gpdf_persist_file_get_current_uri (persist_file));

	gpdf_control_private_error_dialog (control, primary, message, FALSE, TRUE);

	g_free (primary);

	gpdf_control_pdf_doc_changed (control);
}

static void
gpdf_control_destroy (BonoboObject *object)
{
	GPdfControlPrivate *priv;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (object));

	DBG (g_message ("Destroying GPdfControl"));

	priv = GPDF_CONTROL (object)->priv;

	if (priv->bonobo_page_control) {
		bonobo_object_unref (BONOBO_OBJECT (priv->bonobo_page_control));
		priv->bonobo_page_control = NULL;
	}

	if (priv->title) {
		g_free (priv->title);
		priv->title = NULL;
	}

	BONOBO_CALL_PARENT (BONOBO_OBJECT_CLASS, destroy, (object));
}

static void
gpdf_control_set_property (GObject *object, guint param_id,
			   const GValue *value, GParamSpec *pspec)
{
	GPdfControl *gpdf_control;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (object));

	gpdf_control = GPDF_CONTROL (object);

	switch (param_id) {
	case PROP_PERSIST_STREAM: {
		GPdfPersistStream *pstream;
		
		pstream = GPDF_PERSIST_STREAM (g_value_get_object (value));
		
		gpdf_control->priv->persist_stream = pstream;

/*
 * Disable PersistStream because with it we can't close the control
 * while still printing.  Otherwise we'll get errors.  The
 * BonoboStream will be closed while we're still trying to read from
 * it.  (At least with Nautilus.)
 */
#if 0
		bonobo_object_ref (BONOBO_OBJECT (pstream));
		bonobo_object_add_interface (BONOBO_OBJECT (gpdf_control),
					     BONOBO_OBJECT (pstream));
#endif
		break;
	}
	case PROP_PERSIST_FILE: {
		GPdfPersistFile *pfile;

		pfile = GPDF_PERSIST_FILE (g_value_get_object (value));

		gpdf_control->priv->persist_file = pfile;

		bonobo_object_ref (BONOBO_OBJECT (pfile));
		bonobo_object_add_interface (BONOBO_OBJECT (gpdf_control),
					     BONOBO_OBJECT (pfile));
		break;
	}
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
gpdf_control_setup_persist (GPdfControl *control)
{
	gpdf_persist_stream_set_control (control->priv->persist_stream,
					 BONOBO_CONTROL (control));	
	g_signal_connect (G_OBJECT (control->priv->persist_stream),
			  "set_pdf",
			  G_CALLBACK (persist_stream_set_pdf_cb),
			  control);
	gpdf_persist_file_set_control (control->priv->persist_file,
				       BONOBO_CONTROL (control));
	g_signal_connect (G_OBJECT (control->priv->persist_file),
			  "loading_finished",
			  G_CALLBACK (persist_file_loading_finished_cb),
			  control);
	g_signal_connect (G_OBJECT (control->priv->persist_file),
			  "loading_failed",
			  G_CALLBACK (persist_file_loading_failed_cb),
			  control);
}

static void
property_bag_get_prop (BonoboPropertyBag *bag, BonoboArg *arg, guint arg_id,
		       CORBA_Environment *ev, gpointer user_data)
{
	GPdfControl *control;

	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (user_data));

	control = GPDF_CONTROL (user_data);

	switch (arg_id) {
	case PBPROP_TITLE:
		BONOBO_ARG_SET_STRING (
			arg, 
			CORBA_string_dup (control->priv->title));
		break;
	default:
		break;
	}		
}

static void
gpdf_control_setup_property_bag (GPdfControl *gpdf_control)
{
	BonoboPropertyBag *property_bag;

	property_bag = bonobo_property_bag_new (property_bag_get_prop, NULL,
						gpdf_control);
	bonobo_property_bag_add (property_bag, "title",
				 PBPROP_TITLE, TC_CORBA_string, NULL,
				 "Document title",
				 BONOBO_PROPERTY_READABLE);
	bonobo_control_set_properties (BONOBO_CONTROL (gpdf_control),
				       BONOBO_OBJREF (property_bag),
				       NULL);
	bonobo_object_unref (BONOBO_OBJECT (property_bag));
}

static void
gpdf_control_update_toggles (GPdfControl *gpdf_control)
{
	BonoboUIComponent *ui_component;
	gboolean state;
	
	g_return_if_fail (GPDF_IS_CONTROL (gpdf_control));

	ui_component =
	  bonobo_control_get_ui_component (BONOBO_CONTROL (gpdf_control));

	gpdf_control->priv->ignore_toggles_changes = TRUE;

	/* Update View Sidebar toggle */
	state = GTK_WIDGET_MAPPED (gpdf_control->priv->gpdf_sidebar);
	if (bonobo_ui_component_get_container (ui_component))
	  bonobo_ui_component_set_prop (ui_component,
					VIEW_SIDEBAR_CMDPATH,
					"state", state ? "1" : "0",
					NULL);	
	
	gpdf_control->priv->ignore_toggles_changes = FALSE;	
}

static void
gpdf_control_view_sidebar_changed_cb (BonoboUIComponent *component,
				      const char *path,
				      Bonobo_UIComponent_EventType type,
				      const char *state,
				      GPdfControl *control)
{
	GPdfSidebar *sidebar;
	GtkWidget *sidebar_w;

	g_return_if_fail (GPDF_IS_CONTROL (control));

	if (control->priv->ignore_toggles_changes) return; 

	sidebar_w = control->priv->gpdf_sidebar;
	sidebar = GPDF_SIDEBAR (sidebar_w);

	g_return_if_fail (GPDF_IS_SIDEBAR (sidebar));

	if (GTK_WIDGET_MAPPED (sidebar_w))
		gtk_widget_hide (sidebar_w);
	else
		gtk_widget_show (sidebar_w);

	gpdf_control_update_toggles(control);
}

static void
gpdf_control_sidebar_close_requested_cb (GPdfSidebar *sidebar, 
					 GPdfControl *control)
{
	BonoboUIComponent *ui_component;

	g_return_if_fail (GPDF_IS_SIDEBAR (sidebar));
	g_return_if_fail (GPDF_IS_CONTROL (control));

	ui_component =
	  bonobo_control_get_ui_component (BONOBO_CONTROL (control));

	gpdf_control_view_sidebar_changed_cb
	  (ui_component,
	   VIEW_SIDEBAR_CMDPATH,
	   (Bonobo_UIComponent_EventType)BONOBO_TYPE_UI_COMPONENT,
	   NULL,
	   control);
}

static void
gpdf_control_bookmark_selected_cb (GPdfBookmarksView *gpdf_bookmarks_view,
				   LinkAction        *link_action, 
				   gpointer           data)
{
	GPdfControl *gpdf_control;
	
	gpdf_control = GPDF_CONTROL (data);
	
	g_return_if_fail (GPDF_IS_BOOKMARKS_VIEW (gpdf_bookmarks_view));
	g_return_if_fail (GPDF_IS_CONTROL (gpdf_control));

	gpdf_view_bookmark_selected (GPDF_VIEW (gpdf_control->priv->gpdf_view),
				     link_action);
}

static void
gpdf_control_thumbnail_selected_cb (GPdfThumbnailsView *gpdf_thumbnails_view,
				    gint	       x, 
				    gint	       y, 
				    gint	       w, 
				    gint	       h, 
				    gint	       page, 
				    gpointer           data)
{
	GPdfControl *gpdf_control;
	
	gpdf_control = GPDF_CONTROL (data);
	
	g_return_if_fail (GPDF_IS_THUMBNAILS_VIEW (gpdf_thumbnails_view));
	g_return_if_fail (GPDF_IS_CONTROL (gpdf_control));

	gpdf_view_thumbnail_selected (GPDF_VIEW (gpdf_control->priv->gpdf_view),
				      x, y, w, h, page);
}

#ifdef USE_ANNOTS_VIEW
static void
gpdf_control_annotation_selected_cb (GPdfAnnotsView *gpdf_annots_view,
				     Annot          *annot, 
				     gint            page,
				     gpointer        data)
{
	GPdfControl *gpdf_control;
	
	gpdf_control = GPDF_CONTROL (data);
	
	g_return_if_fail (GPDF_IS_ANNOTS_VIEW (gpdf_annots_view));
	g_return_if_fail (GPDF_IS_CONTROL (gpdf_control));

	gpdf_view_annotation_selected (GPDF_VIEW (gpdf_control->priv->gpdf_view),
				       annot, page);
}

static void
gpdf_control_annotation_toggled_cb (GPdfAnnotsView *gpdf_annots_view,
				    Annot          *annot, 
				    gint            page,
				    gboolean	    active,
				    gpointer        data)
{
	GPdfControl *gpdf_control;
	
	gpdf_control = GPDF_CONTROL (data);
	
	g_return_if_fail (GPDF_IS_ANNOTS_VIEW (gpdf_annots_view));
	g_return_if_fail (GPDF_IS_CONTROL (gpdf_control));

	gpdf_view_annotation_toggled (GPDF_VIEW (gpdf_control->priv->gpdf_view),
				      annot, page, active);
}
#endif

static void
gpdf_control_sidebar_page_changed_cb (GPdfSidebar *sidebar,
				      const gchar *page_id,
				      GPdfControl *control)
{
	GtkWidget *page = NULL;
	
	g_return_if_fail (GPDF_IS_CONTROL (control));

	if (!strncmp (page_id, GPDF_BOOKMARKS_VIEW_PAGE_ID, strlen(GPDF_BOOKMARKS_VIEW_PAGE_ID)))
		page = control->priv->gpdf_bookmarks_view;
	else if (!strncmp (page_id, GPDF_THUMBNAILS_VIEW_PAGE_ID, strlen(GPDF_THUMBNAILS_VIEW_PAGE_ID)))
		page = control->priv->gpdf_thumbnails_view;
#ifdef USE_ANNOTS_VIEW
	else if (!strncmp (page_id, GPDF_ANNOTS_VIEW_PAGE_ID, strlen(GPDF_ANNOTS_VIEW_PAGE_ID)))
		page = control->priv->gpdf_annots_view;
#endif
	
	if (page)
		gpdf_sidebar_set_content
			(GPDF_SIDEBAR (control->priv->gpdf_sidebar),
			 G_OBJECT (page));
}

static void
gpdf_control_setup_view_widgets (GPdfControl *control)
{
	GtkWidget *vbox;
	GtkWidget *hpaned;
	GtkWidget *sidebar;
	GtkWidget *sw;
	GtkWidget *appbar;
	GtkWidget *gpdf_view;
	BonoboUIComponent *ui_component;
	
	ui_component =
	  bonobo_control_get_ui_component (BONOBO_CONTROL (control));

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	control->priv->vbox = vbox;
	
	hpaned = gtk_hpaned_new ();
	gtk_widget_show (hpaned);
	control->priv->hpaned = hpaned;

	sidebar = gpdf_sidebar_new ();
	control->priv->gpdf_sidebar = sidebar;
	g_signal_connect (G_OBJECT(sidebar),
			  "close_requested",
			  G_CALLBACK (gpdf_control_sidebar_close_requested_cb),
			  control);
	gpdf_sidebar_add_page (GPDF_SIDEBAR (sidebar),
			       _("Bookmarks"), GPDF_BOOKMARKS_VIEW_PAGE_ID,
			       NULL, 
			       FALSE);
	gpdf_sidebar_add_page (GPDF_SIDEBAR (sidebar),
			       _("Pages"), GPDF_THUMBNAILS_VIEW_PAGE_ID,
			       NULL, 
			       FALSE);
#ifdef USE_ANNOTS_VIEW
	gpdf_sidebar_add_page (GPDF_SIDEBAR (sidebar),
			       _("Annotations"), GPDF_ANNOTS_VIEW_PAGE_ID,
			       NULL, 
			       FALSE);
#endif
	
	g_signal_connect (sidebar, "page_changed",
			  G_CALLBACK (gpdf_control_sidebar_page_changed_cb),
			  control);

	sw = gtk_scrolled_window_new (NULL, NULL);
	control->priv->scrolled_window = sw;
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw),
					GTK_POLICY_AUTOMATIC,
					GTK_POLICY_AUTOMATIC);

	appbar = gnome_appbar_new (TRUE, TRUE, GNOME_PREFERENCES_NEVER);
	gtk_widget_show (appbar);
	control->priv->appbar = appbar;
	control->priv->gnome_appbar = GNOME_APPBAR(appbar);
	control->priv->progress = gnome_appbar_get_progress (GNOME_APPBAR (appbar));

	gtk_paned_add1 (GTK_PANED (hpaned), sidebar);

	gtk_paned_add2 (GTK_PANED (hpaned), sw);

	gtk_box_pack_start (GTK_BOX (vbox), hpaned, TRUE, TRUE, 0);

	gtk_box_pack_end (GTK_BOX (vbox), appbar, FALSE, FALSE, 0);

	control->priv->gpdf_view =
	  gpdf_view =
	    GTK_WIDGET (g_object_new (GPDF_TYPE_VIEW,
				"aa", TRUE,	/* FIXME: a pref ? */
				"control", control, 
				NULL));

	gtk_container_add (GTK_CONTAINER (sw), gpdf_view);

	/* Workaround for 132489 */
	gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (sw))
		->step_increment = 10;
	gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (sw))
		->step_increment = 10;
	
	g_signal_connect (G_OBJECT (gpdf_view), "page_changed",
			  G_CALLBACK (gc_page_changed_enable_page_buttons),
			  control);
	gtk_widget_show_all (sw);
	gtk_widget_grab_focus (GTK_WIDGET (gpdf_view));
}

static void
gpdf_control_setup_zoomable (GPdfControl *control)
{
	BonoboZoomable *zoomable = bonobo_zoomable_new ();

	control->priv->zoomable = zoomable;

	g_signal_connect (G_OBJECT (zoomable), "set_frame",
			  G_CALLBACK (gc_zoomable_set_frame_cb),
			  control);
	g_signal_connect (G_OBJECT (zoomable), "set_zoom_level",
			  G_CALLBACK (gc_zoomable_set_zoom_level_cb),
			  control);
	g_signal_connect (G_OBJECT (zoomable), "zoom_in",
			  G_CALLBACK (gc_zoom_in_cb),
			  control);
	g_signal_connect (G_OBJECT (zoomable), "zoom_out",
			  G_CALLBACK (gc_zoom_out_cb),
			  control);
	g_signal_connect (G_OBJECT (zoomable), "zoom_to_fit",
			  G_CALLBACK (gc_zoom_to_fit_cb),
			  control);
	g_signal_connect (G_OBJECT (zoomable), "zoom_to_default",
			  G_CALLBACK (gc_zoom_to_default_cb),
			  control);

	g_signal_connect (G_OBJECT (control->priv->gpdf_view), "zoom_changed",
			  G_CALLBACK (gc_zoom_changed_report_to_zoomable),
			  control);

	bonobo_zoomable_set_parameters_full (
		control->priv->zoomable,
		1.0,
		preferred_zoom_levels [0],
		preferred_zoom_levels [n_zoom_levels - 1],
		TRUE, TRUE, TRUE,
		preferred_zoom_levels,
		preferred_zoom_level_names,
		n_zoom_levels);

	bonobo_object_add_interface (BONOBO_OBJECT (control),
				     BONOBO_OBJECT (zoomable));
}

static void
gpdf_control_setup_page_control (GPdfControl *control)
{
	GtkWidget *page_control;
	BonoboControl *bonobo_page_control;

	page_control = GTK_WIDGET (g_object_new (GPDF_TYPE_PAGE_CONTROL, NULL));

	g_signal_connect (G_OBJECT (page_control), "set_page",
			  G_CALLBACK (gc_set_page_cb),
			  control);

	g_signal_connect (G_OBJECT (control->priv->gpdf_view), "page_changed",
			  G_CALLBACK (gc_page_changed_update_page_control),
			  control);

	gtk_widget_show (page_control);
	bonobo_page_control = bonobo_control_new (page_control);

	control->priv->bonobo_page_control = bonobo_page_control;
}

static GObject *
gpdf_control_constructor (GType type,
			  guint n_construct_properties,
			  GObjectConstructParam *construct_params)
{
	GObject *object;
	GPdfControl *control;
	GPdfControlPrivate *priv;

	object = G_OBJECT_CLASS (parent_class)->constructor (
		type, n_construct_properties, construct_params);

	control = GPDF_CONTROL (object);
	priv = control->priv;

	g_return_val_if_fail (priv->persist_stream != NULL, NULL);
	g_return_val_if_fail (priv->persist_file != NULL, NULL);
	gpdf_control_setup_persist (control);
	gpdf_control_setup_view_widgets (control);
	gpdf_control_setup_zoomable (control);
	gpdf_control_setup_page_control (control);

	bonobo_control_construct (BONOBO_CONTROL (object),
				  control->priv->vbox);

	control->priv->title = g_strdup (_("PDF Document"));
	gpdf_control_setup_property_bag (control);

	return object;
}

static void
gpdf_control_disconnected (BonoboControl *control)
{
	GPdfControlPrivate *priv;
	
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (control));

	priv = GPDF_CONTROL (control)->priv;

	gpdf_control_save_sidebar_state (GPDF_CONTROL (control));

	if (priv->file_chooser) {
		gtk_widget_destroy (priv->file_chooser);
		priv->file_chooser = NULL;
	}

	if (priv->properties) {
		gtk_widget_destroy (priv->properties);
		priv->properties = NULL;
	}

	if (priv->print_dialog) {
		gtk_widget_destroy (priv->print_dialog);
		priv->print_dialog = NULL;
	}

	BONOBO_CALL_PARENT (BONOBO_CONTROL_CLASS, disconnected, (control));
}

static void
gpdf_control_dispose (GObject *object)
{
	GPdfControl *control;
	
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (object));

	control = GPDF_CONTROL (object); 

	if (G_IS_OBJECT (control->priv->gpdf_bookmarks_view)) {
		g_object_unref (control->priv->gpdf_bookmarks_view);
		control->priv->gpdf_bookmarks_view = NULL; 
	}

	if (G_IS_OBJECT (control->priv->gpdf_thumbnails_view)) {
		g_object_unref (control->priv->gpdf_thumbnails_view);
		control->priv->gpdf_thumbnails_view = NULL; 
	}

#ifdef USE_ANNOTS_VIEW
	if (G_IS_OBJECT (control->priv->gpdf_annots_view)) {
		g_object_unref (control->priv->gpdf_annots_view);
		control->priv->gpdf_annots_view = NULL; 
	}
#endif
	BONOBO_CALL_PARENT (G_OBJECT_CLASS, dispose, (object));
}

static void
gpdf_control_finalize (GObject *object)
{
	g_return_if_fail (GPDF_IS_NON_NULL_CONTROL (object));

	g_free ((GPDF_CONTROL (object))->priv);

	BONOBO_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

static void
gpdf_control_class_init (GPdfControlClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS (klass);

	object_class->constructor = gpdf_control_constructor;
	object_class->dispose = gpdf_control_dispose;
	object_class->finalize = gpdf_control_finalize;
	object_class->set_property = gpdf_control_set_property;

	BONOBO_CONTROL_CLASS (klass)->activate = gpdf_control_activate;
	BONOBO_CONTROL_CLASS (klass)->disconnected = gpdf_control_disconnected;

	BONOBO_OBJECT_CLASS (klass)->destroy = gpdf_control_destroy;

	g_object_class_install_property (
		object_class, PROP_PERSIST_STREAM,
		g_param_spec_object ("persist_stream",
				     "PersistStream",
				     "PersistStream",
				     GPDF_TYPE_PERSIST_STREAM,
				     (GParamFlags)(G_PARAM_WRITABLE |
						   G_PARAM_CONSTRUCT_ONLY)));
	g_object_class_install_property (
		object_class, PROP_PERSIST_FILE,
		g_param_spec_object ("persist_file",
				     "PersistFile",
				     "PersistFile",
				     GPDF_TYPE_PERSIST_FILE,
				     (GParamFlags)(G_PARAM_WRITABLE |
						   G_PARAM_CONSTRUCT_ONLY)));
}

static void
gpdf_control_instance_init (GPdfControl *control)
{
	control->priv = g_new0 (GPdfControlPrivate, 1);
}

END_EXTERN_C

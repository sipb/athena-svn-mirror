#include <config.h>

#include "cell-renderer-uri.h"
#include "bb-marshal.h"

#include <libgnome/gnome-i18n.h>
#include <libgnome/gnome-macros.h>
#include <libgnome/gnome-url.h>

enum {
	PROP_0,
	PROP_URI,
	PROP_SHOWN,
};

enum {
	SIGNAL_URI_SHOWN,
	LAST_SIGNAL,
};

static guint uri_cell_signals[LAST_SIGNAL] = { 0 };

GNOME_CLASS_BOILERPLATE (CellRendererUri, cell_renderer_uri, 
			 GtkCellRendererText, GTK_TYPE_CELL_RENDERER_TEXT)

static void
cell_renderer_uri_get_property (GObject    *object,
				guint       param_id,
				GValue     *value,
				GParamSpec *pspec)
{
	CellRendererUri *cru = CELL_RENDERER_URI (object);

	switch (param_id) {
	case PROP_URI:
		g_value_set_string (value, cru->uri);
		break;
	case PROP_SHOWN:
		g_value_set_boolean (value, cru->shown);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static void
cell_renderer_uri_set_property (GObject      *object,
				guint         param_id,
				const GValue *value,
				GParamSpec   *pspec)
{
	CellRendererUri *cru = CELL_RENDERER_URI (object);

	switch (param_id) {
	case PROP_URI:
		g_free (cru->uri);
		cru->uri = g_strdup (g_value_get_string (value));
		break;
	case PROP_SHOWN:
		cru->shown = g_value_get_boolean (value);
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, param_id, pspec);
		break;
	}
}

static gboolean
cell_renderer_uri_activate (GtkCellRenderer      *cell,
			    GdkEvent             *event,
			    GtkWidget            *widget,
			    const char           *path,
			    GdkRectangle         *background_area,
			    GdkRectangle         *cell_area,
			    GtkCellRendererState  flags)
{
	CellRendererUri *cru;

	cru = CELL_RENDERER_URI (cell);

	if (!cru->uri)
		return FALSE;

	if (gnome_url_show (cru->uri, NULL))
		g_signal_emit (cell, uri_cell_signals[SIGNAL_URI_SHOWN], 0, path);
			      
	return TRUE;
}

static void
cell_renderer_uri_render (GtkCellRenderer *cell,
			  GdkWindow *window,
			  GtkWidget *widget,
			  GdkRectangle *background_area,
			  GdkRectangle *cell_area,
			  GdkRectangle *expose_area,
			  GtkCellRendererState flags)
{
	CellRendererUri *cru = CELL_RENDERER_URI (cell);

	g_object_set (G_OBJECT (cell),
		      "foreground", cru->shown ? "#840084" : "#0000ff",
		      NULL);

	GNOME_CALL_PARENT (GTK_CELL_RENDERER_CLASS, render, (cell, window, widget, background_area, cell_area, expose_area, flags));
}

static void
cell_renderer_uri_finalize (GObject *object)
{
	CellRendererUri *cru = CELL_RENDERER_URI (object);
	g_free (cru->uri);
	cru->uri = NULL;
	GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

static void
cell_renderer_uri_instance_init (CellRendererUri *cru)
{
	/* FIXME: get from style properties */
	g_object_set (G_OBJECT (cru),
		      "foreground", "blue",
		      "underline", PANGO_UNDERLINE_SINGLE,
		      "mode", GTK_CELL_RENDERER_MODE_ACTIVATABLE,
		      NULL);
}

static void
cell_renderer_uri_class_init (CellRendererUriClass *class)
{
	GObjectClass         *object_class   = G_OBJECT_CLASS (class);
	GtkCellRendererClass *renderer_class = GTK_CELL_RENDERER_CLASS (class);

	object_class->get_property = cell_renderer_uri_get_property;
	object_class->set_property = cell_renderer_uri_set_property;
	object_class->finalize     = cell_renderer_uri_finalize;

	renderer_class->activate   = cell_renderer_uri_activate;
	renderer_class->render     = cell_renderer_uri_render;

	g_object_class_install_property (object_class,
					 PROP_URI,
					 g_param_spec_string ("uri",
							      _("URI"),
							      _("URI to show when clicked."),
							      NULL,
							      G_PARAM_READWRITE));

	g_object_class_install_property (object_class,
					 PROP_SHOWN,
					 g_param_spec_boolean ("visited",
							       _("Visited"),
							       _("If the URI has been visited before."),
							       FALSE, G_PARAM_READWRITE));
	/* FIXME: install color style properties */

	uri_cell_signals[SIGNAL_URI_SHOWN] =
		g_signal_new ("uri_visited",
			      G_TYPE_FROM_CLASS (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET (CellRendererUriClass, uri_visited),
			      NULL, NULL,
			      _bb_marshal_VOID__STRING,
			      G_TYPE_NONE, 1, G_TYPE_STRING);
}

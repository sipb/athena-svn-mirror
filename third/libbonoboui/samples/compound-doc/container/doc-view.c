#include "config.h"
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-canvas-item.h>
#include <libgnomecanvas/gnome-canvas-rect-ellipse.h>

#include "doc-view.h"

typedef struct {
	SampleDoc 		*doc;
	Bonobo_UIContainer 	 uic;
	GnomeCanvas		*canvas;
	GnomeCanvasItem		*selection;
	GnomeCanvasItem		*handle_group;
} SampleDocView;
	

static void
layout_changed_cb (SampleComponent *comp, GnomeCanvasItem *item)
{
	gdouble affine [6];

	sample_component_get_affine (comp, affine);
	gnome_canvas_item_affine_absolute (item, affine);
}

static gboolean
item_pressed_cb (GnomeCanvasItem *item, SampleDocView *view)
{
	if (view->selection && (view->selection != item))
		gnome_canvas_item_set (view->selection, 
				       "selected", FALSE, 
				       NULL);

	view->selection = item;
	gnome_canvas_item_set (view->selection, "selected", TRUE, NULL);

	return TRUE;
}

static void
add_canvas_item (SampleDocView *view, SampleComponent *comp)
{
	GnomeCanvasItem *item;
	gdouble affine[6];

	item = gnome_canvas_item_new (gnome_canvas_root (view->canvas),
				      bonobo_canvas_item_get_type (),
				      "corba_server",
				      sample_component_get_server (comp),
				      "corba_ui_container",
				      view->uic,
				      NULL);

	sample_component_get_affine (comp, affine);
	gnome_canvas_item_affine_absolute (item, affine);

	g_signal_connect_data (G_OBJECT (comp), "changed", 
			       G_CALLBACK (layout_changed_cb), item, 
			       NULL, 0);

	g_signal_connect_data (G_OBJECT (item), "button_press_event", 
			       G_CALLBACK (item_pressed_cb), view, 
			       NULL, 0);

}

static void
add_components (SampleDocView *view)
{
	GList *l, *comps;

	comps = sample_doc_get_components (view->doc);

	for (l = comps; l; l = l->next)
		add_canvas_item (view, SAMPLE_COMPONENT (l->data));

	g_list_free (comps);
}

static void
background_cb (GnomeCanvasItem *item, GdkEvent *event, SampleDocView *view)
{
	if (view->selection && (event->button.button == 1)) {
		g_object_unref (view->handle_group);
		view->handle_group = NULL;
		view->selection = NULL;
	}
}

static void
destroy_view (GObject *obj, SampleDocView *view)
{
	g_object_unref (G_OBJECT (view->doc));
	bonobo_object_release_unref (view->uic, NULL);
	g_free (view);
}
	
GtkWidget *
sample_doc_view_new (SampleDoc *doc, Bonobo_UIContainer uic)
{
	SampleDocView *view;
	GtkWidget *canvas;
	GnomeCanvasItem *bg;

	view = g_new0 (SampleDocView, 1);
	if (!view)
		return NULL;

        canvas = gnome_canvas_new ();
	view->canvas = GNOME_CANVAS (canvas);
	gnome_canvas_set_scroll_region (view->canvas, -400.0, -300.0, 
					400.0, 300.0);
	g_signal_connect_data (G_OBJECT (canvas), "finalize",
			       G_CALLBACK (destroy_view), view,
			       NULL, 0);

	bg = gnome_canvas_item_new (gnome_canvas_root (view->canvas),
				    gnome_canvas_rect_get_type (),
			            "x1", -400.0, "y1", -300.0,
			            "x2", 400.0, "y2", 300.0,
			            "fill_color", "white", NULL);

	g_signal_connect_data (G_OBJECT (bg), "button_press_event",
			       G_CALLBACK (background_cb), view,
			       NULL, 0);

	view->uic = bonobo_object_dup_ref (uic, NULL);

	g_object_ref (G_OBJECT (doc));
	view->doc = doc;

	add_components (view);

	gtk_widget_show_all (canvas);

	return canvas;
}


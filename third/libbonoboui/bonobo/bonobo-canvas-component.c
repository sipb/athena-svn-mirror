/*
 * bonobo-canvas-component.c: implements the CORBA interface for
 * the Bonobo::Canvas:Item interface used in Bonobo::Views.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * (C) 1999-2001 Ximian, Inc.
 */
#include <config.h>
#include <stdio.h>
#include <string.h>
#include <gtk/gtksignal.h>
#include <bonobo/Bonobo.h>
#include <libart_lgpl/art_affine.h>
#include <libgnomecanvas/gnome-canvas.h>
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>
#include <gtk/gtk.h>
#include <bonobo/bonobo-control.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-ui-component.h>
#include <bonobo/bonobo-canvas-component.h>
#include <bonobo/bonobo-ui-marshal.h>
#undef BONOBO_DISABLE_DEPRECATED
#include <bonobo/bonobo-xobject.h>

enum {
	SET_BOUNDS,
	EVENT,
	LAST_SIGNAL
};

static gint gcc_signals [LAST_SIGNAL] = { 0, 0, };

typedef BonoboCanvasComponent Gcc;
#define GCC(x) BONOBO_CANVAS_COMPONENT(x)

struct _BonoboCanvasComponentPrivate {
	GnomeCanvasItem   *item;
};

#define PARENT_TYPE BONOBO_TYPE_OBJECT

/* Returns the GnomeCanvasItemClass of an object */
#define ICLASS(x) GNOME_CANVAS_ITEM_CLASS ((GTK_OBJECT_GET_CLASS (x)))

static GObjectClass *gcc_parent_class;

static gboolean do_update_flag = FALSE;

typedef struct
{
        gpointer *arg1;
        gpointer *arg2;
} EmitLater;


static gboolean
CORBA_SVP_Segment_to_SVPSeg (Bonobo_Canvas_SVPSegment *seg, ArtSVPSeg *art_seg)
{
	int i;

	art_seg->points = art_new (ArtPoint, seg->points._length);
	if (!art_seg->points)
		return FALSE;

	art_seg->dir = seg->up ? 0 : 1;
	art_seg->bbox.x0 = seg->bbox.x0;
	art_seg->bbox.x1 = seg->bbox.x1;
	art_seg->bbox.y0 = seg->bbox.y0;
	art_seg->bbox.y1 = seg->bbox.y1;

	art_seg->n_points = seg->points._length;

	for (i = 0; i < art_seg->n_points; i++){
		art_seg->points [i].x = seg->points._buffer [i].x;
		art_seg->points [i].y = seg->points._buffer [i].y;
	}

	return TRUE;
}

static void
free_seg (ArtSVPSeg *seg)
{
	g_assert (seg != NULL);
	g_assert (seg->points != NULL);
	
	art_free (seg->points);
}

/*
 * Encodes an ArtUta
 */
static Bonobo_Canvas_ArtUTA *
CORBA_UTA (ArtUta *uta)
{
	Bonobo_Canvas_ArtUTA *cuta;

	cuta = Bonobo_Canvas_ArtUTA__alloc ();
	if (!cuta)
		return NULL;

	if (!uta) {
		cuta->width = 0;
		cuta->height = 0;
		cuta->utiles._length = 0;
		cuta->utiles._maximum = 0;

		return cuta;
	}
	cuta->utiles._buffer = CORBA_sequence_Bonobo_Canvas_int32_allocbuf (uta->width * uta->height);
	cuta->utiles._length = uta->width * uta->height;
	cuta->utiles._maximum = uta->width * uta->height;
	if (!cuta->utiles._buffer) {
		CORBA_free (cuta);
		return NULL;
	}
		
	cuta->x0 = uta->x0;
	cuta->y0 = uta->y0;
	cuta->width = uta->width;
	cuta->height = uta->height;

	memcpy (cuta->utiles._buffer, uta->utiles, uta->width * uta->height * sizeof (ArtUtaBbox));

	return cuta;
}

static void
restore_state (GnomeCanvasItem *item, const Bonobo_Canvas_State *state)
{
	double affine [6];
	int i;

	for (i = 0; i < 6; i++)
		affine [i] = state->item_aff [i];

	gnome_canvas_item_affine_absolute (item->canvas->root, affine);
	item->canvas->pixels_per_unit = state->pixels_per_unit;
	item->canvas->scroll_x1 = state->canvas_scroll_x1;
	item->canvas->scroll_y1 = state->canvas_scroll_y1;
	item->canvas->zoom_xofs = state->zoom_xofs;
	item->canvas->zoom_yofs = state->zoom_yofs;
}

/* This is copied from gnome-canvas.c since it is declared static */
static void
invoke_update (GnomeCanvasItem *item, double *affine, ArtSVP *clip_path, int flags)
{
	int child_flags;
	double *child_affine;
        double i2w[6], w2c[6], i2c[6];

	child_flags = flags;
	if (!(item->object.flags & GNOME_CANVAS_ITEM_VISIBLE))
		child_flags &= ~GNOME_CANVAS_UPDATE_IS_VISIBLE;

	/* Apply the child item's transform */
        gnome_canvas_item_i2w_affine (item, i2w);
        gnome_canvas_w2c_affine (item->canvas, w2c);
        art_affine_multiply (i2c, i2w, w2c);
        child_affine = i2c;

	/* apply object flags to child flags */

	child_flags &= ~GNOME_CANVAS_UPDATE_REQUESTED;

	if (item->object.flags & GNOME_CANVAS_ITEM_NEED_UPDATE)
		child_flags |= GNOME_CANVAS_UPDATE_REQUESTED;

	if (item->object.flags & GNOME_CANVAS_ITEM_NEED_AFFINE)
		child_flags |= GNOME_CANVAS_UPDATE_AFFINE;

	if (item->object.flags & GNOME_CANVAS_ITEM_NEED_CLIP)
		child_flags |= GNOME_CANVAS_UPDATE_CLIP;

	if (item->object.flags & GNOME_CANVAS_ITEM_NEED_VIS)
		child_flags |= GNOME_CANVAS_UPDATE_VISIBILITY;

	if ((child_flags & (GNOME_CANVAS_UPDATE_REQUESTED
			    | GNOME_CANVAS_UPDATE_AFFINE
			    | GNOME_CANVAS_UPDATE_CLIP
			    | GNOME_CANVAS_UPDATE_VISIBILITY))
	    && GNOME_CANVAS_ITEM_GET_CLASS (item)->update)
		(* GNOME_CANVAS_ITEM_GET_CLASS (item)->update) (
			item, child_affine, clip_path, child_flags);
}

static Bonobo_Canvas_ArtUTA *
impl_Bonobo_Canvas_Component_update (PortableServer_Servant     servant,
				     const Bonobo_Canvas_State *state,
				     const Bonobo_Canvas_affine aff,
				     const Bonobo_Canvas_SVP   *clip_path,
				     CORBA_long                 flags,
				     CORBA_double              *x1, 
				     CORBA_double              *y1, 
				     CORBA_double              *x2, 
				     CORBA_double              *y2, 
				     CORBA_Environment         *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	double affine [6];
	int i;
	ArtSVP *svp = NULL;
	Bonobo_Canvas_ArtUTA *cuta;

	GnomeCanvasItemClass *gci_class = gtk_type_class (
					gnome_canvas_item_get_type ());

	restore_state (item, state);
	for (i = 0; i < 6; i++)
		affine [i] = aff [i];

	if (clip_path->_length > 0) {
		svp = art_alloc (sizeof (ArtSVP) + (clip_path->_length * sizeof (ArtSVPSeg)));
		if (svp == NULL)
			goto fail;

		svp->n_segs = clip_path->_length;
		
		for (i = 0; svp->n_segs; i++) {
			gboolean ok;
		
			ok = CORBA_SVP_Segment_to_SVPSeg (&clip_path->_buffer [i], &svp->segs [i]);

			if (!ok) {
				int j;

				for (j = 0; j < i; j++) {
					free_seg (&svp->segs [j]);
					art_free (svp);
					goto fail;
				}
			}
		}
	}

	invoke_update (item, (double *)aff, svp, flags);

	if (svp){
		for (i = 0; i < svp->n_segs; i++)
			free_seg (&svp->segs [i]);
		art_free (svp);
	}

 fail:
	if (getenv ("CC_DEBUG"))
		printf ("%g %g %g %g\n", item->x1, item->x2, item->y1, item->y2);
	*x1 = item->x1;
	*x2 = item->x2;
	*y1 = item->y1;
	*y2 = item->y2;

	cuta = CORBA_UTA (item->canvas->redraw_area);
	if (cuta == NULL) {
		CORBA_exception_set_system (ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
		return NULL;
	}

	/*
	 * Now, mark our canvas as fully up to date
	 */

        /* Clears flags for root item. */
	(* gci_class->update) (item->canvas->root, affine, svp, flags);

	if (item->canvas->redraw_area) {
		art_uta_free (item->canvas->redraw_area);
		item->canvas->redraw_area = NULL;
	}
	item->canvas->need_redraw = FALSE;
	
	return cuta;
}

static GdkGC *the_gc;

static void
impl_Bonobo_Canvas_Component_realize (PortableServer_Servant  servant,
				      const CORBA_char       *window,
				      CORBA_Environment      *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	GdkWindow *gdk_window = gdk_window_foreign_new_for_display
		(gtk_widget_get_display (GTK_WIDGET (item->canvas)),
		 bonobo_control_x11_from_window_id (window));

	if (gdk_window == NULL) {
		g_warning ("Invalid window id passed='%s'", window);
		return;
	}

	the_gc = gdk_gc_new (gdk_window);
	item->canvas->layout.bin_window = gdk_window;
	ICLASS (item)->realize (item);
}

static void
impl_Bonobo_Canvas_Component_unrealize (PortableServer_Servant servant,
					CORBA_Environment     *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);

	ICLASS (item)->unrealize (item);

	if (item->canvas->layout.bin_window) {
		g_object_unref (item->canvas->layout.bin_window);
		item->canvas->layout.bin_window = NULL;
	}
}

static void
impl_Bonobo_Canvas_Component_map (PortableServer_Servant servant,
				  CORBA_Environment     *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	
	ICLASS (item)->map (item);
}

static void
impl_Bonobo_Canvas_Component_unmap (PortableServer_Servant servant,
				    CORBA_Environment     *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	
	ICLASS (item)->unmap (item);
}

static void
my_gdk_pixmap_foreign_release (GdkPixmap *pixmap)
{
	GdkWindowObject *priv = (GdkWindowObject *) pixmap;

	if (G_OBJECT (priv)->ref_count != 1){
		g_warning ("This item is keeping a refcount to a foreign pixmap");
		return;
	}

#ifdef FIXME
	gdk_xid_table_remove (priv->xwindow);
	g_dataset_destroy (priv);
	g_free (priv);
#endif
}

static void
impl_Bonobo_Canvas_Component_draw (PortableServer_Servant        servant,
				   const Bonobo_Canvas_State    *state,
				   const CORBA_char             *drawable_id,
				   CORBA_short                   x,
				   CORBA_short                   y,
				   CORBA_short                   width,
				   CORBA_short                   height,
				   CORBA_Environment            *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	GdkPixmap *pix;
	
	gdk_flush ();
	pix = gdk_pixmap_foreign_new_for_display
		(gtk_widget_get_display (GTK_WIDGET (item->canvas)),
		 bonobo_control_x11_from_window_id (drawable_id));

	if (pix == NULL){
		g_warning ("Invalid window id passed='%s'", drawable_id);
		return;
	}

	restore_state (item, state);
	ICLASS (item)->draw (item, pix, x, y, width, height);

	my_gdk_pixmap_foreign_release (pix);
	gdk_flush ();
}

static void
impl_Bonobo_Canvas_Component_render (PortableServer_Servant servant,
				     Bonobo_Canvas_Buf     *buf,
				     CORBA_Environment     *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	GnomeCanvasBuf canvas_buf;

	if (!(buf->flags & Bonobo_Canvas_IS_BUF)) {
		buf->rgb_buf._length = buf->row_stride * (buf->rect.y1 - buf->rect.y0);
		buf->rgb_buf._maximum = buf->rgb_buf._length;
		
		buf->rgb_buf._buffer = CORBA_sequence_CORBA_octet_allocbuf (
			buf->rgb_buf._length);
		CORBA_sequence_set_release (&buf->rgb_buf, TRUE);

		if (buf->rgb_buf._buffer == NULL) {
			CORBA_exception_set_system (
				ev, ex_CORBA_NO_MEMORY, CORBA_COMPLETED_NO);
			return;
		}
	}

	canvas_buf.buf = buf->rgb_buf._buffer;
	
	canvas_buf.buf_rowstride = buf->row_stride;
	canvas_buf.rect.x0 = buf->rect.x0;
	canvas_buf.rect.x1 = buf->rect.x1;
	canvas_buf.rect.y0 = buf->rect.y0;
	canvas_buf.rect.y1 = buf->rect.y1;
	canvas_buf.bg_color = buf->bg_color;
	if (buf->flags & Bonobo_Canvas_IS_BG)
		canvas_buf.is_bg = 1;
	else
		canvas_buf.is_bg = 0;

	if (buf->flags & Bonobo_Canvas_IS_BUF)
		canvas_buf.is_buf = 1;
	else
		canvas_buf.is_buf = 0;

	ICLASS (item)->render (item, &canvas_buf);

	/* return */
	buf->flags =
		(canvas_buf.is_bg ? Bonobo_Canvas_IS_BG : 0) |
		(canvas_buf.is_buf ? Bonobo_Canvas_IS_BUF : 0);
}

static CORBA_boolean 
impl_Bonobo_Canvas_Component_contains (PortableServer_Servant servant,
				       CORBA_double           x,
				       CORBA_double           y,
				       CORBA_Environment     *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
        GnomeCanvasItem *new_item;
        int cx, cy;
	CORBA_boolean ret;

        gnome_canvas_w2c (item->canvas, x, y, &cx, &cy);

	if (getenv ("CC_DEBUG"))
		printf ("Point %g %g: ", x, y);
	ret = ICLASS (item)->point (item, x, y, cx, cy, &new_item) == 0.0 &&
                new_item != NULL;
	if (getenv ("CC_DEBUG"))
		printf ("=> %s\n", ret ? "yes" : "no");
	
	return ret;
}

static void
impl_Bonobo_Canvas_Component_bounds (PortableServer_Servant     servant,
				     const Bonobo_Canvas_State *state,
				     CORBA_double              *x1,
				     CORBA_double              *x2,
				     CORBA_double              *y1,
				     CORBA_double              *y2,
				     CORBA_Environment         *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);

	restore_state (item, state);
	ICLASS (item)->bounds (item, x1, y1, x2, y2);
}

/*
 * Converts the event marshalled from the container into a GdkEvent
 */
static void
Bonobo_Gdk_Event_to_GdkEvent (const Bonobo_Gdk_Event *gnome_event, GdkEvent *gdk_event)
{
	switch (gnome_event->_d){
	case Bonobo_Gdk_FOCUS:
		gdk_event->type = GDK_FOCUS_CHANGE;
		gdk_event->focus_change.in = gnome_event->_u.focus.inside;
		return;
		
	case Bonobo_Gdk_KEY:
		if (gnome_event->_u.key.type == Bonobo_Gdk_KEY_PRESS)
			gdk_event->type = GDK_KEY_PRESS;
		else
			gdk_event->type = GDK_KEY_RELEASE;
		gdk_event->key.time = gnome_event->_u.key.time;
		gdk_event->key.state = gnome_event->_u.key.state;
		gdk_event->key.keyval = gnome_event->_u.key.keyval;
		gdk_event->key.length = gnome_event->_u.key.length;
		gdk_event->key.string = g_strdup (gnome_event->_u.key.str);
		return;
		
	case Bonobo_Gdk_MOTION:
		gdk_event->type = GDK_MOTION_NOTIFY;
		gdk_event->motion.time = gnome_event->_u.motion.time;
		gdk_event->motion.x = gnome_event->_u.motion.x;
		gdk_event->motion.y = gnome_event->_u.motion.y;
		gdk_event->motion.x_root = gnome_event->_u.motion.x_root;
		gdk_event->motion.y_root = gnome_event->_u.motion.y_root;
#ifdef FIXME
		gdk_event->motion.xtilt = gnome_event->_u.motion.xtilt;
		gdk_event->motion.ytilt = gnome_event->_u.motion.ytilt;
#endif
		gdk_event->motion.state = gnome_event->_u.motion.state;
		gdk_event->motion.is_hint = gnome_event->_u.motion.is_hint;
		return;
		
	case Bonobo_Gdk_BUTTON:
		switch (gnome_event->_u.button.type){
		case Bonobo_Gdk_BUTTON_PRESS:
			gdk_event->type = GDK_BUTTON_PRESS;
			break;
		case Bonobo_Gdk_BUTTON_RELEASE:
			gdk_event->type = GDK_BUTTON_RELEASE;
			break;
		case Bonobo_Gdk_BUTTON_2_PRESS:
			gdk_event->type = GDK_2BUTTON_PRESS;
			break;
		case Bonobo_Gdk_BUTTON_3_PRESS:
			gdk_event->type = GDK_3BUTTON_PRESS;
			break;
		}
		gdk_event->button.time   = gnome_event->_u.button.time;
		gdk_event->button.x      = gnome_event->_u.button.x;
		gdk_event->button.y      = gnome_event->_u.button.y;
		gdk_event->button.x_root = gnome_event->_u.button.x_root;
		gdk_event->button.y_root = gnome_event->_u.button.y_root;
		gdk_event->button.button = gnome_event->_u.button.button;
		return;
		
	case Bonobo_Gdk_CROSSING:
		if (gnome_event->_u.crossing.type == Bonobo_Gdk_ENTER)
			gdk_event->type = GDK_ENTER_NOTIFY;
		else
			gdk_event->type = GDK_LEAVE_NOTIFY;
		
		gdk_event->crossing.time   = gnome_event->_u.crossing.time;
		gdk_event->crossing.x      = gnome_event->_u.crossing.x;
		gdk_event->crossing.y      = gnome_event->_u.crossing.y;
		gdk_event->crossing.x_root = gnome_event->_u.crossing.x_root;
		gdk_event->crossing.y_root = gnome_event->_u.crossing.y_root;
		gdk_event->crossing.state  = gnome_event->_u.crossing.state;
		switch (gnome_event->_u.crossing.mode){
		case Bonobo_Gdk_NORMAL:
			gdk_event->crossing.mode = GDK_CROSSING_NORMAL;
			break;
			
		case Bonobo_Gdk_GRAB:
			gdk_event->crossing.mode = GDK_CROSSING_GRAB;
			break;
		case Bonobo_Gdk_UNGRAB:
			gdk_event->crossing.mode = GDK_CROSSING_UNGRAB;
			break;
		}
		return;
	}
	g_assert_not_reached ();
}

/**
 * handle_event:
 * @canvas: the pseudo-canvas that this component is part of.
 * @ev: The GdkEvent event as set up for the component.
 *
 * Returns: True if a canvas item handles the event and returns true.
 *
 * Passes the remote item's event back into the local pseudo-canvas so that
 * canvas items can see events the normal way as if they were not using bonobo.
 */
static gboolean 
handle_event(GtkWidget *canvas, GdkEvent *ev)
{
        GtkWidgetClass *klass = GTK_WIDGET_GET_CLASS(canvas);
        gboolean retval = FALSE;

        switch (ev->type) {
                case GDK_ENTER_NOTIFY:
                        gnome_canvas_world_to_window(GNOME_CANVAS(canvas),
                                        ev->crossing.x, ev->crossing.y,
                                        &ev->crossing.x, &ev->crossing.y);
                        retval = (klass->enter_notify_event)(canvas, 
                                        &ev->crossing);
                        break;
                case GDK_LEAVE_NOTIFY:
                        gnome_canvas_world_to_window(GNOME_CANVAS(canvas),
                                        ev->crossing.x, ev->crossing.y,
                                        &ev->crossing.x, &ev->crossing.y);
                        retval = (klass->leave_notify_event)(canvas, 
                                        &ev->crossing);
                        break;
                case GDK_MOTION_NOTIFY:
                        gnome_canvas_world_to_window(GNOME_CANVAS(canvas),
                                        ev->motion.x, ev->motion.y,
                                        &ev->motion.x, &ev->motion.y);
                        retval = (klass->motion_notify_event)(canvas, 
                                        &ev->motion);
                        break;
                case GDK_BUTTON_PRESS:
                case GDK_2BUTTON_PRESS:
                case GDK_3BUTTON_PRESS:
                        gnome_canvas_world_to_window(GNOME_CANVAS(canvas),
                                        ev->button.x, ev->button.y,
                                        &ev->button.x, &ev->button.y);
                        retval = (klass->button_press_event)(canvas, 
                                        &ev->button);
                        break;
                case GDK_BUTTON_RELEASE:
                        gnome_canvas_world_to_window(GNOME_CANVAS(canvas),
                                        ev->button.x, ev->button.y,
                                        &ev->button.x, &ev->button.y);
                        retval = (klass->button_release_event)(canvas, 
                                        &ev->button);
                        break;
                case GDK_KEY_PRESS:
                        retval = (klass->key_press_event)(canvas, &ev->key);
                        break;
                case GDK_KEY_RELEASE:
                        retval = (klass->key_release_event)(canvas, &ev->key);
                        break;
                case GDK_FOCUS_CHANGE:
                        if (&ev->focus_change.in)
                                retval = (klass->focus_in_event)(canvas,
                                        &ev->focus_change);
                        else
                                retval = (klass->focus_out_event)(canvas,
                                        &ev->focus_change);
                        break;
                default:
                        g_warning("Canvas event not handled %d", ev->type);
                        break;

        }
        return retval;
}

static gboolean
handle_event_later (EmitLater *data)
{
        GtkWidget *canvas = GTK_WIDGET(data->arg1);
        GdkEvent *event = (GdkEvent *)data->arg2;

        handle_event(canvas, event);
        gdk_event_free(event);
        g_free(data);
        return FALSE;
}

/*
 * Receives events from the container end, decodes it into a synthetic
 * GdkEvent and forwards this to the CanvasItem
 */
static CORBA_boolean
impl_Bonobo_Canvas_Component_event (PortableServer_Servant     servant,
				    const Bonobo_Canvas_State *state,
				    const Bonobo_Gdk_Event    *gnome_event,
				    CORBA_Environment         *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	GdkEvent *gdk_event = gdk_event_new(GDK_NOTHING);
	CORBA_boolean retval = FALSE;
        EmitLater *data;

	restore_state (item, state);
        
        gdk_event->any.window = item->canvas->layout.bin_window;
        g_object_ref(gdk_event->any.window);

	Bonobo_Gdk_Event_to_GdkEvent (gnome_event, gdk_event);

        /*
         * Problem: When dealing with multiple canvas component's within the
         * same process it is possible to get to this point when one of the
         * pseudo-canvas's is inside its idle loop.  This is normally not a
         * problem unless an event from one component can trigger a request for
         * an update on another component.
         *
         * Solution: If any component is in do_update, set up an idle_handler
         * and send the event later. If the event is sent later, the client will
         * get a false return value.  Normally this value is used to determine
         * whether or not to propagate the event up the canvas group tree.
         */

        if (!do_update_flag) {
                retval = handle_event(GTK_WIDGET(item->canvas), gdk_event);
                gdk_event_free(gdk_event);
        }
        else {
                data = g_new0(EmitLater, 1);
                data->arg1 = (gpointer)item->canvas;
                data->arg2 = (gpointer)gdk_event;
                /* has a higher priority then do_update*/
                g_idle_add_full(GDK_PRIORITY_REDRAW-10,
                                (GSourceFunc)handle_event_later, data, NULL);
        }

	return retval;
}

static void
impl_Bonobo_Canvas_Component_setCanvasSize (PortableServer_Servant servant,
					    CORBA_short            x,
					    CORBA_short            y,
					    CORBA_short            width,
					    CORBA_short            height,
					    CORBA_Environment     *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	GtkAllocation alloc;

	alloc.x = x;
	alloc.y = y;
	alloc.width = width;
	alloc.height = height;

	gtk_widget_size_allocate (GTK_WIDGET (item->canvas), &alloc);
}

static void
set_bounds_later(EmitLater *data)
{
        CORBA_Environment ev;

        CORBA_exception_init (&ev);

        g_signal_emit(GCC(data->arg1), gcc_signals [SET_BOUNDS], 0, (const
                                Bonobo_Canvas_DRect *) data->arg2, &ev);

        g_free(data);
        CORBA_exception_free(&ev);
}

static void
impl_Bonobo_Canvas_Component_setBounds (PortableServer_Servant     servant,
					const Bonobo_Canvas_DRect *bbox,
					CORBA_Environment         *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
        EmitLater *data;

        if (!do_update_flag) {
                g_signal_emit (gcc, gcc_signals [SET_BOUNDS], 0, bbox, &ev);
        }
        else {
                data = g_new0(EmitLater, 1);
                data->arg1 = (gpointer)gcc;
                data->arg2 = (gpointer)bbox;
                g_idle_add_full(GDK_PRIORITY_REDRAW-10,
                                (GSourceFunc)set_bounds_later, data, NULL);
        }
}

static void
gcc_finalize (GObject *object)
{
	Gcc *gcc = GCC (object);
	GnomeCanvasItem *item = BONOBO_CANVAS_COMPONENT (object)->priv->item;

	gtk_object_destroy (GTK_OBJECT (item->canvas));
	g_free (gcc->priv);

	gcc_parent_class->finalize (object);
}

/* Ripped from gtk+/gtk/gtkmain.c */
static gboolean
_bonobo_boolean_handled_accumulator (GSignalInvocationHint *ihint,
				     GValue                *return_accu,
				     const GValue          *handler_return,
				     gpointer               dummy)
{
	gboolean continue_emission;
	gboolean signal_handled;
  
	signal_handled = g_value_get_boolean (handler_return);
	g_value_set_boolean (return_accu, signal_handled);
	continue_emission = !signal_handled;
	
	return continue_emission;
}

static void
bonobo_canvas_component_class_init (BonoboCanvasComponentClass *klass)
{
	GObjectClass *object_class = (GObjectClass *) klass;
	POA_Bonobo_Canvas_Component__epv *epv = &klass->epv;

	gcc_parent_class = g_type_class_peek_parent(klass);

	object_class->finalize = gcc_finalize;

	gcc_signals [SET_BOUNDS] = 
                g_signal_new ("set_bounds",
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET(BonoboCanvasComponentClass, set_bounds),
			      NULL, NULL,
			      bonobo_ui_marshal_VOID__POINTER_POINTER,
			      G_TYPE_NONE, 2,
			      G_TYPE_POINTER, G_TYPE_POINTER);

	gcc_signals [EVENT] = 
		g_signal_new ("event", 
			      G_OBJECT_CLASS_TYPE (object_class),
			      G_SIGNAL_RUN_LAST,
			      G_STRUCT_OFFSET(BonoboCanvasComponentClass, event),
			      _bonobo_boolean_handled_accumulator, NULL,
			      bonobo_ui_marshal_BOOLEAN__POINTER,
			      G_TYPE_BOOLEAN, 1,
			      G_TYPE_POINTER);

	epv->update         = impl_Bonobo_Canvas_Component_update;
	epv->realize        = impl_Bonobo_Canvas_Component_realize;
	epv->unrealize      = impl_Bonobo_Canvas_Component_unrealize;
	epv->map            = impl_Bonobo_Canvas_Component_map;
	epv->unmap          = impl_Bonobo_Canvas_Component_unmap;
	epv->draw           = impl_Bonobo_Canvas_Component_draw;
	epv->render         = impl_Bonobo_Canvas_Component_render;
	epv->bounds         = impl_Bonobo_Canvas_Component_bounds;
	epv->event          = impl_Bonobo_Canvas_Component_event;
	epv->contains       = impl_Bonobo_Canvas_Component_contains;
	epv->setCanvasSize  = impl_Bonobo_Canvas_Component_setCanvasSize;
	epv->setBounds      = impl_Bonobo_Canvas_Component_setBounds;
}

static void
bonobo_canvas_component_init (GObject *object)
{
	Gcc *gcc = GCC (object);

	gcc->priv = g_new0 (BonoboCanvasComponentPrivate, 1);
}

BONOBO_TYPE_FUNC_FULL (BonoboCanvasComponent, 
			   Bonobo_Canvas_Component,
			   PARENT_TYPE,
			   bonobo_canvas_component)


/**
 * bonobo_canvas_component_construct:
 * @comp: a #BonoboCanvasComponent to initialize
 * @item: A #GnomeCanvasItem that is being exported
 *
 * Creates a CORBA server for the interface Bonobo::Canvas::Item
 * wrapping @item.
 *
 * Returns: The BonoboCanvasComponent.
 */
BonoboCanvasComponent *
bonobo_canvas_component_construct (BonoboCanvasComponent  *comp,
				  GnomeCanvasItem         *item)
{
	g_return_val_if_fail (GNOME_IS_CANVAS_ITEM (item), NULL);
	g_return_val_if_fail (BONOBO_IS_CANVAS_COMPONENT (comp), NULL);

	comp->priv->item = item;

	return comp;
}

				  
/**
 * bonobo_canvas_component_new:
 * @item: A GnomeCanvasItem that is being exported
 *
 * Creates a CORBA server for the interface Bonobo::Canvas::Item
 * wrapping @item.
 *
 * Returns: The BonoboCanvasComponent.
 */
BonoboCanvasComponent *
bonobo_canvas_component_new (GnomeCanvasItem *item)
{
	BonoboCanvasComponent *comp;
	
	g_return_val_if_fail (GNOME_IS_CANVAS_ITEM (item), NULL);
	
	comp = g_object_new (BONOBO_TYPE_CANVAS_COMPONENT, NULL);

	return bonobo_canvas_component_construct (comp, item);
}

/** 
 * bonobo_canvas_component_get_item:
 * @comp: A #BonoboCanvasComponent object
 *
 * Returns: The GnomeCanvasItem that this BonoboCanvasComponent proxies
 */
GnomeCanvasItem *
bonobo_canvas_component_get_item (BonoboCanvasComponent *comp)
{
	g_return_val_if_fail (comp != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_CANVAS_COMPONENT (comp), NULL);

	return comp->priv->item;
}

/*
 * Hack root item
 *
 * This is a hack since we can not modify the existing GNOME Canvas to handle
 * this case.
 *
 * Here is the problem we are solving:
 *
 *    1. Items usually queue a request to be updated/redrawn by calling
 *       gnome_canvas_item_request_update().  This triggers in the container
 *       canvas an idle handler to be queued to update the display on the
 *       idle handler.
 *        
 *    2. There is no way we can catch this on the Canvas.
 *
 * To catch this we do:
 *
 *    3. replace the regular Canvas' root field (of type GnomeCanvasGroup)
 *       with a RootItemHack item.  This item has an overriden ->update method
 *       that will notify the container canvas on the container process about
 *       our update requirement. 
 */

static GnomeCanvasGroupClass *rih_parent_class;

typedef struct {
	GnomeCanvasGroup       group;
	Bonobo_Canvas_ComponentProxy proxy;
	GnomeCanvasItem *orig_root;
} RootItemHack;

typedef struct {
	GnomeCanvasGroupClass parent_class;
} RootItemHackClass;

static GType root_item_hack_get_type (void);
#define ROOT_ITEM_HACK_TYPE (root_item_hack_get_type ())
#define ROOT_ITEM_HACK(obj) (GTK_CHECK_CAST((obj), ROOT_ITEM_HACK_TYPE, RootItemHack))

static void
rih_dispose (GObject *obj)
{
	RootItemHack *rih = ROOT_ITEM_HACK (obj);

	rih->proxy = bonobo_object_release_unref (rih->proxy, NULL);

	if (rih->orig_root)
		gtk_object_destroy (GTK_OBJECT (rih->orig_root));
	rih->orig_root = NULL;

	G_OBJECT_CLASS (rih_parent_class)->dispose (obj);
}

/*
 * Invoked by our local canvas when an update is requested,
 * we forward this to the container canvas
 */
static void
rih_update (GnomeCanvasItem *item, double affine [6], ArtSVP *svp, int flags)
{
	RootItemHack *rih = (RootItemHack *) item;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

        /* If we get here then we know the GnomeCanvas must be inside
         * do_update. The flag tells ALL components not to send events that
         * might trigger another update request and thereby cause the canvas
         * NEED_UPDATE flags to become unsyncronized.
         */
        do_update_flag = TRUE;

	Bonobo_Canvas_ComponentProxy_requestUpdate (rih->proxy, &ev);

        do_update_flag = FALSE;

	CORBA_exception_free (&ev);
}

static void
rih_class_init (GObjectClass *klass)
{
	GnomeCanvasItemClass *item_class = (GnomeCanvasItemClass *) klass;
	rih_parent_class = g_type_class_peek_parent (klass);

	klass->dispose = rih_dispose;

	item_class->update = rih_update;
}
      
static GType
root_item_hack_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GTypeInfo info = {
			sizeof (RootItemHackClass),
			NULL, NULL,
			(GClassInitFunc) rih_class_init,
			NULL, NULL,
			sizeof (RootItemHack),
			0,
			NULL, NULL
		};

		type = g_type_register_static (
			gnome_canvas_group_get_type (),
			"RootItemHack", &info, 0);
	}

	return type;
}

static RootItemHack *
root_item_hack_new (GnomeCanvas *canvas, Bonobo_Canvas_ComponentProxy proxy)
{
	RootItemHack *item_hack;

	item_hack = g_object_new (root_item_hack_get_type (), NULL);
	item_hack->proxy = proxy;
	item_hack->orig_root = canvas->root;
	GNOME_CANVAS_ITEM (item_hack)->canvas = canvas;

	return item_hack;
}

/**
 * bonobo_canvas_new:
 * @is_aa: Flag indicating is antialiased canvas is desired
 * @proxy: Remote proxy for the component this canvas will support
 *
 * Returns: A #GnomeCanvas with the root replaced by a forwarding item.
 */ 
GnomeCanvas *
bonobo_canvas_new (gboolean is_aa, Bonobo_Canvas_ComponentProxy proxy)
{
	GnomeCanvas *canvas;
	GnomeCanvasItem *orig_root;

	if (is_aa)
		canvas = GNOME_CANVAS (gnome_canvas_new_aa ());
	else
		canvas = GNOME_CANVAS (gnome_canvas_new ());

	orig_root = canvas->root;

	canvas->root = GNOME_CANVAS_ITEM (root_item_hack_new (canvas, proxy));

        /* A hack to prevent gtkwidget from issuing a warning about there not
           being a parent class. */
        gtk_container_add(GTK_CONTAINER (gtk_window_new(GTK_WINDOW_TOPLEVEL)),
                            GTK_WIDGET(canvas));
	gtk_widget_realize (GTK_WIDGET (canvas));

	/* Gross */
	GTK_WIDGET_SET_FLAGS (canvas, GTK_VISIBLE | GTK_MAPPED);

	return canvas;
}

/**
 * bonobo_canvas_component_grab:
 * @comp: A #BonoboCanvasComponent object
 * @mask: Mask of events to grab
 * @cursor: #GdkCursor to display during grab
 * @time: Time of last event before grab
 *
 * Grabs the mouse focus via a call to the remote proxy.
 */
void
bonobo_canvas_component_grab (BonoboCanvasComponent *comp, guint mask,
			      GdkCursor *cursor, guint32 time,
			      CORBA_Environment     *opt_ev)
{
	CORBA_Environment *ev, temp_ev;

	if (!opt_ev) {
		CORBA_exception_init (&temp_ev);
		ev = &temp_ev;
	} else
		ev = opt_ev;

	Bonobo_Canvas_ComponentProxy_grabFocus (
		ROOT_ITEM_HACK (comp->priv->item->canvas->root)->proxy, 
		mask, cursor->type, time, ev);

	if (!opt_ev)
		CORBA_exception_free (&temp_ev);
}

/**
 * bonobo_canvas_component_ungrab:
 * @comp: A #BonoboCanvasComponent object
 * @time: Time of last event before grab
 *
 * Grabs the mouse focus via a call to the remote proxy.
 */
void
bonobo_canvas_component_ungrab (BonoboCanvasComponent *comp, guint32 time,
				CORBA_Environment     *opt_ev)
{
	CORBA_Environment *ev, temp_ev;

	if (!opt_ev) {
		CORBA_exception_init (&temp_ev);
		ev = &temp_ev;
	} else
		ev = opt_ev;

	Bonobo_Canvas_ComponentProxy_ungrabFocus (
		ROOT_ITEM_HACK (comp->priv->item->canvas->root)->proxy, time, ev);

	if (!opt_ev)
		CORBA_exception_free (&temp_ev);
}

/**
 * bonobo_canvas_component_get_ui_container:
 * @comp: A #BonoboCanvasComponent object
 *
 * Returns: The UI container for the component's remote proxy.
 */
Bonobo_UIContainer
bonobo_canvas_component_get_ui_container (BonoboCanvasComponent *comp,
					  CORBA_Environment     *opt_ev)
{
	Bonobo_UIContainer corba_uic;
	CORBA_Environment *ev, temp_ev;

	if (!opt_ev) {
		CORBA_exception_init (&temp_ev);
		ev = &temp_ev;
	} else
		ev = opt_ev;

	corba_uic = Bonobo_Canvas_ComponentProxy_getUIContainer (
		ROOT_ITEM_HACK (comp->priv->item->canvas->root)->proxy, ev);

	if (!opt_ev)
		CORBA_exception_free (&temp_ev);

	return corba_uic;
}

/* BonoboCanvasComponentFactory is used to replace the old BonoboEmbeddable
 * object.  It sets up a canvas component factory to conform with the current
 * Bonobo IDL.
 */

#define BONOBO_CANVAS_COMPONENT_FACTORY_TYPE       \
   (bonobo_canvas_component_factory_get_type())

#define BONOBO_CANVAS_COMPONENT_FACTORY(o)    \
   (G_TYPE_CHECK_INSTANCE_CAST ((o),\
   BONOBO_CANVAS_COMPONENT_FACTORY_TYPE, BonoboCanvasComponentFactory))

typedef struct _BonoboCanvasComponentFactoryPrivate \
   BonoboCanvasComponentFactoryPrivate;

typedef struct {
        BonoboObject base;
        BonoboCanvasComponentFactoryPrivate *priv;
} BonoboCanvasComponentFactory;

typedef struct {
        BonoboObjectClass parent_class;

        POA_Bonobo_CanvasComponentFactory__epv epv;
} BonoboCanvasComponentFactoryClass;
       
GType bonobo_canvas_component_factory_get_type(void) G_GNUC_CONST;

struct _BonoboCanvasComponentFactoryPrivate {
   GnomeItemCreator item_creator;
   void *item_creator_data;
};

static GObjectClass *gccf_parent_class;

static Bonobo_Canvas_Component
impl_Bonobo_canvas_component_factory_createCanvasItem (
   PortableServer_Servant servant, CORBA_boolean aa,
   const Bonobo_Canvas_ComponentProxy _item_proxy,
   CORBA_Environment *ev)
{
        BonoboCanvasComponentFactory *factory = BONOBO_CANVAS_COMPONENT_FACTORY(
              bonobo_object_from_servant (servant));
        Bonobo_Canvas_ComponentProxy item_proxy;
        BonoboCanvasComponent *component;
        GnomeCanvas *pseudo_canvas;

        if (factory->priv->item_creator == NULL)
                return CORBA_OBJECT_NIL;

        item_proxy = CORBA_Object_duplicate (_item_proxy, ev);

	pseudo_canvas = bonobo_canvas_new (aa, item_proxy);

        component = (*factory->priv->item_creator)(
                /*factory,*/ pseudo_canvas, factory->priv->item_creator_data);

        return bonobo_object_dup_ref (BONOBO_OBJREF (component), ev);
}

static void
bonobo_canvas_component_factory_class_init (
      BonoboCanvasComponentFactoryClass *klass)
{
        POA_Bonobo_CanvasComponentFactory__epv *epv = &klass->epv;

        gccf_parent_class = g_type_class_peek_parent(klass);
        epv->createCanvasComponent = 
           impl_Bonobo_canvas_component_factory_createCanvasItem;
}

static void
bonobo_canvas_component_factory_init (BonoboCanvasComponentFactory *factory)
{
        factory->priv = g_new0 (BonoboCanvasComponentFactoryPrivate, 1);
}

BONOBO_TYPE_FUNC_FULL (BonoboCanvasComponentFactory,
                       Bonobo_CanvasComponentFactory,
                       BONOBO_TYPE_X_OBJECT,
                       bonobo_canvas_component_factory);

/**
 * bonobo_canvas_component_factory_new:
 * @item_factory: A callback invoke when the container activates the object.
 * @user_data: User data pointer.
 *
 * Returns: The object to be passed into bonobo_generic_factory_main.
 */
BonoboObject *bonobo_canvas_component_factory_new(
      GnomeItemCreator item_factory, void *user_data)
{
        BonoboCanvasComponentFactory *factory;
        factory = g_object_new (BONOBO_CANVAS_COMPONENT_FACTORY_TYPE, NULL);

        factory->priv->item_creator = item_factory;
        factory->priv->item_creator_data = user_data;

        return BONOBO_OBJECT(factory); 
}

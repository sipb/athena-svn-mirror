/*
 * bonobo-canvas-item.c: GnomeCanvasItem implementation to serve as a client-
 *			 proxy for embedding remote canvas-items.
 *
 * Author:
 *     Miguel de Icaza (miguel@kernel.org)
 *
 * (C) 1999, 2000 Ximian, Inc.
 */

/* FIXME: this needs re-writing to use BonoboObject ! */

#include <config.h>
#include <stdio.h>
#include <string.h>
#include <bonobo/Bonobo.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-canvas-item.h>
#include <bonobo/bonobo-object.h>
#include <bonobo/bonobo-types.h>
#include <bonobo/bonobo-main.h>
#include <libgnome/gnome-i18n.h>
#include <libgnome/gnome-macros.h>
#include <gdk/gdkprivate.h>
#include <gdk/gdkx.h>
#include <gtk/gtksignal.h>

GNOME_CLASS_BOILERPLATE (BonoboCanvasItem,
			 bonobo_canvas_item,
			 GObject,
			 GNOME_TYPE_CANVAS_ITEM);

typedef struct {
	POA_Bonobo_Canvas_ComponentProxy proxy_servant;
	GnomeCanvasItem           *item_bound;
	PortableServer_ObjectId   *oid;
	Bonobo_UIContainer	   ui_container;
} ComponentProxyServant;

struct _BonoboCanvasItemPrivate {
	Bonobo_Canvas_Component object;
	ComponentProxyServant  *proxy;
	int               realize_pending;
};

enum {
	PROP_0,
	PROP_CORBA_FACTORY,
	PROP_CORBA_UI_CONTAINER,
};

/*
 * Horizontal space saver
 */
#define GBI(x)          BONOBO_CANVAS_ITEM(x)
typedef BonoboCanvasItem Gbi;

/*
 * Creates a Bonobo_Canvas_SVPSegment structure representing the ArtSVPSeg
 * structure, suitable for sending over the network
 */
static gboolean
art_svp_segment_to_CORBA_SVP_Segment (ArtSVPSeg *seg, Bonobo_Canvas_SVPSegment *segment)
{
	int i;
	
	segment->points._buffer = CORBA_sequence_Bonobo_Canvas_Point_allocbuf (seg->n_points);
	if (segment->points._buffer == NULL)
		return FALSE;

	segment->points._maximum = seg->n_points;
	segment->points._length = seg->n_points;
	
	if (seg->dir == 0)
		segment->up = CORBA_TRUE;
	else
		segment->up = CORBA_FALSE;

	segment->bbox.x0 = seg->bbox.x0;
	segment->bbox.x1 = seg->bbox.x1;
	segment->bbox.y0 = seg->bbox.y0;
	segment->bbox.y1 = seg->bbox.y1;

	for (i = 0; i < seg->n_points; i++){
		segment->points._buffer [i].x = seg->points [i].x;
		segment->points._buffer [i].y = seg->points [i].y;
	}

	return TRUE;
}

/*
 * Creates a Bonobo_Canvas_SVP CORBA structure from the art_svp, suitable
 * for sending over the wire
 */
static Bonobo_Canvas_SVP *
art_svp_to_CORBA_SVP (ArtSVP *art_svp)
{
	Bonobo_Canvas_SVP *svp;
	int i;
	
	svp = Bonobo_Canvas_SVP__alloc ();
	if (!svp)
		return NULL;
	
	if (art_svp){
		svp->_buffer = CORBA_sequence_Bonobo_Canvas_SVPSegment_allocbuf (art_svp->n_segs);
		if (svp->_buffer == NULL){
			svp->_length = 0;
			svp->_maximum = 0;
			return svp;
		}
		svp->_maximum = art_svp->n_segs;
		svp->_length = art_svp->n_segs;

		for (i = 0; i < art_svp->n_segs; i++){
			gboolean ok;
			
			ok = art_svp_segment_to_CORBA_SVP_Segment (
				&art_svp->segs [i], &svp->_buffer [i]);
			if (!ok){
				int j;
				
				for (j = 0; j < i; j++)
					CORBA_free (&svp->_buffer [j]);
				CORBA_free (svp);
				return NULL;
			}
		}
	} else {
		svp->_maximum = 0;
		svp->_length = 0;
	}

	return svp;
}

static ArtUta *
uta_from_cuta (Bonobo_Canvas_ArtUTA *cuta)
{
	ArtUta *uta;

	uta = art_uta_new (cuta->x0, cuta->y0, cuta->x0 + cuta->width, cuta->y0 + cuta->height);
	memcpy (uta->utiles, cuta->utiles._buffer, cuta->width * cuta->height * sizeof (ArtUtaBbox));

	return uta;
}

static void
prepare_state (GnomeCanvasItem *item, Bonobo_Canvas_State *target)
{
	double item_affine [6];
	GnomeCanvas *canvas = item->canvas;
	int i;

	gnome_canvas_item_i2w_affine (item, item_affine);
	for (i = 0; i < 6; i++)
		target->item_aff [i] = item_affine [i];

	target->pixels_per_unit = canvas->pixels_per_unit;
	target->canvas_scroll_x1 = canvas->scroll_x1;
	target->canvas_scroll_y1 = canvas->scroll_y1;
	target->zoom_xofs = canvas->zoom_xofs;
	target->zoom_yofs = canvas->zoom_yofs;
}

static void
gbi_update (GnomeCanvasItem *item, double *item_affine,
	    ArtSVP *item_clip_path, int item_flags)
{
	Gbi *gbi = GBI (item);
	Bonobo_Canvas_affine affine;
	Bonobo_Canvas_State state;
	Bonobo_Canvas_SVP *clip_path = NULL;
	CORBA_Environment ev;
	CORBA_double x1, y1, x2, y2;
	Bonobo_Canvas_ArtUTA *cuta;
	int i;

	if (getenv ("DEBUG_BI"))
		g_message ("gbi_update");

	GNOME_CALL_PARENT (GNOME_CANVAS_ITEM_CLASS, update,
			   (item, item_affine, item_clip_path, item_flags));
	
	for (i = 0; i < 6; i++)
		affine [i] = item_affine [i];

	clip_path = art_svp_to_CORBA_SVP (item_clip_path);
	if (!clip_path)
		return;

	CORBA_exception_init (&ev);
	prepare_state (item, &state);
	cuta = Bonobo_Canvas_Component_update (
		gbi->priv->object,
		&state, affine, clip_path, item_flags,
		&x1, &y1, &x2, &y2,
		&ev);

	if (!BONOBO_EX (&ev)){
		if (cuta->width > 0 && cuta->height > 0){
			ArtUta *uta;

			uta = uta_from_cuta (cuta);
			gnome_canvas_request_redraw_uta (item->canvas, uta);
		}

		item->x1 = x1;
		item->y1 = y1;
		item->x2 = x2;
		item->y2 = y2;

		if (getenv ("DEBUG_BI"))
			g_message ("Bbox: %g %g %g %g", x1, y1, x2, y2);

		CORBA_free (cuta);
	}
	
	CORBA_exception_free (&ev);

	CORBA_free (clip_path);
}

static void
proxy_size_allocate (GnomeCanvas *canvas, GtkAllocation *allocation, BonoboCanvasItem *bonobo_item)
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);
	Bonobo_Canvas_Component_setCanvasSize (
		bonobo_item->priv->object,
		allocation->x, allocation->y,
		allocation->width, allocation->height, &ev);
	CORBA_exception_free (&ev);
}

static void
gbi_realize (GnomeCanvasItem *item)
{
	Gbi *gbi = GBI (item);
	CORBA_Environment ev;

	if (getenv ("DEBUG_BI"))
		g_message ("gbi_realize");
	
	GNOME_CALL_PARENT (GNOME_CANVAS_ITEM_CLASS, realize, (item));

	if (gbi->priv->object == CORBA_OBJECT_NIL) {
		gbi->priv->realize_pending = 1;
		return;
	}
		
	g_signal_connect (item->canvas, "size_allocate",
			  G_CALLBACK (proxy_size_allocate), item);

	CORBA_exception_init (&ev);
	gdk_flush ();
	Bonobo_Canvas_Component_realize (
		gbi->priv->object, 
		GDK_WINDOW_XWINDOW (item->canvas->layout.bin_window),
		&ev);
	CORBA_exception_free (&ev);
}

static void
gbi_unrealize (GnomeCanvasItem *item)
{
	Gbi *gbi = GBI (item);
	CORBA_Environment ev;
	
	if (getenv ("DEBUG_BI"))
		g_message ("gbi_unrealize");

	if (gbi->priv->object != CORBA_OBJECT_NIL){
		CORBA_exception_init (&ev);
		Bonobo_Canvas_Component_unrealize (gbi->priv->object, &ev);
		CORBA_exception_free (&ev);
	}

	GNOME_CALL_PARENT (GNOME_CANVAS_ITEM_CLASS, unrealize, (item));
}

static void
gbi_draw (GnomeCanvasItem *item, GdkDrawable *drawable, int x, int y, int width, int height)
{
	Gbi *gbi = GBI (item);
	CORBA_Environment ev;
	Bonobo_Canvas_State state;
	
	if (getenv ("DEBUG_BI"))
		g_message ("draw: %d %d %d %d", x, y, width, height);

	/*
	 * This call ensures the drawable XID is allocated on the X server
	 */
	gdk_flush ();
	CORBA_exception_init (&ev);

	prepare_state (item, &state);
	Bonobo_Canvas_Component_draw (
		gbi->priv->object,
		&state,
		GDK_WINDOW_XWINDOW (drawable),
		x, y, width, height,
		&ev);
	CORBA_exception_free (&ev);
}

static double
gbi_point (GnomeCanvasItem *item, double x, double y, int cx, int cy, GnomeCanvasItem **actual)
{
	Gbi *gbi = GBI (item);
	CORBA_Environment ev;
	
	if (getenv ("DEBUG_BI"))
		g_message ("gbi_point %g %g", x, y);
	
	CORBA_exception_init (&ev);
	if (Bonobo_Canvas_Component_contains (gbi->priv->object, x, y, &ev)){
		CORBA_exception_free (&ev);
		*actual = item;
		if (getenv ("DEBUG_BI"))
			g_message ("event inside");
		return 0.0;
	}
	CORBA_exception_free (&ev);

	if (getenv ("DEBUG_BI"))
		g_message ("event outside");
	*actual = NULL;
	return 1000.0;
}

static void
gbi_bounds (GnomeCanvasItem *item, double *x1, double *y1, double *x2, double *y2)
{
	Gbi *gbi = GBI (item);
	CORBA_Environment ev;
	Bonobo_Canvas_State state;
	
	if (getenv ("DEBUG_BI"))
		g_message ("gbi_bounds");
	
	CORBA_exception_init (&ev);
	prepare_state (item, &state);
	Bonobo_Canvas_Component_bounds (gbi->priv->object, &state, x1, y1, x2, y2, &ev);
	CORBA_exception_free (&ev);

	if (getenv ("DEBUG_BI"))
		g_message ("gbi_bounds %g %g %g %g", *x1, *y1, *x2, *y2);
}

static void
gbi_render (GnomeCanvasItem *item, GnomeCanvasBuf *buf)
{
	Gbi *gbi = GBI (item);
	Bonobo_Canvas_Buf *cbuf;
	CORBA_Environment ev;

	if (getenv ("DEBUG_BI"))
		g_message ("gbi_render (%d %d)-(%d %d)",
			buf->rect.x0, buf->rect.y0,
			buf->rect.x1, buf->rect.y1);

	cbuf = Bonobo_Canvas_Buf__alloc ();
	if (!cbuf)
		return;

	cbuf->rgb_buf._buffer = buf->buf;

#if 0
	/*
	 * Inneficient!
	 */
	if (!buf->is_buf)
		gnome_canvas_buf_ensure_buf (buf);
#endif
	
	if (buf->is_buf){
		cbuf->rgb_buf._maximum = buf->buf_rowstride * (buf->rect.y1 - buf->rect.y0);
		cbuf->rgb_buf._length = buf->buf_rowstride * (buf->rect.y1 - buf->rect.y0);
		cbuf->rgb_buf._buffer = buf->buf;
		CORBA_sequence_set_release (&cbuf->rgb_buf, FALSE);
	} else {
		cbuf->rgb_buf._maximum = 0;
		cbuf->rgb_buf._length = 0;
		cbuf->rgb_buf._buffer = NULL;
	}
	cbuf->row_stride = buf->buf_rowstride;
	
	cbuf->rect.x0 = buf->rect.x0;
	cbuf->rect.x1 = buf->rect.x1;
	cbuf->rect.y0 = buf->rect.y0;
	cbuf->rect.y1 = buf->rect.y1;
	cbuf->bg_color = buf->bg_color;
	cbuf->flags =
		(buf->is_bg  ? Bonobo_Canvas_IS_BG : 0) |
		(buf->is_buf ? Bonobo_Canvas_IS_BUF : 0);
	
	CORBA_exception_init (&ev);
	Bonobo_Canvas_Component_render (gbi->priv->object, cbuf, &ev);
	if (BONOBO_EX (&ev)){
		CORBA_exception_free (&ev);
		return;
	}
	CORBA_exception_free (&ev);
	
	memcpy (buf->buf, cbuf->rgb_buf._buffer, cbuf->rgb_buf._length);
	buf->is_bg  = (cbuf->flags & Bonobo_Canvas_IS_BG) != 0;
	buf->is_buf = (cbuf->flags & Bonobo_Canvas_IS_BUF) != 0;
	
	CORBA_free (cbuf);
}

static Bonobo_Gdk_Event *
gdk_event_to_bonobo_event (GdkEvent *event)
{
	Bonobo_Gdk_Event *e = Bonobo_Gdk_Event__alloc ();

	if (e == NULL)
		return NULL;
			
	switch (event->type){

	case GDK_FOCUS_CHANGE:
		e->_d = Bonobo_Gdk_FOCUS;
		e->_u.focus.inside = event->focus_change.in;
		return e;
			
	case GDK_KEY_PRESS:
	case GDK_KEY_RELEASE:
		e->_d = Bonobo_Gdk_KEY;

		if (event->type == GDK_KEY_PRESS)
			e->_u.key.type = Bonobo_Gdk_KEY_PRESS;
		else
			e->_u.key.type = Bonobo_Gdk_KEY_RELEASE;
		e->_u.key.time =   event->key.time;
		e->_u.key.state =  event->key.state;
		e->_u.key.keyval = event->key.keyval;
		e->_u.key.length = event->key.length;
		e->_u.key.str = CORBA_string_dup (event->key.string);
		return e;

	case GDK_MOTION_NOTIFY:
		e->_d = Bonobo_Gdk_MOTION;
		e->_u.motion.time = event->motion.time;
		e->_u.motion.x = event->motion.x;
		e->_u.motion.y = event->motion.x;
		e->_u.motion.x_root = event->motion.x_root;
		e->_u.motion.y_root = event->motion.y_root;
#ifdef FIXME
		e->_u.motion.xtilt = event->motion.xtilt;
		e->_u.motion.ytilt = event->motion.ytilt;
#endif
		e->_u.motion.state = event->motion.state;
		e->_u.motion.is_hint = event->motion.is_hint != 0;
		return e;
		
	case GDK_BUTTON_PRESS:
	case GDK_2BUTTON_PRESS:
	case GDK_3BUTTON_PRESS:
	case GDK_BUTTON_RELEASE:
		e->_d = Bonobo_Gdk_BUTTON;
		if (event->type == GDK_BUTTON_PRESS)
			e->_u.button.type = Bonobo_Gdk_BUTTON_PRESS;
		else if (event->type == GDK_BUTTON_RELEASE)
			e->_u.button.type = Bonobo_Gdk_BUTTON_RELEASE;
		else if (event->type == GDK_2BUTTON_PRESS)
			e->_u.button.type = Bonobo_Gdk_BUTTON_2_PRESS;
		else if (event->type == GDK_3BUTTON_PRESS)
			e->_u.button.type = Bonobo_Gdk_BUTTON_3_PRESS;
		e->_u.button.time = event->button.time;
		e->_u.button.x = event->button.x;
		e->_u.button.y = event->button.y;
		e->_u.button.x_root = event->button.x_root;
		e->_u.button.y_root = event->button.y_root;
		e->_u.button.button = event->button.button;
		return e;

	case GDK_ENTER_NOTIFY:
	case GDK_LEAVE_NOTIFY:
		e->_d = Bonobo_Gdk_CROSSING;
		if (event->type == GDK_ENTER_NOTIFY)
			e->_u.crossing.type = Bonobo_Gdk_ENTER;
		else
			e->_u.crossing.type = Bonobo_Gdk_LEAVE;
		e->_u.crossing.time = event->crossing.time;
		e->_u.crossing.x = event->crossing.x;
		e->_u.crossing.y = event->crossing.y;
		e->_u.crossing.x_root = event->crossing.x_root;
		e->_u.crossing.y_root = event->crossing.y_root;

		switch (event->crossing.mode){
		case GDK_CROSSING_NORMAL:
			e->_u.crossing.mode = Bonobo_Gdk_NORMAL;
			break;

		case GDK_CROSSING_GRAB:
			e->_u.crossing.mode = Bonobo_Gdk_GRAB;
			break;
			
		case GDK_CROSSING_UNGRAB:
			e->_u.crossing.mode = Bonobo_Gdk_UNGRAB;
			break;
		}
		return e;

	default:
		g_warning ("Unsupported event received");
	}
	return NULL;
}

static gint
gbi_event (GnomeCanvasItem *item, GdkEvent *event)
{
	Gbi *gbi = GBI (item);
	CORBA_Environment ev;
	Bonobo_Gdk_Event *corba_event;
	Bonobo_Canvas_State state;
	CORBA_boolean ret;
	
	if (getenv ("DEBUG_BI"))
		g_message ("gbi_event");
	
	corba_event = gdk_event_to_bonobo_event (event);
	if (corba_event == NULL)
		return FALSE;
	
	CORBA_exception_init (&ev);
	prepare_state (item, &state);
	ret = Bonobo_Canvas_Component_event (gbi->priv->object, &state, corba_event, &ev);
	CORBA_exception_free (&ev);
	CORBA_free (corba_event);

	return (gint) ret;
}

static void
gbi_set_property (GObject      *object,
		  guint         property_id,
		  const GValue *value,
		  GParamSpec   *pspec)
{
	Gbi *gbi = GBI (object);
	Bonobo_Canvas_ComponentProxy proxy_ref;
	Bonobo_CanvasComponentFactory factory;
	CORBA_Environment ev;

	switch (property_id) {
	case PROP_CORBA_FACTORY:

		CORBA_exception_init (&ev);

		gbi->priv->object = bonobo_object_release_unref (gbi->priv->object, &ev);

		factory = bonobo_value_get_corba_object (value);

		g_return_if_fail (factory != CORBA_OBJECT_NIL);

		proxy_ref = PortableServer_POA_servant_to_reference (
				bonobo_poa (), (void *) gbi->priv->proxy, &ev);

		gbi->priv->object = 
			Bonobo_CanvasComponentFactory_createCanvasComponent (
				factory, GNOME_CANVAS_ITEM (gbi)->canvas->aa, 
				proxy_ref, &ev);

		if (ev._major != CORBA_NO_EXCEPTION)
			gbi->priv->object = CORBA_OBJECT_NIL;

		CORBA_Object_release (factory, &ev);

		CORBA_exception_free (&ev);

		if (gbi->priv->object == CORBA_OBJECT_NIL) {
			g_object_unref (gbi);
			return;
		}

		/* Initial size notification */
		proxy_size_allocate (
			GNOME_CANVAS_ITEM (gbi)->canvas,
			&(GTK_WIDGET (GNOME_CANVAS_ITEM (gbi)->canvas)->allocation),
			gbi);
	
		if (gbi->priv->realize_pending){
			gbi->priv->realize_pending = 0;
			gbi_realize (GNOME_CANVAS_ITEM (gbi));
		}
		break;

	case PROP_CORBA_UI_CONTAINER:
		gbi->priv->proxy->ui_container = bonobo_value_get_unknown (value);

		g_return_if_fail (gbi->priv->proxy->ui_container != CORBA_OBJECT_NIL);
		break;

	default:
		g_warning ("Unexpected arg_id %u", property_id);
		break;
	}
}

static void
gbi_finalize (GObject *object)
{
	Gbi *gbi = GBI (object);
	CORBA_Environment ev;

	if (getenv ("DEBUG_BI"))
		g_message ("gbi_finalize");

	CORBA_exception_init (&ev);

	bonobo_object_release_unref (gbi->priv->object, &ev);

	if (gbi->priv->proxy){
		ComponentProxyServant *proxy = gbi->priv->proxy;
		
		PortableServer_POA_deactivate_object (bonobo_poa (), proxy->oid, &ev);
		POA_Bonobo_Unknown__fini ((void *) proxy, &ev);
		CORBA_free (proxy->oid);
		g_free (proxy);
	}
	
	g_free (gbi->priv);
	CORBA_exception_free (&ev);

	GNOME_CALL_PARENT (G_OBJECT_CLASS, finalize, (object));
}

static void
bonobo_canvas_item_class_init (BonoboCanvasItemClass *object_class)
{
	GObjectClass *gobject_class = (GObjectClass *) object_class;
	GnomeCanvasItemClass *item_class =
		(GnomeCanvasItemClass *) object_class;

	g_object_class_install_property (
		gobject_class,
		PROP_CORBA_FACTORY,
		g_param_spec_boxed (
			"corba_factory",
			_("corba factory"),
			_("The factory pointer"),
			BONOBO_TYPE_STATIC_UNKNOWN,
			G_PARAM_WRITABLE));

	g_object_class_install_property (
		gobject_class,
		PROP_CORBA_UI_CONTAINER,
		g_param_spec_boxed (
			"corba_ui_factory",
			_("corba UI container"),
			_("The User interface container"),
			BONOBO_TYPE_STATIC_UNKNOWN,
			G_PARAM_WRITABLE));
	
	gobject_class->finalize     = gbi_finalize;
	gobject_class->set_property = gbi_set_property;

	item_class->update     = gbi_update;
	item_class->realize    = gbi_realize;
	item_class->unrealize  = gbi_unrealize;
	item_class->draw       = gbi_draw;
	item_class->point      = gbi_point;
	item_class->bounds     = gbi_bounds;
	item_class->render     = gbi_render;
	item_class->event      = gbi_event;
}

static void
impl_Bonobo_Canvas_ComponentProxy_requestUpdate (PortableServer_Servant servant,
					         CORBA_Environment     *ev)
{
	ComponentProxyServant *item_proxy = (ComponentProxyServant *) servant;

	gnome_canvas_item_request_update (item_proxy->item_bound);

}
					    
static void
impl_Bonobo_Canvas_ComponentProxy_grabFocus (PortableServer_Servant servant,
					     guint32                mask, 
					     gint32                 cursor_type,
					     guint32                time,
					     CORBA_Environment     *ev)
{
	ComponentProxyServant *item_proxy = (ComponentProxyServant *) servant;
	GdkCursor *cursor;

	cursor = gdk_cursor_new ((GdkCursorType) cursor_type);

	gnome_canvas_item_grab (item_proxy->item_bound, mask, cursor, time);
}

static void
impl_Bonobo_Canvas_ComponentProxy_ungrabFocus (PortableServer_Servant servant,
					       guint32 time,
					       CORBA_Environment *ev)
{
	ComponentProxyServant *item_proxy = (ComponentProxyServant *) servant;

	gnome_canvas_item_ungrab (item_proxy->item_bound, time);
}

static Bonobo_UIContainer
impl_Bonobo_Canvas_ComponentProxy_getUIContainer (PortableServer_Servant servant,
						  CORBA_Environment     *ev)
{
	ComponentProxyServant *item_proxy = (ComponentProxyServant *) servant;

	g_return_val_if_fail (item_proxy->ui_container != CORBA_OBJECT_NIL,
			      CORBA_OBJECT_NIL);

	return bonobo_object_dup_ref (item_proxy->ui_container, NULL);
}

static POA_Bonobo_Canvas_ComponentProxy__epv item_proxy_epv;

static POA_Bonobo_Canvas_ComponentProxy__vepv item_proxy_vepv = {
	NULL,
	&item_proxy_epv
};

/*
 * Creates a CORBA server to handle the ComponentProxy requests, it is not
 * activated by default
 */
static ComponentProxyServant *
create_proxy (GnomeCanvasItem *item)
{
	ComponentProxyServant *item_proxy = g_new0 (ComponentProxyServant, 1);
	CORBA_Environment ev;
	
	CORBA_exception_init (&ev);
	POA_Bonobo_Canvas_ComponentProxy__init ((PortableServer_Servant) item_proxy, &ev);

	item_proxy_epv.requestUpdate  = impl_Bonobo_Canvas_ComponentProxy_requestUpdate;
	item_proxy_epv.grabFocus      = impl_Bonobo_Canvas_ComponentProxy_grabFocus;
	item_proxy_epv.ungrabFocus    = impl_Bonobo_Canvas_ComponentProxy_ungrabFocus;
	item_proxy_epv.getUIContainer = impl_Bonobo_Canvas_ComponentProxy_getUIContainer;

	item_proxy->proxy_servant.vepv = &item_proxy_vepv;
	item_proxy->item_bound = item;

	item_proxy->oid = PortableServer_POA_activate_object (
		bonobo_poa (), (void *) item_proxy, &ev);

	CORBA_exception_free (&ev);

	return item_proxy;
}

static void
bonobo_canvas_item_instance_init (BonoboCanvasItem *gbi)
{
	gbi->priv = g_new0 (BonoboCanvasItemPrivate, 1);
	gbi->priv->proxy = create_proxy (GNOME_CANVAS_ITEM (gbi));
}

void
bonobo_canvas_item_set_bounds (BonoboCanvasItem *item, double x1, double y1,
			       double x2, double y2)
{
	g_warning ("Unimplemented");
}

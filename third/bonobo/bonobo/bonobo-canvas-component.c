/*
 * bonobo-canvas-component.c: implements the CORBA interface for
 * the Bonobo::Canvas:Item interface used in Bonobo::Views.
 *
 * Author:
 *   Miguel de Icaza (miguel@kernel.org)
 *
 * (C) 1999 Helix Code, Inc.
 */
#include <stdio.h>
#include <config.h>
#include <gtk/gtksignal.h>
#include <bonobo/Bonobo.h>
#include <libgnomeui/gnome-canvas.h>
#include <gdk/gdkx.h>
#include <gdk/gdkprivate.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-ui-component.h>
#include <bonobo/bonobo-canvas-component.h>

enum {
	SET_BOUNDS,
	LAST_SIGNAL
};

static gint gcc_signals [LAST_SIGNAL] = { 0, };

typedef BonoboCanvasComponent Gcc;
#define GCC(x) BONOBO_CANVAS_COMPONENT(x)

struct _BonoboCanvasComponentPrivate {
	/*
	 * The item
	 */
	GnomeCanvasItem   *item;

	GnomeCanvasItem   *original_root;

	Bonobo_UIComponent ui_component;
};

/*
 * Returns the GnomeCanvasItemClass of an object
 */
#define ICLASS(x) GNOME_CANVAS_ITEM_CLASS ((GTK_OBJECT (x)->klass))

POA_Bonobo_Canvas_Component__vepv bonobo_canvas_component_vepv;

static BonoboObjectClass *gcc_parent_class;

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

	gnome_canvas_item_affine_absolute (item, affine);
	item->canvas->pixels_per_unit = state->pixels_per_unit;
	item->canvas->scroll_x1 = state->canvas_scroll_x1;
	item->canvas->scroll_y1 = state->canvas_scroll_y1;
	item->canvas->zoom_xofs = state->zoom_xofs;
	item->canvas->zoom_yofs = state->zoom_yofs;
	GTK_LAYOUT (item->canvas)->xoffset = state->xoffset;
	GTK_LAYOUT (item->canvas)->yoffset = state->yoffset;
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
	
	ICLASS (item)->update (item, (double *)aff, svp, flags);

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
				      Bonobo_Canvas_window_id window,
				      CORBA_Environment      *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));
	GnomeCanvasItem *item = GNOME_CANVAS_ITEM (gcc->priv->item);
	GdkWindow *gdk_window = gdk_window_foreign_new (window);

	if (gdk_window == NULL) {
		g_warning ("Invalid window id passed=0x%x", window);
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
		gdk_pixmap_unref (item->canvas->layout.bin_window);
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
	GdkWindowPrivate *priv = (GdkWindowPrivate *) pixmap;

	if (priv->ref_count != 1){
		g_warning ("This item is keeping a refcount to a foreign pixmap");
		return;
	}

	gdk_xid_table_remove (priv->xwindow);
	g_dataset_destroy (priv);
	g_free (priv);
}

static void
impl_Bonobo_Canvas_Component_draw (PortableServer_Servant        servant,
				   const Bonobo_Canvas_State    *state,
				   const Bonobo_Canvas_window_id drawable,
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
	pix = gdk_pixmap_foreign_new (drawable);

	if (pix == NULL){
		g_warning ("Invalid window id passed=0x%x", drawable);
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
	CORBA_boolean ret;
	
	if (getenv ("CC_DEBUG"))
		printf ("Point %g %g: ", x, y);
	ret = ICLASS (item)->point (item, x, y, 0, 0, &new_item) == 0.0;
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
		gdk_event->motion.xtilt = gnome_event->_u.motion.xtilt;
		gdk_event->motion.ytilt = gnome_event->_u.motion.ytilt;
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

static void
free_event (GdkEvent *event)
{
	if (event->type == GDK_KEY_RELEASE || event->type == GDK_KEY_PRESS)
		g_free (event->key.string);
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
	GdkEvent gdk_event;
	int retval;

	if (!ICLASS (item)->event)
		return FALSE;

	Bonobo_Gdk_Event_to_GdkEvent (gnome_event, &gdk_event);

	restore_state (item, state);
	retval = ICLASS (item)->event (item, &gdk_event);
	free_event (&gdk_event);

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
impl_Bonobo_Canvas_Component_setBounds (PortableServer_Servant     servant,
					const Bonobo_Canvas_DRect *bbox,
					CORBA_Environment         *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));

	gtk_signal_emit (GTK_OBJECT (gcc), gcc_signals [SET_BOUNDS], bbox, &ev);
}

static Bonobo_UIComponent
impl_Bonobo_Canvas_Component_getUIComponent (PortableServer_Servant   servant,
					     CORBA_Environment       *ev)
{
	Gcc *gcc = GCC (bonobo_object_from_servant (servant));

	return bonobo_object_dup_ref (gcc->priv->ui_component, ev);
}

/**
 * bonobo_canvas_component_set_ui_component:
 * @comp: 
 * @ui_component: 
 * 
 * Set a Bonobo_UIComponent
 **/
void
bonobo_canvas_component_set_ui_component (BonoboCanvasComponent *comp,
					  Bonobo_UIComponent     ui_component)
{
	g_return_if_fail (BONOBO_IS_CANVAS_COMPONENT (comp));

	bonobo_object_release_unref (comp->priv->ui_component, NULL);
	comp->priv->ui_component = bonobo_object_dup_ref (ui_component, NULL);
}

/**
 * bonobo_canvas_component_get_ui_component:
 * @comp: the canvas component
 * 
 * Get an associated Bonobo_UIComponent 
 * 
 * Return value: a UI component reference
 **/
Bonobo_UIComponent
bonobo_canvas_component_get_ui_component (BonoboCanvasComponent *comp)
					  
{
	g_return_val_if_fail (BONOBO_IS_CANVAS_COMPONENT (comp),
			      CORBA_OBJECT_NIL);

	return bonobo_object_dup_ref (comp->priv->ui_component, NULL);
}

/**
 * bonobo_canvas_component_get_epv:
 *
 * Returns: The EPV for the BonoboCanvasComponent.
 */
POA_Bonobo_Canvas_Component__epv *
bonobo_canvas_component_get_epv (void)
{
	POA_Bonobo_Canvas_Component__epv *epv;

	epv = g_new0 (POA_Bonobo_Canvas_Component__epv, 1);

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
	epv->getUIComponent = impl_Bonobo_Canvas_Component_getUIComponent;

	return epv;
}

static void
gcc_corba_class_init (void)
{
	/*
	 * Initialize the VEPV
	 */
	bonobo_canvas_component_vepv.Bonobo_Unknown_epv     = bonobo_object_get_epv ();
	bonobo_canvas_component_vepv.Bonobo_Canvas_Component_epv = bonobo_canvas_component_get_epv ();
}

static void
gcc_destroy (GtkObject *object)
{
	Gcc *gcc = GCC (object);

	bonobo_object_release_unref (gcc->priv->ui_component, NULL);

	GTK_OBJECT_CLASS (gcc_parent_class)->destroy (object);
}

static void
gcc_finalize (GtkObject *object)
{
	Gcc *gcc = GCC (object);

	g_free (gcc->priv);

	GTK_OBJECT_CLASS (gcc_parent_class)->finalize (object);
}

static void
gcc_class_init (GtkObjectClass *object_class)
{
	gcc_parent_class = gtk_type_class (bonobo_object_get_type ());

	gcc_corba_class_init ();
	object_class->destroy  = gcc_destroy;
	object_class->finalize = gcc_finalize;

	gcc_signals [SET_BOUNDS] = 
                gtk_signal_new ("set_bounds",
                                GTK_RUN_LAST,
                                object_class->type,
                                GTK_SIGNAL_OFFSET (BonoboCanvasComponentClass, set_bounds), 
                                gtk_marshal_NONE__POINTER_POINTER,
                                GTK_TYPE_NONE, 2,
				GTK_TYPE_POINTER, GTK_TYPE_POINTER);

	gtk_object_class_add_signals (object_class, gcc_signals, LAST_SIGNAL);
}

static void
gcc_init (GtkObject *object)
{
	Gcc *gcc = GCC (object);

	gcc->priv = g_new0 (BonoboCanvasComponentPrivate, 1);

	gcc->priv->ui_component = CORBA_OBJECT_NIL;
}

GtkType
bonobo_canvas_component_get_type (void)
{
	static GtkType type = 0;

	if (!type){
		GtkTypeInfo info = {
			"BonoboCanvasComponent",
			sizeof (BonoboCanvasComponent),
			sizeof (BonoboCanvasComponentClass),
			(GtkClassInitFunc) gcc_class_init,
			(GtkObjectInitFunc) gcc_init,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_object_get_type (), &info);
	}

	return type;
}

Bonobo_Canvas_Component
bonobo_canvas_component_object_create (BonoboObject *object)
{
	POA_Bonobo_Canvas_Component *servant;
	CORBA_Environment ev;
	
	servant = (POA_Bonobo_Canvas_Component *) g_new0 (BonoboObjectServant, 1);
	servant->vepv = &bonobo_canvas_component_vepv;

	CORBA_exception_init (&ev);
	POA_Bonobo_Canvas_Component__init ((PortableServer_Servant) servant, &ev);
	if (BONOBO_EX (&ev)){
		g_free (servant);
		CORBA_exception_free (&ev);
		return CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);
	return (Bonobo_View) bonobo_object_activate_servant (object, servant);
}

/**
 * bonobo_canvas_component_construct:
 * @comp: a #BonoboCanvasComponent to initialize
 * @corba_canvas_comp: A Bonobo_Canvas_Component corba object.
 * @item: A #GnomeCanvasItem that is being exported
 *
 * Creates a CORBA server for the interface Bonobo::Canvas::Item
 * wrapping @item.
 *
 * Returns: The BonoboCanvasComponent.
 */
BonoboCanvasComponent *
bonobo_canvas_component_construct (BonoboCanvasComponent  *comp,
				  Bonobo_Canvas_Component  corba_canvas_comp,
				  GnomeCanvasItem         *item)
{
	g_return_val_if_fail (comp != NULL, NULL);
	g_return_val_if_fail (BONOBO_IS_CANVAS_COMPONENT (comp), NULL);
	g_return_val_if_fail (corba_canvas_comp != NULL, NULL);
	g_return_val_if_fail (item != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_CANVAS_ITEM (item), NULL);

	bonobo_object_construct (BONOBO_OBJECT (comp), corba_canvas_comp);
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
	Bonobo_Canvas_Component corba_canvas_comp;
	
	g_return_val_if_fail (item != NULL, NULL);
	g_return_val_if_fail (GNOME_IS_CANVAS_ITEM (item), NULL);
	
	comp = gtk_type_new (bonobo_canvas_component_get_type ());
	corba_canvas_comp = bonobo_canvas_component_object_create (
		BONOBO_OBJECT (comp));

	if (corba_canvas_comp == CORBA_OBJECT_NIL){
		bonobo_object_unref (BONOBO_OBJECT (comp));
		return NULL;
	}
	return bonobo_canvas_component_construct (comp, corba_canvas_comp, item);
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

typedef struct {
	GnomeCanvasGroup       group;
	Bonobo_Canvas_ComponentProxy proxy;
} RootItemHack;

typedef struct {
	GnomeCanvasGroupClass parent_class;
} RootItemHackClass;

/*
 * Invoked by our local canvas when an update is requested,
 * we forward this to the container canvas
 */
static void
rih_update (GnomeCanvasItem *item, double affine [6], ArtSVP *svp, int flags)
{
	RootItemHack *rih = (RootItemHack *) item;
	CORBA_Environment ev;
	Bonobo_Canvas_ArtUTA *cuta;

	cuta = CORBA_UTA (item->canvas->redraw_area);

	CORBA_exception_init (&ev);
	Bonobo_Canvas_ComponentProxy_updateArea (rih->proxy, cuta, &ev);
	CORBA_free (cuta);
	CORBA_exception_free (&ev);

	/*
	 * Mark our canvas as fully updated
	 */
	if (item->canvas->redraw_area)
		art_uta_free (item->canvas->redraw_area);
	item->canvas->redraw_area = NULL;
	item->canvas->need_redraw = FALSE;
}

static void
rih_class_init (GnomeCanvasItemClass *item_class)
{
	item_class->update = rih_update;
}
      
static GtkType
root_item_hack_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"RootItemHack",
			sizeof (RootItemHack),
			sizeof (RootItemHackClass),
			(GtkClassInitFunc) rih_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved_1 */
			NULL, /* reserved_2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (gnome_canvas_group_get_type (), &info);
	}

	return type;
}

static RootItemHack *
root_item_hack_new (GnomeCanvas *canvas, Bonobo_Canvas_ComponentProxy proxy)
{
	RootItemHack *item_hack;

	item_hack = gtk_type_new (root_item_hack_get_type ());
	item_hack->proxy = proxy;
	GNOME_CANVAS_ITEM (item_hack)->canvas = canvas;

	return item_hack;
}

/**
 * bonobo_canvas_component_set_proxy:
 * @comp: A #BonoboCanvasComponent to operate on
 * @proxy: A Bonobo_Canvas_ComponentProxy CORBA object reference to our update proxy
 *
 * This routine sets the updating proxy for the @comp Canvas Component to be
 * the @proxy.
 *
 * This modifies the canvas bound to the BonoboCanvasComponent object to have
 * our 'filtering' root item for propagating the updates and repaints to the
 * container canvas.
 */
void
bonobo_canvas_component_set_proxy (BonoboCanvasComponent *comp, Bonobo_Canvas_ComponentProxy proxy)
{
	GnomeCanvas *canvas;
	
	g_return_if_fail (comp != NULL);
	g_return_if_fail (BONOBO_IS_CANVAS_COMPONENT (comp));

	canvas = comp->priv->item->canvas;
	
	comp->priv->original_root = canvas->root;
	canvas->root = GNOME_CANVAS_ITEM (root_item_hack_new (canvas, proxy));

	gtk_widget_realize (GTK_WIDGET (canvas));
	
	/*
	 * Gross
	 */
	GTK_WIDGET_SET_FLAGS (canvas, GTK_VISIBLE | GTK_MAPPED);
}

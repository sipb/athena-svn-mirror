/*
 * Primitive gesture recognition
 *
 * Author:
 *      Michael Meeks <mmeeks@gnu.org>
 */

#include <glib.h>
#include <math.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <gtkgesture.h>

typedef struct {
	char           *gesture;
	GtkGestureFunc  cb_func;
	gpointer        user_data_a;
	gpointer        user_data_b;
} Callback;

typedef struct {
	GtkGestureHandler  h;
	GList             *callbacks;
	GList             *points;
	gboolean           stroking;
	guint              button;
	gdouble            tlx, tly, brx, bry;
} GestureHandlerPriv;

typedef struct {
	gdouble x;
	gdouble y;
} GesturePoint;

static GestureHandlerPriv *
gesture_data_new (GtkWidget *widget, guint button)
{
	GestureHandlerPriv *ghp = g_new (GestureHandlerPriv, 1);

	ghp->h.widget  = widget;
	ghp->callbacks = NULL;
	ghp->points    = NULL;
	ghp->stroking  = FALSE;
	ghp->button    = button;

	ghp->tlx = 0.0;
	ghp->tly = 0.0;
	ghp->brx = 0.0;
	ghp->bry = 0.0;

	return ghp;
}

static void
gesture_data_free_points (GestureHandlerPriv *ghp)
{
	GList *l;
	g_return_if_fail (ghp != NULL);
	
	for (l = ghp->points; l; l = g_list_next (l))
		g_free (l->data);

	g_list_free (ghp->points);

	ghp->points = NULL;
}

static void
gesture_data_append_point (GestureHandlerPriv *ghp,
			  const gdouble * const x,
			  const gdouble * const y)
{
	GesturePoint *ghpoint;

	g_return_if_fail (ghp != NULL);
	ghpoint = g_new (GesturePoint, 1);

	ghpoint->x = *x;
	ghpoint->y = *y;

	/* Attempt at rendering */
	if (GTK_WIDGET_FLAGS (GTK_WIDGET (ghp->h.widget)) & GTK_NO_WINDOW)
		g_warning ("No window for the gesturing widget");
	else if (ghp->points) {
		GdkGC       *gc;
		GdkColor     col;
		GesturePoint *lghpoint = ghp->points->data;

		g_return_if_fail (lghpoint != NULL);
		g_return_if_fail (gdk_color_parse ("darkred", &col));

		gc = gdk_gc_new (ghp->h.widget->window);
		gdk_gc_set_foreground (gc, &col);
		gdk_gc_set_line_attributes (gc, 1, GDK_LINE_SOLID, GDK_CAP_BUTT,
					    GDK_JOIN_MITER);
		gdk_draw_line (ghp->h.widget->window, gc,
			       ghpoint->x, ghpoint->y,
			       lghpoint->x, lghpoint->y);
		g_object_unref (G_OBJECT (gc));
/*		g_message ("Draw line %d %d %d %d", (int)ghpoint->x,
			   (int)ghpoint->y,
			   (int)lghpoint->x,
			   (int)lghpoint->y); */
	}

	ghp->points = g_list_prepend (ghp->points, ghpoint);
}

/*
 * Normalize the gestures dimensions,
 * tlx, tly are purely for interest / debug.
 * The result is in the range 0-2, 0-2 x vs. y
 * There are good reasons for several passes.
 */
static void
gesture_normalize_dimensions (GestureHandlerPriv *ghp)
{
	GList   *l;
	gdouble  dx, dy;

	ghp->points = g_list_reverse (ghp->points);

	/* Find the top left corner */
	for (l = ghp->points; l; l = g_list_next (l)) {
		const GesturePoint * const ghpoint = l->data;

		if (!g_list_previous (l)) {
			ghp->tlx = ghpoint->x;
			ghp->tly = ghpoint->y;
			ghp->brx = ghpoint->x;
			ghp->bry = ghpoint->y;
		} else {
			if (ghpoint->x < ghp->tlx)
				ghp->tlx = ghpoint->x;
			if (ghpoint->y < ghp->tly)
				ghp->tly = ghpoint->y;
			if (ghpoint->x > ghp->brx)
				ghp->brx = ghpoint->x;
			if (ghpoint->y > ghp->bry)
				ghp->bry = ghpoint->y;
		}
	}

 	/*  Relativize [ John 3:18 ] to tlc */
	for (l = ghp->points; l; l = g_list_next (l)) {
		GesturePoint * const ghpoint = l->data;

		ghpoint->x -= ghp->tlx;
		ghpoint->y -= ghp->tly;
	}

	/* We want to detect gestures such as 012 */
	dx = ghp->brx - ghp->tlx;
	dy = ghp->bry - ghp->tly;
/*	g_message ("dx %f dy %f", dx, dy);*/
	if (dx > 2.0 * dy)
		dy = dx;
	else if (dy > 2.0 * dx)
		dx = dy;

	/* Normalize to side calculation. */
	for (l = ghp->points; l; l = g_list_next (l)) {
		GesturePoint * const ghpoint = l->data;

		ghpoint->x = (ghpoint->x * 3.0) / dx;
		ghpoint->y = (ghpoint->y * 3.0) / dy;
	}
}

/*
 * The resolution of this bitmap may need tweaking.
 */
#define GESTURE_BITMAP_MAX 4.0
int gesture_lookup[12][12] = {
	{ 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1 },
	{ 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1 },
	{ 1, 1, 1, 0,  0, 1, 1, 0,  0, 0, 1, 1 },
	{ 1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 1, 1 },
	{ 1, 1, 0, 0,  0, 1, 1, 0,  0, 0, 1, 1 },
	{ 1, 1, 1, 0,  1, 1, 1, 1,  0, 1, 1, 1 },
	{ 1, 1, 1, 0,  1, 1, 1, 1,  0, 1, 1, 1 },
	{ 1, 1, 0, 0,  0, 1, 1, 0,  0, 0, 1, 1 },
	{ 1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 1, 1 },
	{ 1, 1, 1, 0,  0, 1, 1, 0,  0, 1, 1, 1 },
	{ 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1 },
	{ 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1 }
};

static GArray *
gesture_analyze (GestureHandlerPriv *ghp)
{
	GArray  *ans = g_array_new (FALSE, FALSE, sizeof (int));

	gesture_normalize_dimensions (ghp);
/*	g_message ("The gesture data is %f %f %f %f",
	ghp->tlx, ghp->tly, ghp->brx, ghp->bry);*/
	gtk_widget_queue_draw_area (ghp->h.widget,
				    ghp->tlx, ghp->tly,
				    ghp->brx - ghp->tlx,
				    ghp->bry - ghp->tly);
	{ /* Gesture the data */
		guint  lastpos;
		GList *l;
		
		lastpos = -1;
		for (l = ghp->points; l; l = g_list_next (l)) {
			guint x, y, sx, sy;
			GesturePoint * const ghpoint = l->data;
			x  = floor (ghpoint->x * GESTURE_BITMAP_MAX);
			y  = floor (ghpoint->y * GESTURE_BITMAP_MAX);
			sx = floor (ghpoint->x);
			sy = floor (ghpoint->y);
			if (sx >= 3)
				sx = 2;
			if (sy >= 3)
				sy = 2;
			if (gesture_lookup[x][y] &&
			    (sx + sy * 3) != lastpos) {
				lastpos = sx + sy * 3;
				g_array_append_val (ans, lastpos);
/*				g_message (" (%f, %f: = [ %d %d])=%d ", ghpoint->x, ghpoint->y, sx, sy, lastpos);*/
			}
		}
	}

/*	for (i = 0; i < ans->len; i++)
		printf (" %d ", g_array_index (ans, int, i));
		printf ("\n");*/

	return ans;
}

/*
 * Expand to allow "63048|6308" style syntax.
 */
static gboolean
gesture_path_compare (GArray *path, const char * compare)
{
	gint       i, j;
	gchar    **gestures;
	gboolean   ans;

	if (!path || path->len == 0 || !compare || !compare[0])
		return FALSE;

	gestures = g_strsplit (compare, "|", -1);
	
	ans = FALSE;
	for (j = 0; gestures[j] && !ans; j++) {
		for (i = 0, ans = TRUE; gestures[j][i] && i < path->len && ans; i++) {
			if (gestures[j][i] - '0' != g_array_index (path, int, i))
				ans = FALSE;
		}

		if (ans && i < path->len)
			ans = FALSE;

		g_free (gestures[j]);
	}

	while (gestures[j])
		g_free (gestures[j++]);
	g_free (gestures);

	return ans;
}

static gint
gesture_event (GtkWidget *widget, GdkEvent *event,
	       GestureHandlerPriv *ghp)
{
	gint handled = FALSE;

	g_return_val_if_fail (ghp != NULL, FALSE);
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	switch (event->type) {
	case GDK_BUTTON_PRESS:
		if (event->button.button == ghp->button) {
			ghp->stroking = TRUE;
			gesture_data_free_points (ghp);
			handled = TRUE;
		}
		break;
	case GDK_LEAVE_NOTIFY:
		if (!ghp->stroking)
			break;
		/* else drop down */
	case GDK_BUTTON_RELEASE:
		if (event->button.button == ghp->button) {
			GArray *path;
			GList  *l;

			ghp->stroking = FALSE;

			path = gesture_analyze (ghp);

			for (l = ghp->callbacks; l; l = g_list_next (l)) {
				Callback *cb = (Callback *)l->data;
				if (gesture_path_compare (path, cb->gesture))
					cb->cb_func ((GtkGestureHandler *)ghp,
						     cb->user_data_a,
						     cb->user_data_b);
			}

			g_array_free (path, TRUE);
			
			handled = TRUE;
		}
		break;
	case GDK_MOTION_NOTIFY:
		if (ghp->stroking) {
			gesture_data_append_point (ghp, &event->motion.x,
						  &event->motion.y);
			handled = TRUE;
		}
		break;
	default:
		break;
	}

	return handled;
}

void
gtk_gesture_handler_attach (GtkGestureHandler *gh,
			    GtkWidget *widget)
{
/*	g_message ("Connecting gesture event handler");*/
	gtk_widget_set_events (GTK_WIDGET (widget),
			       GDK_POINTER_MOTION_MASK |
			       GDK_BUTTON_MOTION_MASK |
			       GDK_BUTTON_PRESS_MASK |
	                       GDK_LEAVE_NOTIFY_MASK);
	g_signal_connect (G_OBJECT (widget), "event",
			  G_CALLBACK (gesture_event), gh);
}

GtkGestureHandler *
gtk_gesture_handler_new (GtkWidget *widget)
{
	GtkGestureHandler *gh;
      
	g_return_val_if_fail (widget != NULL, NULL);

	gh = (GtkGestureHandler *)gesture_data_new (widget, 2);

	gtk_gesture_handler_attach (gh, widget);

	return gh;
}

void
gtk_gesture_add_callback (GtkGestureHandler *gh,
			  const char        *gesture,
			  GtkGestureFunc     cb_func,
			  gpointer           user_data_a,
			  gpointer           user_data_b)
{
	Callback *cb;
	GestureHandlerPriv *ghp = (GestureHandlerPriv *)gh;

	g_return_if_fail (gh != NULL);
	g_return_if_fail (cb_func != NULL);
	g_return_if_fail (gesture != NULL);

	cb              = g_new (Callback, 1);
	cb->gesture     = g_strdup (gesture);
	cb->cb_func     = cb_func;
	cb->user_data_a = user_data_a;
	cb->user_data_b = user_data_b;

	ghp->callbacks  = g_list_prepend (ghp->callbacks, cb);
}

void
gtk_gesture_handler_destroy (GtkGestureHandler *gh)
{
	GestureHandlerPriv *ghp = (GestureHandlerPriv *)gh;

	while (ghp->callbacks) {
		Callback *cb = ghp->callbacks->data;

		if (cb->gesture)
			g_free (cb->gesture);
		cb->gesture = NULL;
		g_free (cb);

		ghp->callbacks = g_list_remove (ghp->callbacks, cb);
	}

	if (ghp->points)
		gesture_data_free_points (ghp);

	g_free (ghp);
}

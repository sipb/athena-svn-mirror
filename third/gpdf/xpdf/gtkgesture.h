/*
 * Primitive gesture recognition
 *
 * Author:
 *      Michael Meeks <mmeeks@gnu.org>
 */

/*
 *   A gesture is described by the path traced in 9 regions which are
 * numbered thus:
 *
 *   0  1  2       So to describe a standard 'zoom in' gesture we would
 *   3  4  5       have "048", for a standard 'help' stroke we might have
 *   6  7  8       "3157|3012587|3012547" for example tracing a '?' shape.
 */

#ifndef GTK_GESTURE_HANDLER_H
#define GTK_GESTURE_HANDLER_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct {
	GtkWidget *widget;
} GtkGestureHandler;

typedef void (*GtkGestureFunc) (GtkGestureHandler *gh, gpointer user_data_a,
				gpointer user_data_b);
#define GTK_GESTURE_FUNC(g)    ((GtkGestureFunc)(g))

GtkGestureHandler *gtk_gesture_handler_new     (GtkWidget *widget);
void               gtk_gesture_handler_attach  (GtkGestureHandler *gh,
						GtkWidget *widget);
void               gtk_gesture_add_callback    (GtkGestureHandler *gh,
						const char *gesture,
						GtkGestureFunc cb_func,
						gpointer user_data_a,
						gpointer user_data_b);
void               gtk_gesture_handler_destroy (GtkGestureHandler *gh);

G_END_DECLS

#endif /* GTK_GESTURE_HANDLER_H */

#ifndef __GST_XWINDOW_H__
#define __GST_XWINDOW_H__

#include <glib.h>
#include <X11/Xlib.h>

G_BEGIN_DECLS

typedef struct _GstXWindow	      GstXWindow;

#define GST_XWINDOW_XID(window)	((window)->win)

struct _GstXWindow
{
  Screen *screen;
  Display *disp;
  Window root, win;
  gulong white, black;
  gint screen_num;
  gint width, height;
  gint depth;
  GC gc;
};


GstXWindow* 	_gst_xwindow_new 	(gint width, gint height, gboolean toplevel);
void 		_gst_xwindow_destroy 	(GstXWindow *window);

void 		_gst_xwindow_resize 	(GstXWindow *window, gint width, gint height);

G_END_DECLS

#endif /* __GST_XWINDOW_H__ */

/* gcc -ansi -pedantic on GNU/Linux causes warnings and errors
 * unless this is defined:
 * warning: #warning "Files using this header must be compiled with _SVID_SOURCE or _XOPEN_SOURCE"
 */
#ifndef _XOPEN_SOURCE
#  define _XOPEN_SOURCE 1
#endif

#include <X11/Xlib.h>
#include "gstvideosink.h"
#include <string.h> /* strncmp */

typedef struct _GstXImageInfo GstXImageInfo;
struct _GstXImageInfo {
  GstImageInfo info;
  Display *display;
  Window window;
  GC gc;
  gint x, y, w, h;
  /* window specific from here */
  GstElement *sink;
  gulong handler_id;
};

static void
gst_xwindow_free (GstImageInfo *info)
{
  GstXImageInfo *window = (GstXImageInfo *) info;
  
  g_signal_handler_disconnect (window->sink, window->handler_id);
  XFreeGC (window->display, window->gc);
  XCloseDisplay (window->display);
  g_free (window);
}
static void
gst_xwindow_callback(GObject *object, GParamSpec *pspec, GstXImageInfo *data)
{
  XWindowAttributes attr; 
  XGetWindowAttributes(data->display, data->window, &attr); 

  if (strncmp (pspec->name, "width", 5) == 0 || strncmp (pspec->name, "height", 6) == 0)
  {
    gint w = 0;
    gint h = 0;
    g_object_get (object, "width", &w, NULL);
    g_object_get (object, "height", &h, NULL);
    if (w > attr.width || h > attr.height)
    {
      attr.width = w;
      attr.height = h;
      XResizeWindow (data->display, data->window, attr.width, attr.height);
      XMapRaised (data->display, data->window);
    }
  }
  if (attr.width != data->w || attr.height != data->h)
  {
    data->w = attr.width;
    data->h = attr.height;
  }
}
void
gst_xwindow_new (GstElement *sink)
{
  XGCValues values;
  GstXImageInfo *new;
  XSetWindowAttributes attrib;
  
  new = g_new0 (GstXImageInfo, 1);

  if (sink == NULL)
  {
    sink = gst_element_factory_make ("videosink", "videosink");
    g_assert (sink != NULL);
  }
  
  /* fill in the ImageInfo */
  new->info.id = GST_MAKE_FOURCC ('X', 'l', 'i', 'b');
  new->info.free_info = gst_xwindow_free;
  
  new->display = XOpenDisplay (NULL);
  if (!new->display) {
    g_warning ("open display failed!\n");
    g_free (new);
    return;
  }

  /* set sizes */
  new->x = 0;
  new->y = 0;
  new->w = 10;
  new->h = 10;

  attrib.background_pixel = XBlackPixel (new->display, DefaultScreen (new->display));
  new->window = XCreateWindow (new->display, DefaultRootWindow (new->display), 
			       new->x, new->y, new->w, new->h, 0, CopyFromParent, 
			       CopyFromParent, CopyFromParent, CWBackPixel, &attrib);
  
  if (!new->window) {
    g_warning ("create window failed\n");
    g_free (new);
    return;
  }

  XSelectInput (new->display, new->window, ExposureMask | StructureNotifyMask);

  new->gc = XCreateGC (new->display, new->window, 0, &values);

  g_object_set (sink, "hook", new, NULL);
  new->sink = sink;
  new->handler_id = g_signal_connect (sink, "notify", G_CALLBACK (gst_xwindow_callback), new);
}


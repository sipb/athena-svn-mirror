#ifndef __GST_XIMAGE_H__
#define __GST_XIMAGE_H__

#include "config.h"

#define USE_SHM

#include <glib.h> 
#include <X11/Xlib.h> 

#ifdef USE_SHM
#include <X11/extensions/XShm.h>
#endif /* USE_SHM */

#include "gstxwindow.h"

#include "gstimage.h"

G_BEGIN_DECLS

#define GST_XIMAGE(img)		((GstXImage*)(img))

typedef struct _GstXImage	      GstXImage;

#define GST_XIMAGE_BPP(img)		((img)->ximage->bits_per_pixel)
#define GST_XIMAGE_DEPTH(img)		((img)->ximage->depth)
#define GST_XIMAGE_ENDIANNESS(img)	((img)->endianness)
#define GST_XIMAGE_RED_MASK(img)	((img)->visual->red_mask)
#define GST_XIMAGE_GREEN_MASK(img)	((img)->visual->green_mask)
#define GST_XIMAGE_BLUE_MASK(img)	((img)->visual->blue_mask)

struct _GstXImage
{
  GstImage parent;

  GstXWindow *window;
#ifdef USE_SHM
  XShmSegmentInfo SHMInfo;
#else
  gpointer *SHMInfo;
#endif
  XImage *ximage;
  Visual *visual;
  gint width, height;
  gulong endianness;
};

GstXImage*	_gst_ximage_new    	(GstXWindow *window, gint width, gint height);

G_END_DECLS

#endif /* __GST_XIMAGE_H__ */

#ifndef __GST_XVIMAGE_H__
#define __GST_XVIMAGE_H__

#include <glib.h>
#include <X11/Xlib.h>

#ifdef HAVE_XVIDEO
# include <X11/extensions/Xv.h>
# include <X11/extensions/Xvlib.h>
#endif

#include "gstxwindow.h"

#include "gstimage.h"

G_BEGIN_DECLS

typedef struct _GstXvImage	      GstXvImage;
typedef guint64                       GstXvImageType;

#define GST_XVIMAGE(img)	((GstXvImage *)(img))

struct _GstXvImage
{
  GstImage parent;

  GstXWindow *window;
  GstXvImageType  type;
#ifdef HAVE_XVIDEO
  XvImage *ximage;
#else
  gpointer ximage;
#endif
  gpointer x_shm_info;
  gint im_adaptor;
  gint im_port;
  gint im_format;
  gint width, height;
};

void 		_gst_xvimage_init		(void);
gboolean 	_gst_xvimage_check_xvideo	(void);

GstCaps* 	_gst_xvimage_get_capslist	(void);

GstXvImage*  	_gst_xvimage_new    		(GstXvImageType type,
						 GstXWindow *window,
				 		 gint width,
				 		 gint height);

G_END_DECLS

#endif /* __GST_XVIMAGE_H__ */

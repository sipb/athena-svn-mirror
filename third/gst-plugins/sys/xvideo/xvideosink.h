/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __GST_XVIDEOSINK_H__
#define __GST_XVIDEOSINK_H__

#include <gst/gst.h>

#include "gstxwindow.h"
#include "gstxvimage.h"
#include "gstximage.h"

G_BEGIN_DECLS

#define GST_TYPE_XVIDEOSINK \
  (gst_xvideosink_get_type())
#define GST_XVIDEOSINK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_XVIDEOSINK,GstXVideoSink))
#define GST_XVIDEOSINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_XVIDEOSINK,GstXVideoSink))
#define GST_IS_XVIDEOSINK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_XVIDEOSINK))
#define GST_IS_XVIDEOSINK_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_XVIDEOSINK))

#define MAX_FLIP_BUFFERS 1

typedef struct _GstXVideoSink GstXVideoSink;
typedef struct _GstXVideoSinkClass GstXVideoSinkClass;

struct _GstXVideoSink {
  GstElement 	 element;

  GstPad 	*sinkpad;

  GstXWindow 	*window;
  GstImage 	*image;

  guint32 	 format;
  gint 		 width, height;
  gint 		 pixel_width, pixel_height;
  gint64 	 correction;
  GstClockID	 id;

  gint 	 	 frames_displayed;
  guint64 	 frame_time;
  gboolean 	 disable_xv;
  gboolean 	 toplevel;
  gboolean 	 send_xid;
  gboolean 	 need_new_window;

  GstClock 	*clock;
  GstCaps 	*formats;
  gboolean 	 auto_size;

  GstBufferPool *bufferpool;
  GMutex 	*lock;
  GSList 	*image_pool;
  GMutex 	*pool_lock;
};

struct _GstXVideoSinkClass {
  GstElementClass parent_class;

  /* signals */
  void (*frame_displayed) (GstElement *element);
  void (*have_size) 	  (GstElement *element, gint width, gint height);
  void (*have_xid) 	  (GstElement *element, gint xid);
};

GType gst_xvideosink_get_type(void);

G_END_DECLS

#endif /* __GST_XVIDEOSINK_H__ */

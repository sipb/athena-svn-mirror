/* GStreamer VGA plugin
 * Copyright (C) 2001 Ronald Bultje <rbultje@ronald.bitfreak.net>
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


#ifndef __GST_VGAVIDEOSINK_H__
#define __GST_VGAVIDEOSINK_H__

#include <gst/gst.h>

#include <vga.h>
#include <vgagl.h>

G_BEGIN_DECLS

#define GST_TYPE_VGAVIDEOSINK \
  (gst_vgavideosink_get_type())
#define GST_VGAVIDEOSINK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_VGAVIDEOSINK,GstVGAVideoSink))
#define GST_VGAVIDEOSINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_VGAVIDEOSINK,GstVGAVideoSink))
#define GST_IS_VGAVIDEOSINK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_VGAVIDEOSINK))
#define GST_IS_VGAVIDEOSINK_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_VGAVIDEOSINK))

typedef enum {
  GST_VGAVIDEOSINK_OPEN = GST_ELEMENT_FLAG_LAST,
  GST_VGAVIDEOSINK_FLAG_LAST = GST_ELEMENT_FLAG_LAST + 2,
} GstVGAVideoSinkFlags;

typedef struct _GstVGAVideoSink GstVGAVideoSink;
typedef struct _GstVGAVideoSinkClass GstVGAVideoSinkClass;

struct _GstVGAVideoSink {
  GstElement element;

  GstPad *sinkpad;

  gulong format;
  gint width, height;

  GraphicsContext *physicalscreen;
  GraphicsContext *virtualscreen;

  gint frames_displayed;
  guint64 frame_time;

  GstClock *clock;

};

struct _GstVGAVideoSinkClass {
  GstElementClass parent_class;

  /* signals */
  void (*frame_displayed) (GstElement *element);
  void (*have_size) 	  (GstElement *element, guint width, guint height);
};

GType gst_vgasink_get_type(void);

G_END_DECLS

#endif /* __GST_VGAVIDEOSINK_H__ */

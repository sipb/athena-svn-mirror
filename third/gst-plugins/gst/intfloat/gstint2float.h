/* GStreamer
 * Copyright (C) <2001> Steve Baker <stevebaker_org@yahoo.co.uk>
 *
 * int2float.h
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


#ifndef __GST_INT2FLOAT_H__
#define __GST_INT2FLOAT_H__


#include <config.h>
#include <gst/gst.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GST_TYPE_INT2FLOAT \
  (gst_int2float_get_type())
#define GST_INT2FLOAT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_INT2FLOAT,GstInt2Float))
#define GST_INT2FLOAT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_INT2FLOAT,GstInt2Float))
#define GST_IS_INT2FLOAT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_INT2FLOAT))
#define GST_IS_INT2FLOAT_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_INT2FLOAT))

#define GST_INT2FLOAT_SRCPAD(list)   ((GstPad*)(list->data))

typedef struct _GstInt2Float GstInt2Float;
typedef struct _GstInt2FloatClass GstInt2FloatClass;

struct _GstInt2Float {
  GstElement element;

  GstPad *sinkpad;
  GSList *srcpads;
  
  gint numsrcpads;
  
  GstCaps *intcaps;
  GstCaps *floatcaps;
  gint channels, rate;
  gboolean in_capsnego;
};

struct _GstInt2FloatClass {
  GstElementClass parent_class;
};

GType gst_int2float_get_type(void);
gboolean gst_int2float_factory_init (GstPlugin *plugin);
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GST_INT2FLOAT_H__ */

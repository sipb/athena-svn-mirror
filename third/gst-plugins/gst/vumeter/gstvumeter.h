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


#ifndef __GST_VUMETER_H__
#define __GST_VUMETER_H__


#include <config.h>
#include <gst/gst.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GST_TYPE_VUMETER \
  (gst_vumeter_get_type())
#define GST_VUMETER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_VUMETER,GstVuMeter))
#define GST_VUMETER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_VUMETER,GstVuMeter))
#define GST_IS_VUMETER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_VUMETER))
#define GST_IS_VUMETER_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_VUMETER))

typedef struct _GstVuMeter GstVuMeter;
typedef struct _GstVuMeterClass GstVuMeterClass;

struct _GstVuMeter {
  GstElement element;

  GstPad *sinkpad;

  gint32 volume;
  gint32 volume_left;
  gint32 volume_right;
};

struct _GstVuMeterClass {
  GstElementClass parent_class;
};

GType gst_vumeter_get_type(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GST_VUMETER_H__ */
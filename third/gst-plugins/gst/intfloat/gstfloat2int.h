/* GStreamer
 * Copyright (C) <2001> Steve Baker <stevebaker_org@yahoo.co.uk>
 *
 * float2int.h
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


#ifndef __GST_FLOAT2INT_H__
#define __GST_FLOAT2INT_H__


#include <config.h>
#include <gst/gst.h>
#include <gst/bytestream/bytestream.h>


#define GST_TYPE_FLOAT2INT \
  (gst_float2int_get_type())
#define GST_FLOAT2INT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_FLOAT2INT,GstFloat2Int))
#define GST_FLOAT2INT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_FLOAT2INT,GstFloat2Int))
#define GST_IS_FLOAT2INT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_FLOAT2INT))
#define GST_IS_FLOAT2INT_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_FLOAT2INT))

#define GST_FLOAT2INT_CHANNEL(list)   ((GstFloat2IntInputChannel*)(list->data))

typedef struct _GstFloat2Int GstFloat2Int;
typedef struct _GstFloat2IntClass GstFloat2IntClass;
typedef struct _GstFloat2IntInputChannel GstFloat2IntInputChannel;

struct _GstFloat2IntInputChannel {
  GstPad        *sinkpad;
  GstByteStream *bytestream;
  gboolean eos;
};

struct _GstFloat2Int {
  GstElement element;

  GstPad *srcpad;
  GSList *channels;
  GstBufferPool *pool;
  
  gint numchannels; /* Number of pads on the element */
  gint channelcount; /* counter to get safest pad name */
  
  GstCaps *intcaps;
  GstCaps *floatcaps;
  gint rate;
  gint64 offset;
};

struct _GstFloat2IntClass {
  GstElementClass parent_class;
};

GType gst_float2int_get_type(void);
gboolean gst_float2int_factory_init (GstPlugin *plugin);


#endif /* __GST_FLOAT2INT_H__ */

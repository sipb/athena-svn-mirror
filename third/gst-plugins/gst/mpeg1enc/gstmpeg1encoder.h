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


#ifndef __GST_MPEG1ENCODER_H__
#define __GST_MPEG1ENCODER_H__


#include <config.h>
#include <gst/gst.h>
#include "globals.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GST_TYPE_MPEG1ENCODER \
  (gst_mpeg1encoder_get_type())
#define GST_MPEG1ENCODER(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MPEG1ENCODER,GstMpeg1encoder))
#define GST_MPEG1ENCODER_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_MPEG1ENCODER,GstMpeg1encoder))
#define GST_IS_MPEG1ENCODER(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_MPEG1ENCODER))
#define GST_IS_MPEG1ENCODER_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_MPEG1ENCODER))

typedef struct _GstMpeg1encoder GstMpeg1encoder;
typedef struct _GstMpeg1encoderClass GstMpeg1encoderClass;

struct _GstMpeg1encoder {
  GstElement element;

  /* pads */
  GstPad *sinkpad,*srcpad;

  mpeg1encoder_VidStream *encoder;
  int state;
  /* the timestamp of the next frame */
  guint64 next_time;
  /* the interval between frames */
  guint64 time_interval;

  /* video state */
  gint format;
  gint width;
  gint height;
  /* the size of the output buffer */
  gint outsize;

};

struct _GstMpeg1encoderClass {
  GstElementClass parent_class;
};

GType gst_mpeg1encoder_get_type(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GST_MPEG1ENCODER_H__ */

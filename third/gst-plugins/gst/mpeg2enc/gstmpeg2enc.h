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


#ifndef __GST_MPEG2ENC_H__
#define __GST_MPEG2ENC_H__


#include <config.h>
#include <gst/gst.h>
#include "mpeg2enc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GST_TYPE_MPEG2ENC \
  (gst_mpeg2enc_get_type())
#define GST_MPEG2ENC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MPEG2ENC,GstMpeg2enc))
#define GST_MPEG2ENC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_MPEG2ENC,GstMpeg2enc))
#define GST_IS_MPEG2ENC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_MPEG2ENC))
#define GST_IS_MPEG2ENC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_MPEG2ENC))

typedef struct _GstMpeg2enc GstMpeg2enc;
typedef struct _GstMpeg2encClass GstMpeg2encClass;

struct _GstMpeg2enc {
  GstElement element;

  /* pads */
  GstPad *sinkpad,*srcpad;

  mpeg2enc_vid_stream *encoder;
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

struct _GstMpeg2encClass {
  GstElementClass parent_class;
};

GType gst_mpeg2enc_get_type(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GST_MPEG2ENC_H__ */

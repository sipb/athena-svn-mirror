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


#ifndef __GST_WINENC_H__
#define __GST_WINENC_H__


#include "config.h"

#include <videoencoder.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gst/gst.h>


#define GST_TYPE_WINENC \
  (gst_winenc_get_type())
#define GST_WINENC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_WINENC,GstWinEnc))
#define GST_WINENC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_WINENC,GstWinEnc))
#define GST_IS_WINENC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_WINENC))
#define GST_IS_WINENC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_WINENC))

typedef struct _GstWinEnc GstWinEnc;
typedef struct _GstWinEncClass GstWinEncClass;

struct _GstWinEnc {
  GstElement element;

  /* pads */
  GstPad *sinkpad,*srcpad;

  /* video state */
  gulong format;
  gint width;
  gint height;

  gint bitrate, quality, keyframe;
  gulong compression;

  gulong last_frame_size;
  BITMAPINFOHEADER bh;
  IVideoEncoder *encoder;
};

struct _GstWinEncClass {
  GstElementClass parent_class;

  /* signals */
  void (*frame_encoded) (GstElement *element, GstPad *pad);
};

GType gst_winenc_get_type(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GST_WINENC_H__ */

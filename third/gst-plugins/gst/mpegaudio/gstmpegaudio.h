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


#ifndef __GST_MPEGAUDIO_H__
#define __GST_MPEGAUDIO_H__


#include <gst/gst.h>

#include "musicin.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GST_TYPE_MPEGAUDIO \
  (gst_mpegaudio_get_type())
#define GST_MPEGAUDIO(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MPEGAUDIO,GstMpegAudio))
#define GST_MPEGAUDIO_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_MPEGAUDIO,GstMpegAudio))
#define GST_IS_MPEGAUDIO(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_MPEGAUDIO))
#define GST_IS_MPEGAUDIO_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_MPEGAUDIO))

typedef struct _GstMpegAudio GstMpegAudio;
typedef struct _GstMpegAudioClass GstMpegAudioClass;

struct _GstMpegAudio {
  GstElement element;

  /* pads */
  GstPad *sinkpad,*srcpad;

  /* state */
  struct mpegaudio_encoder *encoder;

  guchar *partialbuf;
  gulong partialsize;
};

struct _GstMpegAudioClass {
  GstElementClass parent_class;
};

GType gst_mpegaudio_get_type(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GST_MPEGAUDIO_H__ */

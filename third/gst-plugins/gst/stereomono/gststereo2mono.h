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


#ifndef __GST_STEREO2MONO_H__
#define __GST_STEREO2MONO_H__


#include <config.h>
#include <gst/gst.h>
/* #include <gst/meta/audioraw.h> */


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GST_TYPE_STEREO2MONO \
  (gst_stereo2mono_get_type())
#define GST_STEREO2MONO(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_STEREO2MONO,GstStereo2Mono))
#define GST_STEREO2MONO_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_ULAW,GstStereo2Mono))
#define GST_IS_STEREO2MONO(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_STEREO2MONO))
#define GST_IS_STEREO2MONO_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_STEREO2MONO))

typedef struct _GstStereo2Mono GstStereo2Mono;
typedef struct _GstStereo2MonoClass GstStereo2MonoClass;

struct _GstStereo2Mono {
  GstElement element;

  GstPad *sinkpad,*srcpad;

  /*MetaAudioRaw meta; */

  gint width;
};

struct _GstStereo2MonoClass {
  GstElementClass parent_class;
};

GType gst_stereo2mono_get_type(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GST_STEREO_H__ */

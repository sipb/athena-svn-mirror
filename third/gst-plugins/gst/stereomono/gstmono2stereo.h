/* GStreamer
 * Copyright (C) <2001> Steve Baker <stevebaker_org@yahoo.co.uk>
 *
 * mono2stereo.c
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


#ifndef __GST_MONO2STEREO_H__
#define __GST_MONO2STEREO_H__


#include <config.h>
#include <gst/gst.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GST_TYPE_MONO2STEREO \
  (gst_mono2stereo_get_type())
#define GST_MONO2STEREO(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MONO2STEREO,GstMono2Stereo))
#define GST_MONO2STEREO_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_MONO2STEREO,GstMono2Stereo))
#define GST_IS_MONO2STEREO(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_MONO2STEREO))
#define GST_IS_MONO2STEREO_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_MONO2STEREO))

typedef struct _GstMono2Stereo GstMono2Stereo;
typedef struct _GstMono2StereoClass GstMono2StereoClass;

struct _GstMono2Stereo {
  GstElement element;

  GstPad *sinkpad,*srcpad;
  
  gint width;

  /* pan can be between -1.0 and 1.0 */
  gfloat pan;
  gfloat pan_left;
  gfloat pan_right;
};

struct _GstMono2StereoClass {
  GstElementClass parent_class;
};

GType gst_mono2stereo_get_type(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GST_MONO2STEREO_H__ */

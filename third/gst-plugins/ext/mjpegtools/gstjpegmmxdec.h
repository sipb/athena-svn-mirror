/* GStreamer JPEG/MMX decoder plugin
 * Copyright (C) 2002 Ronald Bultje <rbultje@ronald.bitfreak.net>
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


#ifndef __GST_JPEGMMXDEC_H__
#define __GST_JPEGMMXDEC_H__


#include <config.h>
#include <gst/gst.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GST_TYPE_JPEGMMXDEC \
  (gst_jpegmmxdec_get_type())
#define GST_JPEGMMXDEC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_JPEGMMXDEC, GstJpegMMXDec))
#define GST_JPEGMMXDEC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_JPEGMMXDEC, GstJpegMMXDec))
#define GST_IS_JPEGMMXDEC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_JPEGMMXDEC))
#define GST_IS_JPEGMMXDEC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_JPEGMMXDEC))

typedef struct _GstJpegMMXDec GstJpegMMXDec;
typedef struct _GstJpegMMXDecClass GstJpegMMXDecClass;

struct _GstJpegMMXDec {
  GstElement element;

  /* pads */
  GstPad *sinkpad, *srcpad;

  /* video state */
  gint width;
  gint height;
};

struct _GstJpegMMXDecClass {
  GstElementClass parent_class;
};

GType gst_jpegmmxdec_get_type(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GST_JPEGMMXDEC_H__ */

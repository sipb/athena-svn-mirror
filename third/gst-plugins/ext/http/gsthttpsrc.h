/* GStreamer
 * Copyright (C) 1999,2000 Erik Walthinsen <omega@cse.ogi.edu>
 *                    2000 Wim Taymans <wtay@chello.be>
 *
 * gsthttpsrc.h: 
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


#ifndef __GST_HTTPSRC_H__
#define __GST_HTTPSRC_H__


#include <config.h>
#include <gst/gst.h>

#include <ghttp.h>


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


GstElementDetails gst_httpsrc_details;


#define GST_TYPE_HTTPSRC \
  (gst_httpsrc_get_type())
#define GST_HTTPSRC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_HTTPSRC,GstHttpSrc))
#define GST_HTTPSRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_HTTPSRC,GstHttpSrcClass))
#define GST_IS_HTTPSRC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_HTTPSRC))
#define GST_IS_HTTPSRC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_HTTPSRC))

typedef enum {
  GST_HTTPSRC_OPEN		= GST_ELEMENT_FLAG_LAST,

  GST_HTTPSRC_FLAG_LAST		= GST_ELEMENT_FLAG_LAST+2,
} GstHttpSrcFlags;

typedef struct _GstHttpSrc GstHttpSrc;
typedef struct _GstHttpSrcClass GstHttpSrcClass;

struct _GstHttpSrc {
  GstElement element;
  /* pads */
  GstPad *srcpad;

  gchar *url;
  ghttp_request *request;
  int fd;

  gulong curoffset;			/* current offset in file */
  gulong bytes_per_read;		/* bytes per read */
};

struct _GstHttpSrcClass {
  GstElementClass parent_class;
};

GType gst_httpsrc_get_type(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GST_HTTPSRC_H__ */
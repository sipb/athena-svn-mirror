/* GStreamer
 * Copyright (C) 1999,2000 Erik Walthinsen <omega@cse.ogi.edu>
 *                    2000 Wim Taymans <wtay@chello.be>
 *
 * gstossgst.h: 
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


#ifndef __GST_OSSGST_H__
#define __GST_OSSGST_H__


#include <config.h>
#include <gst/gst.h>

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define GST_TYPE_OSSGST \
  (gst_ossgst_get_type())
#define GST_OSSGST(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_OSSGST,GstOssGst))
#define GST_OSSGST_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_OSSGST,GstOssGstClass))
#define GST_IS_OSSGST(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_OSSGST))
#define GST_IS_OSSGST_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_OSSGST))

typedef enum {
  GST_OSSGST_OPEN		= GST_ELEMENT_FLAG_LAST,

  GST_OSSGST_FLAG_LAST		= GST_ELEMENT_FLAG_LAST+2,
} GstOssGstFlags;

typedef struct _GstOssGst GstOssGst;
typedef struct _GstOssGstClass GstOssGstClass;

struct _GstOssGst {
  GstElement element;

  GstPad *srcpad;

  gint fdout[2];
  gint fdin[2];
  pid_t   childpid;

  /* soundcard state */
  gboolean mute;
  gchar *command;
};

struct _GstOssGstClass {
  GstElementClass parent_class;

  /* signals */
};

GType gst_ossgst_get_type(void);

gboolean gst_ossgst_factory_init(GstPlugin *plugin);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GST_OSSGST_H__ */
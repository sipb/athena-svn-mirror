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


#ifndef __GST_WINDEC_H__
#define __GST_WINDEC_H__


#include "config.h"

#include <stdlib.h>
#include <avifile.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gst/gst.h>

#define GST_TYPE_WINDEC \
  (gst_windec_get_type())
#define GST_WINDEC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_WINDEC,GstWinDec))
#define GST_WINDEC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_WINDEC,GstWinDec))
#define GST_IS_WINDEC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_WINDEC))
#define GST_IS_WINDEC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_WINDEC))

typedef struct _GstWinDec GstWinDec;
typedef struct _GstWinDecClass GstWinDecClass;

struct _GstWinDec {
  GstElement element;

  /* pads */
  GstPad *sinkpad, *srcpad;

  GstBufferPool *pool;

  BITMAPINFOHEADER bh, obh;
  IVideoDecoder *decoder;
};

struct _GstWinDecClass {
  GstElementClass parent_class;
};

GType gst_windec_get_type (void);
	
#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GST_WINDEC_H__ */

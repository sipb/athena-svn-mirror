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

#ifndef __GST_IMAGE_H__
#define __GST_IMAGE_H__

G_BEGIN_DECLS

#define GST_IMAGE(img)			((GstImage *)(img))
#define GST_IMAGE_TYPE(img)		(GST_IMAGE (img)->type)
#define GST_IMAGE_DATA(img)		(GST_IMAGE (img)->data)
#define GST_IMAGE_SIZE(img)		(GST_IMAGE (img)->size)
#define GST_IMAGE_DESTROYFUNC(img)	(GST_IMAGE (img)->destroyfunc)
#define GST_IMAGE_PUTFUNC(img)		(GST_IMAGE (img)->putfunc)

typedef struct _GstImage	GstImage;

typedef enum {
  GST_TYPE_XIMAGE,
  GST_TYPE_XVIMAGE,
} GstImageType;

typedef void 	(*GstImageDestroyFunc) 		(GstImage *image);
typedef void 	(*GstImagePutFunc)     		(GstXWindow *window, GstImage *image);

struct _GstImage {
  GstImageType 		 type;

  guint8 		*data;
  gint     		 size;

  GstImageDestroyFunc	 destroyfunc;
  GstImagePutFunc	 putfunc;
};

#define _gst_image_destroy(img) 	(GST_IMAGE_DESTROYFUNC (img) (img))
#define _gst_image_put(window,img) 	(GST_IMAGE_PUTFUNC (img) (window, img))

G_END_DECLS

#endif /* __GST_IMAGE_H__ */

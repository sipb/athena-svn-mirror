/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) <2003> David Schleef <ds@schleef.org>
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

/*
 * This file was (probably) generated from
 * gstvideotemplate.c,v 1.15 2004/05/21 22:39:29 leroutier Exp 
 * and
 * $Id: gstvideoexample.c,v 1.1.1.1 2004-10-06 18:29:39 ghudson Exp $
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gstvideofilter.h>
#include <string.h>

#define GST_TYPE_VIDEOEXAMPLE \
  (gst_videoexample_get_type())
#define GST_VIDEOEXAMPLE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_VIDEOEXAMPLE,GstVideoexample))
#define GST_VIDEOEXAMPLE_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_VIDEOEXAMPLE,GstVideoexampleClass))
#define GST_IS_VIDEOEXAMPLE(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_VIDEOEXAMPLE))
#define GST_IS_VIDEOEXAMPLE_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_VIDEOEXAMPLE))

typedef struct _GstVideoexample GstVideoexample;
typedef struct _GstVideoexampleClass GstVideoexampleClass;

struct _GstVideoexample
{
  GstVideofilter videofilter;

};

struct _GstVideoexampleClass
{
  GstVideofilterClass parent_class;
};


/* GstVideoexample signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  ARG_0
      /* FILL ME */
};

static void gst_videoexample_base_init (gpointer g_class);
static void gst_videoexample_class_init (gpointer g_class, gpointer class_data);
static void gst_videoexample_init (GTypeInstance * instance, gpointer g_class);

static void gst_videoexample_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_videoexample_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static void gst_videoexample_planar411 (GstVideofilter * videofilter,
    void *dest, void *src);
static void gst_videoexample_setup (GstVideofilter * videofilter);

GType
gst_videoexample_get_type (void)
{
  static GType videoexample_type = 0;

  if (!videoexample_type) {
    static const GTypeInfo videoexample_info = {
      sizeof (GstVideoexampleClass),
      gst_videoexample_base_init,
      NULL,
      gst_videoexample_class_init,
      NULL,
      NULL,
      sizeof (GstVideoexample),
      0,
      gst_videoexample_init,
    };

    videoexample_type = g_type_register_static (GST_TYPE_VIDEOFILTER,
        "GstVideoexample", &videoexample_info, 0);
  }
  return videoexample_type;
}

static GstVideofilterFormat gst_videoexample_formats[] = {
  {"I420", 12, gst_videoexample_planar411,},
};


static void
gst_videoexample_base_init (gpointer g_class)
{
  static GstElementDetails videoexample_details =
      GST_ELEMENT_DETAILS ("Video Filter Template",
      "Filter/Effect/Video",
      "Template for a video filter",
      "David Schleef <ds@schleef.org>");
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);
  GstVideofilterClass *videofilter_class = GST_VIDEOFILTER_CLASS (g_class);
  int i;

  gst_element_class_set_details (element_class, &videoexample_details);

  for (i = 0; i < G_N_ELEMENTS (gst_videoexample_formats); i++) {
    gst_videofilter_class_add_format (videofilter_class,
        gst_videoexample_formats + i);
  }

  gst_videofilter_class_add_pad_templates (GST_VIDEOFILTER_CLASS (g_class));
}

static void
gst_videoexample_class_init (gpointer g_class, gpointer class_data)
{
  GObjectClass *gobject_class;
  GstVideofilterClass *videofilter_class;

  gobject_class = G_OBJECT_CLASS (g_class);
  videofilter_class = GST_VIDEOFILTER_CLASS (g_class);

#if 0
  g_object_class_install_property (gobject_class, ARG_METHOD,
      g_param_spec_enum ("method", "method", "method",
          GST_TYPE_VIDEOEXAMPLE_METHOD, GST_VIDEOEXAMPLE_METHOD_1,
          G_PARAM_READWRITE));
#endif

  gobject_class->set_property = gst_videoexample_set_property;
  gobject_class->get_property = gst_videoexample_get_property;

  videofilter_class->setup = gst_videoexample_setup;
}

static void
gst_videoexample_init (GTypeInstance * instance, gpointer g_class)
{
  GstVideoexample *videoexample = GST_VIDEOEXAMPLE (instance);
  GstVideofilter *videofilter;

  GST_DEBUG ("gst_videoexample_init");

  videofilter = GST_VIDEOFILTER (videoexample);

  /* do stuff */
}

static void
gst_videoexample_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstVideoexample *src;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_VIDEOEXAMPLE (object));
  src = GST_VIDEOEXAMPLE (object);

  GST_DEBUG ("gst_videoexample_set_property");
  switch (prop_id) {
#if 0
    case ARG_METHOD:
      src->method = g_value_get_enum (value);
      break;
#endif
    default:
      break;
  }
}

static void
gst_videoexample_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstVideoexample *src;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_VIDEOEXAMPLE (object));
  src = GST_VIDEOEXAMPLE (object);

  switch (prop_id) {
#if 0
    case ARG_METHOD:
      g_value_set_enum (value, src->method);
      break;
#endif
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_library_load ("gstvideofilter"))
    return FALSE;

  return gst_element_register (plugin, "videoexample", GST_RANK_NONE,
      GST_TYPE_VIDEOEXAMPLE);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "videoexample",
    "Template for a video filter",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE, GST_ORIGIN)

     static void gst_videoexample_setup (GstVideofilter * videofilter)
{
  GstVideoexample *videoexample;

  g_return_if_fail (GST_IS_VIDEOEXAMPLE (videofilter));
  videoexample = GST_VIDEOEXAMPLE (videofilter);

  /* if any setup needs to be done, do it here */

}

static void
gst_videoexample_planar411 (GstVideofilter * videofilter, void *dest, void *src)
{
  GstVideoexample *videoexample;
  int width = gst_videofilter_get_input_width (videofilter);
  int height = gst_videofilter_get_input_height (videofilter);

  g_return_if_fail (GST_IS_VIDEOEXAMPLE (videofilter));
  videoexample = GST_VIDEOEXAMPLE (videofilter);

  /* do something interesting here.  This simply copies the source
   * to the destination. */
  memcpy (dest, src, width * height + (width / 2) * (height / 2) * 2);
}

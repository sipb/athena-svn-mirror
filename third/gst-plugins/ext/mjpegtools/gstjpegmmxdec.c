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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdint.h>
#include <jpegutils.h>
#include <lav_io.h>
#include "gstjpegmmxdec.h"


/* elementfactory information */
GstElementDetails gst_jpegmmxdec_details = {
  "JPEG/MMX decoder",
  "Filter/Decoder/Image",
  "LGPL",
  ".jpeg",
  VERSION,
  "Ronald Bultje <rbultje@ronald.bitfreak.net>",
  "(C) 2002",
};


/* JpegMMXDec signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0,
  /* FILL ME */
};


static void                gst_jpegmmxdec_class_init (GstJpegMMXDecClass *klass);
static void                gst_jpegmmxdec_init       (GstJpegMMXDec      *jpegmmxdec);
static void                gst_jpegmmxdec_chain      (GstPad             *pad,
                                                      GstBuffer          *buf);
static GstPadLinkReturn gst_jpegmmxdec_connect    (GstPad             *pad,
                                                      GstCaps            *vscapslist);


static GstPadTemplate *sink_template, *src_template;

static GstElementClass *parent_class = NULL;
/*static guint gst_jpegmmxdec_signals[LAST_SIGNAL] = { 0 }; */


GType
gst_jpegmmxdec_get_type(void)
{
  static GType jpegmmxdec_type = 0;

  if (!jpegmmxdec_type)
  {
    static const GTypeInfo jpegmmxdec_info = {
      sizeof(GstJpegMMXDecClass),
      NULL,
      NULL,
      (GClassInitFunc)gst_jpegmmxdec_class_init,
      NULL,
      NULL,
      sizeof(GstJpegMMXDec),
      0,
      (GInstanceInitFunc)gst_jpegmmxdec_init,
    };
    jpegmmxdec_type = g_type_register_static(GST_TYPE_ELEMENT, "GstJpegMMXDec", &jpegmmxdec_info, 0);
  }
  return jpegmmxdec_type;
}


static void
gst_jpegmmxdec_class_init (GstJpegMMXDecClass *klass)
{
  GstElementClass *gstelement_class;

  gstelement_class = (GstElementClass*)klass;

  parent_class = g_type_class_ref(GST_TYPE_ELEMENT);
}


static void
gst_jpegmmxdec_init (GstJpegMMXDec *jpegmmxdec)
{
  /* create the sink pad */
  jpegmmxdec->sinkpad = gst_pad_new_from_template (sink_template, "sink");
  gst_element_add_pad(GST_ELEMENT(jpegmmxdec), jpegmmxdec->sinkpad);

  gst_pad_set_chain_function(jpegmmxdec->sinkpad, gst_jpegmmxdec_chain);
  gst_pad_set_link_function(jpegmmxdec->sinkpad, gst_jpegmmxdec_connect);

  /* create the src pad */
  jpegmmxdec->srcpad = gst_pad_new_from_template (src_template, "src");
  gst_element_add_pad(GST_ELEMENT(jpegmmxdec), jpegmmxdec->srcpad);

  /* reset the initial video state */
  jpegmmxdec->width = -1;
  jpegmmxdec->height = -1;
}


static void
gst_jpegmmxdec_chain (GstPad    *pad,
                      GstBuffer *buf)
{
  GstJpegMMXDec *jpegmmxdec;
  GstBuffer *outbuf;

  g_return_if_fail(pad != NULL);
  g_return_if_fail(GST_IS_PAD(pad));
  g_return_if_fail(buf != NULL);

  jpegmmxdec = GST_JPEGMMXDEC (GST_OBJECT_PARENT (pad));

  outbuf = gst_buffer_new();
  GST_BUFFER_SIZE(outbuf) = jpegmmxdec->width * jpegmmxdec->height * 3 / 2;
  GST_BUFFER_DATA(outbuf) = g_malloc(GST_BUFFER_SIZE(outbuf));
  GST_BUFFER_TIMESTAMP(outbuf) = GST_BUFFER_TIMESTAMP(buf);

  /* TODO: interlacing */
  if (decode_jpeg_raw(GST_BUFFER_DATA(buf), GST_BUFFER_SIZE(buf),
                      0, CHROMA420, jpegmmxdec->width, jpegmmxdec->height,
                      GST_BUFFER_DATA(outbuf),
                      GST_BUFFER_DATA(outbuf) + (jpegmmxdec->width * jpegmmxdec->height),
                      GST_BUFFER_DATA(outbuf) + (jpegmmxdec->width * jpegmmxdec->height * 5 / 4)
                     ) < 0)
  {
    gst_element_error(GST_ELEMENT(jpegmmxdec),
      "Failed to decode JPEG image");
  }
  else
  {
    gst_pad_push(jpegmmxdec->srcpad, outbuf);
  }

  gst_buffer_unref(buf);
}


static GstPadLinkReturn
gst_jpegmmxdec_connect (GstPad  *pad,
                        GstCaps *vscaps)
{
  GstJpegMMXDec *jpegmmxdec;
  GstCaps *caps, *newcaps;

  jpegmmxdec = GST_JPEGMMXDEC(gst_pad_get_parent (pad));

  /* we are not going to act on variable caps */
  if (!GST_CAPS_IS_FIXED (vscaps))
    return GST_PAD_LINK_DELAYED;

  for (caps = vscaps; caps != NULL; caps = vscaps = vscaps->next)
  {
    int w, h;
    gst_caps_get_int(caps, "width", &w);
    gst_caps_get_int(caps, "height", &h);
    newcaps = gst_caps_new("jpegmmxdec_caps",
                           "video/raw",
                           gst_props_new(
                             "format", GST_PROPS_FOURCC(GST_MAKE_FOURCC('I','4','2','0')),
                             "width",  GST_PROPS_INT(w),
                             "height", GST_PROPS_INT(h),
                             NULL       )
                          );
    if (gst_pad_try_set_caps(jpegmmxdec->srcpad, newcaps) > 0)
    {
      jpegmmxdec->width = w;
      jpegmmxdec->height = h;
      return GST_PAD_LINK_OK;
    }
  }

  /* if we got here - it's not good */
  return GST_PAD_LINK_REFUSED;
}


static gboolean
plugin_init (GModule   *module,
             GstPlugin *plugin)
{
  GstElementFactory *factory;
  GstCaps *caps;

  /* create an elementfactory for the v4lmjpegsrcparse element */
  factory = gst_element_factory_new("jpegmmxdec", GST_TYPE_JPEGMMXDEC,
                                   &gst_jpegmmxdec_details);
  g_return_val_if_fail(factory != NULL, FALSE);
  gst_element_factory_set_rank (factory, GST_ELEMENT_RANK_PRIMARY);

  caps = gst_caps_new ("jpegmmxdec_caps",
                       "video/jpeg",
                       gst_props_new (
                          "width",  GST_PROPS_INT_RANGE (0, G_MAXINT),
                          "height", GST_PROPS_INT_RANGE (0, G_MAXINT),
                          NULL       )
                      );

  sink_template = gst_pad_template_new (
                  "sink",
                  GST_PAD_SINK,
                  GST_PAD_ALWAYS,
                  caps, NULL);

  gst_element_factory_add_pad_template (factory, sink_template);

  caps = gst_caps_new ("jpegmmxdec_caps",
                       "video/raw",
                       gst_props_new (
                          "format", GST_PROPS_FOURCC(GST_MAKE_FOURCC('I','4','2','0')),
                          "width",  GST_PROPS_INT_RANGE (0, G_MAXINT),
                          "height", GST_PROPS_INT_RANGE (0, G_MAXINT),
                          NULL       )
                      );

  src_template = gst_pad_template_new (
                 "src",
                 GST_PAD_SRC,
                 GST_PAD_ALWAYS,
                 caps, NULL);

  gst_element_factory_add_pad_template (factory, src_template);

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  return TRUE;
}


GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "jpegmmxdec",
  plugin_init
};

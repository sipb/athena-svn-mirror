/* GStreamer JPEG/MMX encoder plugin
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
#include "gstjpegmmxenc.h"


/* elementfactory information */
GstElementDetails gst_jpegmmxenc_details = {
  "JPEG/MMX encoder",
  "Filter/Encoder/Image",
  "LGPL",
  ".jpeg",
  VERSION,
  "Ronald Bultje <rbultje@ronald.bitfreak.net>",
  "(C) 2002",
};


/* JpegMMXEnc signals and args */
enum {
  FRAME_ENCODED,
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_QUALITY,
  ARG_BUFSIZE,
  /* FILL ME */
};


static void                gst_jpegmmxenc_class_init   (GstJpegMMXEncClass *klass);
static void                gst_jpegmmxenc_init         (GstJpegMMXEnc      *jpegmmxenx);
static void                gst_jpegmmxenc_chain        (GstPad             *pad,
                                                        GstBuffer          *buf);
static GstPadLinkReturn gst_jpegmmxenc_connect      (GstPad             *pad,
                                                        GstCaps            *vscapslist);
static void                gst_jpegmmxenc_set_property (GObject            *object,
                                                        guint              prop_id,
                                                        const GValue       *value,
                                                        GParamSpec         *pspec);
static void                gst_jpegmmxenc_get_property (GObject            *object,
                                                        guint              prop_id,
                                                        GValue             *value,
                                                        GParamSpec         *pspec);


static GstPadTemplate *sink_template, *src_template;

static GstElementClass *parent_class = NULL;
static guint gst_jpegmmxenc_signals[LAST_SIGNAL] = { 0 };


GType
gst_jpegmmxenc_get_type (void)
{
  static GType jpegmmxenc_type = 0;

  if (!jpegmmxenc_type)
  {
    static const GTypeInfo jpegmmxenc_info = {
      sizeof(GstJpegMMXEncClass),
      NULL,
      NULL,
      (GClassInitFunc)gst_jpegmmxenc_class_init,
      NULL,
      NULL,
      sizeof(GstJpegMMXEnc),
      0,
      (GInstanceInitFunc)gst_jpegmmxenc_init,
    };
    jpegmmxenc_type = g_type_register_static(GST_TYPE_ELEMENT, "GstJpegMMXEnc", &jpegmmxenc_info, 0);
  }
  return jpegmmxenc_type;
}


static void
gst_jpegmmxenc_class_init (GstJpegMMXEncClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)klass;

  parent_class = g_type_class_ref(GST_TYPE_ELEMENT);

  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_QUALITY,
    g_param_spec_int("quality","Quality","Quality of the resulting JPEG image",
    G_MININT,G_MAXINT,0,G_PARAM_READWRITE));

  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_BUFSIZE,
    g_param_spec_int("buffer_size","Buffer Size","Size of the JPEG buffers",
    G_MININT,G_MAXINT,0,G_PARAM_READWRITE));

  gobject_class->set_property = gst_jpegmmxenc_set_property;
  gobject_class->get_property = gst_jpegmmxenc_get_property;

  gst_jpegmmxenc_signals[FRAME_ENCODED] =
    g_signal_new ("frame_encoded", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET (GstJpegMMXEncClass, frame_encoded), NULL, NULL,
                   g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
}


static void
gst_jpegmmxenc_init (GstJpegMMXEnc *jpegmmxenc)
{
  /* create the sink pad */
  jpegmmxenc->sinkpad = gst_pad_new_from_template (sink_template, "sink");
  gst_element_add_pad(GST_ELEMENT(jpegmmxenc), jpegmmxenc->sinkpad);

  gst_pad_set_chain_function(jpegmmxenc->sinkpad, gst_jpegmmxenc_chain);
  gst_pad_set_link_function(jpegmmxenc->sinkpad, gst_jpegmmxenc_connect);

  /* create the src pad */
  jpegmmxenc->srcpad = gst_pad_new_from_template (src_template, "src");
  gst_element_add_pad(GST_ELEMENT(jpegmmxenc), jpegmmxenc->srcpad);

  /* reset the initial video state */
  jpegmmxenc->width = -1;
  jpegmmxenc->height = -1;

  /* default quality */
  jpegmmxenc->quality = 50;

  /* default buffer size */
  jpegmmxenc->buffer_size = 256; /* KB */
}


static void
gst_jpegmmxenc_chain (GstPad    *pad,
                      GstBuffer *buf)
{
  GstJpegMMXEnc *jpegmmxenc;
  GstBuffer *outbuf;
  glong bufsize;

  g_return_if_fail (pad != NULL);
  g_return_if_fail (GST_IS_PAD (pad));
  g_return_if_fail (buf != NULL);

  jpegmmxenc = GST_JPEGMMXENC (GST_OBJECT_PARENT (pad));

  outbuf = gst_buffer_new();
  GST_BUFFER_SIZE(outbuf) = jpegmmxenc->buffer_size * 1024;
  GST_BUFFER_DATA(outbuf) = g_malloc(GST_BUFFER_SIZE(outbuf));
  GST_BUFFER_TIMESTAMP(outbuf) = GST_BUFFER_TIMESTAMP(buf);

  /* TODO: interlacing */
  bufsize = encode_jpeg_raw(
              GST_BUFFER_DATA(outbuf), GST_BUFFER_SIZE(outbuf),
              jpegmmxenc->quality, 0, CHROMA420, jpegmmxenc->width, jpegmmxenc->height,
              GST_BUFFER_DATA(buf),
              GST_BUFFER_DATA(buf) + (jpegmmxenc->width * jpegmmxenc->height),
              GST_BUFFER_DATA(buf) + (jpegmmxenc->width * jpegmmxenc->height * 5 / 4)
           );
  if (bufsize < 0)
  {
    gst_element_error(GST_ELEMENT(jpegmmxenc),
      "Failed to encode JPEG image");
  }
  else
  {
    GST_BUFFER_SIZE(outbuf) = bufsize;
    gst_pad_push(jpegmmxenc->srcpad, outbuf);

    g_signal_emit(G_OBJECT(jpegmmxenc),gst_jpegmmxenc_signals[FRAME_ENCODED], 0);
  }

  gst_buffer_unref(buf);
}


static GstPadLinkReturn
gst_jpegmmxenc_connect (GstPad  *pad,
                        GstCaps *vscaps)
{
  GstJpegMMXEnc *jpegmmxenc;
  GstCaps *caps, *newcaps;

  jpegmmxenc = GST_JPEGMMXENC (gst_pad_get_parent (pad));

  /* we are not going to act on variable caps */
  if (!GST_CAPS_IS_FIXED (vscaps))
    return GST_PAD_LINK_DELAYED;

  for (caps = vscaps; caps != NULL; caps = vscaps = vscaps->next)
  {
    int w,h;
    guint32 fourcc;
    gst_caps_get_int(caps, "width", &w);
    gst_caps_get_int(caps, "height", &h);
    gst_caps_get_fourcc_int(caps, "format", &fourcc);
    switch (fourcc)
    {
      case GST_MAKE_FOURCC('I','4','2','0'):
      case GST_MAKE_FOURCC('I','Y','U','V'):
        newcaps = gst_caps_new("jpegmmxenc_caps",
                               "video/jpeg",
                               gst_props_new(
                                 "width",  GST_PROPS_INT(w),
                                 "height", GST_PROPS_INT(h),
                                 NULL       )
                               );

        if (gst_pad_try_set_caps(jpegmmxenc->srcpad, newcaps) > 0)
        {
          jpegmmxenc->width = w;
          jpegmmxenc->height = h;

          return GST_PAD_LINK_OK;
        }
        break;
    }
  }

  /* if we got here - it's not good */
  return GST_PAD_LINK_REFUSED;
}


static void
gst_jpegmmxenc_set_property (GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  GstJpegMMXEnc *jpegmmxenc;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_JPEGMMXENC (object));
  jpegmmxenc = GST_JPEGMMXENC(object);

  switch (prop_id)
  {
    case ARG_QUALITY:
      jpegmmxenc->quality = g_value_get_int(value);
      break;
    case ARG_BUFSIZE:
      jpegmmxenc->buffer_size = g_value_get_int(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static void
gst_jpegmmxenc_get_property (GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec)
{
  GstJpegMMXEnc *jpegmmxenc;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_JPEGMMXENC (object));
  jpegmmxenc = GST_JPEGMMXENC(object);

  switch (prop_id) {
    case ARG_QUALITY:
      g_value_set_int(value, jpegmmxenc->quality);
      break;
    case ARG_BUFSIZE:
      g_value_set_int(value, jpegmmxenc->buffer_size);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static gboolean
plugin_init (GModule   *module,
             GstPlugin *plugin)
{
  GstElementFactory *factory;
  GstCaps *caps;

  /* create an elementfactory for the v4lmjpegsrcparse element */
  factory = gst_element_factory_new("jpegmmxenc", GST_TYPE_JPEGMMXENC,
                                   &gst_jpegmmxenc_details);
  g_return_val_if_fail(factory != NULL, FALSE);

  caps = gst_caps_new ("jpegmmxenc_caps",
                       "video/jpeg",
                       gst_props_new (
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

  caps = gst_caps_new ("jpegmmxenc_caps",
                       "video/raw",
                       gst_props_new (
                          "format", GST_PROPS_FOURCC(GST_MAKE_FOURCC('I','4','2','0')),
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

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  return TRUE;
}


GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "jpegmmxenc",
  plugin_init
};

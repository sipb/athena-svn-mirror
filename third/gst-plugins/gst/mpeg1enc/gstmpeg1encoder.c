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

#define DEBUG_ENABLED

#include "gstmpeg1encoder.h"

/* elementfactory information */
static GstElementDetails gst_mpeg1encoder_details = {
  "mpeg1 and mpeg2 video encoder",
  "Codec/Video/Encoder",
  "LGPL",
  "Uses modified mpeg1encoder V1.2a to encode MPEG video streams",
  VERSION,
  "(C) 1996, MPEG Software Simulation Group\n"
  "Wim Taymans <wim.taymans@tvd.be>",
  "(C) 2000",
};

/* Mpeg1encoder signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0,
  /* FILL ME */
};

static void	gst_mpeg1encoder_class_init	(GstMpeg1encoderClass *klass);
static void	gst_mpeg1encoder_init		(GstMpeg1encoder *mpeg1encoder);

static void	gst_mpeg1encoder_chain		(GstPad *pad, GstBuffer *buf);

static GstElementClass *parent_class = NULL;
/*static guint gst_mpeg1encoder_signals[LAST_SIGNAL] = { 0 };*/

GType
gst_mpeg1encoder_get_type (void)
{
  static GType mpeg1encoder_type = 0;

  if (!mpeg1encoder_type) {
    static const GTypeInfo mpeg1encoder_info = {
      sizeof(GstMpeg1encoderClass),      NULL,
      NULL,
      (GClassInitFunc)gst_mpeg1encoder_class_init,
      NULL,
      NULL,
      sizeof(GstMpeg1encoder),
      0,
      (GInstanceInitFunc)gst_mpeg1encoder_init,
    };
    mpeg1encoder_type = g_type_register_static(GST_TYPE_ELEMENT, "GstMpeg1encoder", &mpeg1encoder_info, 0);
  }
  return mpeg1encoder_type;
}

static void
gst_mpeg1encoder_class_init (GstMpeg1encoderClass *klass)
{
  GstElementClass *gstelement_class;

  gstelement_class = (GstElementClass*)klass;

  parent_class = g_type_class_ref(GST_TYPE_ELEMENT);
}

static void
gst_mpeg1encoder_init (GstMpeg1encoder *mpeg1encoder)
{
  /* create the sink and src pads */
  mpeg1encoder->sinkpad = gst_pad_new("sink",GST_PAD_SINK);
  gst_element_add_pad(GST_ELEMENT(mpeg1encoder),mpeg1encoder->sinkpad);
  gst_pad_set_chain_function(mpeg1encoder->sinkpad,gst_mpeg1encoder_chain);
  mpeg1encoder->srcpad = gst_pad_new("src",GST_PAD_SRC);
  gst_element_add_pad(GST_ELEMENT(mpeg1encoder),mpeg1encoder->srcpad);

  /* initialize the mpeg1encoder encoder state */
  mpeg1encoder->encoder = mpeg1encoder_new_encoder();
  mpeg1encoder->next_time = 0;

  /* reset the initial video state */
  mpeg1encoder->format = -1;
  mpeg1encoder->width = -1;
  mpeg1encoder->height = -1;
}

static void
gst_mpeg1encoder_chain (GstPad *pad, GstBuffer *buf)
{
  GstMpeg1encoder *mpeg1encoder;
  guchar *data;
  gulong size;

  g_return_if_fail(pad != NULL);
  g_return_if_fail(GST_IS_PAD(pad));
  g_return_if_fail(buf != NULL);
  /*g_return_if_fail(GST_IS_BUFFER(buf));*/

  mpeg1encoder = GST_MPEG1ENCODER (GST_OBJECT_PARENT (pad));

  /* first deal with video metadata */
  /*
  meta = gst_buffer_get_first_meta(buf);
  if (meta) {
    if (mpeg1encoder->format != ((MetaVideoRaw *)meta)->format ||
        mpeg1encoder->width  != ((MetaVideoRaw *)meta)->width ||
        mpeg1encoder->height != ((MetaVideoRaw *)meta)->height) {

      mpeg1encoder->format = ((MetaVideoRaw *)meta)->format;
      mpeg1encoder->width  = ((MetaVideoRaw *)meta)->width;
      mpeg1encoder->height = ((MetaVideoRaw *)meta)->height;

      mpeg1encoder->encoder->seq.HorizontalSize = mpeg1encoder->width;
      mpeg1encoder->encoder->seq.VerticalSize = mpeg1encoder->height;
    }
  }
  */

  data = (guchar *)GST_BUFFER_DATA(buf);
  size = GST_BUFFER_SIZE(buf);


  GST_DEBUG (0,"gst_mpeg1encoder_chain: got buffer of %ld bytes in '%s'",size,
          GST_OBJECT_NAME (mpeg1encoder));

  mpeg1encoder->state = mpeg1encoder_new_picture(mpeg1encoder->encoder, data, size, mpeg1encoder->state);

  if (mpeg1encoder->state & NEW_DATA) {
    /*outbuf = gst_buffer_new();*/
    /*GST_BUFFER_SIZE(outbuf) = mpeg1encoder->encoder->pb.newlen;*/
    /*GST_BUFFER_DATA(outbuf) = mpeg1encoder->encoder->pb.outbase;*/

    /*GST_DEBUG (0,"gst_mpeg1encoder_chain: pushing buffer %d", GST_BUFFER_SIZE(outbuf)); */
    /*gst_pad_push(mpeg1encoder->srcpad,outbuf); */
  }

  gst_buffer_unref(buf);
}

static gboolean
plugin_init (GModule *module, GstPlugin *plugin)
{
  GstElementFactory *factory;

  /* create an elementfactory for the mpeg1encoder element */
  factory = gst_element_factory_new("mpeg1enc", GST_TYPE_MPEG1ENCODER,
                                   &gst_mpeg1encoder_details);
  g_return_val_if_fail(factory != NULL, FALSE);
  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "mpeg1enc",
  plugin_init
};

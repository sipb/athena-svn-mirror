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

#include <math.h>
/*#define DEBUG_ENABLED*/

#include "gstmpeg2enc.h"

/* elementfactory information */
static GstElementDetails gst_mpeg2enc_details = {
  "mpeg1 and mpeg2 video encoder",
  "Codec/Video/Encoder",
  "GPL",
  "Uses modified mpeg2encode V1.2a to encode MPEG video streams",
  VERSION,
  "(C) 1996, MPEG Software Simulation Group\n"
  "Wim Taymans <wim.taymans@tvd.be>",
  "(C) 2000",
};

/* Mpeg2enc signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_FPS,
  ARG_BITRATE,
  /* FILL ME */
};

static double video_rates[16] =
{
  0.0,
  24000.0/1001.,
  24.0,
  25.0,
  30000.0/1001.,
  30.0,
  50.0,
  60000.0/1001.,
  60.0,
  1,
  5,
  10,
  12,
  15,
  0,
  0
};

GST_PAD_TEMPLATE_FACTORY (sink_template_factory,
  "sink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
    "mpeg2enc_sink_caps",
    "video/raw",
      "format",		GST_PROPS_FOURCC (GST_MAKE_FOURCC ('I','4','2','0')),
      "width",		GST_PROPS_INT_RANGE (16, 4096),
      "height",		GST_PROPS_INT_RANGE (16, 4096)
  )
)

static void	gst_mpeg2enc_class_init		(GstMpeg2encClass *klass);
static void	gst_mpeg2enc_init		(GstMpeg2enc *mpeg2enc);

static void	gst_mpeg2enc_set_property	(GObject *object, guint prop_id, 
						 const GValue *value, GParamSpec *pspec);
static void	gst_mpeg2enc_get_property	(GObject *object, guint prop_id, 
						 GValue *value, GParamSpec *pspec);

static void	gst_mpeg2enc_chain		(GstPad *pad, GstBuffer *buf);

static GstElementClass *parent_class = NULL;
/*static guint gst_mpeg2enc_signals[LAST_SIGNAL] = { 0 };*/

GType
gst_mpeg2enc_get_type (void)
{
  static GType mpeg2enc_type = 0;

  if (!mpeg2enc_type) {
    static const GTypeInfo mpeg2enc_info = {
      sizeof(GstMpeg2encClass),      
      NULL,
      NULL,
      (GClassInitFunc)gst_mpeg2enc_class_init,
      NULL,
      NULL,
      sizeof(GstMpeg2enc),
      0,
      (GInstanceInitFunc)gst_mpeg2enc_init,
    };
    mpeg2enc_type = g_type_register_static(GST_TYPE_ELEMENT, "GstMpeg2enc", &mpeg2enc_info, 0);
  }
  return mpeg2enc_type;
}

static void
gst_mpeg2enc_class_init (GstMpeg2encClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)klass;

  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_FPS,
    g_param_spec_double("frames_per_second","frames_per_second","frames_per_second",
                        -G_MAXDOUBLE,G_MAXDOUBLE,0,G_PARAM_READWRITE)); /* CHECKME*/
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_BITRATE,
    g_param_spec_int("bitrate","bitrate","bitrate",
                        0,1500000,0,G_PARAM_READWRITE)); /* CHECKME */

  parent_class = g_type_class_ref(GST_TYPE_ELEMENT);

  gobject_class->set_property = gst_mpeg2enc_set_property;
  gobject_class->get_property = gst_mpeg2enc_get_property;

}

static GstPadLinkReturn
gst_mpeg2enc_sinkconnect (GstPad *pad, GstCaps *caps)
{
  gint width, height;
  GstMpeg2enc *mpeg2enc;

  mpeg2enc = GST_MPEG2ENC (gst_pad_get_parent (pad));

  if (!GST_CAPS_IS_FIXED (caps)) 
    return GST_PAD_LINK_DELAYED;

  gst_caps_get_int (caps, "width", &width);
  gst_caps_get_int (caps, "height", &height);
  
  mpeg2enc->encoder->seq.horizontal_size = width;
  mpeg2enc->encoder->seq.display_horizontal_size = width;
  mpeg2enc->encoder->seq.vertical_size = height;
  mpeg2enc->encoder->seq.display_vertical_size = height;
  mpeg2enc->encoder->seq.frame_rate_code = 3; /* default 25 fps */

  return GST_PAD_LINK_OK;
}

static void
gst_mpeg2enc_init (GstMpeg2enc *mpeg2enc)
{
  /* create the sink and src pads */
  mpeg2enc->sinkpad = gst_pad_new_from_template (
		  GST_PAD_TEMPLATE_GET (sink_template_factory), "sink");
  gst_element_add_pad(GST_ELEMENT(mpeg2enc),mpeg2enc->sinkpad);
  gst_pad_set_chain_function (mpeg2enc->sinkpad, gst_mpeg2enc_chain);
  gst_pad_set_link_function (mpeg2enc->sinkpad, gst_mpeg2enc_sinkconnect);

  mpeg2enc->srcpad = gst_pad_new("src",GST_PAD_SRC);
  gst_element_add_pad(GST_ELEMENT(mpeg2enc),mpeg2enc->srcpad);

  /* initialize the mpeg2enc encoder state */
  mpeg2enc->encoder = mpeg2enc_new_encoder();
  mpeg2enc->next_time = 0;

  /* reset the initial video state */
  mpeg2enc->format = -1;
  mpeg2enc->width = -1;
  mpeg2enc->height = -1;
}

static void
gst_mpeg2enc_chain (GstPad *pad, GstBuffer *buf)
{
  GstMpeg2enc *mpeg2enc;
  guchar *data;
  gulong size;
  GstBuffer *outbuf;

  g_return_if_fail(pad != NULL);
  g_return_if_fail(GST_IS_PAD(pad));
  g_return_if_fail(buf != NULL);
  /*g_return_if_fail(GST_IS_BUFFER(buf));*/

  mpeg2enc = GST_MPEG2ENC (gst_pad_get_parent (pad));

  data = (guchar *)GST_BUFFER_DATA(buf);
  size = GST_BUFFER_SIZE(buf);
  GST_DEBUG (0,"gst_mpeg2enc_chain: got buffer of %ld bytes in '%s'",size,
          GST_OBJECT_NAME (mpeg2enc));

  mpeg2enc->state = mpeg2enc_new_picture(mpeg2enc->encoder, data, size, mpeg2enc->state);

  if (mpeg2enc->state & NEW_DATA) {
    outbuf = gst_buffer_new();
    GST_BUFFER_SIZE(outbuf) = mpeg2enc->encoder->pb.newlen;
    GST_BUFFER_DATA(outbuf) = mpeg2enc->encoder->pb.outbase;

    GST_DEBUG (0,"gst_mpeg2enc_chain: pushing buffer %d", GST_BUFFER_SIZE(outbuf));
    gst_pad_push(mpeg2enc->srcpad,outbuf);
  }

  gst_buffer_unref(buf);
}

static void
gst_mpeg2enc_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  GstMpeg2enc *src;
  int i;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail(GST_IS_MPEG2ENC(object));
  src = GST_MPEG2ENC(object);

  switch (prop_id) {
    case ARG_FPS:
      src->encoder->seq.frame_rate_code = 3; /* default 25 fps */
      for (i=0; i< 16; i++) {
	if (fabs(video_rates[i] - g_value_get_double (value)) < .001) {
          src->encoder->seq.frame_rate_code = i;
	  gst_info("mpeg2enc: setting framerate for encoding to %g\n", video_rates[i]);
	  break;
	}
      }
      break;
    case ARG_BITRATE:
      src->encoder->seq.bit_rate = g_value_get_int (value);
      break;
    default:
      break;
  }
}

static void
gst_mpeg2enc_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GstMpeg2enc *src;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail(GST_IS_MPEG2ENC(object));
  src = GST_MPEG2ENC(object);

  switch (prop_id) {
    case ARG_FPS:
      g_value_set_double (value, src->encoder->seq.frame_rate_code);
      break;
    case ARG_BITRATE:
      g_value_set_int (value, src->encoder->seq.bit_rate);
      break;
    default:
      break;
  }
}


static gboolean
plugin_init (GModule *module, GstPlugin *plugin)
{
  GstElementFactory *factory;

  /* this filter needs the getbits package */
  if (!gst_library_load ("gstputbits"))
    return FALSE;
  if (!gst_library_load ("gstidct"))
    return FALSE;

  /* create an elementfactory for the mpeg2enc element */
  factory = gst_element_factory_new ("mpeg2enc", GST_TYPE_MPEG2ENC,
                                     &gst_mpeg2enc_details);
  g_return_val_if_fail(factory != NULL, FALSE);

  gst_element_factory_add_pad_template (factory, GST_PAD_TEMPLATE_GET (sink_template_factory));

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "mpeg2enc",
  plugin_init
};

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


#include <string.h>

#include "gstopenquicktimedecoder.h"

/* elementfactory information */
static GstElementDetails gst_quicktime_decoder_details = {
  "quicktime parser",
  "Codec/Video/Decoder",
  "LGPL",
  "Decodes a quicktime file into raw audio and video",
  VERSION,
  "Yann <yann@3ivx.com>",
  "(C) 2001",
};

static GstCaps* quicktime_type_find (GstBuffer *buf, gpointer private);

/* typefactory for 'quicktime' */
static GstTypeDefinition quicktimedefinition = {
  "quicktimedecoder_video/quicktime",
  "video/quicktime",
  ".mov",
  quicktime_type_find,
};

/* QuicktimeDecoder signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_BIT_RATE,
  ARG_MEDIA_TIME,
  ARG_CURRENT_TIME,
  /* FILL ME */
};

GST_PAD_TEMPLATE_FACTORY (sink_templ,
  "sink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
    "quicktimedecoder_sink",
    "video/quicktime",
    NULL
  )
)

GST_PAD_TEMPLATE_FACTORY (src_video_templ,
  "video_src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
      "src_video",
      "video/raw",
      "format",    GST_PROPS_LIST (
                     GST_PROPS_FOURCC (GST_MAKE_FOURCC ('Y','V','1','2')),
                     GST_PROPS_FOURCC (GST_MAKE_FOURCC ('I','Y','U','V')),
                     GST_PROPS_FOURCC (GST_MAKE_FOURCC ('I','4','2','0')),
		     GST_PROPS_FOURCC (GST_MAKE_FOURCC ('Y','U','Y','2')),
		     GST_PROPS_FOURCC (GST_MAKE_FOURCC ('R','G','B',' '))
                   ),
      "width",     GST_PROPS_INT_RANGE (16, 4096),
      "height",    GST_PROPS_INT_RANGE (16, 4096)
  )
)

GST_PAD_TEMPLATE_FACTORY (src_audio_templ,
  "audio_src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
    "src_audio",
      "audio/raw",
      "format",     GST_PROPS_STRING ("int"),
      "law",        GST_PROPS_INT (0),
      "endianness", GST_PROPS_INT (G_BYTE_ORDER),
      "signed",     GST_PROPS_BOOLEAN (TRUE),
      "depth",      GST_PROPS_INT (16),
      "width",      GST_PROPS_INT (16),
      "rate",       GST_PROPS_INT_RANGE (11025, 44100),
      "channels",   GST_PROPS_LIST (
                      GST_PROPS_INT (1),
                      GST_PROPS_INT (2)
                    )
  )
)


static void 	gst_quicktime_decoder_init		(GstQuicktimeDecoder *quicktime_decoder);
static void 	gst_quicktime_decoder_class_init	(GstQuicktimeDecoderClass *klass);

static void 	gst_quicktime_decoder_get_property		(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static GstElementClass *parent_class = NULL;
/*static guint gst_quicktime_decoder_signals[LAST_SIGNAL] = { 0 }; */

static GType
gst_quicktime_decoder_get_type (void) 
{
  static GType quicktime_decoder_type = 0;

  if (!quicktime_decoder_type) {
    static const GTypeInfo quicktime_decoder_info = {
      sizeof(GstQuicktimeDecoderClass),      NULL,
      NULL,
      (GClassInitFunc)gst_quicktime_decoder_class_init,
      NULL,
      NULL,
      sizeof(GstQuicktimeDecoder),
      0,
      (GInstanceInitFunc)gst_quicktime_decoder_init,
    };
    quicktime_decoder_type = g_type_register_static (GST_TYPE_BIN, "GstQuicktimeDecoder", &quicktime_decoder_info, 0);
  }
  return quicktime_decoder_type;
}

static void
gst_quicktime_decoder_class_init (GstQuicktimeDecoderClass *klass) 
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)klass;

  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_BIT_RATE,
    g_param_spec_long("bit_rate","bit_rate","bit_rate",
                     G_MINLONG,G_MAXLONG,0,G_PARAM_READABLE)); /* CHECKME */
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_MEDIA_TIME,
    g_param_spec_long("media_time","media_time","media_time",
                     G_MINLONG,G_MAXLONG,0,G_PARAM_READABLE)); /* CHECKME */
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_CURRENT_TIME,
    g_param_spec_long("current_time","current_time","current_time",
                     G_MINLONG,G_MAXLONG,0,G_PARAM_READABLE)); /* CHECKME */

  parent_class = g_type_class_ref (GST_TYPE_BIN);
  
  gobject_class->get_property = gst_quicktime_decoder_get_property;

}

static void 
gst_quicktime_decoder_new_pad (GstElement *element, GstPad *pad, GstQuicktimeDecoder *quicktime_decoder) 
{
  GstCaps *caps;
  GstCaps *targetcaps = NULL;
  const gchar *format;
  gboolean type_found;
  GstElement *type;
  GstElement *new_element = NULL;
  gchar *padname = NULL;
  gchar *gpadname = NULL;
#define QUICKTIME_TYPE_VIDEO  1
#define QUICKTIME_TYPE_AUDIO  2
  gint media_type = 0;
  
  GST_DEBUG (0, "quicktimedecoder: new pad for element \"%s\"", gst_element_get_name (element));

  caps = gst_pad_get_caps (pad);
  gst_caps_get_string (caps, "format", &format);

  if (!strcmp (format, "strf_vids")) { 
    targetcaps = gst_pad_template_get_caps (GST_PAD_TEMPLATE_GET (src_video_templ));
    media_type = QUICKTIME_TYPE_VIDEO;
    gpadname = g_strdup_printf ("video_%02d", quicktime_decoder->count);
  }
  else if (!strcmp (format, "strf_auds")) {
    targetcaps = gst_pad_template_get_caps (GST_PAD_TEMPLATE_GET (src_audio_templ));
    media_type = QUICKTIME_TYPE_AUDIO;
    gpadname = g_strdup_printf ("audio_%02d", quicktime_decoder->count);
  }
  else {
    g_assert_not_reached ();
  }

  gst_element_set_state (GST_ELEMENT (quicktime_decoder), GST_STATE_PAUSED);

  type = gst_element_factory_make ("quicktimetypes", 
		  g_strdup_printf ("typeconvert%d", quicktime_decoder->count));

  if(type == NULL) {
    g_error ("cannot find quicktimetypes plugin\n");
  }
    

  gst_pad_link (pad, gst_element_get_pad (type, "sink"));
  type_found = gst_util_get_bool_arg (G_OBJECT (type), "type_found");

  if (type_found) {
    gst_bin_add (GST_BIN (quicktime_decoder), type);
    
    pad = gst_element_get_pad (type, "src");
    caps = gst_pad_get_caps (pad);

    if (gst_caps_is_always_compatible (caps, targetcaps)) {
      gst_element_add_ghost_pad (GST_ELEMENT (quicktime_decoder), 
	    gst_element_get_pad (type, "src"), gpadname);

      quicktime_decoder->count++;
      goto done;
    }
#ifndef GST_DISABLE_AUTOPLUG
    else {
      GstAutoplug *autoplug;

      autoplug = gst_autoplug_factory_make("static");

      new_element = gst_autoplug_to_caps (autoplug, caps, targetcaps, NULL);

      padname = "src_00";
    }
#endif /* GST_DISABLE_AUTOPLUG */
  }

  if (new_element) {
    gst_pad_link (pad, gst_element_get_pad (new_element, "sink"));
    gst_element_set_name (new_element, g_strdup_printf ("element%d", quicktime_decoder->count));
    gst_bin_add (GST_BIN (quicktime_decoder), new_element);

    gst_element_add_ghost_pad (GST_ELEMENT (quicktime_decoder), 
	    gst_element_get_pad (new_element, padname), gpadname);

    quicktime_decoder->count++;
  }
  else {
    g_warning ("quicktime_decoder: could not autoplug\n");
  }

done: 
  gst_element_set_state (GST_ELEMENT (quicktime_decoder), GST_STATE_PLAYING);
}


static void 
gst_quicktime_decoder_init (GstQuicktimeDecoder *quicktime_decoder) 
{
  quicktime_decoder->demuxer = gst_element_factory_make ("quicktime_demux", "demux");
		
  if (quicktime_decoder->demuxer) {
    gst_bin_add (GST_BIN (quicktime_decoder), quicktime_decoder->demuxer);

    gst_element_add_ghost_pad (GST_ELEMENT (quicktime_decoder), 
		    gst_element_get_pad (quicktime_decoder->demuxer, "sink"), "sink");

    g_signal_connect (G_OBJECT (quicktime_decoder->demuxer),"new_pad", 
		      G_CALLBACK (gst_quicktime_decoder_new_pad),
		      quicktime_decoder);
  }
  else {
    g_error ("wow!, no quicktime demuxer found. help me\n");
  }

  quicktime_decoder->count = 0;
}

static GstCaps*
quicktime_type_find (GstBuffer *buf,
	      gpointer private)
{
  gchar *data = GST_BUFFER_DATA (buf);

  if (!strncmp (&data[4], "wide", 4) ||
      !strncmp (&data[4], "moov", 4) ||
      !strncmp (&data[4], "mdat", 4))  {
    return gst_caps_new ("quicktime_type_find",
                         "video/quicktime",
                         NULL);
  }
  return NULL;
}


static void
gst_quicktime_decoder_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GstQuicktimeDecoder *src;

  g_return_if_fail (GST_IS_QUICKTIME_DECODER (object));

  src = GST_QUICKTIME_DECODER (object);

  switch (prop_id) {
    case ARG_BIT_RATE:
      break;
    case ARG_MEDIA_TIME:
      g_value_set_long (value, gst_util_get_long_arg (G_OBJECT (src->demuxer), "media_time"));
      break;
    case ARG_CURRENT_TIME:
      g_value_set_long (value, gst_util_get_long_arg (G_OBJECT (src->demuxer), "current_time"));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static gboolean
plugin_init (GModule *module, GstPlugin *plugin)
{
  GstElementFactory *factory;
  GstTypeFactory *type;

  factory = gst_element_factory_new("quicktime_decoder",GST_TYPE_QUICKTIME_DECODER,
                                   &gst_quicktime_decoder_details);
  g_return_val_if_fail(factory != NULL, FALSE);
  gst_element_factory_set_rank (factory, GST_ELEMENT_RANK_SECONDARY);

  gst_element_factory_add_pad_template (factory, GST_PAD_TEMPLATE_GET (src_audio_templ));
  gst_element_factory_add_pad_template (factory, GST_PAD_TEMPLATE_GET (src_video_templ));
  gst_element_factory_add_pad_template (factory, GST_PAD_TEMPLATE_GET (sink_templ));

  type = gst_type_factory_new (&quicktimedefinition);
  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (type));

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "quicktime_decoder",
  plugin_init
};



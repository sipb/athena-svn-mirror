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

#include "gstopenquicktimetypes.h"

/* elementfactory information */
static GstElementDetails gst_quicktime_types_details = {
  "quicktime type converter",
  "Codec/Video/Decoder",
  "LGPL",
  "Converts quicktime types into gstreamer types",
  VERSION,
  "Yann <yann@3ivx.com>",
  "(C) 2001",
};

/* QuicktimeTypes signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_TYPE_FOUND,
  /* FILL ME */
};

GST_PAD_TEMPLATE_FACTORY (sink_templ,
  "sink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
    "quicktimetypes_sink",
     "video/quicktime",
      "format",         GST_PROPS_LIST (
      		          GST_PROPS_STRING ("strf_vids"),
      		          GST_PROPS_STRING ("strf_auds")
			)
  )
)

GST_PAD_TEMPLATE_FACTORY (src_templ,
  "video_src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
    "quicktimetypes_src",
    "video/raw",
      "format",         GST_PROPS_LIST (
                          GST_PROPS_FOURCC (GST_MAKE_FOURCC ('Y','U','Y','2')),
                          GST_PROPS_FOURCC (GST_MAKE_FOURCC ('R','G','B',' '))
                        ),
      "width",          GST_PROPS_INT_RANGE (16, 4096),
      "height",         GST_PROPS_INT_RANGE (16, 4096)
  ),
  GST_CAPS_NEW (
    "quicktimetypes_src",
    "video/quicktime",
      "format",         GST_PROPS_STRING ("strf_vids")
  ),
  GST_CAPS_NEW (
    "src_audio",
    "audio/raw",
      "format",           GST_PROPS_STRING ("int"),
      "law",              GST_PROPS_INT (0),
      "endianness",       GST_PROPS_INT (G_BYTE_ORDER),
      "signed",           GST_PROPS_LIST (
      			    GST_PROPS_BOOLEAN (TRUE),
      			    GST_PROPS_BOOLEAN (FALSE)
			  ),
      "width",            GST_PROPS_LIST (
      			    GST_PROPS_INT (8),
      			    GST_PROPS_INT (16)
			  ),
      "depth",            GST_PROPS_LIST (
      			    GST_PROPS_INT (8),
      			    GST_PROPS_INT (16)
			  ),
      "rate",             GST_PROPS_INT_RANGE (11025, 44100),
      "channels",         GST_PROPS_INT_RANGE (1, 2)
  ),
  GST_CAPS_NEW (
    "src_audio",
    "audio/x-mp3",
    NULL
  ),
  GST_CAPS_NEW (
    "src_video",
    "video/jpeg",
    NULL
    ),
 GST_CAPS_NEW (
    "src_video",
    "video/3ivx",
    "width",     GST_PROPS_INT_RANGE (16, 4096),
    "height",    GST_PROPS_INT_RANGE (16, 4096),
    NULL
  )  
)

static void 	gst_quicktime_types_init	(GstQuicktimeTypes *quicktime_types);
static void 	gst_quicktime_types_class_init	(GstQuicktimeTypesClass *klass);

static void 	gst_quicktime_types_chain 	(GstPad *pad, GstBuffer *buffer); 

static void 	gst_quicktime_types_get_property	(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static GstElementClass *parent_class = NULL;
/*static guint gst_quicktime_types_signals[LAST_SIGNAL] = { 0 }; */

GType
gst_quicktime_types_get_type(void) 
{
  static GType quicktime_types_type = 0;

  if (!quicktime_types_type) {
    static const GTypeInfo quicktime_types_info = {
      sizeof(GstQuicktimeTypesClass),      NULL,
      NULL,
      (GClassInitFunc)gst_quicktime_types_class_init,
      NULL,
      NULL,
      sizeof(GstQuicktimeTypes),
      0,
      (GInstanceInitFunc)gst_quicktime_types_init,
    };
    quicktime_types_type = g_type_register_static (GST_TYPE_ELEMENT, "GstQuicktimeTypes", &quicktime_types_info, 0);
  }
  return quicktime_types_type;
}

static void
gst_quicktime_types_class_init (GstQuicktimeTypesClass *klass) 
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);
  
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_TYPE_FOUND,
    g_param_spec_boolean("type_found","type_found","type_found",
                         TRUE,G_PARAM_READABLE)); /* CHECKME */

  gobject_class->get_property = gst_quicktime_types_get_property;
}

static GstPadLinkReturn 
gst_quicktime_types_sinkconnect (GstPad *pad, GstCaps *caps) 
{
  GstQuicktimeTypes *quicktime_types;
  const gchar *format;
  GstCaps *newcaps = NULL;

  GST_DEBUG (0, "gst_quicktime_types_sinkconnect begin");
  quicktime_types = GST_QUICKTIME_TYPES (gst_pad_get_parent (pad));

  gst_caps_get_string (caps, "format", &format);

  if (!strcmp (format, "strf_vids")) {
    guint32 video_format;
    gst_caps_get_fourcc_int (caps, "compression", &video_format);

    GST_INFO (GST_CAT_NEGOTIATION, "video_format %08x, 3ivx format %08x\n", 
		    video_format, GST_MAKE_FOURCC ('3','I','V','1'));

    switch (video_format) {
      /* MJPG-A */
      case GST_MAKE_FOURCC ('m','j','p','a'):
      /* PHOTO JPEG */
      case GST_MAKE_FOURCC ('j','p','e','g'):
        newcaps = gst_caps_new ("quiktime_type_mjpg",
			        "video/jpeg", NULL);
        break;
      case GST_MAKE_FOURCC ('3','I','V','1'):
      {
	gint width, height;
	gst_caps_get_int (caps, "width", &width);
	gst_caps_get_int (caps, "height", &height);

        newcaps = gst_caps_new ("quiktime_type_3ivx",
			        "video/3ivx", 
				gst_props_new (
			          "width",  GST_PROPS_INT (width),
				  "height", GST_PROPS_INT (height),
				  NULL
				));
        break;
      }
      default:
      {
	const gchar *compression;
        /* Don't understand why it passes here ... :((( */
	gst_caps_get_string(caps, "compression", &compression);
        printf("Quicktime codec [%4.4s] currently not supported ... \n", compression);
      }
        break;
    }
  }
  else if (!strcmp (format, "strf_auds")) {
    gint audio_format;
    gint blockalign, size, channels, rate;
    gboolean sign		= (size == 8 ? FALSE : TRUE);
    gst_caps_get_int (caps, "fmt", &audio_format);
    gst_caps_get_int (caps, "blockalign", &blockalign);
    gst_caps_get_int (caps, "size", &size);
    gst_caps_get_int (caps, "channels", &channels);
    gst_caps_get_int (caps, "rate", &rate);

    GST_DEBUG (GST_CAT_PLUGIN_INFO, "quicktimetypes: new caps with audio format:%04x", audio_format);

    switch (audio_format) {
      case 0x0001:
        newcaps = gst_caps_new ("quicktime_type_pcm",
		    "audio/raw",
		    gst_props_new (
		      "format",           GST_PROPS_STRING ("int"),
		      "law",              GST_PROPS_INT (0),
		      "endianness",       GST_PROPS_INT (G_BYTE_ORDER),
		      "signed",           GST_PROPS_BOOLEAN (sign),
		      "width",            GST_PROPS_INT ((blockalign*8)/channels),
		      "depth",            GST_PROPS_INT (size),
		      "rate",             GST_PROPS_INT (rate),
		      "channels",         GST_PROPS_INT (channels),
		      NULL
		    ));
        break;
      case 0x0050:
      case 0x0055:
        newcaps = gst_caps_new ("quicktime_type_mp3",
		                "audio/x-mp3", NULL);
        break;
      default:
        break;
    }
  }
  
  if (newcaps) {
    gst_pad_try_set_caps (quicktime_types->srcpad, newcaps);
    quicktime_types->type_found = TRUE;
    return GST_PAD_LINK_OK;
  }
  return GST_PAD_LINK_REFUSED;
}

static void 
gst_quicktime_types_init (GstQuicktimeTypes *quicktime_types) 
{
  quicktime_types->sinkpad = gst_pad_new_from_template (
		  GST_PAD_TEMPLATE_GET (sink_templ), "sink");
  gst_element_add_pad (GST_ELEMENT (quicktime_types), quicktime_types->sinkpad);
  gst_pad_set_link_function (quicktime_types->sinkpad, gst_quicktime_types_sinkconnect);
  gst_pad_set_chain_function (quicktime_types->sinkpad, gst_quicktime_types_chain);

  quicktime_types->srcpad = gst_pad_new_from_template (
		  GST_PAD_TEMPLATE_GET (src_templ), "src");
  gst_element_add_pad (GST_ELEMENT (quicktime_types), quicktime_types->srcpad);

  quicktime_types->type_found = FALSE;
}

static void 
gst_quicktime_types_chain (GstPad *pad, GstBuffer *buffer) 
{
  GstQuicktimeTypes *quicktime_types;

  quicktime_types = GST_QUICKTIME_TYPES (gst_pad_get_parent (pad));

  if (GST_PAD_IS_LINKED (quicktime_types->srcpad))
    gst_pad_push (quicktime_types->srcpad, buffer);
  else
    gst_buffer_unref (buffer);
}

static void
gst_quicktime_types_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GstQuicktimeTypes *src;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_QUICKTIME_TYPES (object));

  src = GST_QUICKTIME_TYPES (object);

  switch (prop_id) {
    case ARG_TYPE_FOUND:
      g_value_set_boolean (value, src->type_found);
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

  /* create an elementfactory for the quicktime_types element */
  factory = gst_element_factory_new ("quicktimetypes", GST_TYPE_QUICKTIME_TYPES,
                                    &gst_quicktime_types_details);
  g_return_val_if_fail (factory != NULL, FALSE);

  gst_element_factory_add_pad_template (factory, GST_PAD_TEMPLATE_GET (src_templ));
  gst_element_factory_add_pad_template (factory, GST_PAD_TEMPLATE_GET (sink_templ));

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "quicktimetypes",
  plugin_init
};


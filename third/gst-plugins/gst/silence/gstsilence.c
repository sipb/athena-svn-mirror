/* GStreamer
 * Copyright (C) 1999,2000 Erik Walthinsen <omega@cse.ogi.edu>
 *                    2000 Wim Taymans <wtay@chello.be>
 *
 * gstsilence.c: 
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
#include <gstsilence.h>


/* Silence signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_BYTESPERREAD,
  ARG_LAW,
  ARG_CHANNELS,
  ARG_FREQUENCY
};

/* elementfactory information */
static GstElementDetails gst_silence_details = {
  "silence source",
  "Source/Audio",
  "LGPL",
  "Generates silence",
  VERSION,
  "Zaheer Merali <zaheer@grid9.net>",
  "(C) 2001",
};

GST_PAD_TEMPLATE_FACTORY (silence_src_factory,
  "src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
    "silence_src",
    "audio/raw",
      "format",		GST_PROPS_STRING ("int"),
      "law",     	GST_PROPS_INT_RANGE (0,1),
      "endianness",     GST_PROPS_INT (G_BYTE_ORDER),
      "signed",   	GST_PROPS_LIST (
					GST_PROPS_BOOLEAN (TRUE),
					GST_PROPS_BOOLEAN (FALSE)
			),
      "width",   	GST_PROPS_LIST (
			  GST_PROPS_INT (8),
			  GST_PROPS_INT (16)
			),
      "depth",   	GST_PROPS_LIST (
			  GST_PROPS_INT (8),
			  GST_PROPS_INT (16)
			),
      "rate",     	GST_PROPS_INT_RANGE (8000, 48000),
      "channels", 	GST_PROPS_INT_RANGE (1, 2)
  )
)

static void 			gst_silence_class_init	(GstSilenceClass *klass);
static void 			gst_silence_init		(GstSilence *silence);

static void 			gst_silence_set_property	(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void 			gst_silence_get_property	(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static GstElementStateReturn 	gst_silence_change_state	(GstElement *element);

static void 			gst_silence_sync_parms	(GstSilence *silence);

static GstBuffer *		gst_silence_get		(GstPad *pad);

static GstElementClass *parent_class = NULL;
/*static guint gst_osssrc_signals[LAST_SIGNAL] = { 0 }; */

GType
gst_silence_get_type (void) 
{
  static GType silence_type = 0;

  if (!silence_type) {
    static const GTypeInfo silence_info = {
      sizeof(GstSilenceClass),
      NULL,
      NULL,
      (GClassInitFunc)gst_silence_class_init,
      NULL,
      NULL,
      sizeof(GstSilence),
      0,
      (GInstanceInitFunc)gst_silence_init,
    };
    silence_type = g_type_register_static (GST_TYPE_ELEMENT, "GstSilence", &silence_info, 0);
  }
  return silence_type;
}

static void
gst_silence_class_init (GstSilenceClass *klass) 
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_BYTESPERREAD,
    g_param_spec_ulong("bytes_per_read","bytes_per_read","bytes_per_read",
                       0,G_MAXULONG,0,G_PARAM_READWRITE)); /* CHECKME */
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_LAW,
    g_param_spec_int("law","law","law",
                     G_MININT,G_MAXINT,0,G_PARAM_READWRITE)); /* CHECKME */
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_CHANNELS,
    g_param_spec_int("channels","channels","channels",
                     G_MININT,G_MAXINT,0,G_PARAM_READWRITE)); /* CHECKME */
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_FREQUENCY,
    g_param_spec_int("frequency","frequency","frequency",
                     G_MININT,G_MAXINT,0,G_PARAM_READWRITE)); /* CHECKME   */
  gobject_class->set_property = gst_silence_set_property;
  gobject_class->get_property = gst_silence_get_property;

  gstelement_class->change_state = gst_silence_change_state;
}

static void 
gst_silence_init (GstSilence *silence) 
{
  silence->srcpad = gst_pad_new_from_template (
					       GST_PAD_TEMPLATE_GET 
					       (silence_src_factory), "src");
  gst_pad_set_get_function(silence->srcpad,gst_silence_get);
  gst_element_add_pad (GST_ELEMENT (silence), silence->srcpad);

  /* adding some default values */
  silence->law = 0;
  silence->channels = 2;
  silence->frequency = 44100;
  silence->bytes_per_read = 4096;
}

static GstBuffer *
gst_silence_get (GstPad *pad)
{
  GstSilence *src;
  GstBuffer *buf;

  g_return_val_if_fail (pad != NULL, NULL);
  src = GST_SILENCE(gst_pad_get_parent (pad));

  buf = gst_buffer_new ();
  g_return_val_if_fail (buf, NULL);
  
  GST_BUFFER_DATA (buf) = (gpointer)g_malloc (src->bytes_per_read);

  if (src->law == 0)
    memset(GST_BUFFER_DATA (buf), 0, src->bytes_per_read);
  else if (src->law == 1)
    memset(GST_BUFFER_DATA (buf), 128, src->bytes_per_read);
  GST_BUFFER_SIZE (buf) = src->bytes_per_read;
  
  return buf;
}

static void 
gst_silence_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec) 
{
  GstSilence *src;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_SILENCE (object));
  
  src = GST_SILENCE (object);

  switch (prop_id) {
    case ARG_BYTESPERREAD:
      src->bytes_per_read = g_value_get_ulong (value);
      break;
    case ARG_LAW:
      src->law = g_value_get_int (value);
      break;
    case ARG_CHANNELS:
      src->channels = g_value_get_int (value);
      break;
    case ARG_FREQUENCY:
      src->frequency = g_value_get_int (value);
      break;
    default:
      break;
  }
}

static void 
gst_silence_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec) 
{
  GstSilence *src;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_SILENCE (object));
  
  src = GST_SILENCE (object);

  switch (prop_id) {
    case ARG_BYTESPERREAD:
      g_value_set_ulong (value, src->bytes_per_read);
      break;
    case ARG_LAW:
      g_value_set_int (value, src->law);
      break;
    case ARG_CHANNELS:
      g_value_set_int (value, src->channels);
      break;
    case ARG_FREQUENCY:
      g_value_set_int (value, src->frequency);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static GstElementStateReturn 
gst_silence_change_state (GstElement *element) 
{
  g_return_val_if_fail (GST_IS_SILENCE (element), FALSE);
  GST_DEBUG (0, "osssrc: state change");

  if (GST_STATE_PENDING (element) != GST_STATE_NULL) {
    gst_silence_sync_parms (GST_SILENCE (element));
  }
  if (GST_ELEMENT_CLASS (parent_class)->change_state)
    return GST_ELEMENT_CLASS (parent_class)->change_state (element);
  
  return GST_STATE_SUCCESS;
}

static void 
gst_silence_sync_parms (GstSilence *silence)
{
  /* set caps on src pad */
  gst_pad_try_set_caps (silence->srcpad, 
		      GST_CAPS_NEW (
    			"silence_src",
    			"audio/raw",
      			  "format",       GST_PROPS_STRING ("int"),
        		  "law",        GST_PROPS_INT (silence->law),              /*FIXME */
        		  "endianness", GST_PROPS_INT (G_BYTE_ORDER),   /*FIXME */
        		  "signed",     GST_PROPS_BOOLEAN (TRUE),	/*FIXME */
        		  "width",      GST_PROPS_INT (silence->law ? 8 : 16),
        		  "depth",      GST_PROPS_INT (silence->law ? 8 : 16),
        		  "rate",       GST_PROPS_INT (silence->frequency),
        		  "channels",   GST_PROPS_INT (silence->channels)
        	       )); 
}

static gboolean
plugin_init (GModule *module, GstPlugin *plugin)
{
  GstElementFactory *factory;

  /* create the plugin structure */
  /* create an elementfactory for the parseau element and list it */
  factory = gst_element_factory_new ("silence", GST_TYPE_SILENCE,
                                    &gst_silence_details);
  g_return_val_if_fail (factory != NULL, FALSE);

  gst_element_factory_add_pad_template (factory, GST_PAD_TEMPLATE_GET (silence_src_factory));

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "silence",
  plugin_init
};

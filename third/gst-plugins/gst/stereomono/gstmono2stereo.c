/* GStreamer
 * Copyright (C) <2001> Steve Baker <stevebaker_org@yahoo.co.uk>
 *
 * mono2stereo.c
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

#include <gstmono2stereo.h>

/* elementfactory information */
static GstElementDetails mono2stereo_details = {
  "Mono to Stereo effect",
  "Filter/Audio/Conversion",
  "LGPL",
  "Take a single channel and send it over 2 channels with pan",
  VERSION,
  "Steve Baker <stevebaker_org@yahoo.co.uk>",
  "(C) 2001",
};


/* Stereo signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_PAN,
};

static GstPadTemplate*
mono2stereo_sink_factory (void)
{
  return 
    gst_pad_template_new (
  	"sink",
  	GST_PAD_SINK,
  	GST_PAD_ALWAYS,
  	gst_caps_new (
  	  "int_mono_sink",
    	  "audio/raw",
	  gst_props_new (
            "format",       GST_PROPS_STRING ("int"),   
              "law",        GST_PROPS_INT (0),
              "endianness", GST_PROPS_INT (G_BYTE_ORDER),
              "signed",     GST_PROPS_BOOLEAN (TRUE),
              "width",      GST_PROPS_INT (16),
              "depth",      GST_PROPS_INT (16),
              "rate",       GST_PROPS_INT_RANGE (8000, 48000),
    	    "channels", GST_PROPS_INT (1),
	    NULL)),
	NULL);
}

static GstPadTemplate*
mono2stereo_src_factory (void) 
{
  return 
    gst_pad_template_new (
  	"src",
  	GST_PAD_SRC,
  	GST_PAD_ALWAYS,
  	gst_caps_new (
  	  "int_stereo_src",
    	  "audio/raw",
	  gst_props_new (
            "format",       GST_PROPS_STRING ("int"),   
              "law",        GST_PROPS_INT (0),
              "endianness", GST_PROPS_INT (G_BYTE_ORDER),
              "signed",     GST_PROPS_BOOLEAN (TRUE),
              "width",      GST_PROPS_INT (16),
              "depth",      GST_PROPS_INT (16),
              "rate",       GST_PROPS_INT_RANGE (8000, 48000),
    	    "channels",    GST_PROPS_INT (2),
	    NULL)),
	NULL);
}

static GstPadTemplate *srctempl, *sinktempl;

static void	gst_mono2stereo_class_init		(GstMono2StereoClass *klass);
static void	gst_mono2stereo_init			(GstMono2Stereo *plugin);
static inline void gst_mono2stereo_update_pan(GstMono2Stereo *plugin);
static void	gst_mono2stereo_set_property		(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void	gst_mono2stereo_get_property		(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static void	gst_mono2stereo_chain		(GstPad *pad, GstBuffer *buf);

static GstElementClass *parent_class = NULL;

static GstPadLinkReturn
gst_mono2stereo_connect (GstPad *pad, GstCaps *caps)
{
  GstMono2Stereo *filter;
  GstPad *otherpad;
  GstCaps *othercaps;
  
  filter = GST_MONO2STEREO (gst_pad_get_parent (pad));
  g_return_val_if_fail (filter != NULL, GST_PAD_LINK_REFUSED);
  g_return_val_if_fail (GST_IS_MONO2STEREO (filter), GST_PAD_LINK_REFUSED);
  otherpad = (pad == filter->srcpad ? filter->sinkpad : filter->srcpad);
  
  if (GST_CAPS_IS_FIXED (caps)) {
    GstPadLinkReturn set_retval;
    othercaps = gst_caps_copy (caps);
    if (otherpad == filter->srcpad)
      gst_caps_set (othercaps, "channels", GST_PROPS_INT (2));
    else
      gst_caps_set (othercaps, "channels", GST_PROPS_INT (1));

    if ((set_retval = gst_pad_try_set_caps(otherpad, othercaps)) > 0)
      gst_caps_get_int (caps, "width", &filter->width);
    
    return set_retval;
  }
  
  return GST_PAD_LINK_DELAYED;
}

GType
gst_mono2stereo_get_type(void) {
  static GType mono2stereo_type = 0;

  if (!mono2stereo_type) {
    static const GTypeInfo mono2stereo_info = {
      sizeof(GstMono2StereoClass),      NULL,
      NULL,
      (GClassInitFunc)gst_mono2stereo_class_init,
      NULL,
      NULL,
      sizeof(GstMono2Stereo),
      0,
      (GInstanceInitFunc)gst_mono2stereo_init,
    };
    mono2stereo_type = g_type_register_static(GST_TYPE_ELEMENT, "GstMono2Stereo", &mono2stereo_info, 0);
  }
  return mono2stereo_type;
}

static void
gst_mono2stereo_class_init (GstMono2StereoClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)klass;

  parent_class = g_type_class_ref(GST_TYPE_ELEMENT);

  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_PAN,
    g_param_spec_float("pan","pan","pan",
                       -1.0,1.0,0.0,G_PARAM_READWRITE)); /* CHECKME */

  gobject_class->set_property = gst_mono2stereo_set_property;
  gobject_class->get_property = gst_mono2stereo_get_property;
}

static void
gst_mono2stereo_init (GstMono2Stereo *plugin)
{
  plugin->sinkpad = gst_pad_new_from_template (sinktempl, "sink");
  plugin->srcpad  = gst_pad_new_from_template (srctempl,  "src");

  gst_element_add_pad (GST_ELEMENT (plugin), plugin->sinkpad);
  gst_element_add_pad (GST_ELEMENT (plugin), plugin->srcpad);

  gst_pad_set_link_function (plugin->sinkpad, gst_mono2stereo_connect);
  gst_pad_set_link_function (plugin->srcpad,  gst_mono2stereo_connect);

  gst_pad_set_chain_function (plugin->sinkpad, gst_mono2stereo_chain);

  plugin->pan=0.0;
  gst_mono2stereo_update_pan(plugin);
}

static inline void gst_mono2stereo_update_pan(GstMono2Stereo *plugin)
{
  plugin->pan_right = (plugin->pan+1.0) / 2.0;
  plugin->pan_left = 1.0 - plugin->pan_right;
  GST_DEBUG_ELEMENT(0, GST_ELEMENT(plugin), "update pan: %f %f %f\n", plugin->pan, plugin->pan_left, plugin->pan_right);
}

static void
gst_mono2stereo_chain (GstPad *pad, GstBuffer *buf_in)
{
  GstMono2Stereo *plugin;
  GstBuffer *buf_out;
  gint16 *data_in, *data_out;
  gint i;
  gint num_frames;
  
  g_return_if_fail(pad != NULL);
  g_return_if_fail(GST_IS_PAD(pad));
  g_return_if_fail(buf_in != NULL);

  plugin = GST_MONO2STEREO(GST_OBJECT_PARENT (pad));
  g_return_if_fail(plugin != NULL);
  g_return_if_fail(GST_IS_MONO2STEREO(plugin));

  if (GST_IS_EVENT(buf_in)) {
    gst_pad_event_default (plugin->srcpad, GST_EVENT (buf_in));
    return;
  }
  
  data_in = (gint16 *)GST_BUFFER_DATA(buf_in);
  num_frames = GST_BUFFER_SIZE(buf_in)/2;

  buf_out = gst_buffer_new();
  g_return_if_fail (buf_out);
  data_out = g_new(gint16, num_frames * 2);
  GST_BUFFER_DATA(buf_out) = (gpointer) data_out;
  GST_BUFFER_SIZE(buf_out) = num_frames * 4;
  GST_BUFFER_OFFSET(buf_out) = GST_BUFFER_OFFSET(buf_in);
  GST_BUFFER_TIMESTAMP(buf_out) = GST_BUFFER_TIMESTAMP(buf_in);

  for (i = 0; i < num_frames; i++) {
    data_out[i*2] = data_in[i] * plugin->pan_left;
    data_out[i*2+1] = data_in[i] * plugin->pan_right;
  }
  
  gst_buffer_unref(buf_in);
  gst_pad_push(plugin->srcpad,buf_out);
}

static void
gst_mono2stereo_set_property (GObject *object, guint prop_id, 
		              const GValue *value, GParamSpec *pspec)
{
  GstMono2Stereo *plugin;

  g_return_if_fail (GST_IS_MONO2STEREO (object));
  plugin = GST_MONO2STEREO (object);

  switch (prop_id) {
    case ARG_PAN:
      if (g_value_get_double (value) < -1.0){
        plugin->pan = -1.0;
      } else if (g_value_get_double (value) > 1.0){
        plugin->pan = 1.0;
      } else {
        plugin->pan = g_value_get_float (value);
      }
      gst_mono2stereo_update_pan (plugin);
      break;
    default:
      break;
  }
}

static void
gst_mono2stereo_get_property (GObject *object, guint prop_id, 
		              GValue *value, GParamSpec *pspec)
{
  GstMono2Stereo *plugin;

  g_return_if_fail (GST_IS_MONO2STEREO (object));
  plugin = GST_MONO2STEREO (object);

  switch (prop_id) {
    case ARG_PAN:
      g_value_set_float (value, plugin->pan);
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

  factory = gst_element_factory_new ("mono2stereo", GST_TYPE_MONO2STEREO,
                                     &mono2stereo_details);
  g_return_val_if_fail (factory != NULL, FALSE);
  gst_element_factory_set_rank (factory, GST_ELEMENT_RANK_PRIMARY);

  srctempl = mono2stereo_src_factory ();
  gst_element_factory_add_pad_template (factory, srctempl);

  sinktempl = mono2stereo_sink_factory ();
  gst_element_factory_add_pad_template (factory, sinktempl);
  
  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "mono2stereo",
  plugin_init
};

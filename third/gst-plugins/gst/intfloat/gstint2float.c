/* GStreamer
 * Copyright (C) <2001> Steve Baker <stevebaker_org@yahoo.co.uk>
 *
 * int2float.c
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

#include <gstint2float.h>

/* elementfactory information */
static GstElementDetails int2float_details = {
  "Integer to Float effect",
  "Filter/Audio/Conversion",
  "LGPL",
  "Convert from integer to floating point audio data",
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

GST_PAD_TEMPLATE_FACTORY (int2float_src_factory,
  "src%d",
  GST_PAD_SRC,
  GST_PAD_REQUEST,
  GST_CAPS_NEW (
    "float_src",
    "audio/raw",
    "rate",       GST_PROPS_INT_RANGE (4000, 96000),
    "format",     GST_PROPS_STRING ("float"),
    "layout",     GST_PROPS_STRING ("gfloat"),
    "intercept",  GST_PROPS_FLOAT (0.0),
    "slope",      GST_PROPS_FLOAT (1.0),
    "channels",   GST_PROPS_INT (1)
  )
);

GST_PAD_TEMPLATE_FACTORY (int2float_sink_factory,
  "sink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
    "int_sink",
    "audio/raw",
    "format",     GST_PROPS_STRING ("int"),
    "channels",   GST_PROPS_INT_RANGE (1, G_MAXINT),
    "rate",       GST_PROPS_INT_RANGE (4000, 96000),
    "law",        GST_PROPS_INT (0),
    "endianness", GST_PROPS_INT (G_BYTE_ORDER),
    "width",      GST_PROPS_INT (16),
    "depth",      GST_PROPS_INT (16),
    "signed",     GST_PROPS_BOOLEAN (TRUE)
  )
);

static void                 gst_int2float_class_init (GstInt2FloatClass *klass);
static void                 gst_int2float_init (GstInt2Float *plugin);

static GstPadLinkReturn  gst_int2float_connect (GstPad *pad, GstCaps *caps);

static void                 gst_int2float_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void                 gst_int2float_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static GstPad*              gst_int2float_request_new_pad (GstElement *element, GstPadTemplate *temp, const gchar *unused);
static void                 gst_int2float_chain_gint16 (GstPad *pad, GstBuffer *buf_in);
static GstElementStateReturn gst_int2float_change_state (GstElement *element);
static inline GstInt2Float* gst_int2float_get_plugin (GstPad *pad,GstBuffer *buf);

static GstPadTemplate  *srctempl, *sinktempl;
static GstElementClass *parent_class = NULL;

GType
gst_int2float_get_type(void) {
  static GType int2float_type = 0;

  if (!int2float_type) {
    static const GTypeInfo int2float_info = {
      sizeof(GstInt2FloatClass),      NULL,
      NULL,
      (GClassInitFunc)gst_int2float_class_init,
      NULL,
      NULL,
      sizeof(GstInt2Float),
      0,
      (GInstanceInitFunc)gst_int2float_init,
    };
    int2float_type = g_type_register_static(GST_TYPE_ELEMENT, "GstInt2Float", &int2float_info, 0);
  }
  return int2float_type;
}

static void
gst_int2float_class_init (GstInt2FloatClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)klass;

  parent_class = g_type_class_ref(GST_TYPE_ELEMENT);

  gobject_class->set_property = gst_int2float_set_property;
  gobject_class->get_property = gst_int2float_get_property;
  gstelement_class->request_new_pad = gst_int2float_request_new_pad;
  gstelement_class->change_state = gst_int2float_change_state;
}

static void
gst_int2float_init (GstInt2Float *plugin)
{
  plugin->sinkpad = gst_pad_new_from_template(sinktempl,"sink");

  gst_element_add_pad(GST_ELEMENT(plugin),plugin->sinkpad);
  gst_pad_set_chain_function(plugin->sinkpad,gst_int2float_chain_gint16);
  
  gst_pad_set_link_function(plugin->sinkpad, gst_int2float_connect);

  plugin->numsrcpads = 0;
  plugin->srcpads = NULL;
  
  plugin->intcaps = NULL;
  plugin->floatcaps = NULL;
  plugin->channels = 0;
}

/* see notes in gstfloat2int.c */
static GstPadLinkReturn
gst_int2float_connect (GstPad *pad, GstCaps *caps)
{
  GstInt2Float *filter;
  GstCaps *intcaps, *floatcaps;
  GSList *l;
  
  filter = GST_INT2FLOAT (GST_PAD_PARENT (pad));

  if (GST_CAPS_IS_FIXED (caps)) {
    if (pad == filter->sinkpad) {
      /* it's the int pad */
      if (!filter->intcaps) {
        floatcaps = gst_caps_copy (gst_pad_template_get_caps (int2float_src_factory ()));
        gst_caps_get_int (caps, "rate", &filter->rate);
        gst_caps_set (floatcaps, "rate", GST_PROPS_INT (filter->rate));
        GST_CAPS_FLAG_SET (floatcaps, GST_CAPS_FIXED);
        GST_PROPS_FLAG_SET (floatcaps->properties, GST_PROPS_FIXED);

        filter->in_capsnego = TRUE;

	/* FIXME: refcounting? */
        filter->floatcaps = floatcaps;
        filter->intcaps = caps;
        gst_caps_get_int (caps, "channels", &filter->channels);
    
        for (l=filter->srcpads; l; l=l->next)
          if (gst_pad_try_set_caps ((GstPad*) l->data, floatcaps) <= 0)
            return GST_PAD_LINK_REFUSED;
        
        filter->in_capsnego = FALSE;
        return GST_PAD_LINK_OK;
      } else { 
        gst_caps_set (filter->intcaps, "channels", GST_PROPS_INT_RANGE (1, G_MAXINT));
        GST_CAPS_FLAG_UNSET (filter->intcaps, GST_CAPS_FIXED);
        if (gst_caps_intersect (caps, filter->intcaps)) {
          gst_caps_get_int (caps, "channels", &filter->channels);
          gst_caps_set (filter->intcaps, "channels", GST_PROPS_INT (filter->channels));
	  GST_CAPS_FLAG_SET (filter->intcaps, GST_CAPS_FIXED);
          GST_PROPS_FLAG_SET (filter->intcaps->properties, GST_PROPS_FIXED);
          return GST_PAD_LINK_OK;
        } else {
          gst_caps_set (filter->intcaps, "channels", GST_PROPS_INT (filter->channels));
	  GST_CAPS_FLAG_SET (filter->intcaps, GST_CAPS_FIXED);
          GST_PROPS_FLAG_SET (filter->intcaps->properties, GST_PROPS_FIXED);
          return GST_PAD_LINK_REFUSED;
        }
      }
    } else {
      /* it's a float pad */
      gint rate;

      gst_caps_get_int (caps, "rate", &rate);

      if (filter->in_capsnego) {
        /* caps were set on the sink pad, so i started setting on the src pads.
           capsnego reached the end and bounced back to float2int, that started
           setting all of its sink pads, which brought me back here. */
        if (rate == filter->rate)
          return GST_PAD_LINK_OK;
        else
          return GST_PAD_LINK_REFUSED;
      }

      intcaps = gst_caps_copy (gst_pad_template_get_caps (int2float_sink_factory ()));
      gst_caps_set (intcaps, "rate", GST_PROPS_INT (rate), NULL);

      /* intcaps is now variable in channels */

      if (!filter->intcaps) {
        filter->rate = rate;

        if (GST_PAD_PEER (filter->sinkpad)) {
          GstCaps *newcaps, *gottencaps;

          gottencaps = gst_pad_get_allowed_caps (GST_PAD_PEER (filter->sinkpad));
          gst_caps_debug (intcaps, "int caps");
          gst_caps_debug (gottencaps, "gotten_caps");
          newcaps = gst_caps_intersect (gottencaps, intcaps);
          if (!newcaps)
            return GST_PAD_LINK_REFUSED;
          
          if (GST_CAPS_IS_FIXED (newcaps)) {
            gst_caps_get_int (newcaps, "channels", &filter->channels);
            if (gst_pad_try_set_caps (filter->sinkpad, newcaps) <= 0)
              return GST_PAD_LINK_DELAYED;
          }

          filter->intcaps = newcaps;
        } else {
          /* we assume channels is equal to the number of source pads */
          gst_caps_set (intcaps, "channels", GST_PROPS_INT (filter->numsrcpads), NULL);
	  GST_CAPS_FLAG_SET (intcaps, GST_CAPS_FIXED);
          GST_PROPS_FLAG_SET (intcaps->properties, GST_PROPS_FIXED);
          filter->channels = filter->numsrcpads;
        
          if (gst_pad_try_set_caps (filter->sinkpad, intcaps) <= 0)
            return GST_PAD_LINK_REFUSED;

          filter->intcaps = intcaps;
        }
        
        for (l=filter->srcpads; l; l=l->next)
          if (! ((GstPad*) l->data == pad || gst_pad_try_set_caps ((GstPad*) l->data, caps) > 0))
            return GST_PAD_LINK_REFUSED;
        
        /* FIXME: refcounting? */
        filter->floatcaps = caps;
        return GST_PAD_LINK_OK;
      } else if (gst_caps_intersect (intcaps, filter->intcaps)) {
        return GST_PAD_LINK_OK;
      } else {
        return GST_PAD_LINK_REFUSED;
      }
    }
  } else {
    return GST_PAD_LINK_DELAYED;
  }
}

static GstPad*
gst_int2float_request_new_pad (GstElement *element, GstPadTemplate *templ, const gchar *unused) 
{
  gchar *name;
  GstPad *srcpad;
  GstInt2Float *plugin;

  plugin = GST_INT2FLOAT(element);
  
  g_return_val_if_fail(plugin != NULL, NULL);
  g_return_val_if_fail(GST_IS_INT2FLOAT(plugin), NULL);

  if (templ->direction != GST_PAD_SRC) {
    g_warning ("int2float: request new pad that is not a SRC pad\n");
    return NULL;
  }

  name = g_strdup_printf ("src%d", plugin->numsrcpads);
  srcpad = gst_pad_new_from_template (templ, name);
  g_free (name);
  gst_element_add_pad (GST_ELEMENT (plugin), srcpad);
  gst_pad_set_link_function (srcpad, gst_int2float_connect);
  
  plugin->srcpads = g_slist_append (plugin->srcpads, srcpad);
  plugin->numsrcpads++;
  
  return srcpad;
}

static void
gst_int2float_chain_gint16 (GstPad *pad, GstBuffer *buf_in)
{
  GstInt2Float *plugin;
  gint16 *data_in;
  gfloat *data_out;
  gint num_frames, i, j;
  GSList *srcpads;
  GstBuffer **buffers;
  guint buffer_byte_size;
  GstBufferPool *pool;
  
  g_return_if_fail((plugin = gst_int2float_get_plugin(pad,buf_in)));

  if (!plugin->channels) {
    gst_element_error (GST_ELEMENT (plugin), "capsnego was never performed, bailing...");
    return;
  }

  num_frames = GST_BUFFER_SIZE(buf_in)/(2*plugin->channels);
  data_in = (gint16 *)GST_BUFFER_DATA(buf_in);
  buffers = g_new0(GstBuffer*, plugin->numsrcpads);
  buffer_byte_size = sizeof(gfloat) * num_frames;
  pool = gst_buffer_pool_get_default (buffer_byte_size, 4);
  
  for (i = 0; i < plugin->numsrcpads; i++) {

    //buffers[i] = gst_buffer_new_from_pool(pool, 0, 0);
    buffers[i] = gst_buffer_new_and_alloc(buffer_byte_size);
    
    data_out = (gfloat*)GST_BUFFER_DATA(buffers[i]);
    GST_BUFFER_SIZE(buffers[i]) = buffer_byte_size;
    GST_BUFFER_TIMESTAMP(buffers[i]) = GST_BUFFER_TIMESTAMP(buf_in);
    
    for (j = 0; j < num_frames; j++) {
      data_out[j] = ((gfloat)data_in[(j*plugin->channels) + (i % plugin->channels)]) / 32767.0;
    }
  }

  gst_buffer_unref(buf_in);
  srcpads = plugin->srcpads;
   
  for (i = 0; srcpads != NULL; i++) {
    /* g_print("pushing to %s:%s\n", GST_DEBUG_PAD_NAME(GST_INT2FLOAT_SRCPAD(srcpads))); */

    gst_pad_push(GST_INT2FLOAT_SRCPAD(srcpads),buffers[i]);
    srcpads = g_slist_next(srcpads);
  }
  g_free(buffers);
}

static inline GstInt2Float* 
gst_int2float_get_plugin(GstPad *pad,GstBuffer *buf)
{
  GstInt2Float *plugin;
  g_return_val_if_fail(pad != NULL, NULL);
  g_return_val_if_fail(GST_IS_PAD(pad), NULL);
  g_return_val_if_fail(buf != NULL, NULL);

  plugin = GST_INT2FLOAT(GST_OBJECT_PARENT (pad));
  g_return_val_if_fail(plugin != NULL, NULL);
  g_return_val_if_fail(GST_IS_INT2FLOAT(plugin), NULL);
  return plugin;
}

static void
gst_int2float_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  GstInt2Float *plugin;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail(GST_IS_INT2FLOAT(object));
  plugin = GST_INT2FLOAT(object);

  switch (prop_id) {
    default:
      break;
  }
}

static void
gst_int2float_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GstInt2Float *plugin;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail(GST_IS_INT2FLOAT(object));
  plugin = GST_INT2FLOAT(object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static GstElementStateReturn
gst_int2float_change_state (GstElement *element)
{
  GstInt2Float *plugin;

  plugin = GST_INT2FLOAT (element);
  
  switch (GST_STATE_TRANSITION (element)) {
  case GST_STATE_PAUSED_TO_READY:
    plugin->intcaps = NULL;
    plugin->floatcaps = NULL;
    break;

  default:
    break;
  }

  if (GST_ELEMENT_CLASS (parent_class)->change_state) 
    return GST_ELEMENT_CLASS (parent_class)->change_state (element);

  return GST_STATE_SUCCESS;

}

gboolean 
gst_int2float_factory_init (GstPlugin *plugin) 
{
  GstElementFactory *factory;
  
  factory = gst_element_factory_new("int2float",GST_TYPE_INT2FLOAT,
                                   &int2float_details);
  g_return_val_if_fail(factory != NULL, FALSE);
  gst_element_factory_set_rank (factory, GST_ELEMENT_RANK_SECONDARY);
  
  sinktempl = int2float_sink_factory();
  gst_element_factory_add_pad_template (factory, sinktempl);

  srctempl = int2float_src_factory();
  gst_element_factory_add_pad_template (factory, srctempl);
  
  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));
  
  return TRUE;
}

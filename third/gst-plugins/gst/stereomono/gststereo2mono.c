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

#include <gst/gst.h>
#include "gststereo2mono.h"


/* elementfactory information */
static GstElementDetails stereo2mono_details = {
  "Stereo to Mono converter",
  "Filter/Audio/Conversion",
  "LGPL",
  "Convert stereo PCM to mono PCM",
  VERSION,
  "Zaheer Merali <zaheer@bellworldwide.net>",
  "(C) 2001",
};


/* Stereo signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0
};

static GstPadTemplate*
stereo2mono_src_factory (void)
{
  return 
    gst_pad_template_new (
  	"src",
  	GST_PAD_SRC,
  	GST_PAD_ALWAYS,
  	gst_caps_new (
  	  "int_mono_src",
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
stereo2mono_sink_factory (void) 
{
  return 
    gst_pad_template_new (
  	"sink",
  	GST_PAD_SINK,
  	GST_PAD_ALWAYS,
  	gst_caps_new (
  	  "int_stereo_sink",
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

static void		gst_stereo2mono_class_init		(GstStereo2MonoClass *klass);
static void		gst_stereo2mono_init			(GstStereo2Mono *stereo);

/* not used
static void		gst_stereo2mono_set_property			(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void		gst_stereo2mono_get_property			(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
*/

static void		gst_stereo2mono_chain			(GstPad *pad, GstBuffer *buf);
static void inline 	gst_stereo2mono_fast_16bit_chain 	(gint16* data, gint16** mono_data, 
			         				 guint numbytes);
static void inline 	gst_stereo2mono_fast_8bit_chain		(gint8* data, gint8** mono_data,
                                				 guint numbytes);

static GstElementClass *parent_class = NULL;
/*static guint gst_stereo_signals[LAST_SIGNAL] = { 0 }; */

static GstPadLinkReturn
gst_stereo2mono_connect (GstPad *pad, GstCaps *caps)
{
  GstStereo2Mono *filter;
  GstPad *otherpad;
  GstCaps *othercaps;
  
  filter = GST_STEREO2MONO (gst_pad_get_parent (pad));
  g_return_val_if_fail (filter != NULL, GST_PAD_LINK_REFUSED);
  g_return_val_if_fail (GST_IS_STEREO2MONO (filter), GST_PAD_LINK_REFUSED);
  otherpad = (pad == filter->srcpad ? filter->sinkpad : filter->srcpad);
  
  if (GST_CAPS_IS_FIXED (caps)) {
    GstPadLinkReturn set_retval;
    othercaps = gst_caps_copy (caps);
    if (otherpad == filter->srcpad)
      gst_caps_set (othercaps, "channels", GST_PROPS_INT (1));
    else
      gst_caps_set (othercaps, "channels", GST_PROPS_INT (2));
    
    if ((set_retval = gst_pad_try_set_caps (otherpad, othercaps)) > 0)
      gst_caps_get_int (caps, "width", &filter->width);
    
    return set_retval;
  }
  
  return GST_PAD_LINK_DELAYED;
}

GType
gst_stereo2mono_get_type(void) {
  static GType stereo2mono_type = 0;

  if (!stereo2mono_type) {
    static const GTypeInfo stereo2mono_info = {
      sizeof(GstStereo2MonoClass),      NULL,
      NULL,
      (GClassInitFunc)gst_stereo2mono_class_init,
      NULL,
      NULL,
      sizeof(GstStereo2Mono),
      0,
      (GInstanceInitFunc)gst_stereo2mono_init,
    };
    stereo2mono_type = g_type_register_static(GST_TYPE_ELEMENT, "GstStereo2Mono", &stereo2mono_info, 0);
  }
  return stereo2mono_type;
}

static void
gst_stereo2mono_class_init (GstStereo2MonoClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);
/* not used
  gobject_class->set_property = gst_stereo2mono_set_property;
  gobject_class->get_property = gst_stereo2mono_get_property;
*/
}

static void
gst_stereo2mono_init (GstStereo2Mono *stereo)
{
  stereo->sinkpad = gst_pad_new_from_template (sinktempl, "sink");
  gst_pad_set_chain_function (stereo->sinkpad, gst_stereo2mono_chain);
  gst_pad_set_link_function (stereo->sinkpad, gst_stereo2mono_connect);
  gst_element_add_pad (GST_ELEMENT (stereo), stereo->sinkpad);

  stereo->srcpad = gst_pad_new_from_template (srctempl, "src");
  gst_pad_set_link_function (stereo->srcpad, gst_stereo2mono_connect);
  gst_element_add_pad (GST_ELEMENT (stereo), stereo->srcpad);
}

static void
gst_stereo2mono_chain (GstPad *pad,GstBuffer *buf)
{
  GstStereo2Mono *stereo;
  gint16 *data;
  gint16 *mono_data;
  GstBuffer* outbuf;

  g_return_if_fail(pad != NULL);
  g_return_if_fail(GST_IS_PAD(pad));
  g_return_if_fail(buf != NULL);

  stereo = GST_STEREO2MONO(GST_OBJECT_PARENT (pad));
  g_return_if_fail(stereo != NULL);
  g_return_if_fail(GST_IS_STEREO2MONO(stereo));

  if (GST_IS_EVENT(buf)) {
    gst_pad_event_default (stereo->srcpad, GST_EVENT (buf));
    return;
  }
  
  data = (gint16 *)GST_BUFFER_DATA(buf);
  outbuf=gst_buffer_new();
  GST_BUFFER_DATA(outbuf) = (gchar*)g_new(gint16,GST_BUFFER_SIZE(buf)/4);
  GST_BUFFER_SIZE(outbuf) = GST_BUFFER_SIZE(buf)/2;
  GST_BUFFER_OFFSET(outbuf) = GST_BUFFER_OFFSET(buf);
  GST_BUFFER_TIMESTAMP(outbuf) = GST_BUFFER_TIMESTAMP(buf);

  mono_data = (gint16*)GST_BUFFER_DATA(outbuf);
  
  switch (stereo->width) {
    case 16:
      gst_stereo2mono_fast_16bit_chain(data,&mono_data,GST_BUFFER_SIZE(buf));
      break;
    case 8:
      gst_stereo2mono_fast_8bit_chain((gint8*)data,(gint8**)&mono_data,GST_BUFFER_SIZE(buf));
      break;
    default:
      gst_element_error (GST_ELEMENT (stereo), "stereo2mono: capsnego was never performed, bailing...");
      return;
  }
  gst_buffer_unref(buf);
  gst_pad_push(stereo->srcpad,outbuf);
}

static void inline
gst_stereo2mono_fast_16bit_chain(gint16* data, gint16** mono_data, 
			         guint numbytes)
{
  guint i,j;
  /*printf("s2m 16bit: data=0x%x numbytes=%u\n",data,numbytes); */
  for(i=0,j=0;i<numbytes/2;i+=2,j++) {
     /* average the 2 channels */
     (*mono_data)[j]=(data[i]+data[i+1])/2;
  }
}

static void inline
gst_stereo2mono_fast_8bit_chain(gint8* data, gint8** mono_data,
                                guint numbytes)
{
  guint i,j;
  for(i=0,j=0;i<numbytes;i+=2,j++) {
     /* average the 2 channels */
     (*mono_data)[j]=(data[i]+data[i+1])/2;
  }
}
/* not used, so disable by default
static void
gst_stereo2mono_set_property (GObject *object, guint prop_id, 
		              const GValue *value, GParamSpec *pspec)
{
  GstStereo2Mono *stereo;

  g_return_if_fail (GST_IS_STEREO2MONO (object));
  stereo = GST_STEREO2MONO (object);

  switch (prop_id) {
    default:
      break;
  }
}

static void
gst_stereo2mono_get_property (GObject *object, guint prop_id, 
		              GValue *value, GParamSpec *pspec)
{
  GstStereo2Mono *stereo;

  g_return_if_fail (GST_IS_STEREO2MONO (object));
  stereo = GST_STEREO2MONO (object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}
*/

static gboolean
plugin_init (GModule *module, GstPlugin *plugin)
{
  GstElementFactory *factory;

  factory = gst_element_factory_new ("stereo2mono", GST_TYPE_STEREO2MONO,
                                     &stereo2mono_details);
  gst_element_factory_set_rank (factory, GST_ELEMENT_RANK_PRIMARY);
  g_return_val_if_fail (factory != NULL, FALSE);
  
  srctempl = stereo2mono_src_factory ();
  gst_element_factory_add_pad_template (factory, srctempl);

  sinktempl = stereo2mono_sink_factory ();
  gst_element_factory_add_pad_template (factory, sinktempl);

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "stereo2mono",
  plugin_init
};


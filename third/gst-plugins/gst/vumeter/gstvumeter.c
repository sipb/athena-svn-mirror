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


#include <stdlib.h>

#include "gstvumeter.h"

static GstElementDetails gst_vumeter_details = {
  "VU Meter",
  "Sink/Audio",
  "LGPL",
  "Simple volume indicator",
  VERSION,
  "Erik Walthinsen <omega@cse.ogi.edu>",
  "(C) 1999",
};

/* VuMeter signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_VOLUME,
  ARG_VOLUMEPTR,
  ARG_VOLUME_LEFT,
  ARG_VOLUMEPTR_LEFT,
  ARG_VOLUME_RIGHT,
  ARG_VOLUMEPTR_RIGHT,
};

static GstPadTemplate*
sink_factory (void) 
{
  return 
    gst_pad_template_new (
  	"sink",                               /* the name of the pads */
  	GST_PAD_SINK,                         /* type of the pad */
  	GST_PAD_ALWAYS,                       /* ALWAYS/SOMETIMES */
  	gst_caps_new (
     	  "vumeter_sink16",                          /* the name of the caps */
     	  "audio/raw",                               /* the mime type of the caps */
	  gst_props_new (
     		/* Properties follow: */
     	    "format",   GST_PROPS_INT (16),
     	    "depth",    GST_PROPS_INT (16),
	    NULL)),
	NULL);
     /* This properties commented out so that autoplugging works for now: */
     /* the autoplugging needs to be fixed (caps negotiation needed) */
     /*,"channels", GST_PROPS_INT (2) */
}


static void gst_vumeter_class_init(GstVuMeterClass *klass);
static void gst_vumeter_init(GstVuMeter *vumeter);

static void gst_vumeter_chain(GstPad *pad,GstBuffer *buf);

static void gst_vumeter_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void gst_vumeter_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

GstPadTemplate *sink_template;

static GstElementClass *parent_class = NULL;
/*static guint gst_vumeter_signals[LAST_SIGNAL] = { 0 }; */


GType
gst_vumeter_get_type(void)
{
  static GType vumeter_type = 0;

  if (!vumeter_type) {
    static const GTypeInfo vumeter_info = {
      sizeof(GstVuMeterClass),      NULL,
      NULL,
      (GClassInitFunc)gst_vumeter_class_init,
      NULL,
      NULL,
      sizeof(GstVuMeter),
      0,
      (GInstanceInitFunc)gst_vumeter_init,
    };
    vumeter_type = g_type_register_static(GST_TYPE_ELEMENT, "GstVuMeter", &vumeter_info, 0);
  }
  return vumeter_type;
}

static void
gst_vumeter_class_init (GstVuMeterClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)klass;

  parent_class = g_type_class_ref(GST_TYPE_ELEMENT);

  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_VOLUME,
    g_param_spec_int("volume","volume","volume",
                     G_MININT,G_MAXINT,0,G_PARAM_READABLE)); /* CHECKME */
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_VOLUMEPTR,
    g_param_spec_pointer("volumeptr","volumeptr","volumeptr",
                        G_PARAM_READABLE)); /* CHECKME */
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_VOLUME_LEFT,
    g_param_spec_int("volume_left","volume_left","volume_left",
                     G_MININT,G_MAXINT,0,G_PARAM_READABLE)); /* CHECKME */
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_VOLUMEPTR_LEFT,
    g_param_spec_pointer("volumeptr_left","volumeptr_left","volumeptr_left",
                        G_PARAM_READABLE)); /* CHECKME */
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_VOLUME_RIGHT,
    g_param_spec_int("volume_right","volume_right","volume_right",
                     G_MININT,G_MAXINT,0,G_PARAM_READABLE)); /* CHECKME */
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_VOLUMEPTR_RIGHT,
    g_param_spec_pointer("volumeptr_right","volumeptr_right","volumeptr_right",
                        G_PARAM_READABLE)); /* CHECKME */

  gobject_class->set_property = gst_vumeter_set_property;
  gobject_class->get_property = gst_vumeter_get_property;

  /*gstelement_class->change_state = gst_vumeter_change_state; */
}

static void
gst_vumeter_init(GstVuMeter *vumeter)
{
  vumeter->sinkpad = gst_pad_new_from_template (sink_template, "sink");
  gst_element_add_pad(GST_ELEMENT(vumeter), vumeter->sinkpad);
  gst_pad_set_chain_function(vumeter->sinkpad, gst_vumeter_chain);

  vumeter->volume = 0.0;
  vumeter->volume_left = 0.0;
  vumeter->volume_right = 0.0;
}

static void
gst_vumeter_chain(GstPad *pad,GstBuffer *buf)
{
  GstVuMeter *vumeter;
  gint16 *samples;
  gint samplecount,i;
  gint vl = 0, vr = 0;

  g_return_if_fail(pad != NULL);
  g_return_if_fail(GST_IS_PAD(pad));
  g_return_if_fail(buf != NULL);

  vumeter = GST_VUMETER (GST_OBJECT_PARENT (pad));
  g_return_if_fail(vumeter != NULL);
  g_return_if_fail(GST_IS_VUMETER(vumeter));

  /* FIXME: deal with audio metadata */

  /* This is very stereo s16_le specific!!! */
  samples = (gint16 *)GST_BUFFER_DATA(buf);
  samplecount = GST_BUFFER_SIZE(buf) / 4;

  /* FIXME: endianness issues here */
  for (i = 0; i < samplecount; i++) {
    vl = MAX(vl,abs(samples[2*i]));
    vr = MAX(vr,abs(samples[2*i+1]));
  }

  vumeter->volume = MAX(vl,vr);
  vumeter->volume_left = vl;
  vumeter->volume_right = vr;

/*  gst_trace_add_entry(NULL,0,buf,"vumeter: calculated volume"); */

  GST_DEBUG (0, "current volume is %d (l=%d,r=%d) ",
	     vumeter->volume, vumeter->volume_left, vumeter->volume_right);

  gst_buffer_unref(buf);
}

static void
gst_vumeter_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  GstVuMeter *vumeter;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail(GST_IS_VUMETER(object));
  vumeter = GST_VUMETER(object);

  switch (prop_id) {
    default:
      break;
  }
}

static void
gst_vumeter_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GstVuMeter *vumeter;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail(GST_IS_VUMETER(object));
  vumeter = GST_VUMETER(object);

  switch (prop_id) {
    case ARG_VOLUME:
      g_value_set_int (value, vumeter->volume);
      break;
    case ARG_VOLUMEPTR:
      g_value_set_pointer (value, &vumeter->volume);
      break;
    case ARG_VOLUME_LEFT:
      g_value_set_int (value, vumeter->volume_left);
      break;
    case ARG_VOLUMEPTR_LEFT:
      g_value_set_pointer (value, &vumeter->volume_left);
      break;
    case ARG_VOLUME_RIGHT:
      g_value_set_int (value, vumeter->volume_right);
      break;
    case ARG_VOLUMEPTR_RIGHT:
      g_value_set_pointer (value, &vumeter->volume_right);
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

  factory = gst_element_factory_new("vumeter", GST_TYPE_VUMETER,
				   &gst_vumeter_details);
  g_return_val_if_fail(factory != NULL, FALSE);

  sink_template = sink_factory ();
  gst_element_factory_add_pad_template (factory, sink_template);

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "vumeter",
  plugin_init
};


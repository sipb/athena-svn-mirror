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
#include "gstvolenv.h"

#define MSG_L_IDX 17		/* 0x00020000 PLUGIN_LOADING */
#define MSG_E_IDX 18            /* 0x00040000 PLUGIN_ERRORS */

static GstElementDetails volenv_details = {
  "Volume Envelope",
  "Filter/Audio/Effect",
  "LGPL",
  "Volume envelope filter for audio/raw",
  VERSION,
  "Thomas <thomas@apestaart.org>",
  "(C) 2001",
};


/* Filter signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_RISE,
  ARG_LEVEL,
  ARG_CONTROLPOINT
};

static GstPadTemplate*
volenv_src_template_factory (void)
{
  static GstPadTemplate *template = NULL;

  if (!template) {
    template = gst_pad_template_new (
      "src",
      GST_PAD_SRC,
      GST_PAD_ALWAYS,
      gst_caps_new (
       "test_src",
       "audio/raw",
       gst_props_new (
         "channels", GST_PROPS_INT_RANGE (1, 2),
	 NULL)),
      NULL);
  }
  return template;
}

static GstPadTemplate*
volenv_sink_template_factory (void)
{
  static GstPadTemplate *template = NULL;

  if (!template) {
    template = gst_pad_template_new (
      "sink",
      GST_PAD_SINK,
      GST_PAD_ALWAYS,
      gst_caps_new (
        "test_sink",
        "audio/raw",
	gst_props_new (
          "channels",    GST_PROPS_INT_RANGE (1, 2),
	  NULL)),
      NULL);
  }
  return template;
}

static void		gst_volenv_class_init		(GstVolEnvClass *klass);
static void		gst_volenv_init			(GstVolEnv *filter);

static void		gst_volenv_set_property		(GObject *object, guint prop_id, 
							 const GValue *value, GParamSpec *pspec);
static void		gst_volenv_get_property		(GObject *object, guint prop_id, 
							 GValue *value, GParamSpec *pspec);

static void		gst_volenv_chain		(GstPad *pad, GstBuffer *buf);
static void inline 	gst_volenv_fast_16bit_chain 	(gint16* data, gint16** out_data, 
			         			 guint numsamples, GstVolEnv* filter);
static void inline 	gst_volenv_fast_8bit_chain	(gint8* data, gint8** out_data,
                                			 guint numsamples, GstVolEnv* filter);

void print_volume_envelope (GstVolEnv* filter);

static GstElementClass *parent_class = NULL;
/*static guint gst_filter_signals[LAST_SIGNAL] = { 0 }; */

static GstPadLinkReturn
volenv_connect (GstPad *pad, GstCaps *caps)
{
  GstVolEnv *filter;
  GstPad *otherpad;
  
  filter = GST_VOLENV (gst_pad_get_parent (pad));
  otherpad = (pad == filter->srcpad ? filter->sinkpad : filter->srcpad);
  
  if (GST_CAPS_IS_FIXED (caps))
    return gst_pad_try_set_caps (otherpad, caps);
  return GST_PAD_LINK_DELAYED;
}

GType
gst_volenv_get_type(void) {
  static GType volenv_type = 0;

  if (!volenv_type) {
    static const GTypeInfo volenv_info = {
      sizeof(GstVolEnvClass),      NULL,
      NULL,
      (GClassInitFunc)gst_volenv_class_init,
      NULL,
      NULL,
      sizeof(GstVolEnv),
      0,
      (GInstanceInitFunc)gst_volenv_init,
    };
    volenv_type = g_type_register_static(GST_TYPE_ELEMENT, "GstVolEnv", &volenv_info, 0);
  }
  return volenv_type;
}

static void
gst_volenv_class_init (GstVolEnvClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)klass;

  parent_class = g_type_class_ref(GST_TYPE_ELEMENT);
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_RISE,
    g_param_spec_double("rise","rise","rise",
                        -G_MAXDOUBLE,G_MAXDOUBLE,0,G_PARAM_READWRITE)); /* CHECKME */
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_LEVEL,
    g_param_spec_double("level","level","level",
                        -G_MAXDOUBLE,G_MAXDOUBLE,0,G_PARAM_READWRITE)); /* CHECKME */
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_CONTROLPOINT,
    g_param_spec_string("controlpoint","controlpoint","controlpoint",
                        NULL, G_PARAM_READWRITE)); /* CHECKME */

  gobject_class->set_property = gst_volenv_set_property;
  gobject_class->get_property = gst_volenv_get_property;

}

static void
gst_volenv_init (GstVolEnv *filter)
{
  filter->sinkpad = gst_pad_new_from_template(volenv_sink_template_factory(),"sink");
  filter->srcpad = gst_pad_new_from_template(volenv_src_template_factory(),"src");

  filter->run_time = 0.0;
  filter->level = 1.0;
  filter->increase = 0.0;
  filter->rise = 0.0;
  filter->envelope = NULL;
  filter->next_cp = filter->envelope;
  filter->envelope_active = FALSE;

  gst_element_add_pad(GST_ELEMENT(filter),filter->sinkpad);
  gst_pad_set_chain_function(filter->sinkpad,gst_volenv_chain);
  gst_pad_set_link_function(filter->sinkpad, volenv_connect);
  gst_element_add_pad(GST_ELEMENT(filter),filter->srcpad);
  gst_pad_set_link_function(filter->srcpad, volenv_connect);
}

static void
gst_volenv_chain (GstPad *pad, GstBuffer *buf)
{
  GstVolEnv *filter;
  gint16 *in_data;
  gint16 *out_data;
  GstBuffer* outbuf;
  gint width;
  gint num_samples;
  gint sample_rate;

  GstCaps *caps;

  g_return_if_fail(pad != NULL);
  g_return_if_fail(GST_IS_PAD(pad));
  g_return_if_fail(buf != NULL);

  filter = GST_VOLENV(GST_OBJECT_PARENT (pad));
  g_return_if_fail(filter != NULL);
  g_return_if_fail(GST_IS_VOLENV(filter));

/*  printf ("DEBUG : time %f, level %f, increase %e\n", filter->run_time, filter->level, filter->increase); */

  caps = NULL;
  caps = GST_PAD_CAPS(pad);
  if (caps == NULL) {
    gst_element_error (GST_ELEMENT (filter), "capsnego was never performed, bailing...\n");
    gst_buffer_unref (buf);
    return;
  }

  gst_caps_get_int(caps, "width", &width);
  gst_caps_get_int(caps, "rate", &sample_rate);

  in_data = (gint16 *)GST_BUFFER_DATA(buf);
  outbuf=gst_buffer_new();
  GST_BUFFER_DATA(outbuf) = (gchar*)g_new(gint16,GST_BUFFER_SIZE(buf)/2);
  GST_BUFFER_SIZE(outbuf) = GST_BUFFER_SIZE(buf);

  out_data = (gint16*)GST_BUFFER_DATA(outbuf);
  
  num_samples = GST_BUFFER_SIZE(buf)/2;

  switch (width) {
    case 16:
	gst_volenv_fast_16bit_chain(in_data, &out_data, 
            num_samples, filter);
	break;
    case 8:
	num_samples *= 2;
	gst_volenv_fast_8bit_chain((gint8*)in_data,(gint8**)&out_data,
            num_samples, filter);
	break;
  }
  filter->run_time += (double) num_samples / (2 * sample_rate);
  gst_buffer_unref(buf);
  gst_pad_push(filter->srcpad,outbuf);
}

static void inline
gst_volenv_fast_16bit_chain(gint16* in_data, gint16** out_data, 
			         guint num_samples, GstVolEnv *filter)
#include "filter.func"

static void inline
gst_volenv_fast_8bit_chain(gint8* in_data, gint8** out_data,
                                guint num_samples, GstVolEnv *filter)
#include "filter.func"

static void
gst_volenv_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  gint rate = 0;
  GstVolEnv *filter;
  GstCaps *caps;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail(GST_IS_VOLENV(object));
  filter = GST_VOLENV(object);

  if (filter->srcpad == NULL)
  {
    printf ("WARNING : set_property : filter does not have srcpad !\n");
  }

  switch (prop_id) {
    case ARG_RISE:
	/* set the increase to match (rise) per sec */
        /* can only be set after the rate is known ! */

        caps = GST_PAD_CAPS(filter->srcpad);
        if (caps == NULL)
        {
          /* FIXME : Please change this to a better warning method ! */
          printf ("WARNING : set_property : Could not get caps of pad !\n");
        }
        else 
        {
          gst_caps_get_int (caps, "rate", &rate);
        }
    filter->arg_rise = g_value_get_double (value);
    filter->increase = filter->arg_rise / (double) rate;
    GST_DEBUG (MSG_E_IDX, "filter increase set to %e",
                 filter->increase);

    break;
    case ARG_LEVEL:
	/* set the level */
    filter->level = g_value_get_double (value);
    break;
    case ARG_CONTROLPOINT:
	/* register control point */
        {
          double cp_time, cp_level;
	  double *pcp_time, *pcp_level;

	  pcp_time = (double *) g_malloc (sizeof (double));
	  pcp_level = (double *) g_malloc (sizeof (double));
	  
	  filter->control_point = g_value_get_string (value);

	  sscanf (filter->control_point, "%lf:%lf", &cp_time, &cp_level);
          GST_DEBUG (MSG_L_IDX, "volenv : added cp : level %f at time %f",
		cp_level, cp_time);
	  *pcp_time = cp_time;
	  *pcp_level = cp_level;

	  if (filter->envelope == NULL)
          {
	    /* not created yet, so set envelope_active */
	    filter->envelope_active = TRUE;
	    filter->level = cp_level;
	  }

	  filter->envelope = g_list_append(filter->envelope, pcp_time);
	  filter->envelope = g_list_append(filter->envelope, pcp_level);

	  /* if this is the second control point, we can set the (first)
	     next vars */

          if (g_list_length (filter->envelope) == 4)
          {
	    double *pst, *psl;	/* pointers to starting control point */
	    
	    pst = filter->envelope->data;
	    psl = filter->envelope->next->data;

	    filter->next_time = cp_time;
	    filter->next_level = cp_level;
	    filter->next_cp = filter->envelope->next->next;

            /* Calculate rise */
	    if (cp_level == *psl)
            {
		/* if the levels are the same, then no rise necessary */
		/* CHECKME : is this allowed in floating point ? */
		filter->rise = 0.0;
            }
	    else
	    {
	    	filter->rise = (cp_level - *psl) / (cp_time - *pst);
	    }

            GST_DEBUG (MSG_E_IDX, "second cp registered at %f level %f",
		cp_time, cp_level);
            GST_DEBUG (MSG_E_IDX, "first cp registered at %f level %f",
		*pst, *psl);
            GST_DEBUG (MSG_E_IDX, "second cp registered with rise %e",
		filter->rise);
	    /* new caps callback takes care of converting rise to increase */
	  }
        }
    break;
    default:
      break;
  }
}

static void
gst_volenv_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GstVolEnv *filter;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail(GST_IS_VOLENV(object));
  filter = GST_VOLENV(object);

  switch (prop_id) {
    case ARG_RISE:
      g_value_set_double (value, filter->arg_rise);
      break;
    case ARG_LEVEL:
      g_value_set_double (value, filter->level);
      break;
    case ARG_CONTROLPOINT:
      g_value_set_string (value, filter->control_point);
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

  factory = gst_element_factory_new("volenv",GST_TYPE_VOLENV,
                                   &volenv_details);
  g_return_val_if_fail(factory != NULL, FALSE);
  
  gst_element_factory_add_pad_template (factory, volenv_src_template_factory ());
  gst_element_factory_add_pad_template (factory, volenv_sink_template_factory ());

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "volenv",
  plugin_init
};

void 
print_volume_envelope (GstVolEnv* filter)
{
  /* print volume envelope
   * used for debugging
   */

    int i;
    double *ptime, *plevel;

    printf ("print_volume_envelope :\n");
    for (i = 0; i < g_list_length (filter->envelope); i += 2)
    {
      ptime = (double *) g_list_nth_data (filter->envelope, i);
      plevel = (double *) g_list_nth_data (filter->envelope, i + 1);
      printf ("\ttime : %f, level : %f\n", *ptime, *plevel);
    }
}


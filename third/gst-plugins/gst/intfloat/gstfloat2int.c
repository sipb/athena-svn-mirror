/* GStreamer
 * Copyright (C) <2001> Steve Baker <stevebaker_org@yahoo.co.uk>
 *
 * float2int.c
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

#include <gst/floatcast/floatcast.h>

#include <gstfloat2int.h>

/* elementfactory information */
static GstElementDetails float2int_details = {
  "Float to Integer effect",
  "Filter/Audio/Conversion",
  "LGPL",
  "Convert from floating point to integer audio data",
  VERSION,
  "Steve Baker <stevebaker_org@yahoo.co.uk>",
  "(C) 2001",
};


enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0,
};

GST_PAD_TEMPLATE_FACTORY (float2int_sink_factory,
  "sink%d",
  GST_PAD_SINK,
  GST_PAD_REQUEST,
  GST_CAPS_NEW (
    "float2int_sink",
    "audio/raw",
    "rate",       GST_PROPS_INT_RANGE (1, G_MAXINT),
    "format",     GST_PROPS_STRING ("float"),
    "layout",     GST_PROPS_STRING ("gfloat"),
    "intercept",  GST_PROPS_FLOAT(0.0),
    "slope",      GST_PROPS_FLOAT(1.0),
    "channels",   GST_PROPS_INT (1)
  )
);

GST_PAD_TEMPLATE_FACTORY (float2int_src_factory,
  "src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
    "float2int_src",
    "audio/raw",
    "format",     GST_PROPS_STRING ("int"),
    "channels",   GST_PROPS_INT_RANGE (1, G_MAXINT),
    "rate",       GST_PROPS_INT_RANGE (1, G_MAXINT),
    "law",        GST_PROPS_INT (0),
    "endianness", GST_PROPS_INT (G_BYTE_ORDER),
    "width",      GST_PROPS_INT (16),
    "depth",      GST_PROPS_INT (16),
    "signed",     GST_PROPS_BOOLEAN (TRUE)
  )
);

static GstPadTemplate *srctempl, *sinktempl;

static void	gst_float2int_class_init		(GstFloat2IntClass *klass);
static void	gst_float2int_init			(GstFloat2Int *plugin);

static GstPadLinkReturn gst_float2int_connect	(GstPad *pad, GstCaps *caps);

static GstPad*  gst_float2int_request_new_pad		(GstElement *element, GstPadTemplate *temp,
                                                         const gchar *unused);
static void     gst_float2int_pad_removed               (GstElement *element, GstPad *pad);

static void	gst_float2int_loop			(GstElement *element);

static GstElementStateReturn gst_float2int_change_state (GstElement *element);

static GstElementClass *parent_class = NULL;

GType
gst_float2int_get_type(void) {
  static GType float2int_type = 0;

  if (!float2int_type) {
    static const GTypeInfo float2int_info = {
      sizeof(GstFloat2IntClass),      NULL,
      NULL,
      (GClassInitFunc)gst_float2int_class_init,
      NULL,
      NULL,
      sizeof(GstFloat2Int),
      0,
      (GInstanceInitFunc)gst_float2int_init,
    };
    float2int_type = g_type_register_static(GST_TYPE_ELEMENT, "GstFloat2Int", &float2int_info, 0);
  }
  return float2int_type;
}

static void
gst_float2int_class_init (GstFloat2IntClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)klass;

  parent_class = g_type_class_ref(GST_TYPE_ELEMENT);

  gstelement_class->request_new_pad = gst_float2int_request_new_pad;
  gstelement_class->pad_removed = gst_float2int_pad_removed;
  gstelement_class->change_state = gst_float2int_change_state;
}

static void
gst_float2int_init (GstFloat2Int *plugin)
{
  gst_element_set_loop_function (GST_ELEMENT (plugin), gst_float2int_loop);

  plugin->srcpad = gst_pad_new_from_template (srctempl, "src");
  gst_element_add_pad (GST_ELEMENT (plugin), plugin->srcpad);
  gst_pad_set_link_function (plugin->srcpad, gst_float2int_connect);

  plugin->numchannels = 0;
  plugin->channelcount = 0;
  plugin->channels = NULL;
}

static GstBufferPool*
gst_float2int_get_bufferpool (GstPad *pad)
{
  static GstBufferPool *bp = NULL;
  GstBuffer *buffer;
  GstFloat2Int *filter = (GstFloat2Int *) gst_pad_get_parent (pad);

  if (!filter->pool)
    if (! (filter->pool = gst_pad_get_bufferpool (filter->srcpad)))
      filter->pool = gst_buffer_pool_get_default (sizeof(gint16) * 1024 * filter->numchannels, 5);

  if (!bp) {
    /* create a new buffer pool with the same number of frames as the source
       pad's buffer pool */
    if (!(buffer = gst_buffer_new_from_pool (filter->pool, 0, 0)))
      return NULL;
    
    bp = gst_buffer_pool_get_default (GST_BUFFER_SIZE (buffer) / sizeof (gint16) / filter->numchannels * sizeof (gfloat), 8);

    gst_buffer_unref (buffer);
  }
  
  return bp;
}

/* this is a very brittle function. it's hard to come up with a "correct
   behavior". what I (wingo) have chosen is as follows:
   * if the rate of the float2int plugin has already been set by a previous connection
     * if the new rate is different from the old rate, bail
     * otherwise return 'ok' (don't set other caps, no need -- only the rate needs
       setting, and it's the same)
   * otherwise set all of the caps (except for on the connecting pad, of course)
     * bail if that fails, possibly leaving the plugin in an inconsistent state

   FIXME: we really should do something about the inconsistent state case,
   possibly unlinking the offending pad. the assumption with this element,
   though, is that the pipelines on the float side will be parallel (more or
   less the same) and so we should never run into that case. that might be a
   programmer error more than anything.
 */
static GstPadLinkReturn
gst_float2int_connect (GstPad *pad, GstCaps *caps)
{
  GstFloat2Int *filter;
  GstCaps *intcaps, *floatcaps;
  GSList *l;
  gint rate, channels;
  
  filter = GST_FLOAT2INT (GST_PAD_PARENT (pad));
  
  /* we have two variables to play with: "rate" and "channels" on the int side. */
  if (GST_CAPS_IS_FIXED (caps)) {
    if (pad == filter->srcpad) {
      /* it's the int pad */
      if (!filter->intcaps) {
        gst_caps_get_int (caps, "channels", &channels);

        if (channels != filter->numchannels) {
          GST_DEBUG (0, "Tried to set caps with %d channels but there are %d source pads",
                     channels, filter->numchannels);
          return GST_PAD_LINK_REFUSED;
        }
        
        floatcaps = gst_caps_copy (gst_pad_template_get_caps (float2int_sink_factory ()));
        /* FIXME: we really shouldn't have to convert to int and back */
        gst_caps_get_int (caps, "rate", &filter->rate);
        /* g_print("float2int got caps with rate %d\n", rate); */
        gst_caps_set (floatcaps, "rate", GST_PROPS_INT (filter->rate), NULL);
        /* we now know that the caps are fixed. let's set them. this is a hack
           but oh well. */
        floatcaps->fixed = TRUE;
        
        for (l=filter->channels; l; l=l->next)
          if (gst_pad_try_set_caps (GST_FLOAT2INT_CHANNEL (l)->sinkpad, floatcaps) <= 0)
            return GST_PAD_LINK_REFUSED;
        
        /* FIXME: refcounting? */
        filter->floatcaps = floatcaps;
        filter->intcaps = caps;
        return GST_PAD_LINK_OK;
      } else if (gst_caps_intersect (caps, filter->intcaps)) {
        /* they have to be the same since both are fixed */
        return GST_PAD_LINK_OK;
      } else {
        return GST_PAD_LINK_REFUSED;
      }
    } else {
      /* it's a float pad */
      if (!filter->intcaps) {

        /* FIXME: we really shouldn't have to convert to int and back */
        intcaps = gst_caps_copy (gst_pad_template_get_caps (float2int_src_factory ()));
        gst_caps_get_int (caps, "rate", &rate);
        gst_caps_set (intcaps, "rate", GST_PROPS_INT (rate));
        gst_caps_set (intcaps, "channels", GST_PROPS_INT (filter->numchannels));
        intcaps->fixed = TRUE;
        gst_caps_debug (intcaps, "int source pad caps going into try_set_caps()");
        
        if (gst_pad_try_set_caps (filter->srcpad, intcaps) <= 0) {
          return GST_PAD_LINK_REFUSED;
	}

	/* FIXME: refcounting? */
        filter->floatcaps = caps;
        filter->intcaps = intcaps;
        filter->rate = rate;

        for (l=filter->channels; l; l=l->next) {
	  if (GST_FLOAT2INT_CHANNEL (l)->sinkpad == pad) {
	    continue;
	  }
	  
          if (gst_pad_try_set_caps (GST_FLOAT2INT_CHANNEL (l)->sinkpad, caps) == FALSE) {
	    /* We don't have the caps set, so remove them */
	    filter->floatcaps = NULL;
	    filter->intcaps = NULL;
            return GST_PAD_LINK_REFUSED;
	  }
	}
        	
        return GST_PAD_LINK_OK;
      } else if (gst_caps_intersect (caps, filter->floatcaps)) {
        /* they have to be the same since both are fixed */
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
gst_float2int_request_new_pad (GstElement *element, GstPadTemplate *templ, const gchar *unused) 
{
  gchar *name;
  GstFloat2Int *plugin;
  GstFloat2IntInputChannel *channel;

  plugin = GST_FLOAT2INT(element);
  
  g_return_val_if_fail(plugin != NULL, NULL);
  g_return_val_if_fail(GST_IS_FLOAT2INT(plugin), NULL);

  if (templ->direction != GST_PAD_SINK) {
    g_warning ("float2int: request new pad that is not a SINK pad\n");
    return NULL;
  }

  channel = g_new0 (GstFloat2IntInputChannel, 1);
  name = g_strdup_printf ("sink%d", plugin->channelcount);
  channel->sinkpad = gst_pad_new_from_template (templ, name);
  gst_element_add_pad (GST_ELEMENT (plugin), channel->sinkpad);
  gst_pad_set_link_function (channel->sinkpad, gst_float2int_connect);
  gst_pad_set_bufferpool_function (channel->sinkpad, gst_float2int_get_bufferpool);
  channel->bytestream = gst_bytestream_new (channel->sinkpad);
  
  plugin->channels = g_slist_append (plugin->channels, channel);
  plugin->numchannels++;
  plugin->channelcount++;
  
  GST_DEBUG (0, "float2int added pad %s\n", name);

  g_free (name);
  return channel->sinkpad;
}

static void
gst_float2int_pad_removed (GstElement *element,
			   GstPad *pad)
{
  GstFloat2Int *plugin;
  GSList *p;
  
  GST_DEBUG(0, "float2int removed pad %s\n", GST_OBJECT_NAME (pad));
  
  plugin = GST_FLOAT2INT (element);

  /* Find our channel for this pad */
  for (p = plugin->channels; p;) {
    GstFloat2IntInputChannel *channel = p->data;
    GSList *p_copy;

    if (channel->sinkpad == pad) {
      p_copy = p;

      p = p->next;
      plugin->channels = g_slist_remove_link (plugin->channels, p_copy);
      plugin->numchannels--;
      
      g_slist_free (p_copy);

      gst_bytestream_destroy (channel->bytestream);
      g_free (channel);
    } else {
      p = p->next;
    }
  }
}

static gboolean
all_channels_eos (GSList *inputs)
{
  for (; inputs; inputs = inputs->next) {
    GstFloat2IntInputChannel *c = inputs->data;

    if (c->eos == FALSE) {
      return FALSE;
    }
  }

  return TRUE;
}

static void
gst_float2int_loop (GstElement *element)
{
  GstFloat2Int *plugin = (GstFloat2Int*)element;
  GstBuffer *buf_out;
  gfloat *data_in;
  gint16 *data_out;
  gint num_frames, i, j;
  GSList *l;
  GstFloat2IntInputChannel *channel;
  guint32 got_bytes, waiting;
  GstEvent *event = NULL;
    
  if (!plugin->channels) {
    gst_element_error (element, "float2int: at least one sink pad needs to be connected");
    return;
  }

  if (!plugin->pool)
    if (! (plugin->pool = gst_pad_get_bufferpool (plugin->srcpad)))
      plugin->pool = gst_buffer_pool_get_default (sizeof(gint16) * 1024 * plugin->numchannels, 5);

  g_assert (plugin->pool);

  do {
    /* get new output buffer */
    buf_out = gst_buffer_new_from_pool (plugin->pool, 0, 0);
    num_frames = GST_BUFFER_SIZE (buf_out) / plugin->numchannels / sizeof (gint16);
    
    data_out = (gint16*)GST_BUFFER_DATA(buf_out);

    /* FIXME: assert that buf_out is cleanly divisable by numchannels */

    l = plugin->channels;
    i = 0;
    while (l) {
      channel = GST_FLOAT2INT_CHANNEL (l);

      if (channel->eos) {
	l = l->next;
	continue;
      }

      got_bytes = gst_bytestream_peek_bytes (channel->bytestream, (guint8**)&data_in, num_frames * sizeof (gfloat));

      /* FIXME we should do something with the data if got_bytes is more than zero */
      if (got_bytes < num_frames * sizeof (gfloat)) {
        /* we need to check for an event. */
        gst_bytestream_get_status (channel->bytestream, &waiting, &event);

        if (event) {
          switch (GST_EVENT_TYPE(event)) {
          case GST_EVENT_EOS:
            /* if we get an EOS event from one of our sink pads, we assume that
               pad's finished handling data. Let's just send an EOS. */
            GST_DEBUG (0, "got an EOS event");

	    if (all_channels_eos (plugin->channels)) {
              gst_element_set_eos (element);
	    } else {
	      channel->eos = TRUE;
	    }
	    
            gst_pad_push (plugin->srcpad, GST_BUFFER (event));

            gst_pad_event_default (plugin->srcpad, event);
            break;
          default:
            gst_pad_event_default (plugin->srcpad, event);
            break;
          }
        }
      } else {
        if (!plugin->rate) {
          gst_element_error (element, "caps were never set, bailing...");
          return;
        }

        for (j = 0; j < num_frames; j++) {
          data_out[(j*plugin->numchannels) + i] = (gint16)(gst_cast_float(data_in[j] * 32767.0F));
        }
        
        gst_bytestream_flush (channel->bytestream, got_bytes);

        GST_DEBUG (0, "done copying data");
      }
      
      i++;
      l = g_slist_next (l);
    }

    GST_BUFFER_TIMESTAMP(buf_out) = plugin->offset * GST_SECOND / plugin->rate;

    plugin->offset += num_frames;
    gst_pad_push(plugin->srcpad,buf_out);

    gst_element_yield (element);
  } while (TRUE);
}

static GstElementStateReturn
gst_float2int_change_state (GstElement *element)
{
  GstFloat2Int *plugin = (GstFloat2Int *) element;
  GSList *p;

  switch (GST_STATE_TRANSITION (element)) {
    case GST_STATE_READY_TO_PAUSED:
      plugin->offset = 0;
      break;

    case GST_STATE_PAUSED_TO_PLAYING:
      for (p = plugin->channels; p; p = p->next) {
	GstFloat2IntInputChannel *c = p->data;

	c->eos = FALSE;
      }
      break;
      
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
gst_float2int_factory_init (GstPlugin *plugin) 
{
  GstElementFactory *factory;
  
  if (! gst_library_load ("gstbytestream"))
    return FALSE;

  factory = gst_element_factory_new ("float2int", GST_TYPE_FLOAT2INT,
                                     &float2int_details);
  g_return_val_if_fail(factory != NULL, FALSE);
  gst_element_factory_set_rank (factory, GST_ELEMENT_RANK_SECONDARY);
  
  sinktempl = float2int_sink_factory();
  gst_element_factory_add_pad_template (factory, sinktempl);

  srctempl = float2int_src_factory();
  gst_element_factory_add_pad_template (factory, srctempl);
  
  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));
   
  return TRUE;
}

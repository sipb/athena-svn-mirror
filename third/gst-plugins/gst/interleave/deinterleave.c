/* GStreamer deinterleave plugin
 * Copyright (C) 2004 Andy Wingo <wingo at pobox.com>
 * Copyright (C) 2002 Iain Holmes
 *
 * Based on oneton: Iain Holmes <iain@prettypeople.org>
 * Based on stereosplit: Richard Boulton <richard@tartarus.org>
 * Based on stereo2mono: Zaheer Abbas Merali <zaheerabbas at merali dot org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <string.h>
#include <gst/gst.h>
#include "plugin.h"

#define GSTPLUGIN_TYPE_DEINTERLEAVE (gstplugin_deinterleave_get_type ())
#define DEINTERLEAVE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSTPLUGIN_TYPE_DEINTERLEAVE, Deinterleave))

#if 0
#define DEBUG {g_print ("In function %s\n", __FUNCTION__);}
#else
#define DEBUG
#endif

typedef struct _Deinterleave Deinterleave;
typedef struct _DeinterleaveClass DeinterleaveClass;

struct _Deinterleave
{
  GstElement element;

  GstPad *sinkpad;

  int channels;
  gboolean is_int;

  GstBuffer **out_buffers;
  gpointer *out_data;

  GList *srcpads;
};

struct _DeinterleaveClass
{
  GstElementClass parent_class;
};

static GstElementDetails details = {
  "Audio deinterleaver",
  "Filter/Converter/Audio",
  "Splits one interleaved multichannel audio stream into many mono audio streams",
  "Andy Wingo <wingo at pobox.com>, " "Iain <iain@prettypeople.org>"
};

static GstStaticPadTemplate deinterleave_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-raw-int, "
        "endianness = (int) BYTE_ORDER, "
        "signed = (boolean) TRUE, "
        "width = (int) 16, "
        "depth = (int) 16, "
        "rate = (int) [ 1, MAX ], "
        "channels = (int) [ 1, MAX ];"
        "audio/x-raw-float, "
        "rate = (int) [ 1, MAX ], "
        "channels = (int) [ 1, MAX ], "
        "endianness = (int) BYTE_ORDER, "
        "width = (int) 32, " "buffer-frames = (int) [ 0, MAX ]")
    );

static GstStaticPadTemplate deinterleave_src_template =
    GST_STATIC_PAD_TEMPLATE ("src_%d",
    GST_PAD_SRC,
    GST_PAD_SOMETIMES,
    GST_STATIC_CAPS ("audio/x-raw-int, "
        "endianness = (int) BYTE_ORDER, "
        "signed = (boolean) TRUE, "
        "width = (int) 16, "
        "depth = (int) 16, "
        "rate = (int) [ 4000, 96000 ], "
        "channels = (int) 1;"
        "audio/x-raw-float, "
        "rate = (int) [ 1, MAX ], "
        "channels = (int) 1, "
        "endianness = (int) BYTE_ORDER, "
        "width = (int) 32, " "buffer-frames = (int) [ 0, MAX ]")
    );

static void deinterleave_class_init (DeinterleaveClass * klass);
static void deinterleave_base_init (DeinterleaveClass * klass);
static void deinterleave_init (Deinterleave * deinterleave);
static GstCaps *deinterleave_sink_getcaps (GstPad * pad);
static GstCaps *deinterleave_src_getcaps (GstPad * pad);
static GstPadLinkReturn deinterleave_sink_link (GstPad * pad,
    const GstCaps * caps);
static GstElementStateReturn deinterleave_change_state (GstElement * element);
static void deinterleave_chain (GstPad * pad, GstData * _data);
static void inline do_int_deinterleave (gint16 * in_data, int channels,
    gint16 ** out_data, guint numframes);
static void inline do_float_deinterleave (gfloat * in_data, int channels,
    gfloat ** out_data, guint numframes);

static GstElementClass *parent_class = NULL;

GType
gstplugin_deinterleave_get_type (void)
{
  static GType type = 0;

  if (type == 0) {
    static const GTypeInfo info = {
      sizeof (DeinterleaveClass),
      (GBaseInitFunc) deinterleave_base_init, NULL,
      (GClassInitFunc) deinterleave_class_init, NULL, NULL,
      sizeof (Deinterleave), 0, (GInstanceInitFunc) deinterleave_init,
    };

    type = g_type_register_static (GST_TYPE_ELEMENT,
        "GstDeinterleave", &info, 0);
  }
  return type;
}

static void
deinterleave_base_init (DeinterleaveClass * klass)
{
  GstElementClass *eclass = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template
      (eclass, gst_static_pad_template_get (&deinterleave_src_template));
  gst_element_class_add_pad_template
      (eclass, gst_static_pad_template_get (&deinterleave_sink_template));
  gst_element_class_set_details (eclass, &details);
}

static void
deinterleave_class_init (DeinterleaveClass * klass)
{
  GstElementClass *eclass = (GstElementClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  eclass->change_state = deinterleave_change_state;
}

static void
deinterleave_init (Deinterleave * this)
{
  DEBUG;
  this->sinkpad = gst_pad_new_from_template
      (gst_static_pad_template_get (&deinterleave_sink_template), "sink");
  gst_pad_set_chain_function (this->sinkpad, deinterleave_chain);
  gst_pad_set_link_function (this->sinkpad, deinterleave_sink_link);
  gst_pad_set_getcaps_function (this->sinkpad, deinterleave_sink_getcaps);
  gst_element_add_pad (GST_ELEMENT (this), this->sinkpad);

  this->channels = 0;
  this->is_int = TRUE;
  this->srcpads = NULL;
  this->out_buffers = NULL;
  this->out_data = NULL;
}

static void
deinterleave_alloc_channels_data (Deinterleave * this)
{
  if (!this->out_buffers && this->channels) {
    this->out_buffers = g_new0 (GstBuffer *, this->channels);
    this->out_data = g_new0 (gpointer, this->channels);
  }
}

static void
deinterleave_free_channels_data (Deinterleave * this)
{
  g_free (this->out_buffers);
  g_free (this->out_data);
  this->out_buffers = NULL;
  this->out_data = NULL;
}

static GstCaps *
deinterleave_sink_getcaps (GstPad * pad)
{
  Deinterleave *this;
  GList *l;
  GstCaps *ret;

  this = DEINTERLEAVE (GST_OBJECT_PARENT (pad));

  ret = gst_caps_copy (gst_pad_get_pad_template_caps (this->sinkpad));
  for (l = this->srcpads; l; l = l->next) {
    GstCaps *old, *allowed;

    allowed = gst_pad_get_allowed_caps (GST_PAD (l->data));
    old = ret;
    ret = gst_caps_intersect (old, allowed);
    gst_caps_free (old);
    gst_caps_free (allowed);
    if (!ret)
      return gst_caps_new_empty ();
  }

  gst_structure_set (gst_caps_get_structure (ret, 0),
      "channels", GST_TYPE_INT_RANGE, 1, G_MAXINT, NULL);
  return ret;
}

static GstCaps *
deinterleave_src_getcaps (GstPad * pad)
{
  Deinterleave *this;
  GstCaps *ret;

  this = DEINTERLEAVE (GST_OBJECT_PARENT (pad));

  ret = gst_pad_get_allowed_caps (this->sinkpad);
  gst_structure_set (gst_caps_get_structure (ret, 0),
      "channels", G_TYPE_INT, 1, NULL);
  return ret;
}

static GstPadLinkReturn
deinterleave_sink_link (GstPad * pad, const GstCaps * caps)
{
  Deinterleave *this;
  int i, new_chans, need;
  GstCaps *srccaps;
  GList *p;
  GstStructure *structure;
  GstPadLinkReturn srclink;

  this = DEINTERLEAVE (gst_pad_get_parent (pad));

  DEBUG;

  structure = gst_caps_get_structure (caps, 0);

  /* Get the number of channels coming in */
  gst_structure_get_int (structure, "channels", &new_chans);
  this->is_int = (strcmp (gst_structure_get_name (structure),
          "audio/x-raw-int") == 0);

  if (new_chans != this->channels) {
    need = new_chans - this->channels;

    if (need < 0) {
      for (p = g_list_last (this->srcpads); p;) {
        GstPad *peer, *pad = p->data;
        GList *old;

        /* Check if they're connected */
        peer = GST_PAD_PEER (pad);
        if (peer)
          gst_pad_unlink (pad, peer);

        gst_element_remove_pad (GST_ELEMENT (this), pad);

        old = p;
        p = p->prev;
        g_list_free_1 (old);
        /* Remove link to the next */
        if (p != NULL)
          p->next = NULL;
      }
    } else {
      /* Create that number of src pads */
      for (i = this->channels; i < new_chans; i++) {
        GstPad *pad;
        char *pad_name;

        pad_name = g_strdup_printf ("src_%d", i);
        pad = gst_pad_new_from_template
            (gst_static_pad_template_get (&deinterleave_src_template),
            pad_name);
        g_free (pad_name);

        gst_pad_set_getcaps_function (pad, deinterleave_src_getcaps);
        gst_element_add_pad (GST_ELEMENT (this), pad);

        this->srcpads = g_list_append (this->srcpads, pad);
      }
    }

    this->channels = new_chans;
    deinterleave_free_channels_data (this);
    if (GST_STATE (this) == GST_STATE_PLAYING)
      deinterleave_alloc_channels_data (this);
    /* otherwise alloc in the state-change */
  }

  srccaps = gst_caps_copy (caps);
  gst_caps_set_simple (srccaps, "channels", G_TYPE_INT, 1, NULL);
  for (p = this->srcpads; p; p = p->next) {
    srclink = gst_pad_try_set_caps (GST_PAD (p->data), srccaps);
    if (GST_PAD_LINK_FAILED (srclink)) {
      gst_caps_free (srccaps);
      return srclink;
    }
  }

  gst_caps_free (srccaps);

  return GST_PAD_LINK_OK;
}

static GstElementStateReturn
deinterleave_change_state (GstElement * element)
{
  Deinterleave *this = DEINTERLEAVE (element);

  switch (GST_STATE_TRANSITION (element)) {
    case GST_STATE_PAUSED_TO_PLAYING:
      deinterleave_alloc_channels_data (this);
      break;
    case GST_STATE_READY_TO_NULL:
      deinterleave_free_channels_data (this);
      break;
    default:
      break;
  }

  if (parent_class->change_state)
    return parent_class->change_state (element);
  return GST_STATE_SUCCESS;
}

static void
deinterleave_chain (GstPad * pad, GstData * _data)
{
  GstBuffer *buf = GST_BUFFER (_data);
  Deinterleave *this;
  gpointer in_data;
  gpointer *out_data;
  GstBuffer **out_bufs;
  GList *p;
  int i;

  DEBUG;

  this = DEINTERLEAVE (gst_pad_get_parent (pad));
  out_data = this->out_data;
  out_bufs = this->out_buffers;

  if (GST_IS_EVENT (buf)) {
    for (p = this->srcpads; p; p = p->next)
      gst_pad_event_default (GST_PAD (p->data), GST_EVENT (buf));
    return;
  }

  if (this->channels == 0) {
    GST_ELEMENT_ERROR (this, CORE, NEGOTIATION, (NULL),
        ("format wasn't negotiated before chain function"));
    return;
  } else if (this->channels == 1) {
    gst_pad_push (GST_PAD (this->srcpads->data), GST_DATA (buf));
    return;
  }

  in_data = (gpointer) GST_BUFFER_DATA (buf);

  /* Create our buffers */
  for (i = 0; i < this->channels; i++) {
    out_bufs[i] =
        gst_buffer_new_and_alloc (GST_BUFFER_SIZE (buf) / this->channels);
    gst_buffer_stamp (out_bufs[i], buf);
    out_data[i] = (gpointer) GST_BUFFER_DATA (out_bufs[i]);
  }

  if (this->is_int) {
    do_int_deinterleave
        ((gint16 *) in_data,
        this->channels,
        (gint16 **) out_data,
        GST_BUFFER_SIZE (buf) / this->channels / sizeof (gint16));
  } else {
    do_float_deinterleave
        ((gfloat *) in_data,
        this->channels,
        (gfloat **) out_data,
        GST_BUFFER_SIZE (buf) / this->channels / sizeof (gfloat));
  }

  gst_buffer_unref (buf);

  for (i = 0, p = this->srcpads; p; p = p->next, i++)
    gst_pad_push (GST_PAD (p->data), GST_DATA (out_bufs[i]));
}

static void inline
do_int_deinterleave (gint16 * in_data, int channels,
    gint16 ** out_data, guint numframes)
{
  guint i, k;

  for (i = 0; i < numframes; i++)
    for (k = 0; k < channels; k++)
      out_data[k][i] = in_data[channels * i + k];
}

static void inline
do_float_deinterleave (gfloat * in_data, int channels,
    gfloat ** out_data, guint numframes)
{
  guint i, k;

  for (i = 0; i < numframes; i++)
    for (k = 0; k < channels; k++)
      out_data[k][i] = in_data[channels * i + k];
}

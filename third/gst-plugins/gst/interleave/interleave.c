/* GStreamer interleave plugin
 * Copyright (C) 2004 Andy Wingo <wingo at pobox dot com>
 *
 * Based on float2int.c
 * Copyright (C) <2001> Steve Baker <stevebaker_org@yahoo.co.uk>
 * Copyright (C) 2002, 2003 Andy Wingo <wingo at pobox dot com>
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
#include "config.h"
#endif

#include <string.h>
#include <gst/gst.h>
#include <gst/audio/audio.h>
#include "plugin.h"

/* I'm trying a new naming convention in this file, because it's stupid to have
 * Gst everywhere. The type function is gstplugin_foo because we're not really
 * in the core namespace, and we want to prevent any kind of clash.
 * Unfortunately we don't have a GType namespace reserved for Gst-defined
 * plugins. I kept interleave in the function names for better backtraces. */

#define GSTPLUGIN_TYPE_INTERLEAVE \
  (gstplugin_interleave_get_type())
#define INTERLEAVE(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSTPLUGIN_TYPE_INTERLEAVE, Interleave))

#define INTERLEAVE_CHANNEL(list)   ((InterleaveInputChannel*)(list->data))

typedef struct _Interleave Interleave;
typedef struct _InterleaveClass InterleaveClass;
typedef struct _InterleaveInputChannel InterleaveInputChannel;

struct _InterleaveInputChannel
{
  GstPad *sinkpad;
  GstBuffer *buffer;
  gboolean new_media;
};

struct _Interleave
{
  GstElement element;

  GstPad *srcpad;
  GList *channels;

  gint numpads;                 /* Number of pads on the element */
  gint numchannels;             /* Actual number of channels */
  gint channelcount;            /* counter to get safest pad name */
  gboolean is_int;

  gint buffer_frames;

  /* these ones are for the nonbuffered loop */
  gint64 offset;
  gint frames_remaining;
};

struct _InterleaveClass
{
  GstElementClass parent_class;
};

static GstElementDetails details = {
  "Audio interleaver",
  "Filter/Converter/Audio",
  "Folds many mono channels into one interleaved audio stream",
  "Andy Wingo <wingo at pobox.com>"
};

static GstStaticPadTemplate sink_static_template =
    GST_STATIC_PAD_TEMPLATE ("sink%d",
    GST_PAD_SINK,
    GST_PAD_REQUEST,
    GST_STATIC_CAPS ("audio/x-raw-int, "
        "rate = (int) [ 1, MAX ], "
        "channels = (int) 1, "
        "endianness = (int) BYTE_ORDER, "
        "width = (int) 16, "
        "depth = (int) 16, "
        "signed = (boolean) { true, false };"
        "audio/x-raw-float, "
        "rate = (int) [ 1, MAX ], "
        "channels = (int) 1,"
        "endianness = (int) BYTE_ORDER, "
        "width = (int) 32," "buffer-frames = (int) [ 0, MAX ]")
    );

static GstStaticPadTemplate src_static_template =
    GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-raw-int, "
        "rate = (int) [ 1, MAX ], "
        "channels = (int) [ 1, MAX ], "
        "endianness = (int) BYTE_ORDER, "
        "width = (int) 16, "
        "depth = (int) 16, "
        "signed = (boolean) { true, false };"
        "audio/x-raw-float, "
        "rate = (int) [ 1, MAX ], "
        "channels = (int) [ 1, MAX ], "
        "endianness = (int) BYTE_ORDER, "
        "width = (int) 32," "buffer-frames = (int) [ 0, MAX ]")
    );

static void interleave_class_init (InterleaveClass * klass);
static void interleave_base_init (InterleaveClass * klass);
static void interleave_init (Interleave * this);

static GstPad *interleave_request_new_pad (GstElement * element,
    GstPadTemplate * temp, const gchar * unused);
static void interleave_release_pad (GstElement * element, GstPad * pad);
static void interleave_pad_removed (GstElement * element, GstPad * pad);
static GstElementStateReturn interleave_change_state (GstElement * element);

static GstCaps *interleave_getcaps (GstPad * pad);
static GstPadLinkReturn interleave_link (GstPad * pad, const GstCaps * caps);
static void interleave_unlink (GstPad * pad);

static void interleave_buffered_loop (GstElement * element);
static void interleave_bytestream_loop (GstElement * element);

static inline void do_float_interleave (gfloat ** data_in, gint nchannels,
    gfloat * data_out, gint nframes);


static GstElementClass *parent_class = NULL;

GType
gstplugin_interleave_get_type (void)
{
  static GType interleave_type = 0;

  if (!interleave_type) {
    static const GTypeInfo interleave_info = {
      sizeof (InterleaveClass),
      (GBaseInitFunc) interleave_base_init,
      NULL,
      (GClassInitFunc) interleave_class_init,
      NULL,
      NULL,
      sizeof (Interleave),
      0,
      (GInstanceInitFunc) interleave_init,
    };

    interleave_type = g_type_register_static (GST_TYPE_ELEMENT, "GstInterleave",
        &interleave_info, 0);
  }
  return interleave_type;
}

static void
interleave_base_init (InterleaveClass * klass)
{
  GstElementClass *eclass = GST_ELEMENT_CLASS (klass);

  gst_element_class_set_details (eclass, &details);
  gst_element_class_add_pad_template
      (eclass, gst_static_pad_template_get (&src_static_template));
  gst_element_class_add_pad_template
      (eclass, gst_static_pad_template_get (&sink_static_template));
}

static void
interleave_class_init (InterleaveClass * klass)
{
  GObjectClass *oclass;
  GstElementClass *eclass;

  oclass = (GObjectClass *) klass;
  eclass = (GstElementClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  eclass->request_new_pad = interleave_request_new_pad;
  eclass->release_pad = interleave_release_pad;
  eclass->pad_removed = interleave_pad_removed;
  eclass->change_state = interleave_change_state;
}

static void
interleave_init (Interleave * this)
{
  gst_element_set_loop_function (GST_ELEMENT (this), interleave_buffered_loop);

  this->srcpad = gst_pad_new_from_template
      (gst_static_pad_template_get (&src_static_template), "src");
  gst_element_add_pad (GST_ELEMENT (this), this->srcpad);
  gst_pad_set_link_function (this->srcpad, interleave_link);
  gst_pad_set_getcaps_function (this->srcpad, interleave_getcaps);

  this->numchannels = 1;        /* Numchannels must start with 1, even if there are
                                   no channels, as 0 channel audio is invalid
                                   and it stops people linking the interleave sink
                                   before the src */
  this->numpads = 0;
  this->channelcount = 0;
  this->channels = NULL;
}


static GstPad *
interleave_request_new_pad (GstElement * element, GstPadTemplate * templ,
    const gchar * unused)
{
  gchar *name;
  Interleave *this;
  InterleaveInputChannel *channel;

  this = INTERLEAVE (element);

  if (templ->direction != GST_PAD_SINK) {
    g_warning ("interleave: request new pad that is not a SINK pad\n");
    return NULL;
  }

  channel = g_new0 (InterleaveInputChannel, 1);
  name = g_strdup_printf ("sink%d", this->channelcount);
  channel->sinkpad = gst_pad_new_from_template (templ, name);
  gst_element_add_pad (GST_ELEMENT (this), channel->sinkpad);
  gst_pad_set_link_function (channel->sinkpad, interleave_link);
  gst_pad_set_unlink_function (channel->sinkpad, interleave_unlink);
  gst_pad_set_getcaps_function (channel->sinkpad, interleave_getcaps);

  this->channels = g_list_append (this->channels, channel);
  if (this->numpads > 0)
    this->numchannels++;
  this->numpads++;
  this->channelcount++;

  GST_DEBUG ("interleave added pad %s\n", name);

  g_free (name);
  return channel->sinkpad;
}

static void
interleave_release_pad (GstElement * element, GstPad * pad)
{
  gst_element_remove_pad (element, pad);
}

static void
interleave_pad_removed (GstElement * element, GstPad * pad)
{
  Interleave *this;
  GList *p;

  GST_DEBUG ("interleave removing pad %s\n", GST_OBJECT_NAME (pad));

  this = INTERLEAVE (element);

  /* Find our channel for this pad */
  for (p = this->channels; p;) {
    InterleaveInputChannel *channel = p->data;
    GList *p_copy;

    if (channel->sinkpad == pad) {
      p_copy = p;

      p = p->next;
      this->channels = g_list_remove_link (this->channels, p_copy);
      this->numpads--;
      if (this->numpads > 0)
        this->numchannels--;

      g_list_free (p_copy);

      if (channel->buffer)
        gst_buffer_unref (channel->buffer);
      g_free (channel);
    } else {
      p = p->next;
    }
  }
}

static GstElementStateReturn
interleave_change_state (GstElement * element)
{
  Interleave *this = (Interleave *) element;
  GList *p;

  switch (GST_STATE_TRANSITION (element)) {
    case GST_STATE_READY_TO_PAUSED:
      this->offset = 0;
      break;

    case GST_STATE_PAUSED_TO_READY:
      for (p = this->channels; p; p = p->next) {
        InterleaveInputChannel *c = p->data;

        if (c->buffer)
          gst_buffer_unref (c->buffer);
        c->buffer = NULL;
      }
      this->frames_remaining = 0;
      break;

    default:
      break;
  }

  if (parent_class->change_state)
    return parent_class->change_state (element);
  return GST_STATE_SUCCESS;
}

static GstCaps *
interleave_getcaps (GstPad * pad)
{
  Interleave *this;
  GList *channels;
  GstCaps *ret, *allowed_caps, *old;
  int i;

  this = INTERLEAVE (GST_OBJECT_PARENT (pad));

  if (pad == this->srcpad)
    ret = gst_caps_copy (gst_pad_get_pad_template_caps (this->srcpad));
  else
    ret = gst_pad_get_allowed_caps (this->srcpad);

  for (channels = this->channels; channels; channels = channels->next) {
    InterleaveInputChannel *channel = INTERLEAVE_CHANNEL (channels);
    GstPad *sinkpad = channel->sinkpad;

    if (sinkpad == pad)
      continue;

    allowed_caps = gst_pad_get_allowed_caps (sinkpad);
    old = ret;
    ret = gst_caps_intersect (old, allowed_caps);
    gst_caps_free (allowed_caps);
    gst_caps_free (old);
  }

  if (pad == this->srcpad)
    for (i = 0; i < gst_caps_get_size (ret); i++)
      gst_structure_set (gst_caps_get_structure (ret, i),
          "channels", G_TYPE_INT, this->numchannels, NULL);
  else
    for (i = 0; i < gst_caps_get_size (ret); i++)
      gst_structure_set (gst_caps_get_structure (ret, i),
          "channels", G_TYPE_INT, 1, NULL);

  GST_DEBUG ("allowed caps %" GST_PTR_FORMAT, ret);

  return ret;
}

static GstPadLinkReturn
interleave_link (GstPad * pad, const GstCaps * caps)
{
  Interleave *this;
  GList *channels;
  GstPadLinkReturn ret;
  GstStructure *structure;

  this = INTERLEAVE (GST_OBJECT_PARENT (pad));

  if (pad == this->srcpad) {
    GstCaps *sinkcaps = gst_caps_copy (caps);

    gst_caps_set_simple (sinkcaps, "channels", G_TYPE_INT, 1, NULL);

    for (channels = this->channels; channels; channels = channels->next) {
      InterleaveInputChannel *channel = INTERLEAVE_CHANNEL (channels);
      GstPad *sinkpad = channel->sinkpad;

      ret = gst_pad_try_set_caps (sinkpad, sinkcaps);
      if (GST_PAD_LINK_FAILED (ret))
        return ret;
    }
  } else {
    GstCaps *srccaps = gst_caps_copy (caps);

    gst_caps_set_simple (srccaps, "channels", G_TYPE_INT, this->numchannels,
        NULL);
    ret = gst_pad_try_set_caps (this->srcpad, srccaps);
    if (GST_PAD_LINK_FAILED (ret))
      return ret;
  }

  g_print ("Interleave has %d channels\n", this->numchannels);

  /* it's ok, let's record our data */
  structure = gst_caps_get_structure (caps, 0);
  this->is_int =
      (strcmp (gst_structure_get_name (structure), "audio/x-raw-int") == 0);
  if (!this->is_int)
    gst_structure_get_int (structure, "buffer-frames", &this->buffer_frames);
  else
    this->buffer_frames = 0;

  /* with buffer_frames==0 or for int elements, we have no guarantee as to the
     buffer sizes, so we use bytestream to help. otherwise we can have an
     optimised loop function. */
  if (this->buffer_frames == 0)
    gst_element_set_loop_function (GST_ELEMENT (this),
        interleave_bytestream_loop);
  else
    gst_element_set_loop_function (GST_ELEMENT (this),
        interleave_buffered_loop);

  return GST_PAD_LINK_OK;
}

static void
interleave_unlink (GstPad * pad)
{
  Interleave *this;
  GstCaps *caps;

  this = INTERLEAVE (GST_OBJECT_PARENT (pad));

  if (GST_IS_PAD (this->srcpad)) {
    gboolean ret;

    caps = gst_pad_get_caps (this->srcpad);
    gst_caps_set_simple (caps, "channels", G_TYPE_INT, this->numchannels - 1,
        NULL);
    ret = gst_pad_try_set_caps (this->srcpad, caps);
    if (ret == FALSE) {
      g_print ("TSC failed\n");
    }
    gst_pad_renegotiate (this->srcpad);
    gst_caps_free (caps);
  }
}

static gboolean
all_channels_new_media (GList * channels)
{
  GList *p;

  for (p = channels; p; p = p->next) {
    InterleaveInputChannel *c = INTERLEAVE_CHANNEL (p);

    if (c->new_media == FALSE) {
      return FALSE;
    }
  }

  return TRUE;
}

static void
interleave_buffered_loop (GstElement * element)
{
  Interleave *this = (Interleave *) element;
  GstBuffer *buf_out;
  gpointer *data_in;
  gpointer data_out;
  gint i, to_process_bytes;
  GList *l;
  InterleaveInputChannel *channel;

  /* fast allocate on the stack, no need to free */
  data_in = g_newa (gpointer, this->numchannels);

  if (!this->channels) {
    GST_ELEMENT_ERROR (element, CORE, PAD, (NULL),
        ("interleave: at least one sink pad needs to be connected"));
    return;
  }

  to_process_bytes = G_MAXINT;

  /* get input buffers */
  for (l = this->channels, i = 0; l; l = l->next, i++) {
    channel = INTERLEAVE_CHANNEL (l);

    if (channel->new_media) {
      continue;
    }

  get_buffer:
    channel->buffer = GST_BUFFER (gst_pad_pull (channel->sinkpad));

    if (GST_IS_EVENT (channel->buffer)) {
      GstEvent *e;

      GST_DEBUG ("got an event");
      e = GST_EVENT (channel->buffer);
      channel->buffer = NULL;

      switch (GST_EVENT_TYPE (e)) {
        case GST_EVENT_EOS:
          GST_DEBUG ("it's an eos");
          gst_pad_push (this->srcpad, GST_DATA (e));
          gst_element_set_eos (GST_ELEMENT (this));
          return;

        case GST_EVENT_DISCONTINUOUS:
          GST_DEBUG ("its a discont");

          if (GST_EVENT_DISCONT_NEW_MEDIA (e)) {
            channel->new_media = TRUE;
            if (all_channels_new_media (this->channels)) {
              GList *t;

              /* Reset new media */
              for (t = this->channels; t; t = t->next) {
                InterleaveInputChannel *c = INTERLEAVE_CHANNEL (t);

                c->new_media = FALSE;
              }

              /* Push event */
              gst_pad_event_default (channel->sinkpad, e);
              goto get_buffer;
            } else {
              GST_DEBUG ("Not all channels have received a new-media yet");
              gst_event_unref (e);
              goto get_buffer;
            }
          }
          break;

        default:
          GST_DEBUG ("doing default action for event");
          gst_pad_event_default (channel->sinkpad, e);
          goto get_buffer;      /* goto, hahaha */
      }
    }

    /* we might be discarding extra data if one buffer is shorter, but that's
       ok, because the buffer-frames property means if one buffer is short, EOS is
       coming next time. */
    to_process_bytes =
        MIN (GST_BUFFER_SIZE (channel->buffer), to_process_bytes);
    data_in[i] = (gpointer) GST_BUFFER_DATA (channel->buffer);
  }

  /* fast push in the single-channel case */
  if (this->numchannels == 1) {
    channel = INTERLEAVE_CHANNEL (this->channels);
    buf_out = channel->buffer;
    channel->buffer = NULL;
    gst_pad_push (this->srcpad, GST_DATA (buf_out));
    return;
  }

  /* make the output */
  buf_out = gst_buffer_new_and_alloc (to_process_bytes * this->numchannels);
  gst_buffer_stamp (buf_out, INTERLEAVE_CHANNEL (this->channels)->buffer);
  data_out = (gpointer *) GST_BUFFER_DATA (buf_out);

  /* if in the future int gets a buffer-frames property, we should switch here
   * on the type */
  do_float_interleave ((gfloat **) data_in, this->numpads,
      (gfloat *) data_out, to_process_bytes / sizeof (gfloat));

  /* clean up and push */
  for (l = this->channels; l; l = l->next) {
    channel = INTERLEAVE_CHANNEL (l);
    gst_buffer_unref (channel->buffer);
    channel->buffer = NULL;
  }

  gst_pad_push (this->srcpad, GST_DATA (buf_out));
}

static void
interleave_bytestream_loop (GstElement * element)
{
  GST_ELEMENT_ERROR (element, CORE, NOT_IMPLEMENTED, (NULL),
      ("interleave: unbuffered mode is not yet implemented"));

  /* Should look the same as the buffered loop, except that getting the data is
     done via bytestream. Still calls the do_*_interleave functions the same
     way. */
}

static inline void
do_float_interleave (gfloat ** data_in, gint nchannels,
    gfloat * data_out, gint nframes)
{
  int i, j;

  for (i = 0; i < nframes; i++)
    for (j = 0; j < nchannels; j++)
      data_out[i * nchannels + j] = data_in[j][i];
}

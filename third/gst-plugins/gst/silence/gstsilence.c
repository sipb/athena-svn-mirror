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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <string.h>
#include <gstsilence.h>


/* Silence signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  ARG_0,
  ARG_BYTESPERREAD,
  ARG_SYNC
};

/* elementfactory information */
static GstElementDetails gst_silence_details = {
  "silence source",
  "Source/Audio",
  "Generates silence",
  "Zaheer Abbas Merali <zaheerabbas at merali dot org>"
};

static GstStaticPadTemplate gst_silence_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-raw-int, "
        "endianness = (int) BYTE_ORDER, "
        "signed = (boolean) true, "
        "width = (int) 16, "
        "depth = (int) 16, "
        "rate = (int) [ 8000, 48000 ], "
        "channels = (int) [ 1, 2 ]; "
        "audio/x-mulaw, "
        "rate = (int) [ 8000, 48000 ], " "channels = (int) [ 1, 2 ]")
    );

static void gst_silence_class_init (GstSilenceClass * klass);
static void gst_silence_base_init (GstSilenceClass * klass);
static void gst_silence_init (GstSilence * silence);

static void gst_silence_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_silence_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);
static GstElementStateReturn gst_silence_change_state (GstElement * element);
static void gst_silence_set_clock (GstElement * element, GstClock * clock);

static GstData *gst_silence_get (GstPad * pad);
static GstPadLinkReturn gst_silence_link (GstPad * pad, const GstCaps * caps);
static const GstQueryType *gst_silence_get_query_types (GstPad * pad);
static gboolean gst_silence_src_query (GstPad * pad,
    GstQueryType type, GstFormat * format, gint64 * value);

static GstElementClass *parent_class = NULL;

/*static guint gst_osssrc_signals[LAST_SIGNAL] = { 0 }; */

GType
gst_silence_get_type (void)
{
  static GType silence_type = 0;

  if (!silence_type) {
    static const GTypeInfo silence_info = {
      sizeof (GstSilenceClass),
      (GBaseInitFunc) gst_silence_base_init,
      NULL,
      (GClassInitFunc) gst_silence_class_init,
      NULL,
      NULL,
      sizeof (GstSilence),
      0,
      (GInstanceInitFunc) gst_silence_init,
    };

    silence_type =
        g_type_register_static (GST_TYPE_ELEMENT, "GstSilence", &silence_info,
        0);
  }
  return silence_type;
}

static void
gst_silence_base_init (GstSilenceClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_silence_src_template));
  gst_element_class_set_details (element_class, &gst_silence_details);
}

static void
gst_silence_class_init (GstSilenceClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_BYTESPERREAD,
      g_param_spec_ulong ("bytes_per_read", "Bytes per read",
          "Bytes per buffer in one cycle",
          0, G_MAXULONG, 0, G_PARAM_READWRITE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_SYNC,
      g_param_spec_boolean ("sync", "Sync", "Synchronize to clock",
          TRUE, G_PARAM_READWRITE));

  gobject_class->set_property = gst_silence_set_property;
  gobject_class->get_property = gst_silence_get_property;

  gstelement_class->change_state = gst_silence_change_state;
  gstelement_class->set_clock = gst_silence_set_clock;
}

static void
gst_silence_set_clock (GstElement * element, GstClock * clock)
{
  GstSilence *silence;

  silence = GST_SILENCE (element);

  gst_object_replace ((GstObject **) & silence->clock, (GstObject *) clock);
}

static void
gst_silence_init (GstSilence * silence)
{
  silence->srcpad =
      gst_pad_new_from_template (gst_static_pad_template_get
      (&gst_silence_src_template), "src");
  gst_pad_set_get_function (silence->srcpad, gst_silence_get);
  gst_pad_set_link_function (silence->srcpad, gst_silence_link);
  gst_pad_set_query_function (silence->srcpad, gst_silence_src_query);
  gst_pad_set_query_type_function (silence->srcpad,
      gst_silence_get_query_types);
  gst_element_add_pad (GST_ELEMENT (silence), silence->srcpad);

  /* adding some default values */
  silence->law = 0;
  silence->channels = 2;
  silence->frequency = 44100;
  silence->bytes_per_read = 4096;
  silence->width = 2;           /* 2 bytes */
  silence->timestamp = 0;
  silence->offset = 0;
  silence->samples = 0;
}

#define gst_caps_get_int_range(caps, name, min, max) \
  gst_props_entry_get_int_range(gst_props_get_entry((caps)->properties, \
                                                    name), \
                                min, max)

static GstPadLinkReturn
gst_silence_link (GstPad * pad, const GstCaps * caps)
{
  GstSilence *silence = GST_SILENCE (gst_pad_get_parent (pad));
  GstStructure *structure;

  structure = gst_caps_get_structure (caps, 0);

  gst_structure_get_int (structure, "rate", &silence->frequency);
  gst_structure_get_int (structure, "channels", &silence->channels);

  if (!strcmp (gst_structure_get_name (structure), "audio/x-raw-int")) {
    silence->law = 0;
    gst_structure_get_int (structure, "width", &silence->width);
    silence->width /= 8;
  } else {
    silence->law = 1;
    silence->width = 1;
  }

  return GST_PAD_LINK_OK;
}

static const GstQueryType *
gst_silence_get_query_types (GstPad * pad)
{
  static const GstQueryType query_types[] = {
    GST_QUERY_POSITION,
    0,
  };

  return query_types;
}

static gboolean
gst_silence_src_query (GstPad * pad,
    GstQueryType type, GstFormat * format, gint64 * value)
{
  gboolean res = FALSE;
  GstSilence *src;

  src = GST_SILENCE (gst_pad_get_parent (pad));

  switch (type) {
    case GST_QUERY_POSITION:
      switch (*format) {
        case GST_FORMAT_TIME:
          *value = src->timestamp;
          res = TRUE;
          break;
        case GST_FORMAT_DEFAULT:       /* samples */
          *value = src->samples;
          res = TRUE;
          break;
        case GST_FORMAT_BYTES:
          *value = src->offset;
          res = TRUE;
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }

  return res;
}

static GstData *
gst_silence_get (GstPad * pad)
{
  GstSilence *src;
  GstBuffer *buf;
  guint tdiff, samples;
  gint i;

  g_return_val_if_fail (pad != NULL, NULL);
  src = GST_SILENCE (gst_pad_get_parent (pad));

  if (!GST_PAD_CAPS (src->srcpad)) {
    if (gst_silence_link (src->srcpad,
            gst_pad_get_allowed_caps (src->srcpad)) <= 0) {
      GST_ELEMENT_ERROR (src, CORE, NEGOTIATION, (NULL), (NULL));
      return NULL;
    }
  }

  samples = src->bytes_per_read / (src->channels * src->width);
  tdiff = samples * GST_SECOND / src->frequency;

  buf = gst_buffer_new_and_alloc (src->bytes_per_read);
  GST_BUFFER_OFFSET (buf) = src->offset;
  GST_BUFFER_TIMESTAMP (buf) = src->timestamp;
  if (src->sync && src->clock) {
    gst_element_wait (GST_ELEMENT (src), GST_BUFFER_TIMESTAMP (buf));
  }
  GST_BUFFER_DURATION (buf) = tdiff;
  GST_BUFFER_SIZE (buf) = src->bytes_per_read;

  if (src->law || src->width == 1)
    memset (GST_BUFFER_DATA (buf), src->law ? 0 : 128, src->bytes_per_read);
  else {
    /* ok src->width = 2 */
    gint16 *tmp = (gint16 *) GST_BUFFER_DATA (buf);

    for (i = 0; i < src->bytes_per_read / 2; i++) {
      tmp[i] = (gint16) 0;
    }
  }

  src->timestamp += tdiff;
  src->samples += samples;
  src->offset += src->bytes_per_read;

  return GST_DATA (buf);
}

static void
gst_silence_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstSilence *src;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_SILENCE (object));

  src = GST_SILENCE (object);

  switch (prop_id) {
    case ARG_BYTESPERREAD:
      src->bytes_per_read = g_value_get_ulong (value);
      break;
    case ARG_SYNC:
      src->sync = g_value_get_boolean (value);
      break;
    default:
      break;
  }
}

static void
gst_silence_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstSilence *src;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_SILENCE (object));

  src = GST_SILENCE (object);

  switch (prop_id) {
    case ARG_BYTESPERREAD:
      g_value_set_ulong (value, src->bytes_per_read);
      break;
    case ARG_SYNC:
      g_value_set_boolean (value, src->sync);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static GstElementStateReturn
gst_silence_change_state (GstElement * element)
{
  GstSilence *silence = GST_SILENCE (element);

  switch (GST_STATE_PENDING (element)) {
    case GST_STATE_PAUSED_TO_READY:
      silence->timestamp = 0;
      silence->offset = 0;
      silence->samples = 0;
      break;
    default:
      break;
  }

  if (GST_ELEMENT_CLASS (parent_class)->change_state)
    return GST_ELEMENT_CLASS (parent_class)->change_state (element);

  return GST_STATE_SUCCESS;
}

#if 0
static void
gst_silence_sync_parms (GstSilence * silence)
{
  GstCaps *caps;

  if (silence->law) {
    caps = gst_caps_from_string ("audio/x-mulaw");
  } else {
    caps = gst_caps_from_string ("audio/x-raw-int, "
        "endianness = (int) BYTE_ORDER, "
        "signed = (boolean) true, " "width = (int) 16, " "depth = (int) 16");
  }

  gst_caps_set_simple (caps,
      "rate", G_TYPE_INT, (int) silence->frequency,
      "channels", G_TYPE_INT, (int) silence->channels, NULL);

  /* set caps on src pad */
  gst_pad_try_set_caps (silence->srcpad, caps);
}
#endif

static gboolean
plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "silence",
      GST_RANK_NONE, GST_TYPE_SILENCE);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "silence",
    "Generates silent audio",
    plugin_init, VERSION, "LGPL", GST_PACKAGE, GST_ORIGIN)

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

/*#define DEBUG_ENABLED */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "gstmpegaudio.h"

/* elementfactory information */
static GstElementDetails gst_mpegaudio_details = {
  "mpegaudio mp3 encoder",
  "Codec/Encoder/Audio",
  "Uses modified mpegaudio code to encode to mp3 streams",
  "Erik Walthinsen <omega@cse.ogi.edu>\n" "Wim Taymans <wim.taymans@chello.be>"
};


/* MpegAudio signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  ARG_0,
  ARG_MODE,
  ARG_LAYER,
  ARG_MODEL,
  ARG_BITRATE,
  ARG_EMPHASIS
      /* FILL ME */
};

static GstStaticPadTemplate mpegaudio_sink_templ =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/x-raw-int, "
        "endianness = (int) BYTE_ORDER, "
        "signed = (boolean) true, "
        "width = (int) 16, "
        "depth = (int) 16, "
        "rate = (int) { 32000, 44100, 48000 }, " "channels = (int) [ 1, 2 ]")
    );

static GstStaticPadTemplate mpegaudio_src_templ =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("audio/mpeg, "
        "mpegversion = (int) 1, "
        "layer = (int) [ 1, 2 ], "
        "channels = (int) [ 1, 2 ], " "rate = [ 8000, 96000 ]")
    );

/********** Define useful types for non-programmatic interfaces **********/
#define GST_TYPE_MPEGAUDIO_MODE (gst_mpegaudio_mode_get_type())
static GType
gst_mpegaudio_mode_get_type (void)
{
  static GType mpegaudio_mode_type = 0;
  static GEnumValue mpegaudio_modes[] = {
    {MPG_MD_STEREO, "0", "Stereo"},
    {MPG_MD_JOINT_STEREO, "1", "Joint-Stereo"},
    {MPG_MD_DUAL_CHANNEL, "2", "Dual channel"},
    {MPG_MD_MONO, "3", "Mono"},
    {0, NULL, NULL},
  };

  if (!mpegaudio_mode_type) {
    mpegaudio_mode_type =
        g_enum_register_static ("GstMpegAudioMode", mpegaudio_modes);
  }
  return mpegaudio_mode_type;
}

static void gst_mpegaudio_class_init (GstMpegAudioClass * klass);
static void gst_mpegaudio_base_init (GstMpegAudioClass * klass);
static void gst_mpegaudio_init (GstMpegAudio * mpegaudio);

static void gst_mpegaudio_chain (GstPad * pad, GstData * _data);

static void gst_mpegaudio_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);
static void gst_mpegaudio_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);

static GstElementClass *parent_class = NULL;

/*static guint gst_mpegaudio_signals[LAST_SIGNAL] = { 0 }; */

GType
gst_mpegaudio_get_type (void)
{
  static GType mpegaudio_type = 0;

  if (!mpegaudio_type) {
    static const GTypeInfo mpegaudio_info = {
      sizeof (GstMpegAudioClass),
      (GBaseInitFunc) gst_mpegaudio_base_init,
      NULL,
      (GClassInitFunc) gst_mpegaudio_class_init,
      NULL,
      NULL,
      sizeof (GstMpegAudio),
      0,
      (GInstanceInitFunc) gst_mpegaudio_init,
    };

    mpegaudio_type =
        g_type_register_static (GST_TYPE_ELEMENT, "GstMpegAudio",
        &mpegaudio_info, 0);
  }
  return mpegaudio_type;
}

static void
gst_mpegaudio_base_init (GstMpegAudioClass * klass)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&mpegaudio_sink_templ));
  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&mpegaudio_src_templ));
  gst_element_class_set_details (element_class, &gst_mpegaudio_details);
}

static void
gst_mpegaudio_class_init (GstMpegAudioClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_MODE, g_param_spec_enum ("mode", "mode", "mode", GST_TYPE_MPEGAUDIO_MODE, 0, G_PARAM_READWRITE));        /* CHECKME! */
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_LAYER,
      g_param_spec_int ("layer", "layer", "layer", 1, 3, 1, G_PARAM_READWRITE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_MODEL, g_param_spec_int ("model", "model", "model", G_MININT, G_MAXINT, 0, G_PARAM_READWRITE));  /* CHECKME */
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_BITRATE, g_param_spec_int ("bitrate", "bitrate", "bitrate", G_MININT, G_MAXINT, 0, G_PARAM_READWRITE));  /* CHECKME */
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_EMPHASIS, g_param_spec_int ("emphasis", "emphasis", "emphasis", G_MININT, G_MAXINT, 0, G_PARAM_READWRITE));      /* CHECKME */

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  gobject_class->set_property = gst_mpegaudio_set_property;
  gobject_class->get_property = gst_mpegaudio_get_property;

}

static GstPadLinkReturn
gst_mpegaudio_sinkconnect (GstPad * pad, const GstCaps * caps)
{
  GstMpegAudio *mpegaudio;
  gint frequency, channels;
  GstStructure *structure;

  mpegaudio = GST_MPEGAUDIO (gst_pad_get_parent (pad));

  structure = gst_caps_get_structure (caps, 0);

  gst_structure_get_int (structure, "rate", &frequency);
  gst_structure_get_int (structure, "channels", &channels);
  mpegaudio->encoder->frequency = frequency;

  if (channels == 1)
    mpegaudio->encoder->info.mode = MPG_MD_MONO;

  mpegaudio_sync_parms (mpegaudio->encoder);

  caps = gst_caps_new_simple ("audio/mpeg",
      "mpegversion", G_TYPE_INT, 1,
      "layer", G_TYPE_INT, mpegaudio->encoder->info.lay,
      "channels", G_TYPE_INT, channels, "rate", G_TYPE_INT, frequency, NULL);

  return gst_pad_try_set_caps (mpegaudio->srcpad, caps);
}

static void
gst_mpegaudio_init (GstMpegAudio * mpegaudio)
{
  /* create the sink and src pads */
  mpegaudio->sinkpad =
      gst_pad_new_from_template (gst_static_pad_template_get
      (&mpegaudio_sink_templ), "sink");
  gst_element_add_pad (GST_ELEMENT (mpegaudio), mpegaudio->sinkpad);
  gst_pad_set_chain_function (mpegaudio->sinkpad, gst_mpegaudio_chain);
  gst_pad_set_link_function (mpegaudio->sinkpad, gst_mpegaudio_sinkconnect);

  mpegaudio->srcpad =
      gst_pad_new_from_template (gst_static_pad_template_get
      (&mpegaudio_src_templ), "src");
  gst_element_add_pad (GST_ELEMENT (mpegaudio), mpegaudio->srcpad);

  /* initialize the mpegaudio encoder state */
  mpegaudio->encoder = mpegaudio_init_encoder ();
  mpegaudio->partialbuf = NULL;
  mpegaudio->partialsize = 0;
}

static void
gst_mpegaudio_chain (GstPad * pad, GstData * _data)
{
  GstBuffer *buf = GST_BUFFER (_data);
  GstMpegAudio *mpegaudio;
  guchar *data;
  gulong size;
  GstBuffer *outbuf;
  guint handled, tohandle;

  g_return_if_fail (pad != NULL);
  g_return_if_fail (GST_IS_PAD (pad));
  g_return_if_fail (buf != NULL);
/*  g_return_if_fail(GST_IS_BUFFER(buf)); */

  mpegaudio = GST_MPEGAUDIO (gst_pad_get_parent (pad));

  data = (guchar *) GST_BUFFER_DATA (buf);
  size = GST_BUFFER_SIZE (buf);

  GST_DEBUG ("gst_mpegaudio_chain: got buffer of %ld bytes in '%s'", size,
      GST_OBJECT_NAME (mpegaudio));

  handled = 0;
  tohandle = mpegaudio_get_number_of_input_bytes (mpegaudio->encoder);

  if (mpegaudio->partialbuf) {
    mpegaudio->partialbuf =
        g_realloc (mpegaudio->partialbuf, mpegaudio->partialsize + size);
    memcpy (mpegaudio->partialbuf + mpegaudio->partialsize, data, size);

    data = mpegaudio->partialbuf;
    size += mpegaudio->partialsize;
  }

  GST_DEBUG ("need to handle %d bytes", tohandle);
  while (handled + tohandle < size) {

    outbuf = gst_buffer_new ();
    GST_BUFFER_DATA (outbuf) = g_malloc (tohandle);

    GST_DEBUG ("about to encode a frame");
    mpegaudio_encode_frame (mpegaudio->encoder, data, GST_BUFFER_DATA (outbuf),
        (gulong *) & GST_BUFFER_SIZE (outbuf));

    GST_DEBUG ("mpegaudio: pushing %d bytes", GST_BUFFER_SIZE (outbuf));
    gst_pad_push (mpegaudio->srcpad, GST_DATA (outbuf));
    GST_DEBUG ("mpegaudio: pushed buffer");

    data += tohandle;
    handled += tohandle;
  }

  if (size > handled) {
    GST_DEBUG ("mpegaudio: leftover buffer %ld bytes", size - handled);

    if (!mpegaudio->partialbuf) {
      mpegaudio->partialbuf = g_malloc (size - handled);
    }
    memcpy (mpegaudio->partialbuf, data, size - handled);
    mpegaudio->partialsize = size - handled;
  } else {
    if (mpegaudio->partialbuf) {
      g_free (mpegaudio->partialbuf);
      mpegaudio->partialbuf = NULL;
    }
  }

  gst_buffer_unref (buf);
}

static void
gst_mpegaudio_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstMpegAudio *mpegaudio;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_MPEGAUDIO (object));

  mpegaudio = GST_MPEGAUDIO (object);

  switch (prop_id) {
    case ARG_MODE:
      g_value_set_enum (value, mpegaudio->encoder->info.mode);
      break;
    case ARG_LAYER:
      g_value_set_int (value, mpegaudio->encoder->info.lay);
      break;
    case ARG_MODEL:
      g_value_set_int (value, mpegaudio->encoder->model);
      break;
    case ARG_BITRATE:
      g_value_set_int (value, mpegaudio->encoder->bitrate * 1000);
      break;
    case ARG_EMPHASIS:
      g_value_set_int (value, mpegaudio->encoder->info.emphasis);
      break;
    default:
      break;
  }
}

static void
gst_mpegaudio_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstMpegAudio *mpegaudio;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_MPEGAUDIO (object));

  mpegaudio = GST_MPEGAUDIO (object);

  switch (prop_id) {
    case ARG_MODE:
      mpegaudio->encoder->info.mode = g_value_get_int (value);
      break;
    case ARG_LAYER:
      mpegaudio->encoder->info.lay = g_value_get_int (value);
      break;
    case ARG_MODEL:
      mpegaudio->encoder->model = g_value_get_int (value);
      break;
    case ARG_BITRATE:
      mpegaudio->encoder->bitrate = g_value_get_int (value) / 1000;
      break;
    case ARG_EMPHASIS:
      mpegaudio->encoder->info.emphasis = g_value_get_int (value);
      break;
    default:
      break;
  }
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  return gst_element_register (plugin, "mpegaudio",
      GST_RANK_NONE, GST_TYPE_MPEGAUDIO);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "mpegaudio",
    "MPEG-1 layer 1/2 audio encoder",
    plugin_init, VERSION, "LGPL", GST_PACKAGE, GST_ORIGIN)

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

#include <string.h>

#define DEFAULT_COMPRESSION "DIV3"
#define DEFAULT_BITRATE 800
#define DEFAULT_QUALITY 8000
#define DEFAULT_KEYFRAME 20

#include "gstwinenc.h"
#include <creators.h>
#include <image.h>

/* elementfactory information */
GstElementDetails gst_winenc_details = {
  "Windows codec image encoder",
  "Filter/Encoder/Image",
  "Uses the Avifile library to encode avi video using the windows dlls",
  VERSION,
  "Wim Taymans <wim.taymans@chello.be> "
  "Eugene Kuznetsov (http://divx.euro.ru)",
  "(C) 2000",
};

GST_PAD_TEMPLATE_FACTORY (src_factory_2,
  "src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
    "winenc_src",
    "video/avi",
      "format",         GST_PROPS_STRING ("strf_vids")
  )
)

GST_PAD_TEMPLATE_FACTORY (sink_factory_2,
  "sink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
    "winenc_sink",
    "video/raw",
      "format",         GST_PROPS_LIST (
                          GST_PROPS_FOURCC (GST_MAKE_FOURCC ('I','4','2','0')),
                          GST_PROPS_FOURCC (GST_MAKE_FOURCC ('Y','U','Y','2')),
                          GST_PROPS_FOURCC (GST_MAKE_FOURCC ('R','G','B',' '))
                        ),
      "width",          GST_PROPS_INT_RANGE (16, 4096),
      "height",         GST_PROPS_INT_RANGE (16, 4096)
  )
)

/* WinEnc signals and args */
enum {
  /* FILL ME */
  SIGNAL_FRAME_ENCODED,
  LAST_SIGNAL
};

enum {
  ARG_0,
  ARG_BITRATE,
  ARG_QUALITY,
  ARG_COMPRESSION,
  ARG_KEYFRAME,
  ARG_LAST_FRAME_SIZE,
  /* FILL ME */
};

static void 	gst_winenc_class_init		(GstWinEnc *klass);
static void 	gst_winenc_init			(GstWinEnc *winenc);

static void 	gst_winenc_chain		(GstPad *pad, GstBuffer *buf);
static GstPadLinkReturn
		gst_winenc_sinkconnect 		(GstPad *pad, GstCaps *caps); 

static void     gst_winenc_get_property         (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void     gst_winenc_set_property         (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);



static GstElementClass *parent_class = NULL;
static guint gst_winenc_signals[LAST_SIGNAL] = { 0 };

GType
gst_winenc_get_type (void)
{
  static GType winenc_type = 0;

  if (!winenc_type) {
    static const GTypeInfo winenc_info = {
      sizeof(GstWinEncClass),
      NULL,
      NULL,
      (GClassInitFunc) gst_winenc_class_init,
      NULL,
      NULL,
      sizeof(GstWinEnc),
      0,
      (GInstanceInitFunc) gst_winenc_init,
      NULL
    };
    winenc_type = g_type_register_static (GST_TYPE_ELEMENT, "GstWinEnc", &winenc_info, (GTypeFlags)0);
  }
  return winenc_type;
}

static void
gst_winenc_class_init (GstWinEnc *klass) 
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*) klass;
  gstelement_class = (GstElementClass*) klass;


  parent_class = GST_ELEMENT_CLASS (g_type_class_ref (GST_TYPE_ELEMENT));

  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_BITRATE,
    g_param_spec_int ("bitrate", "bitrate", "bitrate",
                      0, G_MAXINT, DEFAULT_BITRATE, (GParamFlags)G_PARAM_READWRITE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_QUALITY,
    g_param_spec_int ("quality", "quality", "quality",
                      0, 10000, DEFAULT_QUALITY, (GParamFlags)G_PARAM_READWRITE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_COMPRESSION,
    g_param_spec_string ("compression", "compression", "compression",
                      DEFAULT_COMPRESSION, (GParamFlags)G_PARAM_READWRITE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_KEYFRAME,
    g_param_spec_int ("keyframe", "keyframe", "keyframe",
                      0, G_MAXINT, DEFAULT_KEYFRAME, (GParamFlags)G_PARAM_READWRITE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_LAST_FRAME_SIZE,
    g_param_spec_ulong ("last_frame_size", "last_frame_size", "last_frame_size",
                      0, G_MAXINT, 0, (GParamFlags)G_PARAM_READABLE));

  gobject_class->set_property = gst_winenc_set_property;
  gobject_class->get_property = gst_winenc_get_property;

  gst_winenc_signals[SIGNAL_FRAME_ENCODED] =
      g_signal_new ("frame_encoded", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (GstWinEncClass, frame_encoded), NULL, NULL,
      g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1, G_TYPE_INT);
}

static void 
gst_winenc_init (GstWinEnc *winenc) 
{
  /* create the sink and src pads */
  winenc->sinkpad = gst_pad_new_from_template (
  		GST_PAD_TEMPLATE_GET (sink_factory_2), "sink");
  gst_element_add_pad (GST_ELEMENT (winenc), winenc->sinkpad);
  gst_pad_set_chain_function (winenc->sinkpad, gst_winenc_chain);
  gst_pad_set_link_function (winenc->sinkpad, gst_winenc_sinkconnect);

  winenc->srcpad = gst_pad_new_from_template (
  		GST_PAD_TEMPLATE_GET (src_factory_2), "src");
  gst_element_add_pad (GST_ELEMENT (winenc), winenc->srcpad);

  // reset the initial video state
  winenc->bitrate = DEFAULT_BITRATE;
  winenc->quality = DEFAULT_QUALITY;
  winenc->keyframe = DEFAULT_KEYFRAME;
  winenc->compression = GST_STR_FOURCC (DEFAULT_COMPRESSION);

  winenc->last_frame_size = 0;
}

static GstPadLinkReturn
gst_winenc_sinkconnect (GstPad *pad, GstCaps *caps) 
{
  GstWinEnc *winenc;
  BITMAPINFOHEADER obh, *bh;
  guint32 format;
  gint width;
  gint height;
  gint bit_count, image_size;
  gint depth = 0;
  
  gst_caps_get_fourcc_int (caps, "format", &format);
  gst_caps_get_int 	  (caps, "width",  &width);
  gst_caps_get_int 	  (caps, "height", &height);

  switch (format) {
    case GST_MAKE_FOURCC ('Y','U','Y','2'):
      image_size = width * height * 2;
      bit_count = 24;
      break;
    case GST_MAKE_FOURCC ('I','4','2','0'):
      image_size = width * height + (width * height / 2);
      bit_count = 24;
      break;
    case GST_MAKE_FOURCC ('R','G','B',' '):
      gst_caps_get_int (caps, "depth", &depth);
      image_size = width * height * (depth/8);
      bit_count = depth;
      break;
    default:
      g_assert_not_reached ();
      return GST_PAD_LINK_REFUSED;  // fix compiler warning, never reached
  }
  winenc = GST_WINENC (gst_pad_get_parent (pad));

  bh = &winenc->bh;

  bh->biSize             = sizeof (BITMAPINFOHEADER);
  bh->biWidth            = width;
  bh->biHeight           = height;
  bh->biPlanes           = 1;
  bh->biBitCount         = bit_count;
  bh->biCompression      = format;
  bh->biSizeImage        = image_size;
  bh->biXPelsPerMeter    = 0;
  bh->biYPelsPerMeter    = 0;
  bh->biClrUsed          = 0;
  bh->biClrImportant     = 0;

  winenc->encoder = Creators::CreateVideoEncoder (winenc->compression, *bh);
  const CodecInfo *ci = CodecInfo::match (winenc->compression);

  if (ci) {
    GST_DEBUG (0, "setting bitrate to %d", winenc->bitrate);
    Creators::SetCodecAttr (*ci, "BitRate", winenc->bitrate);
  }
  winenc->encoder->SetQuality (winenc->quality);
  winenc->encoder->SetKeyFrame (winenc->keyframe);
  obh = winenc->encoder->QueryOutputFormat ();

  if (gst_pad_try_set_caps (winenc->srcpad, 
                  GST_CAPS_NEW (
                    "avidec_video_src",
                    "video/avi",
                        "format",           GST_PROPS_STRING ("strf_vids"),
                          "size",           GST_PROPS_INT (obh.biSize),
                          "width",          GST_PROPS_INT (obh.biWidth),
                          "height",         GST_PROPS_INT (obh.biHeight),
                          "planes",         GST_PROPS_INT (obh.biPlanes),
                          "bit_cnt",        GST_PROPS_INT (obh.biBitCount),
                          "compression",    GST_PROPS_FOURCC (obh.biCompression),
                          "image_size",     GST_PROPS_INT (obh.biSizeImage),
                          "xpels_meter",    GST_PROPS_INT (obh.biXPelsPerMeter),
                          "ypels_meter",    GST_PROPS_INT (obh.biYPelsPerMeter),
                          "num_colors",     GST_PROPS_INT (obh.biClrUsed),
                          "imp_colors",     GST_PROPS_INT (obh.biClrImportant)
                   )) > 0) 
  {
    winenc->encoder->Start ();
    return GST_PAD_LINK_OK;
  }
  return GST_PAD_LINK_REFUSED;
}

static void 
gst_winenc_chain (GstPad *pad, GstBuffer *buf) 
{
  GstWinEnc *winenc;
  uint_t size = 0;
  gint key_frame = 1;
  GstBuffer *outbuf;

  g_return_if_fail (pad != NULL);
  g_return_if_fail (GST_IS_PAD (pad));
  g_return_if_fail (buf != NULL);

  winenc = GST_WINENC (gst_pad_get_parent (pad));

  GST_DEBUG (0,"gst_winenc_chain: got buffer of %d bytes in '%s'", GST_BUFFER_SIZE (buf),
          gst_element_get_name (GST_ELEMENT (winenc)));

  g_assert (GST_BUFFER_SIZE (buf) == (gulong)winenc->bh.biSizeImage);

  CImage *im = new CImage((BitmapInfo*)&winenc->bh, GST_BUFFER_DATA (buf), TRUE);
  gst_buffer_unref(buf);

  outbuf = gst_buffer_new ();
  GST_BUFFER_DATA (outbuf) = (guchar *) g_malloc (winenc->encoder->QueryOutputSize()+4);

  winenc->encoder->EncodeFrame (im, (void*)GST_BUFFER_DATA (outbuf), &key_frame, &size);

  GST_BUFFER_SIZE (outbuf) = size;
  im->Release();

  gst_pad_push(winenc->srcpad, outbuf);
}

static void 
gst_winenc_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  GstWinEnc *winenc;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_WINENC (object));

  winenc = GST_WINENC (object);

  switch(prop_id) {
    case ARG_BITRATE:
      winenc->bitrate = g_value_get_int (value);
      break;
    case ARG_QUALITY:
      winenc->quality = g_value_get_int (value);
      break;
    case ARG_KEYFRAME:
      winenc->keyframe = g_value_get_int (value);
      break;
    case ARG_COMPRESSION:
    {
      const gchar *fourcc = g_value_get_string (value);

      winenc->compression = GST_STR_FOURCC (fourcc);
      break;
    }
    default:
      //G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void 
gst_winenc_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GstWinEnc *winenc;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_WINENC (object));

  winenc = GST_WINENC (object);

  switch (prop_id) {
    case ARG_LAST_FRAME_SIZE:
      g_value_set_ulong (value, winenc->last_frame_size);
      break;
    case ARG_BITRATE:
      g_value_set_int (value, winenc->bitrate);
      break;
    case ARG_QUALITY:
      g_value_set_int (value, winenc->quality);
      break;
    case ARG_KEYFRAME:
      g_value_set_int (value, winenc->keyframe);
      break;
    case ARG_COMPRESSION:
      g_value_set_string (value, g_strdup_printf ("%4.4s", (gchar *)&winenc->compression));
      break;
    default: 
      //G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


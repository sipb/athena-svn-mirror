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

#include "gstwindec.h"

#include <creators.h>
#include <version.h> /* avifile versioning */

/* elementfactory information */
GstElementDetails gst_windec_details = {
  "Windows codec decoder",
  "Filter/Decoder/Image",
  "GPL",
  "Uses the Avifile library to decode avi video using the windows dlls",
  VERSION,
  "Wim Taymans <wim.taymans@chello.be> "
  "Eugene Kuznetsov (http://divx.euro.ru)",
  "(C) 2000",
};

extern GstPadTemplate *wincodec_src_temp;
extern GstPadTemplate *wincodec_sink_temp;

/* Winec signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0,
  /* FILL ME */
};

static void 	gst_windec_class_init		(GstWinDec *klass);
static void 	gst_windec_init			(GstWinDec *windec);

static void 	gst_windec_chain 		(GstPad *pad, GstBuffer *buf);
static GstElementStateReturn
        	gst_windec_change_state 	(GstElement *element);

static GstPadLinkReturn
		gst_windec_srcconnect 		(GstPad *pad, GstCaps *caps);

static void     gst_windec_get_property         (GObject *object, guint prop_id, 
						 GValue *value, GParamSpec *pspec);
static void     gst_windec_set_property         (GObject *object, guint prop_id, 
						 const GValue *value, GParamSpec *pspec);


//static int formats[] = { 32, 24, 16, 15, 0 };

typedef struct _GstWinLoaderData GstWinLoaderData;

static GstElementClass *parent_class = NULL;
//static guint gst_windec_signals[LAST_SIGNAL] = { 0 };

GType
gst_windec_get_type (void) 
{
  static GType windec_type = 0;

  if (!windec_type) {
    static const GTypeInfo windec_info = {
      sizeof(GstWinDecClass),      
      NULL,
      NULL,
      (GClassInitFunc) gst_windec_class_init,
      NULL,
      NULL,
      sizeof(GstWinDec),
      0,
      (GInstanceInitFunc) gst_windec_init,
      NULL
    };
    windec_type = g_type_register_static (GST_TYPE_ELEMENT, "GstWinDec", &windec_info, (GTypeFlags)0);
  }

  return windec_type;
}

static void
gst_windec_class_init (GstWinDec *klass) 
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)klass;

  parent_class = GST_ELEMENT_CLASS (g_type_class_ref (GST_TYPE_ELEMENT));

  gobject_class->set_property = gst_windec_set_property;
  gobject_class->get_property = gst_windec_get_property;

  gstelement_class->change_state = gst_windec_change_state;
}

static GstCaps* 
gst_windec_bh_to_caps (BITMAPINFOHEADER *bh) 
{
    GstCaps *caps;
    gulong compression;
    gint bitCount;
    gint red_mask, green_mask, blue_mask;

    if (bh->biCompression == 0) 
      compression = GST_MAKE_FOURCC ('R','G','B',' ');
    else
      compression = bh->biCompression;

    if (compression != GST_MAKE_FOURCC ('R','G','B',' ')) {
      caps = GST_CAPS_NEW (
	       "windec_caps",
	       "video/raw",
		 "format",    GST_PROPS_FOURCC (compression),
	         "width",      GST_PROPS_INT (bh->biWidth),
	         "height",     GST_PROPS_INT (ABS (bh->biHeight))
	     );
    }
    else {
      bitCount = bh->biBitCount;

      switch (bitCount) {
        case 15:
          red_mask   = 0x7c00;
          green_mask = 0x03e0;
          blue_mask  = 0x001f;
	  break;
        case 16:
          red_mask   = 0xf800;
          green_mask = 0x07e0;
          blue_mask  = 0x001f;
	  break;
        case 32:
        case 24:
          red_mask   = 0x00ff0000;
          green_mask = 0x0000ff00;
          blue_mask  = 0x000000ff;
	  break;
        default:
	  g_warning ("gstwindec: unknown bitCount found\n");
	  red_mask = green_mask = blue_mask = 0;
	  break;
      }

      caps = GST_CAPS_NEW (
		    "windec_caps",
		    "video/raw",
	   	      "format",  GST_PROPS_FOURCC (compression),
	              "bpp",        GST_PROPS_INT (bitCount),
	              "depth",      GST_PROPS_INT (bitCount),
		      "endianness", GST_PROPS_INT (G_LITTLE_ENDIAN),
		      "red_mask",   GST_PROPS_INT (red_mask),
		      "green_mask", GST_PROPS_INT (green_mask),
		      "blue_mask",  GST_PROPS_INT (blue_mask),
	              "width",      GST_PROPS_INT (bh->biWidth),
	              "height",     GST_PROPS_INT (ABS (bh->biHeight))
		   );
    }
    return caps;
}

static GstPadLinkReturn 
gst_windec_sinkconnect (GstPad *pad, GstCaps *caps) 
{
  GstWinDec *windec;
  gint planes, bit_count;
  guint32 compression;
  gint temp_val;

  windec = GST_WINDEC (gst_pad_get_parent (pad));

  if (!GST_CAPS_IS_FIXED (caps))
    return GST_PAD_LINK_DELAYED;

  gst_caps_get_int 	  (caps, "size", 	&temp_val);
  windec->bh.biSize = temp_val;
  gst_caps_get_int 	  (caps, "width", 	&temp_val);
  windec->bh.biWidth = temp_val;
  gst_caps_get_int 	  (caps, "height", 	&temp_val);
  windec->bh.biHeight = temp_val;
  gst_caps_get_int 	  (caps, "planes", 	&planes);
  gst_caps_get_int 	  (caps, "bit_cnt", 	&bit_count);
  gst_caps_get_fourcc_int (caps, "compression", &compression);
  gst_caps_get_int 	  (caps, "image_size", 	&temp_val);
  windec->bh.biSizeImage = temp_val;
  gst_caps_get_int 	  (caps, "xpels_meter", &temp_val);
  windec->bh.biXPelsPerMeter = temp_val;
  gst_caps_get_int 	  (caps, "ypels_meter", &temp_val);
  windec->bh.biYPelsPerMeter = temp_val;
  gst_caps_get_int 	  (caps, "num_colors", 	&temp_val);
  windec->bh.biClrUsed = temp_val;
  gst_caps_get_int 	  (caps, "imp_colors", 	&temp_val);
  windec->bh.biClrImportant = temp_val;

  windec->bh.biPlanes      = planes;
  windec->bh.biBitCount    = bit_count;
  windec->bh.biCompression = compression;

  windec->decoder = Creators::CreateVideoDecoder (windec->bh);
  if (windec->decoder) {
    GstPad *peer;

    windec->decoder->Start ();
#if AVIFILE_MAJOR_VERSION == 0 && AVIFILE_MINOR_VERSION >= 7
    windec->obh = windec->decoder->GetDestFmt();
#else
    windec->obh = windec->decoder->DestFmt();
#endif

    GST_INFO (GST_CAT_NEGOTIATION, "caps: %d format %4.4s", 
    			windec->decoder->GetCapabilities(),
    			(char *)&windec->obh.biCompression);

    peer = gst_pad_get_peer (windec->srcpad);
    if (peer) {
      return gst_windec_srcconnect (windec->srcpad, gst_pad_get_allowed_caps (windec->srcpad));
    }
    return GST_PAD_LINK_OK;
  }
  return GST_PAD_LINK_REFUSED;
}

static GstPadLinkReturn 
gst_windec_srcconnect (GstPad *pad, GstCaps *caps) 
{
  GstWinDec *windec;
  IVideoDecoder::CAPS decoder_caps;
  GstCaps *peercaps;

  windec = GST_WINDEC (gst_pad_get_parent (pad));

  /* we cannot do this without a decoder */
  if (!windec->decoder)
    return GST_PAD_LINK_DELAYED;

  decoder_caps = windec->decoder->GetCapabilities();

  peercaps = gst_caps_intersect (caps,
  			GST_CAPS_NEW (
			  "windec_filter",
			  "video/raw",
			    "width",  GST_PROPS_INT (windec->bh.biWidth),
			    "height", GST_PROPS_INT (windec->bh.biHeight)
			));

  peercaps = gst_caps_normalize (peercaps);
  gst_caps_debug (peercaps, "normalized caps, within gst_windec_srcconnect()");

  /* loop over the peer caps and find something that is supported 
   * by avifile */
  while (peercaps) {
    guint32 fourcc;
    gint bit_count = 0;
    gboolean try_fourcc = FALSE;

    if (gst_caps_has_property (peercaps, "format")) {
      gst_caps_get_fourcc_int (peercaps, "format", &fourcc);
    }
    else {
      /* peer doesn't tell us the format, pick one we can handle */
      if (decoder_caps & IVideoDecoder::CAP_YUY2) 
        fourcc = GST_STR_FOURCC ("YUY2");
      else if (decoder_caps & IVideoDecoder::CAP_YV12) 
        fourcc = GST_STR_FOURCC ("YV12");
      else if (decoder_caps & IVideoDecoder::CAP_I420) 
        fourcc = GST_STR_FOURCC ("I420");
      else 	
        fourcc = GST_STR_FOURCC ("RGB ");
    }

    switch (fourcc) {
      case GST_MAKE_FOURCC ('R','G','B',' '):
        try_fourcc = TRUE;
	if (gst_caps_has_property (peercaps, "depth")) {
	  gst_caps_get_int (peercaps, "depth", &bit_count);
	}
	else {
	  bit_count = 24;
	}
	fourcc = 0;  /* looks like RGB isn't good enough for avifile */
	break;
      case GST_MAKE_FOURCC ('Y','U','Y','2'):
        if (decoder_caps & IVideoDecoder::CAP_YUY2) {
          try_fourcc = TRUE;
	  bit_count = 0;
	}
	break;
      case GST_MAKE_FOURCC ('Y','V','1','2'):
        if (decoder_caps & IVideoDecoder::CAP_YV12) {
          try_fourcc = TRUE;
	  bit_count = 0;
	}
      case GST_MAKE_FOURCC ('I','4','2','0'):
        if (decoder_caps & IVideoDecoder::CAP_I420) {
          try_fourcc = TRUE;
	  bit_count = 0;
	}
	break;
    }
    /* it's something we can try to send to avifile */
    if (try_fourcc) {
      int result = -1;

      GST_DEBUG (GST_CAT_NEGOTIATION, "trying fourcc \"%4.4s\", bit_count %d\n", 
      				(char *)&fourcc, bit_count);

      result = windec->decoder->SetDestFmt (bit_count, fourcc);
      if (result >= 0) {
        GstCaps *setcaps;
	
        windec->obh.biCompression = fourcc;
        windec->obh.biBitCount = bit_count;

	setcaps = gst_windec_bh_to_caps (&windec->obh);

        if (gst_pad_try_set_caps(windec->srcpad, setcaps) > 0) {
          return GST_PAD_LINK_OK;
	}
      }
      else {
        GST_DEBUG (GST_CAT_NEGOTIATION, "codec didn't accept \"%4.4s\", bit_count %d", 
			(char *)&fourcc, bit_count);
      }
    }
    
    peercaps = peercaps->next;
  }

  return GST_PAD_LINK_REFUSED;
}


static void 
gst_windec_init (GstWinDec *windec) 
{
  GST_DEBUG (0,"gst_windec_init: initializing");
  /* create the sink and src pads */
  windec->sinkpad = gst_pad_new_from_template (wincodec_sink_temp, "sink");
  gst_element_add_pad (GST_ELEMENT (windec), windec->sinkpad);
  gst_pad_set_chain_function (windec->sinkpad, gst_windec_chain);
  gst_pad_set_link_function (windec->sinkpad, gst_windec_sinkconnect);

  windec->srcpad = gst_pad_new_from_template (wincodec_src_temp, "src");
  gst_element_add_pad (GST_ELEMENT (windec), windec->srcpad);
  gst_pad_set_link_function (windec->srcpad, gst_windec_srcconnect);

  windec->decoder = NULL;
  windec->pool = NULL;
}

static void 
gst_windec_chain (GstPad *pad, GstBuffer *buf)
{
  GstWinDec *windec;
  GstBuffer *outbuf;

  g_return_if_fail(buf != NULL);
  if (GST_BUFFER_DATA(buf) == NULL) return;

  windec = GST_WINDEC (gst_pad_get_parent (pad));

  if (!GST_PAD_IS_LINKED(windec->srcpad)) {
    GST_DEBUG (0,"gst_windec: src pad not connected");
    gst_buffer_unref(buf);
    return;
  }

  outbuf = NULL;
  if (windec->pool) {
    outbuf = gst_buffer_new_from_pool (windec->pool, 0, 0);
  }

  windec->decoder->DecodeFrame ((void*)GST_BUFFER_DATA (buf), (int)GST_BUFFER_SIZE (buf), 
  				GST_BUFFER_TIMESTAMP (buf), 0, FALSE);

  CImage *img = windec->decoder->GetFrame();

  if (!outbuf) {
    outbuf = gst_buffer_new ();
    GST_BUFFER_SIZE (outbuf) = img->Bytes();
    GST_BUFFER_DATA (outbuf) = (guchar *) g_malloc (GST_BUFFER_SIZE (outbuf));
  }
  if (img->Direction()) {
    g_warning ("FIXME: turn your monitor upside down please :)");
  }

  memcpy (GST_BUFFER_DATA (outbuf), img->Data(), img->Bytes());
  img->Release();

  GST_BUFFER_TIMESTAMP (outbuf) = GST_BUFFER_TIMESTAMP (buf);

  GST_DEBUG (0,"gst_windec: pushing buffer %llu", GST_BUFFER_TIMESTAMP (outbuf));
  gst_pad_push (windec->srcpad, outbuf);
  GST_DEBUG (0,"gst_windec: pushed buffer");

  gst_buffer_unref (buf);
}

static GstElementStateReturn
gst_windec_change_state (GstElement *element)
{ 
  GstWinDec *windec;

  windec = GST_WINDEC (element);
  
  switch (GST_STATE_TRANSITION (element)) {
    case GST_STATE_PAUSED_TO_PLAYING:
      break;
    case GST_STATE_PLAYING_TO_PAUSED:
      if (windec->pool)
        gst_buffer_pool_unref (windec->pool);
      windec->pool = NULL;
      break;
  }

  parent_class->change_state (element);

  return GST_STATE_SUCCESS;
}

static void 
gst_windec_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  GstWinDec *windec;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail(GST_IS_WINDEC(object));
  windec = GST_WINDEC(object);

  switch(prop_id) {
    default:
      break;
  }
}

static void 
gst_windec_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GstWinDec *windec;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail(GST_IS_WINDEC(object));
  windec = GST_WINDEC(object);

  switch(prop_id) {
    default:
      break;
  }
}


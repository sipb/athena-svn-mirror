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
#include "gstrtpsend.h"
#include "gstrtp-common.h"

/* elementfactory information */
static GstElementDetails gst_rtp_send_details = {
  "RTP Sink",
  "RtpSend",
  "GPL",
  "Streams data via RTP",
  VERSION,
  "Zaheer Merali <zaheer@bellworldwide.net>, " "Zeeshan Ali <zak147@yahoo.com",
  "(C) 2001",
};

/* RtpSend signals and args */
enum
{
  /* FILL ME */
  NEW_CAPS,
  SOCKET_CLOSED,
  LAST_SIGNAL
};

enum
{
  ARG_0,
  ARG_IP,
  ARG_PORT,
  ARG_MEDIA_TYPE,
  ARG_MTU
};

GST_PAD_TEMPLATE_FACTORY (sink_factory,
		"sink",
		GST_PAD_SINK,
	        GST_PAD_ALWAYS,
	 	GST_CAPS_NEW (
			"gsm_gsm",
	       		"audio/x-gsm",
	       		"rate", 	GST_PROPS_INT_RANGE (1000, 48000)),
	 	GST_CAPS_NEW (
			"mp3", 
			"audio/x-mp3", 
			"layer",	GST_PROPS_INT_RANGE (1, 3), 
			"bitrate",	GST_PROPS_INT_RANGE (8, 320), 
			"framed",	GST_PROPS_BOOLEAN (TRUE)),
		GST_CAPS_NEW (
			"mpeg_video", 
			"video/mpeg", 
			NULL ),
			/*"mpegversion",      GST_PROPS_INT_RANGE (1, 2),
			 *"systemstream",     GST_PROPS_BOOLEAN (FALSE),
			 *"sliced",           GST_PROPS_BOOLEAN (TRUE) ),
			 */
		GST_CAPS_NEW (
			"audio_raw",
			"audio/raw",
			"format",	GST_PROPS_STRING ("int"),
			"law", 		GST_PROPS_INT (0),
			"endianness", 	GST_PROPS_INT (G_BYTE_ORDER), 
			"signed",	GST_PROPS_BOOLEAN (TRUE), 
			"width",	GST_PROPS_INT (16), 
			"depth",	GST_PROPS_INT (16), 
			"rate",		GST_PROPS_INT_RANGE (1000, 48000),
			"channels", 	GST_PROPS_INT (1))
)

static void gst_rtpsend_class_init (GstRtpSendClass * klass);
static void gst_rtpsend_init (GstRtpSend * rtpsend);

static void gst_rtpsend_chain (GstPad * pad, GstBuffer * buf);

static void gst_rtpsend_set_property (GObject * object, guint prop_id,
				   const GValue * value, GParamSpec * pspec);
static void gst_rtpsend_get_property (GObject * object, guint prop_id,
				   GValue * value, GParamSpec * pspec);
static void gst_rtpsend_set_clock (GstElement *element, GstClock *clock);

static GstPadLinkReturn gst_rtpsend_sinkconnect (GstPad * pad, GstCaps * caps);
static GstElementStateReturn gst_rtpsend_change_state (GstElement * element);

static GstElementClass *parent_class = NULL;
static guint gstrtpsend_signals[LAST_SIGNAL] = { 0 };

static GType gst_rtpsend_get_type (void)
{
  static GType rtpsend_type = 0;

  if (!rtpsend_type) {
    static const GTypeInfo rtpsend_info = {
      sizeof (GstRtpSendClass),
      NULL,
      NULL,
      (GClassInitFunc) gst_rtpsend_class_init,
      NULL,
      NULL,
      sizeof (GstRtpSend),
      0,
      (GInstanceInitFunc) gst_rtpsend_init,
    };

    rtpsend_type = g_type_register_static (GST_TYPE_ELEMENT, "GstRtpSend", &rtpsend_info, 0);
  }
  return rtpsend_type;
}

static void
gst_rtpsend_class_init (GstRtpSendClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_IP, 
		  g_param_spec_string ("ip", "ip", "ip", NULL, 
			  G_PARAM_READWRITE));	/* CHECKME */
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_PORT, 
		  g_param_spec_int ("port", "port", "port", G_MININT, G_MAXINT,
			  0, G_PARAM_READWRITE)); /* CHECKME */
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_MEDIA_TYPE, 
		  g_param_spec_string ("media_type", "media_type", 
			  "media_type", NULL, G_PARAM_READABLE)); /* CHECKME */
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_MTU, 
		  g_param_spec_int ("mtu", "mtu", "mtu", G_MININT, G_MAXINT, 
			  0, G_PARAM_READWRITE)); /* CHECKME */

  /* Registering Signals... */
  gstrtpsend_signals[NEW_CAPS] =
    g_signal_new ("new_caps", G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (GstRtpSendClass, new_caps), NULL,
		  NULL, g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, 
		  G_TYPE_POINTER);
  gstrtpsend_signals[SOCKET_CLOSED] =
    g_signal_new ("socket_closed", G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (GstRtpSendClass, socket_closed), NULL,
		  NULL, g_cclosure_marshal_VOID__INT, G_TYPE_NONE, 1,
		  G_TYPE_INT);

  gobject_class->set_property = gst_rtpsend_set_property;
  gobject_class->get_property = gst_rtpsend_get_property;

  gstelement_class->change_state = gst_rtpsend_change_state;
  gstelement_class->set_clock = gst_rtpsend_set_clock;

  klass->new_caps = NULL;
  klass->socket_closed = NULL;
}

static void
gst_rtpsend_set_clock (GstElement *element, GstClock *clock)
{
  GstRtpSend *rtpsend;
	      
  rtpsend = GST_RTPSEND (element);

  rtpsend->clock = clock;
}

static void
gst_rtpsend_init (GstRtpSend * rtpsend)
{
  rtpsend->sinkpad = gst_pad_new_from_template (GST_PAD_TEMPLATE_GET (sink_factory), "sink");
  gst_element_add_pad (GST_ELEMENT (rtpsend), rtpsend->sinkpad);
  gst_pad_set_chain_function (rtpsend->sinkpad, gst_rtpsend_chain);
  gst_pad_set_link_function (rtpsend->sinkpad, gst_rtpsend_sinkconnect);

  rtpsend->payload_type = PAYLOAD_GSM;

  rtpsend->ip = g_strdup ("127.0.0.1");	
  rtpsend->port = 8000;
  rtpsend->mtu = 160;

  rtpsend->clock = NULL;

}

static GstPadLinkReturn
gst_rtpsend_sinkconnect (GstPad * pad, GstCaps * caps)
{
  GstRtpSend *rtpsend;
  const gchar *mime = gst_caps_get_mime (caps);

  g_print ("new caps called with caps' mime type: %s\n", mime);
  rtpsend = GST_RTPSEND (gst_pad_get_parent (pad));

  if (strcmp (mime, "audio/x-gsm") == 0) {
    rtpsend->payload_type = PAYLOAD_GSM;
    rtpsend->mtu = 160;
  }

  else if (strcmp (mime, "audio/x-mp3") == 0) {
    rtpsend->payload_type = PAYLOAD_MPA;
    rtpsend->mtu = 512;
  }

  else if (strcmp (mime, "video/mpeg") == 0) {
    gboolean systemstream;

    gst_caps_get_boolean (caps, "systemstream", &systemstream);

    if (systemstream == TRUE)
       rtpsend->payload_type = PAYLOAD_BMPEG;
    else
       rtpsend->payload_type = PAYLOAD_MPV;

    rtpsend->mtu = 1024;
  }

  else if (strcmp (mime, "audio/raw") == 0) {
      gint law, width, channels;
      gboolean is_signed;

      gst_caps_get_int (caps, "law", &law);
      gst_caps_get_int (caps, "width", &width);
      gst_caps_get_int (caps, "channels", &channels);
      gst_caps_get_boolean (caps, "signed", &is_signed);

      if (law == 0)
        if (width == 16)
          if (channels == 1)
            if (is_signed == TRUE) {
                rtpsend->payload_type = PAYLOAD_L16_MONO;
                rtpsend->mtu = 1024;
            }
  }

  /* FILL ME  */

  /* Unrecognised Type */
  else {
    rtpsend->payload_type = 0;
    g_warning ("Unsupported media type\n");
  }

  /* Emit a signal */
  g_signal_emit (rtpsend, gstrtpsend_signals[NEW_CAPS], 0, caps);

  return GST_PAD_LINK_OK;
}


static void
gst_rtpsend_chain (GstPad * pad, GstBuffer * buf)
{
  GstRtpSend *rtpsend;
  GstClockTimeDiff *jitter = NULL;
  /*glong ret; */
  guint32 i;

  g_return_if_fail (pad != NULL);
  g_return_if_fail (GST_IS_PAD (pad));
  g_return_if_fail (buf != NULL);

  rtpsend = GST_RTPSEND (GST_OBJECT_PARENT (pad));

  g_return_if_fail (rtpsend != NULL);
  g_return_if_fail (GST_IS_RTPSEND (rtpsend));

  if (rtpsend->clock) {
    GST_DEBUG (0, "rtpsend: clock wait: %llu\n", GST_BUFFER_TIMESTAMP (buf));
    gst_element_clock_wait (GST_ELEMENT (rtpsend), rtpsend->clock, GST_BUFFER_TIMESTAMP (buf), jitter);
  }

  for (i = 0; i < GST_BUFFER_SIZE (buf); i += rtpsend->mtu) {
    if (GST_BUFFER_SIZE (buf) - i > rtpsend->mtu) {
      rtp_send (&rtpsend->conn, GST_BUFFER_DATA (buf) + i, rtpsend->mtu, rtpsend->payload_type, 20);
    }
    else {
      rtp_send (&rtpsend->conn, GST_BUFFER_DATA (buf) + i,
		GST_BUFFER_SIZE (buf) - i, rtpsend->payload_type, 20);
    }
  }
  /*g_print( "Sent\n" ); */
  /*
     if (!ret) {
     g_warning ("rtp send error...\n");
     }
   */

  gst_buffer_unref (buf);
}

static void
gst_rtpsend_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstRtpSend *rtpsend;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_RTPSEND (object));
  rtpsend = GST_RTPSEND (object);

  switch (prop_id) {
    case ARG_IP:
      if (rtpsend->ip) {
	g_free (rtpsend->ip);
      }
      
      rtpsend->ip = g_strdup (g_value_get_string (value));
      break;
    case ARG_PORT:
      rtpsend->port = g_value_get_int (value);
      break;
    case ARG_MTU:
      rtpsend->mtu = g_value_get_int (value);
      break;
    default:
      break;
  }
}

static void
gst_rtpsend_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstRtpSend *rtpsend;
  const gchar *media_type;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_RTPSEND (object));
  rtpsend = GST_RTPSEND (object);

  switch (prop_id) {
    case ARG_IP:
      g_value_set_string (value, rtpsend->ip);
      break;
    case ARG_PORT:
      g_value_set_int (value, rtpsend->port);
      break;
    case ARG_MEDIA_TYPE:
      media_type = caps_to_mediatype (gst_pad_get_caps (rtpsend->sinkpad));
      g_value_set_string (value, media_type);
      break;
    case ARG_MTU:
      g_value_set_int (value, rtpsend->mtu);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static GstElementStateReturn
gst_rtpsend_change_state (GstElement * element)
{
  GstRtpSend *rtpsend;
  guint8 ret_val = 0;

  g_return_val_if_fail (GST_IS_RTPSEND (element), GST_STATE_FAILURE);

  rtpsend = GST_RTPSEND (element);

  GST_DEBUG (0, "state pending %d\n", GST_STATE_PENDING (element));

  /* if going down into NULL state, close the file if it's open */
  switch (GST_STATE_TRANSITION (element)) {
    case GST_STATE_NULL_TO_READY:
      rtp_client_connection (&rtpsend->conn);

      ret_val =  rtp_connection_call (&rtpsend->conn, rtpsend->ip, rtpsend->port);

      if (ret_val) {
	g_print ("initialised RTP successfully...\n");
	/* Emit a message */
	g_signal_emit (G_OBJECT (rtpsend), gstrtpsend_signals[NEW_CAPS],
		       0, gst_pad_get_caps (rtpsend->sinkpad));
      }
      
      else {
	g_warning ("couldn't initialise RTP...\n");
	return GST_STATE_FAILURE;
      }
      
      break;

    case GST_STATE_READY_TO_NULL:
      /*shout_unlink (&icecastsend->conn); */
	/* Emit a message */
	g_signal_emit (G_OBJECT (rtpsend), gstrtpsend_signals[SOCKET_CLOSED],
		       0, rtpsend->conn.packets_sent);
      close (rtpsend->conn.rtp_sock);
      g_free (rtpsend->conn.hostname);
      break;

    default:
      break;
  }

  /* if we haven't failed already, give the parent class a chance to ;-) */
  if (GST_ELEMENT_CLASS (parent_class)->change_state)
    return GST_ELEMENT_CLASS (parent_class)->change_state (element);

  return GST_STATE_SUCCESS;
}

gboolean
gst_rtpsend_plugin_init (GModule * module, GstPlugin * plugin)
{
  GstElementFactory *send;

  send = gst_element_factory_new ("rtpsend", GST_TYPE_RTPSEND, &gst_rtp_send_details);
  g_return_val_if_fail (send != NULL, FALSE);

  gst_element_factory_add_pad_template (send, GST_PAD_TEMPLATE_GET (sink_factory));

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (send));

  return TRUE;
}

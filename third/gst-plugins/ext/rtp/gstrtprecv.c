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
 * Library General Public License for more 
 */

#include <string.h>
#include "gstrtprecv.h"
#include "gstrtp-common.h"

/* elementfactory information */
static GstElementDetails gst_rtp_recv_details = {
  "RTP Source",
  "RtpRecv",
  "GPL",
  "Streams data via RTP",
  VERSION,
  "Zeeshan Ali <zak147@yahoo.com>",
  "(C) 2001",
};

/* RtpRecv signals and args */
enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  ARG_0,
  ARG_PORT,
  ARG_MTU,
  ARG_CAPS,
  ARG_SOCKET_CLOSED,
  ARG_MEDIA_TYPE
};

GST_PAD_TEMPLATE_FACTORY (src_factory,
 	"src",
	GST_PAD_SRC,
	GST_PAD_ALWAYS,
	GST_CAPS_NEW (
		"gsm_gsm",
		"audio/x-gsm",
        	"rate", 	GST_PROPS_INT_RANGE (1000, 48000)),
	GST_CAPS_NEW (
		"mp3", 
		"audio/x-mp3", 
		NULL ),
        	/*"layer",      GST_PROPS_INT_RANGE (1, 3),
	         *"bitrate",    GST_PROPS_INT_RANGE (8, 320),
	         *"framed",     GST_PROPS_BOOLEAN (TRUE) ),
		 */
	GST_CAPS_NEW (
		 "mpeg_video",
		 "video/mpeg",
		 "mpegversion",  GST_PROPS_INT_RANGE (1, 2)),
	         /*"systemstream", GST_PROPS_BOOLEAN (FALSE)),*/
		 /*"sliced",     GST_PROPS_BOOLEAN (TRUE)), */
	GST_CAPS_NEW (
		 "audio_raw",
		 "audio/raw",
	         "format", 	GST_PROPS_STRING ("int"),
		 "law", 	GST_PROPS_INT (0),
		 "endianness",  GST_PROPS_INT (G_BYTE_ORDER), 
		 "signed", 	GST_PROPS_BOOLEAN (TRUE), 
		 "width", 	GST_PROPS_INT (16), 
		 "depth",	GST_PROPS_INT (16), 
		 "rate",	GST_PROPS_INT_RANGE (1000, 48000),
		 "channels", 	GST_PROPS_INT (1))
)
 
static void gst_rtprecv_class_init (GstRtpRecvClass * klass);
static void gst_rtprecv_init (GstRtpRecv * rtprecv);

static GstBuffer *gst_rtprecv_get (GstPad * pad);

static void gst_rtprecv_set_property (GObject * object, guint prop_id,
				   const GValue * value, GParamSpec * pspec);
static void gst_rtprecv_get_property (GObject * object, guint prop_id,
				   GValue * value, GParamSpec * pspec);
static void gst_rtprecv_set_clock (GstElement *element, GstClock *clock);

static GstElementStateReturn gst_rtprecv_change_state (GstElement * element);

static GstElementClass *parent_class = NULL;

static GType gst_rtprecv_get_type (void)
{
  static GType rtprecv_type = 0;

  if (!rtprecv_type) {
    static const GTypeInfo rtprecv_info = {
      sizeof (GstRtpRecvClass),
      NULL,
      NULL,
      (GClassInitFunc) gst_rtprecv_class_init,
      NULL,
      NULL,
      sizeof (GstRtpRecv),
      0,
      (GInstanceInitFunc) gst_rtprecv_init,
    };

    rtprecv_type = g_type_register_static (GST_TYPE_ELEMENT, "GstRtpRecv", &rtprecv_info, 0);
  }
  return rtprecv_type;
}

static void
gst_rtprecv_class_init (GstRtpRecvClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_PORT, 
		g_param_spec_int ("port", "port", "port", G_MININT, G_MAXINT, 
					0, G_PARAM_READWRITE));	/* CHECKME */
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_MTU, 
		g_param_spec_int ("mtu", "mtu", "mtu", G_MININT, G_MAXINT, 
					0, G_PARAM_READWRITE));	/* CHECKME */
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_CAPS, 
		g_param_spec_pointer ("caps", "caps", "caps", 
					G_PARAM_READWRITE));	/* CHECKME */
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_SOCKET_CLOSED, 
		g_param_spec_boolean ("socket_closed", "socket_closed", 
			"socket_closed", 0, G_PARAM_WRITABLE));	/* CHECKME */
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_MEDIA_TYPE, 
		g_param_spec_string ("media_type", "media_type", "media_type", 
			NULL, G_PARAM_READWRITE));	/* CHECKME */

  gobject_class->set_property = gst_rtprecv_set_property;
  gobject_class->get_property = gst_rtprecv_get_property;

  gstelement_class->change_state = gst_rtprecv_change_state;
  gstelement_class->set_clock = gst_rtprecv_set_clock;
}

static void
gst_rtprecv_set_clock (GstElement *element, GstClock *clock)
{
  GstRtpRecv *rtprecv;
	      
  rtprecv = GST_RTPRECV (element);

  rtprecv->clock = clock;
}

static void
gst_rtprecv_init (GstRtpRecv * rtprecv)
{
  rtprecv->srcpad = gst_pad_new_from_template (GST_PAD_TEMPLATE_GET (src_factory), "src");
  gst_element_add_pad (GST_ELEMENT (rtprecv), rtprecv->srcpad);
  gst_pad_set_get_function (rtprecv->srcpad, (GstPadGetFunction) gst_rtprecv_get);

  rtprecv->port = 8000;
  rtprecv->mtu = 160;
  rtprecv->clock = NULL;

}

static GstBuffer *
gst_rtprecv_get (GstPad * pad)
{
  GstRtpRecv *rtprecv;

  /*glong ret; */
  GstBuffer *outbuf = NULL;
  fd_set read_fds;
  struct timeval timeout;

  g_return_val_if_fail (pad != NULL, NULL);
  g_return_val_if_fail (GST_IS_PAD (pad), NULL);

  rtprecv = GST_RTPRECV (GST_OBJECT_PARENT (pad));

  g_return_val_if_fail (rtprecv != NULL, NULL);
  g_return_val_if_fail (GST_IS_RTPRECV (rtprecv), NULL);

  FD_ZERO (&read_fds);

  while(!outbuf) { 
    FD_SET (rtprecv->conn.rtp_sock, &read_fds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;
    
   if (select (rtprecv->conn.rtp_sock + 1, &read_fds, NULL, NULL, &timeout) >= 0) {
      if (FD_ISSET (rtprecv->conn.rtp_sock, &read_fds)) {
        Rtp_Packet packet;
        rtp_payload_t pt;

        //while( ( packet = rtp_receive (&rtprecv->conn, rtprecv->mtu) ) == NULL );
        packet = rtp_receive (&rtprecv->conn, rtprecv->mtu);

        pt = rtp_packet_get_payload_type (packet);

        if (pt != rtprecv->payload_type) {
	  g_warning ("new payload_t %d recieved, no caps-nego?\n", pt);
      	  rtprecv->payload_type = pt;
        }

        outbuf = gst_buffer_new ();
        GST_BUFFER_SIZE (outbuf) = rtp_packet_get_payload_len (packet);
        GST_BUFFER_DATA (outbuf) = g_malloc (GST_BUFFER_SIZE (outbuf));

        memcpy (GST_BUFFER_DATA (outbuf), rtp_packet_get_payload (packet), GST_BUFFER_SIZE (outbuf));
  
	if (rtprecv->clock) {
    	    GST_BUFFER_TIMESTAMP (outbuf) = gst_clock_get_time (rtprecv->clock);
  	}
        
	rtp_packet_free (packet);
      }
      
      else {	 
	 gst_element_interrupt (GST_ELEMENT (rtprecv));
      }
    }
    else {
      perror ("select");
    }
  }
  
   return outbuf;
}


static void
gst_rtprecv_set_property (GObject * object, guint prop_id, const GValue * value, GParamSpec * pspec)
{
  GstRtpRecv *rtprecv;
  GstCaps *caps;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_RTPRECV (object));
  rtprecv = GST_RTPRECV (object);

  switch (prop_id) {
    case ARG_PORT:
      rtprecv->port = g_value_get_int (value);
      break;
    case ARG_MTU:
      rtprecv->mtu = g_value_get_int (value);
      break;
    case ARG_CAPS:
      caps = GST_CAPS (g_value_get_pointer (value));
      gst_pad_try_set_caps (rtprecv->srcpad, caps);
      break;
    case ARG_SOCKET_CLOSED:
      /* wtay: add something suitable here */
      break;
    case ARG_MEDIA_TYPE:
      caps = mediatype_to_caps ((gchar *)g_value_get_string (value), 
		      		 &rtprecv->payload_type, &rtprecv->mtu);
      if( caps != NULL ) {
      	   gst_pad_try_set_caps (rtprecv->srcpad, caps);
      }
      break;
    default:
      break;
  }
}

static void
gst_rtprecv_get_property (GObject * object, guint prop_id, GValue * value, GParamSpec * pspec)
{
  GstRtpRecv *rtprecv;
  const gchar *media_type;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_RTPRECV (object));
  rtprecv = GST_RTPRECV (object);

  switch (prop_id) {
    case ARG_PORT:
      g_value_set_int (value, rtprecv->port);
      break;
    case ARG_MTU:
      g_value_set_int (value, rtprecv->mtu);
      break;
    case ARG_CAPS:
      g_value_set_pointer (value, gst_pad_get_caps (rtprecv->srcpad));
      break;
    case ARG_MEDIA_TYPE:
      media_type = caps_to_mediatype (gst_pad_get_caps (rtprecv->srcpad));
      g_value_set_string (value, media_type);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static GstElementStateReturn
gst_rtprecv_change_state (GstElement * element)
{
  GstRtpRecv *rtprecv;

  g_return_val_if_fail (GST_IS_RTPRECV (element), GST_STATE_FAILURE);

  rtprecv = GST_RTPRECV (element);

  GST_DEBUG (0, "state pending %d\n", GST_STATE_PENDING (element));

  /* if going down into NULL state, close the file if it's open */
  switch (GST_STATE_TRANSITION (element)) {

    case GST_STATE_NULL_TO_READY:

      if (rtp_server_init (&rtprecv->conn, rtprecv->port)) {
	g_print ("initialised RTP successfully...\n");
      }
      else {
	g_warning ("couldn't initialise RTP...\n");
	return GST_STATE_FAILURE;
      }

      rtp_server_connection (&rtprecv->conn);
      break;

    case GST_STATE_READY_TO_NULL:
      /*shout_unlink (&icecastrecv->conn); */
      close (rtprecv->conn.rtp_sock);
      rtprecv->conn.rtp_sock = 0;
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
gst_rtprecv_plugin_init (GModule * module, GstPlugin * plugin)
{
  GstElementFactory *recv;

  recv = gst_element_factory_new ("rtprecv", GST_TYPE_RTPRECV, &gst_rtp_recv_details);
  g_return_val_if_fail (recv != NULL, FALSE);

  gst_element_factory_add_pad_template (recv, GST_PAD_TEMPLATE_GET (src_factory));

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (recv));

  return TRUE;
}

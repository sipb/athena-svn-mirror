#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "rtp-packet.h"
/*#include <rtp/rtp-audio.h>*/

#include "gstrtp-common.h"

#define BUFF_SIZE 10

static const gchar App_Name[] = {	/* Name for APP packets */
  'p', 'v'
};

gint
find_host (gchar * host_name, struct in_addr *address)
{
  struct hostent *host;

  if (inet_aton (host_name, address)) {
    return 1;
  }
  else {
    /* if user has given a hostname not an IP */
    host = gethostbyname (host_name);
  }

  if (!host) {
    g_warning ("error looking up host: %s", host_name);
    return 0;
  }

  memcpy (address, (struct in_addr *) host->h_addr_list[0], sizeof (struct in_addr));

  return 1;
}

void
rtp_send (rtp_sender_info *sen, gpointer buf, int nbytes, rtp_payload_t pt, guint32 nsamp)
{
  Rtp_Packet packet;

  packet = rtp_packet_new_allocate (nbytes, 0, 0);

  rtp_packet_set_csrc_count (packet, 0);
  rtp_packet_set_extension (packet, 0);
  rtp_packet_set_padding (packet, 0);
  rtp_packet_set_version (packet, RTP_VERSION);
  rtp_packet_set_payload_type (packet, (guint8) pt);
  rtp_packet_set_marker (packet, 0);

  g_memmove (rtp_packet_get_payload (packet), buf, nbytes);

  rtp_packet_set_ssrc (packet, g_htonl (sen->ssrc));
  rtp_packet_set_seq (packet, g_htons (sen->seq));
  rtp_packet_set_timestamp (packet, g_htonl (sen->timestamp));

  /*g_print( "sending packet with seq. %d", sen->seq );*/
  sen->addr.sin_port = g_htons (sen->port);
  rtp_packet_send (packet, sen->rtp_sock, (struct sockaddr *) &sen->addr, sizeof (struct sockaddr_in));
  ++sen->seq;
  sen->timestamp += nsamp;
  ++sen->packets_sent;
  sen->octets_sent += rtp_packet_get_packet_len (packet);
  
  rtp_packet_free (packet);
}

Rtp_Packet
rtp_receive (rtp_receiver_info *recv_info, guint32 mtu)
{
  Rtp_Packet packet;
  guint32 packet_size;
  struct sockaddr_in from_addr;
  socklen_t from_len = sizeof (from_addr);

  packet = rtp_packet_read (recv_info->rtp_sock, (struct sockaddr *) &from_addr, &from_len);
  packet_size = rtp_packet_get_payload_len (packet);

  if (packet_size > mtu)
    g_warning ("Recieved RTP Packet's size is larger than MTU\n");

  ++recv_info->packets_sent;
  recv_info->octets_sent += packet_size;

  return packet;
}

/*Rtp_Packet
rtp_receive (rtp_receiver_info * recv_info, guint32 mtu)
{
  * I am including a packet sequencing code of mine, if you dont like
     *  it, plz uncomment the commented code & delete the rest including
     *  all unused varriables. 
   
  static Rtp_Packet packets[BUFF_SIZE];
  static gboolean buffer_full = FALSE;
  static gboolean first_time = TRUE;
  static gboolean missed = FALSE;
  static gint missing = 0;
  static gint last = 0;

  Rtp_Packet packet;
  guint16 seq;
  guint8 i;

  //guint32 packet_size ;			
  //struct sockaddr_in from_addr ;
  //socklen_t from_len = sizeof( from_addr );

  if (!missed) {
    packet = rtp_really_receive (recv_info, mtu);
    seq = g_ntohs (rtp_packet_get_seq (packet));

    //g_print( "no miss %d\n", seq ) ;

    if (first_time) {
      recv_info->seq = seq;
      first_time = FALSE;
      return packet;
    }

    else if (recv_info->seq + 1 != seq) {
      g_print ("seq %d %d\n", recv_info->seq, seq);
      missed = TRUE;
      missing = (recv_info->seq + 1) % BUFF_SIZE;
      last = (recv_info->seq) % BUFF_SIZE;

      for (i = 0; i < BUFF_SIZE; i++)
	packets[i] = NULL;

      packets[seq % BUFF_SIZE] = packet;
      return NULL;
    }

    else {
      recv_info->seq = seq;
      return packet;
    }
  }

  else {
    //g_print( "miss %d %d\n", missing, last ) ;

    if (buffer_full) {
      //g_print( "buffer full\n" ) ;
      if (missing == last) {
	buffer_full = FALSE;
	missed = FALSE;
	recv_info->seq = g_ntohs (rtp_packet_get_seq (packets[last]));
	return packets[last];
      }
      else {
	if (missing >= BUFF_SIZE)
	  missing = 0;
	return packets[missing++];
      }
    }
    else {
      packet = rtp_really_receive (recv_info, mtu);
      seq = g_ntohs (rtp_packet_get_seq (packet));

      if (seq % BUFF_SIZE == last)
	buffer_full = TRUE;

      //g_print( "%d\n", seq ) ;
      packets[seq % BUFF_SIZE] = packet;
      return NULL;
    }
  }

  packet = rtp_packet_read( recv_info->rtp_sock,
     (struct sockaddr *)&from_addr,
     &from_len );
     packet_size = rtp_packet_get_payload_len( packet ) ;

     if( packet_size > mtu )
     g_warning( "Recieved RTP Packet's size is larger than MTU\n" ) ; 

     g_print( "Recieved RTP Packet no.%d\n", 
     g_ntohs( rtp_packet_get_seq( packet ) ) ); 

     ++recv_info->packets_sent;
     recv_info->octets_sent += packet_size ;

     return packet ; 
}*/

void
rtp_client_connection (rtp_sender_info * sen)
{
    g_print ("initializing socket\n");
    
    if ((sen->rtp_sock = socket (PF_INET, SOCK_DGRAM, 0)) < 0) {
      g_print ("open_sockets : Send_Sock");
    }
}

void
rtp_server_connection (rtp_receiver_info * recv)
{
  guint times, ret_val;
	
  /* RTP Socket */
  if ((recv->rtp_sock = socket (PF_INET, SOCK_DGRAM, 0)) < 0) {
    g_warning ("open_sockets : RTP Sock");
  }

  times = 0;

  while (times < 3) {
    if ((ret_val = bind (recv->rtp_sock, (struct sockaddr *) &recv->addr, sizeof (recv->addr))) < 0) {
      g_warning ("could not bind, trying again\n");
    }

    else {
	times = 3;
    }
    
    times++;
  }
}

gint
rtp_connection_call (rtp_sender_info * sen, gchar * hostname, guint port)
{
  struct in_addr call_host;	/* host info of computer to call */
  gint ret_val = 0;

  if ((ret_val = find_host (hostname, &call_host))) {
    g_print ("found host %s\n", hostname);

    sen->addr.sin_family = AF_INET;
    sen->addr.sin_port = g_htons (port);
    g_memmove (&sen->addr.sin_addr.s_addr, &call_host, sizeof (struct in_addr));

    sen->seq = random ();
    sen->ssrc = random ();
    sen->timestamp = random ();
    sen->hostname = g_strdup (hostname);
    sen->port = port;
    sen->packets_sent = 0;
  }
  
  return ret_val;
}

int
rtp_server_init (rtp_receiver_info * recv, guint port)
{
  recv->addr.sin_addr.s_addr = htonl (INADDR_ANY);
  recv->addr.sin_family = AF_INET;
  recv->addr.sin_port = g_htons (port);

  recv->seq = 0;
  recv->ssrc = random ();
  recv->timestamp = random ();
  recv->hostname = g_strdup ("localhost");
  recv->port = port;
  recv->packets_sent = 0;

  return 1;
}

GstCaps*
mediatype_to_caps (gchar *media_type, rtp_payload_t *pt, guint *mtu)
{
  GstCaps *caps = NULL;

  if (strcmp (media_type, "gsm") == 0) {
    *pt = PAYLOAD_GSM;
    caps = GST_CAPS_NEW (
		    "gsm_gsm", 
		    "audio/x-gsm", 
		    "rate", GST_PROPS_INT (8000));
  }

  else if (strcmp (media_type, "mp1") == 0) {
    *pt = PAYLOAD_MPA;
    *mtu = 512;
    caps = GST_CAPS_NEW (
		    "mp3", 
		    "audio/x-mp3", 
		    "layer", GST_PROPS_INT (1));
  }
    
  else if (strcmp (media_type, "mp2") == 0) {
    *pt = PAYLOAD_MPA;
    *mtu = 512;
    caps = GST_CAPS_NEW (
		    "mp3", 
		    "audio/x-mp3", 
		    "layer", GST_PROPS_INT (2));
  }
    
  else if (strcmp (media_type, "mp3") == 0) {
    *pt = PAYLOAD_MPA;
    *mtu = 512;
    caps = GST_CAPS_NEW (
		    "mp3", 
		    "audio/x-mp3", 
		    "layer", GST_PROPS_INT (3));
  }
  
  else if (strcmp (media_type, "mpeg1") == 0) {
    *pt = PAYLOAD_MPV;
    *mtu = 1024;
    caps = GST_CAPS_NEW ("mpeg_video",
			 "video/mpeg",
			 "mpegversion", GST_PROPS_INT (1),
			 "systemstream", GST_PROPS_BOOLEAN (FALSE));
  }

  else if (strcmp (media_type, "mpeg2") == 0) {
    *pt = PAYLOAD_MPV;
    *mtu = 1024;
    caps = GST_CAPS_NEW ("mpeg_video",
			 "video/mpeg",
			 "mpegversion", GST_PROPS_INT (2),
			 "systemstream", GST_PROPS_BOOLEAN (FALSE));
  }

  else if (strcmp (media_type, "mpeg1_sys") == 0) {
    *pt = PAYLOAD_BMPEG;
    *mtu = 1024;
    caps = GST_CAPS_NEW ("mpeg_video",
			 "video/mpeg",
			 "mpegversion", GST_PROPS_INT (1),
			 "systemstream", GST_PROPS_BOOLEAN (TRUE));
  }

  else if (strcmp (media_type, "mpeg2_sys") == 0) {
    *pt = PAYLOAD_BMPEG;
    *mtu = 1024;
    caps = GST_CAPS_NEW ("mpeg_video",
			 "video/mpeg",
			 "mpegversion", GST_PROPS_INT (2),
			 "systemstream", GST_PROPS_BOOLEAN (TRUE));
  }

  else if (strcmp (media_type, "raw_audio") == 0) {
    *pt = PAYLOAD_L16_MONO;
    *mtu = 512;
    caps = GST_CAPS_NEW (
		 "audio_raw",
		 "audio/raw",
	         "format", 	GST_PROPS_STRING ("int"),
		 "law", 	GST_PROPS_INT (0),
		 "endianness",  GST_PROPS_INT (G_BYTE_ORDER), 
		 "signed", 	GST_PROPS_BOOLEAN (TRUE), 
		 "width", 	GST_PROPS_INT (16), 
		 "depth",	GST_PROPS_INT (16), 
		 "rate",	GST_PROPS_INT (8000),
		 "channels", 	GST_PROPS_INT (2));
  }

  return caps;
}

const gchar*
caps_to_mediatype (GstCaps *caps)
{
  const gchar *media_type = NULL;

  if (strcmp (gst_caps_get_mime (caps), "audio/x-gsm") == 0)
    media_type = "gsm";

  else if (strcmp (gst_caps_get_mime (caps), "audio/x-mp3") == 0) {
    gint layer;

    gst_caps_get_int (caps, "layer", &layer);
        if (layer == 1)
      	  media_type = "mp1";
	else if (layer == 2)
	  media_type = "mp2";
	else if (layer == 3)
	  media_type = "mp3";
  }

  else if (strcmp (gst_caps_get_mime (caps), "video/mpeg") == 0) {
    gboolean systemstream;
    gint mpegversion;

    gst_caps_get_boolean (caps, "systemstream", &systemstream);
    gst_caps_get_int (caps, "mpegversion", &mpegversion);

    if (systemstream == FALSE) {
       if (mpegversion == 1)
          media_type = "mpeg1";
       else if (mpegversion == 2)
          media_type = "mpeg2";
    }
    
    else {
        if (mpegversion == 1)
          media_type = "mpeg1_sys";
        else if (mpegversion == 2)
          media_type = "mpeg2_sys";
    }
  }

  else if (strcmp (gst_caps_get_mime (caps), "audio/raw") == 0)
    media_type = "raw_audio";

  return media_type;
}

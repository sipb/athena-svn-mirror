/* Gnome-Streamer
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


#ifndef __GST_RTP_H__
#define __GST_RTP_H__

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <gst/gst.h>

#include "rtp-packet.h"
#include "rtcp-packet.h"
/*#include <rtp/rtp-audio.h>*/

/* We need to define our own payload types & specially for mpeg videos that
* wont look nice if included in files like rtp-audio.h */

typedef enum
{

/* Audio: */
  PAYLOAD_GSM = 3,
  PAYLOAD_L16_MONO = 11,
  PAYLOAD_MPA = 14,		/* Audio MPEG 1-3 */
  PAYLOAD_G723_63 = 16,		/* Not standard */
  PAYLOAD_G723_53 = 17,		/* Not standard */
  PAYLOAD_TS48 = 18,		/* Not standard */
  PAYLOAD_TS41 = 19,		/* Not standard */
  PAYLOAD_G728 = 20,		/* Not standard */
  PAYLOAD_G729 = 21,		/* Not standard */

/* Video: */
  PAYLOAD_MPV = 32,		/* Video MPEG 1 & 2 */

/* BOTH */
  PAYLOAD_BMPEG = 34		/* Not Standard */
}
rtp_payload_t;

typedef struct rtp_info rtp_sender_info;
typedef struct rtp_info rtp_receiver_info;

struct rtp_info
{

  int rtp_sock;
  struct sockaddr_in addr;
  guint32 ssrc;
  guint16 seq;
  guint32 timestamp;
  gchar *hostname;
  guint port;
  guint32 packets_sent;
  guint32 octets_sent;
};

gint find_host (gchar *host_name, struct in_addr *address);

void rtp_send (rtp_sender_info *sen, gpointer buf, int nbytes, rtp_payload_t pt, guint32 nsamp);

Rtp_Packet rtp_receive (rtp_receiver_info *recv, guint32 mtu);

void rtp_client_connection (rtp_sender_info *sen);

void rtp_server_connection (rtp_receiver_info *recv);

gint rtp_connection_call (rtp_sender_info *sen, gchar *hostname, guint port);
gint rtp_server_init (rtp_receiver_info *recv, guint port);

GstCaps *mediatype_to_caps (gchar *media_type, rtp_payload_t *pt, 
					     	guint *mtu );

const gchar *caps_to_mediatype (GstCaps *);

#endif

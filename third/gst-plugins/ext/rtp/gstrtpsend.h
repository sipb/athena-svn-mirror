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


#ifndef __GST_RTPSEND_H__
#define __GST_RTPSEND_H__

#include <gst/gst.h>

#include "gstrtp-common.h"

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */

/* Definition of structure storing data for this element. */
  typedef struct _GstRtpSend GstRtpSend;
  struct _GstRtpSend
  {
    GstElement element;

    GstPad *sinkpad;

    rtp_sender_info conn;

    rtp_payload_t payload_type;

    gchar *ip;
    guint port;
    guint mtu;

    GstClock *clock;
  };

/* Standard definition defining a class for this element. */
  typedef struct _GstRtpSendClass GstRtpSendClass;
  struct _GstRtpSendClass
  {
    GstElementClass parent_class;

    void (*new_caps) (GstElement *send, GstCaps *caps);
    void (*socket_closed) (GstElement *send, guint32 packets_sent);
  };

/* Standard macros for defining types for this element.  */
#define GST_TYPE_RTPSEND \
  (gst_rtpsend_get_type())
#define GST_RTPSEND(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_RTPSEND,GstRtpSend))
#define GST_RTPSEND_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_RTPSEND,GstRtpSend))
#define GST_IS_RTPSEND(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_RTPSEND))
#define GST_IS_RTPSEND_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_RTPSEND))

  gboolean gst_rtpsend_plugin_init (GModule * module, GstPlugin * plugin);

#ifdef __cplusplus
}
#endif				/* __cplusplus */


#endif				/* __GST_RTPSEND_H__ */

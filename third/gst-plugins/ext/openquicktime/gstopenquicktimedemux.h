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


#ifndef __GST_QUCIKTIME_DEMUX_H__
#define __GST_QUICKTIME_DEMUX_H__

#include <config.h>
#include <gst/gst.h>
#include <gst/bytestream/bytestream.h>

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */


#define GST_TYPE_QUICKTIME_DEMUX \
  (gst_quicktime_demux_get_type())
#define GST_QUICKTIME_DEMUX(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_QUICKTIME_DEMUX,GstQuicktimeDemux))
#define GST_QUICKTIME_DEMUX_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_QUICKTIME_DEMUX,GstQuicktimeDemux))
#define GST_IS_QUICKTIME_DEMUX(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_QUICKTIME_DEMUX))
#define GST_IS_QUICKTIME_DEMUX_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_QUICKTIME_DEMUX))


#define GST_QUICKTIME_DEMUX_MAX_AUDIO_PADS	8
#define GST_QUICKTIME_DEMUX_MAX_VIDEO_PADS	8

  typedef struct _GstQuicktimeDemux GstQuicktimeDemux;
  typedef struct _GstQuicktimeDemuxClass GstQuicktimeDemuxClass;

  struct _GstQuicktimeDemux
  {
    GstElement element;

    /* pads */
    GstPad *sinkpad;

    /* File Name */
    gchar *filename;

    /* Quicktime structure */
    void *file;
    GstByteStream *bs;

    /* current timestamp */
    guint64 next_time;
    /* time between 2 frames */
    guint64 time_interval;
    /* total number of frames */
    gulong tot_frames;
    /* current frame */
    gulong current_frame;
    gboolean init;


    guint num_audio_pads;
    guint num_video_pads;
    GstPad *audio_pad[GST_QUICKTIME_DEMUX_MAX_AUDIO_PADS];
    gboolean audio_need_flush[GST_QUICKTIME_DEMUX_MAX_AUDIO_PADS];

    GstPad *video_pad[GST_QUICKTIME_DEMUX_MAX_VIDEO_PADS];
    gboolean video_need_flush[GST_QUICKTIME_DEMUX_MAX_VIDEO_PADS];

    gpointer extra_data;
  };

  struct _GstQuicktimeDemuxClass
  {
    GstElementClass parent_class;
  };


#ifdef __cplusplus
}
#endif				/* __cplusplus */


#endif				/* __GST_QUICKTIME_DEMUXR_H__ */

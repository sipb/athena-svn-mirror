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
#include <openquicktime/openquicktime.h>

#include "gstopenquicktimedemux.h"

/* elementfactory information */
static GstElementDetails gst_quicktime_demux_details = {
  "quicktime parser",
  "Codec/Parser",
  "LGPL",
  "Parse a quicktime file into audio and video",
  VERSION,
  "Yann <yann@3ivx.com>",
  "(C) 2001",
};

static GstCaps *quicktime_type_find (GstBuffer * buf, gpointer private);

/* typefactory for 'quicktime' */
static GstTypeDefinition quicktimedefinition = {
  "quicktime_demux_video/quicktime",
  "video/quicktime",
  ".mov",
  quicktime_type_find,
};

enum
{
  /* FILL ME */
  LAST_SIGNAL
};

enum
{
  ARG_0,
  ARG_BIT_RATE,
  ARG_MEDIA_TIME,
  ARG_CURRENT_TIME,
  /* FILL ME */
};

GST_PAD_TEMPLATE_FACTORY (sink_templ, 
			  "sink", 
			  GST_PAD_SINK, 
			  GST_PAD_ALWAYS,
			  GST_CAPS_NEW (
			    "quicktimedemux_sink",
			    "video/quicktime", NULL)
			  );
/*                         ^ semicolon is here so indent will eat it */

GST_PAD_TEMPLATE_FACTORY (src_video_templ,
			  "video_%02d",
			  GST_PAD_SRC,
			  GST_PAD_SOMETIMES,
			  GST_CAPS_NEW (
			    "quicktime_src_video",
			    "video/quicktime",
			    "format",GST_PROPS_STRING ("strf_vids")),
			  GST_CAPS_NEW (
			    "quicktime_src_video",
			    "video/jpeg",
			    "width",GST_PROPS_INT_RANGE(16,4096),
			    "height",GST_PROPS_INT_RANGE(16,4096))
			  );

GST_PAD_TEMPLATE_FACTORY (src_audio_templ, 
			  "audio_%02d", 
			  GST_PAD_SRC,
			  GST_PAD_SOMETIMES, 
			  GST_CAPS_NEW (
			    "src_audio",
			    "video/quicktime",
			    "format", GST_PROPS_STRING ("strf_auds"))
			  );

static void gst_quicktime_demux_class_init (GstQuicktimeDemuxClass * klass);
static void gst_quicktime_demux_init (GstQuicktimeDemux * quicktime_demux);

static void gst_quicktime_demux_loop (GstElement * element);

static void gst_quicktime_demux_get_property (GObject * object,
					      guint prop_id,
					      GValue * value,
					      GParamSpec * pspec);

static quicktime_t *quicktime_open_pad (GstQuicktimeDemux * quicktime_demux);

static void gst_quicktime_demux_get_property (GObject * object,
					      guint prop_id,
					      GValue * value,
					      GParamSpec * pspec);

static GstElementStateReturn gst_quicktime_demux_change_state (GstElement *
							       element);

static GstElementClass *parent_class = NULL;

static GType
gst_quicktime_demux_get_type (void)
{
  static GType quicktime_demux_type = 0;

  if (!quicktime_demux_type) {
    static const GTypeInfo quicktime_demux_info = {
      sizeof (GstQuicktimeDemuxClass), NULL,
      NULL,
      (GClassInitFunc) gst_quicktime_demux_class_init,
      NULL,
      NULL,
      sizeof (GstQuicktimeDemux),
      0,
      (GInstanceInitFunc) gst_quicktime_demux_init,
    };
    quicktime_demux_type =
      g_type_register_static (GST_TYPE_ELEMENT, "GstQuicktimeDemux",
			      &quicktime_demux_info, 0);
  }
  return quicktime_demux_type;
}

static void
gst_quicktime_demux_get_property (GObject * object, guint prop_id,
				  GValue * value, GParamSpec * pspec)
{
  GstQuicktimeDemux *src;

  g_return_if_fail (GST_IS_QUICKTIME_DEMUX (object));

  src = GST_QUICKTIME_DEMUX (object);

  switch (prop_id) {
  case ARG_BIT_RATE:
    break;
  case ARG_MEDIA_TIME:
    g_value_set_long (value, (src->tot_frames * src->time_interval) / 1000000);
    break;
  case ARG_CURRENT_TIME:
    g_value_set_long (value,
		      (src->current_frame * src->time_interval) / 1000000);
    break;
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
    break;
  }
}



static void
gst_quicktime_demux_class_init (GstQuicktimeDemuxClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_BIT_RATE, g_param_spec_long ("bit_rate", "bit_rate", "bit_rate", G_MINLONG, G_MAXLONG, 0, G_PARAM_READABLE));	/* CHECKME */
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_MEDIA_TIME, g_param_spec_long ("media_time", "media_time", "media_time", G_MINLONG, G_MAXLONG, 0, G_PARAM_READABLE));	/* CHECKME */
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_CURRENT_TIME, g_param_spec_long ("current_time", "current_time", "current_time", G_MINLONG, G_MAXLONG, 0, G_PARAM_READABLE));	/* CHECKME */

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  gobject_class->get_property = gst_quicktime_demux_get_property;

  gstelement_class->change_state = gst_quicktime_demux_change_state;
}

static GstElementStateReturn
gst_quicktime_demux_change_state (GstElement * element)
{
  GstQuicktimeDemux *quicktime_demux = GST_QUICKTIME_DEMUX (element);

  switch (GST_STATE_TRANSITION (element)) {
  case GST_STATE_NULL_TO_READY:
    break;
  case GST_STATE_READY_TO_PAUSED:
    quicktime_demux->bs = gst_bytestream_new (quicktime_demux->sinkpad);
    break;
  case GST_STATE_PAUSED_TO_PLAYING:
    break;
  case GST_STATE_PLAYING_TO_PAUSED:
    break;
  case GST_STATE_PAUSED_TO_READY:
    gst_bytestream_destroy (quicktime_demux->bs);
    break;
  case GST_STATE_READY_TO_NULL:
    break;
  default:
    break;
  }

  parent_class->change_state (element);

  return GST_STATE_SUCCESS;
}

static void
gst_quicktime_demux_init (GstQuicktimeDemux * quicktime_demux)
{
  guint i;

  GST_FLAG_SET (quicktime_demux, GST_ELEMENT_EVENT_AWARE);

  quicktime_demux->sinkpad =
    gst_pad_new_from_template (GST_PAD_TEMPLATE_GET (sink_templ), "sink");
  gst_element_set_loop_function (GST_ELEMENT (quicktime_demux),
				 gst_quicktime_demux_loop);
  gst_element_add_pad (GST_ELEMENT (quicktime_demux), quicktime_demux->sinkpad);

  for (i = 0; i < GST_QUICKTIME_DEMUX_MAX_AUDIO_PADS; i++)
    quicktime_demux->audio_pad[i] = NULL;
  for (i = 0; i < GST_QUICKTIME_DEMUX_MAX_VIDEO_PADS; i++)
    quicktime_demux->video_pad[i] = NULL;

  quicktime_demux->init = TRUE;
}


static GstCaps *
quicktime_type_find (GstBuffer * buf, gpointer private)
{
  gchar *data = GST_BUFFER_DATA (buf);

  if (!strncmp (&data[4], "wide", 4) ||
      !strncmp (&data[4], "moov", 4) || !strncmp (&data[4], "mdat", 4)) {
    return gst_caps_new ("quicktime_type_find", "video/quicktime", NULL);
  }
  return NULL;
}



static gboolean
plugin_init (GModule * module, GstPlugin * plugin)
{
  GstElementFactory *factory;
  GstTypeFactory *type;

  if (!gst_library_load ("gstbytestream"))
    return FALSE;

  factory =
    gst_element_factory_new ("quicktime_demux", GST_TYPE_QUICKTIME_DEMUX,
			     &gst_quicktime_demux_details);
  g_return_val_if_fail (factory != NULL, FALSE);

  gst_element_factory_add_pad_template (factory,
					GST_PAD_TEMPLATE_GET (sink_templ));
  gst_element_factory_add_pad_template (factory,
					GST_PAD_TEMPLATE_GET (src_audio_templ));
  gst_element_factory_add_pad_template (factory,
					GST_PAD_TEMPLATE_GET (src_video_templ));

  type = gst_type_factory_new (&quicktimedefinition);
  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (type));

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "quicktime_demux",
  plugin_init
};


static void
gst_quicktime_demux_loop (GstElement * element)
{
  GstQuicktimeDemux *quicktime_demux;
  quicktime_t *QtFile;

  GST_DEBUG (0, "gst_quicktime_demux_loop: begin");

  g_return_if_fail (element != NULL);
  g_return_if_fail (GST_IS_QUICKTIME_DEMUX (element));

  quicktime_demux = GST_QUICKTIME_DEMUX (element);

  if (quicktime_demux->init) {
    GST_DEBUG (0, "gst_quicktime_demux_loop: init");
    QtFile = quicktime_open_pad (quicktime_demux);
    if (!QtFile) {
      gst_element_error (GST_ELEMENT (element),
			 "gst_quicktime_demux_init: QtFile == NULL ");
      return;
    }

    quicktime_demux->file = QtFile;

    /* timestamp stuff */
    quicktime_demux->next_time = 0;
    quicktime_demux->time_interval = 1000000 / quicktime_frame_rate (QtFile, 0);
    quicktime_demux->tot_frames = quicktime_video_length (QtFile, 0);
    quicktime_demux->current_frame = 0;

    quicktime_demux->num_audio_pads = quicktime_audio_tracks (QtFile);
    quicktime_demux->num_video_pads = quicktime_video_tracks (QtFile);

    quicktime_demux->init = FALSE;
  }

  if (quicktime_demux->num_video_pads) {
    quicktime_t *file = quicktime_demux->file;
    GstPad *outpad = ((quicktime_codec_t *) file->vtracks[0].codec)->priv;
    GstBuffer *outbuf;

    GST_DEBUG (0, "gst_quicktime_demux_loop: video pads");
    if (!GST_PAD_IS_LINKED (outpad)) {
      goto next_audio;
    }

    if (quicktime_video_position (file, 0) >= quicktime_video_length (file, 0)) {
      /* we stop the data source. */
      gst_pad_event_default (quicktime_demux->sinkpad, gst_event_new (GST_EVENT_EOS));
      goto next_audio;
    }

    outbuf = gst_buffer_new ();
    GST_BUFFER_DATA (outbuf) =
      g_malloc (4 * quicktime_video_height (file, 0) *
		quicktime_video_width (file, 0));
    GST_BUFFER_SIZE (outbuf) =
      quicktime_read_frame (file, GST_BUFFER_DATA (outbuf), 0);
    GST_BUFFER_TIMESTAMP (outbuf) = quicktime_demux->next_time;

    quicktime_demux->next_time += quicktime_demux->time_interval;
    quicktime_demux->current_frame++;

    gst_pad_push (outpad, outbuf);
  }

next_audio:
  if (quicktime_demux->num_audio_pads) {
    /* Nothing for the moment */
  }
}

static gboolean
gst_quicktime_handle_event (GstQuicktimeDemux *quicktime_demux)
{
  guint32 remaining;
  GstEvent *event;
  GstEventType type;
  
  gst_bytestream_get_status (quicktime_demux->bs, &remaining, &event);

  type = event? GST_EVENT_TYPE (event) : GST_EVENT_UNKNOWN;

  switch (type) {
    case GST_EVENT_EOS:
      gst_pad_event_default (quicktime_demux->sinkpad, event);
      break;
    case GST_EVENT_SEEK:
      g_warning ("seek event\n");
      break;
    case GST_EVENT_FLUSH:
      //g_warning ("flush event\n");
      break;
    case GST_EVENT_DISCONTINUOUS:
      //g_warning ("discont event\n");
      break;
    default:
      g_warning ("unhandled event %d\n", type);
      break;
  }

  return TRUE;
}


static int
gst_quicktime_read_data (quicktime_t * file, char *data, longest size)
{
  char *peeked;
  guint32 got_bytes;
  GstByteStream *bs = ((GstQuicktimeDemux *) file->stream)->bs;

  GST_DEBUG (0, "gst_quicktime_read_data : %lld bytes ", size);

  /* FIXME i'm emulating how it was before */
  gst_bytestream_seek (bs, file->file_position, GST_SEEK_METHOD_SET);

  do {
    got_bytes = gst_bytestream_peek_bytes (bs, (guint8**)&peeked, size);
    if (got_bytes == size) {
      memcpy (data, peeked, size);
      gst_bytestream_flush (bs, size);

      GST_DEBUG (0, "read offset %lld", file->file_position);
      file->ftell_position += size;
      file->file_position += size;
      return 1;
    }
  } while(gst_quicktime_handle_event((GstQuicktimeDemux *) file->stream));

  return 1;
}


static int
gst_quicktime_seek (quicktime_t * file, longest offset)
{
  GST_DEBUG (0, "gst_quicktime_seek : %lld newoffset", offset);

  file->ftell_position = offset;

  if (offset > file->total_length || offset < 0)
    return 1;

  return 0;
}

static int
gst_quicktime_video_codec (quicktime_t * file, quicktime_video_map_t * vtrack)
{
  /*gulong compressor =
    GST_STR_FOURCC (vtrack->track->mdia.minf.stbl.stsd.table[0].format);
    */
  GstPad *srcpad;

  GstQuicktimeDemux *quicktime_demux = (GstQuicktimeDemux *) file->stream;

  GST_DEBUG (0, "gst_quicktime_video_codec : begin");

  vtrack->codec = (quicktime_codec_t *) calloc (1, sizeof (quicktime_codec_t));

  srcpad =
    gst_pad_new_from_template (GST_PAD_TEMPLATE_GET (src_video_templ),
			       g_strdup_printf ("video_%02d",
						quicktime_demux->
						num_video_pads));
#if 0
  gst_pad_try_set_caps (srcpad,
			GST_CAPS_NEW ("quicktimedec_video_src",
				      "video/quicktime",
				      "format", GST_PROPS_STRING ("strf_vids"), 
				      "width", GST_PROPS_INT (quicktime_video_width
						     (file,
						      quicktime_demux->
						      num_video_pads)),
				      "height", GST_PROPS_INT (quicktime_video_height
						     (file,
						      quicktime_demux->
						      num_video_pads)),
				      "compression", GST_PROPS_FOURCC (compressor)));
#endif
  gst_pad_try_set_caps (srcpad,
			GST_CAPS_NEW ("quicktime_video_src",
				      "video/jpeg",
				      "width", GST_PROPS_INT (
					quicktime_video_width (file,
						      quicktime_demux->
						      num_video_pads)),
				      "height", GST_PROPS_INT (
					quicktime_video_height (file,
						      quicktime_demux->
						      num_video_pads)))
			);

  quicktime_demux->video_pad[quicktime_demux->num_video_pads++] = srcpad;

  ((quicktime_codec_t *) ((file)->vtracks[0].codec))->priv = srcpad;

  gst_element_add_pad (GST_ELEMENT (quicktime_demux), srcpad);

  GST_DEBUG (0, "gst_quicktime_video_codec : end");

  /* everything is fine now ;) */
  return 0;
}

static int
gst_quicktime_audio_codec (quicktime_t * file, quicktime_audio_map_t * atrack)
{
  /* we don't support audio for the moment */
  return 0;
}

static quicktime_t *
quicktime_open_pad (GstQuicktimeDemux * quicktime_demux)
{
  quicktime_t *new_file = calloc (1, sizeof (quicktime_t));

  GST_DEBUG (0, "quicktime_open_pad : begin");

  quicktime_init (new_file);
  new_file->wr = 0;
  new_file->rd = 1;
  new_file->mdat.atom.start = 0;
  new_file->stream = quicktime_demux;

  new_file->decompressed_buffer_size = 0;
  new_file->decompressed_buffer = NULL;
  new_file->decompressed_position = 0;

  new_file->quicktime_read_data = gst_quicktime_read_data;
  new_file->quicktime_write_data = NULL;
  new_file->quicktime_fseek = gst_quicktime_seek;
  new_file->quicktime_init_vcodec = gst_quicktime_video_codec;
  new_file->quicktime_init_acodec = gst_quicktime_audio_codec;

  /* Get length. */
  new_file->total_length = 1000000000;

  if (quicktime_read_info (new_file)) {
    fprintf (stderr, "quicktime_open: error in header\n");
    gst_element_error (GST_ELEMENT (quicktime_demux), "error reading header");
    new_file = NULL;
  } else {
    GST_DEBUG (0, "Info read");
  }

  return new_file;
}

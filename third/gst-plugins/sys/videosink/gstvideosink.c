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

#include <config.h>
#include <string.h>

#include <sys/time.h>

/*#define GST_DEBUG_FORCE_DISABLE*/

#include "gstvideosink.h"

/* elementfactory information */
static GstElementDetails gst_videosink_details = {
  "Video sink",
  "Sink/Video",
  "LGPL",
  "A general video sink",
  VERSION,
  "benjamin Otte <in7y118@public.uni-hamburg.de",
  "(C) 2002",
};

/* xvideosink signals and args */
enum {
  LAST_SIGNAL
};


enum {
  ARG_0,
  ARG_WIDTH,
  ARG_HEIGHT,
  ARG_FRAMES_DISPLAYED,
  ARG_FRAME_TIME,
  ARG_HOOK,
  ARG_MUTE,
  ARG_REPAINT
};

/* Videosink class */
#define GST_TYPE_VIDEOSINK		(gst_videosink_get_type())
#define GST_VIDEOSINK(obj)		(G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_VIDEOSINK,GstVideoSink))
#define GST_VIDEOSINK_CLASS(klass)	(G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_VIDEOSINK,GstVideoSink))
#define GST_IS_VIDEOSINK(obj)		(G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_VIDEOSINK))
#define GST_IS_VIDEOSINK_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_VIDEOSINK))

typedef struct _GstVideoSink GstVideoSink;
typedef struct _GstVideoSinkClass GstVideoSinkClass;

struct _GstVideoSink {
  GstElement element;

  GstPad *sinkpad;

  gint frames_displayed;
  guint64 frame_time;
  gint width, height;
  gboolean muted;
  GstBuffer *last_image; /* not thread safe ? */
  
  GstClock *clock;

  /* bufferpool stuff */
  GstBufferPool *bufferpool;
  GMutex *cache_lock;
  GList *cache;
  
  /* plugins */
  GstImagePlugin* plugin;
  GstImageConnection *conn;
  
  /* allow anybody to hook in here */
  GstImageInfo *hook;
};

struct _GstVideoSinkClass {
  GstElementClass parent_class;

  /* plugins */
  GList *plugins;
};


static GType 			gst_videosink_get_type		(void);
static void			gst_videosink_class_init	(GstVideoSinkClass *klass);
static void			gst_videosink_init		(GstVideoSink *sink);
/* static void 			gst_videosink_dispose 		(GObject *object); */

static void			gst_videosink_chain		(GstPad *pad, GstBuffer *buf);
static void			gst_videosink_set_clock		(GstElement *element, GstClock *clock);
static GstElementStateReturn	gst_videosink_change_state 	(GstElement *element);
static GstPadLinkReturn	gst_videosink_sinkconnect	(GstPad *pad, GstCaps *caps);
static GstCaps *		gst_videosink_getcaps		(GstPad *pad, GstCaps *caps);
static GstBufferPool*		gst_videosink_get_bufferpool	(GstPad *pad);

static void			gst_videosink_set_property	(GObject *object, guint prop_id, 
								 const GValue *value, GParamSpec *pspec);
static void			gst_videosink_get_property	(GObject *object, guint prop_id, 
								 GValue *value, GParamSpec *pspec);

static void			gst_videosink_release_conn	(GstVideoSink *sink);
static void			gst_videosink_append_cache	(GstVideoSink *sink, GstImageData *image);
static gboolean			gst_videosink_set_caps		(GstVideoSink *sink, GstCaps *caps);
/* bufferpool stuff */
static GstBuffer*		gst_videosink_buffer_new 	(GstBufferPool *pool, 
								 gint64 location, 
								 guint size, gpointer user_data);
static void			gst_videosink_buffer_free	(GstBufferPool *pool, 
		                                                 GstBuffer *buffer, 
								 gpointer user_data);

/* prototypes from plugins */
extern GstImagePlugin* 		get_ximage_plugin		(void);
extern GstImagePlugin* 		get_xvimage_plugin		(void);
/* default output */
extern void			gst_xwindow_new 		(GstVideoSink *sink);


static GstPadTemplate *sink_template;

static GstElementClass *parent_class = NULL;
/* static guint gst_videosink_signals[LAST_SIGNAL] = { 0 }; */

static GType
gst_videosink_get_type (void)
{
  static GType videosink_type = 0;

  if (!videosink_type) {
    static const GTypeInfo videosink_info = {
      sizeof(GstVideoSinkClass),
      NULL,
      NULL,
      (GClassInitFunc) gst_videosink_class_init,
      NULL,
      NULL,
      sizeof(GstVideoSink),
      0,
      (GInstanceInitFunc) gst_videosink_init,
    };
    videosink_type = g_type_register_static(GST_TYPE_ELEMENT, "GstVideoSink", &videosink_info, 0);
  }
  return videosink_type;
}

static void
gst_videosink_class_init (GstVideoSinkClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  gobject_class->set_property = gst_videosink_set_property;
  gobject_class->get_property = gst_videosink_get_property;

  g_object_class_install_property (gobject_class, ARG_WIDTH,
    g_param_spec_int ("width", "Width", "The width of the images",
                      G_MININT, G_MAXINT, 0, G_PARAM_READABLE)); /* CHECKME */
  g_object_class_install_property (gobject_class, ARG_HEIGHT,
    g_param_spec_int ("height", "Height", "The height of the images",
                      G_MININT, G_MAXINT, 0, G_PARAM_READABLE)); /* CHECKME */
  g_object_class_install_property (gobject_class, ARG_FRAMES_DISPLAYED,
    g_param_spec_int ("frames_displayed", "Frames Displayed", "The number of frames displayed so far",
                      G_MININT,G_MAXINT, 0, G_PARAM_READWRITE)); /* CHECKME */
  g_object_class_install_property (gobject_class, ARG_FRAME_TIME,
    g_param_spec_int ("frame_time", "Frame time", "The interval between frames",
                      G_MININT, G_MAXINT, 0, G_PARAM_READWRITE)); /* CHECKME */
  g_object_class_install_property (gobject_class, ARG_HOOK,
    g_param_spec_pointer ("hook", "Hook", "The object receiving the output", G_PARAM_WRITABLE));
  g_object_class_install_property (gobject_class, ARG_MUTE,
    g_param_spec_boolean ("mute", "Mute", "if the output is muted", FALSE, G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class, ARG_REPAINT,
    g_param_spec_boolean ("repaint", "Repaint", "repaints the current frame", FALSE, G_PARAM_WRITABLE));

  /* gobject_class->dispose = gst_videosink_dispose; */
  
  gstelement_class->change_state = gst_videosink_change_state;
  gstelement_class->set_clock = gst_videosink_set_clock;
  
  /* plugins */
  klass->plugins = NULL;
  klass->plugins = g_list_append (klass->plugins, get_xvimage_plugin ());
  klass->plugins = g_list_append (klass->plugins, get_ximage_plugin ());
}


static void
gst_videosink_init (GstVideoSink *sink)
{
  sink->sinkpad = gst_pad_new_from_template (sink_template, "sink");
  gst_element_add_pad (GST_ELEMENT (sink), sink->sinkpad);
  gst_pad_set_chain_function (sink->sinkpad, gst_videosink_chain);
  gst_pad_set_link_function (sink->sinkpad, gst_videosink_sinkconnect);
  gst_pad_set_getcaps_function (sink->sinkpad, gst_videosink_getcaps);
  gst_pad_set_bufferpool_function (sink->sinkpad, gst_videosink_get_bufferpool);

  sink->last_image = NULL;
  sink->width = 0;
  sink->height = 0;
  sink->muted = FALSE;
  sink->clock = NULL;  
  GST_FLAG_SET(sink, GST_ELEMENT_THREAD_SUGGESTED);
  GST_FLAG_SET (sink, GST_ELEMENT_EVENT_AWARE);
    
  /* create bufferpool and image cache */
  GST_DEBUG (0, "videosink: creating bufferpool");
  sink->bufferpool = gst_buffer_pool_new (
		    NULL,
		    NULL,
		    (GstBufferPoolBufferNewFunction)gst_videosink_buffer_new,
		    NULL,
		    (GstBufferPoolBufferFreeFunction)gst_videosink_buffer_free,
		    sink);
  sink->cache_lock = g_mutex_new();
  sink->cache = NULL; 
  
  /* plugins */
  sink->plugin = NULL;
  sink->conn = NULL;
  
  /* do initialization of default hook here */
  gst_xwindow_new (sink);
}
static void
gst_videosink_release_conn (GstVideoSink *sink)
{
  if (sink->conn == NULL) return;
  
  /* free last image if any */
  if (sink->last_image != NULL)
  {
    gst_buffer_unref (sink->last_image);
    sink->last_image = NULL;
  }
  /* free cache */
  g_mutex_lock (sink->cache_lock);
  while (sink->cache)
  {
    sink->plugin->free_image ((GstImageData *) sink->cache->data);
    sink->cache = g_list_delete_link (sink->cache, sink->cache);
  }
  g_mutex_unlock (sink->cache_lock);
  
  /* release connection */
  sink->conn->free_conn (sink->conn);
  sink->conn = NULL;
}
static void 		
gst_videosink_append_cache (GstVideoSink *sink, GstImageData *image)
{
  g_mutex_lock (sink->cache_lock);
  sink->cache = g_list_prepend (sink->cache, image);
  g_mutex_unlock (sink->cache_lock);
}
static GstBuffer*
gst_videosink_buffer_new (GstBufferPool *pool, gint64 location, 
		          guint size, gpointer user_data)
{
  GstVideoSink *sink;
  GstBuffer *buffer;
  GstImageData *image;
  
  sink = GST_VIDEOSINK (user_data);
  
  if (sink->cache != NULL) {
    g_mutex_lock (sink->cache_lock);
    image = (GstImageData *) sink->cache->data;
    sink->cache = g_list_delete_link (sink->cache, sink->cache);
    g_mutex_unlock (sink->cache_lock);
  } else {
    image = sink->plugin->get_image (sink->hook, sink->conn);
  }
  
  buffer = gst_buffer_new ();
  GST_BUFFER_DATA (buffer) = image->data;
  GST_BUFFER_SIZE (buffer) = image->size;
  GST_BUFFER_POOL_PRIVATE (buffer) = image;
  
  return buffer;
}
static void
gst_videosink_buffer_free (GstBufferPool *pool, GstBuffer *buffer, gpointer user_data)
{
  GstVideoSink *sink = GST_VIDEOSINK (gst_buffer_pool_get_user_data (GST_BUFFER_BUFFERPOOL (buffer)));

  gst_videosink_append_cache (sink, (GstImageData *) GST_BUFFER_POOL_PRIVATE (buffer));

  /* set to NULL so the data is not freed */
  GST_BUFFER_DATA (buffer) = NULL;

  gst_buffer_default_free (buffer);
}

static GstBufferPool*
gst_videosink_get_bufferpool (GstPad *pad)
{
  GstVideoSink *sink = GST_VIDEOSINK (gst_pad_get_parent (pad));

  return sink->bufferpool;
}
static gboolean
gst_videosink_set_caps (GstVideoSink *sink, GstCaps *caps)
{
  GList *list = ((GstVideoSinkClass *) G_OBJECT_GET_CLASS (sink))->plugins;
  GstImageConnection *conn = NULL;
  while (list)    
  {
    GstImagePlugin *plugin = (GstImagePlugin *) list->data;
    if ((conn = plugin->set_caps (sink->hook, caps)) != NULL)
    {
      gst_videosink_release_conn (sink);
      sink->conn = conn;
      sink->plugin = plugin;
      return TRUE;
    }
    list = g_list_next (list);
  }
  return FALSE;
}
static GstPadLinkReturn
gst_videosink_sinkconnect (GstPad *pad, GstCaps *caps)
{
  GstVideoSink *sink;
  guint32 fourcc, print_format;

  sink = GST_VIDEOSINK (gst_pad_get_parent (pad));

  /* we are not going to act on variable caps */
  if (!GST_CAPS_IS_FIXED (caps))
    return GST_PAD_LINK_DELAYED;
  
  /* try to set the caps on the output */
  if (gst_videosink_set_caps (sink, caps) == FALSE)
  {
    return GST_PAD_LINK_REFUSED;
  }
  
  /* remember width & height */
  gst_caps_get_int (caps, "width", &sink->width);
  gst_caps_get_int (caps, "height", &sink->height);

  gst_caps_get_fourcc_int (caps, "format", &fourcc);
  print_format = GULONG_FROM_LE (fourcc);
  GST_DEBUG (0, "xvideosink: setting %08x (%4.4s) %dx%d\n", 
		  fourcc, (gchar*)&print_format, sink->width, sink->height);

  /* emit signal */
  g_object_freeze_notify (G_OBJECT (sink));
  g_object_notify (G_OBJECT (sink), "width");
  g_object_notify (G_OBJECT (sink), "height");
  g_object_thaw_notify (G_OBJECT (sink));

  return GST_PAD_LINK_OK;
}
static GstCaps *
gst_videosink_getcaps (GstPad *pad, GstCaps *caps)
{
  /* what is the "caps" parameter good for? */
  GstVideoSink *sink = GST_VIDEOSINK (gst_pad_get_parent (pad));
  GstCaps *ret = NULL;
  GList *list = ((GstVideoSinkClass *) G_OBJECT_GET_CLASS (sink))->plugins;
  
  while (list)    
  {
    ret = gst_caps_append (ret, ((GstImagePlugin *) list->data)->get_caps (sink->hook));
    list = g_list_next (list);
  }

  return ret;
}

static void
gst_videosink_set_clock (GstElement *element, GstClock *clock)
{
  GstVideoSink *sink = GST_VIDEOSINK (element);
  
  sink->clock = clock;
}
static void
gst_videosink_chain (GstPad *pad, GstBuffer *buf)
{
  GstVideoSink *sink;
  GstClockTime time = GST_BUFFER_TIMESTAMP (buf);
  GstBuffer *buffer;

  g_return_if_fail (pad != NULL);
  g_return_if_fail (GST_IS_PAD (pad));
  g_return_if_fail (buf != NULL);

  sink = GST_VIDEOSINK (gst_pad_get_parent (pad));

  if (GST_IS_EVENT (buf)) {
    GstEvent *event = GST_EVENT (buf);

    switch (GST_EVENT_TYPE (event)) {
      default:
        gst_pad_event_default (pad, event);
    }
    return;
  }
  GST_DEBUG (0,"videosink: clock wait: %llu %u", 
		  GST_BUFFER_TIMESTAMP (buf), GST_BUFFER_SIZE (buf));

  if (sink->clock && time != -1) {
    GstClockReturn ret;
    GstClockID id = gst_clock_new_single_shot_id (sink->clock, GST_BUFFER_TIMESTAMP (buf));

    ret = gst_element_clock_wait (GST_ELEMENT (sink), id, NULL);
    gst_clock_id_free (id);

    /* we are going to drop early buffers */
    if (ret == GST_CLOCK_EARLY) {
      gst_buffer_unref (buf);
      return;
    }
  }

  /* call the notify _before_ displaying so the handlers can react */
  sink->frames_displayed++;
  g_object_notify (G_OBJECT (sink), "frames_displayed");

  if (!sink->muted)
  {
    /* free last_image, if any */
    if (sink->last_image != NULL)
      gst_buffer_unref (sink->last_image);
    if (sink->bufferpool && GST_BUFFER_BUFFERPOOL (buf) == sink->bufferpool) {
      sink->plugin->put_image (sink->hook, (GstImageData *) GST_BUFFER_POOL_PRIVATE (buf));
      sink->last_image = buf;
    } else {
      buffer = gst_buffer_new_from_pool (gst_videosink_get_bufferpool (sink->sinkpad), 
		                         0, GST_BUFFER_SIZE (buf));
      memcpy (GST_BUFFER_DATA (buffer), GST_BUFFER_DATA (buf), 
	      GST_BUFFER_SIZE (buf) > GST_BUFFER_SIZE (buffer) ? 
	        GST_BUFFER_SIZE (buffer) : GST_BUFFER_SIZE (buf));

      sink->plugin->put_image (sink->hook, (GstImageData *) GST_BUFFER_POOL_PRIVATE (buffer));

      sink->last_image = buffer;
      gst_buffer_unref (buf);
    }
  }
  
}


static void
gst_videosink_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  GstVideoSink *sink;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_VIDEOSINK (object));

  sink = GST_VIDEOSINK (object);

  switch (prop_id) {
    case ARG_FRAMES_DISPLAYED:
      sink->frames_displayed = g_value_get_int (value);
      g_object_notify (object, "frames_displayed");
      break;
    case ARG_FRAME_TIME:
      sink->frame_time = g_value_get_int (value);
      break;
    case ARG_HOOK:
      if (sink->hook)
      {
	sink->hook->free_info (sink->hook);
      }
      sink->hook = g_value_get_pointer (value);
      break;
    case ARG_MUTE:
      sink->muted = g_value_get_boolean (value);
      g_object_notify (object, "mute");
      break;
    case ARG_REPAINT:
      if (sink->last_image != NULL) {
	sink->plugin->put_image (sink->hook, (GstImageData *) GST_BUFFER_POOL_PRIVATE (sink->last_image));
      }
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_videosink_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GstVideoSink *sink;

  /* it's not null if we got it, but it might not be ours */
  sink = GST_VIDEOSINK(object);

  switch (prop_id) {
    case ARG_WIDTH:
      g_value_set_int (value, sink->width);
      break;
    case ARG_HEIGHT:
      g_value_set_int (value, sink->height);
      break;
    case ARG_FRAMES_DISPLAYED:
      g_value_set_int (value, sink->frames_displayed);
      break;
    case ARG_FRAME_TIME:
      g_value_set_int (value, sink->frame_time/1000000);
      break;
    case ARG_MUTE:
      g_value_set_boolean (value, sink->muted);
      break;
    default: 
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}


static GstElementStateReturn
gst_videosink_change_state (GstElement *element)
{
  GstVideoSink *sink;

  sink = GST_VIDEOSINK (element);

  switch (GST_STATE_TRANSITION (element)) {
    case GST_STATE_NULL_TO_READY:
      break;
    case GST_STATE_READY_TO_PAUSED:
      if (sink->conn)
        sink->conn->open_conn (sink->conn, sink->hook);
      break;
    case GST_STATE_PAUSED_TO_PLAYING:
      break;
    case GST_STATE_PLAYING_TO_PAUSED:
      break;
    case GST_STATE_PAUSED_TO_READY:
      if (sink->conn)
        sink->conn->close_conn (sink->conn, sink->hook);
      if (sink->last_image) {
	gst_buffer_unref (sink->last_image);
        sink->last_image = NULL;
      }
      break;
    case GST_STATE_READY_TO_NULL:
      gst_videosink_release_conn (sink);
      break;
  }

  parent_class->change_state (element);

  return GST_STATE_SUCCESS;
}

static gboolean
plugin_init (GModule *module, GstPlugin *plugin)
{
  GstElementFactory *factory;

  /* create an elementfactory for the xvideosink element */
  factory = gst_element_factory_new("videosink",GST_TYPE_VIDEOSINK,
                                   &gst_videosink_details);
  g_return_val_if_fail(factory != NULL, FALSE);

  sink_template = gst_pad_template_new (
		  "sink",
                  GST_PAD_SINK,
  		  GST_PAD_ALWAYS,
		  NULL);

  gst_element_factory_add_pad_template (factory, sink_template);

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "videosink",
  plugin_init
};

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

#include "xvideosink.h"

/* elementfactory information */
static GstElementDetails gst_xvideosink_details = {
  "Video sink",
  "Sink/Video",
  "LGPL",
  "A general X Window video sink",
  VERSION,
  "Wim Taymans <wim.taymans@chello.be>",
  "(C) 2000",
};

/* xvideosink signals and args */
enum {
  SIGNAL_FRAME_DISPLAYED,
  SIGNAL_HAVE_SIZE,
  SIGNAL_HAVE_XID,
  LAST_SIGNAL
};


enum {
  ARG_0,
  ARG_XID,
  ARG_SCALE,
  ARG_FRAMES_DISPLAYED,
  ARG_FRAME_TIME,
  ARG_DISABLE_XV,
  ARG_TOPLEVEL,
  ARG_AUTOSIZE,
  ARG_NEED_NEW_WINDOW,
};
#define FORMAT_IS_RGB(format) ((format) == GST_MAKE_FOURCC ('R','G','B',' '))

static void		gst_xvideosink_class_init	(GstXVideoSinkClass *klass);
static void		gst_xvideosink_init		(GstXVideoSink *xvideosink);
static void 		gst_xvideosink_dispose 		(GObject *object);

static void		gst_xvideosink_chain		(GstPad *pad, GstBuffer *buf);
static void		gst_xvideosink_set_clock	(GstElement *element, GstClock *clock);
static GstElementStateReturn
			gst_xvideosink_change_state 	(GstElement *element);

static gboolean		gst_xvideosink_release_locks	(GstElement *element);
static void		gst_xvideosink_set_property	(GObject *object, guint prop_id, 
							 const GValue *value, GParamSpec *pspec);
static void		gst_xvideosink_get_property	(GObject *object, guint prop_id, 
							 GValue *value, GParamSpec *pspec);

static GstCaps* 	gst_xvideosink_get_pad_template_caps (gboolean with_xv);

static GstPadTemplate *sink_template;
static GstCaps *formats;

static GstElementClass *parent_class = NULL;
static guint gst_xvideosink_signals[LAST_SIGNAL] = { 0 };

GType
gst_xvideosink_get_type (void)
{
  static GType xvideosink_type = 0;

  if (!xvideosink_type) {
    static const GTypeInfo xvideosink_info = {
      sizeof(GstXVideoSinkClass),      NULL,
      NULL,
      (GClassInitFunc)gst_xvideosink_class_init,
      NULL,
      NULL,
      sizeof(GstXVideoSink),
      0,
      (GInstanceInitFunc)gst_xvideosink_init,
    };
    xvideosink_type = g_type_register_static(GST_TYPE_ELEMENT, "GstXVideoSink", &xvideosink_info, 0);
  }
  return xvideosink_type;
}

static void
gst_xvideosink_class_init (GstXVideoSinkClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_XID,
    g_param_spec_int ("xid", "Xid", "The Xid of the window",
                      G_MININT, G_MAXINT, 0, G_PARAM_READABLE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_FRAMES_DISPLAYED,
    g_param_spec_int ("frames_displayed", "Frames Displayed", "The number of frames displayed so far",
                      G_MININT,G_MAXINT, 0, G_PARAM_READWRITE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_FRAME_TIME,
    g_param_spec_int ("frame_time", "Frame time", "The interval between frames",
                      G_MININT, G_MAXINT, 0, G_PARAM_READWRITE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_DISABLE_XV,
    g_param_spec_boolean ("disable_xv", "Disable XV", "Disable Xv images",
                          TRUE, G_PARAM_READWRITE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_TOPLEVEL,
    g_param_spec_boolean ("toplevel", "Toplevel", "Create a toplevel window",
                          TRUE, G_PARAM_READWRITE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_AUTOSIZE,
    g_param_spec_boolean ("auto_size", "Auto Size", "Resizes the window to negotiated size",
                          TRUE, G_PARAM_READWRITE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_NEED_NEW_WINDOW,
    g_param_spec_boolean ("need_new_window", 
	                  "Needs a new X window", 
			  "Request that a new X window be created for embedding",
                          TRUE, G_PARAM_WRITABLE)); 

  gobject_class->set_property = gst_xvideosink_set_property;
  gobject_class->get_property = gst_xvideosink_get_property;

  gst_xvideosink_signals[SIGNAL_FRAME_DISPLAYED] =
    g_signal_new ("frame_displayed", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET (GstXVideoSinkClass, frame_displayed), NULL, NULL,
                   g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
  gst_xvideosink_signals[SIGNAL_HAVE_SIZE] =
    g_signal_new ("have_size", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET (GstXVideoSinkClass, have_size), NULL, NULL,
                   gst_marshal_VOID__INT_INT, G_TYPE_NONE, 2,
                   G_TYPE_INT, G_TYPE_INT);
  gst_xvideosink_signals[SIGNAL_HAVE_XID] =
    g_signal_new ("have_xid", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET (GstXVideoSinkClass, have_xid), NULL, NULL,
                   gst_marshal_VOID__INT, G_TYPE_NONE, 1,
                   G_TYPE_INT);

  gobject_class->dispose = gst_xvideosink_dispose;
  
  gstelement_class->change_state = gst_xvideosink_change_state;
  gstelement_class->set_clock = gst_xvideosink_set_clock;
  gstelement_class->release_locks = gst_xvideosink_release_locks;

}

static void
gst_xvideosink_imagepool_clear (GstXVideoSink *xvideosink)
{
  g_mutex_lock(xvideosink->pool_lock);
  while (xvideosink->image_pool) {
    GstImage *image = GST_IMAGE (xvideosink->image_pool->data);

    xvideosink->image_pool = g_slist_delete_link (xvideosink->image_pool, xvideosink->image_pool);

    _gst_image_destroy (image);
  }
  g_mutex_unlock(xvideosink->pool_lock);
}

G_GNUC_UNUSED static void
gst_xvideosink_buffer_pool_free (GstBufferPool *pool)
{
  GstXVideoSink *xvideosink = pool->user_data;
	
  gst_xvideosink_imagepool_clear (xvideosink);

  gst_buffer_pool_default_free (pool);
}

static GstBuffer*
gst_xvideosink_buffer_new (GstBufferPool *pool,  
		           gint64 location, guint size, gpointer user_data)
{
  GstXVideoSink *xvideosink;
  GstBuffer *buffer;
  GstImage *image;
  
  xvideosink = GST_XVIDEOSINK (user_data);

  g_mutex_lock (xvideosink->pool_lock);
  if (!xvideosink->image_pool) {
    g_mutex_unlock (xvideosink->pool_lock);

    /* we have to lock the X connection */
    g_mutex_lock (xvideosink->lock);
    if (FORMAT_IS_RGB (xvideosink->format)) {
      image = GST_IMAGE (_gst_ximage_new ( 
		         xvideosink->window, 
		         xvideosink->width, 
			 xvideosink->height));
    }
    else {
      image = GST_IMAGE (_gst_xvimage_new ( 
		         xvideosink->format,
		         xvideosink->window, 
		         xvideosink->width, 
			 xvideosink->height));
    }
    g_mutex_unlock (xvideosink->lock);
  }
  else {
    image = xvideosink->image_pool->data;
    xvideosink->image_pool = g_slist_delete_link (xvideosink->image_pool, xvideosink->image_pool);
    g_mutex_unlock (xvideosink->pool_lock);
  }
  if (image == NULL) {
    gst_element_error(GST_ELEMENT (xvideosink), "image creation failed");
    /* FIXME: need better error handling here */
    return NULL;
  }

  buffer = gst_buffer_new ();
  GST_BUFFER_POOL_PRIVATE (buffer) = image;
  GST_BUFFER_DATA (buffer) = GST_IMAGE_DATA (image);
  GST_BUFFER_SIZE (buffer) = GST_IMAGE_SIZE (image);

  return buffer;
}

static void
gst_xvideosink_buffer_free (GstBufferPool *pool, GstBuffer *buffer, gpointer user_data)
{
  GstXVideoSink *xvideosink;
  gboolean keep_buffer = FALSE;
  GstImage *image;

  xvideosink = GST_XVIDEOSINK (user_data);

  /* get the image from the private data */
  image = GST_BUFFER_POOL_PRIVATE (buffer);

  /* the question is, to be or not to be, or rather: do the settings of the buffer still apply? */
  g_mutex_lock (xvideosink->lock);

  if (xvideosink->image) {
    gint image_type, image_size;

    image_type = GST_IMAGE_TYPE (image);
    image_size = GST_IMAGE_SIZE (image);
	    
    /* if size and type matches, keep the buffer */
    if (image_type == GST_IMAGE_TYPE (xvideosink->image) &&
        image_size == GST_IMAGE_SIZE (xvideosink->image)) 
    { 
      keep_buffer = TRUE;
    } 
  }
  g_mutex_unlock(xvideosink->lock);

  if (keep_buffer) {
    g_mutex_lock (xvideosink->pool_lock);
    xvideosink->image_pool = g_slist_prepend (xvideosink->image_pool, image);
    g_mutex_unlock (xvideosink->pool_lock);
  } else {
    _gst_image_destroy (image);
  }

  GST_BUFFER_DATA (buffer) = NULL;

  gst_buffer_default_free (buffer);
}

static GstBufferPool*
gst_xvideosink_get_bufferpool (GstPad *pad)
{
  GstXVideoSink *xvideosink;
  
  xvideosink = GST_XVIDEOSINK (gst_pad_get_parent (pad));

  if (!xvideosink->bufferpool) {
    if (FORMAT_IS_RGB (xvideosink->format)) {
      GST_DEBUG (0, "xvideosink: creating RGB XImage bufferpool");
    }
    else {
      GST_DEBUG (0, "xvideosink: creating YUV XvImage bufferpool");
    }

    xvideosink->bufferpool = gst_buffer_pool_new (
		    NULL,		/* free */
		    NULL,		/* copy */
		    (GstBufferPoolBufferNewFunction)gst_xvideosink_buffer_new,
		    NULL,		/* buffer copy, the default is fine */
		    (GstBufferPoolBufferFreeFunction)gst_xvideosink_buffer_free,
		    xvideosink);

    xvideosink->image_pool = NULL;
  }

  gst_buffer_pool_ref (xvideosink->bufferpool);

  return xvideosink->bufferpool;
}

static void
gst_xvideosink_get_real_size (GstXVideoSink *xvideosink, 
		              gint *real_x, gint *real_y)
{
  gint pwidth, pheight;

  *real_x = xvideosink->width;
  *real_y = xvideosink->height;
  pwidth  = xvideosink->pixel_width;
  pheight = xvideosink->pixel_height;

  if (pwidth && pheight) {
    if (pwidth > pheight) {
      *real_x = (xvideosink->width * pwidth) / pheight;
    }
    else if (pwidth < pheight) {
      *real_y = (xvideosink->height * pheight) / pwidth;
    }
  }
}

static GstPadLinkReturn
gst_xvideosink_sinkconnect (GstPad *pad, GstCaps *caps)
{
  GstXVideoSink *xvideosink;
  gulong print_format;

  xvideosink = GST_XVIDEOSINK (gst_pad_get_parent (pad));

  /* we are not going to act on variable caps */
  if (!GST_CAPS_IS_FIXED (caps))
    return GST_PAD_LINK_DELAYED;

  gst_caps_get (caps, 
		  "format", &xvideosink->format,
		  "width",  &xvideosink->width,
		  "height", &xvideosink->height,
		  NULL);

  if (gst_caps_has_fixed_property (caps, "pixel_width")) 
    gst_caps_get_int (caps, "pixel_width", &xvideosink->pixel_width);
  else 
    xvideosink->pixel_width = 1;

  if (gst_caps_has_fixed_property (caps, "pixel_height")) 
    gst_caps_get_int (caps, "pixel_height", &xvideosink->pixel_height);
  else 
    xvideosink->pixel_height = 1;

  print_format = GULONG_FROM_LE (xvideosink->format);

  GST_DEBUG (0, "xvideosink: setting %08x (%4.4s) %dx%d\n", 
		  xvideosink->format, (gchar*)&print_format, 
		  xvideosink->width, xvideosink->height);

  g_mutex_lock (xvideosink->lock);

  /* if we have any old image settings lying around, clean that up first */
  if (xvideosink->image) {
    _gst_image_destroy (xvideosink->image);
    xvideosink->image = NULL;

    /* clear our imagepool too */
    gst_xvideosink_imagepool_clear (xvideosink);
  }

  /* create a new image to draw onto */
  if (FORMAT_IS_RGB (xvideosink->format)) {
    xvideosink->image = GST_IMAGE (_gst_ximage_new ( 
		      xvideosink->window, 
		      xvideosink->width, xvideosink->height));
  } else {
    if (xvideosink->disable_xv) {
      g_mutex_unlock (xvideosink->lock);
      return GST_PAD_LINK_REFUSED;
    }

    xvideosink->image = GST_IMAGE (_gst_xvimage_new ( 
		      xvideosink->format,
		      xvideosink->window, 
		      xvideosink->width, xvideosink->height));
  }

  if (xvideosink->image == NULL) {
    /* FIXME: need better error handling? */
    gst_element_error (GST_ELEMENT (xvideosink), "image creation failed");
    return GST_PAD_LINK_REFUSED;
  }

  g_mutex_unlock (xvideosink->lock);

  {
    gint real_x, real_y;

    gst_xvideosink_get_real_size (xvideosink, &real_x, &real_y);
    
    if (xvideosink->auto_size) {
      _gst_xwindow_resize (xvideosink->window, real_x, real_y);
    }
    g_signal_emit (G_OBJECT (xvideosink), gst_xvideosink_signals[SIGNAL_HAVE_SIZE], 0,
		    real_x, real_y);
  }

  return GST_PAD_LINK_OK;
}

static GstCaps *
gst_xvideosink_getcaps (GstPad *pad, GstCaps *caps)
{
  GstXVideoSink *xvideosink;

  xvideosink = GST_XVIDEOSINK (gst_pad_get_parent (pad));

  return xvideosink->formats;
}

static void
gst_xvideosink_init (GstXVideoSink *xvideosink)
{
  xvideosink->sinkpad = gst_pad_new_from_template (sink_template, "sink");
  gst_element_add_pad (GST_ELEMENT (xvideosink), xvideosink->sinkpad);
  gst_pad_set_chain_function (xvideosink->sinkpad, gst_xvideosink_chain);
  gst_pad_set_link_function (xvideosink->sinkpad, gst_xvideosink_sinkconnect);
  gst_pad_set_getcaps_function (xvideosink->sinkpad, gst_xvideosink_getcaps);
  gst_pad_set_bufferpool_function (xvideosink->sinkpad, gst_xvideosink_get_bufferpool);

  xvideosink->toplevel = TRUE;
  xvideosink->width = 100;
  xvideosink->height = 100;
  xvideosink->formats = formats;
  xvideosink->lock = g_mutex_new();
  xvideosink->disable_xv = FALSE;
  xvideosink->auto_size = TRUE;
  xvideosink->correction = 0;

  xvideosink->image_pool = NULL;
  xvideosink->pool_lock = g_mutex_new ();
  xvideosink->clock = NULL;
  xvideosink->send_xid = FALSE;
  xvideosink->need_new_window = FALSE;

  xvideosink->image = NULL;
  
  GST_FLAG_SET(xvideosink, GST_ELEMENT_THREAD_SUGGESTED);
}

static void
gst_xvideosink_set_clock (GstElement *element, GstClock *clock)
{
  GstXVideoSink *xvideosink;

  xvideosink = GST_XVIDEOSINK (element);
  
  xvideosink->clock = clock;
}

static void
gst_xvideosink_dispose (GObject *object)
{
  GstXVideoSink *xvideosink;

  xvideosink = GST_XVIDEOSINK (object);

  if (xvideosink->image) {
    _gst_image_destroy (GST_IMAGE (xvideosink->image));
  }
  
  g_mutex_free (xvideosink->lock);
  g_mutex_free (xvideosink->pool_lock);

  if (xvideosink->bufferpool) 
    gst_buffer_pool_free (xvideosink->bufferpool);

  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gst_xvideosink_chain (GstPad *pad, GstBuffer *buf)
{
  GstXVideoSink *xvideosink;
  GstClockTime time = GST_BUFFER_TIMESTAMP (buf);
  gint64 jitter;

  g_return_if_fail (pad != NULL);
  g_return_if_fail (GST_IS_PAD (pad));
  g_return_if_fail (buf != NULL);

  xvideosink = GST_XVIDEOSINK (gst_pad_get_parent (pad));

  if (GST_IS_EVENT (buf)) {
    GstEvent *event = GST_EVENT (buf);
    gint64 offset;

    switch (GST_EVENT_TYPE (event)) {
      case GST_EVENT_DISCONTINUOUS:
	offset = GST_EVENT_DISCONT_OFFSET (event, 0).value;
	g_print ("xvideo discont %lld\n", offset);
	gst_clock_handle_discont (xvideosink->clock, (guint64) offset);
	break;
      default:
	gst_pad_event_default (pad, event);
	break;
    }
    gst_event_unref (event);
    return;
  }

  if (xvideosink->clock && time != -1) {
    GstClockReturn ret;

    xvideosink->id = gst_clock_new_single_shot_id (xvideosink->clock, time);

    GST_DEBUG (0,"videosink: clock %s wait: %llu %u\n", 
		  GST_OBJECT_NAME (xvideosink->clock), time, GST_BUFFER_SIZE (buf));

    ret = gst_clock_id_wait (xvideosink->id, &jitter);
    gst_clock_id_free (xvideosink->id);
    xvideosink->id = NULL;
  }
  if (xvideosink->clock)
    time = gst_clock_get_time (xvideosink->clock);

  g_mutex_lock (xvideosink->lock);
  /* if we have a pool and the image is from this pool, simply _put it */
  if (xvideosink->bufferpool && GST_BUFFER_BUFFERPOOL (buf) == xvideosink->bufferpool) {
    _gst_image_put (xvideosink->window, GST_IMAGE (GST_BUFFER_POOL_PRIVATE (buf)));
  }
  else {
    /* else we have to copy the data into our private image, if we have one... */
    if (xvideosink->image) {
      memcpy (GST_IMAGE_DATA (xvideosink->image), 
	      GST_BUFFER_DATA (buf), 
	      MIN (GST_BUFFER_SIZE (buf), GST_IMAGE_SIZE (xvideosink->image)));

      _gst_image_put (xvideosink->window, xvideosink->image);
    }
    else {
      g_mutex_unlock (xvideosink->lock);
      gst_buffer_unref (buf);
      gst_element_error (GST_ELEMENT (xvideosink), "no image to draw (are you feeding me video?)");
      return;
    }
  }
  xvideosink->frames_displayed++;
  g_mutex_unlock (xvideosink->lock);

  if (xvideosink->clock) {
    jitter = gst_clock_get_time (xvideosink->clock) - time;

    xvideosink->correction = (xvideosink->correction + jitter) >> 1;
    xvideosink->correction = 0;
  }

  if (xvideosink->send_xid){
    g_signal_emit (G_OBJECT (xvideosink), gst_xvideosink_signals[SIGNAL_HAVE_XID], 
		    0, GST_XWINDOW_XID (xvideosink->window));
    xvideosink->send_xid = FALSE;
  }
  g_signal_emit (G_OBJECT (xvideosink), gst_xvideosink_signals[SIGNAL_FRAME_DISPLAYED], 0);


  gst_buffer_unref (buf);
}


static void
gst_xvideosink_set_property (GObject *object, guint prop_id, 
		             const GValue *value, GParamSpec *pspec)
{
  GstXVideoSink *xvideosink;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_XVIDEOSINK (object));

  xvideosink = GST_XVIDEOSINK (object);

  switch (prop_id) {
    case ARG_FRAMES_DISPLAYED:
      xvideosink->frames_displayed = g_value_get_int (value);
      break;
    case ARG_FRAME_TIME:
      xvideosink->frame_time = g_value_get_int (value);
      break;
    case ARG_DISABLE_XV:
      xvideosink->disable_xv = g_value_get_boolean (value);
      xvideosink->formats = gst_xvideosink_get_pad_template_caps (!xvideosink->disable_xv);

      /* FIXME */
      sink_template = gst_pad_template_new (
		  "sink",
                  GST_PAD_SINK,
  		  GST_PAD_ALWAYS,
		  xvideosink->formats, NULL);

      GST_PAD_PAD_TEMPLATE (xvideosink->sinkpad) = sink_template;
      break;
    case ARG_TOPLEVEL:
      xvideosink->toplevel = g_value_get_boolean (value);
      break;
    case ARG_AUTOSIZE:
      xvideosink->auto_size = g_value_get_boolean (value);
      break;
    case ARG_NEED_NEW_WINDOW:
      if (g_value_get_boolean (value)) {
        g_mutex_lock (xvideosink->lock);
        /* FIXME destroying the window causes a crash, so we'll let it leak for now */
        /*_gst_xwindow_destroy (xvideosink->window);*/
        xvideosink->window = _gst_xwindow_new (xvideosink->width, 
			                       xvideosink->height, 
					       xvideosink->toplevel);
        if (!xvideosink->window) {
	  gst_element_error (GST_ELEMENT (xvideosink), "could not create X window");
	  /* FIXME: could use better error handling here */
          g_mutex_unlock (xvideosink->lock);
	  break;
        }
        xvideosink->send_xid = TRUE;
        xvideosink->need_new_window = FALSE;

        g_signal_emit (G_OBJECT (xvideosink), 
		       gst_xvideosink_signals[SIGNAL_HAVE_SIZE], 0,
		       xvideosink->width, xvideosink->height);

        g_mutex_unlock (xvideosink->lock);
      }
      break;
    default:
      break;
  }
}

static void
gst_xvideosink_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GstXVideoSink *xvideosink;

  /* it's not null if we got it, but it might not be ours */
  xvideosink = GST_XVIDEOSINK(object);

  switch (prop_id) {
    case ARG_XID: {
      if (xvideosink->window)
        g_value_set_int (value, GST_XWINDOW_XID (xvideosink->window));
      else
        g_value_set_int (value, 0);
      break;
    }
    case ARG_FRAMES_DISPLAYED: {
      g_value_set_int (value, xvideosink->frames_displayed);
      break;
    }
    case ARG_FRAME_TIME: {
      g_value_set_int (value, xvideosink->frame_time / GST_SECOND);
      break;
    }
    case ARG_DISABLE_XV:
      g_value_set_boolean (value, xvideosink->disable_xv);
      break;
    case ARG_TOPLEVEL:
      g_value_set_boolean (value, xvideosink->toplevel);
      break;
    case ARG_AUTOSIZE:
      g_value_set_boolean (value, xvideosink->auto_size);
      break;
    default: {
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
  }
}

static GstCaps*
gst_xvideosink_get_pad_template_caps (gboolean with_xv)
{
  GstXWindow *window;
  GstXImage *ximage;
  GstCaps *caps = NULL;

  window = _gst_xwindow_new (100, 100, FALSE);
  if (window == NULL) {
    return NULL;
  }
  
  ximage = _gst_ximage_new (window, 100, 100);
  if (ximage) {
    caps = GST_CAPS_NEW (
	     "xvideosink_caps",
	     "video/raw",
	      "format",       GST_PROPS_FOURCC (GST_MAKE_FOURCC ('R', 'G', 'B', ' ')),
	        "bpp",        GST_PROPS_INT (GST_XIMAGE_BPP (ximage)),
	        "depth",      GST_PROPS_INT (GST_XIMAGE_DEPTH (ximage)),
	        "endianness", GST_PROPS_INT (GST_XIMAGE_ENDIANNESS (ximage)),
	        "red_mask",   GST_PROPS_INT (GST_XIMAGE_RED_MASK (ximage)),
	        "green_mask", GST_PROPS_INT (GST_XIMAGE_GREEN_MASK (ximage)),
	        "blue_mask",  GST_PROPS_INT (GST_XIMAGE_BLUE_MASK (ximage)),
	        "width",      GST_PROPS_INT_RANGE (0, G_MAXINT),
	        "height",     GST_PROPS_INT_RANGE (0, G_MAXINT)
	    );

    _gst_image_destroy (GST_IMAGE (ximage));
  }

  if (with_xv && _gst_xvimage_check_xvideo ()) {
    _gst_xvimage_init();

    caps = gst_caps_prepend (caps, _gst_xvimage_get_capslist ());
  }

  _gst_xwindow_destroy (window);

  return caps;
}

static GstElementStateReturn
gst_xvideosink_change_state (GstElement *element)
{
  GstXVideoSink *xvideosink;

  xvideosink = GST_XVIDEOSINK (element);

  switch (GST_STATE_TRANSITION (element)) {
    case GST_STATE_NULL_TO_READY:
      xvideosink->window = _gst_xwindow_new (xvideosink->width, 
		                             xvideosink->height, 
					     xvideosink->toplevel);
      if (!xvideosink->window) {
	gst_element_error (element, "could not create X window");
	return GST_STATE_FAILURE;
      }
      xvideosink->send_xid = TRUE;
      break;
    case GST_STATE_READY_TO_PAUSED:
      xvideosink->correction = 0;
      break;
    case GST_STATE_PAUSED_TO_PLAYING:
      break;
    case GST_STATE_PLAYING_TO_PAUSED:
      break;
    case GST_STATE_PAUSED_TO_READY:
      break;
    case GST_STATE_READY_TO_NULL:
      if (xvideosink->bufferpool)
        gst_buffer_pool_unref (xvideosink->bufferpool);

      xvideosink->bufferpool = NULL;

      if (xvideosink->window) {
	if (xvideosink->image) {
          _gst_image_destroy (xvideosink->image);

	  xvideosink->image = NULL;
	}
	_gst_xwindow_destroy (xvideosink->window);
	xvideosink->window = NULL;
      }
      break;
  }

  parent_class->change_state (element);

  return GST_STATE_SUCCESS;
}

static gboolean
gst_xvideosink_release_locks (GstElement *element)
{
  GstXVideoSink *xvideosink;

  xvideosink = GST_XVIDEOSINK (element);

  if (xvideosink->id) {
    gst_clock_id_unlock (xvideosink->id);
  }

  return TRUE;
}

static gboolean
plugin_init (GModule *module, GstPlugin *plugin)
{
  GstElementFactory *factory;

  /* create an elementfactory for the xvideosink element */
  factory = gst_element_factory_new("xvideosink",GST_TYPE_XVIDEOSINK,
                                   &gst_xvideosink_details);
  g_return_val_if_fail(factory != NULL, FALSE);

  formats = gst_xvideosink_get_pad_template_caps (TRUE);

  sink_template = gst_pad_template_new (
		  "sink",
                  GST_PAD_SINK,
  		  GST_PAD_ALWAYS,
		  formats, NULL);

  gst_element_factory_add_pad_template (factory, sink_template);

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "xvideosink",
  plugin_init
};

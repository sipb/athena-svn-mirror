/* GStreamer
 * OSX video sink
 * Copyright (C) <2004> Zaheer Abbas Merali <zaheerabbas@merali.org>
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

#include "config.h"

/* Object header */
#include "osxvideosink.h"

#import "cocoawindow.h"

/* Debugging category */
#include <gst/gstinfo.h>
GST_DEBUG_CATEGORY_STATIC (gst_debug_osxvideosink);
#define GST_CAT_DEFAULT gst_debug_osxvideosink

static void gst_osxvideosink_buffer_free (GstBuffer * buffer);

/* ElementFactory information */
static GstElementDetails gst_osxvideosink_details =
GST_ELEMENT_DETAILS ("Video sink",
    "Sink/Video",
    "OSX native videosink",
    "Zaheer Abbas Merali <zaheerabbas at merali.org>");

/* Default template - initiated with class struct to allow gst-register to work
   without X running */
static GstStaticPadTemplate gst_osxvideosink_sink_template_factory =
GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("video/x-raw-yuv, "
        "framerate = (double) [ 1.0, 100.0 ], "
        "width = (int) [ 1, MAX ], " "height = (int) [ 1, MAX ], "
        "format = (fourcc) YUY2"
)
    );

enum
{
  ARG_0,
  ARG_EMBED,
  ARG_FULLSCREEN
      /* FILL ME */
};

enum
{
  SIGNAL_VIEW_CREATED,
  LAST_SIGNAL
};

static GstVideoSinkClass *parent_class = NULL;
static guint gst_osxvideosink_signals[LAST_SIGNAL] = { 0 };

/* cocoa event loop - needed if not run in own app */
gpointer cocoa_event_loop(gpointer crap)
{
   GST_DEBUG("About to start cocoa event loop");
   [NSApp run];
 
   GST_DEBUG("Cocoa event loop ended");
   return NULL;
}

/* This function handles GstXImage creation depending on XShm availability */
static GstOSXImage *
gst_osxvideosink_osximage_new (GstOSXVideoSink * osxvideosink, gint width, gint height)
{
  GstOSXImage *osximage = NULL;

  g_return_val_if_fail (GST_IS_OSXVIDEOSINK (osxvideosink), NULL);
  osximage = g_new0 (GstOSXImage, 1);

  osximage->width = width;
  osximage->height = height;
  osximage->data = NULL;
  osximage->osxvideosink = osxvideosink;

  osximage->size =
      (2 * osximage->width * osximage->height)+1;

  osximage->data = g_malloc (osximage->size);

  return osximage;
}

/* This function destroys a GstXImage handling XShm availability */
static void
gst_osxvideosink_osximage_destroy (GstOSXVideoSink * osxvideosink, GstOSXImage * osximage)
{
  g_return_if_fail (osximage != NULL);
  g_return_if_fail (GST_IS_OSXVIDEOSINK (osxvideosink));
  
  /* If the destroyed image is the current one we destroy our reference too */
  if (osxvideosink->cur_image == osximage)
    osxvideosink->cur_image = NULL;

  if (osximage->data) g_free(osximage->data);

  g_free (osximage);
}

/* This function handles osx window creation */
static GstOSXWindow *
gst_osxvideosink_osxwindow_new (GstOSXVideoSink * osxvideosink, gint width, gint height)
{
  GstOSXWindow *osxwindow = NULL;
 
  g_return_val_if_fail (GST_IS_OSXVIDEOSINK (osxvideosink), NULL);

  osxwindow = g_new0 (GstOSXWindow, 1);

  osxwindow->width = width;
  osxwindow->height = height;
  osxwindow->internal = TRUE;

  if (osxvideosink->embed == FALSE) {
    NSAutoreleasePool *pool;
    NSRect rect;

    rect.origin.x=100.0;
    rect.origin.y=100.0;
    rect.size.width=(float)osxwindow->width;
    rect.size.height=(float)osxwindow->height;

    pool = [[NSAutoreleasePool alloc] init];
    [NSApplication sharedApplication];
    osxwindow->win = [[GstWindow alloc] initWithContentRect:rect styleMask: NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask backing: NSBackingStoreBuffered defer: NO screen: nil];
    [osxwindow->win makeKeyAndOrderFront:NSApp];
    osxwindow->gstview = [osxwindow->win gstView];
    if (osxvideosink->fullscreen) [osxwindow->gstview setFullScreen: YES];
    [pool release];
    g_thread_create(cocoa_event_loop, osxvideosink, FALSE, NULL);
  } else {
    /* Needs to be embedded */
    NSRect rect;
   
    rect.origin.x=0.0;
    rect.origin.y=0.0;
    rect.size.width=(float)osxwindow->width;
    rect.size.height=(float)osxwindow->height;
    osxwindow->gstview = [[GstGLView alloc] initWithFrame:rect];
    /* send signal */
    g_signal_emit (G_OBJECT(osxvideosink),
                   gst_osxvideosink_signals[SIGNAL_VIEW_CREATED], 0,
                   osxwindow->gstview); 
  }
  return osxwindow;
}

/* This function destroys a GstXWindow */
static void
gst_osxvideosink_osxwindow_destroy (GstOSXVideoSink * osxvideosink,
    GstOSXWindow * osxwindow)
{
  g_return_if_fail (osxwindow != NULL);
  g_return_if_fail (GST_IS_OSXVIDEOSINK (osxvideosink));


  g_free (osxwindow);
}

/* This function resizes a GstXWindow */
static void
gst_osxvideosink_osxwindow_resize (GstOSXVideoSink * osxvideosink, GstOSXWindow * osxwindow,
    guint width, guint height)
{
  g_return_if_fail (osxwindow != NULL);
  g_return_if_fail (GST_IS_OSXVIDEOSINK (osxvideosink));

  //SizeWindow (osxwindow->win, width, height, 1);
  osxwindow->width = width;
  osxwindow->height = height;

  /* Call relevant cocoa function to resize window */
}

static void
gst_osxvideosink_osxwindow_clear (GstOSXVideoSink * osxvideosink, GstOSXWindow * osxwindow)
{

  g_return_if_fail (osxwindow != NULL);
  g_return_if_fail (GST_IS_OSXVIDEOSINK (osxvideosink));

}
/*
static void
gst_osxvideosink_osxwindow_update_geometry (GstOSXVideoSink * osxvideosink,
    GstOSXWindow * osxwindow)
{

  g_return_if_fail (osxwindow != NULL);
  g_return_if_fail (GST_IS_OSXVIDEOSINK (osxvideosink));

}

static void
gst_osxvideosink_renegotiate_size (GstOSXVideoSink * osxvideosink)
{
  g_return_if_fail (GST_IS_OSXVIDEOSINK (osxvideosink));

  if (!osxvideosink->osxwindow)
    return;

  gst_osxvideosink_osxwindow_update_geometry (osxvideosink, osxvideosink->osxwindow);

  if (osxvideosink->sw_scaling_failed)
    return;

  if (osxvideosink->osxwindow->width <= 1 || osxvideosink->osxwindow->height <= 1)
    return;

  if (GST_PAD_IS_NEGOTIATING (GST_VIDEOSINK_PAD (osxvideosink)) ||
      !gst_pad_is_negotiated (GST_VIDEOSINK_PAD (osxvideosink)))
    return;


  if (GST_VIDEOSINK_WIDTH (osxvideosink) != osxvideosink->osxwindow->width ||
      GST_VIDEOSINK_HEIGHT (osxvideosink) != osxvideosink->osxwindow->height) {
    GstPadLinkReturn r;

    r = gst_pad_try_set_caps (GST_VIDEOSINK_PAD (osxvideosink),
        gst_caps_new_simple ("video/x-raw-yuv",
            "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('Y', 'U', 'Y', '2'),
            "width", G_TYPE_INT, osxvideosink->osxwindow->width,
            "height", G_TYPE_INT, osxvideosink->osxwindow->height,
            "framerate", G_TYPE_DOUBLE, osxvideosink->framerate, NULL));

    if ((r == GST_PAD_LINK_OK) || (r == GST_PAD_LINK_DONE)) {
      GST_VIDEOSINK_WIDTH (osxvideosink) = osxvideosink->osxwindow->width;
      GST_VIDEOSINK_HEIGHT (osxvideosink) = osxvideosink->osxwindow->height;

      if ((osxvideosink->osximage) &&
          ((GST_VIDEOSINK_WIDTH (osxvideosink) != osxvideosink->osximage->width) ||
              (GST_VIDEOSINK_HEIGHT (osxvideosink) !=
                  osxvideosink->osximage->height))) {
        gst_osxvideosink_osximage_destroy (osxvideosink, osxvideosink->osximage);

        osxvideosink->osximage = gst_osxvideosink_osximage_new (osxvideosink,
            GST_VIDEOSINK_WIDTH (osxvideosink),
            GST_VIDEOSINK_HEIGHT (osxvideosink));
      }
    } else {
      osxvideosink->sw_scaling_failed = TRUE;
    }
  }
}
*/

static void
gst_osxvideosink_imagepool_clear (GstOSXVideoSink * osxvideosink)
{
  g_mutex_lock (osxvideosink->pool_lock);

  while (osxvideosink->image_pool) {
    GstOSXImage *osximage = osxvideosink->image_pool->data;

    osxvideosink->image_pool = g_slist_delete_link (osxvideosink->image_pool,
        osxvideosink->image_pool);
    gst_osxvideosink_osximage_destroy (osxvideosink, osximage);
  }

  g_mutex_unlock (osxvideosink->pool_lock);
}

/* Element stuff */

static GstCaps *
gst_osxvideosink_fixate (GstPad * pad, const GstCaps * caps)
{
  GstStructure *structure;
  GstCaps *newcaps;

  if (gst_caps_get_size (caps) > 1)
    return NULL;

  newcaps = gst_caps_copy (caps);
  structure = gst_caps_get_structure (newcaps, 0);

  if (gst_caps_structure_fixate_field_nearest_int (structure, "width", 320)) {
    return newcaps;
  }
  if (gst_caps_structure_fixate_field_nearest_int (structure, "height", 240)) {
    return newcaps;
  }
  if (gst_caps_structure_fixate_field_nearest_double (structure, "framerate",
          30.0)) {
    return newcaps;
  }

  gst_caps_free (newcaps);
  return NULL;
}

static GstCaps *
gst_osxvideosink_getcaps (GstPad * pad)
{
  GstOSXVideoSink *osxvideosink;

  osxvideosink = GST_OSXVIDEOSINK (gst_pad_get_parent (pad));

  return gst_caps_from_string ("video/x-raw-yuv, "
      "format = (fourcc) YUY2, "
      "framerate = (double) [ 1, 100 ], "
      "width = (int) [ 0, MAX ], " "height = (int) [ 0, MAX ] ");
}

static GstPadLinkReturn
gst_osxvideosink_sink_link (GstPad * pad, const GstCaps * caps)
{
  GstOSXVideoSink *osxvideosink;
  gboolean ret;
  GstStructure *structure;

  osxvideosink = GST_OSXVIDEOSINK (gst_pad_get_parent (pad));

  GST_DEBUG (
      "sinkconnect  caps %" GST_PTR_FORMAT , 
      caps);

  structure = gst_caps_get_structure (caps, 0);
  ret = gst_structure_get_int (structure, "width",
      &(GST_VIDEOSINK_WIDTH (osxvideosink)));
  ret &= gst_structure_get_int (structure, "height",
      &(GST_VIDEOSINK_HEIGHT (osxvideosink)));
  ret &= gst_structure_get_double (structure,
      "framerate", &osxvideosink->framerate);
  if (!ret)
    return GST_PAD_LINK_REFUSED;

  osxvideosink->pixel_width = 1;
  gst_structure_get_int (structure, "pixel_width", &osxvideosink->pixel_width);

  osxvideosink->pixel_height = 1;
  gst_structure_get_int (structure, "pixel_height", &osxvideosink->pixel_height);

  /* Creating our window and our image */
  if (!osxvideosink->osxwindow) {
    osxvideosink->osxwindow = gst_osxvideosink_osxwindow_new (osxvideosink,
        GST_VIDEOSINK_WIDTH (osxvideosink), GST_VIDEOSINK_HEIGHT (osxvideosink));
    gst_osxvideosink_osxwindow_clear (osxvideosink, osxvideosink->osxwindow);
  }
  else {
    if (osxvideosink->osxwindow->internal)
      gst_osxvideosink_osxwindow_resize (osxvideosink, osxvideosink->osxwindow,
          GST_VIDEOSINK_WIDTH (osxvideosink), GST_VIDEOSINK_HEIGHT (osxvideosink));
  }

  if ((osxvideosink->osximage) && ((GST_VIDEOSINK_WIDTH (osxvideosink) != osxvideosink->osximage->width) || (GST_VIDEOSINK_HEIGHT (osxvideosink) != osxvideosink->osximage->height))) { /* We renew our ximage only if size changed */
    gst_osxvideosink_osximage_destroy (osxvideosink, osxvideosink->osximage);

    osxvideosink->osximage = gst_osxvideosink_osximage_new (osxvideosink,
        GST_VIDEOSINK_WIDTH (osxvideosink), GST_VIDEOSINK_HEIGHT (osxvideosink));
  } else if (!osxvideosink->osximage)       /* If no ximage, creating one */
    osxvideosink->osximage = gst_osxvideosink_osximage_new (osxvideosink,
        GST_VIDEOSINK_WIDTH (osxvideosink), GST_VIDEOSINK_HEIGHT (osxvideosink));

  return GST_PAD_LINK_OK;
}

static GstElementStateReturn
gst_osxvideosink_change_state (GstElement * element)
{
  GstOSXVideoSink *osxvideosink;

  osxvideosink = GST_OSXVIDEOSINK (element);

  switch (GST_STATE_TRANSITION (element)) {
    case GST_STATE_NULL_TO_READY:
      break;
    case GST_STATE_READY_TO_PAUSED:
      if (osxvideosink->osxwindow)
        gst_osxvideosink_osxwindow_clear (osxvideosink, osxvideosink->osxwindow);
      osxvideosink->time = 0;
      break;
    case GST_STATE_PAUSED_TO_PLAYING:
      break;
    case GST_STATE_PLAYING_TO_PAUSED:
      break;
    case GST_STATE_PAUSED_TO_READY:
      osxvideosink->framerate = 0;
      osxvideosink->sw_scaling_failed = FALSE;
      GST_VIDEOSINK_WIDTH (osxvideosink) = 0;
      GST_VIDEOSINK_HEIGHT (osxvideosink) = 0;
      break;
    case GST_STATE_READY_TO_NULL:
      if (osxvideosink->osximage) {
        gst_osxvideosink_osximage_destroy (osxvideosink, osxvideosink->osximage);
        osxvideosink->osximage = NULL;
      }

      if (osxvideosink->image_pool)
        gst_osxvideosink_imagepool_clear (osxvideosink);

      if (osxvideosink->osxwindow) {
        gst_osxvideosink_osxwindow_destroy (osxvideosink, osxvideosink->osxwindow);
        osxvideosink->osxwindow = NULL;
      }
      break;
  }

  if (GST_ELEMENT_CLASS (parent_class)->change_state)
    return GST_ELEMENT_CLASS (parent_class)->change_state (element);

  return GST_STATE_SUCCESS;
}

static void
gst_osxvideosink_chain (GstPad * pad, GstData * data)
{
  GstBuffer *buf = GST_BUFFER (data);
  GstOSXVideoSink *osxvideosink;

  g_return_if_fail (GST_IS_PAD (pad));
  g_return_if_fail (buf != NULL);

  osxvideosink = GST_OSXVIDEOSINK (gst_pad_get_parent (pad));

  if (GST_IS_EVENT (data)) {
    gst_pad_event_default (pad, GST_EVENT (data));
    return;
  }

  buf = GST_BUFFER (data);
  /* update time */
  if (GST_BUFFER_TIMESTAMP_IS_VALID (buf)) {
    osxvideosink->time = GST_BUFFER_TIMESTAMP (buf);
  }
  GST_DEBUG ("clock wait: %" GST_TIME_FORMAT, GST_TIME_ARGS (osxvideosink->time));

  if (GST_VIDEOSINK_CLOCK (osxvideosink)) {
    gst_element_wait (GST_ELEMENT (osxvideosink), osxvideosink->time);
  }

  /* If this buffer has been allocated using our buffer management we simply
     put the ximage which is in the PRIVATE pointer */
  char* viewdata = [osxvideosink->osxwindow->gstview getTextureBuffer];
  memcpy(viewdata, GST_BUFFER_DATA (buf), GST_BUFFER_SIZE(buf));
  [osxvideosink->osxwindow->gstview displayTexture];
  if (GST_BUFFER_FREE_DATA_FUNC (buf) != gst_osxvideosink_buffer_free) {
 
    if (!osxvideosink->osximage) {
     /* No image available. Something went wrong during capsnego ! */

      gst_buffer_unref (buf);
      GST_ELEMENT_ERROR (osxvideosink, CORE, NEGOTIATION, (NULL),
          ("no format defined before chain function"));
      return;
    }
  }

  /* set correct time for next buffer */
  if (!GST_BUFFER_TIMESTAMP_IS_VALID (buf) && osxvideosink->framerate > 0) {
    osxvideosink->time += GST_SECOND / osxvideosink->framerate;
  }

  gst_buffer_unref (buf);

}

/* Buffer management */

static void
gst_osxvideosink_buffer_free (GstBuffer * buffer)
{
  GstOSXVideoSink *osxvideosink;
  GstOSXImage *osximage;

  osximage = GST_BUFFER_PRIVATE (buffer);

  g_assert (GST_IS_OSXVIDEOSINK (osximage->osxvideosink));
  osxvideosink = osximage->osxvideosink;

  /* If our geometry changed we can't reuse that image. */
  if ((osximage->width != GST_VIDEOSINK_WIDTH (osxvideosink)) ||
      (osximage->height != GST_VIDEOSINK_HEIGHT (osxvideosink)))
    gst_osxvideosink_osximage_destroy (osxvideosink, osximage);
  else {                        /* In that case we can reuse the image and add it to our image pool. */

    g_mutex_lock (osxvideosink->pool_lock);
    osxvideosink->image_pool = g_slist_prepend (osxvideosink->image_pool, osximage);
    g_mutex_unlock (osxvideosink->pool_lock);
  }
}

static GstBuffer *
gst_osxvideosink_buffer_alloc (GstPad * pad, guint64 offset, guint size)
{
  GstOSXVideoSink *osxvideosink;
  GstBuffer *buffer;
  GstOSXImage *osximage = NULL;
  gboolean not_found = TRUE;

  osxvideosink = GST_OSXVIDEOSINK (gst_pad_get_parent (pad));

  g_mutex_lock (osxvideosink->pool_lock);

  /* Walking through the pool cleaning unsuable images and searching for a
     suitable one */
  while (not_found && osxvideosink->image_pool) {
    osximage = osxvideosink->image_pool->data;

    if (osximage) {
      /* Removing from the pool */
      osxvideosink->image_pool = g_slist_delete_link (osxvideosink->image_pool,
          osxvideosink->image_pool);

      if ((osximage->width != GST_VIDEOSINK_WIDTH (osxvideosink)) || (osximage->height != GST_VIDEOSINK_HEIGHT (osxvideosink))) {       /* This image is unusable. Destroying... */
        gst_osxvideosink_osximage_destroy (osxvideosink, osximage);
        osximage = NULL;
      } else {                  /* We found a suitable image */

        break;
      }
    }
  }

  g_mutex_unlock (osxvideosink->pool_lock);

  if (!osximage) {                /* We found no suitable image in the pool. Creating... */
    osximage = gst_osxvideosink_osximage_new (osxvideosink,
        GST_VIDEOSINK_WIDTH (osxvideosink), GST_VIDEOSINK_HEIGHT (osxvideosink));
  }

  if (osximage) {
    buffer = gst_buffer_new ();

    /* Storing some pointers in the buffer */
    GST_BUFFER_PRIVATE (buffer) = osximage;

    GST_BUFFER_DATA (buffer) = osximage->data;
    GST_BUFFER_FREE_DATA_FUNC (buffer) = gst_osxvideosink_buffer_free;
    GST_BUFFER_SIZE (buffer) = osximage->size;
    return buffer;
  } else
    return NULL;
}

/* =========================================== */
/*                                             */
/*              Init & Class init              */
/*                                             */
/* =========================================== */

static void
gst_osxvideosink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstOSXVideoSink *osxvideosink;

  g_return_if_fail (GST_IS_OSXVIDEOSINK (object));

  osxvideosink = GST_OSXVIDEOSINK (object);

  switch (prop_id) {
    case ARG_EMBED:
      osxvideosink->embed = g_value_get_boolean(value);
      break;
    case ARG_FULLSCREEN:
      osxvideosink->fullscreen = g_value_get_boolean(value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_osxvideosink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstOSXVideoSink *osxvideosink;

  g_return_if_fail (GST_IS_OSXVIDEOSINK (object));

  osxvideosink = GST_OSXVIDEOSINK (object);

  switch (prop_id) {
    case ARG_EMBED:
      g_value_set_boolean(value,osxvideosink->embed);
      break;
    case ARG_FULLSCREEN:
      g_value_set_boolean(value,osxvideosink->fullscreen);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_osxvideosink_dispose (GObject * object)
{
  GstOSXVideoSink *osxvideosink;

  osxvideosink = GST_OSXVIDEOSINK (object);

  g_mutex_free (osxvideosink->pool_lock);
  
  G_OBJECT_CLASS (parent_class)->dispose (object);
}

static void
gst_osxvideosink_init (GstOSXVideoSink * osxvideosink)
{
  GST_VIDEOSINK_PAD (osxvideosink) =
      gst_pad_new_from_template (gst_static_pad_template_get
      (&gst_osxvideosink_sink_template_factory), "sink");

  gst_element_add_pad (GST_ELEMENT (osxvideosink),
      GST_VIDEOSINK_PAD (osxvideosink));

  gst_pad_set_chain_function (GST_VIDEOSINK_PAD (osxvideosink),
      gst_osxvideosink_chain);
  gst_pad_set_link_function (GST_VIDEOSINK_PAD (osxvideosink),
      gst_osxvideosink_sink_link);
  gst_pad_set_getcaps_function (GST_VIDEOSINK_PAD (osxvideosink),
      gst_osxvideosink_getcaps);
  gst_pad_set_fixate_function (GST_VIDEOSINK_PAD (osxvideosink),
      gst_osxvideosink_fixate);
  gst_pad_set_bufferalloc_function (GST_VIDEOSINK_PAD (osxvideosink),
      gst_osxvideosink_buffer_alloc);

  osxvideosink->osxwindow = NULL;
  osxvideosink->osximage = NULL;
  osxvideosink->cur_image = NULL;

  osxvideosink->framerate = 0;

  osxvideosink->pixel_width = osxvideosink->pixel_height = 1;

  osxvideosink->image_pool = NULL;
  osxvideosink->pool_lock = g_mutex_new ();
  osxvideosink->sw_scaling_failed = FALSE;
  osxvideosink->embed = FALSE;
  osxvideosink->fullscreen = FALSE;
  GST_FLAG_SET (osxvideosink, GST_ELEMENT_THREAD_SUGGESTED);
  GST_FLAG_SET (osxvideosink, GST_ELEMENT_EVENT_AWARE);

}

static void
gst_osxvideosink_base_init (gpointer g_class)
{
  GstElementClass *element_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_set_details (element_class, &gst_osxvideosink_details);

  gst_element_class_add_pad_template (element_class,
      gst_static_pad_template_get (&gst_osxvideosink_sink_template_factory));
}

static void
gst_osxvideosink_class_init (GstOSXVideoSinkClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  parent_class = g_type_class_ref (GST_TYPE_VIDEOSINK);

  gobject_class->dispose = gst_osxvideosink_dispose;
  gobject_class->set_property = gst_osxvideosink_set_property;
  gobject_class->get_property = gst_osxvideosink_get_property;

  gstelement_class->change_state = gst_osxvideosink_change_state;

  g_object_class_install_property (gobject_class, ARG_EMBED,
      g_param_spec_boolean ("embed", "embed", "When enabled, it  "
          "can be embedded", FALSE,
          G_PARAM_READWRITE));
  g_object_class_install_property (gobject_class, ARG_FULLSCREEN,
      g_param_spec_boolean ("fullscreen", "fullscreen", "When enabled, the view  "
          "is fullscreen", FALSE,
          G_PARAM_READWRITE));
  gst_osxvideosink_signals[SIGNAL_VIEW_CREATED] = g_signal_new("view-created",
    G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_CLEANUP, G_STRUCT_OFFSET (GstOSXVideoSinkClass, view_created), NULL, NULL, g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);
}

/* ============================================================= */
/*                                                               */
/*                       Public Methods                          */
/*                                                               */
/* ============================================================= */

/* =========================================== */
/*                                             */
/*          Object typing & Creation           */
/*                                             */
/* =========================================== */

GType
gst_osxvideosink_get_type (void)
{
  static GType osxvideosink_type = 0;

  if (!osxvideosink_type) {
    static const GTypeInfo osxvideosink_info = {
      sizeof (GstOSXVideoSinkClass),
      gst_osxvideosink_base_init,
      NULL,
      (GClassInitFunc) gst_osxvideosink_class_init,
      NULL,
      NULL,
      sizeof (GstOSXVideoSink),
      0,
      (GInstanceInitFunc) gst_osxvideosink_init,
    };

    osxvideosink_type = g_type_register_static (GST_TYPE_VIDEOSINK,
        "GstOSXVideoSink", &osxvideosink_info, 0);

  }

  return osxvideosink_type;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
  /* Loading the library containing GstVideoSink, our parent object */
  if (!gst_library_load ("gstvideo"))
    return FALSE;

  if (!gst_element_register (plugin, "osxvideosink",
          GST_RANK_PRIMARY, GST_TYPE_OSXVIDEOSINK))
    return FALSE;

  GST_DEBUG_CATEGORY_INIT (gst_debug_osxvideosink, "osxvideosink", 0,
      "osxvideosink element");

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    "osxvideo",
    "OSX native video output plugin",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE, GST_ORIGIN)

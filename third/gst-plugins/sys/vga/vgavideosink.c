/* GStreamer VGA plugin
 * Copyright (C) 2001 Ronald Bultje <rbultje@ronald.bitfreak.net>
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
 *
 */

#include "vgavideosink.h"

/* elementfactory information */
static GstElementDetails gst_vgavideosink_details = {
  "Video sink",
  "Sink/Video",
  "LGPL",
  "A svgalib-based videosink",
  VERSION,
  "Ronald Bultje <rbultje@ronald.bitfreak.net>"
  "Zeeshan Ali <zak147@yahoo.com",
  "(C) 2003",
};

/* vgavideosink signals and args */
enum {
  SIGNAL_FRAME_DISPLAYED,
  LAST_SIGNAL
};


enum {
  ARG_0,
  ARG_WIDTH,
  ARG_HEIGHT,
  ARG_FRAMES_DISPLAYED,
  ARG_FRAME_TIME
};

#define RED_FACTOR 8
#define GREEN_FACTOR 4
#define BLUE_FACTOR 8

#define RED_MASK 0xF800
#define GREEN_MASK 0x07E0
#define BLUE_MASK 0x001F

GST_PAD_TEMPLATE_FACTORY (sink_template,
  "sink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
    "vgavideosink_caps",
    "video/raw",
    "format", 	  GST_PROPS_FOURCC(GST_MAKE_FOURCC('R','G','B',' ')),   
      "bpp", 	    GST_PROPS_INT(16),
      "depth", 	    GST_PROPS_INT(16),
      "endianness", GST_PROPS_INT (G_BYTE_ORDER),
      "red_mask",   GST_PROPS_INT (0xf800),
      "green_mask", GST_PROPS_INT (0x07e0),
      "blue_mask",  GST_PROPS_INT (0x001f),
      "width", 	    GST_PROPS_INT_RANGE (0, G_MAXINT),
      "height",     GST_PROPS_INT_RANGE (0, G_MAXINT)
  )
)
    
static void	gst_vgavideosink_class_init		(GstVGAVideoSinkClass *klass);
static void	gst_vgavideosink_init			(GstVGAVideoSink *vgavideosink);

static void	gst_vgavideosink_chain			(GstPad *pad, GstBuffer *buf);

static void		gst_vgavideosink_set_property	(GObject *object, guint prop_id, 
							 const GValue *value, GParamSpec *pspec);
static void		gst_vgavideosink_get_property	(GObject *object, guint prop_id, 
							 GValue *value, GParamSpec *pspec);
static void	gst_vgavideosink_close			(GstVGAVideoSink *vgavideosink);
static void
gst_vgavideosink_set_clock (GstElement *element, GstClock *clock);

static GstElementStateReturn gst_vgavideosink_change_state (GstElement *element);

static GstElementClass *parent_class = NULL;
static guint gst_vgavideosink_signals[LAST_SIGNAL] = { 0 };

GType
gst_vgavideosink_get_type (void)
{
  static GType vgavideosink_type = 0;

  if (!vgavideosink_type) {
    static const GTypeInfo vgavideosink_info = {
      sizeof(GstVGAVideoSinkClass),      
      NULL,
      NULL,
      (GClassInitFunc)gst_vgavideosink_class_init,
      NULL,
      NULL,
      sizeof(GstVGAVideoSink),
      0,
      (GInstanceInitFunc)gst_vgavideosink_init,
    };
    
    vgavideosink_type = g_type_register_static(GST_TYPE_ELEMENT, "GstVGAVideoSink", &vgavideosink_info, 0);
  }
  
  return vgavideosink_type;
}

static void
gst_vgavideosink_class_init (GstVGAVideoSinkClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)klass;

  parent_class = g_type_class_ref (GST_TYPE_ELEMENT);

  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_WIDTH,
    g_param_spec_int ("width", "width", "The width of the screen",
                      G_MININT, G_MAXINT, 0, G_PARAM_READABLE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_HEIGHT,
    g_param_spec_int ("height", "height", "The height of the screen",
                      G_MININT, G_MAXINT, 0, G_PARAM_READABLE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_FRAMES_DISPLAYED,
   g_param_spec_int ("frames_displayed", "frames_displayed", "frames_displayed",
                      G_MININT, G_MAXINT, 0, G_PARAM_READABLE));
  g_object_class_install_property (G_OBJECT_CLASS (klass), ARG_FRAME_TIME,
    g_param_spec_int ("frame_time", "frame_time", "frame_time",
                      G_MININT, G_MAXINT, 0, G_PARAM_READABLE));
  
  gobject_class->set_property = gst_vgavideosink_set_property;
  gobject_class->get_property = gst_vgavideosink_get_property;

  gst_vgavideosink_signals[SIGNAL_FRAME_DISPLAYED] =
    g_signal_new ("frame_displayed", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST,
                   G_STRUCT_OFFSET (GstVGAVideoSinkClass, frame_displayed), NULL, NULL,
                   g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);
  
  gstelement_class->change_state = gst_vgavideosink_change_state;
  gstelement_class->set_clock = gst_vgavideosink_set_clock;
}

static GstPadLinkReturn
gst_vgavideosink_sinkconnect (GstPad *pad, GstCaps *caps)
{
  GstVGAVideoSink *vgavideosink;

  vgavideosink = GST_VGAVIDEOSINK (gst_pad_get_parent (pad));

  if (!GST_CAPS_IS_FIXED (caps))
    return GST_PAD_LINK_DELAYED;

  gst_caps_get_int (caps, "width", &vgavideosink->width);
  gst_caps_get_int (caps, "height", &vgavideosink->height );

  return GST_PAD_LINK_OK;
}

static void
gst_vgavideosink_set_clock (GstElement *element, GstClock *clock)
{
  GstVGAVideoSink *vgavideosink;

  vgavideosink = GST_VGAVIDEOSINK (element);
  
  vgavideosink->clock = clock;
}

static void
gst_vgavideosink_init (GstVGAVideoSink *vgavideosink)
{
  vgavideosink->sinkpad = gst_pad_new_from_template (
		  GST_PAD_TEMPLATE_GET (sink_template), "sink");
  gst_element_add_pad (GST_ELEMENT (vgavideosink), vgavideosink->sinkpad);
  gst_pad_set_chain_function (vgavideosink->sinkpad, gst_vgavideosink_chain);
  gst_pad_set_link_function (vgavideosink->sinkpad, gst_vgavideosink_sinkconnect);

  vgavideosink->width = -1;
  vgavideosink->height = -1;
  vgavideosink->clock = NULL;
  
  GST_FLAG_SET(vgavideosink, GST_ELEMENT_THREAD_SUGGESTED);
}

static void
gst_vgavideosink_chain (GstPad *pad, GstBuffer *buf)
{
  GstVGAVideoSink *vgavideosink;
  //guint16 **data = (guint16 **)GST_BUFFER_DATA(buf);
  //guint16 *mem = (guint16 *)vga_getgraphmem();
  //gint size;
  //gint16 r,g,b;
  //guint i,j;

  g_return_if_fail (pad != NULL);
  g_return_if_fail (GST_IS_PAD (pad));
  g_return_if_fail (buf != NULL);

  vgavideosink = GST_VGAVIDEOSINK (gst_pad_get_parent (pad));

  GST_DEBUG (0,"videosink: clock wait: %llu", GST_BUFFER_TIMESTAMP(buf));

  if (vgavideosink->clock) {
    GstClockID id = gst_clock_new_single_shot_id (vgavideosink->clock, GST_BUFFER_TIMESTAMP (buf));

    GST_DEBUG (0, "vgavideosink: clock wait: %llu\n", GST_BUFFER_TIMESTAMP (buf));
    gst_element_clock_wait (GST_ELEMENT (vgavideosink), id, NULL);
    gst_clock_id_free (id);
  }
  
  /*for (i=0; i<vgavideosink->height; i++) {
     for (j=0; j<vgavideosink->width; j++) {
	r = ((data[i][j] & RED_MASK) >> 11) * RED_FACTOR;
	g = ((data[i][j] & GREEN_MASK) >> 5) * GREEN_FACTOR;
	b = (data[i][j] & BLUE_MASK) * BLUE_FACTOR;
	 
	//gl_setpixelrgb (j, i, r, g, b);
     }
  }*/
  
  //gl_copyscreen(vgavideosink->physicalscreen);
  
  //size = MIN (GST_BUFFER_SIZE (buf), vgavideosink->width * vgavideosink->height*2);

  /* FIXME copy over the buffer in chuncks of 64K using vga_setpage(gint p) */
  //memcpy (mem, data, 64000);

  g_signal_emit (G_OBJECT (vgavideosink), gst_vgavideosink_signals[SIGNAL_FRAME_DISPLAYED], 0);

  gst_buffer_unref(buf);
}


static void
gst_vgavideosink_set_property (GObject *object, guint prop_id, 
		             const GValue *value, GParamSpec *pspec)
{
  GstVGAVideoSink *vgavideosink;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail (GST_IS_VGAVIDEOSINK (object));

  vgavideosink = GST_VGAVIDEOSINK (object);
}

static void
gst_vgavideosink_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GstVGAVideoSink *vgavideosink;

  /* it's not null if we got it, but it might not be ours */
  vgavideosink = GST_VGAVIDEOSINK(object);

  switch (prop_id) {
    case ARG_FRAMES_DISPLAYED: {
      g_value_set_int (value, vgavideosink->frames_displayed);
      break;
    }
    case ARG_FRAME_TIME: {
      g_value_set_int (value, vgavideosink->frame_time / GST_SECOND);
      break;
    }
    default: {
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
  }
}

static gboolean
gst_vgavideosink_open (GstVGAVideoSink *vgavideosink)
{
  g_return_val_if_fail (!GST_FLAG_IS_SET (vgavideosink ,GST_VGAVIDEOSINK_OPEN), FALSE);

  vga_init(); 

  /*vga_setmode(G640x480x64K);
  gl_setcontextvga(G640x480x64K);
  
  vgavideosink->physicalscreen = gl_allocatecontext();
  gl_getcontext(vgavideosink->physicalscreen);
  gl_setcontextvgavirtual(G640x480x64K);
  vgavideosink->virtualscreen = gl_allocatecontext();*/
  
  GST_FLAG_SET (vgavideosink, GST_VGAVIDEOSINK_OPEN);

  return TRUE;
}

static void
gst_vgavideosink_close (GstVGAVideoSink *vgavideosink)
{
  g_return_if_fail (GST_FLAG_IS_SET (vgavideosink ,GST_VGAVIDEOSINK_OPEN));
 
  /*gl_clearscreen(0);
  vga_setmode(TEXT);*/
    
  GST_FLAG_UNSET (vgavideosink, GST_VGAVIDEOSINK_OPEN);
}

static GstElementStateReturn
gst_vgavideosink_change_state (GstElement *element)
{
  g_return_val_if_fail (GST_IS_VGAVIDEOSINK (element), GST_STATE_FAILURE);

  switch (GST_STATE_TRANSITION (element)) {
    case GST_STATE_READY_TO_NULL:
      if (GST_FLAG_IS_SET (element, GST_VGAVIDEOSINK_OPEN))
        gst_vgavideosink_close (GST_VGAVIDEOSINK (element));
      break;
    case GST_STATE_NULL_TO_READY:
      if (!GST_FLAG_IS_SET (element, GST_VGAVIDEOSINK_OPEN)) {
        if (!gst_vgavideosink_open (GST_VGAVIDEOSINK (element)))
          return GST_STATE_FAILURE;
      }
      break;
  }

  GST_ELEMENT_CLASS (parent_class)->change_state (element);

  return GST_STATE_SUCCESS;
}

static gboolean
plugin_init (GModule *module, GstPlugin *plugin)
{
  GstElementFactory *factory;

  /* create an elementfactory for the vgavideosink element */
  factory = gst_element_factory_new("vgavideosink",GST_TYPE_VGAVIDEOSINK,
                                   &gst_vgavideosink_details);
  g_return_val_if_fail(factory != NULL, FALSE);

  gst_element_factory_add_pad_template (factory, 
		  GST_PAD_TEMPLATE_GET (sink_template));

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "vgavideosink",
  plugin_init
};

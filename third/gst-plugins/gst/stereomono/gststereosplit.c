/* gststereosplit.c
 *
 * Based on stereo2mono: Zaheer Merali <zaheer@bellworldwide.net>
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
#include <gst/gst.h>

#define GST_TYPE_STEREOSPLIT \
  (gst_stereosplit_get_type())
#define GST_STEREOSPLIT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_STEREOSPLIT,GstStereoSplit))
#define GST_STEREOSPLIT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_ULAW,GstStereoSplit))
#define GST_IS_STEREOSPLIT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_STEREOSPLIT))
#define GST_IS_STEREOSPLIT_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_STEREOSPLIT))

typedef struct _GstStereoSplit GstStereoSplit;
typedef struct _GstStereoSplitClass GstStereoSplitClass;

struct _GstStereoSplit {
  GstElement element;

  GstPad *sinkpad,*srcpad1,*srcpad2;

  gint width;
};

struct _GstStereoSplitClass {
  GstElementClass parent_class;
};

GType gst_stereosplit_get_type(void);


static GstElementDetails stereosplit_details = {
  "Stereo splitter",
  "Filter/Audio/Conversion",
  "LGPL",
  "Convert stereo PCM to two mono PCM streams",
  VERSION,
  "Richard Boulton <richard@tartarus.org>",
  "(C) 2001",
};


/* Stereo signals and args */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

enum {
  ARG_0
};

static GstPadTemplate*
stereosplit_src_factory1 (void)
{
  return 
    gst_pad_template_new (
  	"src",
  	GST_PAD_SRC,
  	GST_PAD_ALWAYS,
  	gst_caps_new (
  	  "src1",
    	  "audio/raw",
	  gst_props_new (
            "format",       GST_PROPS_STRING ("int"),   
              "law",        GST_PROPS_INT (0),
              "endianness", GST_PROPS_INT (G_BYTE_ORDER),
              "signed",     GST_PROPS_BOOLEAN (TRUE),
              "width",      GST_PROPS_INT (16),
              "depth",      GST_PROPS_INT (16),
              "rate",       GST_PROPS_INT_RANGE (8000, 48000),
    	    "channels", GST_PROPS_INT (1),
	    NULL)),
	NULL);
}

static GstPadTemplate*
stereosplit_src_factory2 (void)
{
  return 
    gst_pad_template_new (
  	"src",
  	GST_PAD_SRC,
  	GST_PAD_ALWAYS,
  	gst_caps_new (
  	  "src2",
    	  "audio/raw",
	  gst_props_new (
            "format",       GST_PROPS_STRING ("int"),   
              "law",        GST_PROPS_INT (0),
              "endianness", GST_PROPS_INT (G_BYTE_ORDER),
              "signed",     GST_PROPS_BOOLEAN (TRUE),
              "width",      GST_PROPS_INT (16),
              "depth",      GST_PROPS_INT (16),
              "rate",       GST_PROPS_INT_RANGE (8000, 48000),
    	    "channels", GST_PROPS_INT (1),
	    NULL)),
	NULL);
}

static GstPadTemplate*
stereosplit_sink_factory (void) 
{
  return 
    gst_pad_template_new (
  	"sink",
  	GST_PAD_SINK,
  	GST_PAD_ALWAYS,
  	gst_caps_new (
  	  "int_stereo_sink",
    	  "audio/raw",
	  gst_props_new (
            "format",       GST_PROPS_STRING ("int"),   
              "law",        GST_PROPS_INT (0),
              "endianness", GST_PROPS_INT (G_BYTE_ORDER),
              "signed",     GST_PROPS_BOOLEAN (TRUE),
              "width",      GST_PROPS_INT (16),
              "depth",      GST_PROPS_INT (16),
              "rate",       GST_PROPS_INT_RANGE (8000, 48000),
    	    "channels",    GST_PROPS_INT (2),
	    NULL)),
	NULL);
}

static GstPadTemplate *srctempl1, *srctempl2, *sinktempl;

static void		gst_stereosplit_class_init		(GstStereoSplitClass *klass);
static void		gst_stereosplit_init			(GstStereoSplit *stereo);

static void		gst_stereosplit_set_property			(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void		gst_stereosplit_get_property			(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static void		gst_stereosplit_chain			(GstPad *pad, GstBuffer *buf);
static void inline 	gst_stereosplit_fast_16bit_chain 	(gint16* data,
								 gint16** mono_data1, 
								 gint16** mono_data2, 
			         				 guint numbytes);
static void inline 	gst_stereosplit_fast_8bit_chain		(gint8* data,
								 gint8** mono_data1,
								 gint8** mono_data2,
                                				 guint numbytes);

static GstElementClass *parent_class = NULL;
/*static guint gst_stereo_signals[LAST_SIGNAL] = { 0 }; */

static GstPadLinkReturn
gst_stereosplit_connect (GstPad *pad, GstCaps *caps)
{
  GstStereoSplit *filter;
  
  filter = GST_STEREOSPLIT (gst_pad_get_parent (pad));
  g_return_val_if_fail (filter != NULL, GST_PAD_LINK_REFUSED);
  g_return_val_if_fail (GST_IS_STEREOSPLIT (filter), GST_PAD_LINK_REFUSED);

  if (!GST_CAPS_IS_FIXED (caps)) {
      return GST_PAD_LINK_DELAYED;
  }

  if (pad == filter->sinkpad) {
      /* Set both sources to match. */
      GstCaps * srccaps;
      srccaps = gst_caps_copy (caps);
      gst_caps_set (srccaps, "channels", GST_PROPS_INT (1));
      if (gst_pad_try_set_caps (filter->srcpad1, srccaps) <= 0)
	        return GST_PAD_LINK_REFUSED;
      if (gst_pad_try_set_caps (filter->srcpad2, srccaps) <= 0)
	        return GST_PAD_LINK_REFUSED;
  } else {
      /* Set the sink and the other source to match. */
      GstCaps * srccaps;
      GstCaps * sinkcaps;
      GstPad * othersrcpad;

      srccaps = gst_caps_copy (caps);
      sinkcaps = gst_caps_copy (caps);
      othersrcpad = (pad == filter->srcpad1 ? filter->srcpad2 : filter->srcpad1);

      gst_caps_set (srccaps, "channels", GST_PROPS_INT (1));
      gst_caps_set (sinkcaps, "channels", GST_PROPS_INT (2));

      if (gst_pad_try_set_caps (othersrcpad, srccaps) <= 0)
	        return GST_PAD_LINK_REFUSED;
      if (gst_pad_try_set_caps (filter->sinkpad, sinkcaps) <= 0)
	        return GST_PAD_LINK_REFUSED;
  }

  gst_caps_get_int (caps, "width", &filter->width);

  return GST_PAD_LINK_OK;
}

GType
gst_stereosplit_get_type(void) {
  static GType stereosplit_type = 0;

  if (!stereosplit_type) {
    static const GTypeInfo stereosplit_info = {
      sizeof(GstStereoSplitClass),      NULL,
      NULL,
      (GClassInitFunc)gst_stereosplit_class_init,
      NULL,
      NULL,
      sizeof(GstStereoSplit),
      0,
      (GInstanceInitFunc)gst_stereosplit_init,
    };
    stereosplit_type = g_type_register_static(GST_TYPE_ELEMENT, "GstStereoSplit", &stereosplit_info, 0);
  }
  return stereosplit_type;
}

static void
gst_stereosplit_class_init (GstStereoSplitClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)klass;

  parent_class = g_type_class_ref(GST_TYPE_ELEMENT);

  gobject_class->set_property = gst_stereosplit_set_property;
  gobject_class->get_property = gst_stereosplit_get_property;
}

static void
gst_stereosplit_init (GstStereoSplit *stereo)
{
  stereo->sinkpad = gst_pad_new_from_template (sinktempl, "sink");
  gst_pad_set_chain_function (stereo->sinkpad, gst_stereosplit_chain);
  gst_pad_set_link_function (stereo->sinkpad, gst_stereosplit_connect);
  gst_element_add_pad (GST_ELEMENT (stereo), stereo->sinkpad);

  stereo->srcpad1 = gst_pad_new_from_template (srctempl1, "src1");
  gst_pad_set_link_function (stereo->srcpad1, gst_stereosplit_connect);
  gst_element_add_pad (GST_ELEMENT (stereo), stereo->srcpad1);

  stereo->srcpad2 = gst_pad_new_from_template (srctempl2, "src2");
  gst_pad_set_link_function (stereo->srcpad2, gst_stereosplit_connect);
  gst_element_add_pad (GST_ELEMENT (stereo), stereo->srcpad2);
}

static void
gst_stereosplit_chain (GstPad *pad,GstBuffer *buf)
{
  GstStereoSplit *stereo;
  gint16 *data;
  gint16 *mono_data1;
  gint16 *mono_data2;
  GstBuffer* outbuf1;
  GstBuffer* outbuf2;

  g_return_if_fail(pad != NULL);
  g_return_if_fail(GST_IS_PAD(pad));
  g_return_if_fail(buf != NULL);

  stereo = GST_STEREOSPLIT(GST_OBJECT_PARENT (pad));
  g_return_if_fail(stereo != NULL);
  g_return_if_fail(GST_IS_STEREOSPLIT(stereo));

  if (GST_IS_EVENT(buf)) {
    gst_pad_event_default (stereo->srcpad1, GST_EVENT (buf));
    gst_pad_event_default (stereo->srcpad2, GST_EVENT (buf));
    return;
  }
  
  data = (gint16 *)GST_BUFFER_DATA(buf);

  outbuf1=gst_buffer_new();
  GST_BUFFER_DATA(outbuf1) = (gchar*)g_new(gint16,GST_BUFFER_SIZE(buf)/4);
  GST_BUFFER_SIZE(outbuf1) = GST_BUFFER_SIZE(buf)/2;
  GST_BUFFER_OFFSET(outbuf1) = GST_BUFFER_OFFSET(buf);
  GST_BUFFER_TIMESTAMP(outbuf1) = GST_BUFFER_TIMESTAMP(buf);

  outbuf2=gst_buffer_new();
  GST_BUFFER_DATA(outbuf2) = (gchar*)g_new(gint16,GST_BUFFER_SIZE(buf)/4);
  GST_BUFFER_SIZE(outbuf2) = GST_BUFFER_SIZE(buf)/2;
  GST_BUFFER_OFFSET(outbuf2) = GST_BUFFER_OFFSET(buf);
  GST_BUFFER_TIMESTAMP(outbuf2) = GST_BUFFER_TIMESTAMP(buf);

  mono_data1 = (gint16*)GST_BUFFER_DATA(outbuf1);
  mono_data2 = (gint16*)GST_BUFFER_DATA(outbuf2);
  
  switch (stereo->width) {
    case 16:
      gst_stereosplit_fast_16bit_chain(data,&mono_data1,&mono_data2,GST_BUFFER_SIZE(buf));
      break;
    case 8:
      gst_stereosplit_fast_8bit_chain((gint8*)data,(gint8**)&mono_data1,(gint8**)&mono_data2,GST_BUFFER_SIZE(buf));
      break;
    default:
      gst_element_error (GST_ELEMENT (stereo), "stereosplit: capsnego was never performed, bailing...");
      return;
  }
  gst_buffer_unref(buf);
  gst_pad_push(stereo->srcpad1,outbuf1);
  gst_pad_push(stereo->srcpad2,outbuf2);
}

static void inline
gst_stereosplit_fast_16bit_chain(gint16* data,
				 gint16** mono_data1,
				 gint16** mono_data2,
			         guint numbytes)
{
  guint i,j;
  /*printf("s2m 16bit: data=0x%x numbytes=%u\n",data,numbytes); */
  for(i=0,j=0;i<numbytes/2;i+=2,j++) {
     (*mono_data1)[j]=data[i];
     (*mono_data2)[j]=data[i+1];
  }
}

static void inline
gst_stereosplit_fast_8bit_chain(gint8* data,
				gint8** mono_data1,
				gint8** mono_data2,
                                guint numbytes)
{
  guint i,j;
  for(i=0,j=0;i<numbytes;i+=2,j++) {
     /* average the 2 channels */
     (*mono_data1)[j]=data[i];
     (*mono_data2)[j]=data[i+1];
  }
}

static void
gst_stereosplit_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  GstStereoSplit *stereo;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail(GST_IS_STEREOSPLIT(object));
  stereo = GST_STEREOSPLIT(object);

  switch (prop_id) {
    default:
      break;
  }
}

static void
gst_stereosplit_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GstStereoSplit *stereo;

  /* it's not null if we got it, but it might not be ours */
  g_return_if_fail(GST_IS_STEREOSPLIT(object));
  stereo = GST_STEREOSPLIT(object);

  switch (prop_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
plugin_init (GModule *module, GstPlugin *plugin)
{
  GstElementFactory *factory;

  factory = gst_element_factory_new("stereosplit",GST_TYPE_STEREOSPLIT,
                                   &stereosplit_details);
  g_return_val_if_fail(factory != NULL, FALSE);
  
  srctempl1 = stereosplit_src_factory1 ();
  gst_element_factory_add_pad_template (factory, srctempl1);
  srctempl2 = stereosplit_src_factory2 ();
  gst_element_factory_add_pad_template (factory, srctempl2);

  sinktempl = stereosplit_sink_factory ();
  gst_element_factory_add_pad_template (factory, sinktempl);

  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "stereosplit",
  plugin_init
};


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


#include "gstwindec.h"
#include "gstwinenc.h"

GST_PAD_TEMPLATE_FACTORY (sink_factory_1,
  "sink",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
    "wincodec_sink",
    "video/avi",
      "format",		GST_PROPS_STRING ("strf_vids"),
      "width",     	GST_PROPS_INT_RANGE (16, 4096),
      "height",    	GST_PROPS_INT_RANGE (16, 4096)
  )
)


GST_PAD_TEMPLATE_FACTORY (src_factory_1,
  "src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
    "wincodec_src",
    "video/raw",
      "format",    	GST_PROPS_LIST (
       	           	  GST_PROPS_FOURCC (GST_STR_FOURCC ("I420")),
       	           	  GST_PROPS_FOURCC (GST_STR_FOURCC ("YV12")),
       	           	  GST_PROPS_FOURCC (GST_STR_FOURCC ("YUY2")),
               	   	  GST_PROPS_FOURCC (GST_STR_FOURCC ("RGB "))
               	   	),
      "width",     	GST_PROPS_INT_RANGE (16, 4096),
      "height",    	GST_PROPS_INT_RANGE (16, 4096)
  )
)

/* elementfactory information */
extern GstElementDetails gst_windec_details;
extern GstElementDetails gst_winenc_details;

GstPadTemplate *wincodec_src_temp;
GstPadTemplate *wincodec_sink_temp;

static gboolean
plugin_init (GModule *module, GstPlugin *plugin)
{
  GstElementFactory *dec = NULL;
  GstElementFactory *enc = NULL;

  /* create an elementfactory for the windec element */
  enc = gst_element_factory_new ("winenc",GST_TYPE_WINENC,
                                   &gst_winenc_details);
  g_return_val_if_fail(enc != NULL, FALSE); 
  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (enc));

  /* create an elementfactory for the windec element */
  dec = gst_element_factory_new("windec",GST_TYPE_WINDEC,
                                   &gst_windec_details);
  g_return_val_if_fail(dec != NULL, FALSE); 

  gst_element_factory_set_rank (dec, GST_ELEMENT_RANK_PRIMARY);

  wincodec_src_temp = GST_PAD_TEMPLATE_GET (src_factory_1);
  gst_element_factory_add_pad_template (dec, wincodec_src_temp);
  wincodec_sink_temp = GST_PAD_TEMPLATE_GET (sink_factory_1);
  gst_element_factory_add_pad_template (dec, wincodec_sink_temp);
  
  gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (dec));

  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "wincodec",
  plugin_init
};


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

/*#define DEBUG_ENABLED*/
#include <gst/gst.h>

static GstCaps* mpeg2system_type_find(GstBuffer *buf,gpointer private);
static GstCaps* mpeg2_type_find(GstBuffer *buf,gpointer private);

static GstTypeDefinition mpeg2type_definitions[] = {
  { "mpeg2types_video/mpeg;system", "video/mpeg", ".mpg .mpeg .mpe", mpeg2system_type_find },
/*  { "video/mpeg", ".mpg .mpeg .mpe", mpeg2_type_find },*/
  { "mpeg2types_video/mpeg", "video/mpeg", NULL, NULL },
  { NULL, NULL, NULL, NULL },
};

static GstCaps* 
mpeg2system_type_find (GstBuffer *buf, gpointer private) 
{
  gulong head = GULONG_FROM_BE(*((gulong *)GST_BUFFER_DATA(buf)));
  GstCaps *new;

  if (head  != 0x000001ba)
    return NULL;
  if ((*(GST_BUFFER_DATA(buf)+4) & 0xC0) != 0x40)
    return NULL;

  new = gst_caps_new (
		  "mpeg2system_type_find",
		  "video/mpeg",
                  gst_props_new (
                    "mpegversion",  GST_PROPS_INT (2),
                    "systemstream", GST_PROPS_BOOLEAN (TRUE),
                    NULL));
  return new;
}

static GstCaps* 
mpeg2_type_find (GstBuffer *buf, gpointer private) 
{
  gulong head = GULONG_FROM_BE(*((gulong *)GST_BUFFER_DATA(buf)));
  GstCaps *new;

  if (head  != 0x000001b3)
    return NULL;

  new = gst_caps_new (
		  "mpeg2_type_find",
		  "video/mpeg",
                  gst_props_new (
                    "mpegversion",  GST_PROPS_INT (2),
                    "systemstream", GST_PROPS_BOOLEAN (FALSE),
                     NULL));
  return new;
}

static gboolean
plugin_init (GModule *module, GstPlugin *plugin)
{
  gint i=0;

  while (mpeg2type_definitions[i].name) {
    GstTypeFactory *type;

    type = gst_type_factory_new (&mpeg2type_definitions[i]);
    gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (type));
    i++;
  }

/*  gst_info("gsttypes: loaded %d mpeg2 types\n",i);*/

  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "mpeg2types",
  plugin_init
};

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

/* First, include the header file for the plugin, to bring in the
 * object definition and other useful things.
 */
#include "gstxmms.h"
#include "xmms.h"
#include "plugin.h"
#include "pluginenum.h"
#include "gstxmmsinput.h"
#include "gstxmmseffect.h"

struct InputPluginData *ip_data;
struct OutputPluginData *op_data;
struct EffectPluginData *ep_data;
struct GeneralPluginData *gp_data;
struct VisPluginData *vp_data;

gint effects_enabled (void)
{
  return FALSE;
}

EffectPlugin *get_current_effect_plugin (void)
{
  return NULL;
}

gint ctrlsocket_get_session_id (void)
{
  return 0;
}


static gboolean
plugin_init (GModule *module, GstPlugin *plugin)
{
  init_plugins();

  gst_xmms_input_register (plugin, ip_data->input_list);
  gst_xmms_effect_register (plugin, ep_data->effect_list);

  /* Now we can return the pointer to the newly created Plugin object. */
  return TRUE;
}

GstPluginDesc plugin_desc = {
  GST_VERSION_MAJOR,
  GST_VERSION_MINOR,
  "xmms",
  plugin_init
};

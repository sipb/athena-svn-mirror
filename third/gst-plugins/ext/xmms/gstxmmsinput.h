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


#ifndef __GST_XMMS_INPUT_H__
#define __GST_XMMS_INPUT_H__

#include <gst/gst.h>

#include "plugin.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* This is the definition of the element's object structure. */
typedef struct _GstXmmsInput GstXmmsInput;

/* The structure itself is derived from GstElement, as can be seen by the
 * fact that there's a complete instance of the GstElement structure at
 * the beginning of the object.  This allows the element to be cast to
 * an Element or even an Object.
 */
struct _GstXmmsInput {
  GstElement element;

  /* We need to keep track of our pads, so we do so here. */
  GstPad *srcpad;

  /* We'll use this to decide whether to do anything to the data we get. */
  gboolean active;
  gchar *filename;
};

/* The other half of the object is its class.  The class also derives from
 * the same parent, though it must be the class structure this time.
 * Function pointers for polymophic methods and signals are placed in this
 * structure. */
typedef struct _GstXmmsInputClass GstXmmsInputClass;

struct _GstXmmsInputClass {
  GstElementClass parent_class;

  InputPlugin *in_plugin;

  /* signals */
  void (*asdf) (GstElement *element, GstXmmsInput *xmms_input);
};

/* Five standard preprocessing macros are used in the Gtk+ object system.
 * The first uses the object's _get_type function to return the GType
 * of the object.
 */
#define GST_TYPE_XMMS_INPUT \
  (gst_xmms_input_get_type())
/* The second is a checking cast to the correct type.  If the object passed
 * is not the right type, a warning will be generated on stderr.
 */
#define GST_XMMS_INPUT(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_XMMS_INPUT,GstXmmsInput))
/* The third is a checking cast of the class instead of the object. */
#define GST_XMMS_INPUT_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_XMMS_INPUT,GstXmmsInput))
/* The last two simply check to see if the passed pointer is an object or
 * class of the correct type. */
#define GST_IS_XMMS_INPUT(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_XMMS_INPUT))
#define GST_IS_XMMS_INPUT_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_XMMS_INPUT))

void gst_xmms_input_register (GstPlugin *plugin, GList *plugin_list);


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* __GST_XMMS_INPUT_H__ */

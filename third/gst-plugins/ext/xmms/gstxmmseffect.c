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
#include "gstxmmseffect.h"
#include "xmms.h"
#include "plugin.h"
#include "pluginenum.h"

/* These are the signals that this element can fire.  They are zero-
 * based because the numbers themselves are private to the object.
 * LAST_SIGNAL is used for initialization of the signal array.
 */
enum {
  ASDF,
  /* FILL ME */
  LAST_SIGNAL
};

/* Arguments are identified the same way, but cannot be zero, so you
 * must leave the ARG_0 entry in as a placeholder.
 */
enum {
  ARG_0,
  ARG_ACTIVE,
  /* FILL ME */
};

/* This factory is much simpler, and defines the source pad. */
GST_PAD_TEMPLATE_FACTORY (gst_xmms_src_factory,
  "src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
    "xmms_effect_src",
    "audio/raw",
      "format",       GST_PROPS_STRING ("int"),
        "law",        GST_PROPS_INT (0),
        "width",      GST_PROPS_LIST (
                        GST_PROPS_INT (8),
                        GST_PROPS_INT (16)
                      ),
	"depth",      GST_PROPS_LIST (
	                GST_PROPS_INT (8),
	                GST_PROPS_INT (16)
	              )
  )
)

/* This factory is much simpler, and defines the source pad. */
GST_PAD_TEMPLATE_FACTORY (gst_xmms_sink_factory,
  "src",
  GST_PAD_SINK,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
    "xmms_effect_sink",
    "audio/raw",
      "format",       GST_PROPS_STRING ("int"),
        "law",        GST_PROPS_INT (0),
        "width",      GST_PROPS_LIST (
                        GST_PROPS_INT (8),
                        GST_PROPS_INT (16)
                      ),
	"depth",      GST_PROPS_LIST (
	                GST_PROPS_INT (8),
	                GST_PROPS_INT (16)
	              )
  )
)

static GHashTable *global_plugins;

/* A number of functon prototypes are given so we can refer to them later. */
static void	gst_xmms_effect_class_init	(GstXmmsEffectClass *klass);
static void	gst_xmms_effect_init	(GstXmmsEffect *xmms_effect);

static void	gst_xmms_effect_chain	(GstPad *pad, GstBuffer *buffer);

static void	gst_xmms_effect_set_property	(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void	gst_xmms_effect_get_property	(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

/* The parent class pointer needs to be kept around for some object
 * operations.
 */
static GstElementClass *parent_class = NULL;

/* This array holds the ids of the signals registered for this object.
 * The array indexes are based on the enum up above.
 */
/*static guint gst_xmms_effect_signals[LAST_SIGNAL] = { 0 }; */


/* In order to create an instance of an object, the class must be
 * initialized by this function.  GObject will take care of running
 * it, based on the pointer to the function provided above.
 */
static void
gst_xmms_effect_class_init (GstXmmsEffectClass *klass)
{
  /* Class pointers are needed to supply pointers to the private
   * implementations of parent class methods.
   */
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  /* Since the xmms_effect class contains the parent classes, you can simply
   * cast the pointer to get access to the parent classes.
   */
  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)klass;

  /* The parent class is needed for class method overrides. */
  parent_class = g_type_class_ref(GST_TYPE_ELEMENT);

  klass->in_plugin = (EffectPlugin *) g_hash_table_lookup (global_plugins,
		  GINT_TO_POINTER (G_OBJECT_CLASS_TYPE (gobject_class)));

  /* Here we add an argument to the object.  This argument is an integer,
   * and can be both read and written.
   */
  g_object_class_install_property(G_OBJECT_CLASS (klass), ARG_ACTIVE,
    g_param_spec_boolean ("active","active","active",
                          TRUE, G_PARAM_READWRITE)); 

  /* The last thing is to provide the functions that implement get and set
   * of arguments.
   */
  gobject_class->set_property = gst_xmms_effect_set_property;
  gobject_class->get_property = gst_xmms_effect_get_property;
}

static GstPadLinkReturn
gst_xmms_effect_sinkconnect (GstPad *pad, GstCaps *caps)
{
  GstXmmsEffect *xmms_effect = (GstXmmsEffect *) gst_pad_get_parent (pad);
  gboolean sign;
  gint width;
  gulong endianness;
  AFormat format;

  endianness = gst_caps_get_int (caps, "endianness");
  sign = gst_caps_get_boolean (caps, "signed");
  width = gst_caps_get_int (caps, "width");

  if (width == 16) {
    if (sign == TRUE) {
      if (endianness == G_LITTLE_ENDIAN)
        format = FMT_S16_LE;
      else if (endianness == G_BIG_ENDIAN)
        format = FMT_S16_BE;
      else 
        format = FMT_S16_NE;
    }
    else {
      if (endianness == G_LITTLE_ENDIAN)
        format = FMT_U16_LE;
      else if (endianness == G_BIG_ENDIAN)
        format = FMT_U16_BE;
      else
        format = FMT_U16_NE;
    }
  }
  else {
    if (sign == TRUE) {
      format = FMT_S8;
    }
    else {
      format = FMT_U8;
    }
  }

  xmms_effect->format = format;
  xmms_effect->rate = gst_caps_get_int (caps, "rate");
  xmms_effect->channels = gst_caps_get_int (caps, "channels");

  gst_pad_try_set_caps (xmms_effect->srcpad, gst_caps_copy (caps));

  return GST_PAD_LINK_OK;
}

/* This function is responsible for initializing a specific instance of
 * the plugin.
 */
static void
gst_xmms_effect_init(GstXmmsEffect *xmms_effect)
{
  xmms_effect->sinkpad = gst_pad_new_from_template (
		  GST_PAD_TEMPLATE_GET (gst_xmms_sink_factory), "sink");
  gst_element_add_pad (GST_ELEMENT (xmms_effect), xmms_effect->sinkpad);
  gst_pad_set_chain_function (xmms_effect->sinkpad, gst_xmms_effect_chain);
  gst_pad_set_link_function (xmms_effect->sinkpad, gst_xmms_effect_sinkconnect);

  xmms_effect->srcpad = gst_pad_new_from_template (
		  GST_PAD_TEMPLATE_GET (gst_xmms_src_factory), "src");
  gst_element_add_pad (GST_ELEMENT (xmms_effect), xmms_effect->srcpad);

  /* Initialization of element's private variables. */
  xmms_effect->active = TRUE;
  xmms_effect->initialized = FALSE;

  xmms_effect->format = FMT_S16_NE;
  xmms_effect->rate = 44100;
  xmms_effect->channels = 2;
}

static void
gst_xmms_effect_chain (GstPad *pad, GstBuffer *buffer)
{
  EffectPlugin *in_plugin;
  GstXmmsEffect *xmms_effect = (GstXmmsEffect *)gst_pad_get_parent (pad);
  GstXmmsEffectClass *oclass = (GstXmmsEffectClass*)(G_OBJECT_GET_CLASS(xmms_effect));

  gpointer data;
  gint len;

  in_plugin = oclass->in_plugin;

  if (!xmms_effect->initialized) {
    xmms_effect->initialized = TRUE;
    if (in_plugin->init) in_plugin->init ();
  }

  data = GST_BUFFER_DATA (buffer);
  len = GST_BUFFER_SIZE (buffer);

  GST_DEBUG (0, "got buffer %p %p %d", GST_BUFFER_DATA (buffer), data, len);

  GST_BUFFER_FLAG_SET (buffer, GST_BUFFER_DONTFREE);
  gst_buffer_unref (buffer);

  buffer = gst_buffer_new ();
  GST_BUFFER_SIZE (buffer) = in_plugin->mod_samples (&data, len, 
		  xmms_effect->format, xmms_effect->rate, xmms_effect->channels);
  GST_BUFFER_DATA (buffer) = data;
  GST_BUFFER_FLAG_SET (buffer, GST_BUFFER_DONTFREE);
  
  GST_DEBUG (0, "pushing buffer %p %d", data, GST_BUFFER_SIZE (buffer));
  gst_pad_push (xmms_effect->srcpad, buffer);
  GST_DEBUG (0, "pushing buffer done");
}

static void
gst_xmms_effect_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  GstXmmsEffect *xmms_effect;

  /* Get a pointer of the right type. */
  xmms_effect = (GstXmmsEffect *)(object);

  /* Check the argument id to see which argument we're setting. */
  switch (prop_id) {
    case ARG_ACTIVE:
      xmms_effect->active = g_value_get_int (value);
      break;
    default:
      break;
  }
}

/* The set function is simply the inverse of the get fuction. */
static void
gst_xmms_effect_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GstXmmsEffect *xmms_effect;

  /* It's not null if we got it, but it might not be ours */
  xmms_effect = (GstXmmsEffect *)(object);

  switch (prop_id) {
    case ARG_ACTIVE:
      g_value_set_int (value, xmms_effect->active);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gchar *
cleanup_name (gchar *ugly_name)
{
  gchar *pretty_name = g_strdup (ugly_name);
  gint length = strlen (ugly_name);
  gint i, j;
  gint trailing = 0;

  g_strdown (pretty_name);

  for (j=0, i=0; i<length; i++) {
    if (ugly_name[i] == ' ') {
      pretty_name[j++] = '_';
      trailing++;
    }
    else if (ugly_name[i] == '.' || ('0' <= ugly_name[i] && ugly_name[i] <= '9')) {
    }
    else {
      pretty_name[j++] = ugly_name[i];
      trailing = 0;
    }
  }
  pretty_name[j-trailing] = '\0';
  return pretty_name;
}

/* This is the entry into the plugin itself.  When the plugin loads,
 * this function is called to register everything that the plugin provides.
 */
void
gst_xmms_effect_register (GstPlugin *plugin, GList *plugin_list)
{
  GstElementFactory *factory;
  GTypeInfo typeinfo = {
    sizeof(GstXmmsEffectClass),      
    NULL,
    NULL,
    (GClassInitFunc)gst_xmms_effect_class_init,
    NULL,
    NULL,
    sizeof(GstXmmsEffect),
    0,
    (GInstanceInitFunc)gst_xmms_effect_init,
  };
  GType type;
  GstElementDetails *details;

  global_plugins = g_hash_table_new (NULL, NULL);

  while (plugin_list) {
    EffectPlugin *in_plugin;
    gchar *type_name;

    in_plugin = (EffectPlugin *) plugin_list->data;

    /* construct the type */
    type_name = g_strdup_printf("XMMS_EFFECT_%s", cleanup_name (in_plugin->description));

    /* if it's already registered, drop it */
    if (g_type_from_name(type_name)) {
      g_free(type_name);
      continue;
    }
    /* create the gtk type now */
    type = g_type_register_static(GST_TYPE_ELEMENT, type_name , &typeinfo, 0);

    /* construct the element details struct */
    details = g_new0(GstElementDetails,1);
    details->longname = g_strdup(in_plugin->description);
    details->klass = "Source/XMMS_EFFECT";
    details->license = "GPL";
    details->description = in_plugin->description;
    details->version = g_strdup("1.0.0");
    details->author = g_strdup("XMMS");
    details->copyright = g_strdup("XMMS");

    g_hash_table_insert (global_plugins, 
		         GINT_TO_POINTER (type), 
			 (gpointer) in_plugin);

    /* register the plugin with gstreamer */
    factory = gst_element_factory_new(type_name,type,details);
    g_return_if_fail(factory != NULL);

    gst_element_factory_add_pad_template (factory, 
		    GST_PAD_TEMPLATE_GET (gst_xmms_src_factory));
    gst_element_factory_add_pad_template (factory, 
		    GST_PAD_TEMPLATE_GET (gst_xmms_sink_factory));

    /* The very last thing is to register the elementfactory with the plugin. */
    gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

    plugin_list = g_list_next (plugin_list);
  }
}

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
#include "gstxmmsinput.h"
#include "xmms.h"
#include "plugin.h"
#include "pluginenum.h"

/* These are the signals that this element can fire.  They are zero-
 * based because the numbers themselves are private to the object.
 * LAST_SIGNAL is used for initialization of the signal array.
 */
enum {
  /* FILL ME */
  LAST_SIGNAL
};

/* Arguments are identified the same way, but cannot be zero, so you
 * must leave the ARG_0 entry in as a placeholder.
 */
enum {
  ARG_0,
  ARG_ACTIVE,
  ARG_FILENAME,
  ARG_SHOW_ABOUT,
  ARG_CONFIGURE,
  ARG_SEEK,
  ARG_TIME,
  ARG_SONG_INFO,
  ARG_SHOW_FILE_INFO,
  /* FILL ME */
};

/* This factory is much simpler, and defines the source pad. */
GST_PAD_TEMPLATE_FACTORY (gst_xmms_src_factory,
  "src",
  GST_PAD_SRC,
  GST_PAD_ALWAYS,
  GST_CAPS_NEW (
    "xmms_input_src",
    "audio/raw",
    NULL
  )
)


static GMutex *global_mutex;
static GCond *global_cond;

static GCond *handoff_cond;

static GstBuffer *global_buffer;

/* FIXME dunno if this can be avoided (mayby use pthread specific stuff, but */
/* then we need to be able to pass something along) */
static GstElement *global_element;

static void init (void)
{
  GST_DEBUG (0, "plugin called init");
}

static int open_audio (AFormat fmt, int rate, int nch)
{
  GstXmmsInput *xmms_input;
  gulong endianness = G_BYTE_ORDER;
  gboolean sign = TRUE;
  gint width = 16;

  GST_DEBUG (0, "plugin called open_audio %d %d %d", fmt, rate, nch);

  xmms_input = (GstXmmsInput *) global_element;

  switch (fmt) {
    case FMT_U8:
     sign = FALSE;
    case FMT_S8:
     width = 8;
     break;
    case FMT_U16_LE:
     sign = FALSE;
    case FMT_S16_LE:
     endianness = G_LITTLE_ENDIAN;
     break;
    case FMT_U16_BE:
     sign = FALSE;
    case FMT_S16_BE:
     endianness = G_BIG_ENDIAN;
     break;
    case FMT_U16_NE:
     sign = FALSE;
     break;
    default:
     break;
  }

  gst_pad_try_set_caps (xmms_input->srcpad,
		    GST_CAPS_NEW (
		      "xmms_input_src_caps",
		      "audio/raw",
			 "format",	GST_PROPS_STRING ("int"),
			 "law",         GST_PROPS_INT (0),
			 "endianness",  GST_PROPS_INT (endianness),
			 "signed",      GST_PROPS_BOOLEAN (sign),
			 "width",       GST_PROPS_INT (width),
			 "depth",       GST_PROPS_INT (width),
			 "rate",        GST_PROPS_INT (rate),
			 "channels",    GST_PROPS_INT (nch)
		     ));
  
  return 1;
}

static void write_audio (void *ptr, int length)
{
  GST_DEBUG (0, "plugin called write_audio %p %d", ptr, length);

  g_mutex_lock (global_mutex);

  global_buffer = gst_buffer_new ();

  GST_BUFFER_SIZE (global_buffer) = length;
  GST_BUFFER_DATA (global_buffer) = ptr;

  GST_BUFFER_FLAG_SET (global_buffer, GST_BUFFER_DONTFREE);

  g_cond_signal (global_cond);

  g_cond_wait (handoff_cond, global_mutex);
  g_mutex_unlock (global_mutex);
}

static void something (void)
{
  GST_DEBUG (0, "plugin called something");
}

static void get_volume (int *l, int *r)
{
  /* hmm why should it care? */
  GST_DEBUG (0, "plugin called get_volume");

  *l =0;
  *r =0;
}

static void set_volume (int l, int r)
{
  GST_DEBUG (0, "plugin called set_volume %d %d", l, r);
}

static void close_audio (void)
{
  GST_DEBUG (0, "plugin called close_audio");
}

static void flush (int time)
{
  GST_DEBUG (0, "plugin called flush %d", time);
}

static void in_pause (short paused)
{
  GST_DEBUG (0, "plugin called in_pause %d", paused);
}

static int buffer_playing (void)
{
  GST_DEBUG (0, "plugin called buffer_playing");

  return 1;
}

static int output_time (void)
{
  GST_DEBUG (0, "plugin called output_time");

  return 1;
}

static int written_time (void)
{
  GST_DEBUG (0, "plugin called written time");

  return 0;
}

static int buffer_free (void)
{
  GST_DEBUG (0, "plugin called buffer_free");

  return 4096;
}

static OutputPlugin dummy_out = {
  NULL,        			/* void *handle;           /* Filled in by xmms */ */
  NULL,				/* char *filename;         /* Filled in by xmms */ */
  "dummy xmms output plugin", 	/* char *description;      /* The description that is shown in the preferences box */ */
  init,   			/* void (*init) (void); */
  something,       		/* void (*about) (void);   /* Show the about box */ */
  something,        		/* void (*configure) (void);       /* Show the configuration dialog */ */
  get_volume,     		/* void (*get_volume) (int *l, int *r); */
  set_volume,    		/* void (*set_volume) (int l, int r);      /* Set the volume */ */
  open_audio,  			/* int (*open_audio) (AFormat fmt, int rate, int nch);      */
  					/* Open the device, if the device can't handle the given 
				           parameters the plugin is responsible for downmixing
				           the data to the right format before outputting it */
  write_audio, 			/* void (*write_audio) (void *ptr, int length);     */
  					/* The input plugin calls this to write data to the output 
				           buffer */
  close_audio,     		/* void (*close_audio) (void);     /* No comment... */ */
  flush,       			/* void (*flush) (int time);        */
  					/* Flush the buffer and set the plugins internal timers to time */
  in_pause,    			/* void (*pause) (short paused);   /* Pause or unpause the output */ */
  buffer_free,  		/* int (*buffer_free) (void);       */
  					/* Return the amount of data that can be written to the buffer,
				           two calls to this without a call to write_audio should make
				           the plugin output audio directly */
  buffer_playing,    		/* int (*buffer_playing) (void);    */
  					/* Returns TRUE if the plugin currently is playing some audio,
				           otherwise return FALSE */
  output_time,    		/* int (*output_time) (void);      /* Return the current playing time */ */
  written_time,    		/* int (*written_time) (void);      */
   					/* Return the length of all the data that has been written to
				           the buffer */
};

static void set_info (gchar *title, int length, int rate, int freq, int nch)
{
  GST_DEBUG (0,"plugin called set info %s %d %d %d %d", title, length, rate, freq, nch);
}

static void add_vis_pcm (int time, AFormat fmt, int nch, int length, void *ptr)
{
  GST_DEBUG (0, "plugin called add_vis_pcm %d %d %d %d %p", time, fmt, nch, length, ptr);
}


static GHashTable *global_plugins;

/* A number of functon prototypes are given so we can refer to them later. */
static void			gst_xmms_input_class_init	(GstXmmsInputClass *klass);
static void			gst_xmms_input_init		(GstXmmsInput *xmms_input);

static GstElementStateReturn    gst_xmms_input_change_state	(GstElement *element);

static void			gst_xmms_input_loop		(GstElement *element);

static void			gst_xmms_input_set_property		(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void			gst_xmms_input_get_property		(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);

static GstPadTemplate *gst_xmms_src_template;

static GstElementClass *parent_class = NULL;
/*static guint gst_xmms_input_signals[LAST_SIGNAL] = { 0 }; */


static void
gst_xmms_input_class_init (GstXmmsInputClass *klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass*)klass;
  gstelement_class = (GstElementClass*)klass;

  /* The parent class is needed for class method overrides. */
  parent_class = g_type_class_ref(GST_TYPE_ELEMENT);

  klass->in_plugin = (InputPlugin *) g_hash_table_lookup (global_plugins,
		  GINT_TO_POINTER (G_OBJECT_CLASS_TYPE (gobject_class)));

  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_ACTIVE,
    g_param_spec_boolean ("active", "active", "active",
                          TRUE, G_PARAM_READWRITE));
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_FILENAME,
    g_param_spec_string ("location", "location", "location", 
                         NULL,G_PARAM_READWRITE)); 
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_SHOW_ABOUT,
    g_param_spec_boolean ("show_about", "show_about", "show_about", 
                          FALSE, G_PARAM_WRITABLE)); 
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_CONFIGURE,
    g_param_spec_boolean ("configure", "configure", "configure", 
                          FALSE, G_PARAM_WRITABLE)); 
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_SEEK,
    g_param_spec_int ("seek", "seek", "seek", 
                      0, G_MAXINT, 0, G_PARAM_WRITABLE));
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_TIME,
    g_param_spec_int ("time", "time", "time", 
                      0, G_MAXINT, 0, G_PARAM_READABLE));
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_SONG_INFO,
    g_param_spec_string ("song_info", "song_info", "song_info", 
                         NULL, G_PARAM_READABLE)); 
  g_object_class_install_property(G_OBJECT_CLASS(klass), ARG_SHOW_FILE_INFO,
    g_param_spec_boolean ("show_file_info", "show_file_info", "show_file_info", 
                          FALSE, G_PARAM_WRITABLE)); 

  gobject_class->set_property = gst_xmms_input_set_property;
  gobject_class->get_property = gst_xmms_input_get_property;

  gstelement_class->change_state = gst_xmms_input_change_state;
}

static void
gst_xmms_input_init(GstXmmsInput *xmms_input)
{
  xmms_input->srcpad = gst_pad_new_from_template (gst_xmms_src_template, "src");
  gst_element_add_pad(GST_ELEMENT(xmms_input),xmms_input->srcpad);
  gst_element_set_loop_function(GST_ELEMENT(xmms_input), gst_xmms_input_loop);

  /* Initialization of element's private variables. */
  xmms_input->active = TRUE;
  xmms_input->filename = NULL;
}

static void
gst_xmms_input_loop (GstElement *element)
{
  InputPlugin *in_plugin;
  GstXmmsInputClass *oclass = (GstXmmsInputClass*)(G_OBJECT_GET_CLASS(element));
  GstXmmsInput *xmms_input;

  xmms_input = (GstXmmsInput *) element;
  in_plugin = oclass->in_plugin;

  while (global_buffer == NULL)
    g_cond_wait (global_cond, global_mutex);

  GST_DEBUG (0, "pushing buffer");
  gst_pad_push (xmms_input->srcpad, global_buffer);
  global_buffer = NULL;
  GST_DEBUG (0, "pushing buffer done");

  g_cond_signal (handoff_cond);
}

static GstElementStateReturn
gst_xmms_input_change_state (GstElement *element)
{
  InputPlugin *in_plugin;
  GstXmmsInputClass *oclass = (GstXmmsInputClass*)(G_OBJECT_GET_CLASS(element));
  GstXmmsInput *xmms_input;

  xmms_input = (GstXmmsInput *) element;
  in_plugin = oclass->in_plugin;

  /* if going down into NULL state, close the file if it's open */
  switch (GST_STATE_TRANSITION (element)) {
    case GST_STATE_NULL_TO_READY:
      g_mutex_lock (global_mutex);
      global_buffer = NULL;
      global_element = element;
      in_plugin->set_info = set_info;
      in_plugin->add_vis_pcm = add_vis_pcm;
      in_plugin->output = &dummy_out;
      break;
    case GST_STATE_READY_TO_PAUSED:
      if (in_plugin->pause)
        in_plugin->pause(TRUE);
      in_plugin->play_file (xmms_input->filename);
      break;
    case GST_STATE_PLAYING_TO_PAUSED:
      if (in_plugin->pause)
        in_plugin->pause (TRUE);
      break;
    case GST_STATE_PAUSED_TO_PLAYING:
      if (in_plugin->pause)
        in_plugin->pause (FALSE);
      break;
    case GST_STATE_PAUSED_TO_READY:
      if (in_plugin->stop)
        in_plugin->stop ();
      break;
    case GST_STATE_READY_TO_NULL:
      g_mutex_unlock (global_mutex);
      break;
  }

  if (GST_ELEMENT_CLASS (parent_class)->change_state)
    return GST_ELEMENT_CLASS (parent_class)->change_state (element);

  return GST_STATE_SUCCESS;
}

static void
gst_xmms_input_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  GstXmmsInput *xmms_input;
  GstXmmsInputClass *oclass = (GstXmmsInputClass*)(G_OBJECT_GET_CLASS(object));
  InputPlugin *in_plugin;

  /* Get a pointer of the right type. */
  xmms_input = (GstXmmsInput *)(object);
  in_plugin = oclass->in_plugin;

  /* Check the argument id to see which argument we're setting. */
  switch (prop_id) {
    case ARG_ACTIVE:
      xmms_input->active = g_value_get_int (value);
      g_print("xmms_input: set active to %d\n",xmms_input->active);
      break;
    case ARG_FILENAME:
      if (xmms_input->filename)
	g_free (xmms_input->filename);
      xmms_input->filename = g_strdup (g_value_get_string (value));
      break;
    case ARG_SHOW_ABOUT:
      if (in_plugin->about && g_value_get_boolean (value)) {
	in_plugin->about();
      }
      break;
    case ARG_CONFIGURE:
      if (in_plugin->configure && g_value_get_boolean (value)) {
	in_plugin->configure();
      }
      break;
    case ARG_SEEK:
      if (in_plugin->seek) {
	in_plugin->seek(g_value_get_int (value));
      }
      break;
    case ARG_SHOW_FILE_INFO:
      if (in_plugin->file_info_box && g_value_get_boolean (value)) {
	in_plugin->file_info_box(xmms_input->filename);
      }
      break;
    default:
      break;
  }
}

/* The set function is simply the inverse of the get fuction. */
static void
gst_xmms_input_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  GstXmmsInput *xmms_input;
  GstXmmsInputClass *oclass;
  InputPlugin *in_plugin;

  oclass = (GstXmmsInputClass*)(G_OBJECT_GET_CLASS(object));

  /* It's not null if we got it, but it might not be ours */
  xmms_input = (GstXmmsInput *)(object);
  in_plugin = oclass->in_plugin;

  switch (prop_id) {
    case ARG_ACTIVE:
      g_value_set_int (value, xmms_input->active);
      break;
    case ARG_FILENAME:
      g_value_set_string (value, xmms_input->filename);
      break;
    case ARG_TIME:
      g_value_set_int (value, (in_plugin->get_time?in_plugin->get_time():0));
      break;
    case ARG_SONG_INFO:
      if (in_plugin->get_song_info) {
	gchar *title;
	gint length;

	in_plugin->get_song_info(xmms_input->filename, &title, &length);
        g_value_set_string (value, g_strdup_printf ("%s:%d", title, length));
      }
      else {
        g_value_set_string (value, "");
      }
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

void
gst_xmms_input_register (GstPlugin *plugin, GList *plugin_list)
{
  GstElementFactory *factory;
  GTypeInfo typeinfo = {
      sizeof(GstXmmsInputClass),      
      NULL,
      NULL,
      (GClassInitFunc)gst_xmms_input_class_init,
      NULL,
      NULL,
      sizeof(GstXmmsInput),
      0,
      (GInstanceInitFunc)gst_xmms_input_init,
  };
  GType type;
  GstElementDetails *details;

  global_plugins = g_hash_table_new (NULL, NULL);
  global_mutex = g_mutex_new();
  global_cond = g_cond_new();
  handoff_cond = g_cond_new();

  while (plugin_list) {
    InputPlugin *in_plugin;
    gchar *type_name;

    in_plugin = (InputPlugin *) plugin_list->data;


    /* construct the type */
    type_name = g_strdup_printf("XMMS_INPUT_%s", cleanup_name (in_plugin->description));

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
    details->klass = "Source/XMMS_INPUT";
    details->klass = "GPL";
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

    gst_xmms_src_template = gst_xmms_src_factory ();
    gst_element_factory_add_pad_template (factory, gst_xmms_src_template);

    /* The very last thing is to register the elementfactory with the plugin. */
    gst_plugin_add_feature (plugin, GST_PLUGIN_FEATURE (factory));

    plugin_list = g_list_next (plugin_list);
  }
}

#include <gst/gst.h>
#include <unistd.h>

/* inherited identity that has a signal for new media */
G_BEGIN_DECLS
#define GST_TYPE_NEWMEDIA \
  (gst_newmedia_get_type())
#define GST_NEWMEDIA(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_NEWMEDIA,GstNewmedia))
#define GST_NEWMEDIA_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_NEWMEDIA,GstNewmediaClass))
#define GST_IS_NEWMEDIA(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_NEWMEDIA))
#define GST_IS_NEWMEDIA_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_NEWMEDIA))
typedef struct _GstNewmedia GstNewmedia;
typedef struct _GstNewmediaClass GstNewmediaClass;

struct _GstNewmedia
{
  GstElement element;

  GstPad *sinkpad;
  GstPad *srcpad;

  gboolean triggered;
};

struct _GstNewmediaClass
{
  GstElementClass parent_class;

};

GType gst_newmedia_get_type (void);

G_END_DECLS GST_DEBUG_CATEGORY_STATIC (gst_newmedia_debug);
#define GST_CAT_DEFAULT gst_newmedia_debug

GstElementDetails gst_newmedia_details = GST_ELEMENT_DETAILS ("newmedia",
    "Generic",
    "Pass data without modification but allow app-triggered newmedia events",
    "Zaheer Abbas Merali");

#define _do_init(bla) \
    GST_DEBUG_CATEGORY_INIT (gst_newmedia_debug, "newmedia", 0, "newmedia element");

GST_BOILERPLATE_FULL (GstNewmedia, gst_newmedia, GstElement, GST_TYPE_ELEMENT,
    _do_init);

static void gst_newmedia_chain (GstPad * pad, GstData * _data);
static void gst_newmedia_trigger (GstNewmedia * newmedia);

static void
gst_newmedia_base_init (gpointer g_class)
{
  GstElementClass *gstelement_class = GST_ELEMENT_CLASS (g_class);

  gst_element_class_set_details (gstelement_class, &gst_newmedia_details);
}


static void
gst_newmedia_class_init (GstNewmediaClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gstelement_class = GST_ELEMENT_CLASS (klass);

}

static void
gst_newmedia_init (GstNewmedia * newmedia)
{
  newmedia->sinkpad = gst_pad_new ("sink", GST_PAD_SINK);
  gst_element_add_pad (GST_ELEMENT (newmedia), newmedia->sinkpad);
  gst_pad_set_chain_function (newmedia->sinkpad,
      GST_DEBUG_FUNCPTR (gst_newmedia_chain));
  gst_pad_set_link_function (newmedia->sinkpad, gst_pad_proxy_pad_link);
  gst_pad_set_getcaps_function (newmedia->sinkpad, gst_pad_proxy_getcaps);

  newmedia->srcpad = gst_pad_new ("src", GST_PAD_SRC);
  gst_element_add_pad (GST_ELEMENT (newmedia), newmedia->srcpad);
  gst_pad_set_link_function (newmedia->srcpad, gst_pad_proxy_pad_link);
  gst_pad_set_getcaps_function (newmedia->srcpad, gst_pad_proxy_getcaps);

  newmedia->triggered = FALSE;

  GST_FLAG_SET (newmedia, GST_ELEMENT_EVENT_AWARE);
}

static void
gst_newmedia_chain (GstPad * pad, GstData * _data)
{
  GstBuffer *buf = GST_BUFFER (_data);
  GstNewmedia *newmedia;
  GstEvent *newmedia_event;

  g_return_if_fail (pad != NULL);
  g_return_if_fail (GST_IS_PAD (pad));
  g_return_if_fail (buf != NULL);

  newmedia = GST_NEWMEDIA (gst_pad_get_parent (pad));

  if (GST_IS_EVENT (buf)) {
    GstEvent *event = GST_EVENT (buf);

    gst_pad_event_default (pad, event);
    return;
  } else {
    if (newmedia->triggered) {
      /* signal the new media discont */
      newmedia_event =
          gst_event_new_discontinuous (TRUE, GST_FORMAT_TIME, (gint64) 0,
          GST_FORMAT_UNDEFINED);
      newmedia->triggered = FALSE;
      gst_pad_push (newmedia->srcpad, GST_DATA (newmedia_event));

    }
    gst_pad_push (newmedia->srcpad, GST_DATA (buf));
  }
}

static void
gst_newmedia_trigger (GstNewmedia * newmedia)
{
  g_print ("Triggered newmedia\n");
  newmedia->triggered = TRUE;
}

gboolean
test_format ()
{
  int i, j;
  gboolean retval;

  GstElement *pipeline, *src, *filter, *sink;

  g_print ("creating sink\n");
  sink = gst_element_factory_make ("multifilesink", "sink");
  g_object_set (G_OBJECT (sink), "location", "format%d.tmp", NULL);

  g_print ("creating src\n");

  src = gst_element_factory_make ("fakesrc", "src");

  g_print ("creating filter\n");

  filter = GST_ELEMENT (g_object_new (GST_TYPE_NEWMEDIA, NULL));
  gst_element_set_name (filter, "newmedia");
  pipeline = gst_pipeline_new ("pipeline");
  gst_bin_add_many (GST_BIN (pipeline), src, filter, sink, NULL);
  gst_element_link_many (src, filter, sink, NULL);

  gst_element_set_state (pipeline, GST_STATE_PLAYING);


  for (i = 0; i < 9; i++) {
    g_print ("i=%d\n", i);
    for (j = 0; j < 9; j++) {
      g_print ("i=%d j=%d\n", i, j);
      gst_bin_iterate (GST_BIN (pipeline));
    }
    if (i < 8)
      gst_newmedia_trigger (GST_NEWMEDIA (filter));
  }

  gst_element_set_state (pipeline, GST_STATE_NULL);

  gst_object_unref (GST_OBJECT (pipeline));

  retval = TRUE;
  for (i = 0; i < 9; i++) {
    gchar *tempfile;

    tempfile = g_strdup_printf ("format%d.tmp", i);
    retval = retval && g_file_test (tempfile, G_FILE_TEST_EXISTS);
    /* yuck but it works */
    unlink (tempfile);
    g_free (tempfile);
  }
  return retval;

  return TRUE;

}

static void
newfile_signal (GstElement * element, gpointer user_data)
{
  int *curindex;
  char *filename;

  curindex = (int *) user_data;
  filename = g_strdup_printf ("signal%d.tmp", *curindex);
  g_object_set (G_OBJECT (element), "location", filename, NULL);
  g_free (filename);
  (*curindex)++;
}

gboolean
test_signal ()
{
  int i, j, curindex;
  gboolean retval;

  GstElement *pipeline, *src, *filter, *sink;

  g_print ("creating sink\n");
  sink = gst_element_factory_make ("multifilesink", "sink");
  g_object_set (G_OBJECT (sink), "location", "signal0.tmp", NULL);
  curindex = 1;
  g_signal_connect (G_OBJECT (sink), "newfile", (GCallback) newfile_signal,
      &curindex);
  g_print ("creating src\n");

  src = gst_element_factory_make ("fakesrc", "src");

  g_print ("creating filter\n");

  filter = GST_ELEMENT (g_object_new (GST_TYPE_NEWMEDIA, NULL));
  gst_element_set_name (filter, "newmedia");
  pipeline = gst_pipeline_new ("pipeline");
  gst_bin_add_many (GST_BIN (pipeline), src, filter, sink, NULL);
  gst_element_link_many (src, filter, sink, NULL);

  gst_element_set_state (pipeline, GST_STATE_PLAYING);


  for (i = 0; i < 9; i++) {
    g_print ("i=%d\n", i);
    for (j = 0; j < 9; j++) {
      g_print ("i=%d j=%d\n", i, j);
      gst_bin_iterate (GST_BIN (pipeline));
    }
    if (i < 8)
      gst_newmedia_trigger (GST_NEWMEDIA (filter));
  }

  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));

  retval = TRUE;
  for (i = 0; i < 9; i++) {
    gchar *tempfile;

    tempfile = g_strdup_printf ("signal%d.tmp", i);
    retval = retval && g_file_test (tempfile, G_FILE_TEST_EXISTS);
    /* yuck but it works */
    unlink (tempfile);
    g_free (tempfile);
  }
  return retval;
}

int
main (int argc, char **argv)
{
  gst_init (&argc, &argv);

  if (!test_format ()) {
    g_print ("Test with location as format%%d.tmp failed\n");
    exit (-1);
  }
  if (!test_signal ()) {
    g_print ("Test using a signal handler to set location every time failed\n");
    exit (-1);
  }
  g_print ("All worked ok!\n");
  return 0;
}

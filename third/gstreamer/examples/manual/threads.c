
/*** block  from ../../docs/manual/advanced-threads.xml ***/
#include <gst/gst.h>

GstElement *thread, *source, *decodebin, *audiosink;

static gboolean
idle_eos (gpointer data)
{
  g_print ("Have idle-func in thread %p\n", g_thread_self ());
  gst_main_quit ();

  /* do this function only once */
  return FALSE;
}

/*
 * EOS will be called when the src element has an end of stream.
 * Note that this function will be called in the thread context.
 * We will place an idle handler to the function that really
 * quits the application.
 */
static void
cb_eos (GstElement *thread,
	gpointer    data) 
{
  g_print ("Have eos in thread %p\n", g_thread_self ());
  g_idle_add ((GSourceFunc) idle_eos, NULL);
}

/*
 * On error, too, you'll want to forward signals to the main
 * thread, especially when using GUI applications.
 */

static void
cb_error (GstElement *thread,
	  GstElement *source,
	  GError     *error,
	  gchar      *debug,
	  gpointer    data)
{
  g_print ("Error in thread %p: %s\n", g_thread_self (), error->message);
  g_idle_add ((GSourceFunc) idle_eos, NULL);
}

/*
 * Link new pad from decodebin to audiosink.
 * Contains no further error checking.
 */

static void
cb_newpad (GstElement *decodebin,
	   GstPad     *pad,
	   gboolean    last,
	   gpointer    data)
{
  gst_pad_link (pad, gst_element_get_pad (audiosink, "sink"));
  gst_bin_add (GST_BIN (thread), audiosink);
  gst_bin_sync_children_state (GST_BIN (thread));
}

gint 
main (gint   argc,
      gchar *argv[]) 
{
  /* init GStreamer */
  gst_init (&argc, &argv);

  /* make sure we have a filename argument */
  if (argc != 2) {
    g_print ("usage: %s <Ogg/Vorbis filename>\n", argv[0]);
    return -1;
  }

  /* create a new thread to hold the elements */
  thread = gst_thread_new ("thread");
  g_signal_connect (thread, "eos", G_CALLBACK (cb_eos), NULL);
  g_signal_connect (thread, "error", G_CALLBACK (cb_error), NULL);

  /* create elements */
  source = gst_element_factory_make ("filesrc", "source");
  g_object_set (G_OBJECT (source), "location", argv[1], NULL);
  decodebin = gst_element_factory_make ("decodebin", "decoder");
  g_signal_connect (decodebin, "new-decoded-pad",
		    G_CALLBACK (cb_newpad), NULL);
  audiosink = gst_element_factory_make ("alsasink", "audiosink");

  /* setup */
  gst_bin_add_many (GST_BIN (thread), source, decodebin, NULL);
  gst_element_link (source, decodebin);
  gst_element_set_state (audiosink, GST_STATE_PAUSED);
  gst_element_set_state (thread, GST_STATE_PLAYING);

  /* no need to iterate. We can now use a mainloop */
  gst_main ();

  /* unset */
  gst_element_set_state (thread, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (thread));

  return 0;
}


/*** block  from ../../docs/manual/highlevel-components.xml ***/
#include <gst/gst.h>

static void
cb_eos (GstElement *play,
	gpointer    data)
{
  gst_main_quit ();
}

static void
cb_error (GstElement *play,
	  GstElement *src,
	  GError     *err,
	  gchar      *debug,
	  gpointer    data)
{
  g_print ("Error: %s\n", err->message);
}

gint
main (gint   argc,
      gchar *argv[])
{
  GstElement *play;

  /* init GStreamer */
  gst_init (&argc, &argv);

  /* make sure we have a URI */
  if (argc != 2) {
    g_print ("Usage: %s <URI>\n", argv[0]);
    return -1;
  }

  /* set up */
  play = gst_element_factory_make ("playbin", "play");
  g_object_set (G_OBJECT (play), "uri", argv[1], NULL);
  g_signal_connect (play, "eos", G_CALLBACK (cb_eos), NULL);
  g_signal_connect (play, "error", G_CALLBACK (cb_error), NULL);
  if (gst_element_set_state (play, GST_STATE_PLAYING) != GST_STATE_SUCCESS) {
    g_print ("Failed to play\n");
    return -1;
  }

  /* now run */
  gst_main ();

  /* also clean up */
  gst_element_set_state (play, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (play));

  return 0;
}

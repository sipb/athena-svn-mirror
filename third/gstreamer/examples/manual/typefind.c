
/*** block  from ../../docs/manual/advanced-autoplugging.xml ***/
#include <gst/gst.h>

static void
cb_typefound (GstElement *typefind,
	      guint       probability,
	      GstCaps    *caps,
	      gpointer    data)
{
  gchar *type;

  type = gst_caps_to_string (caps);
  g_print ("Media type %s found, probability %d%%\n", type, probability);
  g_free (type);

  /* done */
  (* (gboolean *) data) = TRUE;
}

static void
cb_error (GstElement *pipeline,
	  GstElement *source,
	  GError     *error,
	  gchar      *debug,
	  gpointer    data)
{
  g_print ("Error: %s\n", error->message);

  /* done */
  (* (gboolean *) data) = TRUE;
}

gint 
main (gint   argc,
      gchar *argv[])
{
  GstElement *pipeline, *filesrc, *typefind;
  gboolean done = FALSE;

  /* init GStreamer */
  gst_init (&argc, &argv);

  /* check args */
  if (argc != 2) {
    g_print ("Usage: %s <filename>\n", argv[0]);
    return -1;
  }

  /* create a new pipeline to hold the elements */
  pipeline = gst_pipeline_new ("pipe");
  g_signal_connect (pipeline, "error", G_CALLBACK (cb_error), &done);

  /* create file source and typefind element */
  filesrc = gst_element_factory_make ("filesrc", "source");
  g_object_set (G_OBJECT (filesrc), "location", argv[1], NULL);
  typefind = gst_element_factory_make ("typefind", "typefinder");
  g_signal_connect (typefind, "have-type", G_CALLBACK (cb_typefound), &done);

  /* setup */
  gst_bin_add_many (GST_BIN (pipeline), filesrc, typefind, NULL);
  gst_element_link (filesrc, typefind);
  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  /* now iterate until the type is found */
  do {
    if (!gst_bin_iterate (GST_BIN (pipeline)))
      break;
  } while (!done);

  /* unset */
  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));

  return 0;
}

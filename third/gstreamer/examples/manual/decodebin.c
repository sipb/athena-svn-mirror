
/*** block  from ../../docs/manual/highlevel-components.xml ***/
#include <gst/gst.h>

GstElement *pipeline, *audio;
GstPad *audiopad;

static void
cb_newpad (GstElement *decodebin,
	   GstPad     *pad,
	   gboolean    last,
	   gpointer    data)
{
  GstCaps *caps;
  GstStructure *str;

  /* only link audio; only link once */
  if (GST_PAD_IS_LINKED (audiopad))
    return;
  caps = gst_pad_get_caps (pad);
  str = gst_caps_get_structure (caps, 0);
  if (!g_strrstr (gst_structure_get_name (str), "audio"))
    return;

  /* link'n'play */
  gst_pad_link (pad, audiopad);
  gst_bin_add (GST_BIN (pipeline), audio);
  gst_bin_sync_children_state (GST_BIN (pipeline));
}

gint
main (gint   argc,
      gchar *argv[])
{
  GstElement *src, *dec, *conv, *scale, *sink;

  /* init GStreamer */
  gst_init (&argc, &argv);

  /* make sure we have input */
  if (argc != 2) {
    g_print ("Usage: %s <filename>\n", argv[0]);
    return -1;
  }

  /* setup */
  pipeline = gst_pipeline_new ("pipeline");
  src = gst_element_factory_make ("filesrc", "source");
  g_object_set (G_OBJECT (src), "location", argv[1], NULL);
  dec = gst_element_factory_make ("decodebin", "decoder");
  g_signal_connect (dec, "new-decoded-pad", G_CALLBACK (cb_newpad), NULL);
  audio = gst_bin_new ("audiobin");
  conv = gst_element_factory_make ("audioconvert", "aconv");
  audiopad = gst_element_get_pad (conv, "sink");
  scale = gst_element_factory_make ("audioscale", "scale");
  sink = gst_element_factory_make ("alsasink", "sink");
  gst_bin_add_many (GST_BIN (audio), conv, scale, sink, NULL);
  gst_element_link_many (conv, scale, sink, NULL);
  gst_bin_add_many (GST_BIN (pipeline), src, dec, NULL);
  gst_element_link (src, dec);

  /* run */
  gst_element_set_state (audio, GST_STATE_PAUSED);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  while (gst_bin_iterate (GST_BIN (pipeline))) ;

  /* cleanup */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));

  return 0;
}


/*** block  from ../../docs/manual/basics-helloworld.xml ***/
#include <gst/gst.h>

/*
 * Global objects are usually a bad thing. For the purpose of this
 * example, we will use them, however.
 */

GstElement *pipeline, *source, *parser, *decoder, *sink;

static void
new_pad (GstElement *element,
	 GstPad     *pad,
	 gpointer    data)
{
  /* We can now link this pad with the audio decoder and
   * add both decoder and audio output to the pipeline. */
  gst_pad_link (pad, gst_element_get_pad (decoder, "sink"));
  gst_bin_add_many (GST_BIN (pipeline), decoder, sink, NULL);

  /* This function synchronizes a bins state on all of its
   * contained children. */
  gst_bin_sync_children_state (GST_BIN (pipeline));
}

int
main (int   argc,
      char *argv[])
{
  /* initialize GStreamer */
  gst_init (&argc, &argv);

  /* check input arguments */
  if (argc != 2) {
    g_print ("Usage: %s <Ogg/Vorbis filename>\n", argv[0]);
    return -1;
  }

  /* create elements */
  pipeline = gst_pipeline_new ("audio-player");
  source = gst_element_factory_make ("filesrc", "file-source");
  parser = gst_element_factory_make ("oggdemux", "ogg-parser");
  decoder = gst_element_factory_make ("vorbisdec", "vorbis-decoder");
  sink = gst_element_factory_make ("alsasink", "alsa-output");

  /* set filename property on the file source */
  g_object_set (G_OBJECT (source), "location", argv[1], NULL);

  /* link together - note that we cannot link the parser and
   * decoder yet, becuse the parser uses dynamic pads. For that,
   * we set a new-pad signal handler. */
  gst_element_link (source, parser);
  gst_element_link (decoder, sink);
  g_signal_connect (parser, "new-pad", G_CALLBACK (new_pad), NULL);

  /* put all elements in a bin - or at least the ones we will use
   * instantly. */
  gst_bin_add_many (GST_BIN (pipeline), source, parser, NULL);

  /* Now set to playing and iterate. We will set the decoder and
   * audio output to ready so they initialize their memory already.
   * This will decrease the amount of time spent on linking these
   * elements when the Ogg parser emits the new-pad signal. */
  gst_element_set_state (decoder, GST_STATE_READY);
  gst_element_set_state (sink, GST_STATE_READY);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  /* and now iterate - the rest will be automatic from here on.
   * When the file is finished, gst_bin_iterate () will return
   * FALSE, thereby terminating this loop. */
  while (gst_bin_iterate (GST_BIN (pipeline))) ;

  /* clean up nicely */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));

  return 0;
}

/*
 * spider3.c
 * this test tests spider's ability to connect between a mono sinesrc ! lame
 * and a stereo volume ! fakesink, where 
 * - spider ! volume connection is done through a _filtered connection
 * - sinesrc ! lame too
 */

#include <gst/gst.h>

int
main (int argc, char *argv[])
{
  GstElement *pipeline;
  GstElement *sinesrc, *lame, *spider, *volume, *fakesink;
  GstCaps *caps;
  GstPad *pad;
 
  gst_init (&argc, &argv);

  g_print ("Trying spider connection between sinesrc and volume ! fakesink\n");
  g_print ("(with spider ! volume filtered to stereo)\n");
  pipeline = gst_pipeline_new ("test");
  g_assert (pipeline);
  sinesrc = gst_element_factory_make ("sinesrc", "sinesrc");
  g_assert (sinesrc);
  gst_element_set (sinesrc, "samplerate", 44100, NULL);
  lame = gst_element_factory_make ("lame", "lame");
  g_assert (lame);
  spider = gst_element_factory_make ("spider", "spider");
  g_assert (spider);
  volume = gst_element_factory_make ("volume", "volume");
  g_assert (volume);
  fakesink = gst_element_factory_make ("fakesink", "fakesink");
  g_assert (fakesink);

  gst_bin_add_many (GST_BIN (pipeline), 
                    sinesrc, lame, spider, volume, fakesink, NULL);
  /* force standard audio caps on spider ! volume */
  caps = gst_caps_new ("stereo", "audio/raw",
		       gst_props_new (
				  "format",     GST_PROPS_STRING ("int"),
				  "law",        GST_PROPS_INT (0),
				  "endianness", GST_PROPS_INT (G_BYTE_ORDER),
				  "signed",     GST_PROPS_BOOLEAN (TRUE),
				  "width",      GST_PROPS_INT (16),
				  "depth",      GST_PROPS_INT (16),
				  "rate",       GST_PROPS_INT (44100),
				  "channels",   GST_PROPS_INT (2),
				  NULL));

  if (!gst_element_connect_filtered (spider, volume, caps))
    g_error ("Could not connect_filtered spider and volume");

  /* force samplerate on sinesrc ! lame */
  caps = gst_caps_new ("samplerate", "audio/raw",
		       gst_props_new (
				  "rate", GST_PROPS_INT (44100),
				  NULL));
  if (!gst_element_connect_filtered (sinesrc, lame, caps))
    g_error ("Could not connect_filtered sinesrc and lame");

  gst_element_connect (lame, spider);
  gst_element_connect (volume, fakesink);
  g_print ("Setting pipeline to ready\n");
  gst_element_set_state (pipeline, GST_STATE_READY);
  g_print ("Setting pipeline to paused\n");
  gst_element_set_state (pipeline, GST_STATE_PAUSED);

  /* debug sine src caps, make sure they got fixed */
  pad = gst_element_get_pad (sinesrc, "src");
  g_assert (pad);
  caps = gst_pad_get_caps (pad);
  g_assert (caps);
  g_print ("Dumping caps of sinesrc's src pad\n");
  gst_caps_debug (caps, "sinesrc src pad after filtered connection");
  g_assert (GST_CAPS_IS_FIXED (caps));

  /* debug volume sink caps, make sure they got fixed */
  pad = gst_element_get_pad (volume, "sink");
  g_assert (pad);
  caps = gst_pad_get_caps (pad);
  g_assert (caps);
  g_print ("Dumping caps of volume's sink pad\n");
  gst_caps_debug (caps, "volume sink pad after filtered connection");
  g_assert (GST_CAPS_IS_FIXED (caps));

  /* debug volume source caps, make sure they got fixed */
  pad = gst_element_get_pad (volume, "src");
  g_assert (pad);
  caps = gst_pad_get_caps (pad);
  g_assert (caps);
  g_print ("Dumping caps of volume's source pad\n");
  gst_caps_debug (caps, "volume source pad after filtered connection");
  g_assert (GST_CAPS_IS_FIXED (caps));


  g_print ("Setting pipeline to play\n");
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  g_print ("Doing an iteration\n");
  gst_bin_iterate (GST_BIN (pipeline));

  return 0;
}

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
  gchar *caps_str;
  GstPad *pad;

  gst_init (&argc, &argv);

  g_print ("Trying spider connection between sinesrc and volume ! fakesink\n");
  g_print ("(with spider ! volume filtered to stereo)\n");
  pipeline = gst_pipeline_new ("test");
  g_assert (pipeline);
  sinesrc = gst_element_factory_make ("sinesrc", "sinesrc");
  g_assert (sinesrc);
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
  caps = gst_caps_new_simple ("audio/x-raw-int",
      "endianness", G_TYPE_INT, G_BYTE_ORDER,
      "signed", G_TYPE_BOOLEAN, TRUE,
      "width", G_TYPE_INT, 16,
      "depth", G_TYPE_INT, 16,
      "rate", G_TYPE_INT, 44100, "channels", G_TYPE_INT, 2, NULL);

  if (!gst_element_link_filtered (spider, volume, caps))
    g_error ("Could not connect_filtered spider and volume");

  /* force samplerate on sinesrc ! lame */
  caps = gst_caps_new_simple ("audio/x-raw-int",
      "rate", G_TYPE_INT, 44100, NULL);
  if (!gst_element_link_filtered (sinesrc, lame, caps))
    g_error ("Could not connect_filtered sinesrc and lame");

  gst_element_link (lame, spider);
  gst_element_link (volume, fakesink);
  g_print ("Setting pipeline to ready\n");
  gst_element_set_state (pipeline, GST_STATE_READY);
  g_print ("Setting pipeline to paused\n");
  gst_element_set_state (pipeline, GST_STATE_PAUSED);

  /* debug sine src caps, make sure they got fixed */
  pad = gst_element_get_pad (sinesrc, "src");
  g_assert (pad);
  caps = gst_pad_get_caps (pad);
  g_assert (caps);
  caps_str = gst_caps_to_string (caps);
  g_print ("caps of sinesrc's src pad: %s\n", caps_str);
  g_free (caps_str);
  g_assert (gst_caps_is_fixed (caps));

  /* debug volume sink caps, make sure they got fixed */
  pad = gst_element_get_pad (volume, "sink");
  g_assert (pad);
  caps = gst_pad_get_caps (pad);
  g_assert (caps);
  caps_str = gst_caps_to_string (caps);
  g_print ("caps of volume's sink pad: %s\n", caps_str);
  g_free (caps_str);
  g_assert (gst_caps_is_fixed (caps));

  /* debug volume source caps, make sure they got fixed */
  pad = gst_element_get_pad (volume, "src");
  g_assert (pad);
  caps = gst_pad_get_caps (pad);
  g_assert (caps);
  caps_str = gst_caps_to_string (caps);
  g_print ("caps of volume's src pad: %s\n", caps_str);
  g_free (caps_str);
  g_assert (gst_caps_is_fixed (caps));


  g_print ("Setting pipeline to play\n");
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  g_print ("Doing an iteration\n");
  gst_bin_iterate (GST_BIN (pipeline));

  return 0;
}

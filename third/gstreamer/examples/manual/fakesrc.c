
/*** block  from ../../docs/manual/advanced-dataaccess.xml ***/
#include <string.h> /* for memset () */
#include <gst/gst.h>

static void
cb_handoff (GstElement *fakesrc,
	    GstBuffer  *buffer,
	    GstPad     *pad,
	    gpointer    user_data)
{
  static gboolean white = FALSE;

  /* this makes the image black/white */
  memset (GST_BUFFER_DATA (buffer), white ? 0xff : 0x0,
	  GST_BUFFER_SIZE (buffer));
  white = !white;
}

gint
main (gint   argc,
      gchar *argv[])
{
  GstElement *pipeline, *fakesrc, *conv, *videosink;
  GstCaps *filter;

  /* init GStreamer */
  gst_init (&argc, &argv);

  /* setup pipeline */
  pipeline = gst_pipeline_new ("pipeline");
  fakesrc = gst_element_factory_make ("fakesrc", "source");
  conv = gst_element_factory_make ("ffmpegcolorspace", "conv");
  videosink = gst_element_factory_make ("ximagesink", "videosink");

  /* setup */
  filter = gst_caps_new_simple ("video/x-raw-rgb",
				"width", G_TYPE_INT, 384,
				"height", G_TYPE_INT, 288,
				"framerate", G_TYPE_DOUBLE, (gdouble) 1.0,
				"bpp", G_TYPE_INT, 16,
				"depth", G_TYPE_INT, 16,
				"endianness", G_TYPE_INT, G_BYTE_ORDER,
				NULL);
  gst_element_link_filtered (fakesrc, conv, filter);
  gst_element_link (conv, videosink);
  gst_bin_add_many (GST_BIN (pipeline), fakesrc, conv, videosink, NULL);

  /* setup fake source */
  g_object_set (G_OBJECT (fakesrc),
		"signal-handoffs", TRUE,
		"sizemax", 384 * 288 * 2,
		"sizetype", 2, NULL);
  g_signal_connect (fakesrc, "handoff", G_CALLBACK (cb_handoff), NULL);

  /* play */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  while (gst_bin_iterate (GST_BIN (pipeline))) ;

  /* clean up */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));

  return 0;
}

#include <gst/gst.h>

int
main (int argc, char **argv)
{
  GSList *locations;
  int i;

  //gboolean retval;

  GstElement *pipeline, *src, *sink;

  gst_init (&argc, &argv);

  g_print ("creating sink\n");
  sink = gst_element_factory_make ("multifilesink", "sink");
  g_object_set (G_OBJECT (sink), "location", "format%d.tmp", NULL);

  g_print ("creating src\n");

  src = gst_element_factory_make ("multifilesrc", "src");
  //g_object_set (G_OBJECT(src), "newmedia", TRUE, NULL);

  locations = NULL;

  for (i = 0; i < 10; i++) {
    gchar *blah = g_strdup_printf ("blah%d.tmp", i);

    locations = g_slist_append (locations, blah);
  }

  g_object_set (G_OBJECT (src), "locations", locations, NULL);

  pipeline = gst_pipeline_new ("pipeline");
  gst_bin_add_many (GST_BIN (pipeline), src, sink, NULL);
  gst_element_link_many (src, sink, NULL);

  gst_element_set_state (pipeline, GST_STATE_PLAYING);

  while (gst_bin_iterate (GST_BIN (pipeline)));

  return 0;
}

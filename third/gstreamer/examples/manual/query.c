
/*** block a  from ../../docs/manual/advanced-position.xml ***/
#include <gst/gst.h>

gint
main (gint   argc,
      gchar *argv[])
{
  GstElement *sink, *pipeline;

/*** block b  from ../../docs/manual/advanced-position.xml ***/
  gchar *l;

  /* init */
  gst_init (&argc, &argv);

  /* args */
  if (argc != 2) {
    g_print ("Usage: %s <filename>\n", argv[0]);
    return -1;
  }

  /* build pipeline, the easy way */
  l = g_strdup_printf ("filesrc location=\"%s\" ! oggdemux ! vorbisdec ! "
		       "audioconvert ! audioscale ! alsasink name=a",
		       argv[1]);
  pipeline = gst_parse_launch (l, NULL);
  sink = gst_bin_get_by_name (GST_BIN (pipeline), "a");
  g_free (l);

  /* play */
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

/*** block c  from ../../docs/manual/advanced-position.xml ***/
  /* run pipeline */
  do {
    gint64 len, pos;
    GstFormat fmt = GST_FORMAT_TIME;

    if (gst_element_query (sink, GST_QUERY_POSITION, &fmt, &pos) &&
        gst_element_query (sink, GST_QUERY_TOTAL, &fmt, &len)) {
      g_print ("Time: %" GST_TIME_FORMAT " / %" GST_TIME_FORMAT "\r",
	       GST_TIME_ARGS (pos), GST_TIME_ARGS (len));
    }
  } while (gst_bin_iterate (GST_BIN (pipeline)));

/*** block d  from ../../docs/manual/advanced-position.xml ***/
  /* clean up */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (GST_OBJECT (pipeline));

  return 0;

/*** block e  from ../../docs/manual/advanced-position.xml ***/
}

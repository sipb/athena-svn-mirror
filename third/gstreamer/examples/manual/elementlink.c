
/*** block a  from ../../docs/manual/basics-elements.xml ***/
#include <gst/gst.h>

int
main (int   argc,
      char *argv[])
{
  GstElement *source, *filter, *sink;

  /* init */
  gst_init (&argc, &argv);

  /* create elements */
  source = gst_element_factory_make ("fakesrc", "source");
  filter = gst_element_factory_make ("identity", "filter");
  sink = gst_element_factory_make ("fakesink", "sink");

  /* link */
  gst_element_link_many (source, filter, sink, NULL);

/*** block b  from ../../docs/manual/basics-elements.xml ***/
  return 0;

/*** block c  from ../../docs/manual/basics-elements.xml ***/
}

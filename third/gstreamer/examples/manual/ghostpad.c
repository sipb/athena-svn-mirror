
/*** block a  from ../../docs/manual/basics-pads.xml ***/
#include <gst/gst.h>

int
main (int   argc,
      char *argv[])
{
  GstElement *bin, *sink;

  /* init */
  gst_init (&argc, &argv);

  /* create element, add to bin, add ghostpad */
  sink = gst_element_factory_make ("fakesink", "sink");
  bin = gst_bin_new ("mybin");
  gst_bin_add (GST_BIN (bin), sink);
  gst_element_add_ghost_pad (bin,
      gst_element_get_pad (sink, "sink"), "sink");

/*** block b  from ../../docs/manual/basics-pads.xml ***/
  return 0;

/*** block c  from ../../docs/manual/basics-pads.xml ***/
}

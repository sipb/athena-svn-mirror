
/*** block a  from ../../docs/manual/basics-pads.xml ***/
#include <gst/gst.h>

static void
cb_new_pad (GstElement *element,
	    GstPad     *pad,
	    gpointer    data)
{
  g_print ("A new pad %s was created\n", gst_pad_get_name (pad));

  /* here, you would setup a new pad link for the newly created pad */

/*** block b  from ../../docs/manual/basics-pads.xml ***/
}

int 
main(int argc, char *argv[]) 
{
  GstElement *pipeline, *source, *demux;

  /* init */
  gst_init (&argc, &argv);

  /* create elements */
  pipeline = gst_pipeline_new ("my_pipeline");
  source = gst_element_factory_make ("filesrc", "source");
  g_object_set (source, "location", argv[1], NULL);
  demux = gst_element_factory_make ("oggdemux", "demuxer");

  /* you would normally check that the elements were created properly */

  /* put together a pipeline */
  gst_bin_add_many (GST_BIN (pipeline), source, demux, NULL);
  gst_element_link (source, demux);

  /* listen for newly created pads */
  g_signal_connect (demux, "new-pad", G_CALLBACK (cb_new_pad), NULL);

  /* start the pipeline */
  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);
  while (gst_bin_iterate (GST_BIN (pipeline)));
  return 0;

/*** block d  from ../../docs/manual/basics-pads.xml ***/
}

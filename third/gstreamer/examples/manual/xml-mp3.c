/* example-begin xml-mp3.c */
#include <stdlib.h>
#include <gst/gst.h>

gboolean playing;

int 
main (int argc, char *argv[]) 
{
  GstElement *filesrc, *osssink, *queue, *queue2, *decode;
  GstElement *bin;
  GstElement *thread, *thread2;

  gst_init (&argc,&argv);

  if (argc != 2) {
    g_print ("usage: %s <mp3 filename>\n", argv[0]);
    exit (-1);
  }

  /* create a new thread to hold the elements */
  thread = gst_element_factory_make ("thread", "thread");
  g_assert (thread != NULL);
  thread2 = gst_element_factory_make ("thread", "thread2");
  g_assert (thread2 != NULL);

  /* create a new bin to hold the elements */
  bin = gst_bin_new ("bin");
  g_assert (bin != NULL);

  /* create a disk reader */
  filesrc = gst_element_factory_make ("filesrc", "disk_source");
  g_assert (filesrc != NULL);
  g_object_set (G_OBJECT (filesrc), "location", argv[1], NULL);

  queue = gst_element_factory_make ("queue", "queue");
  queue2 = gst_element_factory_make ("queue", "queue2");

  /* and an audio sink */
  osssink = gst_element_factory_make ("osssink", "play_audio");
  g_assert (osssink != NULL);

  decode = gst_element_factory_make ("mad", "decode");
  g_assert (decode != NULL);

  /* add objects to the main bin */
  gst_bin_add (GST_BIN (bin), filesrc);
  gst_bin_add (GST_BIN (bin), queue);

  gst_bin_add (GST_BIN (thread), decode);
  gst_bin_add (GST_BIN (thread), queue2);

  gst_bin_add (GST_BIN (thread2), osssink);
  
  gst_pad_link (gst_element_get_pad (filesrc,"src"),
                   gst_element_get_pad (queue,"sink"));

  gst_pad_link (gst_element_get_pad (queue,"src"),
                   gst_element_get_pad (decode,"sink"));
  gst_pad_link (gst_element_get_pad (decode,"src"),
                   gst_element_get_pad (queue2,"sink"));

  gst_pad_link (gst_element_get_pad (queue2,"src"),
                   gst_element_get_pad (osssink,"sink"));

  gst_bin_add (GST_BIN (bin), thread);
  gst_bin_add (GST_BIN (bin), thread2);

  /* write the bin to stdout */
  gst_xml_write_file (GST_ELEMENT (bin), stdout);

  /* write the bin to a file */
  gst_xml_write_file (GST_ELEMENT (bin), fopen ("xmlTest.gst", "w"));

  exit (0);
}
/* example-end xml-mp3.c */

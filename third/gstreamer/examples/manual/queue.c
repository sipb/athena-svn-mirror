/* example-begin queue.c */
#include <stdlib.h>
#include <gst/gst.h>

gboolean playing;

/* eos will be called when the src element has an end of stream */
void 
eos (GstElement *element, gpointer data) 
{
  g_print ("have eos, quitting\n");

  playing = FALSE;
}

int 
main (int argc, char *argv[]) 
{
  GstElement *filesrc, *audiosink, *queue, *decode;
  GstElement *bin;
  GstElement *thread;

  gst_init (&argc,&argv);

  if (argc != 2) {
    g_print ("usage: %s <mp3 filename>\n", argv[0]);
    exit (-1);
  }

  /* create a new thread to hold the elements */
  thread = gst_thread_new ("thread");
  g_assert (thread != NULL);

  /* create a new bin to hold the elements */
  bin = gst_bin_new ("bin");
  g_assert (bin != NULL);

  /* create a disk reader */
  filesrc = gst_element_factory_make ("filesrc", "disk_source");
  g_assert (filesrc != NULL);
  g_object_set (G_OBJECT (filesrc), "location", argv[1], NULL);
  g_signal_connect (G_OBJECT (filesrc), "eos",
                    G_CALLBACK (eos), thread);

  queue = gst_element_factory_make ("queue", "queue");
  g_assert (queue != NULL);

  /* and an audio sink */
  audiosink = gst_element_factory_make ("osssink", "play_audio");
  g_assert (audiosink != NULL);

  decode = gst_element_factory_make ("mad", "decode");

  /* add objects to the main bin */
  gst_bin_add_many (GST_BIN (thread), decode, audiosink, NULL);

  gst_bin_add_many (GST_BIN (bin), filesrc, queue, thread, NULL);

  
  gst_element_link (filesrc, queue);
  gst_element_link_many (queue, decode, audiosink, NULL);

  /* start playing */
  gst_element_set_state (GST_ELEMENT (bin), GST_STATE_PLAYING);

  playing = TRUE;

  while (playing) {
    gst_bin_iterate (GST_BIN (bin));
  }

  gst_element_set_state (GST_ELEMENT (bin), GST_STATE_NULL);

  return 0;
}
/* example-end queue.c */

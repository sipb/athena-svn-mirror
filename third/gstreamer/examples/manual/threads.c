/* example-begin threads.c */
#include <gst/gst.h>

/* we set this to TRUE right before gst_main (), but there could still
   be a race condition between setting it and entering the function */
gboolean can_quit = FALSE;

/* eos will be called when the src element has an end of stream */
void 
eos (GstElement *src, gpointer data) 
{
  GstThread *thread = GST_THREAD (data);
  g_print ("have eos, quitting\n");

  /* stop the bin */
  gst_element_set_state (GST_ELEMENT (thread), GST_STATE_NULL);

  while (!can_quit) /* waste cycles */ ;
  gst_main_quit ();
}

int 
main (int argc, char *argv[]) 
{
  GstElement *filesrc, *demuxer, *decoder, *converter, *audiosink;
  GstElement *thread;

  if (argc < 2) {
    g_print ("usage: %s <Ogg/Vorbis filename>\n", argv[0]);
    exit (-1);
  }

  gst_init (&argc, &argv);

  /* create a new thread to hold the elements */
  thread = gst_thread_new ("thread");
  g_assert (thread != NULL);

  /* create a disk reader */
  filesrc = gst_element_factory_make ("filesrc", "disk_source");
  g_assert (filesrc != NULL);
  g_object_set (G_OBJECT (filesrc), "location", argv[1], NULL);
  g_signal_connect (G_OBJECT (filesrc), "eos",
                     G_CALLBACK (eos), thread);

  /* create an ogg demuxer */
  demuxer = gst_element_factory_make ("oggdemux", "demuxer");
  g_assert (demuxer != NULL);

  /* create a vorbis decoder */
  decoder = gst_element_factory_make ("vorbisdec", "decoder");
  g_assert (decoder != NULL);

  /* create an audio converter */
  converter = gst_element_factory_make ("audioconvert", "converter");
  g_assert (decoder != NULL);

  /* and an audio sink */
  audiosink = gst_element_factory_make ("osssink", "play_audio");
  g_assert (audiosink != NULL);

  /* add objects to the thread */
  gst_bin_add_many (GST_BIN (thread), filesrc, demuxer, decoder, converter, audiosink, NULL);
  /* link them in the logical order */
  gst_element_link_many (filesrc, demuxer, decoder, converter, audiosink, NULL);

  /* start playing */
  gst_element_set_state (thread, GST_STATE_PLAYING);

  /* do whatever you want here, the thread will be playing */
  g_print ("thread is playing\n");
  
  can_quit = TRUE;
  gst_main ();

  gst_object_unref (GST_OBJECT (thread));

  exit (0);
}
/* example-end threads.c */

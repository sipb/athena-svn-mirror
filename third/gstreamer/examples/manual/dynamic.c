/* example-begin dynamic.c */
#include <string.h>
#include <gst/gst.h>

void 
eof (GstElement *src) 
{
  g_print ("have eos, quitting\n");
  exit (0);
}

gboolean 
idle_func (gpointer data) 
{
  gst_bin_iterate (GST_BIN (data));
  return TRUE;
}

void 
new_pad_created (GstElement *parse, GstPad *pad, GstElement *pipeline) 
{
  GstElement *decode_video = NULL;
  GstElement *decode_audio, *play, *color, *show;
  GstElement *audio_queue, *video_queue;
  GstElement *audio_thread, *video_thread;

  g_print ("***** a new pad %s was created\n", gst_pad_get_name (pad));
  
  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PAUSED);

  /* link to audio pad */
  if (strncmp (gst_pad_get_name (pad), "audio_", 6) == 0) {

    /* construct internal pipeline elements */
    decode_audio = gst_element_factory_make ("mad", "decode_audio");
    g_return_if_fail (decode_audio != NULL);
    play = gst_element_factory_make ("osssink", "play_audio");
    g_return_if_fail (play != NULL);

    /* create the thread and pack stuff into it */
    audio_thread = gst_thread_new ("audio_thread");
    g_return_if_fail (audio_thread != NULL);

    /* construct queue and link everything in the main pipeline */
    audio_queue = gst_element_factory_make ("queue", "audio_queue");
    g_return_if_fail (audio_queue != NULL);

    gst_bin_add_many (GST_BIN (audio_thread), 
                      audio_queue, decode_audio, play, NULL);

    /* set up pad links */
    gst_element_add_ghost_pad (audio_thread,
                               gst_element_get_pad (audio_queue, "sink"),
                               "sink");
    gst_element_link (audio_queue, decode_audio);
    gst_element_link (decode_audio, play);

    gst_bin_add (GST_BIN (pipeline), audio_thread);

    gst_pad_link (pad, gst_element_get_pad (audio_thread, "sink"));

    /* set up thread state and kick things off */
    g_print ("setting to READY state\n");
    gst_element_set_state (GST_ELEMENT (audio_thread), GST_STATE_READY);

  } 
  else if (strncmp (gst_pad_get_name (pad), "video_", 6) == 0) {

    /* construct internal pipeline elements */
    decode_video = gst_element_factory_make ("mpeg2dec", "decode_video");
    g_return_if_fail (decode_video != NULL);

    color = gst_element_factory_make ("colorspace", "color");
    g_return_if_fail (color != NULL);

   
    show = gst_element_factory_make ("xvideosink", "show");
    g_return_if_fail (show != NULL);

    /* construct queue and link everything in the main pipeline */
    video_queue = gst_element_factory_make ("queue", "video_queue");
    g_return_if_fail (video_queue != NULL);

    /* create the thread and pack stuff into it */
    video_thread = gst_thread_new ("video_thread");
    g_return_if_fail (video_thread != NULL);
    gst_bin_add_many (GST_BIN (video_thread), video_queue, 
                      decode_video, color, show, NULL);

    /* set up pad links */
    gst_element_add_ghost_pad (video_thread,
                               gst_element_get_pad (video_queue, "sink"),
                               "sink");
    gst_element_link (video_queue, decode_video);
    gst_element_link_many (decode_video, color, show, NULL);

    gst_bin_add (GST_BIN (pipeline), video_thread);

    gst_pad_link (pad, gst_element_get_pad (video_thread, "sink"));

    /* set up thread state and kick things off */
    g_print ("setting to READY state\n");
    gst_element_set_state (GST_ELEMENT (video_thread), GST_STATE_READY);
  }
  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);
}

int 
main (int argc, char *argv[]) 
{
  GstElement *pipeline, *src, *demux;

  gst_init (&argc, &argv);

  pipeline = gst_pipeline_new ("pipeline");
  g_return_val_if_fail (pipeline != NULL, -1);

  src = gst_element_factory_make ("filesrc", "src");
  g_return_val_if_fail (src != NULL, -1);
  if (argc < 2) 
    g_error ("Please specify a video file to play !");

  g_object_set (G_OBJECT (src), "location", argv[1], NULL);

  demux = gst_element_factory_make ("mpegdemux", "demux");
  g_return_val_if_fail (demux != NULL, -1);

  gst_bin_add_many (GST_BIN (pipeline), src, demux, NULL);

  g_signal_connect (G_OBJECT (demux), "new_pad",
                     G_CALLBACK (new_pad_created), pipeline);

  g_signal_connect (G_OBJECT (src), "eos",
                     G_CALLBACK (eof), NULL);

  gst_element_link (src, demux);

  gst_element_set_state (GST_ELEMENT (pipeline), GST_STATE_PLAYING);

  g_idle_add (idle_func, pipeline);

  gst_main ();

  return 0;
}
/* example-end dynamic.c */

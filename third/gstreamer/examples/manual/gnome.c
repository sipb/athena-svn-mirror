/* example-begin gnome.c */
#include <gnome.h>
#include <gst/gst.h>

int
main (int argc, char **argv)
{
  GstPoptOption options[] = {
          { NULL, '\0', POPT_ARG_INCLUDE_TABLE, NULL, 0, "GStreamer", NULL },
            POPT_TABLEEND
        };
  GnomeProgram *program;
  poptContext context;
  const gchar **argvn;

  GstElement *pipeline;
  GstElement *src, *sink;

  options[0].arg = (void *) gst_init_get_popt_table ();
  g_print ("Calling gnome_program_init with the GStreamer popt table\n");
  /* gnome_program_init will initialize GStreamer now
   * as a side effect of having the GStreamer popt table passed. */
  if (! (program = gnome_program_init ("my_package", "0.1", LIBGNOMEUI_MODULE,
                                       argc, argv,
                                       GNOME_PARAM_POPT_TABLE, options,
                                       NULL)))
    g_error ("gnome_program_init failed");

  g_print ("Getting gnome-program popt context\n");
  g_object_get (program, "popt-context", &context, NULL);
  argvn = poptGetArgs (context);
  if (!argvn) {
    g_print ("Run this example with some arguments to see how it works.\n");
    return 0;
  }

  g_print ("Printing rest of arguments\n");
  while (*argvn) {
    g_print ("argument: %s\n", *argvn);
    ++argvn;
  }

  /* do some GStreamer things to show everything's initialized properly */
  g_print ("Doing some GStreamer stuff to show that everything works\n");
  pipeline = gst_pipeline_new ("pipeline");
  src = gst_element_factory_make ("fakesrc", "src");
  sink = gst_element_factory_make ("fakesink", "sink");
  gst_bin_add_many (GST_BIN (pipeline), src, sink, NULL);
  gst_element_link (src, sink);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);
  gst_bin_iterate (GST_BIN (pipeline));
  gst_element_set_state (pipeline, GST_STATE_NULL);

  return 0;
}
/* example-end gnome.c */

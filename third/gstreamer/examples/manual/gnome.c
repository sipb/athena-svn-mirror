/* example-begin gnome.c */
#include <gnome.h>
#include <gst/gst.h>

int
main (int argc, char **argv)
{
  struct poptOption options[] = {
          { NULL, '\0', POPT_ARG_INCLUDE_TABLE, NULL, 0, "GStreamer", NULL },
            POPT_TABLEEND
        };
  GnomeProgram *program;
  poptContext context;
  const gchar **argvn;

  options[0].arg = (void *) gst_init_get_popt_table ();
  if (! (program = gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE,
                                       argc, argv,
                                       GNOME_PARAM_POPT_TABLE, options,
                                       NULL)))
  g_error ("gnome_program_init failed");

  g_object_get (program, "popt-context", &context, NULL);
  argvn = poptGetArgs (context);
  if (!argvn) {
    g_print ("Run this example with some arguments to see how it works.\n");
    return 0;
  }

  while (*argvn) {
    g_print ("argument: %s\n", *argvn);
    ++argvn;
  }

  return 0;
}
/* example-end gnome.c */


/*** block  from ../../docs/manual/basics-init.xml ***/
#include <gst/gst.h>

int
main (int   argc,
      char *argv[])
{
  gboolean silent = FALSE;
  gchar *savefile = NULL;
  struct poptOption options[] = {
    {"silent",	's',  POPT_ARG_NONE|POPT_ARGFLAG_STRIP,   &silent,   0,
     "do not output status information", NULL},
    {"output",	'o',  POPT_ARG_STRING|POPT_ARGFLAG_STRIP, &savefile, 0,
     "save xml representation of pipeline to FILE and exit", "FILE"},
    POPT_TABLEEND
  };

  gst_init_with_popt_table (&argc, &argv, options);

  printf ("Run me with --help to see the Application options appended.\n");

  return 0;
}


/*** block  from ../../docs/manual/basics-init.xml ***/
#include <gst/gst.h>

int
main (int   argc,
      char *argv[])
{
  guint major, minor, micro;

  gst_init (&argc, &argv);

  gst_version (&major, &minor, &micro);
  printf ("This program is linked against GStreamer %d.%d.%d\n",
          major, minor, micro);

  return 0;
}

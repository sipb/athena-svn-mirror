
/*** block a  from ../../docs/manual/appendix-integration.xml ***/
#include <gnome.h>
#include <gst/gst.h>

gint
main (gint   argc,
      gchar *argv[])
{
  struct poptOption options[] = {
    {NULL, '\0', POPT_ARG_INCLUDE_TABLE, NULL, 0, "GStreamer", NULL},
    POPT_TABLEEND
  };

  /* init GStreamer and GNOME using the GStreamer popt tables */
  options[0].arg = (void *) gst_init_get_popt_table ();
  gnome_program_init ("my-application", "0.0.1", LIBGNOMEUI_MODULE, argc, argv,
		      GNOME_PARAM_POPT_TABLE, options,
		      NULL);

/*** block b  from ../../docs/manual/appendix-integration.xml ***/
  return 0;

/*** block c  from ../../docs/manual/appendix-integration.xml ***/
}

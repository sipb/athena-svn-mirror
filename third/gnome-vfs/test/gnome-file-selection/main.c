#include <gtk/gtk.h>
#include <gnome.h>
#include <libgnorba/gnorba.h>

#include "libgnomevfs/gnome-vfs.h"

#include "gnome-file-selection.h"

static void
delete_event (GtkObject *object, gpointer data)
{
  gtk_main_quit ();

  chdir ("/home/ettore/gnome/gnome-vfs/build/test/gnome-file-selection");
}

int
main (int argc, char **argv)
{
  GtkWidget *fs;

#ifdef WITH_CORBA

  CORBA_Environment ev;

  CORBA_exception_init (&ev);

  gnome_CORBA_init ("TestFileSelection", "0.0", &argc, argv, 0, &ev);

#else

  g_thread_init (NULL);
  gnome_init ("TestFileSelection", "0.0", argc, argv);

#endif

  if (argc == 2)
    chdir (argv[1]);

  gnome_vfs_init ();

  fs = gnome_file_selection_new ("test");

  gnome_file_selection_append_filter (GNOME_FILE_SELECTION (fs),
                                      "All files",
                                      "*\0");
  gnome_file_selection_append_filter (GNOME_FILE_SELECTION (fs),
                                      "Pixmap files",
                                      "*.jpg\0*.gif\0*.png\0");
  gnome_file_selection_append_filter (GNOME_FILE_SELECTION (fs),
                                      "Compressed tar files",
                                      "*.tar.gz\0");

  gtk_signal_connect (GTK_OBJECT (fs), "delete_event",
                      GTK_SIGNAL_FUNC (delete_event), NULL);
  gtk_widget_show (GTK_WIDGET (fs));

  gtk_main ();

  return 0;
}

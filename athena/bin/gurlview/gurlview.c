/* Display a URL via the user's configured GNOME handler.
 *
 * $Id: gurlview.c,v 1.1 2002-06-24 13:32:31 rbasch Exp $
 */

#include <libgnomeui/libgnomeui.h>
#include <libgnome/gnome-url.h>

static void usage();

int main(int argc, char **argv)
{
  if (gnome_init("gurlview", "", argc, argv) != 0)
    {
      fprintf(stderr, "gurlview: could not initialize GNOME library\n");
      exit(1);
    }

  if (argc != 2)
    usage();

  /* Note that gnome_url_show's return is void. */
  gnome_url_show(argv[1]);

  exit(0);
}

static void usage()
{
  fprintf(stderr, "gurlview: Usage: gurlview <URL>\n");
  exit(1);
}

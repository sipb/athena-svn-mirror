/* Bonobo component browser
 *
 * AUTHORS:
 *      Dan Siemon <dan@coverfire.com>
 *      Rodrigo Moya <rodrigo@gnome-db.org>
 *      Patanjali Somayaji <patanjali@morelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "bonobo-browser.h"
#include <bonobo/bonobo-main.h>
#include <gtk/gtk.h>
#ifdef ENABLE_NLS
#include <locale.h>
#endif

int
main (int argc, char *argv [])
{
#ifdef ENABLE_NLS
	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, BONOBO_SUPPORT_LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif
					
	gtk_init (&argc, &argv);
	bonobo_init (&argc, argv);

	bonobo_browser_create_window ();

	/* run the application */
	bonobo_main ();
	return 0;
}

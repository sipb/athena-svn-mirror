#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <libgnome/gnome-i18n.h>
#include <string.h>
#include <gnome.h>
#include <bonobo.h>
#include <stdio.h>
#include <glib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gtk/gtkscrolledwindow.h>

#include <glade/glade.h>

#include "Editor.h"

#include "gtkhtml.h"
#include "gtkhtml-properties.h"
#include "htmlcursor.h"
#include "htmlengine.h"
#include "htmlengine-edit.h"
#include "htmlengine-edit-cut-and-paste.h"
#include "htmlengine-edit-movement.h"
#include "htmlengine-edit-selection-updater.h"
#include "htmlimage.h"
#include "htmlinterval.h"
#include "htmlselection.h"
#include "htmlfontmanager.h"
#include "htmlsettings.h"
#include "htmlpainter.h"
#include "htmlplainpainter.h"

#include "engine.h"
#include "menubar.h"
#include "persist-file.h"
#include "persist-stream.h"
#include "popup.h"
#include "toolbar.h"
#include "properties.h"
#include "text.h"
#include "paragraph.h"
#include "body.h"
#include "spell.h"
#include "html-stream-mem.h"

#include "gtkhtmldebug.h"
#include "editor-control-factory.h"

int
main (int argc, char **argv)
{
	BonoboGenericFactory *factory;
#ifdef GTKHTML_HAVE_GCONF
	GError  *gconf_error  = NULL;
#endif

	/* Initialize the i18n support */
	bindtextdomain (GNOME_EXPLICIT_TRANSLATION_DOMAIN, GNOMELOCALEDIR);
	bind_textdomain_codeset (GNOME_EXPLICIT_TRANSLATION_DOMAIN, "UTF-8");

	gnome_program_init(PACKAGE, VERSION, LIBGNOMEUI_MODULE, argc, argv, 
			   GNOME_PROGRAM_STANDARD_PROPERTIES,
			   GNOME_PARAM_HUMAN_READABLE_NAME, _("GtkHTML Editor Control"),			   
			   NULL);

	/* #ifdef GTKHTML_HAVE_GCONF
	if (!gconf_init (argc, argv, &gconf_error)) {
		g_assert (gconf_error != NULL);
		g_error ("GConf init failed:\n  %s", gconf_error->message);
		return 1;
	}
	#endif */

	factory = bonobo_generic_factory_new (CONTROL_FACTORY_ID, editor_control_factory, NULL);

	if (factory) {
		bonobo_running_context_auto_exit_unref (BONOBO_OBJECT (factory));
	
		bonobo_activate ();
		bonobo_main ();

		return bonobo_ui_debug_shutdown ();
	} else
		return 1;

}

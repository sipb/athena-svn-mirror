/**
 * application/pdf Bonobo Generic Factory.
 *
 * Author:
 *   Michael Meeks <michael@ximian.com>
 *
 * Copyright 1999, 2000 Ximian, Inc.
 */

#include <aconf.h>
#include <stdio.h>
#include <config.h>

#include "gpdf-g-switch.h"
extern "C" { /* Aargh */
#  include <bonobo/bonobo-ui-main.h>
#  include <bonobo/bonobo-generic-factory.h>
#  include <libgnomeui/gnome-ui-init.h>
#  include <libgnomeui/gnome-client.h>
}
#include "gpdf-g-switch.h"
#include "gpdf-persist-stream.h"
#include "gpdf-persist-file.h"
#include "gpdf-control.h"
#include "nautilus-pdf-property-page.h"

#include "GlobalParams.h"
#include "gpdf-stock-icons.h"

static BonoboObject *
gpdf_factory (BonoboGenericFactory *factory,
	      const char *component_id,
	      gpointer closure)
{
	g_return_val_if_fail (factory != NULL, NULL);
	g_return_val_if_fail (component_id != NULL, NULL);

	if (!strcmp (component_id, "OAFIID:GNOME_PDF_Control")) {
                GPdfPersistStream *persist_stream;
		GPdfPersistFile *persist_file;
                GPdfControl *control;

                persist_stream = gpdf_persist_stream_new (component_id);

                if (persist_stream == NULL) {
                        g_warning ("Could not create GPdfPersistStream");
                        return NULL;
                }

                persist_file = gpdf_persist_file_new (component_id);

                if (persist_file == NULL) {
                        g_warning ("Could not create GPdfPersistFile");
                        return NULL;
                }

		control = GPDF_CONTROL (
			g_object_new (GPDF_TYPE_CONTROL,
				      "persist_stream", persist_stream,
				      "persist_file", persist_file,
				      NULL));

                bonobo_object_unref (BONOBO_OBJECT (persist_stream));	
                bonobo_object_unref (BONOBO_OBJECT (persist_file));	
		bonobo_control_life_instrument (BONOBO_CONTROL (control));

                return BONOBO_OBJECT (control);        
        } else if (!strcmp (component_id,
                            "OAFIID:GNOME_PDF_NautilusPropertyPage")) {
                BonoboControl *control = BONOBO_CONTROL (
                        g_object_new (GPDF_TYPE_NAUTILUS_PROPERTY_PAGE, NULL));
		bonobo_control_life_instrument (control);
		return BONOBO_OBJECT (control);
	} else {
		g_warning ("Unknown IID %s requested", component_id);
		return NULL;
	}
}

int
main (int argc, char *argv [])
{
        int retval;

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, GNOMELOCALEDIR);
        bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif
	gnome_program_init ("gnome-pdf-version", 0,
			    LIBGNOMEUI_MODULE,
			    argc, argv,
			    GNOME_CLIENT_PARAM_SM_CONNECT, FALSE,
			    NULL); 

        BONOBO_FACTORY_INIT ("gnome-pdf-viewer", VERSION, &argc, argv);
  
	//FIXME
	globalParams = new GlobalParams("");
        gpdf_stock_icons_init ();

	retval = bonobo_generic_factory_main ("OAFIID:GNOME_PDF_Factory",
                                              gpdf_factory, NULL);

	delete globalParams;

	return retval;
}

/* $Id: bonobo-hello.c,v 1.1.1.1 2001-01-16 15:26:02 ghudson Exp $ */
/*
  Bonobo-Hello Copyright (C) 2000 ÉRDI Gergõ <cactus@cactus.rulez.org>
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2
  (included in the RadioActive distribution in doc/GPL) as published by
  the Free Software Foundation.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <gnome.h>
#include "config.h"
#include <liboaf/liboaf.h>
#include <bonobo.h>

#include "hello-embeddable.h"

static BonoboGenericFactory *factory = NULL;
static gint running_objects = 0;

static void
hello_bonobo_destroy (BonoboEmbeddable *embeddable,
		      gpointer user_data)
{
	running_objects--;
	if (running_objects > 0)
		return;

	if (factory)
		bonobo_object_unref (BONOBO_OBJECT (factory));
	else
		g_warning ("Serious ref counting error");
	factory = NULL;

	gtk_main_quit ();
}

static BonoboObject*
hello_embeddable_factory (BonoboGenericFactory *f, gpointer data)
{
	HelloBonoboEmbeddable *embeddable;

	embeddable = gtk_type_new (HELLO_BONOBO_EMBEDDABLE_TYPE);

	g_return_val_if_fail(embeddable != NULL, NULL);

	running_objects++;

	/* Install destructor */
	gtk_signal_connect (GTK_OBJECT(embeddable), "destroy",
			    GTK_SIGNAL_FUNC(hello_bonobo_destroy),
			    NULL);

	embeddable = hello_bonobo_embeddable_construct (embeddable);

	return BONOBO_OBJECT (embeddable);
}

static void
hello_bonobo_init (void)
{
	factory =
		bonobo_generic_factory_new (
			"OAFIID:Bonobo_Sample_Hello_EmbeddableFactory",
			hello_embeddable_factory, NULL);
	if (!factory)
		g_warning ("Couldn't register hello object factory");
}

static void
server_factory_init (int argc, char **argv)
{
	CORBA_Environment ev;
	CORBA_ORB orb;

	CORBA_exception_init (&ev);

	gnome_init_with_popt_table (PACKAGE, VERSION,
				    argc, argv, oaf_popt_options, 0, NULL);
	orb = oaf_init (argc, argv);

	if (!bonobo_init (orb,
			  CORBA_OBJECT_NIL,
			  CORBA_OBJECT_NIL))
		    g_error (_("I could not initialize Bonobo"));

	CORBA_exception_free (&ev);
}

int
main (int argc, char **argv)
{
	server_factory_init (argc, argv);
	hello_bonobo_init ();

	bonobo_main ();

	return 0;
}


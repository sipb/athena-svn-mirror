/*
 * main.c: Startup code for the Echo Bonobo Component.
 *
 * Author:
 *   Miguel de Icaza (miguel@helixcode.com)
 *
 * (C) 1999, 2000 Helix Code, Inc.  http://www.helixcode.com
 */
#include <config.h>
#include <bonobo.h>

#include "Bonobo_Sample_Echo.h"
#include "echo.h"

static BonoboObject *
echo_factory (BonoboGenericFactory *this_factory, void *data)
{
	Echo *echo;

	echo = echo_new ();
	
	if (echo == NULL) 
		return NULL;
   
	return BONOBO_OBJECT (echo);
}

BONOBO_OAF_FACTORY ("OAFIID:Bonobo_Sample_Echo_Factory",
		    "echo", VERSION, 
		    echo_factory,
		    NULL)

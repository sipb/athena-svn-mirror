/*
 * gnome-moniker-cache.c: 
 *
 * Author:
 *	Dietmar Maurer (dietmar@helixcode.com)
 *
 * Copyright 2000 Helix Code, Inc.
 */
#include <config.h>
#include <bonobo/bonobo-storage.h>
#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-moniker-extender.h>
#include <bonobo/bonobo-moniker-util.h>
#include <libgnome/gnome-mime.h>
#include <liboaf/liboaf.h>

#include "bonobo-moniker-std.h"

Bonobo_Unknown
bonobo_moniker_cache_resolve (BonoboMoniker               *moniker,
			      const Bonobo_ResolveOptions *options,
			      const CORBA_char            *requested_interface,
			      CORBA_Environment           *ev)
{
	g_warning ("cache_resolve: not implemented");

	return CORBA_OBJECT_NIL; /* use the extender */
}

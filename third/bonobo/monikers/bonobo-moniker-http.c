/*
 * bonobo-moniker-http.c: HTTP based Moniker
 *
 * Author:
 *   Joe Shaw (joe@helixcode.com)
 *
 * Copyright (c) 2000 Helix Code, Inc.
 */
#include <config.h>
#include <bonobo/bonobo.h>

#include "bonobo-moniker-http.h"

static Bonobo_Unknown
http_resolve (BonoboMoniker *moniker,
	      const Bonobo_ResolveOptions *options,
	      const CORBA_char *requested_interface,
	      CORBA_Environment *ev)
{
	const char *url = bonobo_moniker_get_name (moniker);
	char *real_url;

	g_warning ("Going to resolve the http now");

	/* because resolving the moniker drops the "http:" */
	real_url = g_strconcat ("http:", url, NULL);

	if (strcmp (requested_interface, "IDL:Bonobo/Control:1.0") == 0) {
		BonoboObjectClient *client;
		Bonobo_Unknown object;

		client = bonobo_object_activate ("OAFIID:GNOME_GtkHTML_EBrowser", 0);

		if (!client) {
			/* FIXME: Set a InterfaceNotFound exception here? */
			return CORBA_OBJECT_NIL;
		}

		object = BONOBO_OBJREF (client);
			
		if  (ev->_major != CORBA_NO_EXCEPTION)
			return CORBA_OBJECT_NIL;

		if  (object == CORBA_OBJECT_NIL) {
			g_warning ("Can't find object satisfying requirements");
			CORBA_exception_set  (
				ev, CORBA_USER_EXCEPTION,
				ex_Bonobo_Moniker_InterfaceNotFound, NULL);
			return CORBA_OBJECT_NIL;
		}

		return bonobo_moniker_util_qi_return (
			object, requested_interface, ev);
	}
	else if (strcmp (requested_interface, "IDL:Bonobo/Stream:1.0") == 0) {
		BonoboStream *stream;

		stream = bonobo_stream_open_full (
			"http", real_url, Bonobo_Storage_READ, 0644, ev);

		if (!stream) {
			g_warning ("Failed to open stream '%s'", real_url);
			g_free (real_url);
			CORBA_exception_set (
				ev, CORBA_USER_EXCEPTION,
				ex_Bonobo_Moniker_InterfaceNotFound, NULL);

			return CORBA_OBJECT_NIL;
		}

		g_free (real_url);
		return CORBA_Object_duplicate (BONOBO_OBJREF (stream), ev);
	}

	return CORBA_OBJECT_NIL;
}

static BonoboObject *
bonobo_moniker_http_factory (BonoboGenericFactory *this, void *closure)
{
	return BONOBO_OBJECT (bonobo_moniker_simple_new (
		"http:", http_resolve));
}

BONOBO_OAF_FACTORY ("OAFIID:Bonobo_Moniker_http_Factory",
		    "http-moniker", VERSION,
		    bonobo_moniker_http_factory,
		    NULL)

/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-moniker-simple: Simplified object naming abstraction
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright 2000, Helix Code, Inc.
 */
#include <config.h>

#include <bonobo/bonobo-moniker.h>
#include <bonobo/bonobo-moniker-simple.h>

static Bonobo_Unknown
simple_resolve (BonoboMoniker               *moniker,
		const Bonobo_ResolveOptions *options,
		const CORBA_char            *requested_interface,
		CORBA_Environment           *ev)
{
	BonoboMonikerSimple *simple;

	g_return_val_if_fail (BONOBO_IS_MONIKER_SIMPLE (moniker),
			      CORBA_OBJECT_NIL);

	simple = BONOBO_MONIKER_SIMPLE (moniker);

	return simple->resolve_fn (
		moniker, options, requested_interface, ev);
}

static void
bonobo_moniker_simple_class_init (BonoboMonikerClass *klass)
{
	klass->resolve = simple_resolve;
}

/**
 * bonobo_moniker_simple_get_type:
 *
 * Returns: the GtkType for a BonoboMonikerSimple.
 */
GtkType
bonobo_moniker_simple_get_type (void)
{
	static GtkType type = 0;

	if (!type) {
		GtkTypeInfo info = {
			"BonoboMonikerSimple",
			sizeof (BonoboMonikerSimple),
			sizeof (BonoboMonikerSimpleClass),
			(GtkClassInitFunc)  bonobo_moniker_simple_class_init,
			(GtkObjectInitFunc) NULL,
			NULL, /* reserved 1 */
			NULL, /* reserved 2 */
			(GtkClassInitFunc) NULL
		};

		type = gtk_type_unique (bonobo_moniker_get_type (), &info);
	}

	return type;
}

/**
 * bonobo_moniker_simple_construct:
 * @moniker: 
 * @corba_moniker: 
 * @name: 
 * @resolve_fn: 
 * 
 * Constructs a simple moniker
 * 
 * Return value: the constructed moniker or NULL on failure.
 **/
BonoboMoniker *
bonobo_moniker_simple_construct (BonoboMonikerSimple         *moniker,
				 Bonobo_Moniker               corba_moniker,
				 const char                  *name,
				 BonoboMonikerSimpleResolveFn resolve_fn)
{
	g_return_val_if_fail (resolve_fn != NULL, NULL);

	moniker->resolve_fn = resolve_fn;

	return bonobo_moniker_construct (
		BONOBO_MONIKER (moniker), corba_moniker, name);
}

/**
 * bonobo_moniker_simple_new:
 * @name: the display name for the moniker
 * @resolve_fn: a resolve function for the moniker
 * 
 * Create a new instance of a simplified moniker.
 * 
 * Return value: the moniker object
 **/
BonoboMoniker *
bonobo_moniker_simple_new (const char                  *name,
			   BonoboMonikerSimpleResolveFn resolve_fn)
{
	BonoboMoniker *moniker;

	moniker = gtk_type_new (bonobo_moniker_simple_get_type ());

	return bonobo_moniker_simple_construct (
		BONOBO_MONIKER_SIMPLE (moniker), CORBA_OBJECT_NIL,
		name, resolve_fn);
}


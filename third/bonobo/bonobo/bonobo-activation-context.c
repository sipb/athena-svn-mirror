/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/**
 * bonobo-activation-context.c: A global activation interface
 *
 * Author:
 *	Michael Meeks (michael@helixcode.com)
 *
 * Copyright (C) 2000, Helix Code, Inc.
 */
#include <config.h>
#include <gtk/gtksignal.h>

#include <bonobo/bonobo-exception.h>
#include <bonobo/bonobo-moniker-util.h>
#include <bonobo/bonobo-moniker-extender.h>
#include <bonobo/bonobo-activation-context.h>

POA_Bonobo_ActivationContext__vepv bonobo_activation_context_vepv;

static Bonobo_Moniker
impl_Bonobo_ActivationContext_createWithParent (PortableServer_Servant servant,
						Bonobo_Moniker         optParent,
						const CORBA_char      *name,
						CORBA_Environment     *ev)
{
	return bonobo_moniker_util_new_from_name_full (
		optParent, name, ev);
}

static Bonobo_Moniker
impl_Bonobo_ActivationContext_createFromName (PortableServer_Servant servant,
					      const CORBA_char      *name,
					      CORBA_Environment     *ev)
{
	return impl_Bonobo_ActivationContext_createWithParent (
		servant, CORBA_OBJECT_NIL, name, ev);
}

static Bonobo_Unknown
impl_Bonobo_ActivationContext_getObject (PortableServer_Servant servant,
					 const CORBA_char      *name,
					 const CORBA_char      *repo_id,
					 CORBA_Environment     *ev)
{
	return bonobo_get_object (name, repo_id, ev);
}

static Bonobo_MonikerExtender
impl_Bonobo_ActivationContext_getExtender (PortableServer_Servant servant,
					   const CORBA_char      *monikerPrefix,
					   const CORBA_char      *interfaceId,
					   CORBA_Environment     *ev)
{
	return bonobo_moniker_find_extender (monikerPrefix, interfaceId, ev);
}

/**
 * bonobo_activation_context_get_epv:
 *
 * Returns: The EPV for the default BonoboActivationContext implementation. 
 */
static POA_Bonobo_ActivationContext__epv *
bonobo_activation_context_get_epv (void)
{
	POA_Bonobo_ActivationContext__epv *epv;

	epv = g_new0 (POA_Bonobo_ActivationContext__epv, 1);

	epv->getObject        = impl_Bonobo_ActivationContext_getObject;
	epv->createFromName   = impl_Bonobo_ActivationContext_createFromName;
	epv->createWithParent = impl_Bonobo_ActivationContext_createWithParent;
	epv->getExtender      = impl_Bonobo_ActivationContext_getExtender;

	return epv;
}

static void
init_activation_context_corba_class (void)
{
	/* The VEPV */
	bonobo_activation_context_vepv.Bonobo_Unknown_epv           = bonobo_object_get_epv ();
	bonobo_activation_context_vepv.Bonobo_ActivationContext_epv = bonobo_activation_context_get_epv ();
}

static void
bonobo_activation_context_class_init (BonoboObjectClass *klass)
{
	init_activation_context_corba_class ();
}

static GtkType
bonobo_activation_context_get_type (void)
{
        static GtkType type = 0;

        if (!type) {
                GtkTypeInfo info = {
                        "BonoboActivationContext",
                        sizeof (BonoboActivationContext),
                        sizeof (BonoboObjectClass),
                        (GtkClassInitFunc) bonobo_activation_context_class_init,
                        (GtkObjectInitFunc) NULL,
                        NULL, /* reserved 1 */
                        NULL, /* reserved 2 */
                        (GtkClassInitFunc) NULL
                };

                type = gtk_type_unique (bonobo_object_get_type (), &info);
        }

        return type;
}

static Bonobo_ActivationContext
bonobo_activation_context_corba_object_create (BonoboObject *object)
{
        POA_Bonobo_ActivationContext *servant;
        CORBA_Environment ev;

        servant = (POA_Bonobo_ActivationContext *) g_new0 (BonoboObjectServant, 1);
        servant->vepv = &bonobo_activation_context_vepv;

        CORBA_exception_init (&ev);

        POA_Bonobo_ActivationContext__init ((PortableServer_Servant) servant, &ev);
        if (BONOBO_EX (&ev)) {
                g_free (servant);
                CORBA_exception_free (&ev);
                return CORBA_OBJECT_NIL;
        }

        CORBA_exception_free (&ev);

        return bonobo_object_activate_servant (object, servant);
}

BonoboObject *
bonobo_activation_context_new (void)
{
	BonoboObject *object;
	Bonobo_ActivationContext corba_activation_context;

	object = gtk_type_new (bonobo_activation_context_get_type ());

	corba_activation_context =
		bonobo_activation_context_corba_object_create (object);

	if (corba_activation_context == CORBA_OBJECT_NIL) {
		bonobo_object_unref (BONOBO_OBJECT (object));
		return NULL;
	}

        return bonobo_object_construct (object, corba_activation_context);
}

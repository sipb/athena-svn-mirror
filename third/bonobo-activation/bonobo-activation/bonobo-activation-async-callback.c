/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  oaf-async: A library for accessing oafd in a nice way.
 *
 *  Copyright (C) 2000 Eazel, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Mathieu Lacage <mathieu@eazel.com>
 *
 */

#include <config.h>

#include <string.h>
#include <bonobo-activation/bonobo-activation-async.h>
#include <bonobo-activation/bonobo-activation-async-callback.h>
#include <bonobo-activation/bonobo-activation-i18n.h>
#include <bonobo-activation/bonobo-activation-init.h>
#include <bonobo-activation/bonobo-activation-shlib.h>
#include <bonobo-activation/Bonobo_ActivationContext.h>

/*** App-specific servant structures ***/

typedef struct {
        POA_Bonobo_ActivationCallback servant;
        PortableServer_POA            poa;
        BonoboActivationCallback      callback;
        gpointer                      user_data;
        CORBA_Object                  objref;
} impl_POA_Bonobo_ActivationCallback;


/*** Stub implementations ***/

static void
impl_Bonobo_ActivationCallback__finalize (
        PortableServer_Servant servant,
        CORBA_Environment *ev)
{
        g_free (servant);
}

static void
impl_Bonobo_ActivationCallback__destroy (
        impl_POA_Bonobo_ActivationCallback *servant, 
        CORBA_Environment *ev)
{
        PortableServer_ObjectId *objid;

        objid = PortableServer_POA_servant_to_id (servant->poa, servant, ev);
        PortableServer_POA_deactivate_object (servant->poa, objid, ev);
        CORBA_free (objid);

        CORBA_Object_release (servant->objref, ev);
}

static void
impl_Bonobo_ActivationCallback_report_activation_failed (
        PortableServer_Servant _servant,
        const CORBA_char * reason,
        CORBA_Environment * ev)
{
        char *message;
        impl_POA_Bonobo_ActivationCallback * servant;

        servant = (impl_POA_Bonobo_ActivationCallback *) _servant;

        message = g_strconcat ("Activation failed: ", reason, NULL);
        servant->callback (CORBA_OBJECT_NIL, message, servant->user_data);
        g_free (message);
        
        /* destroy this object */
        impl_Bonobo_ActivationCallback__destroy (servant, ev);
}

static void
impl_Bonobo_ActivationCallback_report_activation_succeeded (
        PortableServer_Servant _servant,
        const Bonobo_ActivationResult * result,
        CORBA_Environment * ev)
{
        CORBA_Object retval;
        impl_POA_Bonobo_ActivationCallback * servant;

        servant = (impl_POA_Bonobo_ActivationCallback *) _servant;

        retval = CORBA_OBJECT_NIL;

	switch (result->res._d) {
	case Bonobo_ACTIVATION_RESULT_SHLIB:
                retval = bonobo_activation_activate_shlib_server (
                        (Bonobo_ActivationResult *) result, ev);
		break;
	case Bonobo_ACTIVATION_RESULT_OBJECT:
		retval = CORBA_Object_duplicate (result->res._u.res_object, ev);
		break;
	case Bonobo_ACTIVATION_RESULT_NONE:
                retval = CORBA_OBJECT_NIL;
                break;
	default:
                g_assert_not_reached ();
		break;
	}

        if (retval == CORBA_OBJECT_NIL) {
                const char * msg = _("No server corresponding to your query");

                if ((ev) && (ev)->_major != CORBA_NO_EXCEPTION &&
                    !strcmp(ev->_id, "IDL:Bonobo/GeneralError:1.0")) {
                        Bonobo_GeneralError *err = ev->_any._value;

                        if (err && err->description)
                                msg = err->description;
                }
                servant->callback (CORBA_OBJECT_NIL, msg, servant->user_data);
        } else {
                servant->callback (retval, NULL, servant->user_data);
        }

        /* destroy this object */
        impl_Bonobo_ActivationCallback__destroy (servant, ev);
}

/*** epv structures ***/

static PortableServer_ServantBase__epv impl_Bonobo_ActivationCallback_base_epv = {
        NULL, /* _private data */
        impl_Bonobo_ActivationCallback__finalize,
        NULL, /* default_POA routine */
};
static POA_Bonobo_ActivationCallback__epv impl_Bonobo_ActivationCallback_epv = {
        NULL, /* _private */
        &impl_Bonobo_ActivationCallback_report_activation_failed,
        &impl_Bonobo_ActivationCallback_report_activation_succeeded,
};

/* FIXME: fill me in / deal with me globaly */
static POA_Bonobo_Unknown__epv impl_Bonobo_Unknown_epv = {
	NULL, /* _private data */
	NULL,
	NULL,
        NULL
};

/*** vepv structures ***/

static POA_Bonobo_ActivationCallback__vepv impl_Bonobo_ActivationCallback_vepv = {
        &impl_Bonobo_ActivationCallback_base_epv,
        &impl_Bonobo_Unknown_epv,
        &impl_Bonobo_ActivationCallback_epv,
};


CORBA_Object
bonobo_activation_async_corba_callback_new (BonoboActivationCallback callback,
                                            gpointer user_data,
                                            CORBA_Environment * ev)
{
   CORBA_Object retval;
   impl_POA_Bonobo_ActivationCallback *newservant;
   PortableServer_ObjectId *objid;
   PortableServer_POA poa;
   PortableServer_POAManager manager;
   CORBA_ORB orb;

   g_return_val_if_fail (callback != NULL, CORBA_OBJECT_NIL);

   orb = bonobo_activation_orb_get ();

   poa =  (PortableServer_POA) CORBA_ORB_resolve_initial_references (orb, "RootPOA", ev);
   manager = PortableServer_POA__get_the_POAManager (poa, ev);
   PortableServer_POAManager_activate (manager, ev);

   newservant = g_new0(impl_POA_Bonobo_ActivationCallback, 1);
   newservant->servant.vepv = &impl_Bonobo_ActivationCallback_vepv;
   newservant->poa = poa;
   newservant->callback = callback;
   newservant->user_data = user_data;

   POA_Bonobo_ActivationCallback__init((PortableServer_Servant) newservant, ev);
   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   newservant->objref = retval;

   CORBA_Object_release ((CORBA_Object) manager, ev);
   CORBA_Object_release ((CORBA_Object) poa, ev);

   return retval;
}


/* $Id: authn-callback-wrapper.c,v 1.1.1.1 2001-01-16 15:25:55 ghudson Exp $
 * 
 * Copyright (C) 2000 Eazel, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Author:  Michael Fleming <mfleming@eazel.com>
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "libammonite.h"
#include <glib.h>

/* TRUE if no CORBA exception has occured */
#define NO_CORBA_EXCEPTION(ev)  (CORBA_NO_EXCEPTION == (ev)._major)

static void
impl_EazelProxy_AuthnCallback_succeeded (
	PortableServer_Servant servant,
	const EazelProxy_User * user,
	CORBA_Environment *ev
);
static void
impl_EazelProxy_AuthnCallback_failed (
	PortableServer_Servant servant, 
	const EazelProxy_User * user, 
	const EazelProxy_AuthnFailInfo *info, 
	CORBA_Environment *ev
);

typedef struct {
	POA_EazelProxy_AuthnCallback 	poa;
	AmmoniteAuthCallbackWrapperFuncs funcs;
	gpointer user_data;
} impl_POA_EazelProxy_AuthnCallback;

/*******************************************************************
 * Module Globals
 *******************************************************************/

static PortableServer_ServantBase__epv base_epv = { NULL, NULL, NULL };
static POA_EazelProxy_AuthnCallback__epv authncallback_epv = {
	NULL,
	impl_EazelProxy_AuthnCallback_succeeded,
	impl_EazelProxy_AuthnCallback_failed
};

static POA_EazelProxy_AuthnCallback__vepv authncallback_vepv = {
	&base_epv, &authncallback_epv
};

/*******************************************************************
 * EazelProxy_AuthnCallback impl
 *******************************************************************/

static void
impl_EazelProxy_AuthnCallback_succeeded (
	PortableServer_Servant servant,
	const EazelProxy_User * user,
	CORBA_Environment *ev
) {
	impl_POA_EazelProxy_AuthnCallback *my_servant;
	my_servant =  (impl_POA_EazelProxy_AuthnCallback *) servant;

	if (my_servant->funcs.succeeded) {
		my_servant->funcs.succeeded (user, my_servant->user_data, ev);
	}
}

static void
impl_EazelProxy_AuthnCallback_failed (
	PortableServer_Servant servant, 
	const EazelProxy_User * user, 
	const EazelProxy_AuthnFailInfo *info, 
	CORBA_Environment *ev
) {
	impl_POA_EazelProxy_AuthnCallback *my_servant;
	my_servant =  (impl_POA_EazelProxy_AuthnCallback *) servant;

	if (my_servant->funcs.failed) {
		my_servant->funcs.failed (user, info, my_servant->user_data, ev);
	}
}


EazelProxy_AuthnCallback
ammonite_auth_callback_wrapper_new (
	PortableServer_POA poa, 
	const AmmoniteAuthCallbackWrapperFuncs *funcs,
	gpointer user_data
) {

	CORBA_Environment ev;
	impl_POA_EazelProxy_AuthnCallback *servant;
	EazelProxy_AuthnCallback ret;

	g_return_val_if_fail ( NULL != funcs, CORBA_OBJECT_NIL);	
	
	CORBA_exception_init (&ev);

	servant = g_new0 (impl_POA_EazelProxy_AuthnCallback, 1);

	servant->poa.vepv = &authncallback_vepv;

	memcpy ( &(servant->funcs), funcs, sizeof(*funcs));
	servant->user_data = user_data;

	POA_EazelProxy_AuthnCallback__init (servant, &ev);

	CORBA_free (PortableServer_POA_activate_object (poa, servant, &ev));

	ret = (EazelProxy_AuthnCallback)PortableServer_POA_servant_to_reference (poa, servant, &ev);

	if (! NO_CORBA_EXCEPTION (ev) ) {
		g_warning ("Could not instantiate EazelProxy_AuthnCallback corba object");
		g_free (servant);
		servant = NULL;
		ret = CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);

	return ret;
}

void
ammonite_auth_callback_wrapper_free (PortableServer_POA poa, EazelProxy_AuthnCallback object)
{
	CORBA_Environment ev;
	PortableServer_ObjectId *object_id;
	impl_POA_EazelProxy_AuthnCallback * servant;

	CORBA_exception_init (&ev);

	servant = PortableServer_POA_reference_to_servant (poa, object, &ev);

	object_id = PortableServer_POA_servant_to_id (poa, servant, &ev);

	PortableServer_POA_deactivate_object (poa, object_id, &ev);

	CORBA_free (object_id);

	POA_EazelProxy_AuthnCallback__fini (servant, &ev);
	g_free (servant);

	CORBA_exception_free (&ev);
}


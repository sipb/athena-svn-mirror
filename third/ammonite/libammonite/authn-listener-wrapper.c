
/* $Id: authn-listener-wrapper.c,v 1.1.1.1 2001-01-16 15:25:59 ghudson Exp $
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
impl_EazelProxy_UserListener_user_authenticated (
	PortableServer_Servant servant,
	const EazelProxy_User * user,
	CORBA_Environment *ev
);
static void
impl_EazelProxy_UserListener_authenticated_no_longer (
	PortableServer_Servant servant, 
	const EazelProxy_User * user, 
	const EazelProxy_AuthnFailInfo *info, 
	CORBA_Environment *ev
);
static void
impl_EazelProxy_UserListener_user_logout (
	PortableServer_Servant servant, 
	const EazelProxy_User * user, 
	CORBA_Environment *ev
);

typedef struct {
	POA_EazelProxy_UserListener 	poa;
	AmmoniteUserListenerWrapperFuncs funcs;
	gpointer user_data;
} impl_POA_EazelProxy_UserListener;

/*******************************************************************
 * Module Globals
 *******************************************************************/

static PortableServer_ServantBase__epv base_epv = { NULL, NULL, NULL };
static POA_EazelProxy_UserListener__epv userlistener_epv = {
	NULL,
	impl_EazelProxy_UserListener_user_authenticated,
	impl_EazelProxy_UserListener_authenticated_no_longer,
	impl_EazelProxy_UserListener_user_logout
};

static POA_EazelProxy_UserListener__vepv userlistener_vepv = {
	&base_epv, &userlistener_epv
};

/*******************************************************************
 * EazelProxy_UserListener impl
 *******************************************************************/


static void
impl_EazelProxy_UserListener_user_authenticated (
	PortableServer_Servant servant,
	const EazelProxy_User * user,
	CORBA_Environment *ev
) {
	impl_POA_EazelProxy_UserListener *my_servant;
	my_servant =  (impl_POA_EazelProxy_UserListener *) servant;

	if (my_servant->funcs.user_authenticated) {
		my_servant->funcs.user_authenticated (user, my_servant->user_data, ev);
	}
}

static void
impl_EazelProxy_UserListener_authenticated_no_longer (
	PortableServer_Servant servant, 
	const EazelProxy_User * user, 
	const EazelProxy_AuthnFailInfo *info, 
	CORBA_Environment *ev
) {
	impl_POA_EazelProxy_UserListener *my_servant;
	my_servant =  (impl_POA_EazelProxy_UserListener *) servant;

	if (my_servant->funcs.user_authenticated_no_longer) {
		my_servant->funcs.user_authenticated_no_longer (user, info, my_servant->user_data, ev);
	}
}

static void
impl_EazelProxy_UserListener_user_logout (
	PortableServer_Servant servant, 
	const EazelProxy_User * user, 
	CORBA_Environment *ev
) {
	impl_POA_EazelProxy_UserListener *my_servant;
	my_servant =  (impl_POA_EazelProxy_UserListener *) servant;

	if (my_servant->funcs.user_logout) {
		my_servant->funcs.user_logout (user, my_servant->user_data, ev);
	}
}

EazelProxy_UserListener
ammonite_user_listener_wrapper_new (
	PortableServer_POA poa, 
	const AmmoniteUserListenerWrapperFuncs *funcs,
	gpointer user_data
) {

	CORBA_Environment ev;
	impl_POA_EazelProxy_UserListener *servant;
	EazelProxy_UserListener ret;

	g_return_val_if_fail ( NULL != funcs, CORBA_OBJECT_NIL);	
	
	CORBA_exception_init (&ev);

	servant = g_new0 (impl_POA_EazelProxy_UserListener, 1);

	servant->poa.vepv = &userlistener_vepv;

	memcpy ( &(servant->funcs), funcs, sizeof(*funcs));
	servant->user_data = user_data;

	POA_EazelProxy_UserListener__init (servant, &ev);

	CORBA_free (PortableServer_POA_activate_object (poa, servant, &ev));

	ret = (EazelProxy_UserListener)PortableServer_POA_servant_to_reference (poa, servant, &ev);

	if (! NO_CORBA_EXCEPTION (ev) ) {
		g_warning ("Could not instantiate EazelProxy_UserListener corba object");
		g_free (servant);
		servant = NULL;
		ret = CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);

	return ret;
}

void
ammonite_user_listener_wrapper_free (PortableServer_POA poa, EazelProxy_UserListener object)
{
	CORBA_Environment ev;
	PortableServer_ObjectId *object_id;
	impl_POA_EazelProxy_UserListener * servant;

	CORBA_exception_init (&ev);

	servant = PortableServer_POA_reference_to_servant (poa, object, &ev);

	object_id = PortableServer_POA_servant_to_id (poa, servant, &ev);

	PortableServer_POA_deactivate_object (poa, object_id, &ev);

	CORBA_free (object_id);

	POA_EazelProxy_UserListener__fini (servant, &ev);
	g_free (servant);

	CORBA_exception_free (&ev);
}


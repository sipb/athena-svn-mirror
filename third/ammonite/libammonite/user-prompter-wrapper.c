
/* $Id: user-prompter-wrapper.c,v 1.1.1.1 2001-01-16 15:26:00 ghudson Exp $
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

static CORBA_boolean
impl_EazelProxy_UserPrompter_prompt_authenticate (
	PortableServer_Servant servant,
	const EazelProxy_User * user,
	const EazelProxy_AuthnPromptKind kind,
	EazelProxy_AuthnInfo **authninfo,
	CORBA_Environment *ev
);

typedef struct {
	POA_EazelProxy_UserPrompter 	poa;
	AmmoniteUserPrompterWrapperFuncs funcs;
	gpointer user_data;
} impl_POA_EazelProxy_UserPrompter;


/*******************************************************************
 * Module Globals
 *******************************************************************/

static PortableServer_ServantBase__epv base_epv = { NULL, NULL, NULL };
static POA_EazelProxy_UserPrompter__epv userprompter_epv = {
	NULL,
	impl_EazelProxy_UserPrompter_prompt_authenticate
};

static POA_EazelProxy_UserPrompter__vepv userprompter_vepv = {
	&base_epv, &userprompter_epv
};

/*******************************************************************
 * EazelProxy_UserPrompter impl
 *******************************************************************/

static CORBA_boolean
impl_EazelProxy_UserPrompter_prompt_authenticate (
	PortableServer_Servant servant,
	const EazelProxy_User * user,
	const EazelProxy_AuthnPromptKind kind,
	EazelProxy_AuthnInfo **authninfo,
	CORBA_Environment *ev
) {
	impl_POA_EazelProxy_UserPrompter *my_servant;
	my_servant =  (impl_POA_EazelProxy_UserPrompter *) servant;

	if (my_servant->funcs.prompt_authenticate) {
		return my_servant->funcs.prompt_authenticate (user, kind, authninfo, my_servant->user_data, ev);
	}

	return FALSE;
}


EazelProxy_UserPrompter
ammonite_userprompter_wrapper_new (
	PortableServer_POA poa, 
	const AmmoniteUserPrompterWrapperFuncs *funcs,
	gpointer user_data
) {

	CORBA_Environment ev;
	impl_POA_EazelProxy_UserPrompter *servant;
	EazelProxy_UserPrompter ret;

	g_return_val_if_fail ( NULL != funcs, CORBA_OBJECT_NIL);	
	
	CORBA_exception_init (&ev);

	servant = g_new0 (impl_POA_EazelProxy_UserPrompter, 1);

	servant->poa.vepv = &userprompter_vepv;

	memcpy ( &(servant->funcs), funcs, sizeof(*funcs));
	servant->user_data = user_data;

	POA_EazelProxy_UserPrompter__init (servant, &ev);

	CORBA_free (PortableServer_POA_activate_object (poa, servant, &ev));

	ret = (EazelProxy_UserPrompter)PortableServer_POA_servant_to_reference (poa, servant, &ev);

	if (! NO_CORBA_EXCEPTION (ev) ) {
		g_warning ("Could not instantiate EazelProxy_UserPrompter corba object");
		g_free (servant);
		servant = NULL;
		ret = CORBA_OBJECT_NIL;
	}

	CORBA_exception_free (&ev);

	return ret;
}

void
ammonite_userprompter_wrapper_free (PortableServer_POA poa, EazelProxy_UserPrompter object)
{
	CORBA_Environment ev;
	PortableServer_ObjectId *object_id;
	impl_POA_EazelProxy_UserPrompter * servant;

	CORBA_exception_init (&ev);

	servant = PortableServer_POA_reference_to_servant (poa, object, &ev);

	object_id = PortableServer_POA_servant_to_id (poa, servant, &ev);

	PortableServer_POA_deactivate_object (poa, object_id, &ev);

	CORBA_free (object_id);

	POA_EazelProxy_UserPrompter__fini (servant, &ev);
	g_free (servant);

	CORBA_exception_free (&ev);
}


/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 8 -*- */
/*
 *  liboaf: A library for accessing oafd in a nice way.
 *
 *  Copyright (C) 1999, 2000 Red Hat, Inc.
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
 *  Author: Elliot Lee <sopwith@redhat.com>
 *
 */
/* The  folowing API is not intended for application use.
   It is intended only for people who want to extend OAF bootstraping system.
   I have no idea why we have all this tralala but Eliot knows and he _tried_
   to explain it in docs/oaf-regloc.txt
   If you can figure out this whole mess, please, mail me: <mathieu@eazel.com>
   I feel dumb.
*/

#ifndef OAF_REGISTRATION_H
#define OAF_REGISTRATION_H

typedef struct {
	const char *name;
	const char *session_name;
	const char *username;
        const char *hostname;
        const char *domain;
} OAFRegistrationCategory;

typedef struct _OAFRegistrationLocation OAFRegistrationLocation;
struct _OAFRegistrationLocation {
	void (*lock) (const OAFRegistrationLocation * regloc,
		      gpointer user_data);
	void (*unlock) (const OAFRegistrationLocation * regloc,
			gpointer user_data);
	char *(*check) (const OAFRegistrationLocation * regloc,
			const OAFRegistrationCategory * regcat,
			int *ret_distance, gpointer user_data);

	void (*register_new) (const OAFRegistrationLocation * regloc,
			      const char *ior,
			      const OAFRegistrationCategory * regcat,
			      gpointer user_data);
	void (*unregister) (const OAFRegistrationLocation * regloc,
			    const char *ior,
			    const OAFRegistrationCategory * regcat,
			    gpointer user_data);
};

typedef CORBA_Object (*OAFServiceActivator) (const OAFRegistrationCategory *
					     regcat, const char **cmd,
					     int ior_fd,
					     CORBA_Environment * ev);





void oaf_registration_location_add (const OAFRegistrationLocation * regloc,
				    int priority, gpointer user_data);

CORBA_Object oaf_registration_check (const OAFRegistrationCategory * regcat,
				     CORBA_Environment * ev);
void oaf_registration_set (const OAFRegistrationCategory * regcat,
			   CORBA_Object obj, CORBA_Environment * ev);
void oaf_registration_unset (const OAFRegistrationCategory * regcat,
			     CORBA_Object obj, CORBA_Environment * ev);



/* Do not release() the returned value */
CORBA_Object oaf_service_get (const OAFRegistrationCategory * regcat);

void oaf_registration_activator_add (OAFServiceActivator act_func,
				     int priority);


#endif /* OAF_REGISTRATION_H */

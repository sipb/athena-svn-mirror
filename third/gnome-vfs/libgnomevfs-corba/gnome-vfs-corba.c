/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-corba.c - CORBA stuff for the GNOME Virtual File System.

   Copyright (C) 1999 Free Software Foundation

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Ettore Perazzoli <ettore@comm2000.it> */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <orb/orbit.h>

#include <liboaf/liboaf.h>

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"

#include "gnome-vfs-corba.h"


static gboolean corba_already_initialized = FALSE;
G_LOCK_DEFINE_STATIC (corba_already_initialized);

CORBA_ORB gnome_vfs_orb;
PortableServer_POA gnome_vfs_poa;
PortableServer_POAManager gnome_vfs_poa_manager;



static CORBA_ORB
gnome_vfs_get_orb ()
{
	return oaf_orb_get ();
}


gboolean
gnome_vfs_corba_init (gboolean deps_init)
{
	CORBA_Environment ev;

	G_LOCK (corba_already_initialized);

	if (corba_already_initialized) {
		G_UNLOCK (corba_already_initialized);
		return TRUE;
	}

	if (! gnome_vfs_method_init ())
		return FALSE;

	if(deps_init && !gnome_vfs_get_orb ()) {
		char *argv[] = {"fake", NULL};
		int argc = 1;

		oaf_init (argc, argv);
	}

	gnome_vfs_orb = gnome_vfs_get_orb ();
	if (!gnome_vfs_orb) {
		/* FIXME bugzilla.eazel.com 1211 */
		g_warning ("GNOME CORBA support was not initialized.");
		return FALSE;
	}

	CORBA_exception_init (&ev);

	gnome_vfs_poa = (PortableServer_POA)
		CORBA_ORB_resolve_initial_references (gnome_vfs_orb,
						      "RootPOA", &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("Cannot find RootPOA.");
		CORBA_exception_free (&ev);
		return FALSE;
	}

	gnome_vfs_poa_manager = PortableServer_POA__get_the_POAManager
		(gnome_vfs_poa, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("Cannot get the POA manager.");
		CORBA_exception_free (&ev);
		return FALSE;
	}

	PortableServer_POAManager_activate (gnome_vfs_poa_manager, &ev);
	if (ev._major != CORBA_NO_EXCEPTION) {
		g_warning ("Cannot activate the POA manager.");
		CORBA_exception_free (&ev);
		return FALSE;
	}

	CORBA_exception_free (&ev);

	corba_already_initialized = TRUE;
	G_UNLOCK (corba_already_initialized);

	return TRUE;
}



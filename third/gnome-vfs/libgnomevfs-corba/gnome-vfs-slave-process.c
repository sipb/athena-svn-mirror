/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-slave-process.c - Communication with the CORBA slave process for
   asynchronous operation in the GNOME Virtual File System.

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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
     
#include <orb/orbit.h>

#include "gnome-vfs.h"
#include "gnome-vfs-private.h"

#include "gnome-vfs-corba.h"
#include "gnome-vfs-slave.h"
#include "gnome-vfs-slave-launch.h"

#include "gnome-vfs-slave-process.h"


GnomeVFSSlaveProcess *
gnome_vfs_slave_process_new (void)
{
	GnomeVFSSlaveProcess *new;
	GnomeVFSProcess *process;

	if (! gnome_vfs_corba_init (TRUE))
		return NULL;

	new = g_new (GnomeVFSSlaveProcess, 1);

	new->notify_objref = CORBA_OBJECT_NIL;
	new->notify_servant = NULL;

	new->request_objref = CORBA_OBJECT_NIL;
	new->file_handle_objref = CORBA_OBJECT_NIL;

	new->operation_in_progress = GNOME_VFS_ASYNC_OP_NONE;

	new->callback = NULL;
	new->callback_data = NULL;

	new->context = gnome_vfs_context_new();
	
	CORBA_exception_init (&new->ev);

	if (! gnome_vfs_slave_notify_create (new)) {
		gnome_vfs_slave_process_destroy (new);
		return NULL;
	}

	process = gnome_vfs_slave_launch (new->notify_objref,
					  &new->request_objref);
	if (process == CORBA_OBJECT_NIL) {
		gnome_vfs_slave_process_destroy (new);
		return NULL;
	}
	
	/* We don't free process or keep a reference, but this is _not_
	   a memory leak, it will get gnome_vfs_process_free()d when
	   the associated child process dies. */
	
	return new;
}

#if 0				
/* FIXME bugzilla.eazel.com 1124: */
void
gnome_vfs_slave_process_reset (GnomeVFSSlaveProcess *slave,
			       GnomeVFSSlaveProcessResetCallback callback,
			       gpointer callback_data)
{
	g_return_if_fail (slave != NULL);
	g_return_if_fail (callback != NULL);

	/*  free_op_info (slave); ? */

	slave->operation_in_progress = GNOME_VFS_ASYNC_OP_RESET;
	slave->callback = callback;
	slave->callback_data = callback_data;

	if (slave->request_objref != CORBA_OBJECT_NIL) {
		GNOME_VFS_Slave_Request_reset (slave->request_objref,
					       &slave->ev);
		if (slave->ev._major != CORBA_NO_EXCEPTION) {
			CORBA_char *ior;
			CORBA_Environment ev;

			CORBA_exception_init (&ev);
			ior = CORBA_ORB_object_to_string
				(gnome_vfs_orb, slave->request_objref, &ev);

			if (slave->ev._major != CORBA_NO_EXCEPTION) 
				g_warning (_("Cannot reset GNOME::VFS::Slave %s -- exception %s"),
					   ior, CORBA_exception_id (&slave->ev));
			else
				g_warning (_("Cannot reset GNOME::VFS::Slave (IOR unknown) -- exception %s"),
					   CORBA_exception_id (&slave->ev));

			CORBA_exception_free (&ev);
			CORBA_free (ior);
		}
	}
}
#endif

void
gnome_vfs_slave_process_destroy (GnomeVFSSlaveProcess *slave)
{
	g_return_if_fail (slave != NULL);

	/*  free_op_info (slave); */

	slave->operation_in_progress = GNOME_VFS_ASYNC_OP_NONE;

	if (slave->request_objref != CORBA_OBJECT_NIL) {
		GNOME_VFS_Slave_Request_die (slave->request_objref,
					     &slave->ev);
		if (slave->ev._major != CORBA_NO_EXCEPTION) {
			CORBA_char *ior;
			CORBA_Environment ev;

			CORBA_exception_init (&ev);
			ior = CORBA_ORB_object_to_string
				(gnome_vfs_orb, slave->request_objref, &ev);

			if (slave->ev._major != CORBA_NO_EXCEPTION) 
				g_warning (_("Cannot kill GNOME::VFS::Slave %s -- exception %s"),
					   ior, CORBA_exception_id (&slave->ev));
			else
				g_warning (_("Cannot kill GNOME::VFS::Slave (IOR unknown) -- exception %s"),
					   CORBA_exception_id (&slave->ev));

			CORBA_exception_free (&ev);
			CORBA_free (ior);
		} 
	}
}


gboolean
gnome_vfs_slave_process_busy (GnomeVFSSlaveProcess *slave)
{
	g_return_val_if_fail (slave != NULL, FALSE);

	if (slave->file_handle_objref != CORBA_OBJECT_NIL
	    || slave->operation_in_progress != GNOME_VFS_ASYNC_OP_NONE)
		return TRUE;

	return FALSE;
}

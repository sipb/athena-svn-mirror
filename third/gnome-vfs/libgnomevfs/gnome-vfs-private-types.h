/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-private-types.h - Declaration of private types for the
   GNOME Virtual File System.

   Copyright (C) 1999 Free Software Foundation

   The Gnome Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Ettore Perazzoli <ettore@gnu.org> */

#ifndef GNOME_VFS_PRIVATE_TYPES_H
#define GNOME_VFS_PRIVATE_TYPES_H

#include <glib.h>


typedef struct GnomeVFSProgressCallbackState {

	/* xfer state */
	GnomeVFSXferProgressInfo *progress_info;	

	/* Callback called for every xfer operation. For async calls called 
	   in async xfer context. */
	GnomeVFSXferProgressCallback sync_callback;

	/* Callback called periodically every few hundred miliseconds
	   and whenever user interaction is needed. For async calls
	   called in the context of the async call caller. */
	GnomeVFSXferProgressCallback update_callback;

	/* User data passed to sync_callback. */
	gpointer user_data;

	/* Async job state passed to the update callback. */
	gpointer async_job_data;

	/* When will update_callback be called next. */
	gint64 next_update_callback_time;

	/* When will update_callback be called next. */
	gint64 next_text_update_callback_time;

	/* Period at which the update_callback will be called. */
	gint64 update_callback_period;
} GnomeVFSProgressCallbackState;


typedef struct GnomeVFSShellpatternFilter GnomeVFSShellpatternFilter;
typedef struct GnomeVFSRegexpFilter GnomeVFSRegexpFilter;

#endif /* _GNOME_VFS_PRIVATE_TYPES_H */

/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-context.h - context VFS modules can use to communicate with gnome-vfs proper

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

   Author: Havoc Pennington <hp@redhat.com> */

#ifndef GNOME_VFS_CONTEXT_H
#define GNOME_VFS_CONTEXT_H

#include "gnome-vfs.h"

GnomeVFSContext* gnome_vfs_context_new                   (void);
void             gnome_vfs_context_ref                   (GnomeVFSContext *ctx);
void             gnome_vfs_context_unref                 (GnomeVFSContext *ctx);

/* To be really thread-safe, these need to return objects with an increased
   refcount; however they don't, only one thread at a time
   can use GnomeVFSContext */
GnomeVFSMessageCallbacks*
                 gnome_vfs_context_get_message_callbacks (GnomeVFSContext *ctx);

GnomeVFSCancellation*
                 gnome_vfs_context_get_cancellation      (GnomeVFSContext *ctx);

/* returns NULL if no redirection occurred */
const gchar*     gnome_vfs_context_get_redirect_uri      (GnomeVFSContext *ctx);
void             gnome_vfs_context_set_redirect_uri      (GnomeVFSContext *ctx,
                                                          const gchar     *uri);

/* Convenience - both of these accept a NULL context object */
#define          gnome_vfs_context_check_cancellation(x) (gnome_vfs_cancellation_check((x) ? gnome_vfs_context_get_cancellation((x)) : NULL))

void             gnome_vfs_context_emit_message           (GnomeVFSContext *ctx,
                                                           const gchar* message);

#endif /* GNOME_VFS_CONTEXT_H */

/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gnome-vfs-context.c - context VFS modules can use to communicate with gnome-vfs proper

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

#include "gnome-vfs-context.h"
#include "gnome-vfs-cancellation.h"
#include "gnome-vfs-messages.h"

#include <stdio.h>

struct GnomeVFSContext {
        GnomeVFSCancellation *cancellation;
        GnomeVFSMessageCallbacks *callbacks;
        gchar* redirect_uri;
        guint refcount;
};

GnomeVFSContext*
gnome_vfs_context_new (void)
{
        GnomeVFSContext *ctx;

        ctx = g_new0(GnomeVFSContext, 1);

        ctx->cancellation = gnome_vfs_cancellation_new();
        ctx->callbacks = gnome_vfs_message_callbacks_new();
        ctx->redirect_uri = NULL;
        ctx->refcount = 1;
  
        return ctx;
}

void
gnome_vfs_context_ref (GnomeVFSContext *ctx)
{
        g_return_if_fail(ctx != NULL);
  
        ctx->refcount += 1;
}

void
gnome_vfs_context_unref (GnomeVFSContext *ctx)
{
        g_return_if_fail(ctx != NULL);
        g_return_if_fail(ctx->refcount > 0);
  
        if (ctx->refcount == 1) {
                gnome_vfs_cancellation_destroy(ctx->cancellation);
                gnome_vfs_message_callbacks_destroy(ctx->callbacks);
                if (ctx->redirect_uri)
                        g_free(ctx->redirect_uri);
          
                g_free(ctx);
        } else {
                ctx->refcount -= 1;
        }
}


GnomeVFSMessageCallbacks*
gnome_vfs_context_get_message_callbacks (GnomeVFSContext *ctx)
{
        g_return_val_if_fail(ctx != NULL, NULL);
        return ctx->callbacks;
}

GnomeVFSCancellation*
gnome_vfs_context_get_cancellation (GnomeVFSContext *ctx)
{
        g_return_val_if_fail(ctx != NULL, NULL);
        return ctx->cancellation;
}


const gchar*
gnome_vfs_context_get_redirect_uri      (GnomeVFSContext *ctx)
{
        g_return_val_if_fail(ctx != NULL, NULL);
        return ctx->redirect_uri;
}

void
gnome_vfs_context_set_redirect_uri      (GnomeVFSContext *ctx,
                                         const gchar     *uri)
{
        g_return_if_fail(ctx != NULL);
        
        if (ctx->redirect_uri)
                g_free(ctx->redirect_uri);
        
        ctx->redirect_uri = uri ? g_strdup(uri) : NULL;
}

void
gnome_vfs_context_emit_message           (GnomeVFSContext *ctx,
                                          const gchar* message)
{
        GnomeVFSMessageCallbacks *callbacks;
        
        if (ctx == NULL) {
                fprintf(stderr, "Debug: NULL context so not reporting status: %s\n", message);
                return;
        }
        
        callbacks = ctx->callbacks;
        
        if (callbacks)
                gnome_vfs_message_callbacks_emit (callbacks, message);
}

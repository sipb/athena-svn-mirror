/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */

/*
   eel-mime-extensions.c: MIME database manipulation
 
   Copyright (C) 2004 Novell, Inc.
 
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

   Authors: Dave Camp <dave@novell.com>
*/

#include <glib.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>

#ifndef EEL_MIME_EXTENSIONS_H
#define EEL_MIME_EXTENSIONS_H


GnomeVFSMimeApplication *eel_mime_add_application           (const char *mime_type,
							     const char *command_line,
							     const char *name,
							     gboolean    needs_terminal);
gboolean                 eel_mime_add_glob_type             (const char *mime_type,
							     const char *description,
							     const char *glob);
gboolean                 eel_mime_set_default_application   (const char *mime_type,
							     const char *id);
gboolean                 eel_mime_application_is_user_owned (const char *id);
void                     eel_mime_application_remove        (const char *id);
GnomeVFSMimeApplication *eel_mime_check_for_duplicates      (const char *mime_type,
							     const char *command_line);

#endif

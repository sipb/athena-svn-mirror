
/* GnomeMeeting -- A Video-Conferencing application
 * Copyright (C) 2000-2004 Damien Sandras
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 * GnomeMeeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */

/*
 *                         gtk-text-buffer-addon.h  -  description 
 *                         ------------------------------------
 *   begin                : Sat Nov 29 2003, but based on older code
 *   copyright            : (C) 2000-2004 by Julien Puydt
 *                                           Miguel Rodríguez,
 *                                           StÃ©phane Wirtel
 *                                           Kenneth Christiansen
 *   description          : Add-on functions for regex-based context menus
 *
 */


#include <gtk/gtk.h>
#include <string.h>


#ifndef __GTK_TEXT_BUF_ADD_H
#define __GTK_TEXT_BUF_ADD_H

G_BEGIN_DECLS

/**
 * gtk_text_buffer_insert_with_uris:
 * @buf: A pointer to a GtkTextBuffer
 * @iter: An iterator for the buffer
 * @text: A text string
 *
 * Inserts @text into the @buf, but with all regex shown
 * in a beautified way
 **/
void gtk_text_buffer_insert_with_regex (GtkTextBuffer *buf,
					GtkTextIter *iter,
					const gchar *text);
/**
 * Prototype of the type of functions that are able to display a regex-matched text
 *
 **/
typedef void (*RegexDisplayFunc)(GtkTextBuffer *,
				 GtkTextIter *,
				 GtkTextTag *,
				 const gchar *);

// below are two example implementations of that prototype: 

/**
 * gtk_text_buffer_insert_plain:
 * @buf: A pointer to a GtkTextBuffer
 * @iter: An iterator for the buffer
 * @tag: A pointer to a GtkTextTag
 * @text: A text string
 *
 * Inserts @text into the @buf, with the given tag, as simple text
 **/
void gtk_text_buffer_insert_plain (GtkTextBuffer *buf,
				   GtkTextIter *iter,
				   GtkTextTag *tag,
				   const gchar *text);

/**
 * gtk_text_buffer_insert_smiley:
 * @buf: A pointer to a GtkTextBuffer
 * @iter: An iterator for the buffer
 * @tag: A pointer to a GtkTextTag
 * @smile: A text string
 *
 * Inserts @smile into the @buf, with the given tag, as an image looked up in a table
 **/
void gtk_text_buffer_insert_smiley (GtkTextBuffer *buf,
				    GtkTextIter *iter,
				    GtkTextTag *tag,
				    const gchar *smile);

G_END_DECLS

#endif


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
 *                         gtk-text-tag-addon.h  -  description 
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
#include <regex.h>
#include "gtk-text-buffer-addon.h"

#ifndef __GTK_TEXT_TAG_ADD_H
#define __GTK_TEXT_TAG_ADD_H

G_BEGIN_DECLS

/**
 * gtk_text_tag_set_regex:
 * @tag: A pointer to a GtkTextTag
 * @regex: A pointer to a string, representing a regular expression
 *
 * This function associates a regular expression to the given text tag
 **/
gboolean gtk_text_tag_set_regex (GtkTextTag *tag,
				 const gchar *regex);

/**
 * gtk_text_tag_get_regex:
 * @tag: A pointer to a GtkTextTag
 *
 * This function returns the regular expression associated to the given text tag
 **/
regex_t *gtk_text_tag_get_regex (GtkTextTag *tag);

/**
 * gtk_text_tag_add_action_to_regex;
 * @tag: A pointer to a GtkTextTag
 * @action_name: A pointer to a string, giving the name under
 *                          which the action will appear in the context menu
 * @func: The function to call when the action is selected, receiving the string matched by the regex
 *
 * This function adds a menu entry to the context menu of the regex,
 * with @func being the associated callback. It enables the context menu on right click only when the first
 * action is added
 **/
void gtk_text_tag_add_action_to_regex (GtkTextTag *tag,
				       const gchar *action_name,
				       GtkSignalFunc func);

/**
 * gtk_text_tag_add_actions_to_regex:
 * @tag: A pointer to a GtkTextTag
 * 
 * This function adds a menu entry to the context menu of the regex,
 * with  the associated callback, for each pair of @action_name, @func
 * as for gtk_text_tag_add_action_to_regex; allows to add several actions in one go
 **/
void gtk_text_tag_add_actions_to_regex (GtkTextTag *tag,
					const gchar *first_action_name,
					...);

/**
 * gtk_text_tag_set_regex_display:
 * @tag: A pointer to a GtkTextTag
 * @func: A display function, as defined in gtk-text-buffer-addon.h
 *
 * This function sets which function is used to display the text matched by the regex
 * associated with the tag
 **/
void gtk_text_tag_set_regex_display (GtkTextTag *tag, RegexDisplayFunc func);

G_END_DECLS

#endif


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
 *                         gtk-text-tag-addon.c  -  description 
 *                         ------------------------------------
 *   begin                : Sat Nov 29 2003, but based on older code
 *   copyright            : (C) 2000-2004 by Julien Puydt
 *                                           Miguel Rodríguez,
 *                                           StÃ©phane Wirtel
 *                                           Kenneth Christiansen
 *   description          : Add-on functions for regex-based context menus
 *
 */

#include "gtk-text-tag-addon.h"

/*
 this is the function that prompts the popup menu when a regex-enabled gtk text tag is right-clicked
*/
static gboolean
regex_event (GtkTextTag *texttag, GObject *arg1, GdkEvent *event,
	     GtkTextIter *iter, gpointer user_data)
{
  if (event->type == GDK_BUTTON_PRESS && event->button.button == 3) {
    gchar *txt;

    GtkTextIter *start = gtk_text_iter_copy (iter);
    GtkTextIter *end = gtk_text_iter_copy (iter);
    
    gtk_text_iter_backward_to_tag_toggle (start, texttag);
    gtk_text_iter_forward_to_tag_toggle (end, texttag);
    
    txt = gtk_text_buffer_get_slice (gtk_text_iter_get_buffer (iter),
                                     start, end, FALSE);
    
    g_object_set_data_full (G_OBJECT (user_data), "clicked-regex",
                            txt, g_free);
    
    gtk_menu_popup ((GtkMenu *) user_data, NULL, NULL, NULL, NULL,
                    event->button.button, event->button.time);
    
    gtk_text_iter_free (start);
    gtk_text_iter_free (end);
    
    return TRUE;
  }
  
  return FALSE;
}

/* this function is called whenever an action is activated in a regex context menu; it finds which matching
 text  was clicked, which function is to be called, then calls it with the text
*/
static void
regex_menu_callback (GtkMenuItem *menu_item, gpointer data)
{
  void (*func)(gchar *);
  gchar *txt;

  func = g_object_get_data (G_OBJECT(menu_item), "regex-callback");
  
  txt = g_object_get_data (G_OBJECT(data), "clicked-regex");

  g_assert (func != NULL);
  (*func)(txt);
}

/**
 * gtk_text_tag_set_regex:
 * @tag: A pointer to a GtkTextTag
 * @regex: A pointer to a string, representing a regular expression
 *
 * This function associates a regular expression to the given text tag
 **/
gboolean
gtk_text_tag_set_regex (GtkTextTag *tag,
			const gchar *regex_string)
{
  regex_t *regex = (regex_t *)g_malloc (sizeof(regex_t));
  if (regex == NULL)
    return FALSE;

  if (regcomp (regex, regex_string, REG_EXTENDED) != 0) {
    regfree (regex);
    return FALSE;
  }
  
  g_object_set_data_full (G_OBJECT(tag), "regex", regex,
			  (GDestroyNotify) regfree);
  
  return TRUE;
}

/**
 * gtk_text_tag_get_regex:
 * @tag: A pointer to a GtkTextTag
 *
 * This function returns the regular expression associated to the given text tag
 **/
regex_t *
gtk_text_tag_get_regex (GtkTextTag *tag)
{
  return g_object_get_data (G_OBJECT(tag), "regex");
}

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
void
gtk_text_tag_add_action_to_regex (GtkTextTag *tag,
				       const gchar *action_name,
				       GtkSignalFunc func)
{
  GtkWidget *popup, *menu_item;

  g_return_if_fail (action_name != NULL && func != NULL);
  g_return_if_fail (g_object_get_data (G_OBJECT(tag), "regex") != NULL);

  popup = g_object_get_data (G_OBJECT(tag), "regex-popup");

  if (popup == NULL) {
    popup = gtk_menu_new ();
    g_object_set_data_full (G_OBJECT(tag), "regex-popup", popup,
			    (GtkDestroyNotify)gtk_widget_destroy);
    g_signal_connect (tag, "event", (GtkSignalFunc) regex_event, popup);
  }
  
  menu_item = gtk_menu_item_new_with_label (action_name);
  gtk_widget_show (menu_item);
  gtk_menu_shell_append (GTK_MENU_SHELL(popup), menu_item);
  g_signal_connect_after (GTK_OBJECT(menu_item), "activate",
			  (GCallback) regex_menu_callback, popup);
  g_object_set_data (G_OBJECT(menu_item), "regex-callback", func);
}

/**
 * gtk_text_tag_add_actions_to_regex:
 * @tag: A pointer to a GtkTextTag
 * 
 * This function adds a menu entry to the context menu of the regex,
 * with  the associated callback, for each pair of @action_name, @func
 * as for gtk_text_tag_add_action_to_regex; allows to add several actions in one go
 **/
void
gtk_text_tag_add_actions_to_regex (GtkTextTag *tag,
					const char *first_action_name,
					...)
{
  const gchar *action_name;
  GtkSignalFunc func;
  va_list args;

  if (g_object_get_data (G_OBJECT(tag), "regex") == NULL) {
    g_warning ("Missing regex for action.");
    return;
  }

  va_start (args, first_action_name);

  action_name = first_action_name;
  while (action_name != NULL) {
      func = va_arg (args, GtkSignalFunc);
      if (func == NULL)
	break;
      gtk_text_tag_add_action_to_regex (tag, action_name, func);
      action_name = va_arg (args, gchar *);
  }
  va_end (args);  
}

/**
 * gtk_text_tag_set_regex_display:
 * @tag: A pointer to a GtkTextTag
 * @func: A display function, as defined in gtk-text-buffer-addon.h
 *
 * This function sets which function is used to display the text matched by the regex
 * associated with the tag
 **/
void
gtk_text_tag_set_regex_display (GtkTextTag *tag, RegexDisplayFunc func)
{
  g_object_set_data (G_OBJECT(tag), "regex-display", func);
}


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
 *                         gtk-text-view-addon.c  -  description 
 *                         ------------------------------------
 *   begin                : Sat Nov 29 2003, but based on older code
 *   copyright            : (C) 2000-2004 by Julien Puydt
 *                                           Miguel Rodríguez,
 *                                           StÃ©phane Wirtel
 *                                           Kenneth Christiansen
 *   description          : Add-on functions for regex-based context menus
 *
 */

#include "gtk-text-view-addon.h"

/* this function highlights regex-enabled gtk text tag when the mouse hovers over them */
static gboolean
regex_highlightning_callback (GtkWidget *widget,
			      GdkEventMotion *event, gpointer user_data)
{
  GSList *tag_list, *tmp_list;
  GtkTextIter iter;
  gboolean problem_flag = FALSE; // Damien doesn't like goto's...
  gboolean found = FALSE;
  gint x = 0;
  gint y = 0;
  GdkModifierType state; 
  GtkTextTag *tag = NULL;
  gdk_window_get_pointer (event->window, &x, &y, &state);

  gtk_text_view_get_iter_at_location ((GtkTextView *)widget, &iter, x, y);

  tag_list = gtk_text_iter_get_tags (&iter);
  tmp_list = tag_list;
  if (g_slist_length (tag_list) == 0)
    problem_flag = TRUE;
 
  if (problem_flag == FALSE) {
    while (tmp_list != NULL) {
      tag = (GtkTextTag *)tmp_list->data;
      if (g_object_get_data (G_OBJECT(tag), "regex") != NULL)
        {
	  found = TRUE;
	  break;
        }
      tmp_list = tmp_list->next;
    }
  }
  g_slist_free (tag_list);
  if (found == FALSE)
    problem_flag = TRUE;
  
  if (problem_flag == FALSE) {
    GdkCursor *cursor = gdk_cursor_new (GDK_HAND2);
    gdk_window_set_cursor (event->window, cursor);
    gdk_cursor_unref (cursor);
    return TRUE;
  }

  {
    gdk_window_set_cursor (event->window, NULL);
    return TRUE;
  }
}

/**
 * gtk_text_view_new_with_regex:
 * 
 * Creates a new gtk text view, but with the added functionality that regex-enabled text tags in it
 * gain highlighting when the mouse hovers over them
 **/
GtkWidget *
gtk_text_view_new_with_regex (void)
{
  GtkWidget *textview = gtk_text_view_new ();
  g_signal_connect (textview, "motion-notify-event",
		    (GtkSignalFunc)regex_highlightning_callback, NULL);
  return textview;
}    

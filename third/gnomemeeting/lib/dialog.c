/*  dialog.c
 *
 *  GnomeMeeting -- A Video-Conferencing application
 *  Copyright (C) 2000-2002 Damien Sandras
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 *  Authors: Damien Sandras <dsandras@seconix.com>
 *           Jorn Baayen <jorn@nl.linux.com>
 *           Kenneth Christiansen <kenneth@gnu.org>
 */

/*
 *                         dialog.c  -  description
 *                         ------------------------
 *   begin                : Mon Jun 17 2002
 *   copyright            : (C) 2000-2002 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          to create dialogs for GnomeMeeting.
 */

#include "../config.h"

#include <gtk/gtk.h>
#include <glib.h>

#ifndef DISABLE_GNOME
#include <gnome.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "dialog.h"

#ifdef WIN32
#define vsnprintf _vsnprintf
#endif

#ifndef _
#ifdef DISABLE_GNOME
#include <libintl.h>
#define _(x) gettext(x)
#ifdef gettext_noop
#define N_(String) gettext_noop (String)
#else
#define N_(String) (String)
#endif
#endif
#endif


static GtkWidget *
gnomemeeting_dialog (GtkWindow *parent,
		     const char *primary_text,
		     const char *format, 
		     va_list args,
		     GtkMessageType type);


GtkWidget *
gnomemeeting_error_dialog (GtkWindow *parent,
			   const char *primary_text,
			   const char *format,
			   ...)
{
  GtkWidget *dialog = NULL;
  va_list args;
  
  va_start (args, format);

  dialog =
    gnomemeeting_dialog (parent, primary_text, format, args,
			 GTK_MESSAGE_ERROR);
  
  va_end (args);

  return dialog;
}


GtkWidget *
gnomemeeting_warning_dialog (GtkWindow *parent,
			     const char *primary_text,
			     const char *format,
			     ...)
{
  GtkWidget *dialog = NULL;
  va_list args;
  
  va_start (args, format);

  dialog =
    gnomemeeting_dialog (parent, primary_text, format, args,
			 GTK_MESSAGE_WARNING);
  
  va_end (args);

  return dialog;
}


GtkWidget *
gnomemeeting_message_dialog (GtkWindow *parent,
			     const char *primary_text,
			     const char *format,
			     ...)
{
  GtkWidget *dialog = NULL;
  va_list args;
  
  va_start (args, format);
  
  dialog =
    gnomemeeting_dialog (parent, primary_text, format, args, GTK_MESSAGE_INFO);
  
  va_end (args);

  return dialog;
}


static void 
warning_dialog_destroyed_cb (GtkWidget *w,
			     gint i,
			     gpointer data)
{
  GList *children = NULL;
  
  children = gtk_container_get_children (GTK_CONTAINER (GTK_DIALOG (w)->vbox));

  g_return_if_fail (data != NULL);

  while (children) {
    
    if (GTK_IS_TOGGLE_BUTTON (children->data)) 
      g_object_set_data (G_OBJECT (gtk_window_get_transient_for (GTK_WINDOW (w))), (const char *) data, GINT_TO_POINTER (GTK_TOGGLE_BUTTON (children->data)->active));
  
    children = g_list_next (children);
  }

  gtk_widget_destroy (GTK_WIDGET (w));
}


GtkWidget *
gnomemeeting_warning_dialog_on_widget (GtkWindow *parent, 
                                       const char *key,
				       const char *primary_text,
                                       const char *format,
				       ...)
{
  va_list args;
  
  GtkWidget *button = NULL;
  GtkWidget *dialog = NULL;

  char buffer[1025];

  gchar *prim_text = NULL;
  gchar *dialog_text = NULL;

  gboolean do_not_show = FALSE;
  
  va_start (args, format);
  
  g_return_val_if_fail (parent != NULL, NULL);
  g_return_val_if_fail (key != NULL, NULL);

     
  /* if not set, do_not_show will get the value of 0 */
  do_not_show = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (parent), key));
  
  if (do_not_show)
    /* doesn't show warning dialog as state is 'hide' */
    return NULL;
 
  button = 
    gtk_check_button_new_with_label (_("Do not show this dialog again"));
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), do_not_show);
  
  vsnprintf (buffer, 1024, format, args);

  prim_text =
    g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>",
		     primary_text);
  vsnprintf (buffer, 1024, format, args);
  
  dialog_text =
    g_strdup_printf ("%s\n\n%s", prim_text, buffer);

  dialog = gtk_message_dialog_new (parent, 
                                   0,
                                   GTK_MESSAGE_WARNING,
                                   GTK_BUTTONS_OK,
                                   "");
  gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);
  gtk_window_present (GTK_WINDOW (dialog));
  
  gtk_window_set_title (GTK_WINDOW (dialog), "");
  gtk_label_set_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (dialog)->label),
			dialog_text);

  gtk_container_add (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), 
                     button);
  
  gtk_widget_show_all (dialog);

  g_signal_connect_data (GTK_OBJECT (dialog), "response",
			 GTK_SIGNAL_FUNC (warning_dialog_destroyed_cb),
			 (gpointer) g_strdup (key),
			 (GClosureNotify) g_free,
			 (GConnectFlags) 0);
  
  va_end (args);

  g_free (prim_text);
  g_free (dialog_text);

  return dialog;
}


/**
 * gnomemeeting_dialog
 *
 * @parent: The parent window of the dialog.
 * @format: a char * including printf formats
 * @args  : va_list that the @format char * uses.
 * @type  : specifies the kind of GtkMessageType dialogs to use. 
 *
 * Creates and runs a dialog and destroys it afterward. 
 **/
static GtkWidget *
gnomemeeting_dialog (GtkWindow *parent,
		     const char *prim_text,
                     const char *format, 
                     va_list args, 
                     GtkMessageType type)
{
  GtkWidget *dialog;
  gchar *primary_text = NULL;
  gchar *dialog_text = NULL;
  char buffer [1025];

  primary_text =
    g_strdup_printf ("<span weight=\"bold\" size=\"larger\">%s</span>",
		     prim_text);
  
  vsnprintf (buffer, 1024, format, args);

  dialog_text =
    g_strdup_printf ("%s\n\n%s", primary_text, buffer);
  
  dialog =
    gtk_message_dialog_new (parent, 
                            GTK_DIALOG_MODAL, 
                            type,
			    GTK_BUTTONS_OK, "");

  gtk_window_set_title (GTK_WINDOW (dialog), "");
  gtk_label_set_markup (GTK_LABEL (GTK_MESSAGE_DIALOG (dialog)->label),
			dialog_text);
  
  g_signal_connect_swapped (GTK_OBJECT (dialog), "response",
                            G_CALLBACK (gtk_widget_destroy),
                            GTK_OBJECT (dialog));
  
  gtk_widget_show_all (dialog);

  g_free (dialog_text);
  g_free (primary_text);

  return dialog;
}

/* session-properties.c - Edit session properties.

   Copyright 1999 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA. 

   Authors: Felix Bellaby */

#ifdef HAVE_CONFIG_H
#   include <config.h>
#endif

#include <gnome.h>
#include "capplet-widget.h"
#include "gsm-client-list.h"
#include "gsm-protocol.h"
#include "session-properties.h"

/* time waited before restarting the last session saved/selected */
static gint chooser_delay = 10000;

/* chooser widgets */
static GtkObject *protocol;
static GtkWidget *client_list;

static GtkWidget *dialog;
static GtkWidget *session_list;
static GtkWidget *gnome_foot;
static GtkWidget *left_scrolled;
static GtkWidget *right_scrolled;

/* session list callbacks */
static void sess_select_row_cb (GtkWidget *widget, gint row);
static void last_session_cb (GtkWidget *w, gchar* name);
static void saved_sessions_cb (GtkWidget *w, GSList* session_names);

/* other widget callback prototypes */
static gint start_timeout (gpointer data);
static void start_cb (GtkWidget *widget);
static void cancel_cb (GtkWidget *widget);
static void right_initialized_cb (GtkWidget *widget);

void
chooser_build (void)
{
  GtkWidget *paned;
  gchar* foot_file = gnome_pixmap_file ("gnome-logo-large.png");
  gchar* title = _("Session");
  GtkRequisition req;

  /* gnome foot */
  gnome_foot = gnome_pixmap_new_from_file (foot_file);
  gtk_widget_size_request (gnome_foot, &req);

  /* session list */
  session_list = gtk_clist_new_with_titles(1, &title);
  gtk_widget_set_usize (session_list, req.width + GNOME_PAD, req.height);
  gtk_signal_connect (GTK_OBJECT (session_list), "select_row",
		      sess_select_row_cb, NULL);

  /* left scrolled window */
  left_scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (left_scrolled),
				  GTK_POLICY_NEVER, GTK_POLICY_NEVER);
  gtk_container_add (GTK_CONTAINER (left_scrolled), session_list);

  /* client list */
  client_list = gsm_client_list_new ();

  /* right scrolled window */
  right_scrolled = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (right_scrolled),
				  GTK_POLICY_NEVER, GTK_POLICY_NEVER);
  gtk_container_add (GTK_CONTAINER (right_scrolled), client_list);

  /* paned window */
  paned = gtk_hpaned_new();
  gtk_paned_handle_size (GTK_PANED (paned), 10);
  gtk_paned_gutter_size (GTK_PANED (paned), 10);
  gtk_paned_add1 (GTK_PANED (paned), left_scrolled);
  gtk_paned_add2 (GTK_PANED (paned), right_scrolled);

  /* dialog */
  dialog = gnome_dialog_new (_("Session Chooser"), NULL);
  gtk_container_add (GTK_CONTAINER (GNOME_DIALOG (dialog)->vbox), paned);
  gnome_dialog_append_button_with_pixmap (GNOME_DIALOG (dialog),
					  _("Start Session"),
					  GNOME_STOCK_BUTTON_OK);
  gnome_dialog_append_button_with_pixmap (GNOME_DIALOG (dialog),
					  _("Cancel Login"),
					  GNOME_STOCK_BUTTON_CANCEL);
  gnome_dialog_set_default (GNOME_DIALOG (dialog), 0);
  gnome_dialog_button_connect (GNOME_DIALOG (dialog), 0,
			       GTK_SIGNAL_FUNC (start_cb), NULL);
  gnome_dialog_button_connect (GNOME_DIALOG (dialog), 1,
			       GTK_SIGNAL_FUNC (cancel_cb), NULL);

  gtk_signal_connect (GTK_OBJECT (dialog), "delete_event",
		      GTK_SIGNAL_FUNC (gtk_main_quit), NULL);

  gtk_signal_connect (GTK_OBJECT (protocol), "last_session", 
		      GTK_SIGNAL_FUNC (last_session_cb), NULL);
  gsm_protocol_get_last_session (GSM_PROTOCOL (protocol));

  gtk_signal_connect (GTK_OBJECT (protocol), "saved_sessions", 
		      GTK_SIGNAL_FUNC (saved_sessions_cb), NULL);
  gsm_protocol_get_saved_sessions (GSM_PROTOCOL (protocol));
}

/* SESSION LIST CALLBACKS */
static gint chooser_timeout = -1;

static void
start (void)
{
  GtkWidget *viewport;

  gtk_signal_connect(GTK_OBJECT(client_list), "started",
		     GTK_SIGNAL_FUNC (gtk_main_quit), NULL);
  gsm_client_list_start_session (GSM_CLIENT_LIST (client_list));
  gnome_dialog_set_sensitive (GNOME_DIALOG (dialog), 0, FALSE);
  gnome_dialog_set_sensitive (GNOME_DIALOG (dialog), 1, FALSE);
  gtk_signal_disconnect_by_func (GTK_OBJECT (session_list), 
				 sess_select_row_cb, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (left_scrolled),
				  GTK_POLICY_NEVER, GTK_POLICY_NEVER);
  gtk_container_remove (GTK_CONTAINER (left_scrolled), session_list);
  viewport = gtk_viewport_new (gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (left_scrolled)),
			       gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (left_scrolled)));
  gtk_container_add (GTK_CONTAINER (viewport), gnome_foot);
  gtk_widget_show_all (viewport);
  gtk_container_add (GTK_CONTAINER (left_scrolled), viewport);
}

static void
start_cb (GtkWidget *widget)
{
  if (chooser_timeout != -1)
    gtk_timeout_remove (chooser_timeout);

  start ();
}

static gint
start_timeout (gpointer data)
{
  start ();
  return 0;
}


static void
sess_select_row_cb (GtkWidget* widget, gint row)
{
  gchar* name;

  gtk_clist_get_text (GTK_CLIST (widget), row, 0, &name);
  gsm_client_list_saved_session (GSM_CLIENT_LIST (client_list), name);
  if (chooser_timeout == -1)
    {
      gtk_signal_connect(GTK_OBJECT (client_list), "initialized",
			 GTK_SIGNAL_FUNC (right_initialized_cb), NULL);
    }
  else
    {
      gtk_timeout_remove (chooser_timeout);
      chooser_timeout = gtk_timeout_add (chooser_delay, start_timeout, NULL);
    }
}

static void
cancel_cb (GtkWidget *widget)
{
  gnome_client_request_save (gnome_master_client(), GNOME_SAVE_BOTH, 1, 
			     GNOME_INTERACT_ANY, 0, 1);
}

static gchar* last_session = NULL;

static void
last_session_cb (GtkWidget *widget, gchar* name)
{
  last_session = g_strdup (name);
}

/* gnome-session is responsible for ensuring that there is always
 * at least one session to choose. */  
static void
saved_sessions_cb (GtkWidget *widget, GSList* session_names)
{
  GSList *list;

  gint selected_row = 0;
  
  for (list = session_names; list; list = list->next)
    {
      gint row;
      gchar* name = (gchar*)list->data;
      row = gtk_clist_append (GTK_CLIST (session_list), &name);
      if (! strcmp (last_session, name))
	selected_row = row;
    }
  gtk_clist_select_row (GTK_CLIST (session_list), selected_row, 0);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (left_scrolled),
				  GTK_POLICY_AUTOMATIC, 
				  GTK_POLICY_AUTOMATIC);
}

static void
right_initialized_cb (GtkWidget *widget)
{
  gboolean wait = (GTK_CLIST (session_list)->rows > 1);
  /* We are normally started BEFORE the WM: */
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER);
  if (!wait)
    start();
  gtk_widget_show_all (dialog);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (right_scrolled),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  if (wait)
    chooser_timeout = gtk_timeout_add (chooser_delay, start_timeout, NULL); 
  gtk_signal_disconnect_by_func (GTK_OBJECT (client_list), 
				 right_initialized_cb, NULL);
}


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
 * GnomeMeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */


/*
 *                         misc.cpp  -  description
 *                         ------------------------
 *   begin                : Thu Nov 22 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains miscellaneous functions.
 *   Additional Code      : De Michele Cristiano, Miguel Rodríguez 
 *
 */


#include "../config.h"

#include "misc.h"
#include "gnomemeeting.h"
#include "callbacks.h"
#include "config.h"

#include "dialog.h"
#include "gconf_widgets_extensions.h"

#include "../pixmaps/text_logo.xpm"

#include <gdk/gdkx.h>
#include <X11/Xlib.h>


/* Declarations */
extern GtkWidget *gm;


/* The functions */


void 
gnomemeeting_threads_enter () 
{
  if ((PThread::Current () != NULL) && (PThread::Current ()->GetThreadName () != "gnomemeeting")) {    
    gdk_threads_enter ();
  }
}


void 
gnomemeeting_threads_leave () 
{
  if ((PThread::Current () != NULL) && (PThread::Current ()->GetThreadName () != "gnomemeeting")) {

    gdk_threads_leave ();
  }
}


GtkWidget *
gnomemeeting_button_new (const char *lbl, 
			 GtkWidget *pixmap)
{
  GtkWidget *button = NULL;
  GtkWidget *hbox2 = NULL;
  GtkWidget *label = NULL;
  
  button = gtk_button_new ();
  label = gtk_label_new_with_mnemonic (lbl);
  hbox2 = gtk_hbox_new (FALSE, 0);

  gtk_box_pack_start(GTK_BOX (hbox2), pixmap, TRUE, TRUE, 0);  
  gtk_box_pack_start(GTK_BOX (hbox2), label, TRUE, TRUE, 0);
  
  gtk_container_add (GTK_CONTAINER (button), hbox2);

  return button;
}

void gnomemeeting_init_main_window_logo (GtkWidget *image)
{
  GdkPixbuf *tmp = NULL;
  GdkPixbuf *text_logo_pix = NULL;
  GtkRequisition size_request;

  GmWindow *gw = GnomeMeeting::Process ()->GetMainWindow ();

  gtk_widget_size_request (GTK_WIDGET (gw->video_frame), &size_request);

  if ((size_request.width != GM_QCIF_WIDTH) || 
      (size_request.height != GM_QCIF_HEIGHT)) {

     gtk_widget_set_size_request (GTK_WIDGET (gw->video_frame),
				  176, 144);
  }

  text_logo_pix = gdk_pixbuf_new_from_xpm_data ((const char **) text_logo_xpm);
  tmp = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, 176, 144);
  gdk_pixbuf_fill (tmp, 0x000000FF);  /* Opaque black */

  gdk_pixbuf_copy_area (text_logo_pix, 0, 0, 176, 60, 
			tmp, 0, 42);
  gtk_image_set_from_pixbuf (GTK_IMAGE (image),
			     GDK_PIXBUF (tmp));

  g_object_unref (text_logo_pix);
  g_object_unref (tmp);
}


/* This function overrides from a pwlib function */
#ifndef STATIC_LIBS_USED
static gboolean
assert_error_msg (gpointer data)
{
  /* FIX ME: message */
  gdk_threads_enter ();
  gnomemeeting_error_dialog (GTK_WINDOW (gm), _("Generic error"),
			     (gchar *) data);
  gdk_threads_leave ();

  return FALSE;
}


void 
PAssertFunc (const char *file, int line, 
	     const char *className, const char *msg)
{
  static bool inAssert;

  if (inAssert)
    return;

  inAssert = true;

  g_idle_add (assert_error_msg, (gpointer) msg);

  inAssert = FALSE;

  return;
}
#endif


GtkWidget * 
gnomemeeting_incoming_call_popup_new (gchar *utf8_name, 
				      gchar *utf8_app,
				      gchar *utf8_url)
{
  GtkWidget *label = NULL;
  GtkWidget *widget = NULL;
  GtkWidget *vbox = NULL;
  GtkWidget *b1 = NULL;
  GtkWidget *b2 = NULL;

  gchar *msg = NULL;
  GmWindow  *gw = NULL;
    
  gw = GnomeMeeting::Process ()->GetMainWindow ();

  widget = gtk_dialog_new ();
  b2 = gtk_dialog_add_button (GTK_DIALOG (widget),
			      _("Reject"), 0);
  b1 = gtk_dialog_add_button (GTK_DIALOG (widget),
			      _("Accept"), 1);

  gtk_dialog_set_default_response (GTK_DIALOG (widget), 1);

  vbox = GTK_DIALOG (widget)->vbox;
  
  msg = g_strdup_printf ("%s <i>%s</i>",
			 _("Incoming call from"), (const char*) utf8_name);
  label = gtk_label_new (NULL);
  gtk_label_set_markup (GTK_LABEL (label), msg);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 10);
  gtk_misc_set_alignment (GTK_MISC (label), 0.5, 0.0);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  g_free (msg);

  if (utf8_url) {
    
    label = gtk_label_new (NULL);
    msg =
      g_strdup_printf ("<b>%s</b> <span foreground=\"blue\"><u>%s</u></span>",
		       _("Remote URL:"), utf8_url);
    gtk_label_set_markup (GTK_LABEL (label), msg);
    gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
    g_free (msg);
  }

  if (utf8_app) {

    label = gtk_label_new (NULL);
    msg = g_strdup_printf ("<b>%s</b> %s",
			   _("Remote Application:"), utf8_app);
    gtk_label_set_markup (GTK_LABEL (label), msg);
    gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);
    gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.0);
    g_free (msg);
  }

  g_signal_connect (G_OBJECT (b1), "clicked",
		    G_CALLBACK (connect_cb), gw);

  g_signal_connect (G_OBJECT (b2), "clicked",
		    G_CALLBACK (disconnect_cb), gw);

  g_signal_connect (G_OBJECT (widget), "delete-event",
		    G_CALLBACK (disconnect_cb), gw);
  
  gtk_window_set_transient_for (GTK_WINDOW (widget),
				GTK_WINDOW (gm));

  gtk_widget_show_all (widget);

  return widget;
}


static int statusbar_clear_msg (gpointer data)
{
  GmWindow *gw = NULL;
  int id = 0;

  gdk_threads_enter ();

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  id = gtk_statusbar_get_context_id (GTK_STATUSBAR (gw->statusbar),
				     "statusbar");

  gtk_statusbar_remove (GTK_STATUSBAR (gw->statusbar), id, 
			GPOINTER_TO_INT (data));

  gdk_threads_leave ();

  return FALSE;
}


void 
gnomemeeting_statusbar_flash (GtkWidget *widget, const char *msg, ...)
{
  va_list args;
  char    buffer [1025];
  int timeout_id = 0;
  int msg_id = 0;

  gint id = 0;
  int len = g_slist_length ((GSList *) (GTK_STATUSBAR (widget)->messages));
  id = gtk_statusbar_get_context_id (GTK_STATUSBAR (widget), "statusbar");
  
  for (int i = 0 ; i < len ; i++)
    gtk_statusbar_pop (GTK_STATUSBAR (widget), id);


  va_start (args, msg);
  vsnprintf (buffer, 1024, msg, args);

  msg_id = gtk_statusbar_push (GTK_STATUSBAR (widget), id, buffer);

  timeout_id = gtk_timeout_add (4000, statusbar_clear_msg, 
				GINT_TO_POINTER (msg_id));

  va_end (args);
}


void 
gnomemeeting_statusbar_push (GtkWidget *widget, const char *msg, ...)
{
  gint id = 0;
  int len = g_slist_length ((GSList *) (GTK_STATUSBAR (widget)->messages));
  id = gtk_statusbar_get_context_id (GTK_STATUSBAR (widget), "statusbar");
  
  for (int i = 0 ; i < len ; i++)
    gtk_statusbar_pop (GTK_STATUSBAR (widget), id);

  if (msg) {

    va_list args;
    char buffer [1025];

    va_start (args, msg);
    vsnprintf (buffer, 1024, msg, args);

    gtk_statusbar_push (GTK_STATUSBAR (widget), id, buffer);

    va_end (args);
  }
}


GtkWidget *
gnomemeeting_video_window_new (gchar *title, 
			       GtkWidget *&image,
			       gchar *window_name)
{
  GtkWidget *vbox = NULL;
  GtkWidget *window = NULL;

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), title);
  g_object_set_data_full (G_OBJECT (window), "window_name",
			  g_strdup (window_name), g_free);
  
  vbox = gtk_vbox_new (0, FALSE);
  image = gtk_image_new ();

  gtk_box_pack_start (GTK_BOX (vbox), image, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (window), 0);

  gtk_container_add (GTK_CONTAINER (window), vbox);
  gtk_widget_realize (image);

  gtk_widget_set_size_request (GTK_WIDGET (image), 176, 144);
  gtk_window_set_resizable (GTK_WINDOW (window), TRUE);

  gtk_widget_show_all (vbox);
    
  g_signal_connect (G_OBJECT (window), "delete_event",
		    G_CALLBACK (gtk_widget_hide_on_delete), 0);

  return window;
}


PString gnomemeeting_pstring_cut (PString s)
{
  PString s2 = s;

  if (s.IsEmpty ())
    return s2;

  PINDEX bracket = s2.Find('[');                                          
                                                                               
  if (bracket != P_MAX_INDEX)                                                
    s2 = s2.Left (bracket);                                            

  bracket = s2.Find('(');                                                 
                                                                               
  if (bracket != P_MAX_INDEX)                                                
    s2 = s2.Left (bracket);     

  return s2;
}


gchar *gnomemeeting_from_iso88591_to_utf8 (PString iso_string)
{
  if (iso_string.IsEmpty ())
    return NULL;

  gchar *utf_8_string =
    g_convert ((const char *) iso_string.GetPointer (),
               iso_string.GetSize (), "UTF-8", "ISO-8859-1", 0, 0, 0);
  
  return utf_8_string;
}


gchar *gnomemeeting_get_utf8 (PString str)
{
  gchar *utf8_str = NULL;

  if (g_utf8_validate ((gchar *) (const unsigned char*) str, -1, NULL))
    utf8_str = g_strdup ((char *) (const char *) (str));
  else
    utf8_str = gnomemeeting_from_iso88591_to_utf8 (str);

  return utf8_str;
}
    

/* Stolen from GDK */
static void
gdk_wmspec_change_state (gboolean add,
			 GdkWindow *window,
			 GdkAtom state1,
			 GdkAtom state2)
{
  GdkDisplay *display = 
    gdk_screen_get_display (gdk_drawable_get_screen (GDK_DRAWABLE (window)));
  XEvent xev;
  
#define _NET_WM_STATE_REMOVE        0    /* remove/unset property */
#define _NET_WM_STATE_ADD           1    /* add/set property */
#define _NET_WM_STATE_TOGGLE        2    /* toggle property  */  
  
  xev.xclient.type = ClientMessage;
  xev.xclient.serial = 0;
  xev.xclient.send_event = True;
  xev.xclient.window = GDK_WINDOW_XID (window);
  xev.xclient.message_type = 
    gdk_x11_get_xatom_by_name_for_display (display, "_NET_WM_STATE");
  xev.xclient.format = 32;
  xev.xclient.data.l[0] = add ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE;
  xev.xclient.data.l[1] = gdk_x11_atom_to_xatom_for_display (display, state1);
  xev.xclient.data.l[2] = gdk_x11_atom_to_xatom_for_display (display, state2);
  
  XSendEvent (GDK_WINDOW_XDISPLAY (window),
	      GDK_WINDOW_XWINDOW (gdk_screen_get_root_window (gdk_drawable_get_screen (GDK_DRAWABLE (window)))),
	      False, SubstructureRedirectMask | SubstructureNotifyMask,
	      &xev);
}


void
gdk_window_set_always_on_top (GdkWindow *window, 
			      gboolean enable)
{
  gdk_wmspec_change_state (enable, window, 
			   gdk_atom_intern ("_NET_WM_STATE_ABOVE", FALSE), 0);
}


gboolean 
gnomemeeting_window_is_visible (GtkWidget *w)
{
  return (GTK_WIDGET_VISIBLE (w) && !(gdk_window_get_state (GDK_WINDOW (w->window)) & GDK_WINDOW_STATE_ICONIFIED));
}


void
gnomemeeting_window_show (GtkWidget *w)
{
  int x = 0;
  int y = 0;

  gchar *window_name = NULL;
  gchar *gconf_key_size = NULL;
  gchar *gconf_key_position = NULL;
  gchar *size = NULL;
  gchar *position = NULL;
  gchar **couple = NULL;
  
  if (!w)
    return;
  
  window_name = (char *) g_object_get_data (G_OBJECT (w), "window_name");

  if (!window_name)
    return;
  
  gconf_key_position =
    g_strdup_printf ("%s%s/position", USER_INTERFACE_KEY, window_name);
  gconf_key_size =
    g_strdup_printf ("%s%s/size", USER_INTERFACE_KEY, window_name);  

  if (!gnomemeeting_window_is_visible (w)) {
    
    position = gconf_get_string (gconf_key_position);
    if (position)
      couple = g_strsplit (position, ",", 0);

    if (couple && couple [0])
      x = atoi (couple [0]);
    if (couple && couple [1])
      y = atoi (couple [1]);


    if (x != 0 && y != 0)
      gtk_window_move (GTK_WINDOW (w), x, y);

    gtk_window_present (GTK_WINDOW (w));

    g_strfreev (couple);
    g_free (position);


    if (gtk_window_get_resizable (GTK_WINDOW (w))) {

      size = gconf_get_string (gconf_key_size);
      if (size)
	couple = g_strsplit (size, ",", 0);

      if (couple && couple [0])
	x = atoi (couple [0]);
      if (couple && couple [1])
	y = atoi (couple [1]);

      if (x > 0 && y > 0)
	gtk_window_resize (GTK_WINDOW (w), x, y);

      g_strfreev (couple);
      g_free (size);
    }

    gtk_widget_show (w);
  }
  
  g_free (gconf_key_position);
  g_free (gconf_key_size);
}


void
gnomemeeting_window_hide (GtkWidget *w)
{
  int x = 0;
  int y = 0;

  gchar *window_name = NULL;
  gchar *gconf_key_size = NULL;
  gchar *gconf_key_position = NULL;
  gchar *size = NULL;
  gchar *position = NULL;
  
  if (!w)
    return;
  
  window_name = (char *) g_object_get_data (G_OBJECT (w), "window_name");

  if (!window_name)
    return;
 
  gconf_key_position =
    g_strdup_printf ("%s%s/position", USER_INTERFACE_KEY, window_name);
  gconf_key_size =
    g_strdup_printf ("%s%s/size", USER_INTERFACE_KEY, window_name);

  
  /* If the window is visible, save its position and hide the window */
  if (gnomemeeting_window_is_visible (w)) {
    
    gtk_window_get_position (GTK_WINDOW (w), &x, &y);
    position = g_strdup_printf ("%d,%d", x, y);
    gconf_set_string (gconf_key_position, position);
    g_free (position);

    if (gtk_window_get_resizable (GTK_WINDOW (w))) {

      gtk_window_get_size (GTK_WINDOW (w), &x, &y);
      size = g_strdup_printf ("%d,%d", x, y);
      gconf_set_string (gconf_key_size, size);
      g_free (size);
    }

	
    gtk_widget_hide (w);
  }
    
  
  g_free (gconf_key_position);
  g_free (gconf_key_size);
}


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
 *                         tray.cpp  -  description
 *                         ------------------------
 *   begin                : Wed Oct 3 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras, 2002 by Miguel 
 *                          Rodríguez
 *   description          : This file contains all functions needed for
 *                          system tray icon.
 *   Additional code      : migrax@terra.es
 *
 */


#include "../config.h"

#include "tray.h"
#include "gnomemeeting.h"
#include "eggtrayicon.h"
#include "menu.h"
#include "callbacks.h"
#include "misc.h"

#include "stock-icons.h"
#include "gconf_widgets_extensions.h"


/* Declarations */

typedef struct _GmTray {

  GtkWidget *image;
  gboolean ringing;
  gboolean embedded;
} GmTray;


extern GtkWidget *gm;


static gint tray_icon_embedded (GtkWidget *, 
                                gpointer);

static gint tray_clicked_callback (GtkWidget *, 
                                   GdkEventButton *, 
                                   gpointer);

static gint tray_icon_destroyed (GtkWidget *, 
                                 gpointer);


/* DESCRIPTION  :  This callback is called when the tray appears on the panel.
 * BEHAVIOR     :  Store the info in the GMObject.
 * PRE          :  /
 */
static gint 
tray_icon_embedded (GtkWidget *tray_icon, 
                    gpointer data)
{
  GmTray *gt = NULL;
  
  IncomingCallMode icm = AVAILABLE;

  gt = (GmTray *) g_object_get_data (G_OBJECT (tray_icon), "GMObject");
  
  if (!gt)
    return FALSE;
 
  /* Check the current incoming call mode */
  icm =
    (IncomingCallMode) gconf_get_int (CALL_OPTIONS_KEY "incoming_call_mode");

  gnomemeeting_tray_update (tray_icon, GMH323EndPoint::Standby, icm);
  gt->embedded = TRUE;

  return true;
}


/* DESCRIPTION  :  This callback is called when the panel gets closed
 *                 after the tray has been embedded.
 * BEHAVIOR     :  Create a new tray_icon and substitute the old one.
 * PRE          :  /
 */
static gint 
tray_icon_destroyed (GtkWidget *tray, 
                     gpointer data) 
{
  GmWindow *gw = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  /* Somehow the delete_event never got called, so we use "destroy" */
  if (tray != gw->docklet)
    return TRUE;
  
  gw->docklet = gnomemeeting_init_tray ();

  gnomemeeting_window_show (gm);

  
  return TRUE;
}


/* DESCRIPTION  :  This callback is called when the user double clicks on the 
 *                 tray event-box.
 * BEHAVIOR     :  Show / hide the GnomeMeeting GUI.
 * PRE          :  data != NULL.
 */
static gint
tray_clicked_callback (GtkWidget *w,
		       GdkEventButton *event,
		       gpointer data)
{
  GtkWidget *widget = NULL;
  GmWindow *gw = NULL;

  gw = GnomeMeeting::Process ()->GetMainWindow ();
  
  if (event->type == GDK_BUTTON_PRESS) {

    if (event->button == 1)
      widget = gm;
    else if (event->button == 2)
      widget = gw->ldap_window;
    else
      return FALSE;

      
    if (!gnomemeeting_window_is_visible (widget))
      gnomemeeting_window_show (widget);
    else
      gnomemeeting_window_hide (widget);

    return TRUE;
  }
  
  return FALSE;
}


/* The functions */
GtkWidget *
gnomemeeting_init_tray ()
{
  GtkWidget *tray_icon = NULL;
  GtkWidget *event_box = NULL;

  GmTray *gt = NULL;

  /* Build the internal GMObject structure */
  gt = (GmTray *) g_malloc (sizeof (GmTray));

  /* Start building the GMObject and associate the structure
   * to the object so that it is deleted when the object is
   * destroyed
   */
  tray_icon = GTK_WIDGET (egg_tray_icon_new (_("GnomeMeeting Tray Icon")));

  event_box = gtk_event_box_new ();
  gt->image = gtk_image_new_from_stock (GM_STOCK_STATUS_AVAILABLE,
                                        GTK_ICON_SIZE_MENU);
  gt->ringing = FALSE;
  gt->embedded = FALSE;
  
  gtk_container_add (GTK_CONTAINER (event_box), gt->image);
  gtk_container_add (GTK_CONTAINER (tray_icon), event_box);
  
  g_object_set_data_full (G_OBJECT (tray_icon), "GMObject",
                          (gpointer) gt, (GDestroyNotify) (g_free));

  /* Connect the signals */
  g_signal_connect (G_OBJECT (tray_icon), "embedded",
		    G_CALLBACK (tray_icon_embedded), NULL);
  g_signal_connect (G_OBJECT (tray_icon), "destroy",
		    G_CALLBACK (tray_icon_destroyed), NULL);
  g_signal_connect (G_OBJECT (event_box), "button_press_event",
		    G_CALLBACK (tray_clicked_callback), NULL);

  gtk_widget_show_all (tray_icon);
  
  return tray_icon;
}


void 
gnomemeeting_tray_update (GtkWidget *tray_icon,
                          GMH323EndPoint::CallingState calling_state, 
                          IncomingCallMode icm,
                          BOOL forward_on_busy)
{
  GmTray *gt = NULL;
  
  gt = (GmTray *) g_object_get_data (G_OBJECT (tray_icon), "GMObject");

  if (!gt)
    return;
  
  if (calling_state == GMH323EndPoint::Standby) {

    switch (icm) {

    case (AVAILABLE): 
      gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
                                GM_STOCK_STATUS_AVAILABLE, 
                                GTK_ICON_SIZE_MENU);
      break;
   
    case (FREE_FOR_CHAT):  
      gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
                                GM_STOCK_STATUS_FREE_FOR_CHAT, 
                                GTK_ICON_SIZE_MENU);
      break;
    
    case (BUSY):  
      gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
                                GM_STOCK_STATUS_BUSY, 
                                GTK_ICON_SIZE_MENU);
      break;
    
    case (FORWARD):  
      gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
                                GM_STOCK_STATUS_FORWARD, 
                                GTK_ICON_SIZE_MENU);
      break;

    default:
      break;
    }
  }
  else {

    if (forward_on_busy)
      gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
                                GM_STOCK_STATUS_FORWARD, 
                                GTK_ICON_SIZE_MENU);
    else
      gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
                                GM_STOCK_STATUS_IN_A_CALL, 
                                GTK_ICON_SIZE_MENU);
  }
}


void 
gnomemeeting_tray_ring (GtkWidget *tray)
{
  GmTray *gt = NULL;

  gt = (GmTray *) g_object_get_data (G_OBJECT (tray), "GMObject");

  if (!gt)
    return;

  if (gt->ringing) {

    gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
                              GM_STOCK_STATUS_AVAILABLE, 
                              GTK_ICON_SIZE_MENU);
    gt->ringing = FALSE;
  }
  else {

    gtk_image_set_from_stock (GTK_IMAGE (gt->image), 
                              GM_STOCK_STATUS_RINGING, 
                              GTK_ICON_SIZE_MENU); 
    gt->ringing = TRUE;
  }
}


gboolean 
gnomemeeting_tray_is_ringing (GtkWidget *tray)
{
  GmTray *gt = NULL;

  gt = (GmTray *) g_object_get_data (G_OBJECT (tray), "GMObject");

  if (!gt)
    return FALSE;
  
  return (gt->ringing);
}


gboolean 
gnomemeeting_tray_is_embedded (GtkWidget *tray)
{
  GmTray *gt = NULL;

  gt = (GmTray *) g_object_get_data (G_OBJECT (tray), "GMObject");

  if (!gt)
    return FALSE;

  return (gt->embedded);
}

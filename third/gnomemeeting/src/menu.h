
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
 *                         menu.h  -  description 
 *                         ----------------------
 *   begin                : Tue Dec 23 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : Functions to create the menus.
 *
 */


#ifndef _MENU_H_
#define _MENU_H_

#include "gtk_menu_extensions.h"

enum {

  LOCAL_VIDEO, 
  REMOTE_VIDEO, 
  BOTH_INCRUSTED, 
  BOTH_LOCAL, 
  BOTH
};

#include "common.h"


/* The functions */


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Create the menu and return a pointer to the newly created
 *                 menu. The menu is created in its initial state, with
 *                 required items being unsensitive.
 * PRE          :  The accel group.
 */
GtkWidget *gnomemeeting_init_menu (GtkAccelGroup *);


GtkWidget *gnomemeeting_video_popup_init_menu (GtkWidget *,
					       GtkAccelGroup *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Creates a video menu which will popup, and attach it
 *                 to the given widget.
 * PRE          :  The widget to attach the menu to, and the accelgroup.
 */
GtkWidget *gnomemeeting_tray_init_menu (GtkWidget *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the speed dials menu in the call menu given the
 *                 main menu.
 * PRE          :  /
 */
void gnomemeeting_speed_dials_menu_update (GtkWidget *);

     
/* DESCRIPTION  :  /
 * BEHAVIOR     :  Updates the main menu items that depend on the current
 *                 calling state of the endpoint.
 * PRE          :  A valid GMH323EndPoint calling state.
 */
void gnomemeeting_menu_update_sensitivity (unsigned);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Updates the main menu items that depend on the currently
 *                 opened audio and video channels.
 * PRE          :  The first parameter is TRUE if we are updating video
 *                 channels related items, FALSE if we are updating audio
 *                 channels related items. The second parameter is TRUE
 *                 if we are transmitting audio (or video), the third is TRUE
 *                 if we are receiving audio (or video).
 */
void gnomemeeting_menu_update_sensitivity (BOOL,
					   BOOL,
					   BOOL);

#endif




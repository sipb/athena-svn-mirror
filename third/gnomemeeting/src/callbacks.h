
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
 *                         callbacks.h  -  description
 *                         ---------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains callbacks common to several
 *                          files.
 *
 */


#ifndef _CALLBACKS_H
#define _CALLBACKS_H

#include "common.h"


/* DESCRIPTION  :  This callback is called when the user chooses to hold
 *                 a call.
 * BEHAVIOR     :  Hold the current call.
 * PRE          :  /
 */
void hold_call_cb (GtkWidget *,
		   gpointer);


/* DESCRIPTION  :  This callback is called when the user chooses to forward
 *                 a call.
 * BEHAVIOR     :  Forward the current call.
 * PRE          :  /
 */
void transfer_call_cb (GtkWidget *,
		       gpointer);


/* DESCRIPTION  :  This callback is called when the user chooses to save.
 * BEHAVIOR     :  Saves the picture in the current video stream in a file.
 * PRE          :  /
 */
void save_callback (GtkWidget *,
		    gpointer);


/* DESCRIPTION  :  This callback is called when the user chooses to pause
 *                 the audio transmission.
 * BEHAVIOR     :  Pause the audio or video channel transmission.
 * PRE          :  gpointer = 0 (audio) or 1 (video)
 */
void pause_channel_callback (GtkWidget *,
			     gpointer);


/* DESCRIPTION  :  This callback is called when the user chooses to open
 *                 the about window.
 * BEHAVIOR     :  Open the about window.
 * PRE          :  /
 */
void about_callback (GtkWidget *,
		     gpointer);


/* DESCRIPTION  :  This callback is called when the user chooses to open
 *                 the help window.
 * BEHAVIOR     :  Open the help window.
 * PRE          :  /
 */
void help_cb (GtkWidget *,
              gpointer);


/* DESCRIPTION  :  This callback is called when the user choose to establish
 *                 a connection.
 * BEHAVIOR     :  Call the remote endpoint or accept the incoming call.
 * PRE          :  /
 */
void connect_cb (GtkWidget *,
		 gpointer);


/* DESCRIPTION  :  This callback is called when the user choose to stop
 *                 a connection.
 * BEHAVIOR     :  Do not accept the incoming call or stops the current call.
 * PRE          :  /
 */
void disconnect_cb (GtkWidget *,
		    gpointer);


/* DESCRIPTION  :  This callback is called when the user chooses to quit.
 * BEHAVIOR     :  Quit.
 * PRE          :  /
 */
void quit_callback (GtkWidget *,
		    gpointer);


/* DESCRIPTION  :  Simple wrapper that will call gnomemeeting_hide_window.
 * BEHAVIOR     :  Calls gnomemeeting_window_hide.
 * PRE          :  /
 */
gboolean delete_window_cb (GtkWidget *,
                           GdkEvent *,
                           gpointer);


/* DESCRIPTION  :  Simple wrapper that will call gnomemeeting_show_window.
 * BEHAVIOR     :  Calls gnomemeeting_window_show.
 * PRE          :  The gpointer is a valid pointer to the GtkWindow that needs
 *                 to be shown with the correct size and position.
 */
void show_window_cb (GtkWidget *,
		     gpointer);


/* DESCRIPTION  :  Quit callback.
 * BEHAVIOR     :  Disconnects, then simply call gtk_main_quit.
 * PRE          :  /
 */
void gtk_main_quit_callback (int,
			     gpointer);
#endif


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
 *                         main_window.h  -  description
 *                         -----------------------------
 *   begin                : Mon Mar 26 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          build the main window.
 */


#ifndef _MAIN_INTERFACE_H
#define _MAIN_INTERFACE_H

#include "common.h"

void gnomemeeting_dialpad_event (const char);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the main window sensitivity following the calling
 *                 state.
 * PRE          :  A valid GMH323EndPoint calling state.
 */
void gnomemeeting_main_window_update_sensitivity (unsigned);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Update the main window sensitivity following the opened
 *                 and closed audio and video channels.
 * PRE          :  The first parameter is TRUE if we are updating video
 *                 channels related items, FALSE if we are updating audio
 *                 channels related items. The second parameter is TRUE
 *                 if we are transmitting audio (or video), the third is TRUE
 *                 if we are receiving audio (or video).
 */
void gnomemeeting_main_window_update_sensitivity (BOOL,
						  BOOL,
						  BOOL);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Builds the main window and adds the popup to the image.
 * PRE          :  Accels.
 **/
GtkWidget *gnomemeeting_main_window_new (GmWindow *);



/* DESCRIPTION  :  /
 * BEHAVIOR     :  Enable/disable the progress animation in the statusbar.
 * PRE          :  /
 */
void gnomemeeting_main_window_enable_statusbar_progress (gboolean);
#endif


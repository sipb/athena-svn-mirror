
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
 *                         tray.h  -  description
 *                         ----------------------
 *   begin                : Wed Oct 3 2001
 *   copyright            : (C) 2000-2004 by Damien Sandras, 2002 by Miguel
 *                          Rodríguez
 *   description          : This file contains all functions needed for
 *                          system tray icon.
 *   Additional code      : migrax@terra.es
 *
 */


#ifndef _TRAY_H_
#define _TRAY_H_

#include "common.h"
#include "endpoint.h"


G_BEGIN_DECLS

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Init the tray.
 * PRE          :  / 
 */
GtkWidget *gnomemeeting_init_tray ();


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Updates the tray icon following the current calling state, 
*                 the incoming call mode and forward on busy setting.
* PRE          :  A valid current calling state and a valid incoming call mode
 *                 or the tray icon won't be updated at all. A valid tray icon.
 */
void gnomemeeting_tray_update (GtkWidget *,
                               GMH323EndPoint::CallingState, 
                               IncomingCallMode,
                               BOOL = FALSE);

/* DESCRIPTION  : /
 * BEHAVIOR     : Displays the ringing icon or not.
 * PRE          : A valid tray icon.
 */
void gnomemeeting_tray_ring (GtkWidget *);


/* DESCRIPTION  : /
 * BEHAVIOR     : Returns true if the tray shows a ringing phone.
 * PRE          : A valid tray icon.
 */
gboolean gnomemeeting_tray_is_ringing (GtkWidget *);


/* DESCRIPTION  : / 
 * BEHAVIOR     : Returns true if the tray is embedded in the panel.
 * PRE          : A valid tray icon.
 */
gboolean gnomemeeting_tray_is_embedded (GtkWidget *);

G_END_DECLS

#endif


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
 *                         pref_window.h  -  description
 *                         -----------------------------
 *   begin                : Tue Dec 26 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          create the preferences window and all its callbacks
 *   Additional code      : Miguel Rodríguez Pérez  <migrax@terra.es> 
 */


#ifndef _PREFERENCES_H_
#define _PREFERENCES_H_

#include "common.h"


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Refreshes the devices list in the GUI to update them from
 *                 the GnomeMeeting available devices list.
 * PRE          :  /
 */
void 
gnomemeeting_pref_window_update_devices_list ();


/* DESCRIPTION  :  /
 * BEHAVIOR     :  It builds the preferences window
 *                 (sections' ctree / Notebook pages) and connect GTK signals
 *                 to appropriate callbacks, then returns it.
 * PRE          :  /
 */
GtkWidget *
gnomemeeting_pref_window_new (GmPrefWindow *);


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Add the sound events to the tree view. 
 * PRE          :  /
 */
void
gnomemeeting_prefs_window_sound_events_list_build (GtkTreeView *); 


/* DESCRIPTION  :  / 
 * BEHAVIOR     :  Add the codecs to the codecs list. 
 * PRE          :  The second argument indicates if a quicknet card is used
 *                 and the third if software codecs are supported or not.
 */
void 
gnomemeeting_codecs_list_build (GtkListStore *,
				BOOL,
				BOOL);


#endif
     

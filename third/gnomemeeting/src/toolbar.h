
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
 *                        toolbar.h  -  description
 *                        -------------------------
 *   begin                : Sat Dec 23 2000
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains all the functions needed to
 *                          create the toolbar.
 *
 */


#ifndef _TOOLBAR_H_
#define _TOOLBAR_H_

#include "common.h"


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Create the main toolbar and return a pointer to the newly
 *                 created toolbar.
 * PRE          :  /
 */
GtkWidget *gnomemeeting_init_main_toolbar (void);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Create the left toolbar and return a pointer to the newly
 *                 created toolbar.
 * PRE          :  /
 */
GtkWidget *gnomemeeting_init_left_toolbar (void);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Updates the connect button.
 * PRE          :  /
 */
void connect_button_update_pixmap (GtkToggleButton *, int);
#endif

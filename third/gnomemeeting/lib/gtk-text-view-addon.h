
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
 * GnomeMeeting is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OpenH323 and Pwlib, and distribute the combination, without
 * applying the requirements of the GNU GPL to the OpenH323 program, as long
 * as you do follow the requirements of the GNU GPL for all the rest of the
 * software thus combined.
 */

/*
 *                         gtk-text-view-addon.h  -  description 
 *                         ------------------------------------
 *   begin                : Sat Nov 29 2003, but based on older code
 *   copyright            : (C) 2000-2004 by Julien Puydt
 *                                           Miguel Rodríguez,
 *                                           StÃ©phane Wirtel
 *                                           Kenneth Christiansen
 *   description          : Add-on functions for regex-based context menus
 *
 */

#include <gtk/gtk.h>

#ifndef __GTK_TEXT_VIEW_ADD_H
#define __GTK_TEXT_VIEW_ADD_H

G_BEGIN_DECLS

/**
 * gtk_text_view_new_with_regex:
 * 
 * Creates a new gtk text view, but with the added functionality that regex-enabled text tags in it
 * gain highlighting when the mouse hovers over them
 **/
GtkWidget *gtk_text_view_new_with_regex (void);

G_END_DECLS

#endif

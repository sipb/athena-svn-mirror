
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
 *                         chat_window.h  -  description
 *                         -----------------------------
 *   begin                : Wed Jan 23 2002
 *   copyright            : (C) 2000-2004 by Damien Sandras
 *   description          : This file contains functions to build the chat
 *                          window. It uses DTMF tones.
 *   Additional code      : Kenneth Christiansen  <kenneth@gnu.org>
 *
 */


#ifndef __CHAT_WINDOW_H
#define __CHAT_WINDOW_H

#include "common.h"


G_BEGIN_DECLS


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Initializes the text chat view.
 * PRE          :  /
 */
GtkWidget *
gnomemeeting_text_chat_new (GmTextChat *);


/* DESCRIPTION  :  /
 * BEHAVIOR     :  Clears the text chat view.
 * PRE          :  The first parameter is fictive so that the function
 *                 can be used in a callback.
 */
void
gnomemeeting_text_chat_clear (GtkWidget *,
			      GmTextChat *);


/* DESCRIPTION: /
 * BEHAVIOR :  Signals the text chat that a connection begins or ends
 */
void
gnomemeeting_text_chat_call_start_notification (void);
void
gnomemeeting_text_chat_call_stop_notification (void);

/* DESCRIPTION  :  /
 * BEHAVIOR     :  Displays the colored text chat message,
 *		with some enhancements (context menu
 *		for uris, graphics for smileys, etc)
 * PRE          :  The name of the user, the name of the remote user,
 *                 0 for local user string, 1 for remote user received string.
 */
void
gnomemeeting_text_chat_insert (PString,
			       PString,
			       int);

G_END_DECLS

#endif /* __CHAT_WINDOW_H */

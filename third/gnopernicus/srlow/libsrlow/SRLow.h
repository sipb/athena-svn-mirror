/* SRLow.h
 *
 * Copyright 2001, 2002 Sun Microsystems, Inc.,
 * Copyright 2001, 2002 BAUM Retec, A.G.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _SRLOW_H
#define _SRLOW_H

#include "SRObject.h"
#include "SREvent.h"

#define SR_MOUSE_BUTTON_LEFT	1
#define SR_MOUSE_BUTTON_RIGHT	2


/**
 * SRLClientHandle :
 *
 * Handle for a registered client.
**/
typedef long SRLClientHandle;

/* value who means an invalid client */
#define SRL_CLIENT_HANDLE_INVALID -1


/**
 * SRLClient :
 * @event_proc: pointer to a #SROnEventPproc function.
 *
 * Structure containing address of client functions called 
 * when something is happens.
**/
typedef struct _SRLClient
{
    SROnEventProc event_proc;
}SRLClient;

gboolean srl_init 		();
gboolean srl_terminate 		();
SRLClientHandle srl_add_client 	(const SRLClient *client);
gboolean srl_remove_client 	(SRLClientHandle client_handle);
gboolean srl_mouse_move 	(gint x, gint y);
gboolean srl_mouse_click 	(gint button);
gboolean srl_mouse_button_down 	(gint button);
gboolean srl_mouse_button_up 	(gint button);
gboolean srl_set_watch_for_object (SRObject *obj);
void     srl_unwatch_all_objects ();

#endif 	/*_SRLOW_H*/

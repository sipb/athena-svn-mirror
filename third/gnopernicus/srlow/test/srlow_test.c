/* srlow_test.c
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "SRLow.h"
#include "SRTest.h"


void 
event_notify (const SREvent *event, 
	      unsigned long flags)
{

    SRObject *obj;
    SREventType type;
    static int first_time = 1;
    
    if (first_time)
    {
	srl_mouse_move (100, 100);
	srl_mouse_click (SR_MOUSE_BUTTON_LEFT);
	
	sleep (2);
	srl_mouse_move (120, 120);
	srl_mouse_button_down (SR_MOUSE_BUTTON_LEFT);    
	srl_mouse_button_up (SR_MOUSE_BUTTON_LEFT);

	first_time = 0;
    }


    if (sre_get_type (event, &type))
    {
	gchar *msg = NULL;
	switch (type)
	{
	    case SR_EVENT_SRO:
		msg = g_strdup ("EVENT");
		break;
	    default:
		break;
	};
	if (msg)
	{
	    if (sre_get_event_data (event, (void**)&obj))
	    {
		SRTest_print_obj (obj, msg);
	    }
	}
	g_free (msg);	
    }
}

int
main(int argc, 
     char **argv)
{
    SRLClient client;
    SRLClientHandle client_handle;
    
    sru_init ();

    srl_init ();
    client.event_proc = (SROnEventProc) event_notify;
    client_handle = srl_add_client (&client);
    fprintf (stderr, "CLIENT HANDLE : %d" , (int) client_handle);

    
    sru_entry_loop ();
    
    srl_remove_client (client_handle);
    srl_terminate ();
    sru_terminate ();
    return 0;
}


/* gtk_srlow_test.c
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

#include "SRLow.h"
#include "SRTest.h"
#include <string.h>

void event_notify (SREvent *event, unsigned long flags)
{
    SREventType type;
    if (sre_get_type (event, &type))
    {
	SRObject *obj;
	char tmp[20] = "";
	sre_get_event_data (event, (void **)&obj);
	switch (type)
	{
	    case SR_EVENT_SRO:
		strcpy (tmp, "FOCUS");
		break;
	    default:
		return;
	}
	if (tmp[0])
	{
	    SRTest_print_obj (obj, tmp);
	    SRTest_show_obj (obj);
	}
    }
}



int
main(int argc, char **argv)
{
    SRLClient client;
    SRLClientHandle client_handle;

    client.event_proc = (SROnEventProc) event_notify;

    SRTest_init (&argc, &argv);
    sru_init  ();
    srl_init ();
    client_handle = srl_add_client (&client);
    


    sru_entry_loop ();
    
    srl_remove_client (client_handle);
    return 0;
}


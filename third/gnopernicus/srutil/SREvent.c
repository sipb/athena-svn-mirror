/* SREvent.c
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

#include "SREvent.h"
#include <cspi/spi.h>
#include "config.h"
#include "SRMessages.h"
#include <bonobo.h>
/*
 *
 * Screen Reader Event Object's methods
 *
 */
 
/* make all initialization for a SREvent structure.
   event pointer MUST not be NULL */    
static gboolean 
sre_init (SREvent *event)
{
    struct timeval time_value;

    gettimeofday(&time_value, NULL);

    event->type = SR_EVENT_NULL;
    event->time = time_value.tv_sec * 1000000 + time_value.tv_usec;
    event->data = NULL;
    event->data_destructor = NULL;

    return TRUE;
}

/* make all necessary stuff for cleanning a SREvent structure.
    event pointer MUST not be NULL */
static gboolean 
sre_terminate (SREvent *event)
{

    if(event->data_destructor != NULL)
    {
	event->data_destructor(event->data);
    }

    return TRUE ;
}


SREvent* 
sre_new ()
{
    SREvent *event = NULL;
    event = (SREvent *) g_malloc (sizeof (SREvent));
    if (event)
    {
	if ( !sre_init (event))
	{
	    g_free(event);
	    event = NULL;
	}
	else
	{
	    event->ref_count = 1;
	}
    }
    return event;
}

guint32 
sre_add_reference (SREvent *event)
{
    if ( !event )
	return SR_ERROR;

    (event->ref_count)++;

    return event->ref_count;
}

guint32 
sre_release_reference (SREvent *event)
{
    guint32 rv;
    if (!event)
	return SR_ERROR;
    (event->ref_count)--;
    rv = event->ref_count;
    if (event->ref_count == 0)
    {
	sre_terminate (event);
	g_free (event);
    };

    return rv;
}


gboolean 
sre_get_type (const SREvent *event, SREventType *type )
{
    if (!event || !type)
	return FALSE;

    *type = event->type;

    return TRUE ;
}

gboolean 
sre_get_time_stamp (const SREvent *event, SRTimeStamp *time)
{
    if (!event || !time)
	return FALSE;

    *time = event->time;

    return TRUE ;
}

gboolean 
sre_get_event_data (const SREvent *event, gpointer *data)
{

    if (!event || !data)
	return FALSE;

    *data = event->data ;

    return TRUE ;
}


/**
 * sru_exit_loop:
 *
 * Function called when want to exit monitoring events in system.
 * For more information see #sru_entry_loop.
 *
**/
void 
sru_exit_loop ()
{
    SPI_event_quit ();
}

/**
 * sru_entry_loop:
 *
 * Function called when want to start monitoring events in system.
 * This function does not return control. To exit call #sru_exit_loop
 * from within an event handler.
**/
void 
sru_entry_loop ()
{
    SPI_event_main ();
}

/**
 * sru_init:
 *
 * Function called to make all initialization for using at-spi.
 * Returns: 0 on success, otherwisw an integer error code.
 *
**/
int 
sru_init ()
{
    return SPI_init ();
}

/**
 * sru_exit:
 *
 * Function called when at-spi is not anymore used.
 * Returns: 0 if there were no leaks, otherwise non zero.
**/
int 
sru_terminate ()
{
#ifdef SRU_PARANOIA
    int rv  = 0;
    
    rv = SPI_exit ();
    bonobo_debug_shutdown ();
    return rv;
#else
    return SPI_exit ();
#endif
}

/* SREvent.h
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

#ifndef _SREVENT_H
#define _SREVENT_H

#include <glib.h>
#include <sys/time.h>



/**
 * SR_ERROR:
 * 
 * a constant indicating an error 
 *
**/
#define SR_ERROR	-1

typedef long int LONG ;

/*
 *
 * Screen Reader Event Types
 *
 */
typedef enum  
{
    SR_EVENT_NULL,
    
    SR_EVENT_SRO,
    SR_EVENT_WINDOW,
    SR_EVENT_TOOLTIP,
    SR_EVENT_MOUSE,
    SR_EVENT_MONITOR,
    SR_EVENT_MAG,
    SR_EVENT_KEYBOARD_ECHO,
    SR_EVENT_COMMAND_LAYER_CHANGED,
    SR_EVENT_COMMAND_LAYER,
    SR_EVENT_HOTKEY,
    SR_EVENT_KEY,
    SR_EVENT_CONFIG_CHANGED,
}SREventType;


/*
 *
 * Screen Reader Time Stamp
 *
 */
typedef guint64 SRTimeStamp;

/**
 * SREventDataDestructor:
 *
 * A function prototype for a function which will be used to destroy
 * data from the EventData member of SR_EVENT
 *
**/
typedef void (*SREventDataDestructor)(gpointer);

/*
 *
 * Screen Reader Event
 *
 */
/**
 * SREvent:
 * @ref_count: number of objects which point to this structure.
 * @type: #SR_EVENT_TYPE screan reader type of event.
 * @time: #SR_TIME_STAMP screen reader time of event.
 * @data: a pointer to the data incapsulated in the structure
 * @data_destructor: a pointer to a function, which destroy data
 * 		in @ata.
**/
typedef struct _SREvent
{
    guint32			ref_count;
    SREventType			type;
    SRTimeStamp			time;
    gpointer			data;
    SREventDataDestructor	data_destructor;
}SREvent;


/*
 *
 * Screen Reader Event's methods
 *
 */
SREvent *sre_new		();
guint32 sre_add_reference	(SREvent *event);
guint32 sre_release_reference	(SREvent *event);
gboolean sre_get_type		(const SREvent *event, SREventType *type);
gboolean sre_get_time_stamp	(const SREvent *event, SRTimeStamp *time);
gboolean sre_get_event_data	(const SREvent *event, void **data);


/**
 * SROnEventProc
 *
 * Prototype for a function called when an event occurs
 * 	event - a SREvent structure containing information
 *		about the event
 *	flags - not used yet
**/
typedef void (*SROnEventProc)
    (const SREvent *event, unsigned long flags);

/**
 * SROnConfigChanged
 *
 * Prototype for a function implemented in every module, which 
 * will be used by SRcore to notify them when configuration changes
 * occurs.
 * 	config_structure - a pointer to a SRConfigStructure defined in
 *			libsrconf.h, a structure which contains info
 * 			about the changes.
**/
typedef void (*SROnConfigChanged)
    (gpointer config_structure);

/**
 * sru_exit_loop:
 *
 * Function called when want to exit monitoring events in system. 
 * For more information see #sru_entry_loop.
 * 
**/
void sru_exit_loop ();

/**
 * sru_entry_loop:
 *
 * Function called when want to start monitoring events in system.
 * This function does not return control. To exit call #sru_exit_loop
 * from within an event handler.
**/
void sru_entry_loop ();

/**
 * sru_init:
 *
 * Function called to make all initialization for using at-spi.
 * Returns: 0 on success, otherwisw an integer error code.
 * 
**/
int sru_init ();

/**
 * sru_terminate:
 *
 * Function called when at-spi is not anymore used.
 * Returns: 0 if there were no leaks, otherwise non zero.
**/
int sru_terminate ();

#endif

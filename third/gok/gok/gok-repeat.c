/* gok-repeat.c
*
* Copyright 2003 Sun Microsystems, Inc.,
* Copyright 2003 University Of Toronto
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gok-repeat.h"
#include "gok-data.h"
#include "gok-log.h"
#include "main.h"

#define MIN_REPEAT_RATE 20

/* quick and dirty globals for now.. TODO: structify*/
static guint gokrepeattimerId;
static gboolean gokrepeatarmed = FALSE;
static gboolean gokrepeatstarted = FALSE;
static GokKey *armed_key = NULL;

/* private prototypes */
static gboolean gok_repeat_on_timer (gpointer);


/**
* gok_repeat_getArmed
*
* check whether the gok is in repeat mode or not.
*
* returns: gboolean.
**/
gboolean
gok_repeat_getArmed()
{
	return gokrepeatarmed;
}

/**
* gok_repeat_getArmed
*
* check whether the gok has starting repeating or not.
*
* returns: gboolean.
**/
gboolean
gok_repeat_getStarted()
{
	return gokrepeatstarted;
}


/**
* gok_repeat_toggle_armed
*
* toggles repeat mode on and off.
* @key: a pointer to the GokKey which is the current
* 'repeat next key' node.
*
* returns: boolean indicating whether resulting state is armed or not.
**/
gboolean
gok_repeat_toggle_armed (GokKey *key)
{
	if (gokrepeatarmed) 
	{
		gok_repeat_disarm ();
		return FALSE;
	}
	else
	{
		gok_repeat_arm (key);
		return TRUE;
	}
}


/**
* gok_repeat_arm
*
* turns repeat mode oon
*
* returns: void.
**/
void 
gok_repeat_arm(GokKey *key) 
{
	gok_log_enter();
	gokrepeatarmed = TRUE;
	armed_key = key;
	key->ComponentState.active = TRUE;
	gok_key_update_toggle_state (key);
	gok_log_leave();
}


/**
* gok_repeat_stop
*
* stops current repeating
*
* returns: void.
**/
void 
gok_repeat_stop()
{
	gok_log_enter();
	if (gokrepeatstarted == TRUE) {
		gokrepeatstarted = FALSE;
		g_source_remove(gokrepeattimerId);	
	}
	gok_log_leave();
}

/**
* gok_repeat_disarm
*
* turns repeat mode off
*
* returns: void.
**/
void 
gok_repeat_disarm()
{
	gok_log_enter();
	if (gokrepeatarmed == TRUE) {
		gokrepeatarmed = FALSE;
	}
	if (armed_key && GTK_IS_TOGGLE_BUTTON (armed_key->pButton)) 
	{
	        armed_key->ComponentState.active = FALSE;
	        gok_key_update_toggle_state (armed_key);
	}
	armed_key = NULL;
	gok_log_leave();
}


/**
* gok_repeat_key
* @pKey: pointer to the key to repeat..
*
* This is the "gok_repeat_start" function. It returns TRUE if the key
* is a repeatable key, and the key is going to now repeat.
*
* returns: gboolean.
**/
gboolean 
gok_repeat_key(GokKey* pKey)
{
	/* TODO use the parts of the key that are repeatable */
	gboolean returncode = FALSE;
	gint rate = 0;
	gok_log_enter();
	if (gok_key_isRepeatable(pKey) != TRUE) {
		gok_log_x("this key is not repeatable...");
	}
	else if (gokrepeatarmed != TRUE)
	{
		gok_log_x("Repeat is not armed. Aborting...");
	}
	else {
		rate = gok_data_get_repeat_rate();
		rate = rate * 10; /* conversion to milliseconds */
		rate = (rate > MIN_REPEAT_RATE)? rate : MIN_REPEAT_RATE;
		gokrepeattimerId = g_timeout_add_full (G_PRIORITY_HIGH_IDLE, 
			rate, gok_repeat_on_timer, (gpointer)pKey, NULL);
		gokrepeatstarted = TRUE;
		returncode = TRUE;
	}
	
	gok_log_leave();
	return returncode;
}

/* private timer handler */
static gboolean 
gok_repeat_on_timer (gpointer data)
{
	gok_log_enter();
	/* TODO: this key might no longer exist! need to clone...*/
	gok_keyboard_output_key (gok_main_get_current_keyboard (), (GokKey*)data);
	
	gok_log_leave();
	return TRUE;
	
}

void gok_repeat_drop_refs (GokKey *key)
{
      if (armed_key == key) 
      {
	   armed_key = NULL;
      }
}

/* coreconf.h
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

#ifndef __SRCORE__
#define __SRCORE__ 0


#include <glib.h>
/**
 * General Setting structure
**/
typedef struct
{
    gboolean  	magnifier; 	/*Run magnifier? */
    gboolean  	speech;		/*Run speech? */
    gboolean  	braille; 	/*Run braille? */
    gboolean	braille_monitor; /*Run braille monitor*/
	
    gboolean	minimize; 	/*Run Gnopernicus minimize? */
	
} General;

/**
 * Module state enum
**/

typedef enum
    {
	ACTIVE = 0,
	INACTIVE
    } ModuleState;


/**
 * Initialize SRCORE configuration listener			
**/
gboolean 	srcore_gconf_client_init 		(void);
void 		srcore_terminate			(General *general);
void 		srcore_exit_all 			(gboolean exit_val);
/**
 * General settings
**/
General* 	srcore_general_setting_init		(gboolean set_struct);
General* 	srcore_general_setting_new 		(void);
void 		srcore_general_setting_free 		(General* general);

/**
 * Load default General structure					
**/
void 		srcore_load_default_settings		(General* general);
void 		srcore_load_default_screen_review 	(void);


/**
 * Set Methods 							
**/
void		srcore_find_text_set 			(gchar 	  *text);
void 		srcore_general_setting_set 		(General  *general);
void 		srcore_magnifier_status_set 		(gboolean status);
void 		srcore_speech_status_set		(gboolean status);
void		srcore_braille_status_set		(gboolean status);
void		srcore_braille_monitor_status_set	(gboolean status);
void 		srcore_exit_ack_set			(gboolean ack);
void		srcore_first_run_set			(gboolean ack);
void		srcore_minimize_set			(gboolean minimize);
void		srcore_take_mouse_set 			(gboolean mouse_move);
void		srcore_simulate_click_set 		(gboolean simulate);
void 		srcore_language_set 			(gchar	  *id);
void		srcore_screen_review_set 		(gint flags);
/**
 * Get Methods 							
**/
gchar*		srcore_find_text_get 			(void);
gboolean 	srcore_magnifier_status_get 		(void);
gboolean 	srcore_speech_status_get 		(void);
gboolean 	srcore_braille_status_get 		(void);
gboolean	srcore_braille_monitor_status_get	(void);
gboolean	srcore_minimize_get			(void);
gboolean	srcore_take_mouse_get			(void);
gboolean	srcore_simulate_click_get		(void);
gchar*		srcore_language_get 			(void);
gchar*		srcore_ip_get 				(void);
gint		srcore_screen_review_get 		(void);
#endif

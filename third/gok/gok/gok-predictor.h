/* gok-predictor.h 
* 
* Copyright 2002 Sun Microsystems, Inc.,
* Copyright 2002 University Of Toronto 
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

#ifndef __defined_gok_predictor_h
#define __defined_gok_predictor_h

#include "arraylist.h"
#include "command-predictor.h"
#include "command-vertex-data.h"
#include "gok-keyboard.h"
#include "main.h"

/*
  constants
*/

/**
 * Maximum number of vertices in the prediction graph.
 */
#define GOK_PREDICTOR_MAX_VERTICES 1000


/*
  type definitions
*/

/*
 * Reference for a Gok_Predictor object.
 * It must be created by gok_predictor_open() and
 * freed by gok_predictor_close().
 */
typedef struct Gok_Predictor_Struct *Gok_Predictor;


/*
  function declarations
*/

/**
 * gok_predictor_open:
 *
 * Initialization function.
 * If a session file exists, it will recreate the previous state.
 *
 * Returns: a new Gok_Predictor object.
 */
Gok_Predictor gok_predictor_open();

/**
 * gok_predictor_close:
 * @gok_predictor: the instance of Gok_Predictor object to destroy
 *
 * Tear down function.
 * Releases the resources used by a Gok_Predictor object.
 * Also dumps the data to a session file.
 */
void gok_predictor_close(const Gok_Predictor gok_predictor);

/**
 * gok_predictor_add_key:
 * @gok_predictor: the instance of Gok_Predictor object
 * @vertex_data: a Command_Vertex_Data object that is created by
 * command_vertex_data_create(char *keyboard_id, char *key_id,
 * COMMAND_VERTEX_DATA_DEFAULT_WEIGHT)
 *
 * Adds a key to be used for upcoming predictions.
 */
void gok_predictor_add_key(const Gok_Predictor gok_predictor,
			   const Command_Vertex_Data vertex_data);

/**
 * gok_predictor_get:
 * @gok_predictor: the instance of Gok_Predictor object
 * @num_predictions:  number of key predictions to return
 * @keyboard_id: NOT IMPLEMENTED
 * @algorithm: an enumerated type that defines the algorithm used
 * for the prediction
 *
 * Gets the next n predictions according to the algorithm.
 *
 * Returns: an Arraylist of Command_Vertex_Data objects
 */
const Arraylist gok_predictor_get(const Gok_Predictor gok_predictor,
				  const int num_predictions,
				  const char *keyboard_id,
				  const Prediction_Algorithm algorithm);

/**
 * gok_predictor_remove_keyboard:
 * @gok_predictor: the instance of Gok_Predictor object
 * @keyboard_id: id of keyboard to remove from prediction
 *
 * Function to remove a whole keyboard from the prediction.
 */
void gok_predictor_remove_keyboard(const Gok_Predictor gok_predictor,
				   const char *keyboard_id);

/**
 * gok_predictor_remove_key:
 * @gok_predictor: the instance of Gok_Predictor object
 * @vertex_data: a Command_Vertex_Data object that is created by
 * command_vertex_data_create(char *keyboard_id, char *key_id,
 * COMMAND_VERTEX_DATA_DEFAULT_WEIGHT)
 *
 * Function to remove a key from the prediction.
 */
void gok_predictor_remove_key(const Gok_Predictor gok_predictor,
			      const Command_Vertex_Data vertex_data);

/* TODO - document these functions: */
void gok_predictor_on (gboolean bOnOff);
void gok_predictor_change_number_predictions (int Number);
gboolean gok_predictor_add_prediction_keys (GokKeyboard* pKeyboard);
int gok_predictor_predict ( Gok_Predictor gP );
/*int gok_predictor_predict ( Gok_Predictor gP, const gchar* in_Keyboard, const gchar* in_Key);*/
GokKey* gok_predictor_get_real(gchar* pKeyboardName, gchar* pKeyLabel);
void gok_predictor_log(Gok_Predictor gP, gchar* keyboardID, gchar* keyID);
Gok_Predictor gok_main_get_command_predictor(void); /* TODO - this should go in main.h - but circular include problem */



#endif /* __defined_gok_predictor_h */

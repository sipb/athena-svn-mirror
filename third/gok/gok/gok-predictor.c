/* gok-predictor.c 
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "gok-predictor.h"
#include "gok-keyslotter.h"
#include "command-vertex.h"
#include "gok-log.h"
#include "gok-modifier.h"
#include "gok-data.h"
#include "gok-keyboard.h"



/*
  structures
*/
struct Gok_Predictor_Struct {
  Command_Graph _command_graph;
  Command_Predictor _command_predictor;
  Arraylist _recency_list;
  Arraylist _frequency_list;  
};



/*
  private function declarations
*/
static void populate_arraylist_of_predictions(const Command_Vertex *predictions, Arraylist arraylist, int max_predictions);


Gok_Predictor gok_predictor_open()
{
	  Gok_Predictor gok_predictor;
	  
	  gok_log_enter();
	
#ifdef GOK_DEBUG
	  gok_predictor = malloc(sizeof(struct Gok_Predictor_Struct));
#else
	  gok_predictor = checked_malloc(sizeof(struct Gok_Predictor_Struct));
#endif
	  gok_predictor->_command_graph = command_graph_reconstruct_from_binary();
	  if (NULL == gok_predictor->_command_graph)
	  {
	      gok_predictor->_command_graph = command_graph_create();
	  }
	  gok_predictor->_command_predictor = command_predictor_create(gok_predictor->_command_graph);
	  gok_predictor->_recency_list = arraylist_create(command_vertex_data_equals);
	  gok_predictor->_frequency_list = arraylist_create(command_vertex_data_equals);
	
	  gok_predictor_on(TRUE); /* TODO get from gok-data */
	  gok_log_leave();
	  return gok_predictor;
}

void gok_predictor_close(const Gok_Predictor gok_predictor)
{
	  gok_log_enter();
	  command_graph_dump_to_binary(gok_predictor->_command_graph);
	
	  arraylist_free(gok_predictor->_frequency_list);
	  arraylist_free(gok_predictor->_recency_list);
	  command_graph_free(gok_predictor->_command_graph);
	  command_predictor_free(gok_predictor->_command_predictor);
	  free(gok_predictor);
	  gok_log_leave();
}

void gok_predictor_add_key(const Gok_Predictor gok_predictor, const Command_Vertex_Data vertex_data)
{
	  command_graph_add_vertex(gok_predictor->_command_graph, command_vertex_create(vertex_data));
}

const Arraylist gok_predictor_get(const Gok_Predictor gok_predictor,
				  const int num_predictions,
				  const char *keyboard_id,
				  const Prediction_Algorithm algorithm)
{
	  int max_predictions = (MAX_NUM_PREDICTIONS < num_predictions)? MAX_NUM_PREDICTIONS: num_predictions;
	gok_log_enter();
	  if (RECENCY == algorithm)
	  {
	      populate_arraylist_of_predictions(command_predictor_predict_same_as_last(gok_predictor->_command_predictor), gok_predictor->_recency_list, max_predictions);
	      
	gok_log_leave();
	      return gok_predictor->_recency_list;
	  }
	  else if (FREQUENCY == algorithm)
	  {
	      populate_arraylist_of_predictions(command_predictor_predict_high_stat(gok_predictor->_command_predictor), gok_predictor->_frequency_list, max_predictions);
	
	gok_log_leave();
	      return gok_predictor->_frequency_list;
	  }
	gok_log_leave();
	  return NULL;
}

static void populate_arraylist_of_predictions(const Command_Vertex *predictions, Arraylist arraylist, int max_predictions)
{
	  int index;
	
	  arraylist_clear(arraylist);
	  for (index = 0; (index < max_predictions) && (predictions[index] != NULL); index++)
	  {
	      arraylist_add(arraylist, command_vertex_get_vertex_data(predictions[index]));
	  }
}

void gok_predictor_remove_keyboard(const Gok_Predictor gok_predictor, const char *keyboard_id)
{
}

void gok_predictor_remove_key(const Gok_Predictor gok_predictor, const Command_Vertex_Data vertex_data)
{
}

/* *********************************************************************************************************** */

/**
* gok_predictor_on
* 
* Turns on (or off) the predictor.
* This adds (or removes) the predictor keys to every keyboard that supports 
* command prediction.
*
* returns: void.
**/
void gok_predictor_on (gboolean bOnOff)
{
	gok_log_enter();
	gok_keyslotter_on(bOnOff, KEYTYPE_COMMANDPREDICT);
	gok_log_leave();
}

/**
* gok_predictor_change_number_predictions
*
* @Number: Number of prediction keys.
*
* Changes the number of word prediction keys displayed on the keyboard. This
* should be called after the user has selected a new number from the settings
* dialog.
*
* returns: void
**/
void gok_predictor_change_number_predictions (int Number)
{
	gok_keyslotter_change_number_predictions( Number, KEYTYPE_COMMANDPREDICT );
}

/**
* gok_predictor_add_prediction_keys
*
* @pKeyboard: Pointer to the keyboard that gets the new prediction keys.
*
* Adds a row of prediction keys to the given keyboard.
*
* returns: TRUE if the prediction keys were added, FALSE if not.
**/
gboolean gok_predictor_add_prediction_keys (GokKeyboard* pKeyboard)
{
	return gok_keyslotter_add_prediction_keys (pKeyboard, KEYTYPE_COMMANDPREDICT);
}


/**
* gok_predictor_predict
* 
* Makes a prediction. If the currently displayed keyboard is showing prediction
* keys then they are filled in with the predictions.
*
* returns: The number of words predicted.
**/
int gok_predictor_predict ( Gok_Predictor gP )
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	GokKey* pRealKey;
	Arraylist predictions;
	Command_Vertex_Data cvd;
	gint count;
	gchar* keyboardID;
	gchar* keyID;
	gint practicalPredictions = 0;
	pRealKey = NULL;
	
	gok_log_enter();
	
	/* is command prediction turned on? */
	if (gok_data_get_commandprediction() == FALSE)
	{
		gok_log_leave();
		return 0;
	}
	
	/* get the currently displayed keyboard */
	pKeyboard = gok_main_get_current_keyboard();
	
	/* does it support command prediction? */
	if (gok_keyboard_get_supports_commandprediction (pKeyboard) == FALSE)
	{
		gok_log_leave();
		return 0;
	}

	/* TODO clear the predictor keys */

	/* make the prediction */
	predictions = gok_predictor_get( gP, gok_data_get_num_predictions(), gok_keyboard_get_name(pKeyboard), FREQUENCY);
	
	practicalPredictions = arraylist_size(predictions);
	
	if (practicalPredictions > gok_data_get_num_predictions())
	{
		practicalPredictions = gok_data_get_num_predictions();
	}
	
	
	/* fill in the command prediction keys */
	count = 0;
	pKey = pKeyboard->pKeyFirst;
	
	while (count < practicalPredictions)
	{
		cvd = (Command_Vertex_Data) arraylist_get( predictions, count ); /* cast is necessary as arraylist is generic */
		if (cvd != NULL)
		{
			/* copy the key info from this key to the prediction key */
			/* get the next word completion key on the keyboard */
			while (pKey != NULL)
			{
				if (pKey->Type == KEYTYPE_COMMANDPREDICT)
				{
					keyboardID = (gchar*)command_vertex_data_get_keyboard_id( cvd ); /* TODO - don't need yet? */
					keyID = (gchar*)command_vertex_data_get_key_id( cvd );
					/* find the real key */
					pRealKey = gok_predictor_get_real(keyboardID, keyID);
					if (pRealKey == NULL)
					{
						gok_log_x("badness in gok command prediction -- can't find predicted key!");
						break;/* TODO - fail gracefully */
					}
					/* store a pointer to the real key, on the predictor key */
					pKey->pMimicKey = pRealKey;
					/* copy the real key style into predictor key */
					pKey->Style = pRealKey->Style;
					gok_key_set_button_name(pKey); /* update style info */
					/*gtk_widget_set_style(pKey->pButton, pKey->Style);*/
					gok_key_set_button_label (pKey, keyID);
					gok_key_add_label (pKey, keyID, 0, 0, NULL);
									
					pKey = pKey->pKeyNext;
					pRealKey = NULL;
					break;
				}
				pKey = pKey->pKeyNext;
			}
		}
		else
		{
			gok_log_x("could not get vertex for command prediction!");
		}
		if (pKey == NULL)
		{
			break;
		}
		count++;
	}
		
		
	/* change the font size for the word completion keys */
	if (count > 0)
	{
		gok_keyboard_calculate_font_size_group (pKeyboard, FONT_SIZE_GROUP_WORDCOMPLETE, TRUE);
	}
	
	gok_log_leave();
	return count;
}


GokKey* gok_predictor_get_real(gchar* pKeyboardName, gchar* pKeyLabel)
{
	GokKeyboard* pKeyboard;
	GokKey* pKey;
	
	gok_log_enter();
	gok_log ("looking for keyboard:%s",pKeyboardName);
	/* figure out what key this predictor key is predicting*/
	pKeyboard = gok_main_get_first_keyboard();
	g_assert (pKeyboard != NULL);
	while (pKeyboard != NULL)
	{
		if ( strcmp(gok_keyboard_get_name(pKeyboard), pKeyboardName) == 0 )
		{
			gok_log ("looking for key:%s",pKeyLabel);
			pKey = pKeyboard->pKeyFirst;
			while (pKey != NULL)
			{
				if (pKey->Type != KEYTYPE_COMMANDPREDICT)
				{
					if (strcmp(gok_key_get_label(pKey),pKeyLabel) == 0)
					{
						gok_log_leave();
						return pKey;
					}
				}
				pKey = pKey->pKeyNext;
			}
		}
		pKeyboard = pKeyboard->pKeyboardNext;
	}
	gok_log_leave();
	return NULL;
}

void gok_predictor_log(Gok_Predictor gP, gchar* keyboardID, gchar* keyID)
{
	Command_Vertex_Data cvd;
	gok_log_enter();
	gok_log("logging keyboard:%s, and key:%s",keyboardID,keyID);
	cvd = command_vertex_data_create(keyboardID,keyID,1.0);
	if (cvd == NULL)
	{
		gok_log_x("could not create vertex for command prediction!");
	}
	else
	{
		gok_predictor_add_key(gP, cvd);
	}
	gok_log_leave();
}


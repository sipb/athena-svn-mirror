/* command-predictor.h 
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

#ifndef __defined_command_predictor_h
#define __defined_command_predictor_h

#include <stdlib.h>
#include "command-graph.h"
#include "command-vertex.h"


/*
  type definitions
*/
typedef struct Command_Predictor_Struct *Command_Predictor;

typedef enum {
  RECENCY,
  FREQUENCY
} Prediction_Algorithm;


/*
  function declarations
*/
void command_predictor_free(const Command_Predictor predictor);
Command_Predictor command_predictor_create(const Command_Graph graph);
void command_predictor_calculate_recency_percentages(const Command_Predictor predictor);
const float command_predictor_get_recency_percent_correct(const Command_Predictor predictor, const int index);
void command_predictor_calculate_frequency_percentages(const Command_Predictor predictor);
const float command_predictor_get_frequency_percent_correct(const Command_Predictor predictor, const int index);
const Command_Vertex* command_predictor_predict_same_as_last(const Command_Predictor predictor);
const Command_Vertex* command_predictor_predict_high_stat(const Command_Predictor predictor);
void command_predictor_evaluate_prediction(const Command_Predictor predictor,
					   const Command_Vertex actual_vertex,
					   const Prediction_Algorithm algorithm,
					   const int num_predictions);

#endif /* __defined_command_predictor_h */

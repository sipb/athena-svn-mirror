/* command-predictor.c
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

#include "command-predictor.h"


/*
  private variables
*/
static const size_t recency_size = sizeof(Command_Vertex) * MAX_NUM_PREDICTIONS;


/*
  structures
*/
struct Command_Predictor_Struct {
  Command_Graph _graph;

  int _recency_true_count[MAX_NUM_PREDICTIONS];
  float _recency_percent_correct[MAX_NUM_PREDICTIONS];
  Command_Vertex _predicted_recency_commands[MAX_NUM_PREDICTIONS];

  int _frequency_true_count[MAX_NUM_PREDICTIONS];
  float _frequency_percent_correct[MAX_NUM_PREDICTIONS];
  Command_Vertex _predicted_frequency_commands[MAX_NUM_PREDICTIONS];
};


/*
  private function declarations
*/
static void inc_recency_true_count(const Command_Predictor predictor, const int index);
static void inc_frequency_true_count(const Command_Predictor predictor, const int index);
static void populate_predicted_frequency_commands(Command_Vertex *predicted_frequency_commands,
						  const Arraylist vertices_list,
						  const int length);
static void do_evaluate_prediction(const Command_Predictor predictor,
				   const Command_Vertex *predicted_commands,
				   const Command_Vertex actual_vertex,
				   const int num_predictions,
				   void (*inc_true_count)(const Command_Predictor, const int));


void command_predictor_free(const Command_Predictor predictor)
{
  free(predictor);
}

Command_Predictor command_predictor_create(const Command_Graph graph)
{
  Command_Predictor predictor;
  int index;

#ifdef GOK_DEBUG
  predictor = malloc(sizeof(struct Command_Predictor_Struct));
#else
  predictor = checked_malloc(sizeof(struct Command_Predictor_Struct));
#endif
  predictor->_graph = graph;
  for (index = 0; index < MAX_NUM_PREDICTIONS; index++)
    {
      predictor->_recency_true_count[index] = 0;
      predictor->_recency_percent_correct[index] = 0.0;
      predictor->_predicted_recency_commands[index] = NULL;

      predictor->_frequency_true_count[index] = 0;
      predictor->_frequency_percent_correct[index] = 0.0;
      predictor->_predicted_frequency_commands[index] = NULL;
    }

  return predictor;
}

static void inc_recency_true_count(const Command_Predictor predictor, const int index)
{
  predictor->_recency_true_count[index]++;
}

static void inc_frequency_true_count(const Command_Predictor predictor, const int index)
{
  predictor->_frequency_true_count[index]++;
}

void command_predictor_calculate_recency_percentages(const Command_Predictor predictor)
{
  int index;

  for (index = 0; index < MAX_NUM_PREDICTIONS; index++)
    {
      predictor->_recency_percent_correct[index] =
	(float)predictor->_recency_true_count[index] / (float)command_graph_total_commands(predictor->_graph);
    }
}

const float command_predictor_get_recency_percent_correct(const Command_Predictor predictor, const int index)
{
  return predictor->_recency_percent_correct[index];
}

void command_predictor_calculate_frequency_percentages(const Command_Predictor predictor)
{
  int index;

  for (index = 0; index < MAX_NUM_PREDICTIONS; index++)
    {
      predictor->_frequency_percent_correct[index] =
	(float)predictor->_frequency_true_count[index] / (float)command_graph_total_commands(predictor->_graph);
    }
}

const float command_predictor_get_frequency_percent_correct(const Command_Predictor predictor, const int index)
{
  return predictor->_frequency_percent_correct[index];
}

const Command_Vertex* command_predictor_predict_same_as_last(const Command_Predictor predictor)
{
  memcpy(predictor->_predicted_recency_commands,
	 command_graph_get_last_n_added_vertices(predictor->_graph),
	 recency_size);

  return predictor->_predicted_recency_commands;
}

const Command_Vertex* command_predictor_predict_high_stat(const Command_Predictor predictor)
{
  int graph_size = command_graph_size(predictor->_graph);
  Arraylist vertices_list;

  if (graph_size < 1)
    {
      return predictor->_predicted_frequency_commands;
    }

  command_graph_sort_by_stat(predictor->_graph);
  vertices_list = command_graph_get_vertices(predictor->_graph);
  if (graph_size < MAX_NUM_PREDICTIONS)
    {
      populate_predicted_frequency_commands(predictor->_predicted_frequency_commands,
					    vertices_list,
					    graph_size);
    }
  else
    {
      populate_predicted_frequency_commands(predictor->_predicted_frequency_commands,
					    vertices_list,
					    MAX_NUM_PREDICTIONS);
    }

  return predictor->_predicted_frequency_commands;
}

static void populate_predicted_frequency_commands(Command_Vertex *predicted_frequency_commands,
						  const Arraylist vertices_list,
						  const int length)
{
  int index;

  for (index = 0; index < length; index++)
    {
      predicted_frequency_commands[index] = arraylist_get(vertices_list, index);
    }
}

void command_predictor_evaluate_prediction(const Command_Predictor predictor,
					   const Command_Vertex actual_vertex,
					   const Prediction_Algorithm algorithm,
					   const int num_predictions)
{
  if (RECENCY == algorithm)
    {
      do_evaluate_prediction(predictor,
			     predictor->_predicted_recency_commands,
			     actual_vertex,
			     num_predictions,
			     inc_recency_true_count);
    }
  else if (FREQUENCY == algorithm)
    {
      do_evaluate_prediction(predictor,
			     predictor->_predicted_frequency_commands,
			     actual_vertex,
			     num_predictions,
			     inc_frequency_true_count);
    }
}

static void do_evaluate_prediction(const Command_Predictor predictor,
				   const Command_Vertex *predicted_commands,
				   const Command_Vertex actual_vertex,
				   const int num_predictions,
				   void (*inc_true_count)(const Command_Predictor, const int))
{
  int index;

  for (index = 0; (NULL != predicted_commands[index]) && (index < num_predictions); index++)
    {
      if (command_vertex_equals(actual_vertex, predicted_commands[index]))
	{
	  (*inc_true_count)(predictor, num_predictions - 1);
	  return;
	}
    }
}

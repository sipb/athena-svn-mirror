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

#include "command-edge-data.h"

/*
  constants
*/
static const size_t float_size = sizeof(float);


/*
  structures
*/
struct Command_Edge_Data_Struct {
  float _weight;
};


Boolean command_edge_data_equals(const Object data_1, const Object data_2)
{
  return (command_edge_data_get_weight((Command_Edge_Data)data_1) ==
	  command_edge_data_get_weight((Command_Edge_Data)data_2));
}

void command_edge_data_free(const Command_Edge_Data data)
{
  free(data);
}

Command_Edge_Data command_edge_data_create(const float weight)
{
  Command_Edge_Data data;

#ifdef GOK_DEBUG
  data = malloc(sizeof(struct Command_Edge_Data_Struct));
#else
  data = checked_malloc(sizeof(struct Command_Edge_Data_Struct));
#endif
  command_edge_data_set_weight(data, weight);

  return data;
}

void command_edge_data_set_weight(const Command_Edge_Data data, const float weight)
{
  data->_weight = weight;
}

float command_edge_data_get_weight(const Command_Edge_Data data)
{
  return data->_weight;
}

void command_edge_data_dump_text(const Command_Edge_Data data, FILE *stream)
{
  fprintf(stream, "Edge Weight: %f", command_edge_data_get_weight(data));
}

void command_edge_data_dump_binary(const Command_Edge_Data data, FILE *stream)
{
  const float weight = command_edge_data_get_weight(data);

  fwrite(&weight, float_size, 1, stream);
}

/* command-vertex-data.h 
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

#ifndef __defined_command_vertex_data_h
#define __defined_command_vertex_data_h

#include "global.h"


/*
  constants
*/
#define COMMAND_VERTEX_DATA_DEFAULT_WEIGHT 1.0
#define COMMAND_VERTEX_DATA_WEIGHT_DELTA 1.0


/*
  type definitions
*/
typedef struct Command_Vertex_Data_Struct *Command_Vertex_Data;


/*
  function declarations
*/
const Boolean command_vertex_data_equals(const Object data_1, const Object data_2);
void command_vertex_data_free(const Command_Vertex_Data data);
Command_Vertex_Data command_vertex_data_create(const char *keyboard_id, const char *key_id, const float weight);
Command_Vertex_Data command_vertex_data_create_from_log(char *line);
const char *command_vertex_data_get_keyboard_id(const Command_Vertex_Data data);
const char *command_vertex_data_get_key_id(const Command_Vertex_Data data);
void command_vertex_data_set_weight(const Command_Vertex_Data data, const float weight);
float command_vertex_data_get_weight(const Command_Vertex_Data data);
void command_vertex_data_dump_text(const Command_Vertex_Data data, FILE *stream);
void command_vertex_data_dump_binary(const Command_Vertex_Data data, FILE *stream);


#endif /* __defined_command_vertex_data_h */

/* command-edge.c 
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

#include "command-edge.h"

/*
  constants
*/
static const size_t char_size = sizeof(char);
static const size_t short_size = sizeof(short);
static const size_t float_size = sizeof(float);


/*
  structures
*/
struct Command_Edge_Struct {
  Command_Edge_Data _edge_data;
  Command_Vertex _next_vertex;
};


const Boolean command_edge_equals(const Object edge_1, const Object edge_2)
{
  return (command_vertex_equals(command_edge_get_next_vertex((Command_Edge)edge_1),
				command_edge_get_next_vertex((Command_Edge)edge_2)));
}

void command_edge_free(const Command_Edge edge)
{
  command_edge_data_free(edge->_edge_data);
  free(edge);
}

Command_Edge command_edge_create(const Command_Vertex next_vertex, const Command_Edge_Data edge_data)
{
  Command_Edge edge;

#ifdef GOK_DEBUG
  edge = malloc(sizeof(struct Command_Edge_Struct));
#else
  edge = checked_malloc(sizeof(struct Command_Edge_Struct));
#endif
  edge->_edge_data = edge_data;
  edge->_next_vertex = next_vertex;

  return edge;
}

void command_edge_set_edge_data(const Command_Edge edge, const Command_Edge_Data edge_data)
{
  command_edge_data_free(edge->_edge_data);
  edge->_edge_data = edge_data;
}

const Command_Edge_Data command_edge_get_edge_data(const Command_Edge edge)
{
  return edge->_edge_data;
}

const Command_Vertex command_edge_get_next_vertex(const Command_Edge edge)
{
  return edge->_next_vertex;
}

void command_edge_dump_text(const Command_Edge edge, FILE *stream)
{
  fprintf(stream, "Edge: Next ");
  command_vertex_dump_text(edge->_next_vertex, stream);
  fprintf(stream, " ");
  command_edge_data_dump_text(command_edge_get_edge_data(edge), stream);
  fflush(stream);
}

void command_edge_dump_binary(const Command_Edge edge, FILE *stream)
{
  static const short type = (short)STORAGE_TYPE_COMMAND_EDGE;
  const char *keyboard_id = command_vertex_data_get_keyboard_id(command_vertex_get_vertex_data(command_edge_get_next_vertex(edge)));
  const short keyboard_id_length = (short)strlen(keyboard_id);
  const char *key_id = command_vertex_data_get_key_id(command_vertex_get_vertex_data(command_edge_get_next_vertex(edge)));
  const short key_id_length = (short)strlen(key_id);

  fwrite(&type, short_size, 1, stream);
  fwrite(&keyboard_id_length, short_size, 1, stream);
  fwrite(&key_id_length, short_size, 1, stream);
  fwrite(keyboard_id, char_size, keyboard_id_length, stream);
  fwrite(key_id, char_size, key_id_length, stream);
  command_edge_data_dump_binary(command_edge_get_edge_data(edge), stream);
  fflush(stream);
}

/* command-vertex.h 
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

#include "command-vertex.h"

/*
  constants
*/
static const size_t short_size = sizeof(short);


/*
  structures
*/
struct Command_Vertex_Struct {
  Command_Vertex_Data _vertex_data;
  Arraylist _outbound_edges;
};


/*
  private function declarations
*/
static const int command_edge_weight_sort(const Object edge_1, const Object edge_2);
static void remove_outbound_edges(const Command_Vertex vertex);


const Boolean command_vertex_equals(const Object vertex_1, const Object vertex_2)
{
  return command_vertex_data_equals(command_vertex_get_vertex_data((Command_Vertex)vertex_1),
				    command_vertex_get_vertex_data((Command_Vertex)vertex_2));
}

void command_vertex_free(const Command_Vertex vertex)
{
  command_vertex_data_free(vertex->_vertex_data);
  remove_outbound_edges(vertex);
  arraylist_free(vertex->_outbound_edges);
  free(vertex);
}

static void remove_outbound_edges(const Command_Vertex vertex)
{
  Arraylist edges_list = command_vertex_get_edges(vertex);
  int length = arraylist_size(edges_list);
  int i;

  for (i = 0; i < length; i++)
    {
      command_edge_free(arraylist_get(edges_list, i));
    }
}

Command_Vertex command_vertex_create(const Command_Vertex_Data vertex_data)
{
  Command_Vertex vertex;

  if (vertex_data == NULL)
    {
      return NULL;
    }
#ifdef GOK_DEBUG
  vertex = malloc(sizeof(struct Command_Vertex_Struct));
#else
  vertex = checked_malloc(sizeof(struct Command_Vertex_Struct));
#endif
  vertex->_vertex_data = vertex_data;
  vertex->_outbound_edges = arraylist_create(command_edge_equals);

  return vertex;
}

void command_vertex_set_vertex_data(const Command_Vertex vertex, const Command_Vertex_Data vertex_data)
{
  command_vertex_data_free(vertex->_vertex_data);
  vertex->_vertex_data = vertex_data;
}

const Command_Vertex_Data command_vertex_get_vertex_data(const Command_Vertex vertex)
{
  return vertex->_vertex_data;
}

void command_vertex_sort_by_stat(Command_Vertex vertex)
{
  arraylist_sort(command_vertex_get_edges(vertex), command_edge_weight_sort);
}

static const int command_edge_weight_sort(const Object edge_1, const Object edge_2)
{
  float edge_weight_1;
  float edge_weight_2;

  edge_weight_1 = command_edge_data_get_weight(command_edge_get_edge_data((Object)*(int*)edge_1));
  edge_weight_2 = command_edge_data_get_weight(command_edge_get_edge_data((Object)*(int*)edge_2));

  return (int)(edge_weight_2 - edge_weight_1);
}

const Command_Edge command_vertex_get_edge(const Command_Vertex vertex, const Command_Edge edge)
{
  Arraylist edges_list = command_vertex_get_edges(vertex);
  int index = arraylist_index_of(edges_list, edge);

  if (index > -1)
    {
      return (Command_Edge)arraylist_get(edges_list, index);
    }
  return NULL;
}

void command_vertex_remove_edge(const Command_Vertex vertex, const Command_Edge edge)
{
  Arraylist edges_list = command_vertex_get_edges(vertex);

  if (arraylist_contains(edges_list, edge))
    {
      arraylist_remove(edges_list, edge);
    }
  command_edge_free(edge);
}

void command_vertex_add_edge(const Command_Vertex vertex, const Command_Edge edge)
{
  Arraylist edges_list = command_vertex_get_edges(vertex);

  if (!arraylist_contains(edges_list, edge))
    {
      arraylist_add(edges_list, edge);
    }
  else
    {
      Command_Edge_Data actual_edge_data;
      float edge_weight;
      float actual_edge_weight;

      edge_weight = command_edge_data_get_weight(command_edge_get_edge_data(edge));
      actual_edge_data = command_edge_get_edge_data(arraylist_get(edges_list,
								  arraylist_index_of(edges_list, edge)));
      actual_edge_weight = command_edge_data_get_weight(actual_edge_data);
      command_edge_data_set_weight(actual_edge_data,
				   actual_edge_weight + edge_weight);
      command_edge_free(edge);
    }
}

const Arraylist command_vertex_get_edges(const Command_Vertex vertex)
{
  return vertex->_outbound_edges;
}

void command_vertex_dump_text(const Command_Vertex vertex, FILE *stream)
{
  fprintf(stream, "Vertex: ");
  command_vertex_data_dump_text(command_vertex_get_vertex_data(vertex), stream);
  fflush(stream);
}

void command_vertex_dump_binary(const Command_Vertex vertex, FILE *stream)
{
  static const short type = (short)STORAGE_TYPE_COMMAND_VERTEX;

  fwrite(&type, short_size, 1, stream);
  command_vertex_data_dump_binary(command_vertex_get_vertex_data(vertex), stream);
  fflush(stream);
}

/* command-graph.h 
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

#ifndef __defined_command_graph_h
#define __defined_command_graph_h

#include <sys/stat.h>
#include "global.h"
#include "arraylist.h"
#include "command-vertex.h"
#include "gnome.h"


/*
  constants
*/
#define MAX_NUM_PREDICTIONS 10


/*
  type definitions
*/
typedef struct Command_Graph_Struct *Command_Graph;


/*
  function declarations
*/
void command_graph_free(const Command_Graph graph);
Command_Graph command_graph_create();
void command_graph_add_vertex(const Command_Graph graph, const Command_Vertex vertex);
const Command_Vertex *command_graph_get_last_n_added_vertices(const Command_Graph graph);
void command_graph_remove_vertex(const Command_Graph graph, const Command_Vertex vertex);
Command_Vertex command_graph_get_vertex(const Command_Graph graph, const Command_Vertex vertex);
int command_graph_size(const Command_Graph graph);
int command_graph_total_commands(const Command_Graph graph);
Arraylist command_graph_get_vertices(const Command_Graph graph);
Command_Vertex command_graph_get_last_added_vertex(const Command_Graph graph);
void command_graph_sort_by_stat(const Command_Graph graph);
void command_graph_sort_by_keyboard(const Command_Graph graph);
void command_graph_dump_to_text(const Command_Graph graph, FILE *stream);
void command_graph_dump_to_binary(const Command_Graph graph);
Command_Graph command_graph_reconstruct_from_binary();


#endif /* __defined_command_graph_h */

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

#ifndef __defined_command_vertex_h
#define __defined_command_vertex_h

#include "global.h"
#include "arraylist.h"
#include "command-common.h"
#include "command-vertex-data.h"
#include "command-edge.h"


/*
  function declarations
*/
const Boolean command_vertex_equals(const Object vertex_1, const Object vertex_2);
void command_vertex_free(const Command_Vertex vertex);
Command_Vertex command_vertex_create(const Command_Vertex_Data vertex_data);
void command_vertex_set_vertex_data(const Command_Vertex vertex, const Command_Vertex_Data vertex_data);
const Command_Vertex_Data command_vertex_get_vertex_data(const Command_Vertex vertex);
void command_vertex_sort_by_stat(const Command_Vertex vertex);
const Command_Edge command_vertex_get_edge(const Command_Vertex vertex, const Command_Edge edge);
void command_vertex_remove_edge(const Command_Vertex vertex, const Command_Edge edge);
void command_vertex_add_edge(const Command_Vertex vertex, const Command_Edge edge);
const Arraylist command_vertex_get_edges(const Command_Vertex vertex);
void command_vertex_dump_text(const Command_Vertex vertex, FILE *stream);
void command_vertex_dump_binary(const Command_Vertex vertex, FILE *stream);


#endif /* __defined_command_vertex_h */

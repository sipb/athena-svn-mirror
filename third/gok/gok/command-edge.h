/* command-edge.h 
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

#ifndef __defined_command_edge_h
#define __defined_command_edge_h

#include "global.h"
#include "command-common.h"
#include "command-vertex.h"
#include "command-edge-data.h"
#include "command-vertex-data.h"


/*
  function declarations
*/
const Boolean command_edge_equals(const Object edge_1, const Object edge_2);
void command_edge_free(const Command_Edge edge);
Command_Edge command_edge_create(const Command_Vertex next_vertex, const Command_Edge_Data edge_data);
void command_edge_set_edge_data(const Command_Edge edge, const Command_Edge_Data edge_data);
const Command_Edge_Data command_edge_get_edge_data(const Command_Edge edge);
const Command_Vertex command_edge_get_next_vertex(const Command_Edge edge);
void command_edge_dump_text(const Command_Edge edge, FILE *stream);
void command_edge_dump_binary(const Command_Edge edge, FILE *stream);


#endif /* __defined_command_edge_h */

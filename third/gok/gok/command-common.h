/* command-common.h 
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

#ifndef __defined_command_common_h
#define __defined_command_common_h

/*
  constants
*/
#define STORAGE_TYPE_COMMAND_VERTEX (short)1
#define STORAGE_TYPE_COMMAND_EDGE (short)2


/*
  type definitions
*/
typedef struct Command_Vertex_Struct *Command_Vertex;
typedef struct Command_Edge_Struct *Command_Edge;


#endif /* __defined_command_common_h */

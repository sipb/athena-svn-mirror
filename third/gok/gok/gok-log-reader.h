/* gok-log-reader.c
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

#ifndef __defined_gok_log_reader_h
#define __defined_gok_log_reader_h

#include <stdio.h>
#include <sys/stat.h>
#include "global.h"
#include "command-vertex.h"
#include "command-vertex-data.h"


/*
  type definitions
*/
typedef struct Gok_Log_Reader_Struct *Gok_Log_Reader;

/*
  function declarations
*/
void gok_log_reader_free(Gok_Log_Reader reader);
Gok_Log_Reader gok_log_reader_create(const char *filename);
char *gok_log_reader_get_next_command(Gok_Log_Reader reader);
/* Command_Vertex gok_log_reader_create_command_vertex(char *command); */


#endif /* __defined_gok_log_reader_h */

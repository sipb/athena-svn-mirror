/* global.h 
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

#ifndef __defined_global_h
#define __defined_global_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/*
  constants
*/
#ifndef TRUE
#define TRUE 1
#endif /* TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

#ifndef EMPTY_STRING
#define EMPTY_STRING ""
#endif /* EMPTY_STRING */


/*
  type definitions
*/
#ifndef Boolean
#define Boolean short unsigned int
#endif /* Boolean */

#ifndef Object
#define Object void*
#endif /* Object */


/*
  function declarations
*/
Boolean string_equals(const char *string_1, const char *string_2);
Boolean string_not_equals(const char *string_1, const char *string_2);
Boolean string_empty(const char *string_1);
Boolean string_starts_with(const char *string_1, const char *string_2);
Boolean string_ends_with(const char *string_1, const char *string_2);
void string_trim(char *string_1);
void *checked_malloc(const size_t size);


#endif /* __defined_global_h */

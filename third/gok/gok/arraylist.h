/* gok-predictor.h 
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

#ifndef __defined_arraylist_h
#define __defined_arraylist_h

#include <stdlib.h>
#include <stdio.h>


/*
  constants
*/
#undef TRUE
#define TRUE 1

#undef FALSE
#define FALSE 0


/*
  type definitions
*/
#undef Boolean
#define Boolean short unsigned int

#undef Object
#define Object void*

typedef struct Arraylist_Struct *Arraylist;


/*
  function declarations
*/
void arraylist_free(const Arraylist list);
Arraylist arraylist_create(const Boolean (*equals)(const Object object_1, const Object object_2));
Boolean arraylist_add(const Arraylist list, Object object);
Boolean arraylist_remove(const Arraylist list, const Object object);
Boolean arraylist_contains(const Arraylist list, const Object object);
int arraylist_index_of(const Arraylist list, const Object object);
Boolean arraylist_is_empty(const Arraylist list);
int arraylist_size(const Arraylist list);
Object arraylist_get(const Arraylist list, const int index);
void arraylist_clear(const Arraylist list);
void arraylist_sort(const Arraylist list, const int (*compare)(const Object object_1, const Object object_2));


#endif /* __defined_arraylist_h */

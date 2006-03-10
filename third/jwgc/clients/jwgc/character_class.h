#ifndef character_class_TYPE
#define character_class_TYPE

/*
 *      Copyright (c) 1989 by the Massachusetts Institute of Technology.
 *      For copying and distribution information, see the file
 *      "mit-copyright.h".
 *
 *      Modified for jwgc by Daniel Henninger.
 */

#include "mit-copyright.h"

#include "new_string.h"

#define  NUMBER_OF_CHARACTERS   256

typedef char character_class[NUMBER_OF_CHARACTERS];

extern /* character_class */ char * string_to_character_class();

#endif
